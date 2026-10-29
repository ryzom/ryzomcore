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

#ifndef SERVDASH_DASH_CLIENT_H
#define SERVDASH_DASH_CLIENT_H

#include "registry.h"

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

/// One line of a mirrored process log.
struct CMirrorLine
{
	quint64 Idx = 0;
	/// Milliseconds since the Unix epoch.
	qint64 Ts = 0;
	/// From stdout or stderr, or a note of the dashboard, rather than from the console channel.
	bool Raw = false;
	QString Text;
};

struct CMirrorCommand
{
	QString Name;
	/// Command, Handler or Member.
	QString Kind;
	QString Args;
	QString Help;
};

/// The ProcessInfo of the protocol.
struct CProcInfo
{
	QString Name;
	QString Label;
	QString Title;
	/// New, Launched, Captured, Ready, Stopping or Stopped.
	QString State;
	bool Started = false;
	bool Control = false;
	bool ControlConnected = false;
	QStringList Labels;
	bool HasPid = false;
	qint64 Pid = 0;
	bool HasStartedAt = false;
	qint64 StartedAt = 0;
	bool HasExitCode = false;
	int ExitCode = 0;
	bool RelaunchPending = false;
	int CrashChain = 0;
	bool HasLaunchCtrl = false;
	QString CtrlValue;
	bool HasPriority = false;
	int Priority = 0;
	bool AutoStart = false;
	QString Command;
	QString WorkingDirectory;
	quint64 NextIdx = 0;
	quint64 FirstIdx = 0;
	int ChannelVersion = 0;

	static CProcInfo fromJson(const QJsonObject &o);
};

struct CMirrorProcess
{
	CProcInfo Info;
	QVector<CMirrorLine> Lines;
	QMap<QString, CMirrorCommand> Commands;

	/// The dashboard reports next_idx with the process state only; the lines move it on here.
	void trackIndices();
};

/// A client of one dashboard, servdash_gui or servdash_cli: it attaches to the endpoint of a
/// registry record and mirrors the processes, their logs and their command tables, as the
/// terminal client of servdash_cli does. The MCP server serves from these mirrors.
class CDashClient : public QObject
{
	Q_OBJECT

public:
	explicit CDashClient(const Registry::CRecord &record, QObject *parent = nullptr);

	void open();
	bool isConnected() const { return m_Connected; }
	/// Whether the endpoint ever answered; a record of a dashboard that is gone never does.
	bool everConnected() const { return m_EverConnected; }
	const Registry::CRecord &record() const { return m_Record; }
	const QString &title() const { return m_Title; }
	int daemonVersion() const { return m_Version; }
	const QVector<CMirrorProcess> &processes() const { return m_Procs; }
	int indexOf(const QString &name) const;

	void send(const QJsonValue &msg);
	/// The answer of an Exec once it arrived; true when taken.
	bool takeExecResult(const QString &id, bool &ok);

signals:
	void connected();
	/// Anything of the mirror changed.
	void changed();
	/// The connection ended: the dashboard exited or never answered.
	void gone();

private slots:
	void onConnected();
	void onReadyRead();
	void onDisconnected();
	void onError(QLocalSocket::LocalSocketError error);

private:
	void apply(const QString &variant, const QJsonObject &fields);

	Registry::CRecord m_Record;
	QString m_Title;
	int m_Version = 0;
	QVector<CMirrorProcess> m_Procs;
	QLocalSocket *m_Socket = nullptr;
	QByteArray m_Buffer;
	QHash<QString, bool> m_ExecResults;
	bool m_Connected = false;
	bool m_EverConnected = false;
	bool m_Gone = false;
};

#endif // SERVDASH_DASH_CLIENT_H
