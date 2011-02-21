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

#include "stdpch.h"
#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>

// Qt includes
#include <QtCore/QDir>
#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QSplashScreen>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/dynloadlib.h>
#include <nel/misc/path.h>
#include <nel/misc/command.h>

// Project includes
#include "modules.h"
#include "extension_system/iplugin_spec.h"
#include "extension_system/plugin_manager.h"

static const char *appNameC = "ObjectViewerQt";

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

using namespace std;
using namespace NLMISC;

namespace NLQT
{

namespace
{

CFileDisplayer *s_FileDisplayer = NULL;

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

#define OVQT_OLD true

sint main(int argc, char **argv)
{
	// go nel!
	new NLMISC::CApplicationContext;
	{
		// use log.log if NEL_LOG_IN_FILE and NLQT_USE_LOG_LOG defined as 1
		createDebug(NULL, NLQT_USE_LOG_LOG, false);
#if NLQT_USE_LOG
		// create NLQT_LOG_FILE
		// filedisplayer only deletes the 001 etc
		if (NLQT_ERASE_LOG && CFile::isExists(NLQT_LOG_FILE))
			CFile::deleteFile(NLQT_LOG_FILE);
		// initialize the log file
		NLQT::s_FileDisplayer = new CFileDisplayer();
		NLQT::s_FileDisplayer->setParam(NLQT_LOG_FILE, NLQT_ERASE_LOG);
		DebugLog->addDisplayer(NLQT::s_FileDisplayer);
		InfoLog->addDisplayer(NLQT::s_FileDisplayer);
		WarningLog->addDisplayer(NLQT::s_FileDisplayer);
		AssertLog->addDisplayer(NLQT::s_FileDisplayer);
		ErrorLog->addDisplayer(NLQT::s_FileDisplayer);
#endif

		nlinfo("Welcome to NeL Object Viewer Qt!");
	}
	QApplication app(argc, argv);
	QSplashScreen *splash = new QSplashScreen();
	splash->setPixmap(QPixmap(":/images/nel_ide_load.png"));
	splash->show();

	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings *settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
										QLatin1String("RyzomCore"), QLatin1String(appNameC));

	QTranslator translator;
	QTranslator qtTranslator;
	QString locale = settings->value("Language", QLocale::system().name()).toString();
	QString qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
	translator.load("object_viewer_qt_" + locale, ":/translations");
	qtTranslator.load("qt_" + locale, qtTrPath);
	app.installTranslator(&translator);
	app.installTranslator(&qtTranslator);

#if defined(NL_OS_MAC)
	QDir::setCurrent(qApp->applicationDirPath() + QString("/../Resources"));
	CLibrary::addLibPath((qApp->applicationDirPath() + QString("/../PlugIns/nel")).toStdString());
#endif

#if defined(OVQT_OLD)
	Modules::init();

	Modules::plugMan().setSettings(settings);

	// load and set remap extensions from config
	Modules::config().configRemapExtensions();
	// load and set search paths from config
	Modules::config().configSearchPaths();

	Modules::mainWin().showMaximized();
	Modules::plugMan().addObject(&Modules::mainWin());

#if !defined(NL_OS_MAC)
	Modules::plugMan().setPluginPaths(QStringList() << QString("./plugins"));
#else
	Modules::plugMan().setPluginPaths(QStringList() <<
									  qApp->applicationDirPath() + QString("/../PlugIns/ovqt"));
#endif

	Modules::plugMan().loadPlugins();

	QStringList errors;
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, Modules::plugMan().plugins())
	if (spec->hasError())
		errors.append(spec->fileName() + " : " + spec->errorString());

	if (!errors.isEmpty())
		QMessageBox::warning(0, QCoreApplication::translate("Application", "Object Viewer Qt - Plugin loader messages"),
							 errors.join(QString::fromLatin1("\n\n")));

	splash->finish(&Modules::mainWin());
	int result = app.exec();
	Modules::release();
#else
	ExtensionSystem::CPluginManager pluginManager;
	pluginManager.setSettings(settings);
	QStringList pluginPaths;
#if !defined(NL_OS_MAC)
	pluginPaths << QString("./plugins");
#else
	pluginPaths << qApp->applicationDirPath() + QString("/../PlugIns/ovqt");
#endif

	pluginManager.setPluginPaths(pluginPaths);
	pluginManager.loadPlugins();

	splash->hide();

	const QList<ExtensionSystem::IPluginSpec *> plugins = pluginManager.plugins();
	ExtensionSystem::IPluginSpec *corePlugin = 0;
	Q_FOREACH(ExtensionSystem::IPluginSpec *spec, plugins)
	{
		if (spec->name() == QLatin1String("Core"))
		{
			corePlugin = spec;
			break;
		}
	}

	if (!corePlugin)
	{
		QDir absolutePluginPaths(pluginPaths.join(QLatin1String(",")));
		QString absolutePaths = absolutePluginPaths.absolutePath();
		const QString reason = QCoreApplication::translate("Application", "Could not find ovqt_plugin_core in %1").arg(absolutePaths);
		displayError(msgCoreLoadFailure(reason));
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
		QMessageBox::warning(0, QCoreApplication::translate("Application", "Object Viewer Qt - Plugin loader messages"),
							 errors.join(QString::fromLatin1("\n\n")));

	int result = app.exec();
#endif

	return result;
}