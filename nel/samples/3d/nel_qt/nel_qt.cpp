// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include <nel/misc/types_nl.h>
#include "nel_qt.h"

// STL includes
#include <stdio.h>
#ifdef NL_OS_WINDOWS
#	include <windows.h>
#	include <direct.h>
#	include <tchar.h>
#endif

// Qt includes
#include <QtGui/QApplication>
#include <QtCore/QMap>
#include <QtCore/qdebug.h>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/command.h>

// Project includes
#include "nel_qt_config.h"
#include "main_window.h"

using namespace std;
using namespace NLMISC;

namespace NLQT {

namespace {

CFileDisplayer *s_FileDisplayer = NULL;

} /* anonymous namespace */

} /* namespace NLQT */

void usage()
{
	/* from Qt sample */

	qWarning() << "Usage: mainwindow [-SizeHint<color> <width>x<height>] ...";
	exit(1);
}

QMap<QString, QSize> parseCustomSizeHints(int argc, char **argv)
{
	/* from Qt sample */

	QMap<QString, QSize> result;

	for (int i = 1; i < argc; ++i) {
		QString arg = QString::fromLocal8Bit(argv[i]);

		if (arg.startsWith(QLatin1String("-SizeHint"))) {
			QString name = arg.mid(9);
			if (name.isEmpty())
				usage();
			if (++i == argc)
				usage();
			QString sizeStr = QString::fromLocal8Bit(argv[i]);
			int idx = sizeStr.indexOf(QLatin1Char('x'));
			if (idx == -1)
				usage();
			bool ok;
			int w = sizeStr.left(idx).toInt(&ok);
			if (!ok)
				usage();
			int h = sizeStr.mid(idx + 1).toInt(&ok);
			if (!ok)
				usage();
			result[name] = QSize(w, h);
		}
	}

	return result;
}

#ifdef NL_OS_WINDOWS
#	ifdef _UNICODE
#		define tstring wstring
#	else
#		define tstring string
#	endif
#endif

sint main(int argc, char **argv)
{
#if defined(NL_OS_WINDOWS) && !FINAL_VERSION
	// ensure paths are ok before powering up nel
	{
		OutputDebugString("    ********************************    \n");
		OutputDebugString("    *        DEVELOPER MODE        *    \n");
		OutputDebugString("    ********************************    \n\n");
		FILE *f = _tfopen(_T(NLQT_CONFIG_FILE_DEFAULT), _T("r"));
		if (!f)
		{
			OutputDebugString("    ********************************    \n");
			OutputDebugString("    *  CHANGING WORKING DIRECTORY  *    \n");
			OutputDebugString("    ********************************    \n\n");
			char cwd[256];
			_tgetcwd(cwd, 256);
			tstring workdir(cwd);
			workdir += "\\..\\..\\..\\bin\\tools\\";
			_tchdir(workdir.c_str());
			f = _tfopen(_T(NLQT_CONFIG_FILE_DEFAULT), _T("r"));
			if (!f)
			{
				OutputDebugString("    ********************************    \n");
				OutputDebugString("    *    DEFAULT CONFIG MISSING    *    \n");
				OutputDebugString("    ********************************    \n\n");
				return EXIT_FAILURE;
			}
		}
		fclose(f);
	}
#endif

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
	}
	
	// low fragmentation heap (windows)
#if NLQT_LOW_FRAGMENTATION_HEAP
	ULONG heapFragValue = 2; // enable low fragmentation heap
	if (HeapSetInformation(GetProcessHeap(), 
		HeapCompatibilityInformation, 
		&heapFragValue, sizeof(heapFragValue)))
	{
		nlinfo("HeapSetInformation OK!\n");
	}
	else 
	{
		nlwarning("HeapSetInformation FAIL! (%d)\n", GetLastError());
	}
#endif

#ifdef NL_OS_WINDOWS
	HRESULT hr;
	hr = hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool coInitOk = (hr == S_OK) || (hr == S_FALSE);
#endif
	QApplication app(argc, const_cast<char **>(argv));
	QMap<QString, QSize> customSizeHints = parseCustomSizeHints(argc, argv);
	NLQT::CMainWindow mainWin(customSizeHints);
	mainWin.resize(800, 600);
	mainWin.show(); // calls isVisible(true)
	int result = app.exec();
#ifdef NL_OS_WINDOWS
	if (coInitOk) CoUninitialize();
#endif
	return result;
}

/* end of file */
