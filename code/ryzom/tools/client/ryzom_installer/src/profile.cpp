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

void CProfile::loadFromSettings(const QSettings &settings)
{
	id = settings.value("id").toString();
	name = settings.value("name").toString();
	server = settings.value("server").toString();
	executable = settings.value("executable").toString();
	arguments = settings.value("arguments").toString();
	comments = settings.value("comments").toString();
	desktopShortcut = settings.value("desktop_shortcut").toBool();
	menuShortcut = settings.value("menu_shortcut").toBool();
}

void CProfile::saveToSettings(QSettings &settings) const
{
	settings.setValue("id", id);
	settings.setValue("name", name);
	settings.setValue("server", server);
	settings.setValue("executable", executable);
	settings.setValue("arguments", arguments);
	settings.setValue("comments", comments);
	settings.setValue("desktop_shortcut", desktopShortcut);
	settings.setValue("menu_shortcut", menuShortcut);
}

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

	QString exe = getClientFullPath();
	QString workingDir = s.getDirectory();

	QString profileArguments = QString("--profile %1").arg(id);

	// append custom arguments
	if (!arguments.isEmpty()) profileArguments += QString(" %1").arg(arguments);

	QString icon;

#ifdef Q_OS_WIN32
	// under Windows, icon is included in executable
	icon = exe;
#else
	// icon is in the same directory as client
	icon = s.getDirectory() + "/ryzom_client.png";
#endif

	if (desktopShortcut)
	{
		QString shortcut = getClientDesktopShortcutFullPath();

		// create desktop shortcut
		createLink(shortcut, name, exe, profileArguments, icon, workingDir);
	}

	if (menuShortcut)
	{
		QString shortcut = getClientMenuShortcutFullPath();

		// create menu shortcut
		createLink(shortcut, name, exe, profileArguments, icon, workingDir);
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

bool CProfile::createClientConfig() const
{
	// where to search and put client.cfg
	QString directory = getDirectory();
	QString filename = directory + "/client.cfg";

	const CServer &s = CConfigFile::getInstance()->getServer(server);

	// create the 2 initial lines of client.cfg
	QString rootConfigFilenameLine = QString("RootConfigFilename = \"%1\";").arg(s.getDefaultClientConfigFullPath());
	QString languageCodeline = QString("LanguageCode = \"%1\";\n").arg(CConfigFile::getInstance()->getLanguage());

	QString content;

	QFile file(filename);

	if (file.open(QFile::ReadOnly))
	{
		// read while content as UTF-8 text
		content = QString::fromUtf8(file.readAll());

		// search the end of the first line
		int pos = content.indexOf('\n');

		if (pos > 0)
		{
			// don't remove the \r under Windows
			if (content[pos - 1] == '\r') pos--;

			// update RootConfigFilename to be sure it points on right client_default.cfg
			content = content.mid(pos);
			content.prepend(rootConfigFilenameLine);
		}

		file.close();
	}
	else
	{
		// create initial client.cfg
		content += rootConfigFilenameLine + "\n";
		content += languageCodeline + "\n";
	}

	// write the new content of client.cfg
	if (!file.open(QFile::WriteOnly)) return false;

	file.write(content.toUtf8());
	file.close();

	return true;
}
