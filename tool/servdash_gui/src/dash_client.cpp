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

#include "dash_client.h"

#include "wire.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>

namespace {

/// Lines kept per process in the mirror; a dashboard holds as many.
const int s_MaxMirrorLines = 10000;
/// Run starts and ends kept per dashboard for the waits that watch them.
const int s_MaxEvents = 256;
/// How long a dashboard older than the Synced message may take to send what follows its last Snapshot.
const int s_SettleWithoutMarkerMs = 250;

CMirrorLine lineFromJson(const QJsonObject &o)
{
	CMirrorLine line;
	line.Idx = (quint64)o.value("idx").toDouble();
	line.Ts = (qint64)o.value("ts").toDouble();
	line.Raw = o.value("raw").toBool();
	for (const QJsonValue &span : o.value("spans").toArray())
		line.Text += span.toObject().value("text").toString();
	return line;
}

CMirrorCommand commandFromJson(const QJsonObject &o)
{
	CMirrorCommand command;
	command.Name = o.value("name").toString();
	command.Kind = o.value("kind").toString();
	command.Args = o.value("args").toString();
	command.Help = o.value("help").toString();
	return command;
}

} // namespace

CProcInfo CProcInfo::fromJson(const QJsonObject &o)
{
	CProcInfo info;
	info.Name = o.value("name").toString();
	info.Label = o.value("label").toString();
	info.Title = o.value("title").toString();
	info.State = o.value("state").toString();
	info.Started = o.value("started").toBool();
	info.Control = o.value("control").toBool();
	info.ControlConnected = o.value("control_connected").toBool();
	for (const QJsonValue &label : o.value("labels").toArray())
		info.Labels.append(label.toString());
	info.HasPid = o.value("pid").isDouble();
	info.Pid = (qint64)o.value("pid").toDouble();
	info.HasStartedAt = o.value("started_at").isDouble();
	info.StartedAt = (qint64)o.value("started_at").toDouble();
	info.HasExitCode = o.value("exit_code").isDouble();
	info.ExitCode = o.value("exit_code").toInt();
	info.RelaunchPending = o.value("relaunch_pending").toBool();
	info.CrashChain = o.value("crash_chain").toInt();
	info.HasLaunchCtrl = o.value("has_launch_ctrl").toBool();
	info.CtrlValue = o.value("ctrl_value").toString();
	info.HasPriority = o.value("priority").isDouble();
	info.Priority = o.value("priority").toInt();
	info.AutoStart = o.value("auto_start").toBool();
	info.Command = o.value("command").toString();
	info.WorkingDirectory = o.value("working_directory").toString();
	info.NextIdx = (quint64)o.value("next_idx").toDouble();
	info.FirstIdx = (quint64)o.value("first_idx").toDouble();
	info.ChannelVersion = o.value("channel_version").toInt();
	info.StartCount = o.value("start_count").toInt();
	info.HasReadyMs = o.value("ready_ms").isDouble();
	info.ReadyMs = (qint64)o.value("ready_ms").toDouble();
	info.LastExit = o.value("last_exit").toString();
	return info;
}

QString CProcInfo::exitText() const
{
	if (!LastExit.isEmpty())
		return LastExit;
	return HasExitCode ? QString("code %1").arg(ExitCode) : QString();
}

void CMirrorProcess::trackIndices()
{
	if (!Lines.isEmpty())
		Info.NextIdx = qMax(Info.NextIdx, Lines.last().Idx + 1);
	Info.FirstIdx = Lines.isEmpty() ? 0 : Lines.first().Idx;
}

CDashClient::CDashClient(const Registry::CRecord &record, QObject *parent)
    : QObject(parent)
    , m_Record(record)
    , m_Title(record.Title)
{
	m_Socket = new QLocalSocket(this);
	connect(m_Socket, &QLocalSocket::connected, this, &CDashClient::onConnected);
	connect(m_Socket, &QLocalSocket::readyRead, this, &CDashClient::onReadyRead);
	connect(m_Socket, &QLocalSocket::disconnected, this, &CDashClient::onDisconnected);
	connect(m_Socket, &QLocalSocket::errorOccurred, this, &CDashClient::onError);
	m_SettleTimer = new QTimer(this);
	m_SettleTimer->setSingleShot(true);
	m_SettleTimer->setInterval(s_SettleWithoutMarkerMs);
	connect(m_SettleTimer, &QTimer::timeout, this, &CDashClient::onSettled);
}

void CDashClient::open()
{
	m_OpenedAt.start();
	m_Socket->connectToServer(m_Record.Endpoint);
}

void CDashClient::pushEvent(const QString &name, bool began, const QString &how, qint64 upMs)
{
	CMirrorEvent event;
	event.Seq = ++m_EventSeq;
	event.Name = name;
	event.Began = began;
	event.How = how;
	event.UpMs = upMs;
	m_Events.append(event);
	if (m_Events.size() > s_MaxEvents)
		m_Events.remove(0, m_Events.size() - s_MaxEvents);
}

void CDashClient::checkSyncedWithoutMarker()
{
	// A dashboard that sends no Synced message is at the end of its state once every process got
	// its Snapshot, but for the command table of the last process, which follows its Snapshot
	// when it has one. So the state counts as whole with the next message, whatever it is, or
	// when none comes for a moment.
	if (!m_Synced && !m_Settling && m_Version < 3 && m_GotProcesses && m_Awaiting.isEmpty())
	{
		m_Settling = true;
		m_SettleTimer->start();
	}
}

void CDashClient::onSettled()
{
	if (!m_Settling)
		return;
	m_Settling = false;
	m_SettleTimer->stop();
	m_Synced = true;
	emit changed();
}

int CDashClient::indexOf(const QString &name) const
{
	for (int i = 0; i < m_Procs.size(); ++i)
		if (m_Procs[i].Info.Name == name)
			return i;
	return -1;
}

void CDashClient::send(const QJsonValue &msg)
{
	if (m_Connected)
		m_Socket->write(Wire::encode(msg));
}

bool CDashClient::takeExecResult(const QString &id, bool &ok)
{
	QHash<QString, bool>::iterator it = m_ExecResults.find(id);
	if (it == m_ExecResults.end())
		return false;
	ok = it.value();
	m_ExecResults.erase(it);
	return true;
}

void CDashClient::onConnected()
{
	m_Connected = true;
	m_EverConnected = true;
	QJsonObject hello;
	hello["version"] = Wire::Version;
	send(Wire::message("Hello", hello));
	emit connected();
}

void CDashClient::onDisconnected()
{
	if (m_Gone)
		return;
	m_Gone = true;
	m_Connected = false;
	emit gone();
}

void CDashClient::onError(QLocalSocket::LocalSocketError error)
{
	Q_UNUSED(error);
	if (m_Gone)
		return;
	// a dashboard that stopped answering, or a record of one that is gone
	m_Gone = true;
	m_Connected = false;
	emit gone();
}

void CDashClient::onReadyRead()
{
	m_Buffer += m_Socket->readAll();
	int pos;
	bool any = false;
	while ((pos = m_Buffer.indexOf('\n')) >= 0)
	{
		const QByteArray raw = m_Buffer.left(pos);
		m_Buffer.remove(0, pos + 1);
		QJsonValue msg;
		const QJsonDocument doc = QJsonDocument::fromJson(raw);
		if (doc.isObject())
			msg = doc.object();
		else
		{
			const QByteArray trimmed = raw.trimmed();
			if (trimmed.size() >= 2 && trimmed.startsWith('"') && trimmed.endsWith('"'))
				msg = QJsonDocument::fromJson("[" + trimmed + "]").array().first();
		}
		QString variant;
		QJsonObject fields;
		if (!Wire::decode(msg, variant, fields))
			continue;
		const bool wasSettling = m_Settling;
		apply(variant, fields);
		if (wasSettling)
		{
			// what followed the last Snapshot of a dashboard without the Synced message
			m_Settling = false;
			m_SettleTimer->stop();
			m_Synced = true;
		}
		any = true;
		if (variant == "Bye")
		{
			m_Gone = true;
			m_Connected = false;
			emit gone();
			return;
		}
	}
	if (any)
		emit changed();
}

void CDashClient::apply(const QString &variant, const QJsonObject &fields)
{
	const QString name = fields.value("name").toString();
	const int idx = indexOf(name);
	if (variant == "Hello")
	{
		m_Version = fields.value("version").toInt();
		m_Title = fields.value("title").toString();
	}
	else if (variant == "Processes")
	{
		QVector<CMirrorProcess> old = m_Procs;
		m_Procs.clear();
		for (const QJsonValue &value : fields.value("processes").toArray())
		{
			CMirrorProcess process;
			process.Info = CProcInfo::fromJson(value.toObject());
			for (const CMirrorProcess &prev : old)
			{
				if (prev.Info.Name == process.Info.Name)
				{
					process.Lines = prev.Lines;
					process.Commands = prev.Commands;
					break;
				}
			}
			m_Procs.append(process);
		}
		m_GotProcesses = true;
		m_Awaiting.clear();
		for (const CMirrorProcess &process : m_Procs)
			m_Awaiting.insert(process.Info.Name);
		checkSyncedWithoutMarker();
	}
	else if (variant == "Synced")
	{
		m_Synced = true;
	}
	else if (variant == "Process")
	{
		CProcInfo info = CProcInfo::fromJson(fields.value("info").toObject());
		int at = indexOf(info.Name);
		if (at < 0)
		{
			CMirrorProcess process;
			process.Info = info;
			m_Procs.append(process);
		}
		else
		{
			// a run is known by its start time, which a dashboard reports while the process runs;
			// a change of it ends one run and begins another
			const CProcInfo old = m_Procs[at].Info;
			const bool changed = old.HasStartedAt != info.HasStartedAt || old.StartedAt != info.StartedAt;
			m_Procs[at].Info = info;
			m_Procs[at].trackIndices();
			if (changed && old.HasStartedAt)
				pushEvent(info.Name, false, info.exitText(), qMax<qint64>(0, QDateTime::currentMSecsSinceEpoch() - old.StartedAt));
			if (changed && info.HasStartedAt)
				pushEvent(info.Name, true, QString(), -1);
		}
	}
	else if (variant == "Label")
	{
		if (idx < 0)
			return;
		const int index = fields.value("index").toInt(-1);
		if (index < 0 || index >= 256)
			return;
		QStringList &labels = m_Procs[idx].Info.Labels;
		while (labels.size() <= index)
			labels.append(QString());
		labels[index] = fields.value("value").toString();
	}
	else if (variant == "Snapshot")
	{
		m_Awaiting.remove(name);
		checkSyncedWithoutMarker();
		if (idx < 0)
			return;
		m_Procs[idx].Lines.clear();
		for (const QJsonValue &value : fields.value("lines").toArray())
			m_Procs[idx].Lines.append(lineFromJson(value.toObject()));
		m_Procs[idx].trackIndices();
	}
	else if (variant == "Lines")
	{
		if (idx < 0)
			return;
		QVector<CMirrorLine> &lines = m_Procs[idx].Lines;
		for (const QJsonValue &value : fields.value("lines").toArray())
			lines.append(lineFromJson(value.toObject()));
		if (lines.size() > s_MaxMirrorLines)
			lines.remove(0, lines.size() - s_MaxMirrorLines);
		m_Procs[idx].trackIndices();
	}
	else if (variant == "Clear")
	{
		if (idx >= 0)
			m_Procs[idx].Lines.clear();
	}
	else if (variant == "Commands")
	{
		if (idx < 0)
			return;
		m_Procs[idx].Commands.clear();
		for (const QJsonValue &value : fields.value("commands").toArray())
		{
			CMirrorCommand command = commandFromJson(value.toObject());
			m_Procs[idx].Commands.insert(command.Name, command);
		}
	}
	else if (variant == "Command")
	{
		if (idx < 0)
			return;
		CMirrorCommand command = commandFromJson(fields.value("command").toObject());
		m_Procs[idx].Commands.insert(command.Name, command);
	}
	else if (variant == "Uncommand")
	{
		if (idx >= 0)
			m_Procs[idx].Commands.remove(fields.value("command").toString());
	}
	else if (variant == "ExecDone")
	{
		m_ExecResults.insert(fields.value("id").toString(), fields.value("ok").toBool());
	}
	// Notice and Bye carry nothing to mirror
}
