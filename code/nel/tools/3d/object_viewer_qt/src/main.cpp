#include "stdpch.h"
#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>

// Qt includes
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QSplashScreen>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/command.h>
#include "extension_system/plugin_spec.h"
// Project includes
#include "modules.h"

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

sint main(int argc, char **argv)
{
	// go nel!
	{
		// use log.log if NEL_LOG_IN_FILE and NLQT_USE_LOG_LOG defined as 1
		createDebug(NULL, NLQT_USE_LOG_LOG, false);
#if NLQT_USE_LOG
		// create toverhex_client.log
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

		nlinfo("Welcome to NeL!");

		NLMISC::CPath::remapExtension("tga", "png", true);
	}

	QApplication app(argc, argv);
	QSplashScreen *splash = new QSplashScreen();
	splash->setPixmap(QPixmap(":/images/nel_ide_load.png"));
	splash->show();

	Modules::init();

	// load and set remap extensions from config
	Modules::config().configRemapExtensions();
	// load and set search paths from config
	Modules::config().configSearchPaths();

	Modules::mainWin().showMaximized();
	Modules::plugMan().addObject(&Modules::mainWin());

	Modules::plugMan().setPluginPaths(QStringList() << QString("./plugins"));
	Modules::plugMan().loadPlugins();
	
	QStringList errors;
	Q_FOREACH (NLQT::CPluginSpec *spec, Modules::plugMan().plugins())
		if (spec->hasError())
			errors.append(spec->fileName() + " : " + spec->errorString());
	
	if (!errors.isEmpty())
		QMessageBox::warning(0,
				QCoreApplication::translate("Application", "Object Viewer Qt - Plugin loader messages"),
				errors.join(QString::fromLatin1("\n\n")));

	splash->finish(&Modules::mainWin());
	int result = app.exec();
	Modules::release();
	return result;
}
