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

#ifndef SERVDASH_PROCESS_ENTRY_H
#define SERVDASH_PROCESS_ENTRY_H

#include "dashboard_config.h"
#include "launch_ctrl.h"

#include <QColor>
#include <QElapsedTimer>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>

#include <memory>

class QLocalServer;
class QLocalSocket;

/// One process of the dashboard: its configuration, state, child process, output, launch
/// control files and, for a NeL service, the console channel the service connects to with
/// `--ctrl`. The state rules are those of the Win32 servdash (ProcessHost, TerminalHost and
/// ProcessControl), so a shard behaves the same under every dashboard.
class CProcessEntry : public QObject
{
	Q_OBJECT

public:
	enum TState
	{
		New,
		/// Process started.
		Launched,
		/// Console attached: control channel connected, or the pipes of a plain process.
		Captured,
		/// Ready pattern matched, or none given.
		Ready,
		Stopping,
		Stopped
	};

	/// What an entry of the service's command table is.
	enum TCommandKind
	{
		/// A command, or a variable (its name starts with an upper case letter).
		CommandKind,
		/// A named command handler instance; its commands follow as `instance.command`.
		HandlerKind,
		/// A command of a handler instance.
		MemberKind
	};

	/// A command of the service, for completion; `instance.command` for a member command.
	struct CCommand
	{
		QString Name;
		TCommandKind Kind = CommandKind;
		/// The argument help, such as `<serviceName> [sid=<serviceId>]`.
		QString Args;
		QString Help;
	};

	struct CLine
	{
		QString Text;
		/// Invalid for the default text colour.
		QColor Color;
		/// From stdout or stderr, or a note of the dashboard, rather than from the console channel.
		bool Raw = false;
		/// Local time of arrival, `hh:mm:ss`.
		QString Time;
	};

	CProcessEntry(const CSectionConfig &config, QObject *parent = nullptr);
	~CProcessEntry() override;

	const CSectionConfig &config() const { return m_Config; }
	const QString &name() const { return m_Config.Name; }
	TState state() const { return m_State; }
	/// Whether the start button means stop. Set on launch, cleared on exit.
	bool started() const { return m_Started || m_RelaunchPending; }
	bool running() const { return m_Process != nullptr; }
	/// Button text: the Title key, else the process title once known, else the command.
	QString label() const { return m_Config.Title.isEmpty() ? m_Title : m_Config.Title; }
	/// The title the process reports over the console channel, or the label.
	const QString &title() const { return m_Title; }
	bool controlConnected() const { return m_Control != nullptr; }
	/// Console labels as the service created them: empty is a row separator, `@Name|cmd` a button.
	const QStringList &labels() const { return m_Labels; }
	/// The pane: console lines, the raw output and the dashboard's notes, in arrival order.
	const QVector<CLine> &lines() const { return m_Lines; }
	/// The command table of the service, by name, as it reports it over the console channel.
	const QMap<QString, CCommand> &commands() const { return m_Commands; }
	/// Whether the startup rules put the process in the start sequence.
	bool autoLaunch() const { return m_AutoLaunch; }
	bool hasLaunchCtrl() const { return m_LaunchCtrl != nullptr; }

	/// ProcessHost.Launch: start the process now; while stopping, relaunch after the exit.
	void launch();
	/// ProcessHost.Shutdown: ask the process to stop; a second request kills it.
	void shutdown();
	void kill();
	/// ProcessControl.startButton_Click and the stop half of it: through the launch control
	/// file when there is one, so the admin executor service stays in step.
	void startButton();
	void stopButton();
	/// A line for the process stdin.
	void sendInput(const QString &text);
	/// A NeL command over the console channel, or on stdin while it is not connected.
	void sendCommand(const QString &text);
	void clearConsole();
	/// Called four times a second: launch control polling and the capture timeout.
	void tick();
	/// While the window closes nothing may start, not even a relaunch after a stop.
	void setLaunchBlocked(bool blocked);

signals:
	void stateChanged(CProcessEntry *entry);
	void titleChanged(CProcessEntry *entry);
	void controlChanged(CProcessEntry *entry);
	void labelChanged(CProcessEntry *entry, int index, const QString &value);
	void labelsReset(CProcessEntry *entry);
	/// The command table changed: an entry came or went, or the table was dropped before a replay.
	void commandsChanged(CProcessEntry *entry);
	void lineAdded(CProcessEntry *entry, const CProcessEntry::CLine &line);
	/// The lines changed as a whole: cleared, or the console part dropped before a replay.
	void linesReset(CProcessEntry *entry);
	/// The process wants a place in the start sequence (ProcessControl.LaunchWanted).
	void launchWanted(CProcessEntry *entry);
	/// The process has to stop (MainWindow.stopClicked).
	void stopWanted(CProcessEntry *entry);
	/// The process exited (after the state change).
	void exited(CProcessEntry *entry);

private slots:
	void onReadyReadOutput();
	void onReadyReadError();
	void onFinished(int exitCode, QProcess::ExitStatus status);
	void onNewControlConnection();
	void onControlReadyRead();
	void onControlDisconnected();

private:
	void setState(TState state);
	void note(const QString &text, const QColor &color = QColor());
	void appendLine(const CLine &line);
	void readLines(QByteArray &buffer, const QByteArray &data, bool checkReady = true);
	void flushLine(QByteArray &buffer, bool checkReady);
	void handleOutputLine(const QString &text, bool checkReady);
	void handleControlLine(const QString &line);
	void applyCtrlValue(int value);
	void killChild();
	void releaseProcess();

	CSectionConfig m_Config;
	TState m_State = New;
	QString m_Title;
	bool m_Started = false;
	bool m_ShutdownRequested = false;
	bool m_KillRequested = false;
	bool m_LaunchBlocked = false;
	bool m_RestartOnce = false;
	/// Counts the RestartDelay relaunches scheduled; a stop or kill makes the pending one stale.
	int m_RelaunchGen = 0;
	/// A RestartDelay relaunch is scheduled; it shows as started, so the button cancels it.
	bool m_RelaunchPending = false;
	/// Exits in a row that came within the first minute of a run; doubles the RestartDelay each time.
	int m_CrashChain = 0;
	bool m_ReadyTriggered = false;
	bool m_AutoLaunch = true;
	QRegularExpression m_ReadyPattern;
	QProcess *m_Process = nullptr;
	QByteArray m_OutputBuffer;
	QByteArray m_ErrorBuffer;
	QVector<CLine> m_Lines;
	QStringList m_Labels;
	QMap<QString, CCommand> m_Commands;
	QLocalServer *m_Server = nullptr;
	QLocalSocket *m_Control = nullptr;
	QByteArray m_ControlBuffer;
	std::unique_ptr<CLaunchCtrl> m_LaunchCtrl;
	QElapsedTimer m_LaunchedAt;
};

#endif // SERVDASH_PROCESS_ENTRY_H
