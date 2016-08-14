// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "stdpch.h"
#include "profile.h"
#include "configfile.h"
#include "utils.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

const CProfile NoProfile;

QString CProfile::getDirectory() const
{
	return CConfigFile::getInstance()->getProfileDirectory() + "/" + id;
}

QString CProfile::getClientFullPath() const
{
	if (!executable.isEmpty()) return executable;

	const CServer &s = CConfigFile::getInstance()->getServer(server);

	return s.getClientFullPath();
}

QString CProfile::getClientDesktopShortcutFullPath() const
{
#ifdef Q_OS_WIN32
	return CConfigFile::getInstance()->getDesktopDirectory() + "/" + name + ".lnk";
#elif defined(Q_OS_MAC)
	return "";
#else
	return CConfigFile::getInstance()->getDesktopDirectory() + "/" + name + ".desktop";
#endif
}

QString CProfile::getClientMenuShortcutFullPath() const
{
#ifdef Q_OS_WIN32
	return CConfigFile::getInstance()->getMenuDirectory() + "/" + name + ".lnk";
#elif defined(Q_OS_MAC)
	return "";
#else
	return CConfigFile::getInstance()->getMenuDirectory() + "/" + name + ".desktop";
#endif
}

void CProfile::createShortcuts() const
{
	const CServer &s = CConfigFile::getInstance()->getServer(server);

	QString executable = getClientFullPath();
	QString workingDir = s.getDirectory();

	QString arguments = QString("--profile %1").arg(id);

	// append custom arguments
	if (!arguments.isEmpty()) arguments += QString(" %1").arg(arguments);

	QString icon;

#ifdef Q_OS_WIN32
	// under Windows, icon is included in executable
	icon = executable;
#else
	// icon is in the same directory as client
	icon = s.getDirectory() + "/ryzom_client.png";
#endif

	if (desktopShortcut)
	{
		QString shortcut = getClientDesktopShortcutFullPath();

		// create desktop shortcut
		createLink(shortcut, name, executable, arguments, icon, workingDir);
	}

	if (menuShortcut)
	{
		QString shortcut = getClientMenuShortcutFullPath();

		// create menu shortcut
		createLink(shortcut, name, executable, arguments, icon, workingDir);
	}
}

void CProfile::deleteShortcuts() const
{
	// delete desktop shortcut
	QString link = getClientDesktopShortcutFullPath();

	if (QFile::exists(link)) QFile::remove(link);

	// delete menu shortcut
	link = getClientMenuShortcutFullPath();

	if (QFile::exists(link)) QFile::remove(link);
}

void CProfile::updateShortcuts() const
{
	deleteShortcuts();
	createShortcuts();
}
