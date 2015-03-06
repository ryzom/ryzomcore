// Ryzom Core - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "pm_watcher.h"
#include "extension_system/iplugin_manager.h"
#include "splash_screen.h"

namespace
{
	enum Progress
	{
		PLUGINS_LOADED = 10,
		PLUGINS_INITIALIZED = 90,
		PLUGINS_STARTED = 100
	};
}

void PluginManagerWatcher::connect()
{
	QObject::connect( pm, SIGNAL( pluginLoading( const char * ) ), this, SLOT( onPluginLoading( const char * ) ) );
	QObject::connect( pm, SIGNAL( pluginInitializing( const char * ) ), this, SLOT( onPluginInitializing( const char * ) ) );
	QObject::connect( pm, SIGNAL( pluginStarting( const char * ) ), this, SLOT( onPluginStarting( const char * ) ) );
	QObject::connect( pm, SIGNAL( pluginsLoaded() ), this, SLOT( onPluginsLoaded() ) );
	QObject::connect( pm, SIGNAL( pluginsInitialized() ), this, SLOT( onPluginsInitialized() ) );
	QObject::connect( pm, SIGNAL( pluginsStarted() ), this, SLOT( onPluginsStarted() ) );
	QObject::connect( pm, SIGNAL( pluginCount( int ) ), this, SLOT( onPluginCount( int ) ) );
}

void PluginManagerWatcher::disconnect()
{
	QObject::disconnect( pm, SIGNAL( pluginLoading( const char * ) ), this, SLOT( onPluginLoading( const char * ) ) );
	QObject::disconnect( pm, SIGNAL( pluginInitializing( const char * ) ), this, SLOT( onPluginInitializing( const char * ) ) );
	QObject::disconnect( pm, SIGNAL( pluginStarting( const char * ) ), this, SLOT( onPluginStarting( const char * ) ) );
	QObject::disconnect( pm, SIGNAL( pluginsLoaded() ), this, SLOT( onPluginsLoaded() ) );
	QObject::disconnect( pm, SIGNAL( pluginsInitialized() ), this, SLOT( onPluginsInitialized() ) );
	QObject::disconnect( pm, SIGNAL( pluginsStarted() ), this, SLOT( onPluginsStarted() ) );
	QObject::disconnect( pm, SIGNAL( pluginCount( int ) ), this, SLOT( onPluginCount( int ) ) );
}

void PluginManagerWatcher::onPluginLoading( const char *plugin )
{
	QString s = "Loading plugin ";
	s += plugin;
	s += "...";
	sp->setText( s );

	sp->advanceProgress( PLUGINS_LOADED / pluginCount );
}

void PluginManagerWatcher::onPluginInitializing( const char *plugin )
{
	QString s = "Initializing plugin ";
	s += plugin;
	s += "...";
	sp->setText( s );

	sp->advanceProgress( ( PLUGINS_INITIALIZED - PLUGINS_LOADED ) / pluginCount );
}

void PluginManagerWatcher::onPluginStarting( const char *plugin )
{
	QString s = "Starting plugin ";
	s += plugin;
	s += "...";
	sp->setText( s );

	sp->advanceProgress( ( PLUGINS_STARTED - PLUGINS_INITIALIZED ) / pluginCount );
}


void PluginManagerWatcher::onPluginsLoaded()
{
	sp->setProgress( PLUGINS_LOADED );
}

void PluginManagerWatcher::onPluginsInitialized()
{
	sp->setProgress( PLUGINS_INITIALIZED );
}

void PluginManagerWatcher::onPluginsStarted()
{
	sp->setProgress( PLUGINS_STARTED );
}

void PluginManagerWatcher::onPluginCount( int count )
{
	pluginCount = count;
}