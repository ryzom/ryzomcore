// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
// Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#ifndef IPLUGINMANAGER_H
#define IPLUGINMANAGER_H

#include "iplugin_spec.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QSettings>

namespace ExtensionSystem
{
class IPluginSpec;

/**
@interface IPluginManager
@brief Interface for plugin system that manages the plugins, their life cycle and their registered objects.
@details The plugin manager is used for the following tasks:
- Manage plugins and their state
- Manipulate a 'common object pool'
*/
class IPluginManager: public QObject
{
	Q_OBJECT
public:
	IPluginManager(QObject *parent = 0): QObject(parent) {}
	virtual ~IPluginManager() {}

	// Object pool operations
	virtual void addObject(QObject *obj) = 0;
	virtual void removeObject(QObject *obj) = 0;
	virtual QList<QObject *> allObjects() const = 0;

	// Plugin operations
	virtual void loadPlugins() = 0;
	virtual QStringList getPluginPaths() const = 0;
	virtual void setPluginPaths(const QStringList &paths) = 0;
	virtual QList<ExtensionSystem::IPluginSpec *> plugins() const = 0;

	virtual bool loadPlugin( const QString &plugin ) = 0;
	virtual bool unloadPlugin( ExtensionSystem::IPluginSpec *plugin ) = 0;

	// Settings
	virtual void setSettings(QSettings *settings) = 0;
	virtual QSettings *settings() const = 0;

	// Auxiliary operations

	/// Retrieve all objects of a given type from the object pool.
	template <typename T>
	QList<T *> getObjects() const
	{
		QList<QObject *> all = allObjects();
		QList<T *> objects;
		Q_FOREACH(QObject *obj, all)
		{
			T *tObj = qobject_cast<T *>(obj);
			if (tObj)
				objects.append(tObj);
		}
		return objects;
	}

	/// Retrieve the object of a given type from the object pool.
	template <typename T>
	T *getObject() const
	{
		QList<QObject *> all = allObjects();
		T *result = 0;
		Q_FOREACH(QObject *obj, all)
		{
			T *tObj = qobject_cast<T *>(obj);
			if (tObj)
			{
				result = tObj;
				break;
			}
		}
		return result;
	}

	QObject *objectByName(const QString &name) const
	{
		QList<QObject *> all = allObjects();
		QObject *result = 0;
		Q_FOREACH(QObject *qobj, all)
		{
			if (qobj->objectName() == name)
			{
				result = qobj;
				break;
			}
		}
		return result;
	}

	ExtensionSystem::IPluginSpec *pluginByName(const QString &name) const
	{
		QList<ExtensionSystem::IPluginSpec *> all = plugins();
		ExtensionSystem::IPluginSpec *result = 0;
		Q_FOREACH (ExtensionSystem::IPluginSpec *spec, all)
		{
			if (spec->name() == name)
			{
				result = spec;
				break;
			}
		}
		return result;
	}

Q_SIGNALS:
	/// Signal that \a obj has been added to the object pool.
	void objectAdded(QObject *obj);

	/// Signal that \a obj will be removed from the object pool.
	void aboutToRemoveObject(QObject *obj);

	/// Signal that the list of available plugins has changed.
	void pluginsChanged();

	void pluginCount( int count );

	void pluginLoading( const char *plugin );
	void pluginInitializing( const char *plugin );
	void pluginStarting( const char *plugin );

	void pluginsLoaded();
	void pluginsInitialized();
	void pluginsStarted();
};

}; // namespace ExtensionSystem

#endif // IPLUGINMANAGER_H
