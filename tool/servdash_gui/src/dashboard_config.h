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

#ifndef SERVDASH_DASHBOARD_CONFIG_H
#define SERVDASH_DASHBOARD_CONFIG_H

#include <QString>
#include <QStringList>
#include <QVector>

#include <climits>

/// The servdash ini file: `;` comments, `[section]` per process, `Key=Value` split at the first
/// `=`, no trimming, and the unnamed first section holds the dashboard Title. Same format and
/// keys as the Win32 servdash and servdash_cli, so the files the patch terminal generates work.
struct CLaunchCtrlConfig
{
	QString CtrlFile;
	QString StateFile;
	/// LAUNCH, RUNNING, STOP, STOPPED in the generated files.
	QString Values[4];
};

struct CSectionConfig
{
	QString Name;
	/// `Title`: fixed button text; without it the button follows the process title.
	QString Title;
	QString WorkingDirectory;
	QString LaunchCmd;
	QString LaunchArgs;
	QString ShutdownCmd;
	QString ShutdownArgs;
	/// `ShutdownInput`: a line written to the process stdin to stop it.
	QString ShutdownInput;
	/// `ReadyPattern`: regex on the process title (control sections) or on the output lines.
	QString ReadyPattern;
	/// `Priority`: start order group, INT_MAX when absent (one at a time).
	int Priority = INT_MAX;
	bool AutoStart = true;
	/// `Control=1`: a NeL service, shown through its console channel.
	bool Control = false;
	bool HasLaunchCtrl = false;
	CLaunchCtrlConfig LaunchCtrl;

	/// The button text before a process title arrives.
	QString label() const { return Title.isEmpty() ? LaunchCmd : Title; }
	/// The program to run: a relative command starting with `.` is taken from the working
	/// directory and made absolute, since the process runs in that directory too.
	QString program(const QString &command) const;
};

struct CDashboardConfig
{
	QString Title;
	QVector<CSectionConfig> Sections;

	/// Reads the file; a problem is reported in `error` and leaves the config empty.
	static bool read(const QString &path, CDashboardConfig &config, QString &error);
};

#endif // SERVDASH_DASHBOARD_CONFIG_H
