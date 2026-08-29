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
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>

#include <QApplication>

#include "georges_editor_qt_config.h"
#include "../../3d/shared_widgets/common.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
	NLMISC::CApplicationContext appContext;

	// use log.log if NEL_LOG_IN_FILE and NLQT_USE_LOG_LOG defined as 1
	createDebug(NULL, NLQT_USE_LOG_LOG, false);

#if NLQT_USE_LOG
	// create tool log file
	static NLMISC::CFileDisplayer *s_FileDisplayer = NULL;
	if (NLQT_ERASE_LOG && NLMISC::CFile::isExists(NLQT_LOG_FILE))
		NLMISC::CFile::deleteFile(NLQT_LOG_FILE);
	s_FileDisplayer = new NLMISC::CFileDisplayer();
	s_FileDisplayer->setParam(NLQT_LOG_FILE, NLQT_ERASE_LOG);
	NLMISC::DebugLog->addDisplayer(s_FileDisplayer);
	NLMISC::InfoLog->addDisplayer(s_FileDisplayer);
	NLMISC::WarningLog->addDisplayer(s_FileDisplayer);
	NLMISC::AssertLog->addDisplayer(s_FileDisplayer);
	NLMISC::ErrorLog->addDisplayer(s_FileDisplayer);
#endif

	nlinfo("Welcome to Georges Editor Qt!");

	// Qt library path setup
	NLQT::preApplication();

	QApplication app(argc, argv);
	app.setApplicationName("Georges Editor Qt");
	app.setOrganizationName("NeL");

	// Dark Fusion palette as the default — will be overridden
	// by QtStyle/QtPalette config callbacks in MainWindow constructor
	NLQT::postApplication();

	MainWindow mainWindow;
	mainWindow.resize(1024, 768);
	mainWindow.show();

	// Open files passed on command line
	QStringList args = app.arguments();
	for (int i = 1; i < args.size(); ++i)
	{
		mainWindow.openDocument(args[i]);
	}

	return app.exec();
}
