// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

// Qt Include
#include <QtCore/QReadWriteLock>

// Project include
#include "iplugin_manager.h"

namespace ExtensionSystem
{

class IPlugin;
class PluginSpec;

class PluginManager : public IPluginManager
{
	Q_OBJECT

public:
	PluginManager(QObject *parent = 0);
	~PluginManager();

	// Object pool operations
	virtual void addObject(QObject *obj);
	virtual void removeObject(QObject *obj);
	virtual QList<QObject *> allObjects() const;

	// Plugin operations
	virtual void loadPlugins();
	virtual QStringList getPluginPaths() const;
	virtual void setPluginPaths(const QStringList &paths);
	virtual QList<IPluginSpec *> plugins() const;
	QList<PluginSpec *> loadQueue();

	bool loadPlugin( const QString &plugin );
	bool unloadPlugin( ExtensionSystem::IPluginSpec *plugin );


	// Settings
	virtual void setSettings(QSettings *settings);
	virtual QSettings *settings() const;
	void readSettings();
	void writeSettings();

private:
	bool loadPluginSpec( const QString &plugin );
	void removePlugin( ExtensionSystem::IPluginSpec *plugin );
	void setPluginState(PluginSpec *spec, int destState);
	void readPluginPaths();
	bool loadQueue(PluginSpec *spec, QList<PluginSpec *> &queue, QList<PluginSpec *> &circularityCheckQueue);
	void stopAll();
	void deleteAll();

	mutable QReadWriteLock m_lock;

	QSettings *m_settings;
	QString m_extension;
	QList<PluginSpec *> m_pluginSpecs;
	QList<IPluginSpec *> m_ipluginSpecs;
	QStringList m_pluginPaths;
	QList<QObject *> m_allObjects;

}; // class PluginManager

} // namespace ExtensionSystem

#endif // PLUGINMANAGER_H
