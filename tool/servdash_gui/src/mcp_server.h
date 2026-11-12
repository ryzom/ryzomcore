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

#ifndef SERVDASH_MCP_SERVER_H
#define SERVDASH_MCP_SERVER_H

#include "dash_client.h"

#include <QDeadlineTimer>
#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>

#include <functional>

class QTimer;

/// Reads stdin line by line on its own thread, since the standard input blocks.
class CStdinReader : public QThread
{
	Q_OBJECT

signals:
	void lineRead(const QByteArray &line);
	void closed();

protected:
	void run() override;
};

/// The MCP server: `servdash_gui --mcp [<ini>...]` speaks the Model Context Protocol over
/// stdin and stdout, for an agent that watches, drives and debugs the shards and the patchman
/// services of this machine. It attaches to every running dashboard of the user as it appears
/// (servdash_gui and servdash_cli alike), mirrors them, and forwards the orders; the
/// dashboards keep running when it exits. The same tools as `servdash_cli --mcp`.
///
/// A tool call that waits (a log follow, a restart) is a waiter that the mirrors' changes and
/// a timer re-check, so other calls go on meanwhile.
class CMcpServer : public QObject
{
	Q_OBJECT

public:
	explicit CMcpServer(QObject *parent = nullptr);

	/// Starts the dashboards of these ini files when they do not run, then serves stdin.
	void start(const QStringList &inis);

signals:
	/// Stdin closed: the client is gone and the server should exit.
	void finished();

private slots:
	void onLineRead(const QByteArray &line);
	void onStdinClosed();
	void onDiscoveryTick();
	void onClientChanged();
	void onClientGone();
	void onWaiterTick();

private:
	struct CWaiter;
	typedef std::function<void(const QString &text, bool isError)> TReply;

	void handle(const QJsonObject &msg);
	void writeMessage(const QJsonValue &msg);
	void reply(const QJsonValue &id, const QJsonValue &result);
	void replyError(const QJsonValue &id, int code, const QString &message);
	/// Runs the tool once the mirrors are whole.
	void callTool(const QString &name, const QJsonObject &args, const TReply &reply);
	void runTool(const QString &name, const QJsonObject &args, const TReply &reply);
	/// Whether the dashboard a call with these arguments names (every dashboard, when it names
	/// none) has sent its state, or has had its time to do so.
	bool mirrorsSettled(const QJsonObject &args) const;
	/// Whether the dashboards this server started from its command line are attached, or have
	/// had their time; looks for them in the registry while they are not.
	bool expectedAttached();
	QJsonValue toolList() const;

	/// A call that completes later: `check` runs on every change and on the tick, `finish` once.
	void addWaiter(int waitMs, int idleMs, const std::function<int()> &check, const std::function<void(bool)> &finish);
	void pollWaiters();

	// discovery
	void attach(const Registry::CRecord &record);
	bool startDashboardProcess(const QString &iniPath, QString &error);

	// resolution; null with an error text when the selector names nothing, or names a dashboard
	// that has not sent its state yet
	CDashClient *findDash(const QJsonObject &args, QString &error) const;
	/// The same for a dashboard in any condition, for what needs no mirror of it.
	CDashClient *findDashAny(const QJsonObject &args, QString &error) const;
	CDashClient *findTarget(const QJsonObject &args, int &index, QString &error) const;

	// tools
	void toolListDashboards(const TReply &reply);
	void toolStartDashboard(const QJsonObject &args, const TReply &reply);
	void toolShutdownDashboard(const QJsonObject &args, const TReply &reply);
	void toolListProcesses(const QJsonObject &args, const TReply &reply);
	void toolGetProcess(const QJsonObject &args, const TReply &reply);
	void toolProcessOrder(const QJsonObject &args, const QString &order, const TReply &reply);
	void toolRestartProcess(const QJsonObject &args, const TReply &reply);
	void toolAll(const QJsonObject &args, bool start, const TReply &reply);
	void toolWaitForState(const QJsonObject &args, const TReply &reply);
	void toolListCommands(const QJsonObject &args, const TReply &reply);
	void toolRunCommand(const QJsonObject &args, const TReply &reply);
	void toolSendInput(const QJsonObject &args, const TReply &reply);
	void toolGetLog(const QJsonObject &args, const TReply &reply);
	void toolClearLog(const QJsonObject &args, const TReply &reply);

	CStdinReader *m_Stdin = nullptr;
	QTimer *m_DiscoveryTimer = nullptr;
	QTimer *m_WaiterTimer = nullptr;
	/// The attached and connecting dashboards by record id.
	QMap<QString, CDashClient *> m_Dashes;
	/// Records whose endpoint did not answer, and when to try them again.
	QHash<QString, QDeadlineTimer> m_RetryAfter;
	/// The dashboards of the command line that are not attached yet, and until when to wait for them.
	QHash<QString, QDeadlineTimer> m_Expected;
	QList<CWaiter *> m_Waiters;
	/// The waiters are being polled; a poll asked for meanwhile runs as another round.
	bool m_Polling = false;
	bool m_PollAgain = false;
	quint64 m_NextExecId = 0;
};

#endif // SERVDASH_MCP_SERVER_H
