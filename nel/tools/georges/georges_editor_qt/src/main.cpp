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

// Define GEORGES_USE_DARK_THEME to enable the dark Fusion palette from
// shared_widgets/common.h (matching Panoply Preview and nel_qt sample).
// The default is enabled. Set to 0 to use the OS-native look and feel.
#ifndef GEORGES_USE_DARK_THEME
#define GEORGES_USE_DARK_THEME 1
#endif

#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>

#include <QApplication>
#include <QStyleFactory>

#include "../../3d/shared_widgets/common.h"
#include "main_window.h"

int main(int argc, char *argv[])
{
	NLMISC::CApplicationContext appContext;

	// use log.log if NEL_LOG_IN_FILE defined as 1
	createDebug(NULL, false, false);

#if GEORGES_USE_DARK_THEME
	NLQT::preApplication();
#endif
	QApplication app(argc, argv);
	app.setApplicationName("Georges Editor Qt");
	app.setOrganizationName("NeL");
#if GEORGES_USE_DARK_THEME
	NLQT::postApplication();
#endif

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
