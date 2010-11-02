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

#include "modules.h"

NLQT::CConfiguration *Modules::_configuration = NULL;
NLQT::CObjectViewer *Modules::_objectViewer = NULL;
NLQT::CGeorges *Modules::_georges = NULL;
NLQT::CMainWindow *Modules::_mainWindow = NULL;
	
void Modules::init()
{
	if (_configuration == NULL) _configuration = new NLQT::CConfiguration;
	config().init();
	
	if (_objectViewer == NULL) _objectViewer = new NLQT::CObjectViewer;
	if (_georges == NULL) _georges = new NLQT::CGeorges;
	if (_mainWindow == NULL) _mainWindow = new NLQT::CMainWindow;
}

void Modules::release()
{
	delete _mainWindow; _mainWindow = NULL;
	delete _objectViewer; _objectViewer = NULL;
	delete _georges; _georges = NULL;
	
	config().release();
	delete _configuration; _configuration = NULL;
}
