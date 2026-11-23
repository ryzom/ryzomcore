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

#ifndef SERVDASH_DASHBOARD_H
#define SERVDASH_DASHBOARD_H

#include "dashboard_config.h"
#include "process_entry.h"

#include <QList>
#include <QObject>

class QTimer;

/// The processes of one ini file and the start order between them: MainWindow.autoStart of
/// the Win32 servdash. Unprioritised processes start one at a time, waiting for the previous
/// one to be captured; prioritised ones start when nothing of a lower priority value is still
/// queued, starting or stopping.
class CDashboard : public QObject
{
	Q_OBJECT

public:
	CDashboard(const CDashboardConfig &config, QObject *parent = nullptr);

	const QString &title() const { return m_Title; }
	const QList<CProcessEntry *> &entries() const { return m_Entries; }
	bool anyRunning() const;
	bool closing() const { return m_Closing; }

	/// Puts the auto start processes in the queue and runs the start order.
	void start();
	void launchAll();
	void shutdownAll();
	/// MainWindow_FormClosing: no more launches, everything stops, `allStopped` follows.
	void beginClosing();
	/// A second close while stopping: kill what is left.
	void killAll();

signals:
	/// Closing was requested and nothing runs any more.
	void allStopped();

private slots:
	void onLaunchWanted(CProcessEntry *entry);
	void onStopWanted(CProcessEntry *entry);
	void onStateChanged(CProcessEntry *entry);
	void onExited(CProcessEntry *entry);
	void onTick();

private:
	void autoStart();
	void checkAllStopped();

	QString m_Title;
	QList<CProcessEntry *> m_Entries;
	/// Processes waiting for their turn; `LaunchProcesses` in the Win32 tool.
	QList<CProcessEntry *> m_Queue;
	QTimer *m_Timer = nullptr;
	bool m_Closing = false;
	bool m_AutoStarting = false;
	bool m_AutoStartAgain = false;
};

#endif // SERVDASH_DASHBOARD_H
