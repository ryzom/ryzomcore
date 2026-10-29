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

#include "mcp_server.h"

#include "wire.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>
#include <QTimer>

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#ifdef Q_OS_WIN
#include <fcntl.h>
#include <io.h>
#endif

namespace {

const char *const s_ServerName = "servdash_gui";
/// Newest first; the client's version is echoed when it is one of these.
const char *const s_ProtocolVersions[] = { "2025-06-18", "2025-03-26", "2024-11-05" };
const int s_DiscoveryIntervalMs = 2000;
/// A dashboard whose endpoint did not answer is tried again after this.
const int s_RetryIntervalMs = 10000;
const int s_WaiterTickMs = 100;
/// The longest a tool call waits for anything.
const double s_MaxWaitSeconds = 300.0;
const int s_DefaultLogLines = 100;
const int s_MaxLogLines = 5000;
const int s_MaxLineChars = 2000;
/// send_input returns once the process has been quiet this long after answering.
const int s_InputIdleMs = 400;

const char *const s_Instructions = "Service dashboards of Ryzom Core on this machine, one per ini file: a shard "
                                   "(its services), or the patchman services of a server. The processes of a dashboard are the sections of "
                                   "its ini; a NeL service has a console channel with a command table (list_commands) and status labels, a "
                                   "plain program only its output. Log lines are indexed per process from 1 and never renumbered, and carry "
                                   "their arrival time; get_log selects by index range, time range or count and can keep following for a "
                                   "while, so a window of a log is reproducible. Run NeL commands with run_command and the arguments as a "
                                   "list: they reach the service already split, nothing is parsed, so values with spaces, quotes or "
                                   "semicolons are safe. The dashboard parameter can be left out while one dashboard runs.";

// *** arguments ***

QString argStr(const QJsonObject &args, const QString &key)
{
	const QJsonValue v = args.value(key);
	if (v.isString())
		return v.toString().trimmed().isEmpty() ? QString() : v.toString();
	if (v.isDouble())
		return QString::number(v.toDouble());
	return QString();
}

bool argDouble(const QJsonObject &args, const QString &key, double &value)
{
	const QJsonValue v = args.value(key);
	if (v.isDouble())
	{
		value = v.toDouble();
		return true;
	}
	if (v.isString())
	{
		bool ok = false;
		double parsed = v.toString().trimmed().toDouble(&ok);
		if (ok)
			value = parsed;
		return ok;
	}
	return false;
}

bool argU64(const QJsonObject &args, const QString &key, quint64 &value)
{
	double d = 0;
	if (!argDouble(args, key, d) || d < 0)
		return false;
	value = (quint64)d;
	return true;
}

QStringList argStrs(const QJsonObject &args, const QString &key)
{
	QStringList out;
	const QJsonValue v = args.value(key);
	if (v.isArray())
	{
		for (const QJsonValue &item : v.toArray())
			out.append(item.isString() ? item.toString() : QJsonDocument(QJsonArray { item }).toJson(QJsonDocument::Compact).mid(1).chopped(1));
	}
	else if (v.isString())
	{
		out.append(v.toString());
	}
	return out;
}

int waitMs(const QJsonObject &args, double defaultSeconds)
{
	double seconds = defaultSeconds;
	argDouble(args, "wait_seconds", seconds);
	seconds = qBound(0.0, seconds, s_MaxWaitSeconds);
	return (int)(seconds * 1000.0);
}

enum TPrefixMode
{
	PrefixNone,
	PrefixCompact,
	PrefixFull
};

TPrefixMode prefixMode(const QJsonObject &args)
{
	const QString mode = argStr(args, "prefix");
	if (mode == "compact")
		return PrefixCompact;
	if (mode == "full")
		return PrefixFull;
	return PrefixNone;
}

// *** formatting ***

qint64 nowMs()
{
	return QDateTime::currentMSecsSinceEpoch();
}

QString formatTime(qint64 ts)
{
	return QDateTime::fromMSecsSinceEpoch(ts).toString("HH:mm:ss.zzz");
}

QString formatDateTime(qint64 ts)
{
	return QDateTime::fromMSecsSinceEpoch(ts).toString("yyyy-MM-dd HH:mm:ss");
}

QString formatDuration(qint64 ms)
{
	const qint64 secs = qMax<qint64>(0, ms / 1000);
	if (secs >= 3600)
		return QString("%1h%2m").arg(secs / 3600).arg((secs % 3600) / 60, 2, 10, QChar('0'));
	if (secs >= 60)
		return QString("%1m%2s").arg(secs / 60).arg(secs % 60, 2, 10, QChar('0'));
	return QString("%1s").arg(secs);
}

QString displayState(const CProcInfo &info)
{
	return info.State.toLower();
}

bool isReady(const CProcInfo &info)
{
	return info.State == "Ready";
}

bool isRunning(const CProcInfo &info)
{
	return info.Started && (info.State == "Launched" || info.State == "Captured" || info.State == "Ready");
}

bool isStopped(const CProcInfo &info)
{
	return !info.Started && (info.State == "Stopped" || info.State == "New");
}

/// The NeL log prefix as CWindowDisplayer::stringifyMessage writes it: type, thread id, file,
/// line and function, each optional, then `: ` and the message.
struct CParsedLine
{
	QString Level;
	QString File;
	QString Line;
	QString Func;
	QString Msg;
};

bool parsePrefix(const QString &text, CParsedLine &parsed)
{
	static const QRegularExpression s_Prefix("^(?:(INF|WRN|ERR|DBG|AST|STT|UKN) +)?[0-9a-fA-F]{1,8}(?: +(\\S+) +(\\d+)(?: +(\\S+))?)?: (.*)$");
	const QRegularExpressionMatch match = s_Prefix.match(text);
	if (!match.hasMatch())
		return false;
	parsed.Level = match.captured(1);
	parsed.File = match.captured(2);
	parsed.Line = match.captured(3);
	parsed.Func = match.captured(4);
	parsed.Msg = match.captured(5);
	return true;
}

/// The log type of a line as get_log shows and filters it: the NeL type, `out` for plain output,
/// `dash` for a note of the dashboard, `log` for a console line without a prefix.
QString lineLevel(const CMirrorLine &line)
{
	if (line.Raw && line.Text.startsWith("> "))
		return "dash";
	CParsedLine parsed;
	if (parsePrefix(line.Text, parsed) && !parsed.Level.isEmpty())
		return parsed.Level;
	return line.Raw ? "out" : "log";
}

QString truncateChars(const QString &text, int max)
{
	if (text.size() <= max)
		return text;
	return text.left(max) + " [...]";
}

QString formatLine(const CMirrorLine &line, const QString &process, TPrefixMode mode)
{
	QString out = QString("%1 %2").arg(line.Idx).arg(formatTime(line.Ts));
	if (!process.isEmpty())
		out += QString(" [%1]").arg(process);
	const QString level = lineLevel(line);
	out += QString(" %1").arg(level, -4);
	QString body;
	CParsedLine parsed;
	const bool hasPrefix = parsePrefix(line.Text, parsed);
	switch (mode)
	{
	case PrefixFull: body = line.Text; break;
	case PrefixCompact:
		if (hasPrefix)
		{
			if (!parsed.File.isEmpty() && !parsed.Line.isEmpty())
				body += QString("%1:%2 ").arg(parsed.File, parsed.Line);
			if (!parsed.Func.isEmpty())
				body += parsed.Func + ": ";
			body += parsed.Msg;
		}
		else
		{
			body = line.Text;
		}
		break;
	case PrefixNone:
		if (hasPrefix)
			body = parsed.Msg;
		else if (level == "dash")
			body = line.Text.mid(2);
		else
			body = line.Text;
		break;
	}
	out += " " + truncateChars(body, s_MaxLineChars);
	return out;
}

/// An absolute time in milliseconds from the forms get_log accepts.
bool parseTime(const QString &textIn, qint64 &ms, QString &error)
{
	const QString text = textIn.trimmed();
	if (text.startsWith('-'))
	{
		// relative to now: -90s, -5m, -2h, -1d
		const QString rest = text.mid(1);
		if (rest.size() < 2)
		{
			error = QString("cannot read the relative time '%1'").arg(text);
			return false;
		}
		bool ok = false;
		const double value = rest.left(rest.size() - 1).toDouble(&ok);
		if (!ok)
		{
			error = QString("cannot read the relative time '%1'").arg(text);
			return false;
		}
		double secs = 0;
		switch (rest.at(rest.size() - 1).toLatin1())
		{
		case 's': secs = value; break;
		case 'm': secs = value * 60.0; break;
		case 'h': secs = value * 3600.0; break;
		case 'd': secs = value * 86400.0; break;
		default: error = QString("a relative time ends in s, m, h or d: '%1'").arg(text); return false;
		}
		ms = nowMs() - (qint64)(secs * 1000.0);
		return true;
	}
	bool numeric = false;
	const double number = text.toDouble(&numeric);
	if (numeric)
	{
		// Unix seconds, or milliseconds when too large for seconds
		ms = number > 1e11 ? (qint64)number : (qint64)(number * 1000.0);
		return true;
	}
	QDateTime parsed = QDateTime::fromString(text, Qt::ISODateWithMs);
	if (!parsed.isValid())
		parsed = QDateTime::fromString(text, Qt::ISODate);
	if (!parsed.isValid())
	{
		for (const char *format : { "yyyy-MM-dd HH:mm:ss.zzz", "yyyy-MM-dd HH:mm:ss", "yyyy-MM-dd HH:mm" })
		{
			parsed = QDateTime::fromString(text, format);
			if (parsed.isValid())
				break;
		}
	}
	if (parsed.isValid())
	{
		ms = parsed.toMSecsSinceEpoch();
		return true;
	}
	for (const char *format : { "HH:mm:ss.zzz", "HH:mm:ss", "HH:mm" })
	{
		const QTime time = QTime::fromString(text, format);
		if (time.isValid())
		{
			ms = QDateTime(QDate::currentDate(), time).toMSecsSinceEpoch();
			return true;
		}
	}
	error = QString("cannot read the time '%1': use HH:MM[:SS[.mmm]], an ISO 8601 date and time, Unix seconds or a relative -30s, -5m, -2h").arg(text);
	return false;
}

QString dashSummary(const CDashClient *dash)
{
	return QString("'%1' (id %2, %3, %4)").arg(dash->title(), dash->record().Id, dash->record().Tool, dash->record().IniPath);
}

QString processLine(const CMirrorProcess &p)
{
	const CProcInfo &info = p.Info;
	QStringList flags;
	if (info.HasPid)
		flags.append(QString("pid %1").arg(info.Pid));
	if (info.HasStartedAt)
		flags.append(QString("up %1").arg(formatDuration(nowMs() - info.StartedAt)));
	if (info.Control)
		flags.append(info.ControlConnected ? "console connected" : "console not connected");
	if (info.HasLaunchCtrl)
		flags.append(QString("launch_ctrl %1").arg(info.CtrlValue));
	if (info.RelaunchPending)
		flags.append(QString("restart pending (%1 quick exits)").arg(info.CrashChain));
	if (info.HasExitCode && !info.Started)
		flags.append(QString("last exit code %1").arg(info.ExitCode));
	if (p.Lines.isEmpty())
		flags.append("no lines");
	else
		flags.append(QString("lines %1..%2").arg(p.Lines.first().Idx).arg(p.Lines.last().Idx));
	return QString("%1 %2 %3  %4").arg(info.Name, -20).arg(displayState(info), -9).arg(flags.join(", "), info.Label == info.Name ? QString() : info.Label);
}

QJsonObject toolResult(const QString &text, bool isError)
{
	QJsonObject content;
	content["type"] = "text";
	content["text"] = text;
	QJsonObject result;
	result["content"] = QJsonArray { content };
	result["isError"] = isError;
	return result;
}

// *** tool schemas ***

QJsonObject prop(const QString &type, const QString &description)
{
	QJsonObject o;
	o["type"] = type;
	o["description"] = description;
	return o;
}

QJsonObject dashboardProp()
{
	return prop("string", "Which dashboard: its title, id or id prefix, ini path or the end of it. Optional while one dashboard runs.");
}

QJsonObject processProp()
{
	return prop("string", "The process: its section name in the ini, or its label.");
}

QJsonObject waitProp(const QString &what)
{
	return prop("number", QString("Seconds to wait for %1, 0 to return at once. At most 300.").arg(what));
}

QJsonObject prefixProp()
{
	QJsonObject o = prop("string", "How much of the NeL log prefix (type, thread, file, line, function) to show: none keeps the type only (default), compact adds file:line and function, full keeps the line as logged.");
	o["enum"] = QJsonArray { "none", "compact", "full" };
	return o;
}

QJsonObject stringArrayProp(const QString &description)
{
	QJsonObject items;
	items["type"] = "string";
	QJsonObject o;
	o["type"] = "array";
	o["items"] = items;
	o["description"] = description;
	return o;
}

QJsonObject tool(const QString &name, const QString &description, const QJsonObject &properties, const QStringList &required)
{
	QJsonObject schema;
	schema["type"] = "object";
	schema["properties"] = properties;
	schema["required"] = QJsonArray::fromStringList(required);
	QJsonObject o;
	o["name"] = name;
	o["description"] = description;
	o["inputSchema"] = schema;
	return o;
}

QJsonObject props(std::initializer_list<QPair<QString, QJsonObject>> list)
{
	QJsonObject o;
	for (const QPair<QString, QJsonObject> &kv : list)
		o[kv.first] = kv.second;
	return o;
}

} // namespace

// *** stdin ***

void CStdinReader::run()
{
	std::string line;
	while (std::getline(std::cin, line))
		emit lineRead(QByteArray::fromStdString(line));
	emit closed();
}

// *** the server ***

struct CMcpServer::CWaiter
{
	enum
	{
		Wait = 0,
		Progress = 1,
		Done = 2
	};
	std::function<int()> Check;
	std::function<void(bool)> Finish;
	QDeadlineTimer Deadline;
	int IdleMs = 0;
	bool Progressed = false;
	QDeadlineTimer Idle;
};

CMcpServer::CMcpServer(QObject *parent)
    : QObject(parent)
{
	m_DiscoveryTimer = new QTimer(this);
	m_DiscoveryTimer->setInterval(s_DiscoveryIntervalMs);
	connect(m_DiscoveryTimer, &QTimer::timeout, this, &CMcpServer::onDiscoveryTick);
	m_WaiterTimer = new QTimer(this);
	m_WaiterTimer->setInterval(s_WaiterTickMs);
	connect(m_WaiterTimer, &QTimer::timeout, this, &CMcpServer::onWaiterTick);
}

void CMcpServer::start(const QStringList &inis)
{
#ifdef Q_OS_WIN
	// the protocol is newline framed JSON; no text mode translation on the way out
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	for (const QString &ini : inis)
	{
		QString error;
		if (!startDashboardProcess(ini, error) && !error.isEmpty())
			std::fprintf(stderr, "%s\n", qPrintable(error));
	}
	onDiscoveryTick();
	m_DiscoveryTimer->start();
	m_WaiterTimer->start();
	m_Stdin = new CStdinReader();
	connect(m_Stdin, &CStdinReader::lineRead, this, &CMcpServer::onLineRead);
	connect(m_Stdin, &CStdinReader::closed, this, &CMcpServer::onStdinClosed);
	m_Stdin->start();
}

void CMcpServer::onStdinClosed()
{
	// the client closing stdin ends the server; the dashboards stay
	emit finished();
}

void CMcpServer::writeMessage(const QJsonValue &msg)
{
	const QByteArray bytes = Wire::encode(msg);
	std::fwrite(bytes.constData(), 1, (size_t)bytes.size(), stdout);
	std::fflush(stdout);
}

void CMcpServer::reply(const QJsonValue &id, const QJsonValue &result)
{
	QJsonObject o;
	o["jsonrpc"] = "2.0";
	o["id"] = id;
	o["result"] = result;
	writeMessage(o);
}

void CMcpServer::replyError(const QJsonValue &id, int code, const QString &message)
{
	QJsonObject error;
	error["code"] = code;
	error["message"] = message;
	QJsonObject o;
	o["jsonrpc"] = "2.0";
	o["id"] = id;
	o["error"] = error;
	writeMessage(o);
}

void CMcpServer::onLineRead(const QByteArray &line)
{
	if (line.trimmed().isEmpty())
		return;
	QJsonParseError parseError;
	const QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);
	if (!doc.isObject())
	{
		replyError(QJsonValue(), -32700, QString("parse error: %1").arg(parseError.errorString()));
		return;
	}
	handle(doc.object());
}

void CMcpServer::handle(const QJsonObject &msg)
{
	const QString method = msg.value("method").toString();
	const QJsonObject params = msg.value("params").toObject();
	// notifications need no answer; a message without a method is a response to a request we
	// never sent
	if (!msg.contains("id") || method.isEmpty() || method.startsWith("notifications/"))
		return;
	const QJsonValue id = msg.value("id");
	if (method == "initialize")
	{
		const QString requested = params.value("protocolVersion").toString();
		QString version = s_ProtocolVersions[0];
		for (const char *known : s_ProtocolVersions)
			if (requested == known)
				version = known;
		QJsonObject tools;
		tools["listChanged"] = false;
		QJsonObject capabilities;
		capabilities["tools"] = tools;
		QJsonObject info;
		info["name"] = s_ServerName;
		info["version"] = QCoreApplication::applicationVersion().isEmpty() ? "0.1.0" : QCoreApplication::applicationVersion();
		QJsonObject result;
		result["protocolVersion"] = version;
		result["capabilities"] = capabilities;
		result["serverInfo"] = info;
		result["instructions"] = s_Instructions;
		reply(id, result);
	}
	else if (method == "ping")
	{
		reply(id, QJsonObject());
	}
	else if (method == "tools/list")
	{
		QJsonObject result;
		result["tools"] = toolList();
		reply(id, result);
	}
	else if (method == "tools/call")
	{
		callTool(params.value("name").toString(), params.value("arguments").toObject(), [this, id](const QString &text, bool isError) { reply(id, toolResult(text, isError)); });
	}
	else if (method == "resources/list")
	{
		QJsonObject result;
		result["resources"] = QJsonArray();
		reply(id, result);
	}
	else if (method == "resources/templates/list")
	{
		QJsonObject result;
		result["resourceTemplates"] = QJsonArray();
		reply(id, result);
	}
	else if (method == "prompts/list")
	{
		QJsonObject result;
		result["prompts"] = QJsonArray();
		reply(id, result);
	}
	else
	{
		replyError(id, -32601, QString("method not found: %1").arg(method));
	}
}

QJsonValue CMcpServer::toolList() const
{
	QJsonArray tools;
	tools.append(tool("list_dashboards", "The dashboards running on this machine for this user: shards and patchman services, with their processes counted.", QJsonObject(), {}));
	tools.append(tool("start_dashboard", "Starts a dashboard window for an ini file (a shard) when none runs for it, and waits until it answers. Its auto start processes come up in order.",
	    props({ { "ini_path", prop("string", "Path of the dashboard ini file.") }, { "wait_seconds", waitProp("the dashboard to answer (default 15)") } }), { "ini_path" }));
	tools.append(tool("shutdown_dashboard", "Stops every process of a dashboard and exits it, like the Shutdown All button followed by closing the window.",
	    props({ { "dashboard", dashboardProp() }, { "wait_seconds", waitProp("the dashboard to exit (default 60)") } }), {}));
	tools.append(tool("list_processes", "The processes of a dashboard with their state, pid, uptime, launch control and log line range.", props({ { "dashboard", dashboardProp() } }), {}));
	tools.append(tool("get_process", "Everything about one process: configuration, state, the service's status labels and buttons, its log range and command table size.",
	    props({ { "dashboard", dashboardProp() }, { "process", processProp() } }), { "process" }));
	tools.append(tool("start_process", "Starts a process, through its launch control file when it has one so the admin executor stays in step; the Start button.",
	    props({ { "dashboard", dashboardProp() }, { "process", processProp() }, { "wait_seconds", waitProp("the process to be ready (default 0)") } }), { "process" }));
	tools.append(tool("stop_process", "Asks a process to stop (quit over its console channel, or its shutdown command); the Stop button. A second stop kills it.",
	    props({ { "dashboard", dashboardProp() }, { "process", processProp() }, { "wait_seconds", waitProp("the process to exit (default 0)") } }), { "process" }));
	tools.append(tool("kill_process", "Kills a process at once.", props({ { "dashboard", dashboardProp() }, { "process", processProp() }, { "wait_seconds", waitProp("the process to exit (default 0)") } }), { "process" }));
	tools.append(tool("restart_process", "Stops a process, waits for it to exit, starts it again and waits for it to be ready.",
	    props({ { "dashboard", dashboardProp() }, { "process", processProp() }, { "wait_seconds", waitProp("each of the two steps (default 60)") } }), { "process" }));
	tools.append(tool("start_all", "Starts every process of a dashboard in the configured order; the Launch All button.", props({ { "dashboard", dashboardProp() }, { "wait_seconds", waitProp("every process to be ready (default 0)") } }), {}));
	tools.append(tool("stop_all", "Stops every running process of a dashboard; the Shutdown All button.", props({ { "dashboard", dashboardProp() }, { "wait_seconds", waitProp("every process to exit (default 0)") } }), {}));
	QJsonObject stateProp = prop("string", "The state to wait for.");
	stateProp["enum"] = QJsonArray { "ready", "running", "stopped" };
	tools.append(tool("wait_for_state", "Waits until a process reaches a state: ready (the service is up), running (started, console attached or not), stopped.",
	    props({ { "dashboard", dashboardProp() }, { "process", processProp() }, { "state", stateProp }, { "wait_seconds", waitProp("the state (default 60)") } }), { "process", "state" }));
	tools.append(tool("list_commands",
	    "The command table of a NeL service as it reports it: commands and variables with their argument syntax and help, named handler instances and their member commands (instance.command). Filter by prefix or a word of the name or help.",
	    props({ { "dashboard", dashboardProp() }, { "process", processProp() }, { "prefix", prop("string", "Only names starting with this (case sensitive, as the commands are).") },
	        { "contains", prop("string", "Only entries whose name or help contains this (case insensitive).") }, { "limit", prop("integer", "At most this many entries (default 300).") } }),
	    { "process" }));
	tools.append(tool("run_command",
	    "Runs a NeL command on a service over its console channel and returns what it logged. The arguments go as a list and reach the command as given, nothing is parsed. A variable is read by its name alone and set with one argument.",
	    props({ { "dashboard", dashboardProp() }, { "process", processProp() }, { "command", prop("string", "The command or variable name, instance.command for a member command.") },
	        { "args", stringArrayProp("The arguments, one per entry.") }, { "wait_seconds", waitProp("the command to complete (default 10)") }, { "prefix", prefixProp() } }),
	    { "process", "command" }));
	tools.append(tool("send_input", "Writes a line to the stdin of a process (a plain program, or a NeL service without its console channel) and returns what it printed in response.",
	    props({ { "dashboard", dashboardProp() }, { "process", processProp() }, { "text", prop("string", "The line to write, without the newline.") }, { "wait_seconds", waitProp("output to settle (default 3)") },
	        { "prefix", prefixProp() } }),
	    { "process", "text" }));
	tools.append(tool("get_log",
	    "Lines of a process log, or of every process of the dashboard merged by time. Select by index range (from_idx, to_idx), by time range (from_time, to_time), or the last N lines; then optionally keep following for follow_seconds, up to max_lines in total. Each line: index, time, log type, message. Indices are per process, from 1, never reused.",
	    props({ { "dashboard", dashboardProp() }, { "process", prop("string", "The process; leave out for every process of the dashboard, each line then names its process.") },
	        { "last", prop("integer", "The last N lines of the selection (default 100 when no range is given).") }, { "from_idx", prop("integer", "First line index, inclusive.") },
	        { "to_idx", prop("integer", "Last line index, inclusive.") },
	        { "from_time", prop("string", "Earliest arrival time: HH:MM[:SS[.mmm]] today, an ISO 8601 date and time, Unix seconds or milliseconds, or relative such as -90s, -5m, -2h.") },
	        { "to_time", prop("string", "Latest arrival time, same forms.") },
	        { "follow_seconds", prop("number", "After the selection, keep collecting new lines for this long (at most 300), stopping early at max_lines.") },
	        { "max_lines", prop("integer", "At most this many lines in the answer (default 200, at most 5000).") }, { "prefix", prefixProp() },
	        { "levels", stringArrayProp("Only these log types: INF, WRN, ERR, DBG, AST, STT; out (plain output of the process), dash (notes of the dashboard).") },
	        { "match", prop("string", "Only lines matching this regular expression (case insensitive).") } }),
	    {}));
	tools.append(tool("clear_log", "Drops the held lines of a process; indices keep counting.", props({ { "dashboard", dashboardProp() }, { "process", processProp() } }), { "process" }));
	return tools;
}

void CMcpServer::callTool(const QString &name, const QJsonObject &args, const TReply &reply)
{
	if (name == "list_dashboards")
		toolListDashboards(reply);
	else if (name == "start_dashboard")
		toolStartDashboard(args, reply);
	else if (name == "shutdown_dashboard")
		toolShutdownDashboard(args, reply);
	else if (name == "list_processes")
		toolListProcesses(args, reply);
	else if (name == "get_process")
		toolGetProcess(args, reply);
	else if (name == "start_process")
		toolProcessOrder(args, "Start", reply);
	else if (name == "stop_process")
		toolProcessOrder(args, "Stop", reply);
	else if (name == "kill_process")
		toolProcessOrder(args, "Kill", reply);
	else if (name == "restart_process")
		toolRestartProcess(args, reply);
	else if (name == "start_all")
		toolAll(args, true, reply);
	else if (name == "stop_all")
		toolAll(args, false, reply);
	else if (name == "wait_for_state")
		toolWaitForState(args, reply);
	else if (name == "list_commands")
		toolListCommands(args, reply);
	else if (name == "run_command")
		toolRunCommand(args, reply);
	else if (name == "send_input")
		toolSendInput(args, reply);
	else if (name == "get_log")
		toolGetLog(args, reply);
	else if (name == "clear_log")
		toolClearLog(args, reply);
	else
		reply(QString("unknown tool '%1'").arg(name), true);
}

// *** waiters ***

void CMcpServer::addWaiter(int waitMs, int idleMs, const std::function<int()> &check, const std::function<void(bool)> &finish)
{
	if (check() == CWaiter::Done)
	{
		finish(true);
		return;
	}
	if (waitMs <= 0)
	{
		finish(false);
		return;
	}
	CWaiter *waiter = new CWaiter();
	waiter->Check = check;
	waiter->Finish = finish;
	waiter->Deadline = QDeadlineTimer(waitMs);
	waiter->IdleMs = idleMs;
	m_Waiters.append(waiter);
}

void CMcpServer::pollWaiters()
{
	// a finish may add waiters of its own (a restart waits twice), so this walks a copy
	const QList<CWaiter *> waiters = m_Waiters;
	for (CWaiter *waiter : waiters)
	{
		if (!m_Waiters.contains(waiter))
			continue;
		const int result = waiter->Check();
		bool finished = false;
		bool reached = false;
		if (result == CWaiter::Done)
		{
			finished = true;
			reached = true;
		}
		else
		{
			if (result == CWaiter::Progress)
			{
				waiter->Progressed = true;
				if (waiter->IdleMs > 0)
					waiter->Idle = QDeadlineTimer(waiter->IdleMs);
			}
			if (waiter->Deadline.hasExpired() || (waiter->IdleMs > 0 && waiter->Progressed && waiter->Idle.hasExpired()))
				finished = true;
		}
		if (finished)
		{
			m_Waiters.removeOne(waiter);
			waiter->Finish(reached);
			delete waiter;
		}
	}
}

void CMcpServer::onWaiterTick()
{
	pollWaiters();
}

void CMcpServer::onClientChanged()
{
	pollWaiters();
}

// *** discovery ***

void CMcpServer::onDiscoveryTick()
{
	for (const Registry::CRecord &record : Registry::read())
	{
		if (m_Dashes.contains(record.Id))
			continue;
		QHash<QString, QDeadlineTimer>::iterator retry = m_RetryAfter.find(record.Id);
		if (retry != m_RetryAfter.end())
		{
			if (!retry.value().hasExpired())
				continue;
			m_RetryAfter.erase(retry);
		}
		attach(record);
	}
}

void CMcpServer::attach(const Registry::CRecord &record)
{
	CDashClient *client = new CDashClient(record, this);
	connect(client, &CDashClient::connected, this, &CMcpServer::onClientChanged);
	connect(client, &CDashClient::changed, this, &CMcpServer::onClientChanged);
	connect(client, &CDashClient::gone, this, &CMcpServer::onClientGone);
	m_Dashes.insert(record.Id, client);
	client->open();
}

void CMcpServer::onClientGone()
{
	CDashClient *client = qobject_cast<CDashClient *>(sender());
	if (client == nullptr)
		return;
	const QString id = client->record().Id;
	if (!client->everConnected())
	{
		std::fprintf(stderr, "cannot reach the dashboard '%s' at %s\n", qPrintable(client->record().Title), qPrintable(client->record().Endpoint));
		m_RetryAfter.insert(id, QDeadlineTimer(s_RetryIntervalMs));
	}
	m_Dashes.remove(id);
	client->deleteLater();
	pollWaiters();
}

bool CMcpServer::startDashboardProcess(const QString &iniPath, QString &error)
{
	QFileInfo info(iniPath);
	if (!info.exists())
	{
		error = QString("cannot find '%1'").arg(iniPath);
		return false;
	}
	const QString id = Registry::idFor(info.absoluteFilePath());
	for (const Registry::CRecord &record : Registry::read())
		if (record.Id == id)
			return false;
	// a window of this tool for the ini, detached: it outlives the server
	if (!QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList() << info.absoluteFilePath(), info.absolutePath()))
	{
		error = QString("cannot start servdash_gui for '%1'").arg(iniPath);
		return false;
	}
	return true;
}

// *** resolution ***

CDashClient *CMcpServer::findDash(const QJsonObject &args, QString &error) const
{
	QList<CDashClient *> dashes;
	for (CDashClient *client : m_Dashes)
		if (client->isConnected())
			dashes.append(client);
	std::sort(dashes.begin(), dashes.end(), [](const CDashClient *a, const CDashClient *b) {
		return a->record().Started != b->record().Started ? a->record().Started < b->record().Started : a->record().Id < b->record().Id;
	});
	if (dashes.isEmpty())
	{
		error = "no dashboard is running; start_dashboard starts one from its ini file";
		return nullptr;
	}
	const QString selector = argStr(args, "dashboard");
	QStringList names;
	for (const CDashClient *dash : dashes)
		names.append(dashSummary(dash));
	if (selector.isEmpty())
	{
		if (dashes.size() == 1)
			return dashes.first();
		error = "several dashboards run, name one with the dashboard parameter:\n" + names.join("\n");
		return nullptr;
	}
	const QString lowered = selector.trimmed().toLower();
	QList<CDashClient *> matches;
	for (CDashClient *dash : dashes)
	{
		const Registry::CRecord &r = dash->record();
		const QString ini = r.IniPath.toLower();
		if (r.Id.startsWith(lowered) || dash->title().toLower() == lowered || r.Title.toLower() == lowered || ini == lowered
		    || QString(ini).replace('\\', '/').endsWith(QString(lowered).replace('\\', '/')))
			matches.append(dash);
	}
	if (matches.size() == 1)
		return matches.first();
	error = matches.isEmpty() ? QString("no running dashboard matches '%1'; running:\n%2").arg(selector, names.join("\n")) : QString("'%1' matches several dashboards; use the id").arg(selector);
	return nullptr;
}

CDashClient *CMcpServer::findTarget(const QJsonObject &args, int &index, QString &error) const
{
	CDashClient *dash = findDash(args, error);
	if (dash == nullptr)
		return nullptr;
	const QString name = argStr(args, "process");
	if (name.isEmpty())
	{
		error = "the process parameter is required";
		return nullptr;
	}
	const QString wanted = name.trimmed();
	index = dash->indexOf(wanted);
	if (index >= 0)
		return dash;
	const QString lowered = wanted.toLower();
	QList<int> matches;
	const QVector<CMirrorProcess> &procs = dash->processes();
	for (int i = 0; i < procs.size(); ++i)
	{
		const CProcInfo &info = procs[i].Info;
		if (info.Name.toLower() == lowered || info.Label.toLower() == lowered || info.Title.toLower() == lowered)
			matches.append(i);
	}
	if (matches.size() == 1)
	{
		index = matches.first();
		return dash;
	}
	if (matches.isEmpty())
	{
		QStringList names;
		for (const CMirrorProcess &p : procs)
			names.append(p.Info.Name);
		error = QString("no process '%1' in %2; processes: %3").arg(name, dashSummary(dash), names.join(", "));
	}
	else
	{
		error = QString("'%1' names several processes of %2; use the section name").arg(name, dashSummary(dash));
	}
	return nullptr;
}

// *** dashboard tools ***

void CMcpServer::toolListDashboards(const TReply &reply)
{
	QList<CDashClient *> dashes;
	for (CDashClient *client : m_Dashes)
		if (client->isConnected())
			dashes.append(client);
	if (dashes.isEmpty())
	{
		reply("No dashboard is running. start_dashboard starts one from its ini file.", false);
		return;
	}
	std::sort(dashes.begin(), dashes.end(), [](const CDashClient *a, const CDashClient *b) { return a->record().Started < b->record().Started; });
	QString out = QString("%1 dashboard(s):\n").arg(dashes.size());
	for (const CDashClient *dash : dashes)
	{
		int started = 0;
		int ready = 0;
		for (const CMirrorProcess &p : dash->processes())
		{
			if (p.Info.Started)
				++started;
			if (isReady(p.Info))
				++ready;
		}
		out += QString("- %1: %2 processes, %3 started, %4 ready; dashboard pid %5, since %6\n")
		           .arg(dashSummary(dash))
		           .arg(dash->processes().size())
		           .arg(started)
		           .arg(ready)
		           .arg(dash->record().Pid)
		           .arg(formatDateTime(dash->record().Started * 1000));
	}
	reply(out, false);
}

void CMcpServer::toolStartDashboard(const QJsonObject &args, const TReply &reply)
{
	const QString ini = argStr(args, "ini_path");
	if (ini.isEmpty())
	{
		reply("ini_path is required", true);
		return;
	}
	QFileInfo info(ini);
	if (!info.exists())
	{
		reply(QString("cannot find '%1'").arg(ini), true);
		return;
	}
	const QString id = Registry::idFor(info.absoluteFilePath());
	QString error;
	const bool started = startDashboardProcess(info.absoluteFilePath(), error);
	if (!started && !error.isEmpty())
	{
		reply(error, true);
		return;
	}
	m_RetryAfter.remove(id);
	const QString path = QDir::toNativeSeparators(info.absoluteFilePath());
	const int wait = waitMs(args, 15.0);
	addWaiter(
	    wait, 0,
	    [this, id]() {
		    CDashClient *dash = m_Dashes.value(id, nullptr);
		    if (dash == nullptr)
			    onDiscoveryTick();
		    return dash != nullptr && dash->isConnected() ? (int)CWaiter::Done : (int)CWaiter::Wait;
	    },
	    [this, id, started, path, wait, reply](bool reached) {
		    CDashClient *dash = m_Dashes.value(id, nullptr);
		    if (reached && dash != nullptr)
			    reply(QString("%1 the dashboard %2.").arg(started ? "Started" : "Already running:", dashSummary(dash)), false);
		    else
			    reply(QString("%1 the dashboard for '%2', but it did not answer within %3 s.").arg(started ? "Started" : "Found", path).arg(wait / 1000.0), false);
	    });
}

void CMcpServer::toolShutdownDashboard(const QJsonObject &args, const TReply &reply)
{
	QString error;
	CDashClient *dash = findDash(args, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const QString id = dash->record().Id;
	const QString summary = dashSummary(dash);
	dash->send(QJsonValue("Shutdown"));
	const int wait = waitMs(args, 60.0);
	if (wait == 0)
	{
		reply(QString("Asked %1 to stop its processes and exit.").arg(summary), false);
		return;
	}
	addWaiter(
	    wait, 0, [this, id]() { return m_Dashes.contains(id) ? (int)CWaiter::Wait : (int)CWaiter::Done; },
	    [this, id, summary, wait, reply](bool reached) {
		    if (reached)
		    {
			    reply(QString("The dashboard %1 stopped its processes and exited.").arg(summary), false);
			    return;
		    }
		    QStringList left;
		    CDashClient *dash = m_Dashes.value(id, nullptr);
		    if (dash != nullptr)
			    for (const CMirrorProcess &p : dash->processes())
				    if (p.Info.Started)
					    left.append(p.Info.Name);
		    reply(QString("The dashboard %1 is still stopping after %2 s; still running: %3. It exits when they are gone.").arg(summary).arg(wait / 1000.0).arg(left.join(", ")), false);
	    });
}

// *** process tools ***

void CMcpServer::toolListProcesses(const QJsonObject &args, const TReply &reply)
{
	QString error;
	CDashClient *dash = findDash(args, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	QString out = QString("%1: %2 processes\n").arg(dashSummary(dash)).arg(dash->processes().size());
	for (const CMirrorProcess &p : dash->processes())
		out += processLine(p) + "\n";
	reply(out, false);
}

void CMcpServer::toolGetProcess(const QJsonObject &args, const TReply &reply)
{
	QString error;
	int index = -1;
	CDashClient *dash = findTarget(args, index, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const CMirrorProcess &p = dash->processes()[index];
	const CProcInfo &info = p.Info;
	QString out = QString("Process '%1' of %2\n").arg(info.Name, dashSummary(dash));
	out += QString("state: %1%2\n").arg(displayState(info), info.Started ? " (started)" : "");
	out += QString("label: %1\ntitle: %2\n").arg(info.Label, info.Title);
	out += QString("command: %1\n").arg(info.Command);
	if (!info.WorkingDirectory.isEmpty())
		out += QString("working directory: %1\n").arg(info.WorkingDirectory);
	if (info.HasPid)
		out += QString("pid: %1\n").arg(info.Pid);
	if (info.HasStartedAt)
		out += QString("started: %1 (up %2)\n").arg(formatDateTime(info.StartedAt), formatDuration(nowMs() - info.StartedAt));
	if (info.HasExitCode)
		out += QString("last exit code: %1\n").arg(info.ExitCode);
	out += QString("priority: %1\n").arg(info.HasPriority ? QString::number(info.Priority) : "none (starts one at a time)");
	out += QString("auto start: %1\n").arg(info.AutoStart ? "true" : "false");
	if (info.Control)
	{
		if (info.ControlConnected)
			out += QString("console channel: connected (protocol %1, exec %2)\n").arg(info.ChannelVersion).arg(info.ChannelVersion >= 2 ? "supported" : "not supported, run_command needs a service built with it");
		else
			out += "console channel: not connected\n";
	}
	if (info.HasLaunchCtrl)
		out += QString("launch control: yes, value %1\n").arg(info.CtrlValue);
	if (info.RelaunchPending)
		out += QString("restart pending after %1 quick exits\n").arg(info.CrashChain);
	const quint64 first = p.Lines.isEmpty() ? 0 : p.Lines.first().Idx;
	const quint64 last = p.Lines.isEmpty() ? 0 : p.Lines.last().Idx;
	out += QString("log: %1 lines held, idx %2..%3, next %4\n").arg(p.Lines.size()).arg(first).arg(last).arg(info.NextIdx);
	out += QString("commands known: %1\n").arg(p.Commands.size());
	QStringList values;
	QStringList buttons;
	for (const QString &label : info.Labels)
	{
		if (label.isEmpty())
			continue;
		if (label.startsWith('@'))
		{
			const QString rest = label.mid(1);
			const int bar = rest.indexOf('|');
			buttons.append(bar >= 0 ? QString("%1 (%2)").arg(rest.left(bar), rest.mid(bar + 1)) : rest);
		}
		else
		{
			values.append(label);
		}
	}
	if (!values.isEmpty())
		out += QString("status labels: %1\n").arg(values.join(" | "));
	if (!buttons.isEmpty())
		out += QString("buttons (name (command), run with run_command): %1\n").arg(buttons.join(", "));
	reply(out, false);
}

void CMcpServer::toolProcessOrder(const QJsonObject &args, const QString &order, const TReply &reply)
{
	QString error;
	int index = -1;
	CDashClient *dash = findTarget(args, index, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const CProcInfo before = dash->processes()[index].Info;
	const QString name = before.Name;
	QJsonObject fields;
	fields["name"] = name;
	dash->send(Wire::message(order, fields));
	const QString was = before.RelaunchPending ? ", restart pending" : (before.Started ? ", started" : "");
	const QString head = QString("Sent %1 to '%2' (was %3%4).").arg(order.toLower(), name, displayState(before), was);
	const int wait = waitMs(args, 0.0);
	if (wait == 0)
	{
		reply(head, false);
		return;
	}
	QPointer<CDashClient> guard(dash);
	const bool start = order == "Start";
	addWaiter(
	    wait, 0,
	    [guard, index, start]() {
		    if (guard.isNull() || index >= guard->processes().size())
			    return (int)CWaiter::Done;
		    const CProcInfo &info = guard->processes()[index].Info;
		    return (start ? isReady(info) : isStopped(info)) ? (int)CWaiter::Done : (int)CWaiter::Wait;
	    },
	    [guard, index, head, wait, reply](bool reached) {
		    if (guard.isNull() || index >= guard->processes().size())
		    {
			    reply(head + " The dashboard went away meanwhile.", false);
			    return;
		    }
		    const CProcInfo &after = guard->processes()[index].Info;
		    if (reached)
			    reply(head + QString(" Now %1%2.").arg(displayState(after), after.HasPid ? QString(", pid %1").arg(after.Pid) : QString()), false);
		    else
			    reply(head + QString(" Still %1 after %2 s.").arg(displayState(after)).arg(wait / 1000.0), false);
	    });
}

void CMcpServer::toolRestartProcess(const QJsonObject &args, const TReply &reply)
{
	QString error;
	int index = -1;
	CDashClient *dash = findTarget(args, index, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const int wait = waitMs(args, 60.0);
	const QString name = dash->processes()[index].Info.Name;
	QPointer<CDashClient> guard(dash);
	QJsonObject fields;
	fields["name"] = name;
	const std::function<void(const QString &)> startStep = [this, guard, index, name, wait, fields, reply](const QString &head) {
		if (guard.isNull())
		{
			reply(head + "The dashboard went away meanwhile.", false);
			return;
		}
		guard->send(Wire::message("Start", fields));
		addWaiter(
		    wait, 0,
		    [guard, index]() {
			    if (guard.isNull() || index >= guard->processes().size())
				    return (int)CWaiter::Done;
			    return isReady(guard->processes()[index].Info) ? (int)CWaiter::Done : (int)CWaiter::Wait;
		    },
		    [guard, index, head, wait, reply](bool reached) {
			    if (guard.isNull() || index >= guard->processes().size())
			    {
				    reply(head + "The dashboard went away meanwhile.", false);
				    return;
			    }
			    const CProcInfo &after = guard->processes()[index].Info;
			    if (reached)
				    reply(head + QString("Started again, now ready%1.").arg(after.HasPid ? QString(" with pid %1").arg(after.Pid) : QString()), false);
			    else
				    reply(head + QString("Started again, still %1 after %2 s.").arg(displayState(after)).arg(wait / 1000.0), false);
		    });
	};
	if (!dash->processes()[index].Info.Started)
	{
		startStep(QString("'%1' was not running. ").arg(name));
		return;
	}
	dash->send(Wire::message("Stop", fields));
	addWaiter(
	    wait, 0,
	    [guard, index]() {
		    if (guard.isNull() || index >= guard->processes().size())
			    return (int)CWaiter::Done;
		    return isStopped(guard->processes()[index].Info) ? (int)CWaiter::Done : (int)CWaiter::Wait;
	    },
	    [guard, index, name, wait, reply, startStep](bool reached) {
		    if (!reached)
		    {
			    const QString state = guard.isNull() || index >= guard->processes().size() ? "gone" : displayState(guard->processes()[index].Info);
			    reply(QString("'%1' did not stop within %2 s (still %3); kill_process ends it").arg(name).arg(wait / 1000.0).arg(state), true);
			    return;
		    }
		    startStep(QString("'%1' stopped. ").arg(name));
	    });
}

void CMcpServer::toolAll(const QJsonObject &args, bool start, const TReply &reply)
{
	QString error;
	CDashClient *dash = findDash(args, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	dash->send(QJsonValue(start ? "StartAll" : "StopAll"));
	const QString head = QString("Sent %1 to %2.").arg(start ? "start all" : "stop all", dashSummary(dash));
	const int wait = waitMs(args, 0.0);
	if (wait == 0)
	{
		reply(head, false);
		return;
	}
	QPointer<CDashClient> guard(dash);
	addWaiter(
	    wait, 0,
	    [guard, start]() {
		    if (guard.isNull())
			    return (int)CWaiter::Done;
		    for (const CMirrorProcess &p : guard->processes())
			    if (!(start ? isReady(p.Info) : isStopped(p.Info)))
				    return (int)CWaiter::Wait;
		    return (int)CWaiter::Done;
	    },
	    [guard, start, head, wait, reply](bool reached) {
		    if (reached)
		    {
			    reply(head + QString(" Every process is %1.").arg(start ? "ready" : "stopped"), false);
			    return;
		    }
		    QStringList states;
		    if (!guard.isNull())
			    for (const CMirrorProcess &p : guard->processes())
				    states.append(QString("%1 %2").arg(p.Info.Name, displayState(p.Info)));
		    reply(head + QString(" After %1 s: %2.").arg(wait / 1000.0).arg(states.join(", ")), false);
	    });
}

void CMcpServer::toolWaitForState(const QJsonObject &args, const TReply &reply)
{
	const QString wanted = argStr(args, "state").toLower();
	std::function<bool(const CProcInfo &)> pred;
	if (wanted == "ready")
		pred = isReady;
	else if (wanted == "running")
		pred = isRunning;
	else if (wanted == "stopped")
		pred = isStopped;
	else
	{
		reply(QString("unknown state '%1'; ready, running or stopped").arg(wanted), true);
		return;
	}
	QString error;
	int index = -1;
	CDashClient *dash = findTarget(args, index, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const QString name = dash->processes()[index].Info.Name;
	const int wait = waitMs(args, 60.0);
	QPointer<CDashClient> guard(dash);
	addWaiter(
	    wait, 0,
	    [guard, index, pred]() {
		    if (guard.isNull() || index >= guard->processes().size())
			    return (int)CWaiter::Done;
		    return pred(guard->processes()[index].Info) ? (int)CWaiter::Done : (int)CWaiter::Wait;
	    },
	    [guard, index, name, wanted, wait, reply](bool reached) {
		    const QString state = guard.isNull() || index >= guard->processes().size() ? "gone" : displayState(guard->processes()[index].Info);
		    if (reached)
			    reply(QString("'%1' is %2 (state %3).").arg(name, wanted, state), false);
		    else
			    reply(QString("'%1' is not %2 after %3 s (state %4).").arg(name, wanted).arg(wait / 1000.0).arg(state), false);
	    });
}

// *** command tools ***

void CMcpServer::toolListCommands(const QJsonObject &args, const TReply &reply)
{
	QString error;
	int index = -1;
	CDashClient *dash = findTarget(args, index, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const CMirrorProcess &p = dash->processes()[index];
	if (!p.Info.Control)
	{
		reply(QString("'%1' is not a NeL service with a console channel").arg(p.Info.Name), true);
		return;
	}
	if (p.Commands.isEmpty())
	{
		reply(QString("No command table from '%1' yet%2.").arg(p.Info.Name, p.Info.ControlConnected ? "" : " (console not connected)"), false);
		return;
	}
	const QString prefix = argStr(args, "prefix");
	const QString contains = argStr(args, "contains").toLower();
	quint64 limit = 300;
	argU64(args, "limit", limit);
	int shown = 0;
	int total = 0;
	QString out;
	for (const CMirrorCommand &command : p.Commands)
	{
		if (!prefix.isEmpty() && !command.Name.startsWith(prefix))
			continue;
		if (!contains.isEmpty() && !command.Name.toLower().contains(contains) && !command.Help.toLower().contains(contains))
			continue;
		++total;
		if ((quint64)shown >= limit)
			continue;
		++shown;
		QString line = command.Name;
		if (!command.Args.isEmpty())
			line += " " + command.Args;
		if (command.Kind == "Handler")
			line += " [handler instance]";
		else if (command.Kind == "Member")
			line += " [member]";
		if (!command.Help.isEmpty())
			line += " -- " + command.Help;
		out += line + "\n";
	}
	reply(QString("%1 of %2 commands of '%3'%4:\n").arg(shown).arg(total).arg(p.Info.Name, total > shown ? " (narrow with prefix, contains or limit)" : "") + out, false);
}

namespace {

QStringList linesFrom(const CMirrorProcess &p, quint64 fromIdx, TPrefixMode mode)
{
	QStringList out;
	for (const CMirrorLine &line : p.Lines)
		if (line.Idx >= fromIdx)
			out.append(formatLine(line, QString(), mode));
	return out;
}

quint64 startIndex(const CMirrorProcess &p)
{
	return qMax<quint64>(p.Info.NextIdx, p.Lines.isEmpty() ? 1 : p.Lines.last().Idx + 1);
}

} // namespace

void CMcpServer::toolRunCommand(const QJsonObject &args, const TReply &reply)
{
	const QString command = argStr(args, "command");
	if (command.isEmpty())
	{
		reply("the command parameter is required", true);
		return;
	}
	const QStringList commandArgs = argStrs(args, "args");
	const TPrefixMode mode = prefixMode(args);
	QString error;
	int index = -1;
	CDashClient *dash = findTarget(args, index, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const CMirrorProcess &p = dash->processes()[index];
	if (!p.Info.Control)
	{
		reply(QString("'%1' has no console channel; send_input writes to its stdin").arg(p.Info.Name), true);
		return;
	}
	if (!p.Info.ControlConnected)
	{
		reply(QString("the console of '%1' is not connected (%2); send_input reaches its stdin").arg(p.Info.Name, displayState(p.Info)), true);
		return;
	}
	const QString name = p.Info.Name;
	const quint64 startIdx = startIndex(p);
	const QString execId = QString("mcp-%1-%2").arg(QCoreApplication::applicationPid()).arg(++m_NextExecId);
	QJsonObject fields;
	fields["name"] = name;
	fields["id"] = execId;
	fields["command"] = command;
	fields["args"] = QJsonArray::fromStringList(commandArgs);
	dash->send(Wire::message("Exec", fields));
	QStringList shown;
	shown.append(command);
	for (const QString &arg : commandArgs)
		shown.append(QString("\"%1\"").arg(QString(arg).replace('\\', "\\\\").replace('"', "\\\"")));
	const QString shownText = shown.join(" ");
	const int wait = waitMs(args, 10.0);
	QPointer<CDashClient> guard(dash);
	std::shared_ptr<QPair<bool, bool>> result(new QPair<bool, bool>(false, false)); // taken, ok
	addWaiter(
	    wait, 0,
	    [guard, execId, result]() {
		    if (guard.isNull())
			    return (int)CWaiter::Done;
		    bool ok = false;
		    if (guard->takeExecResult(execId, ok))
		    {
			    result->first = true;
			    result->second = ok;
			    return (int)CWaiter::Done;
		    }
		    return (int)CWaiter::Wait;
	    },
	    [guard, index, name, startIdx, mode, shownText, wait, result, reply](bool reached) {
		    Q_UNUSED(reached);
		    QStringList lines;
		    if (!guard.isNull() && index < guard->processes().size())
			    lines = linesFrom(guard->processes()[index], startIdx, mode);
		    QString head;
		    if (guard.isNull())
			    head = QString("Command %1 on '%2': the dashboard went away").arg(shownText, name);
		    else if (result->first && result->second)
			    head = QString("Command %1 on '%2': ok").arg(shownText, name);
		    else if (result->first)
			    head = QString("Command %1 on '%2': FAILED (unknown command, bad usage, or no console channel)").arg(shownText, name);
		    else
			    head = QString("Command %1 on '%2': no completion marker within %3 s; the lines so far follow (a service built before the exec message never answers one, its output still shows)")
			               .arg(shownText, name)
			               .arg(wait / 1000.0);
		    reply(head + "\n" + lines.join("\n"), false);
	    });
}

void CMcpServer::toolSendInput(const QJsonObject &args, const TReply &reply)
{
	const QString text = argStr(args, "text");
	if (text.isEmpty())
	{
		reply("the text parameter is required", true);
		return;
	}
	const TPrefixMode mode = prefixMode(args);
	QString error;
	int index = -1;
	CDashClient *dash = findTarget(args, index, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const CMirrorProcess &p = dash->processes()[index];
	if (!p.Info.Started)
	{
		reply(QString("'%1' is not running").arg(p.Info.Name), true);
		return;
	}
	const QString name = p.Info.Name;
	const quint64 startIdx = startIndex(p);
	QJsonObject fields;
	fields["name"] = name;
	fields["text"] = text;
	dash->send(Wire::message("Input", fields));
	QPointer<CDashClient> guard(dash);
	std::shared_ptr<quint64> seen(new quint64(startIdx));
	// returns once the process answered and went quiet, or when the time is up
	addWaiter(
	    waitMs(args, 3.0), s_InputIdleMs,
	    [guard, index, seen]() {
		    if (guard.isNull() || index >= guard->processes().size())
			    return (int)CWaiter::Done;
		    const QVector<CMirrorLine> &lines = guard->processes()[index].Lines;
		    if (!lines.isEmpty() && lines.last().Idx >= *seen)
		    {
			    *seen = lines.last().Idx + 1;
			    return (int)CWaiter::Progress;
		    }
		    return (int)CWaiter::Wait;
	    },
	    [guard, index, name, text, startIdx, mode, reply](bool reached) {
		    Q_UNUSED(reached);
		    QStringList lines;
		    if (!guard.isNull() && index < guard->processes().size())
			    lines = linesFrom(guard->processes()[index], startIdx, mode);
		    reply(QString("Wrote \"%1\" to the stdin of '%2'; %3 line(s) followed:\n%4").arg(text, name).arg(lines.size()).arg(lines.join("\n")), false);
	    });
}

// *** log tools ***

namespace {

struct CLogFilter
{
	bool HasFromIdx = false, HasToIdx = false, HasFromTs = false, HasToTs = false;
	quint64 FromIdx = 0, ToIdx = 0;
	qint64 FromTs = 0, ToTs = 0;
	QStringList Levels;
	QRegularExpression Pattern;

	bool accepts(const CMirrorLine &line) const
	{
		if ((HasFromIdx && line.Idx < FromIdx) || (HasToIdx && line.Idx > ToIdx))
			return false;
		if ((HasFromTs && line.Ts < FromTs) || (HasToTs && line.Ts > ToTs))
			return false;
		if (!Levels.isEmpty() && !Levels.contains(lineLevel(line)))
			return false;
		if (Pattern.isValid() && !Pattern.pattern().isEmpty() && !Pattern.match(line.Text).hasMatch())
			return false;
		return true;
	}
};

struct CPicked
{
	qint64 Ts;
	quint64 Idx;
	int ProcIdx;
	QString Text;
};

struct CLogFollow
{
	QString Header;
	QVector<CPicked> Picked;
	QVector<int> ProcIndices;
	QVector<quint64> Cursors;
	int Followed = 0;
	int MaxLines = 0;
	bool Several = false;
	TPrefixMode Mode = PrefixNone;
	CLogFilter Filter;
	bool CanFollow = false;
	double FollowSeconds = 0;
};

} // namespace

void CMcpServer::toolGetLog(const QJsonObject &args, const TReply &reply)
{
	std::shared_ptr<CLogFollow> follow(new CLogFollow());
	follow->Mode = prefixMode(args);
	CLogFilter &filter = follow->Filter;
	filter.HasFromIdx = argU64(args, "from_idx", filter.FromIdx);
	filter.HasToIdx = argU64(args, "to_idx", filter.ToIdx);
	QString timeError;
	const QString fromTime = argStr(args, "from_time");
	if (!fromTime.isEmpty())
	{
		if (!parseTime(fromTime, filter.FromTs, timeError))
		{
			reply(timeError, true);
			return;
		}
		filter.HasFromTs = true;
	}
	const QString toTime = argStr(args, "to_time");
	if (!toTime.isEmpty())
	{
		if (!parseTime(toTime, filter.ToTs, timeError))
		{
			reply(timeError, true);
			return;
		}
		filter.HasToTs = true;
	}
	for (const QString &level : argStrs(args, "levels"))
	{
		const QString lowered = level.toLower();
		filter.Levels.append(lowered == "out" || lowered == "dash" || lowered == "log" ? lowered : level.toUpper());
	}
	const QString match = argStr(args, "match");
	if (!match.isEmpty())
	{
		filter.Pattern = QRegularExpression(match, QRegularExpression::CaseInsensitiveOption);
		if (!filter.Pattern.isValid())
		{
			reply(QString("bad match expression: %1").arg(filter.Pattern.errorString()), true);
			return;
		}
	}
	const bool hasRange = filter.HasFromIdx || filter.HasToIdx || filter.HasFromTs || filter.HasToTs;
	quint64 maxLines = 200;
	argU64(args, "max_lines", maxLines);
	follow->MaxLines = (int)qBound<quint64>(1, maxLines, s_MaxLogLines);
	quint64 last = 0;
	bool hasLast = argU64(args, "last", last);
	if (!hasLast && !hasRange)
	{
		hasLast = true;
		last = s_DefaultLogLines;
	}
	double followSeconds = 0;
	argDouble(args, "follow_seconds", followSeconds);
	follow->FollowSeconds = qBound(0.0, followSeconds, s_MaxWaitSeconds);

	QString error;
	CDashClient *dash = findDash(args, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const QString processName = argStr(args, "process");
	if (!processName.isEmpty())
	{
		int index = -1;
		if (findTarget(args, index, error) == nullptr)
		{
			reply(error, true);
			return;
		}
		follow->ProcIndices.append(index);
	}
	else
	{
		for (int i = 0; i < dash->processes().size(); ++i)
			follow->ProcIndices.append(i);
	}
	follow->Several = follow->ProcIndices.size() > 1;
	for (int pi : follow->ProcIndices)
	{
		const CMirrorProcess &p = dash->processes()[pi];
		QVector<const CMirrorLine *> matching;
		for (const CMirrorLine &line : p.Lines)
			if (filter.accepts(line))
				matching.append(&line);
		const int total = matching.size();
		int from = 0;
		if (hasLast && (quint64)matching.size() > last)
			from = matching.size() - (int)last;
		const quint64 firstHeld = p.Lines.isEmpty() ? 0 : p.Lines.first().Idx;
		const quint64 lastHeld = p.Lines.isEmpty() ? 0 : p.Lines.last().Idx;
		follow->Header += QString("# %1: %2 of %3 matching lines, held %4..%5, next idx %6\n")
		                      .arg(follow->Several ? QString("[%1]").arg(p.Info.Name) : p.Info.Name)
		                      .arg(matching.size() - from)
		                      .arg(total)
		                      .arg(firstHeld)
		                      .arg(lastHeld)
		                      .arg(p.Info.NextIdx);
		for (int i = from; i < matching.size(); ++i)
		{
			CPicked picked;
			picked.Ts = matching[i]->Ts;
			picked.Idx = matching[i]->Idx;
			picked.ProcIdx = pi;
			picked.Text = formatLine(*matching[i], follow->Several ? p.Info.Name : QString(), follow->Mode);
			follow->Picked.append(picked);
		}
		follow->Cursors.append(qMax<quint64>(lastHeld, p.Info.NextIdx > 0 ? p.Info.NextIdx - 1 : 0));
	}
	// to_time in the past makes following pointless; a to_idx too
	follow->CanFollow = follow->FollowSeconds > 0 && !filter.HasToIdx && (!filter.HasToTs || filter.ToTs > nowMs());

	QPointer<CDashClient> guard(dash);
	const std::function<void(bool)> finish = [follow, reply](bool reached) {
		Q_UNUSED(reached);
		if (follow->Several)
			std::sort(follow->Picked.begin(), follow->Picked.end(), [](const CPicked &a, const CPicked &b) {
				if (a.Ts != b.Ts)
					return a.Ts < b.Ts;
				if (a.ProcIdx != b.ProcIdx)
					return a.ProcIdx < b.ProcIdx;
				return a.Idx < b.Idx;
			});
		QString header = follow->Header;
		if (follow->Picked.size() > follow->MaxLines)
		{
			// the selection itself was larger than the cap: keep the newest
			const int excess = follow->Picked.size() - follow->MaxLines;
			follow->Picked.remove(0, excess);
			header += QString("# %1 older lines of the selection left out by max_lines\n").arg(excess);
		}
		if (follow->CanFollow)
			header += QString("# followed for %1 s: %2 new line(s)\n").arg(follow->FollowSeconds).arg(follow->Followed);
		header += QString("# now %1; columns: idx time type message\n").arg(formatDateTime(nowMs()));
		QStringList body;
		for (const CPicked &picked : follow->Picked)
			body.append(picked.Text);
		reply(header + body.join("\n"), false);
	};
	if (!follow->CanFollow || follow->Picked.size() >= follow->MaxLines)
	{
		finish(true);
		return;
	}
	addWaiter(
	    (int)(follow->FollowSeconds * 1000.0), 0,
	    [guard, follow]() {
		    if (guard.isNull())
			    return (int)CWaiter::Done;
		    bool any = false;
		    for (int i = 0; i < follow->ProcIndices.size(); ++i)
		    {
			    const int pi = follow->ProcIndices[i];
			    if (pi >= guard->processes().size())
				    continue;
			    const CMirrorProcess &p = guard->processes()[pi];
			    for (const CMirrorLine &line : p.Lines)
			    {
				    if (line.Idx <= follow->Cursors[i])
					    continue;
				    follow->Cursors[i] = line.Idx;
				    any = true;
				    if (follow->Filter.accepts(line) && follow->Picked.size() < follow->MaxLines)
				    {
					    CPicked picked;
					    picked.Ts = line.Ts;
					    picked.Idx = line.Idx;
					    picked.ProcIdx = pi;
					    picked.Text = formatLine(line, follow->Several ? p.Info.Name : QString(), follow->Mode);
					    follow->Picked.append(picked);
					    ++follow->Followed;
				    }
			    }
		    }
		    if (follow->Picked.size() >= follow->MaxLines)
			    return (int)CWaiter::Done;
		    return any ? (int)CWaiter::Progress : (int)CWaiter::Wait;
	    },
	    finish);
}

void CMcpServer::toolClearLog(const QJsonObject &args, const TReply &reply)
{
	QString error;
	int index = -1;
	CDashClient *dash = findTarget(args, index, error);
	if (dash == nullptr)
	{
		reply(error, true);
		return;
	}
	const QString name = dash->processes()[index].Info.Name;
	QJsonObject fields;
	fields["name"] = name;
	dash->send(Wire::message("ClearConsole", fields));
	reply(QString("Cleared the held lines of '%1'.").arg(name), false);
}
