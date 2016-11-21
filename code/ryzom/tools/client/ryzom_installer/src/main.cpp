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
#include "utils.h"

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

// copy all specified files from current directory to destination directory
bool copyInstallerFiles(const QStringList &files, const QString &destination)
{
	QString path = QApplication::applicationDirPath();

	foreach(const QString &file, files)
	{
		// convert to absolute path
		QString srcPath = path + "/" + file;
		QString dstPath = destination + "/" + file;

		if (QFile::exists(srcPath))
		{
			if (QFile::exists(dstPath))
			{
				if (!QFile::remove(dstPath))
				{
					nlwarning("Unable to delete %s", Q2C(dstPath));
				}
			}

			if (!QFile::copy(srcPath, dstPath))
			{
				nlwarning("Unable to copy %s to %s", Q2C(srcPath), Q2C(dstPath));

				return false;
			}
		}
	}

	return true;
}

int main(int argc, char *argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#ifdef Q_OS_WIN
	CCOMHelper comHelper;
#endif

	NLMISC::CApplicationContext appContext;

	QApplication app(argc, argv);

	QApplication::setApplicationName("Ryzom");
	QApplication::setApplicationVersion(RYZOM_VERSION);
	QApplication::setWindowIcon(QIcon(":/icons/ryzom.ico"));

	// remove first argument because it's not really an argument
	QStringList args = QApplication::arguments();
	args.removeFirst();

	QLocale locale = QLocale::system();

	// load application translations
	QTranslator localTranslator;
	if (localTranslator.load(locale, "ryzom_installer", "_", ":/translations"))
	{
		QApplication::installTranslator(&localTranslator);
	}

	// load Qt default translations
	QTranslator qtTranslator;
	if (qtTranslator.load(locale, "qtbase", "_", ":/translations"))
	{
		QApplication::installTranslator(&qtTranslator);
	}

	// define commandline arguments
	QCommandLineParser parser;
	parser.setApplicationDescription(QApplication::tr("Installation and launcher tool for Ryzom"));
	parser.addHelpOption();

	QCommandLineOption uninstallOption(QStringList() << "u" << "uninstall", QApplication::tr("Uninstall"));
	parser.addOption(uninstallOption);

	QCommandLineOption silentOption(QStringList() << "s" << "silent", QApplication::tr("Silent mode"));
	parser.addOption(silentOption);

	QCommandLineOption versionOption(QStringList() << "v" << "version", QApplication::tr("Version"));
	parser.addOption(versionOption);

	QCommandLineOption installOption(QStringList() << "i" << "install", QApplication::tr("Install itself"));
	parser.addOption(installOption);

	// process the actual command line arguments given by the user
	parser.process(app);

	// don't need to load config file for version
	if (parser.isSet(versionOption))
	{
		printf("Ryzom Installer %s (built on %s)\nCopyright (C) %s\n", RYZOM_VERSION, BUILD_DATE, COPYRIGHT);

		return 0;
	}

	// instanciate ConfigFile
	CConfigFile config;

	bool res = config.load();

	// init log
	CLogHelper logHelper(config.getInstallationDirectory().isEmpty() ? config.getNewInstallationDirectory():config.getInstallationDirectory());

	nlinfo("Launched %s", Q2C(config.getInstallerCurrentFilePath()));

	OperationStep step = res ? config.getInstallNextStep():DisplayNoServerError;

	if (res == DisplayNoServerError)
	{
		QMessageBox::critical(NULL, QApplication::tr("Error"), QApplication::tr("Unable to find ryzom_installer.ini"));
		return 1;
	}

#if defined(Q_OS_WIN) && !defined(_DEBUG)
	// under Windows, Ryzom Installer should always be copied in TEMP directory
	QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

	// check if launched from TEMP directory
	if (step == Done && !config.getInstallerCurrentDirPath().startsWith(tempPath))
	{
		nlinfo("Not launched from TEMP directory");

		// try to delete all temporary installers
		QDir tempDir(tempPath);

		QStringList filter;
		filter << "ryzom_installer_*";

		QStringList dirs = tempDir.entryList(filter, QDir::Dirs);

		foreach(const QString &dir, dirs)
		{
			// delete each directory
			QDir dirToRemove(tempDir);
			dirToRemove.cd(dir);
			dirToRemove.removeRecursively();

			nlinfo("Delete directory %s", Q2C(dir));
		}

		tempPath += QString("/ryzom_installer_%1").arg(QDateTime::currentMSecsSinceEpoch());

		nlinfo("Creating directory %s", Q2C(tempPath));

		// copy installer and required files to TEMP directory
		if (QDir().mkdir(tempPath) && copyInstallerFiles(config.getInstallerRequiredFiles(), tempPath))
		{
			QString tempFile = tempPath + "/" + QFileInfo(config.getInstallerCurrentFilePath()).fileName();

			nlinfo("Launching %s", Q2C(tempFile));

			// launch copy in TEMP directory with same arguments
			if (QProcess::startDetached(tempFile, args, tempPath)) return 0;

			nlwarning("Unable to launch %s", Q2C(tempFile));
		}
	}
#endif

	// use product name from ryzom_installer.ini
	if (!config.getProductName().isEmpty()) QApplication::setApplicationName(config.getProductName());

	if (parser.isSet(uninstallOption))
	{
		nlinfo("Uninstalling...");

		SComponents components;

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

		dialog.setCurrentServerId(config.getProfile().server);
		dialog.setOperation(OperationUninstall);
		dialog.setUninstallComponents(components);

		// TODO: set all components to uninstall

		return dialog.exec() ? 0 : 1;
	}

	if (step == ShowMigrateWizard)
	{
		nlinfo("Display migration dialog");
#ifdef Q_OS_WIN32
		CMigrateDialog dialog;

		if (!dialog.exec()) return 1;

		step = config.getInstallNextStep();
#else
		nlwarning("Migration disabled under Linux and OS X");
#endif
	}
	else if (step == ShowInstallWizard)
	{
		nlinfo("Display installation dialog");

		CInstallDialog dialog;

		if (!dialog.exec()) return 1;

		step = config.getInstallNextStep();
	}

	nlinfo("Next step is %s", Q2C(stepToString(step)));

	bool restartInstaller = false;
	
	if (step != Done)
	{
		COperationDialog dialog;
		dialog.setCurrentServerId(config.getProfile().server);
		dialog.setOperation(config.getSrcServerDirectory().isEmpty() ? OperationInstall:OperationMigrate);

		if (!dialog.exec()) return 1;

		step = config.getInstallNextStep();

		nlinfo("Last step is %s", Q2C(stepToString(step)));

		if (step == LaunchInstalledInstaller)
		{
			// restart more recent installed Installer version
			restartInstaller = true;
		}
		else if (step == Done)
		{
#if defined(Q_OS_WIN) && !defined(_DEBUG)
			// restart Installer, so it could be copied in TEMP and allowed to update itself
			restartInstaller = true;
#endif
		}
	}

	if (restartInstaller)
	{
#ifndef _DEBUG
		nlinfo("Restart Installer %s", Q2C(config.getInstallerInstalledFilePath()));

#ifndef Q_OS_WIN32
		// fix executable permissions under UNIX
		QFile::setPermissions(config.getInstallerInstalledFilePath(), QFile::permissions(config.getInstallerInstalledFilePath()) | QFile::ExeGroup | QFile::ExeUser | QFile::ExeOther);
#endif

		if (QProcess::startDetached(config.getInstallerInstalledFilePath(), args, config.getInstallationDirectory())) return 0;

		nlwarning("Unable to restart Installer %s", Q2C(config.getInstallerInstalledFilePath()));
#endif
	}

	CMainWindow mainWindow;
	mainWindow.show();

	return QApplication::exec();
}
