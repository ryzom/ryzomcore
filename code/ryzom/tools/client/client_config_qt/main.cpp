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

#include "client_config_dialog.h"
#include "system.h"
#include "nel/misc/cmd_args.h"

#include <QSplashScreen>

#ifdef QT_STATICPLUGIN

#include <QtPlugin>

#if defined(Q_OS_WIN32)
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif defined(Q_OS_MAC)
	Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#elif defined(Q_OS_UNIX)
	Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
	Q_IMPORT_PLUGIN(QXcbGlxIntegrationPlugin)
#endif

#endif

int main(sint32 argc, char **argv)
{
	NLMISC::CApplicationContext applicationContext;

	QApplication app(argc, argv);

	// parse command-line arguments
	NLMISC::CCmdArgs args;
	args.setDescription("Ryzom Configuration");
	args.addArg("p", "profile", "id", "Use this profile to determine what directory to use by default");

	if (!args.parse(argc, argv)) return 1;

	QApplication::setWindowIcon(QIcon(":/resources/welcome_icon.png"));
	QPixmap pixmap(":/resources/splash_screen.png" );
	QSplashScreen splash( pixmap );
	splash.show();

	QLocale locale = QLocale::system();

	// load application translations
	QTranslator localTranslator;
	if (localTranslator.load(locale, "ryzom_configuration", "_", ":/translations"))
	{
		QApplication::installTranslator(&localTranslator);
	}

	// load Qt default translations
	QTranslator qtTranslator;
	if (qtTranslator.load(locale, "qt", "_", ":/translations"))
	{
		QApplication::installTranslator(&qtTranslator);
	}

	// Known cases:
	// 1. Steam
	// - Linux and Windows: all files in Steam folder
	// - OS X: client.cfg in ~/Library/Application Support/Ryzom, client_default.cfg in Steam folder
	// 2. Installer
	// - Linux: client.cfg in ~/.ryzom/<config>/ client_default.cfg in ~/.ryzom/ryzom_live/
	// - Windows: client.cfg in Roaming/Ryzom/<config>/ client_default.cfg in Local/Ryzom/ryzom_live/
	// - OS X: client.cfg in ~/Library/Application Support/Ryzom/<config>/ client_default.cfg in ~/Library/Application Support/Ryzom/ryzom_live/

	// default paths
	std::string ryzomDir = NLMISC::CPath::standardizePath(NLMISC::CPath::getApplicationDirectory("Ryzom"));
	std::string currentDir = args.getStartupPath();
	std::string executableDir = args.getProgramPath();

	std::string configFilename = "client.cfg";
	std::string configPath;

	// search client.cfg file in config directory (Ryzom Installer)
	if (args.haveArg("p"))
	{
		ryzomDir = NLMISC::CPath::standardizePath(ryzomDir + args.getArg("p").front());

		// client.cfg is always in profile directory if using -p argument
		configPath = ryzomDir + configFilename;
	}
	else
	{
#ifdef NL_OS_MAC
		// client.cfg is in ~/Library/Application Support/Ryzom under OS X
		configPath = ryzomDir + configFilename;
#else
		// client.cfg is in current directory under other platforms
		configPath = currentDir + configFilename;
#endif
	}

	// if file doesn't exist, create it
	if (!NLMISC::CFile::fileExists(configPath))
	{
		// we need the full path to client_default.cfg
		std::string defaultConfigFilename = "client_default.cfg";
		std::string defaultConfigPath;

#ifdef NL_OS_MAC
		// fix path inside bundle
		defaultConfigPath = NLMISC::CPath::makePathAbsolute("../Resources", executableDir, true) + defaultConfigFilename;
#else
		// same path as executables
		defaultConfigPath = executableDir + defaultConfigFilename;
#endif

		// test if default config exists in determined path
		if (!NLMISC::CFile::fileExists(defaultConfigPath))
		{
			defaultConfigPath = currentDir + defaultConfigFilename;

			// test if default config exists in current path
			if (!NLMISC::CFile::fileExists(defaultConfigPath))
			{
				nlwarning("Unable to find %s", defaultConfigFilename.c_str());
				return 1;
			}
		}

		if (!CSystem::GetInstance().config.create(configPath, defaultConfigPath))
		{
			nlwarning("Unable to create %s", configPath.c_str());
			return 1;
		}
	}

	if (!CSystem::GetInstance().config.load(configPath))
	{
		nlwarning("Unable to load %s", configPath.c_str());
		return 1;
	}

	CClientConfigDialog d;
	d.show();
	splash.finish( &d );

	return app.exec();
}
