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

#include <nel/misc/debug.h>

NLQT::CConfiguration      *Modules::_configuration = NULL;
NLQT::IObjectViewer       *Modules::_objViewerInterface = NULL;
NLQT::CMainWindow         *Modules::_mainWindow = NULL;

void Modules::init(NLMISC::IProgressCallback *cb)
{
	if (loadPlugin())
	{
	_objViewerInterface->setNelContext(NLMISC::INelContext::getInstance());
	}

	if (_configuration == NULL) _configuration = new NLQT::CConfiguration;
	_configuration->setProgressCallback(cb);
	config().init();
	_configuration->setProgressCallback(0);

	if (_mainWindow == NULL) _mainWindow = new NLQT::CMainWindow;
}

void Modules::release()
{
	delete _mainWindow; _mainWindow = NULL;

	config().release();
	delete _configuration; _configuration = NULL;
}

bool Modules::loadPlugin()
{
#if defined(Q_OS_WIN)
	QString pluginPath     = qApp->applicationDirPath();
	QString pluginFilename = "object_viewer_widget_qt.dll";
#elif defined(Q_OS_MAC)
	QString pluginPath     = qApp->applicationDirPath() + "/../PlugIns/";
	QString pluginFilename = "libobject_viewer_widget_qt.so";
#else // LINUX
	QString pluginPath     = qApp->applicationDirPath();
	QString pluginFilename = "libobject_viewer_widget_qt.so";
#endif

	// if(!QFile::exists(pluginPath + pluginFilename))
	// {
	// 	nlwarning("Cannot find %s in %s, fallback to working dir", 
	// 		pluginFilename.toUtf8().constData(), pluginPath.toUtf8().constData());
	// 
	// 	pluginPath = "";
	// 
	// 	Q_FOREACH (QString path, qApp->libraryPaths())
	// 		nlwarning("libraryPaths %s", path.toUtf8().constData());
	// }

	QDir pluginsDir(pluginPath);
	QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(pluginFilename));

	QObject *plugin = pluginLoader.instance();
	if (plugin) 
	{
		_objViewerInterface = qobject_cast<NLQT::IObjectViewer *>(plugin);
		if (_objViewerInterface)
		{
			nlinfo("Loaded %s", 
				pluginsDir.absoluteFilePath(pluginFilename).toUtf8().constData());
			return true;
		}
		else
		{
			nlwarning("Loaded %s, but cannot cast to NLQT::IObjectViewer*", 
				pluginFilename.toUtf8().constData());
		}
	}
	else
	{
		nlwarning("Cannot get plugin instance for %s (searched in %s)", 
			pluginFilename.toUtf8().constData(), 
			(qApp->applicationDirPath() + pluginPath).toUtf8().constData());
	}

	return false;
}
