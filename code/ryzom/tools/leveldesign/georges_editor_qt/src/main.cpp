#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>

// Qt includes
#include <QtGui/QApplication>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QSplashScreen>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/command.h>
#include <nel/misc/dynloadlib.h>

// Project includes
#include "modules.h"
#include "georges_splash.h"

#ifdef HAVE_GEQT_CONFIG_H
#include "geqt_config.h"
#endif

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

void messageHandler(QtMsgType p_type, const char* p_msg) 
{

	fprintf(stderr, "%s\n", p_msg);

	QFile file("qt.log");
	file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

	QChar code;
	switch (p_type)
	{
	case QtDebugMsg:    code = 'D';  break;
	case QtWarningMsg:  code = 'W';  break;
	case QtCriticalMsg: code = 'C';  break;
	case QtFatalMsg:    code = 'F';  break;
	}
	QString dt = QDateTime::currentDateTime().toString("yyyyMMdd hh:mm:ss");

	QTextStream(&file) << QString("%1 [%2]  %3\n").arg(dt).arg(code).arg(QString(p_msg));
}

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
	QApplication app(argc, argv);
	QPixmap pixmap(":/images/georges_logo.png");
	NLQT::CGeorgesSplash splash;
    splash.show();
    app.processEvents();

	NLMISC::CApplicationContext myApplicationContext;

#if defined(NL_OS_MAC)
	QDir::setCurrent(qApp->applicationDirPath() + QString("/../Resources"));
	CLibrary::addLibPath(
		(qApp->applicationDirPath() + QString("/../PlugIns/nel")).toUtf8().constData());
#endif

	// go nel!
	{
		// use log.log if NEL_LOG_IN_FILE and NLQT_USE_LOG_LOG defined as 1
		createDebug(NULL, NLQT_USE_LOG_LOG, false);
		if (QFile::exists("qt.log"))
			QFile::remove("qt.log");

		qInstallMsgHandler(messageHandler);
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

	Modules::init(&splash);
	//Modules::mainWin().resize(800,600);
	Modules::mainWin().show();
	splash.close();
	int result = app.exec();
	Modules::release();
	return result;
}
