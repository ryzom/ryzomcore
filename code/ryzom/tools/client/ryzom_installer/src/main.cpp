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
#include "migratewizarddialog.h"
#include "installwizarddialog.h"
#include "operationdialog.h"

#include "nel/misc/path.h"
#include "nel/misc/ucstring.h"

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

	// TODO: parameters -u (uinstall) and -s (silent)

	QApplication::setApplicationName("Ryzom");
	QApplication::setApplicationVersion(RYZOM_VERSION);
	QApplication::setWindowIcon(QIcon(":/icons/ryzom.ico"));

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
	CConfigFile::InstallationStep step = config.load() ? config.getNextStep():CConfigFile::DisplayNoServerError;

	if (step == CConfigFile::DisplayNoServerError)
	{
		QMessageBox::critical(NULL, QApplication::tr("Error"), QApplication::tr("Unable to find installer.ini"));
		return 1;
	}

	bool displayMainWindow = true;

	if (step == CConfigFile::ShowMigrateWizard)
	{
		CMigrateWizardDialog dialog;

		if (!dialog.exec()) displayMainWindow = false;
	}
	else if (step == CConfigFile::ShowInstallWizard)
	{
		CInstallWizardDialog dialog;

		if (!dialog.exec()) displayMainWindow = false;
	}


	if (displayMainWindow)
	{
		step = config.getNextStep();

		if (step != CConfigFile::Done)
		{
			COperationDialog dialog;

			if (!dialog.exec()) displayMainWindow = false;
		}
	}

	if (displayMainWindow)
	{
		step = config.getNextStep();

		if (step == CConfigFile::Done)
		{
			CMainWindow mainWindow;
			mainWindow.show();

			return QApplication::exec();
		}
	}

	return 0;
}
