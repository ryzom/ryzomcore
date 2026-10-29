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

#ifndef SERVDASH_LAUNCH_CTRL_H
#define SERVDASH_LAUNCH_CTRL_H

#include "dashboard_config.h"

#include <QDateTime>
#include <QString>

#include <functional>

/// The launch control files shared with the admin executor service (AES): it writes
/// `<alias>.launch_ctrl` with LAUNCH or STOP, `<alias>.deferred_launch_ctrl` with LAUNCH to
/// restart a service it told to quit, and polls `<alias>.state` for RUNNING or STOPPED plus
/// `<alias>.start_count` for its crash chain detection. Value indices as in the Win32 tool:
/// 0 LAUNCH, 1 RUNNING, 2 STOP, 3 STOPPED.
class CLaunchCtrl
{
public:
	enum
	{
		Launch = 0,
		Running = 1,
		Stop = 2,
		Stopped = 3
	};

	/// Applies the startup rules: a missing control file is created from AutoStart, an existing
	/// one decides by itself; the state starts as STOPPED and the counter at zero.
	CLaunchCtrl(const CLaunchCtrlConfig &config, bool autoStart, std::function<void(const QString &)> report);

	/// Whether the startup rules said to auto launch.
	bool autoLaunch() const { return m_AutoLaunch; }

	/// The value this side applied last, `LaunchCtrlValue` in the Win32 tool.
	int value() const { return m_Value; }

	/// The value index of a control file text; unknown text reads as the current state, given
	/// as a value index of that state (Running, Stop or Stopped) or Launch for a new process.
	int valueOf(const QString &text, int stateValue) const;

	bool ctrlIs(int index) const;
	void writeCtrl(int index);
	/// Sets the value this side applied and keeps the state file in step with it.
	void setValue(int index);
	void countLaunch();

	/// The new value index when the control file changed since the last poll, else -1. A rewrite
	/// with the same content counts, since the AES rewrites LAUNCH after a crash. `running`
	/// controls the promotion of a deferred order, which only applies to a stopped process.
	int poll(int stateValue, bool running);

	/// After the process exited: a LAUNCH order is consumed so a crash does not relaunch in a
	/// loop, and a deferred order becomes the next order, as the Linux launcher does.
	void afterExit();

private:
	struct CSeen
	{
		bool Exists = false;
		QString Text;
		QDateTime Modified;
		bool operator==(const CSeen &o) const { return Exists == o.Exists && Text == o.Text && Modified == o.Modified; }
		bool operator!=(const CSeen &o) const { return !(*this == o); }
	};

	CSeen observe() const;
	QString readCtrl(bool &exists) const;
	void writeState(int index);
	void writeStartCount();
	void promoteDeferred();
	void writeOrReport(const QString &path, const QString &text);

	CLaunchCtrlConfig m_Config;
	std::function<void(const QString &)> m_Report;
	QString m_DeferredFile;
	QString m_StartCountFile;
	int m_StartCount = 0;
	int m_Value = Stopped;
	bool m_AutoLaunch = false;
	CSeen m_LastSeen;
};

#endif // SERVDASH_LAUNCH_CTRL_H
