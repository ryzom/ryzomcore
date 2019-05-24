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
	language = settings.value("language").toString();
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
	settings.setValue("language", language);
	settings.setValue("desktop_shortcut", desktopShortcut);
	settings.setValue("menu_shortcut", menuShortcut);
}

bool CProfile::isValid(QString &error) const
{
	QRegExp idReg("^[0-9a-z_]+$");

	if (!idReg.exactMatch(id))
	{
		error = QApplication::tr("Profile ID %1 is using invalid characters (only lowercase letters, numbers and underscore are allowed)").arg(id);
		return false;
	}

	QRegExp nameReg("[/\\\\<>?*:.%|\"]");

	int pos = nameReg.indexIn(name);

	if (pos > -1)
	{
		error = QApplication::tr("Profile name %1 is using invalid character %2 at position %3").arg(name).arg(name[pos]).arg(pos);
		return false;
	}

	return true;
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
	return CConfigFile::getInstance()->getDesktopDirectory() + "/" + name;
}

QString CProfile::getClientMenuShortcutFullPath() const
{
	return CConfigFile::getInstance()->getMenuDirectory() + "/" + name;
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

		// make sure directory exists
		QDir().mkpath(CConfigFile::getInstance()->getDesktopDirectory());

		// create desktop shortcut
		if (!createShortcut(shortcut, name, exe, profileArguments, icon, workingDir))
		{
			nlwarning("Unable to create desktop shortcut");
		}
	}

	if (menuShortcut)
	{
		QString shortcut = getClientMenuShortcutFullPath();

		// make sure directory exists
		QDir().mkpath(CConfigFile::getInstance()->getMenuDirectory());

		// create menu shortcut
		if (!createShortcut(shortcut, name, exe, profileArguments, icon, workingDir))
		{
			nlwarning("Unable to create shortcut for client in menu");
		}
	}
}

void CProfile::deleteShortcuts() const
{
	// delete desktop shortcut
	removeShortcut(getClientDesktopShortcutFullPath());

	// delete menu shortcut
	removeShortcut(getClientMenuShortcutFullPath());
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

	// create directory
	QDir().mkpath(directory);

	const CServer &s = CConfigFile::getInstance()->getServer(server);

	// create the 2 initial lines of client.cfg
	QString rootConfigFilenameLine = QString("RootConfigFilename = \"%1\";").arg(s.getDefaultClientConfigFullPath());
	QString languageCodeline = QString("LanguageCode = \"%1\";\n").arg(language);

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
