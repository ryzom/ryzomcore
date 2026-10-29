// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "process_entry.h"

#include <QCoreApplication>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTime>
#include <QSharedPointer>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <signal.h>
#include <sys/prctl.h>
#endif

namespace {

/// Lines kept per process and per stream.
const int s_MaxLines = 5000;
/// A control service that has not connected by then counts as captured, so the start order goes on.
const qint64 s_CaptureTimeoutMs = 30000;

#ifdef Q_OS_WIN
/// A job object that takes the children down with the dashboard, so a crash leaves no orphans.
HANDLE jobObject()
{
	static HANDLE s_Job = []() -> HANDLE {
		HANDLE job = CreateJobObjectW(nullptr, nullptr);
		if (job == nullptr)
			return nullptr;
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
		memset(&info, 0, sizeof(info));
		info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &info, sizeof(info)))
		{
			CloseHandle(job);
			return nullptr;
		}
		return job;
	}();
	return s_Job;
}

void assignToJob(qint64 pid)
{
	HANDLE job = jobObject();
	if (job == nullptr || pid <= 0)
		return;
	HANDLE process = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, (DWORD)pid);
	if (process == nullptr)
		return;
	AssignProcessToJobObject(job, process);
	CloseHandle(process);
}
#endif

/// The child process with the platform ties to the dashboard.
class CChildProcess : public QProcess
{
public:
	explicit CChildProcess(QObject *parent)
	    : QProcess(parent)
	{
#if defined(Q_OS_LINUX) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		// the counterpart of the Windows job object
		setChildProcessModifier([]() { prctl(PR_SET_PDEATHSIG, SIGTERM); });
#endif
	}

#if defined(Q_OS_LINUX) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
protected:
	void setupChildProcess() override { prctl(PR_SET_PDEATHSIG, SIGTERM); }
#endif
};

/// Field escaping of the console channel, see CSocketDisplayer.
QString escapeField(const QString &value)
{
	QString out;
	out.reserve(value.size());
	for (QChar c : value)
	{
		if (c == '\\')
			out += "\\\\";
		else if (c == '\t')
			out += "\\t";
		else if (c == '\n')
			out += "\\n";
		else if (c == '\r')
			out += "\\r";
		else
			out += c;
	}
	return out;
}

QString unescapeField(const QString &value)
{
	QString out;
	out.reserve(value.size());
	for (int i = 0; i < value.size(); ++i)
	{
		QChar c = value[i];
		if (c == '\\' && i + 1 < value.size())
		{
			++i;
			QChar n = value[i];
			if (n == 't')
				out += '\t';
			else if (n == 'n')
				out += '\n';
			else if (n == 'r')
				out += '\r';
			else
				out += n;
		}
		else
		{
			out += c;
		}
	}
	return out;
}

/// The colours CWindowDisplayer::doDisplay assigns per log type; black is the default text colour.
QColor consoleColor(const QString &hex)
{
	bool ok = false;
	uint rgb = hex.toUInt(&ok, 16);
	if (!ok || rgb == 0)
		return QColor();
	return QColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

/// Output lines without terminal escape sequences, and only the text after the last
/// carriage return, which is the final state of a progress line.
QString plainOutput(const QString &raw)
{
	static const QRegularExpression s_Escapes("\x1b\\[[0-9;?]*[A-Za-z]|\x1b\\][^\x07]*(\x07|\x1b\\\\)|\x1b.");
	QString text = raw;
	if (text.endsWith('\r'))
		text.chop(1);
	int cr = text.lastIndexOf('\r');
	if (cr >= 0)
		text = text.mid(cr + 1);
	text.remove(s_Escapes);
	return text;
}

#ifndef Q_OS_WIN
/// Splits a command line the way a POSIX shell would for the simple cases: whitespace
/// separated words, single and double quotes, backslash escapes outside single quotes.
/// The same rules as servdash_cli, so one ini works under both.
QStringList splitArguments(const QString &text)
{
	QStringList words;
	QString current;
	bool inWord = false;
	QChar quote;
	for (int i = 0; i < text.size(); ++i)
	{
		QChar c = text[i];
		if (quote == '\'')
		{
			if (c == '\'')
				quote = QChar();
			else
				current += c;
		}
		else if (quote == '"')
		{
			if (c == '"')
				quote = QChar();
			else if (c == '\\' && i + 1 < text.size() && QString("\"\\$`").contains(text[i + 1]))
				current += text[++i];
			else
				current += c;
		}
		else if (c.isSpace())
		{
			if (inWord)
			{
				words.append(current);
				current.clear();
				inWord = false;
			}
		}
		else if (c == '\'' || c == '"')
		{
			quote = c;
			inWord = true;
		}
		else if (c == '\\')
		{
			if (i + 1 < text.size())
				current += text[++i];
			inWord = true;
		}
		else
		{
			current += c;
			inWord = true;
		}
	}
	if (inWord)
		words.append(current);
	return words;
}
#endif

QString safeName(const QString &name)
{
	QString out;
	for (QChar c : name)
		out += (c.isLetterOrNumber() && c.unicode() < 128) || c == '_' || c == '-' || c == '.' ? c : QChar('_');
	return out;
}

} // namespace

CProcessEntry::CProcessEntry(const CSectionConfig &config, QObject *parent)
    : QObject(parent)
    , m_Config(config)
{
	m_Title = config.label();
	if (!config.ReadyPattern.isEmpty())
		m_ReadyPattern = QRegularExpression(config.ReadyPattern);
	m_AutoLaunch = config.AutoStart;
	if (config.HasLaunchCtrl)
	{
		m_LaunchCtrl.reset(new CLaunchCtrl(config.LaunchCtrl, config.AutoStart, [this](const QString &text) { note(text, Qt::red); }));
		m_AutoLaunch = m_LaunchCtrl->autoLaunch();
	}
	if (config.Control)
	{
		m_Server = new QLocalServer(this);
		m_Server->setSocketOptions(QLocalServer::UserAccessOption);
		QString serverName = QString("servdash_gui.%1.%2").arg(QCoreApplication::applicationPid()).arg(safeName(config.Name));
		QLocalServer::removeServer(serverName);
		if (!m_Server->listen(serverName))
			note(QString("Cannot listen for the service console: %1").arg(m_Server->errorString()), Qt::red);
		connect(m_Server, &QLocalServer::newConnection, this, &CProcessEntry::onNewControlConnection);
	}
}

CProcessEntry::~CProcessEntry()
{
	if (m_Process != nullptr)
	{
		// no finished handling half way through destruction
		m_Process->disconnect(this);
		m_Process->kill();
		m_Process->waitForFinished(2000);
	}
}

void CProcessEntry::setLaunchBlocked(bool blocked)
{
	m_LaunchBlocked = blocked;
}

// *** state ***

void CProcessEntry::setState(TState state)
{
	m_State = state;
	int value = -1;
	switch (state)
	{
	case New: m_Started = false; break;
	case Launched:
	case Captured:
	case Ready:
		m_Started = true;
		value = CLaunchCtrl::Running;
		break;
	case Stopping:
		m_Started = true;
		value = CLaunchCtrl::Stop;
		break;
	case Stopped:
		m_Started = false;
		value = CLaunchCtrl::Stopped;
		break;
	}
	if (m_LaunchCtrl && value >= 0)
		m_LaunchCtrl->setValue(value);
	emit stateChanged(this);
}

void CProcessEntry::note(const QString &text, const QColor &color)
{
	CLine line;
	line.Text = "> " + text;
	line.Color = color;
	line.Raw = true;
	appendLine(line);
}

void CProcessEntry::appendLine(const CLine &line)
{
	CLine stamped = line;
	stamped.Time = QTime::currentTime().toString("HH:mm:ss");
	if (m_Lines.size() >= s_MaxLines)
		m_Lines.removeFirst();
	m_Lines.append(stamped);
	emit lineAdded(this, stamped);
}

// *** launching ***

void CProcessEntry::launch()
{
	// MainWindow.launchClicked: nothing starts once the window is closing, not even a relaunch
	if (running() || m_LaunchBlocked)
		return;
	if (m_ShutdownRequested)
	{
		// Restart(): the exit of the stopping process relaunches it
		m_ShutdownRequested = false;
		m_RestartOnce = true;
		return;
	}

	QString program = m_Config.program(m_Config.LaunchCmd);
	CChildProcess *process = new CChildProcess(this);
	process->setProgram(program);
	QString endpoint = m_Server != nullptr ? m_Server->fullServerName() : QString();
#ifdef Q_OS_WIN
	// the argument string goes to the process as it stands, like the Win32 tool does
	QString native = m_Config.LaunchArgs.trimmed();
	if (!endpoint.isEmpty())
		native += (native.isEmpty() ? "" : " ") + QString("--ctrl=") + endpoint;
	process->setNativeArguments(native);
#else
	QStringList arguments = splitArguments(m_Config.LaunchArgs);
	if (!endpoint.isEmpty())
		arguments << "--ctrl=" + endpoint;
	process->setArguments(arguments);
#endif
	if (!m_Config.WorkingDirectory.isEmpty())
		process->setWorkingDirectory(m_Config.WorkingDirectory);
	process->setProcessChannelMode(QProcess::SeparateChannels);
	connect(process, &QProcess::readyReadStandardOutput, this, &CProcessEntry::onReadyReadOutput);
	connect(process, &QProcess::readyReadStandardError, this, &CProcessEntry::onReadyReadError);
	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CProcessEntry::onFinished);
	process->start();
	if (!process->waitForStarted(10000))
	{
		note(QString("Cannot start '%1': %2").arg(program, process->errorString()), Qt::red);
		// a start that only timed out would still report finished when deleted
		process->disconnect(this);
		process->deleteLater();
		return;
	}
#ifdef Q_OS_WIN
	assignToJob(process->processId());
#endif
	m_Process = process;
	m_OutputBuffer.clear();
	m_ErrorBuffer.clear();
	m_KillRequested = false;
	m_ReadyTriggered = false;
	m_LaunchedAt.start();
	if (m_LaunchCtrl)
		m_LaunchCtrl->countLaunch();

	note("Process Started");
	setState(Launched);
	if (!m_Config.Control)
	{
		// the pipes are the console: captured at once, like TerminalHost
		setState(Captured);
		if (m_ReadyPattern.pattern().isEmpty())
		{
			m_ReadyTriggered = true;
			setState(Ready);
		}
	}
}

void CProcessEntry::killChild()
{
	if (m_Process != nullptr)
	{
		m_KillRequested = true;
		m_Process->kill();
	}
}

void CProcessEntry::shutdown()
{
	if (!running())
		return;
	if (m_ShutdownRequested)
	{
		killChild();
		return;
	}
	m_ShutdownRequested = true;

	QString via;
	if (m_Config.Control)
	{
		if (m_Control != nullptr)
		{
			m_Control->write("cmd\tquit\n");
			via = "quit over the console channel";
		}
		else
		{
			m_Process->write("quit\n");
			via = "quit on stdin";
		}
	}
	else if (!m_Config.ShutdownInput.isEmpty())
	{
		m_Process->write((m_Config.ShutdownInput + "\n").toUtf8());
		via = "the shutdown input";
	}
	else if (!m_Config.ShutdownCmd.isEmpty())
	{
		QString program = m_Config.program(m_Config.ShutdownCmd);
		QProcess *process = new QProcess(this);
		process->setProgram(program);
#ifdef Q_OS_WIN
		process->setNativeArguments(m_Config.ShutdownArgs.trimmed());
#else
		process->setArguments(splitArguments(m_Config.ShutdownArgs));
#endif
		if (!m_Config.WorkingDirectory.isEmpty())
			process->setWorkingDirectory(m_Config.WorkingDirectory);
		process->setProcessChannelMode(QProcess::MergedChannels);
		// its output joins the pane as plain lines; nothing waits for it
		QSharedPointer<QByteArray> buffer(new QByteArray());
		connect(process, &QProcess::readyReadStandardOutput, this, [this, process, buffer]() { readLines(*buffer, process->readAllStandardOutput(), false); });
		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process, buffer]() {
			readLines(*buffer, process->readAllStandardOutput(), false);
			flushLine(*buffer, false);
			process->deleteLater();
		});
		process->start();
		if (!process->waitForStarted(10000))
		{
			note(QString("Cannot run the shutdown command '%1': %2").arg(program, process->errorString()), Qt::red);
			process->deleteLater();
			killChild();
			via = "kill";
		}
		else
		{
			via = "the shutdown command";
		}
	}
	else
	{
		killChild();
		via = "kill";
	}

	note(QString("Stopping (%1)").arg(via));
	setState(Stopping);
}

void CProcessEntry::kill()
{
	if (!running())
		return;
	m_ShutdownRequested = true;
	note("Killing", Qt::red);
	killChild();
	if (m_State != Stopping)
		setState(Stopping);
}

void CProcessEntry::startButton()
{
	if (m_LaunchCtrl)
	{
		m_LaunchCtrl->writeCtrl(CLaunchCtrl::Launch);
		applyCtrlValue(CLaunchCtrl::Launch);
	}
	else
	{
		launch();
	}
}

void CProcessEntry::stopButton()
{
	if (m_LaunchCtrl && !m_LaunchCtrl->ctrlIs(CLaunchCtrl::Stop))
	{
		m_LaunchCtrl->writeCtrl(CLaunchCtrl::Stop);
		applyCtrlValue(CLaunchCtrl::Stop);
	}
	else
	{
		emit stopWanted(this);
	}
}

void CProcessEntry::sendInput(const QString &text)
{
	if (m_Process == nullptr)
	{
		note("The process is not running", QColor(0x80, 0x80, 0x00));
		return;
	}
	m_Process->write((text + "\n").toUtf8());
}

void CProcessEntry::sendCommand(const QString &text)
{
	if (m_Control != nullptr)
		m_Control->write(("cmd\t" + escapeField(text) + "\n").toUtf8());
	else
		sendInput(text);
}

void CProcessEntry::clearConsole()
{
	m_Lines.clear();
	emit linesReset(this);
}

void CProcessEntry::releaseProcess()
{
	if (m_Process == nullptr)
		return;
	m_Process->disconnect(this);
	m_Process->deleteLater();
	m_Process = nullptr;
}

void CProcessEntry::onFinished(int exitCode, QProcess::ExitStatus status)
{
	if (m_Process == nullptr)
		return;
	// the last output may still be waiting in the pipes, and the last line may lack its newline
	readLines(m_OutputBuffer, m_Process->readAllStandardOutput());
	readLines(m_ErrorBuffer, m_Process->readAllStandardError());
	flushLine(m_OutputBuffer, true);
	flushLine(m_ErrorBuffer, true);
	releaseProcess();
	QString how;
	if (status == QProcess::NormalExit)
		how = QString("code %1").arg(exitCode);
	else if (m_KillRequested)
		how = "killed";
	else
		how = "crashed";
	note(QString("Process Exited (%1)").arg(how));
	setState(Stopped);
	if (m_LaunchCtrl)
		m_LaunchCtrl->afterExit();
	bool relaunch = m_RestartOnce && !m_ShutdownRequested;
	m_RestartOnce = false;
	m_ShutdownRequested = false;
	if (relaunch)
		launch();
	emit exited(this);
}

// *** output ***

void CProcessEntry::readLines(QByteArray &buffer, const QByteArray &data, bool checkReady)
{
	buffer += data;
	int pos;
	while ((pos = buffer.indexOf('\n')) >= 0)
	{
		QByteArray raw = buffer.left(pos);
		buffer.remove(0, pos + 1);
		handleOutputLine(plainOutput(QString::fromUtf8(raw)), checkReady);
	}
}

void CProcessEntry::flushLine(QByteArray &buffer, bool checkReady)
{
	if (buffer.isEmpty())
		return;
	handleOutputLine(plainOutput(QString::fromUtf8(buffer)), checkReady);
	buffer.clear();
}

void CProcessEntry::onReadyReadOutput()
{
	if (m_Process != nullptr)
		readLines(m_OutputBuffer, m_Process->readAllStandardOutput());
}

void CProcessEntry::onReadyReadError()
{
	if (m_Process != nullptr)
		readLines(m_ErrorBuffer, m_Process->readAllStandardError());
}

void CProcessEntry::handleOutputLine(const QString &text, bool checkReady)
{
	CLine line;
	line.Text = text;
	line.Raw = true;
	appendLine(line);
	if (checkReady && !m_Config.Control && !m_ReadyTriggered && (m_State == Launched || m_State == Captured) && !m_ReadyPattern.pattern().isEmpty() && m_ReadyPattern.match(text).hasMatch())
	{
		m_ReadyTriggered = true;
		setState(Ready);
	}
}

// *** console channel ***

void CProcessEntry::onNewControlConnection()
{
	while (m_Server->hasPendingConnections())
	{
		QLocalSocket *socket = m_Server->nextPendingConnection();
		if (socket->state() != QLocalSocket::ConnectedState)
		{
			// gone again before we got here
			socket->deleteLater();
			continue;
		}
		if (m_Control != nullptr)
		{
			m_Control->disconnect(this);
			m_Control->deleteLater();
		}
		m_Control = socket;
		m_ControlBuffer.clear();
		connect(socket, &QLocalSocket::readyRead, this, &CProcessEntry::onControlReadyRead);
		connect(socket, &QLocalSocket::disconnected, this, &CProcessEntry::onControlDisconnected);
		// the service replays its history on connection, so its earlier lines go; the raw ones stay
		QVector<CLine> kept;
		for (const CLine &line : m_Lines)
			if (line.Raw)
				kept.append(line);
		m_Lines = kept;
		m_Labels.clear();
		m_Commands.clear();
		emit linesReset(this);
		emit labelsReset(this);
		emit commandsChanged(this);
		emit controlChanged(this);
		if (m_State == Launched)
		{
			setState(Captured);
			if (m_ReadyPattern.pattern().isEmpty())
			{
				m_ReadyTriggered = true;
				setState(Ready);
			}
		}
	}
}

void CProcessEntry::onControlDisconnected()
{
	QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
	if (socket == nullptr || socket != m_Control)
		return;
	m_Control->deleteLater();
	m_Control = nullptr;
	emit controlChanged(this);
}

void CProcessEntry::onControlReadyRead()
{
	QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
	if (socket == nullptr || socket != m_Control)
		return;
	m_ControlBuffer += socket->readAll();
	int pos;
	while ((pos = m_ControlBuffer.indexOf('\n')) >= 0)
	{
		QByteArray raw = m_ControlBuffer.left(pos);
		m_ControlBuffer.remove(0, pos + 1);
		if (raw.endsWith('\r'))
			raw.chop(1);
		handleControlLine(QString::fromUtf8(raw));
	}
}

void CProcessEntry::handleControlLine(const QString &line)
{
	QStringList fields = line.split('\t');
	for (QString &field : fields)
		field = unescapeField(field);
	const QString type = fields.value(0);
	if (type == "hello")
	{
		// nothing to answer
	}
	else if (type == "title")
	{
		m_Title = fields.value(1);
		emit titleChanged(this);
		if (!m_ReadyTriggered && (m_State == Launched || m_State == Captured) && !m_ReadyPattern.pattern().isEmpty() && m_ReadyPattern.match(m_Title).hasMatch())
		{
			m_ReadyTriggered = true;
			setState(Ready);
		}
	}
	else if (type == "label")
	{
		bool ok = false;
		int index = fields.value(1).toInt(&ok);
		if (!ok || index < 0 || index >= 256)
			return;
		while (m_Labels.size() <= index)
			m_Labels.append(QString());
		m_Labels[index] = fields.value(2);
		emit labelChanged(this, index, m_Labels[index]);
	}
	else if (type == "command")
	{
		CCommand command;
		command.Name = fields.value(1);
		if (command.Name.isEmpty())
			return;
		const QString kind = fields.value(2);
		command.Kind = kind == "o" ? HandlerKind : (kind == "m" ? MemberKind : CommandKind);
		command.Args = fields.value(3);
		command.Help = fields.value(4);
		if (m_Commands.size() < 10000 || m_Commands.contains(command.Name))
		{
			m_Commands.insert(command.Name, command);
			emit commandsChanged(this);
		}
	}
	else if (type == "uncommand")
	{
		if (m_Commands.remove(fields.value(1)) > 0)
			emit commandsChanged(this);
	}
	else if (type == "log")
	{
		QColor color = consoleColor(fields.value(1));
		QString text = fields.value(2);
		// every message ends with a newline; a message may hold several lines
		if (text.endsWith('\n'))
			text.chop(1);
		for (QString part : text.split('\n'))
		{
			if (part.endsWith('\r'))
				part.chop(1);
			CLine l;
			l.Text = part;
			l.Color = color;
			appendLine(l);
		}
	}
}

// *** launch control files and timeouts ***

void CProcessEntry::applyCtrlValue(int value)
{
	if (!m_LaunchCtrl || value == m_LaunchCtrl->value())
		return;
	if (value == CLaunchCtrl::Launch && !m_Started)
		emit launchWanted(this);
	else if (value == CLaunchCtrl::Stop && m_State != Stopping && m_Started)
		emit stopWanted(this);
	else
		m_LaunchCtrl->setValue(m_LaunchCtrl->value());
}

void CProcessEntry::tick()
{
	if (m_LaunchCtrl)
	{
		int stateValue = CLaunchCtrl::Launch;
		switch (m_State)
		{
		case Stopping: stateValue = CLaunchCtrl::Stop; break;
		case Stopped: stateValue = CLaunchCtrl::Stopped; break;
		case Launched:
		case Captured:
		case Ready: stateValue = CLaunchCtrl::Running; break;
		case New: stateValue = CLaunchCtrl::Launch; break;
		}
		int value = m_LaunchCtrl->poll(stateValue, running());
		if (value >= 0)
			applyCtrlValue(value);
	}
	if (m_Config.Control && m_State == Launched && running() && m_LaunchedAt.elapsed() > s_CaptureTimeoutMs)
	{
		// a service binary that does not know --ctrl opens its own window and never connects;
		// it must not hold the rest of the shard in the start order forever
		note(QString("No console connection after %1 s; treating the service as started").arg(s_CaptureTimeoutMs / 1000), QColor(0x80, 0x80, 0x00));
		setState(Captured);
		m_ReadyTriggered = true;
		setState(Ready);
	}
}
