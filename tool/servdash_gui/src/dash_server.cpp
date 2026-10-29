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

#include "dash_server.h"

#include "registry.h"
#include "wire.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>

namespace {

/// A client that sends more than this without a newline is not speaking the protocol.
const int s_MaxLineSize = 1024 * 1024;

} // namespace

CDashServer::CDashServer(CDashboard *dashboard, const QString &iniPath, QObject *parent)
    : QObject(parent)
    , m_Dashboard(dashboard)
    , m_IniPath(iniPath)
{
	m_Server = new QLocalServer(this);
	m_Server->setSocketOptions(QLocalServer::UserAccessOption);
	connect(m_Server, &QLocalServer::newConnection, this, &CDashServer::onNewConnection);
	m_CommandsTimer = new QTimer(this);
	m_CommandsTimer->setSingleShot(true);
	m_CommandsTimer->setInterval(50);
	connect(m_CommandsTimer, &QTimer::timeout, this, &CDashServer::flushCommands);
	for (CProcessEntry *entry : m_Dashboard->entries())
	{
		connect(entry, &CProcessEntry::stateChanged, this, &CDashServer::onProcessChanged);
		connect(entry, &CProcessEntry::titleChanged, this, &CDashServer::onProcessChanged);
		connect(entry, &CProcessEntry::controlChanged, this, &CDashServer::onProcessChanged);
		connect(entry, &CProcessEntry::labelsReset, this, &CDashServer::onProcessChanged);
		connect(entry, &CProcessEntry::labelChanged, this, &CDashServer::onLabelChanged);
		connect(entry, &CProcessEntry::commandsChanged, this, &CDashServer::onCommandsChanged);
		connect(entry, &CProcessEntry::lineAdded, this, &CDashServer::onLineAdded);
		connect(entry, &CProcessEntry::linesReset, this, &CDashServer::onLinesReset);
		connect(entry, &CProcessEntry::execDone, this, &CDashServer::onExecDone);
	}
}

CDashServer::~CDashServer()
{
	if (!m_RecordId.isEmpty())
		Registry::remove(m_RecordId);
}

bool CDashServer::listen(QString &error)
{
	const QString name = QString("servdash_gui.%1.dash").arg(QCoreApplication::applicationPid());
	QLocalServer::removeServer(name);
	if (!m_Server->listen(name))
	{
		error = QString("Cannot listen for dashboard clients: %1").arg(m_Server->errorString());
		return false;
	}
	Registry::CRecord record;
	record.Id = Registry::idFor(m_IniPath);
	record.IniPath = QDir::toNativeSeparators(QFileInfo(m_IniPath).absoluteFilePath());
	record.Title = m_Dashboard->title();
	record.Endpoint = m_Server->fullServerName();
	record.Pid = QCoreApplication::applicationPid();
	record.Started = QDateTime::currentSecsSinceEpoch();
	record.Tool = Registry::toolName();
	if (!Registry::write(record, error))
		return false;
	m_RecordId = record.Id;
	return true;
}

QString CDashServer::endpoint() const
{
	return m_Server->fullServerName();
}

// *** clients ***

void CDashServer::onNewConnection()
{
	while (m_Server->hasPendingConnections())
	{
		QLocalSocket *client = m_Server->nextPendingConnection();
		if (client->state() != QLocalSocket::ConnectedState)
		{
			client->deleteLater();
			continue;
		}
		m_Clients.append(client);
		m_Buffers.insert(client, QByteArray());
		connect(client, &QLocalSocket::readyRead, this, &CDashServer::onReadyRead);
		connect(client, &QLocalSocket::disconnected, this, &CDashServer::onDisconnected);
	}
}

void CDashServer::onDisconnected()
{
	QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
	if (client == nullptr)
		return;
	m_Clients.removeAll(client);
	m_Buffers.remove(client);
	client->deleteLater();
}

void CDashServer::onReadyRead()
{
	QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
	if (client == nullptr || !m_Buffers.contains(client))
		return;
	QByteArray &buffer = m_Buffers[client];
	buffer += client->readAll();
	int pos;
	while ((pos = buffer.indexOf('\n')) >= 0)
	{
		const QByteArray raw = buffer.left(pos);
		buffer.remove(0, pos + 1);
		if (raw.trimmed().isEmpty())
			continue;
		const QJsonDocument doc = QJsonDocument::fromJson(raw);
		QJsonValue msg;
		if (doc.isObject())
			msg = doc.object();
		else if (doc.isArray())
			msg = doc.array();
		else
		{
			// a bare variant is a JSON string, which QJsonDocument does not parse on its own
			const QByteArray trimmed = raw.trimmed();
			if (trimmed.size() >= 2 && trimmed.startsWith('"') && trimmed.endsWith('"'))
				msg = QJsonDocument::fromJson("[" + trimmed + "]").array().first();
		}
		if (!msg.isUndefined() && !msg.isNull())
			handleClientMessage(client, msg);
	}
	if (buffer.size() > s_MaxLineSize)
		client->disconnectFromServer();
}

void CDashServer::send(QLocalSocket *client, const QJsonValue &msg)
{
	client->write(Wire::encode(msg));
}

void CDashServer::broadcast(const QJsonValue &msg)
{
	if (m_Clients.isEmpty())
		return;
	const QByteArray bytes = Wire::encode(msg);
	for (QLocalSocket *client : m_Clients)
		client->write(bytes);
}

void CDashServer::sendHello(QLocalSocket *client)
{
	QJsonObject hello;
	hello["version"] = Wire::Version;
	hello["title"] = m_Dashboard->title();
	send(client, Wire::message("Hello", hello));
	QJsonArray processes;
	for (CProcessEntry *entry : m_Dashboard->entries())
		processes.append(Wire::processInfo(entry));
	QJsonObject list;
	list["processes"] = processes;
	send(client, Wire::message("Processes", list));
	for (CProcessEntry *entry : m_Dashboard->entries())
	{
		QJsonArray lines;
		for (const CProcessEntry::CLine &line : entry->lines())
			lines.append(Wire::lineObject(line));
		QJsonObject snapshot;
		snapshot["name"] = entry->name();
		snapshot["lines"] = lines;
		send(client, Wire::message("Snapshot", snapshot));
		if (!entry->commands().isEmpty())
		{
			QJsonObject commands;
			commands["name"] = entry->name();
			commands["commands"] = Wire::commandsArray(entry->commands());
			send(client, Wire::message("Commands", commands));
		}
	}
}

CProcessEntry *CDashServer::findEntry(const QString &name) const
{
	for (CProcessEntry *entry : m_Dashboard->entries())
		if (entry->name() == name)
			return entry;
	return nullptr;
}

void CDashServer::handleClientMessage(QLocalSocket *client, const QJsonValue &msg)
{
	QString variant;
	QJsonObject fields;
	if (!Wire::decode(msg, variant, fields))
		return;
	const QString name = fields.value("name").toString();
	CProcessEntry *entry = findEntry(name);
	if (variant == "Hello")
	{
		sendHello(client);
	}
	else if (variant == "Start")
	{
		if (entry != nullptr)
			entry->startButton();
	}
	else if (variant == "Stop")
	{
		if (entry != nullptr)
			entry->stopButton();
	}
	else if (variant == "Kill")
	{
		if (entry != nullptr)
			entry->kill();
	}
	else if (variant == "StartAll")
	{
		m_Dashboard->launchAll();
	}
	else if (variant == "StopAll")
	{
		m_Dashboard->shutdownAll();
	}
	else if (variant == "Input")
	{
		if (entry != nullptr)
			entry->sendInput(fields.value("text").toString());
	}
	else if (variant == "Command")
	{
		if (entry != nullptr)
			entry->sendCommand(fields.value("text").toString());
	}
	else if (variant == "ClearConsole")
	{
		if (entry != nullptr)
			entry->clearConsole();
	}
	else if (variant == "Exec")
	{
		const QString id = fields.value("id").toString();
		QStringList args;
		for (const QJsonValue &value : fields.value("args").toArray())
			args.append(value.toString());
		if (entry != nullptr)
			entry->execCommand(id, fields.value("command").toString(), args);
		else
			onExecDone(nullptr, id, false);
	}
	else if (variant == "Shutdown")
	{
		QJsonObject notice;
		notice["text"] = "Stopping all services, the dashboard exits when they are gone";
		broadcast(Wire::message("Notice", notice));
		// with nothing running the window closes at once, before anything else could say goodbye
		if (!m_Dashboard->anyRunning())
			sendBye();
		emit shutdownRequested();
	}
}

void CDashServer::sendBye()
{
	broadcast(QJsonValue("Bye"));
	for (QLocalSocket *client : m_Clients)
	{
		client->flush();
		client->waitForBytesWritten(200);
	}
}

// *** the dashboard ***

void CDashServer::onProcessChanged(CProcessEntry *entry)
{
	QJsonObject fields;
	fields["info"] = Wire::processInfo(entry);
	broadcast(Wire::message("Process", fields));
}

void CDashServer::onLabelChanged(CProcessEntry *entry, int index, const QString &value)
{
	QJsonObject fields;
	fields["name"] = entry->name();
	fields["index"] = index;
	fields["value"] = value;
	broadcast(Wire::message("Label", fields));
}

void CDashServer::onCommandsChanged(CProcessEntry *entry)
{
	m_DirtyCommands.insert(entry);
	if (!m_CommandsTimer->isActive())
		m_CommandsTimer->start();
}

void CDashServer::flushCommands()
{
	for (CProcessEntry *entry : m_DirtyCommands)
	{
		QJsonObject fields;
		fields["name"] = entry->name();
		fields["commands"] = Wire::commandsArray(entry->commands());
		broadcast(Wire::message("Commands", fields));
	}
	m_DirtyCommands.clear();
}

void CDashServer::onLineAdded(CProcessEntry *entry, const CProcessEntry::CLine &line)
{
	if (m_Clients.isEmpty())
		return;
	QJsonArray lines;
	lines.append(Wire::lineObject(line));
	QJsonObject fields;
	fields["name"] = entry->name();
	fields["lines"] = lines;
	broadcast(Wire::message("Lines", fields));
}

void CDashServer::onLinesReset(CProcessEntry *entry)
{
	if (m_Clients.isEmpty())
		return;
	QJsonArray lines;
	for (const CProcessEntry::CLine &line : entry->lines())
		lines.append(Wire::lineObject(line));
	QJsonObject fields;
	fields["name"] = entry->name();
	fields["lines"] = lines;
	broadcast(Wire::message("Snapshot", fields));
}

void CDashServer::onExecDone(CProcessEntry *entry, const QString &id, bool ok)
{
	QJsonObject fields;
	fields["name"] = entry != nullptr ? entry->name() : QString();
	fields["id"] = id;
	fields["ok"] = ok;
	broadcast(Wire::message("ExecDone", fields));
}
