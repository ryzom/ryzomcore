/*
    Georges Editor Qt
	Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MODULES_H
#define MODULES_H

#include "configuration.h"
#include "main_window.h"
#include "interfaces.h"

namespace NLMISC {
	class IProgressCallback;
}

class Modules
{
public:
	static void init(NLMISC::IProgressCallback *cb);
	static void release();
	
	static NLQT::CConfiguration &config()  { return *_configuration; }
	static NLQT::IObjectViewer*  objViewInt() { return _objViewerInterface; }
	static NLQT::CMainWindow    &mainWin() { return *_mainWindow; }
private:
	static bool loadPlugin();

	static NLQT::IObjectViewer *_objViewerInterface;
	static NLQT::CConfiguration *_configuration;
	static NLQT::CMainWindow    *_mainWindow;
};

#endif // MODULES_H
