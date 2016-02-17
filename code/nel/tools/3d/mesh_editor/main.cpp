// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  Jan BOON (jan.boon@kaetemi.be)
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

// STL includes
#include <stdio.h>
#ifdef NL_OS_WINDOWS
#	include <windows.h>
#	include <direct.h>
#	include <tchar.h>
#endif

// Qt includes
#include <qglobal.h>

#ifdef Q_COMPILER_RVALUE_REFS
#undef Q_COMPILER_RVALUE_REFS
#endif

#include <QApplication>
#include <QMap>
#include <QStyleFactory>
#include <QFileInfo>

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

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/command.h>
#include <nel/misc/sheet_id.h>

// Project includes
#include "../shared_widgets/common.h"
#include "main_window.h"

using namespace std;
using namespace NLMISC;

#define NLTOOLS_LOG_FILE "mesh_editor.log"

namespace {

CFileDisplayer *s_FileDisplayer = NULL;

} /* anonymous namespace */

#ifdef NL_OS_WINDOWS
#	ifdef _UNICODE
#		define tstring wstring
#	else
#		define tstring string
#	endif
#endif

sint main(int argc, char **argv)
{
	// low fragmentation heap (windows)
#ifdef NL_OS_WINDOWS
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

	std::string localAppData = CPath::getApplicationDirectory("NeL", true);
	std::string appData = CPath::getApplicationDirectory("NeL");
	CFile::createDirectoryTree(localAppData);
	CFile::createDirectoryTree(appData);
	CFile::createDirectoryTree(appData + "screenshots/");

	CFile::setRWAccess(localAppData + "andbasr.ttf");
	QFile::remove(QString::fromStdString(localAppData + "andbasr.ttf"));
	QFile(":/data/andbasr.ttf").copy(QString::fromStdString(localAppData + "andbasr.ttf"));
	CFile::setRWAccess(appData + "mesh_editor_default.cfg");
	QFile::remove(QString::fromStdString(appData + "mesh_editor_default.cfg"));
	QFile(":/data/mesh_editor_default.cfg").copy(QString::fromStdString(appData + "mesh_editor_default.cfg"));
	if (!QFileInfo(QString::fromStdString(appData + "mesh_editor.cfg")).exists())
	{
		QFile(":/data/mesh_editor.cfg").copy(QString::fromStdString(appData + "mesh_editor.cfg"));
		CFile::setRWAccess(appData + "mesh_editor.cfg");
	}

	NLQT::preApplication();
	QApplication app(argc, const_cast<char **>(argv));
	NLQT::postApplication();

	// go nel!
	{
		// use log.log if NEL_LOG_IN_FILE and NLTOOLS_USE_LOG_LOG defined as 1
		createDebug(NULL, false, false);

		// create log
		// filedisplayer only deletes the 001 etc
		if (CFile::isExists(localAppData + NLTOOLS_LOG_FILE))
			CFile::deleteFile(localAppData + NLTOOLS_LOG_FILE);
		// initialize the log file
		s_FileDisplayer = new CFileDisplayer();
		s_FileDisplayer->setParam(localAppData + NLTOOLS_LOG_FILE, true);
		DebugLog->addDisplayer(s_FileDisplayer);
		InfoLog->addDisplayer(s_FileDisplayer);
		WarningLog->addDisplayer(s_FileDisplayer);
		AssertLog->addDisplayer(s_FileDisplayer);
		ErrorLog->addDisplayer(s_FileDisplayer);

		nlinfo("Welcome to NeL!");
	}

	CSheetId::initWithoutSheet();

	CMainWindow mainWin;
	mainWin.resize(1280, 720);
	mainWin.show();

	int result = app.exec();

#ifdef NL_OS_WINDOWS
	if (coInitOk) CoUninitialize();
#endif

	return result;
}

/* end of file */
