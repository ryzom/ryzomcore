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
#include <QElapsedTimer>
#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

class QTimer;

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
	/// How often the dashboard started the process since it opened.
	int StartCount = 0;
	/// Milliseconds from the start of the run to its ready state.
	bool HasReadyMs = false;
	qint64 ReadyMs = 0;
	/// How the last run that ended went out, in the words of the dashboard; empty when it did not say.
	QString LastExit;

	static CProcInfo fromJson(const QJsonObject &o);
	/// How the last run ended: what the dashboard says, or the exit code alone from one older
	/// than that field; empty when no run ended.
	QString exitText() const;
};

/// A run of a process began or ended, as the mirror saw it in the process states. The waits
/// watch these, so an exit between two looks at the state is not missed.
struct CMirrorEvent
{
	quint64 Seq = 0;
	QString Name;
	/// A run began; otherwise one ended.
	bool Began = false;
	/// How the run ended; empty when the dashboard did not say.
	QString How;
	/// How long the run that ended had been up, in milliseconds; -1 when unknown.
	qint64 UpMs = -1;
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
	/// The state the dashboard sends to a new client has arrived in full. Until then the mirror
	/// is a part of the dashboard, and no tool answers from it.
	bool isSynced() const { return m_Synced; }
	/// Milliseconds since the client was opened.
	qint64 sinceOpenedMs() const { return m_OpenedAt.isValid() ? m_OpenedAt.elapsed() : 0; }
	/// The runs that began and ended since the client attached, the last ones of them.
	const QVector<CMirrorEvent> &events() const { return m_Events; }
	/// The sequence number of the newest event so far, 0 before the first.
	quint64 eventSeq() const { return m_EventSeq; }
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
	void onSettled();

private:
	void apply(const QString &variant, const QJsonObject &fields);
	void pushEvent(const QString &name, bool began, const QString &how, qint64 upMs);
	void checkSyncedWithoutMarker();

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
	bool m_Synced = false;
	QElapsedTimer m_OpenedAt;
	/// The processes whose Snapshot has not come since the Processes message; a dashboard older
	/// than the Synced message counts as synced when this runs empty.
	QSet<QString> m_Awaiting;
	bool m_GotProcesses = false;
	/// Such a dashboard sent its last Snapshot; what follows it, or a moment without anything, ends the state.
	bool m_Settling = false;
	QTimer *m_SettleTimer = nullptr;
	QVector<CMirrorEvent> m_Events;
	quint64 m_EventSeq = 0;
};

#endif // SERVDASH_DASH_CLIENT_H
