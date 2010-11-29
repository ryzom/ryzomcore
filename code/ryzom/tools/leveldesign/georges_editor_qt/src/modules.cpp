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

#include <QApplication>
#include <QPluginLoader>
#include <QDir>
#include <QString>

NLQT::CConfiguration      *Modules::_configuration = NULL;
NLQT::CObjectViewerWidget *Modules::_objectViewerWidget = NULL;
NLQT::IObjectViewer       *Modules::_objViewerInterface = NULL;
NLQT::CMainWindow         *Modules::_mainWindow = NULL;
	
void Modules::init()
{
	loadPlugin();

	if (_configuration == NULL) _configuration = new NLQT::CConfiguration;
	config().init();
	
	if (_objectViewerWidget == NULL) _objectViewerWidget = new NLQT::CObjectViewerWidget;
	if (_mainWindow == NULL) _mainWindow = new NLQT::CMainWindow;
}

void Modules::release()
{
	delete _mainWindow; _mainWindow = NULL;
	//delete _objectViewerWidget; _objectViewerWidget = NULL;
	
	config().release();
	delete _configuration; _configuration = NULL;
}

bool Modules::loadPlugin()
 {
     QDir pluginsDir(qApp->applicationDirPath());
 /*#if defined(Q_OS_WIN)
     if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
         pluginsDir.cdUp();
 #elif defined(Q_OS_MAC)
     if (pluginsDir.dirName() == "MacOS") {
         pluginsDir.cdUp();
         pluginsDir.cdUp();
         pluginsDir.cdUp();
     }
 #endif*/
     //pluginsDir.cd("plugins");
     //Q_FOREACH (QString fileName, pluginsDir.entryList(QDir::Files)) {
         QPluginLoader pluginLoader(pluginsDir.absoluteFilePath("object_viewer_widget_qt.dll"));
         QObject *plugin = pluginLoader.instance();
         if (plugin) {
			 _objViewerInterface = qobject_cast<NLQT::IObjectViewer *>(plugin);
             if (_objViewerInterface)
                 return true;
         }
     //}

     return false;
 }