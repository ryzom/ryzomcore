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
#include "mainwindow.h"
#include "configfile.h"
#include "migratedialog.h"
#include "installdialog.h"
#include "uninstalldialog.h"
#include "operationdialog.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef QT_STATICPLUGIN

#include <QtPlugin>

#if defined(Q_OS_WIN32)
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif defined(Q_OS_MAC)
	Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#elif defined(Q_OS_UNIX)
	Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

	Q_IMPORT_PLUGIN(QICOPlugin)

#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

int main(int argc, char *argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	NLMISC::CApplicationContext appContext;

	QApplication app(argc, argv);

	QApplication::setApplicationName("Ryzom");
	QApplication::setApplicationVersion(RYZOM_VERSION);
	QApplication::setWindowIcon(QIcon(":/icons/ryzom.ico"));

	// TODO: if not launched from TEMP dir, copy files to TEMP, restart it and exit 

	QLocale locale = QLocale::system();

	// load application translations
	QTranslator localTranslator;
	if (localTranslator.load(locale, "ryzom_installer", "_", "translations"))
	{
		QApplication::installTranslator(&localTranslator);
	}

	// load Qt default translations
	QTranslator qtTranslator;
	if (qtTranslator.load(locale, "qt", "_", "translations"))
	{
		QApplication::installTranslator(&qtTranslator);
	}

	// instanciate ConfigFile
	CConfigFile config;
	OperationStep step = config.load() ? config.getInstallNextStep():DisplayNoServerError;

	if (step == DisplayNoServerError)
	{
		QMessageBox::critical(NULL, QApplication::tr("Error"), QApplication::tr("Unable to find installer.ini"));
		return 1;
	}

	// use product name from installer.ini
	if (!config.getProductName().isEmpty()) QApplication::setApplicationName(config.getProductName());

	// define commandline arguments
	QCommandLineParser parser;
//	parser.setApplicationDescription(DESCRIPTION);
	parser.addHelpOption();
	parser.addVersionOption();

	// root, username and password are optional because they can be saved in settings file
	QCommandLineOption uninstallOption(QStringList() << "u" << "uninstall", QApplication::tr("Uninstall"));
	parser.addOption(uninstallOption);

	QCommandLineOption silentOption(QStringList() << "s" << "silent", QApplication::tr("Silent mode"));
	parser.addOption(silentOption);

	// process the actual command line arguments given by the user
	parser.process(app);

	if (parser.isSet(uninstallOption))
	{
		SUninstallComponents components;

		// add all servers by default
		for (int i = 0; i < config.getServersCount(); ++i)
		{
			components.servers << config.getServer(i).id;
		}

		// show uninstall wizard dialog if not in silent mode
		if (!parser.isSet(silentOption))
		{
			CUninstallDialog dialog;

			dialog.setSelectedComponents(components);

			// TODO: check real return codes from Uninstallers
			if (!dialog.exec()) return 1;

			components = dialog.getSelectedCompenents();
		}

		COperationDialog dialog;

		dialog.setOperation(OperationUninstall);
		dialog.setUninstallComponents(components);

		// TODO: set all components to uninstall

		return dialog.exec() ? 0 : 1;
	}

	if (step == ShowMigrateWizard)
	{
		CMigrateDialog dialog;

		if (!dialog.exec()) return 1;

		step = config.getNextStep();
	}
	else if (step == ShowInstallWizard)
	{
		CInstallDialog dialog;

		if (!dialog.exec()) return 1;

		step = config.getNextStep();
	}
	
	if (step != Done)
	{
		COperationDialog dialog;
		dialog.setOperation(config.getSrcServerDirectory().isEmpty() ? OperationInstall:OperationMigrate);

		if (!dialog.exec()) return 1;

		step = config.getNextStep();
	}

	CMainWindow mainWindow;
	mainWindow.show();

	return QApplication::exec();
}
