#include "words_dicDlg.h"

#include <QApplication>
#include "nel/misc/app_context.h"
#include "nel/misc/path.h"

#ifdef NL_OS_UNIX
#include <stdlib.h>
#endif // NL_OS_UNIX

#ifndef NL_WORDS_DIC_CFG
#define NL_WORDS_DIC_CFG "."
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

#endif

int main(int argc, char *argv[])
{

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need.
	NLMISC::CApplicationContext myApplicationContext;


#ifdef NL_OS_UNIX
	std::string homeDir = getenv("HOME");
	NLMISC::CPath::addSearchPath( homeDir + "/.nel");
#endif // NL_OS_UNIX

	NLMISC::CPath::addSearchPath(NL_WORDS_DIC_CFG);

	Q_INIT_RESOURCE(words_dic_Qt);
	QApplication app(argc, argv);

	CWords_dicDlg wordsDic;
	wordsDic.show();

	return app.exec();
}
