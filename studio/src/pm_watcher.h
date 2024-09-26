// Ryzom Core - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#ifndef PM_WATCHER_H
#define PM_WATCHER_H

#include <QObject>

namespace ExtensionSystem
{
	class IPluginManager;
}

class SplashScreen;
class PluginManager;

class PluginManagerWatcher : public QObject
{
	Q_OBJECT
public:

	PluginManagerWatcher(){
		sp = NULL;
		pm = NULL;
		pluginCount = 0;
	}

	~PluginManagerWatcher(){
		sp = NULL;
		pm = NULL;
	}

	void setSplashScreen( SplashScreen *s ){ sp = s; }
	void setPluginManager( ExtensionSystem::IPluginManager *m ){ pm = m; }

	void connect();
	void disconnect();

private Q_SLOTS:
	void onPluginLoading( const char *plugin );
	void onPluginInitializing( const char *plugin );
	void onPluginStarting( const char *plugin );

	void onPluginsLoaded();
	void onPluginsInitialized();
	void onPluginsStarted();

	void onPluginCount( int count );

private:
	SplashScreen *sp;
	ExtensionSystem::IPluginManager *pm;
	int pluginCount;
};


#endif

