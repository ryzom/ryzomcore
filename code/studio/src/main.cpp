// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2010  Dzmitry Kamiahin <dnk-88@tut.by>
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

// Project includes
#include "extension_system/iplugin_spec.h"
#include "extension_system/plugin_manager.h"

// NeL includes
#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/dynloadlib.h>
#include <nel/misc/path.h>
#include <nel/misc/command.h>

// Qt includes
#include <QtCore/QDir>
#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
//#include <QtGui/QSplashScreen>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>

#include "settings_dialog.h"
#include "splash_screen.h"
#include "pm_watcher.h"

#ifdef HAVE_OVQT_CONFIG_H
#include "ovqt_config.h"
#endif

static const char *appNameC = "RyzomCoreStudio";

// nel_qt log file name
#define NLQT_LOG_FILE "nel_qt.log"

// clear nel_qt log before use
#define NLQT_ERASE_LOG true

#if !defined (NLQT_USE_LOG_LOG)
#	define NLQT_USE_LOG_LOG true
#endif
#if !defined (NLQT_USE_LOG)
#	define NLQT_USE_LOG 1
#endif

namespace NLQT
{

namespace
{
NLMISC::CFileDisplayer *s_FileDisplayer = NULL;
} /* anonymous namespace */

} /* namespace NLQT */

#ifndef DATA_DIR
#       define DATA_DIR "."
#endif


#ifdef NL_OS_WINDOWS
#	ifdef _UNICODE
#		define tstring wstring
#	else
#		define tstring string
#	endif
#endif

#ifdef Q_OS_WIN

static void displayError(const QString &t) // No console on Windows.
{
	QMessageBox::critical(0, QLatin1String(appNameC), t);
}

#else

static void displayError(const QString &t)
{
	qCritical("%s", qPrintable(t));
}

#endif

static inline QString msgCoreLoadFailure(const QString &why)
{
	return QCoreApplication::translate("Application", "Failed to load Core plugin: %1").arg(why);
}


#ifdef NL_OS_WINDOWS
int __stdcall WinMain(void *hInstance, void *hPrevInstance, void *lpCmdLine, int nShowCmd)
#else // NL_OS_WINDOWS
int main(int argc, char **argv)
#endif // NL_OS_WINDOWS
{
	// go nel!
	new NLMISC::CApplicationContext;
	{
		// use log.log if NEL_LOG_IN_FILE and NLQT_USE_LOG_LOG defined as 1
		NLMISC::createDebug(NULL, NLQT_USE_LOG_LOG, false);
#if NLQT_USE_LOG
		// create NLQT_LOG_FILE
		// filedisplayer only deletes the 001 etc
		if (NLQT_ERASE_LOG && NLMISC::CFile::isExists(NLQT_LOG_FILE))
			NLMISC::CFile::deleteFile(NLQT_LOG_FILE);
		// initialize the log file
		NLQT::s_FileDisplayer = new NLMISC::CFileDisplayer();
		NLQT::s_FileDisplayer->setParam(NLQT_LOG_FILE, NLQT_ERASE_LOG);
		NLMISC::DebugLog->addDisplayer(NLQT::s_FileDisplayer);
		NLMISC::InfoLog->addDisplayer(NLQT::s_FileDisplayer);
		NLMISC::WarningLog->addDisplayer(NLQT::s_FileDisplayer);
		NLMISC::AssertLog->addDisplayer(NLQT::s_FileDisplayer);
		NLMISC::ErrorLog->addDisplayer(NLQT::s_FileDisplayer);
#endif

		nlinfo("Welcome to NeL Object Viewer Qt!");
	}
	QApplication::setGraphicsSystem("raster");
#ifdef NL_OS_WINDOWS
	QApplication app(__argc, __argv);
#else // NL_OS_WINDOWS
	QApplication app(argc, argv);
#endif // NL_OS_WINDOWS

	SplashScreen *splash = new SplashScreen();
	splash->setPixmap(QPixmap(":/images/studio_splash.png"));
	splash->setProgressBarEnabled( true );
	splash->setText( "Starting up..." );
	splash->setProgress( 0 );
	splash->show();

	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
	                                    QLatin1String("RyzomCore"), QLatin1String(appNameC));

	bool firstRun = settings->value( "FirstRun", true ).toBool();
	if( firstRun )
	{
		settings->setValue( "FirstRun", false );
		
		SettingsDialog sd;
		sd.setSettings( settings );
		sd.load();
		sd.exec();
	}

	QTranslator translator;
	QTranslator qtTranslator;
	QString locale = settings->value("Language", QLocale::system().name()).toString();
	QString qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
//	translator.load("object_viewer_qt_" + locale, ":/");
	qtTranslator.load("qt_" + locale, qtTrPath);
	app.installTranslator(&translator);
	app.installTranslator(&qtTranslator);

	splash->setText( "Loading plugins..." );
	splash->setProgress( 20 );

#if defined(NL_OS_MAC)
	QDir::setCurrent(qApp->applicationDirPath() + QString("/../Resources"));
	NLMISC::CLibrary::addLibPath((qApp->applicationDirPath() + QString("/../PlugIns/nel")).toUtf8().constData());
#endif

	ExtensionSystem::PluginManager pluginManager;
	pluginManager.setSettings(settings);
	QStringList pluginPaths;
#if defined(NL_OS_MAC)
	pluginPaths << settings->value("PluginPath", qApp->applicationDirPath() + QString("/../PlugIns/studio")).toString();
#else
	pluginPaths << settings->value("PluginPath", QString("%1/plugins").arg(DATA_DIR)).toString();
#endif

	pluginManager.setPluginPaths(pluginPaths);

	PluginManagerWatcher watcher;
	watcher.setPluginManager( &pluginManager );
	watcher.setSplashScreen( splash );
	watcher.connect();

	pluginManager.loadPlugins();

	watcher.disconnect();
	splash->hide();

	ExtensionSystem::IPluginSpec *corePlugin = pluginManager.pluginByName("Core");
	
	if (!corePlugin)
	{
		QDir absolutePluginPaths(pluginPaths.join(QLatin1String(",")));
		QString absolutePaths = absolutePluginPaths.absolutePath();
		const QString reason = QCoreApplication::translate("Application", "Could not find studio_plugin_core in %1").arg(absolutePaths);
		displayError(msgCoreLoadFailure(reason));

		QString newPath = QFileDialog::getExistingDirectory(0, QCoreApplication::translate("Application", "Change the plugins path"), QDir::homePath());
		bool ok;
		QString text = QInputDialog::getText(0, QCoreApplication::translate("Application", "Enter the plugins path"),
		                                     QCoreApplication::translate("Application", "Plugin path:"), QLineEdit::Normal,
		                                     newPath, &ok);
		if (ok && !text.isEmpty())
			settings->setValue("PluginPath", text);
		settings->sync();
		return 1;
	}
	if (corePlugin->hasError())
	{
		displayError(msgCoreLoadFailure(corePlugin->errorString()));
		return 1;
	}

	QStringList errors;
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, pluginManager.plugins())
	if (spec->hasError())
		errors.append(spec->fileName() + " : " + spec->errorString());

	if (!errors.isEmpty())
		QMessageBox::warning(0, QCoreApplication::translate("Application", "Studio - Plugin loader messages"),
		                     errors.join(QString::fromLatin1("\n\n")));

	int result = app.exec();
	return result;
}
