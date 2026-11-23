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

#ifndef SERVDASH_DASH_SERVER_H
#define SERVDASH_DASH_SERVER_H

#include "dashboard.h"

#include <QByteArray>
#include <QHash>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>

class QLocalServer;
class QLocalSocket;
class QTimer;

/// The dashboard as servdash_cli clients see it: the same JSON line protocol a servdash_cli
/// daemon serves, on a local endpoint registered in the shared session registry. The terminal
/// client attaches to it, `servdash_cli --list` names it, and the MCP servers of both tools
/// mirror it. Every state change, label, line and command table change goes to every client;
/// the clients' orders go to the process entries as the buttons would.
class CDashServer : public QObject
{
	Q_OBJECT

public:
	CDashServer(CDashboard *dashboard, const QString &iniPath, QObject *parent = nullptr);
	~CDashServer() override;

	/// Opens the endpoint and writes the registry record.
	bool listen(QString &error);
	QString endpoint() const;

public slots:
	/// Tells every client the dashboard is about to exit, and gives the message time to leave.
	void sendBye();

signals:
	/// A client asked for the shutdown: stop everything and exit, as closing the window does.
	void shutdownRequested();

private slots:
	void onNewConnection();
	void onReadyRead();
	void onDisconnected();
	void onProcessChanged(CProcessEntry *entry);
	void onLabelChanged(CProcessEntry *entry, int index, const QString &value);
	void onCommandsChanged(CProcessEntry *entry);
	void onLineAdded(CProcessEntry *entry, const CProcessEntry::CLine &line);
	void onLinesReset(CProcessEntry *entry);
	void onExecDone(CProcessEntry *entry, const QString &id, bool ok);
	void flushCommands();

private:
	void send(QLocalSocket *client, const QJsonValue &msg);
	void broadcast(const QJsonValue &msg);
	void sendHello(QLocalSocket *client);
	void handleClientMessage(QLocalSocket *client, const QJsonValue &msg);
	CProcessEntry *findEntry(const QString &name) const;

	CDashboard *m_Dashboard;
	QString m_IniPath;
	QString m_RecordId;
	QLocalServer *m_Server = nullptr;
	QList<QLocalSocket *> m_Clients;
	QHash<QLocalSocket *, QByteArray> m_Buffers;
	/// Command tables that changed since the last flush; a service registers its commands one at
	/// a time, so the whole table goes out once per burst.
	QSet<CProcessEntry *> m_DirtyCommands;
	QTimer *m_CommandsTimer = nullptr;
};

#endif // SERVDASH_DASH_SERVER_H
