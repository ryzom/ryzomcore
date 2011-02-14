/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

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

#ifndef IPLUGINMANAGER_H
#define IPLUGINMANAGER_H

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

	// Settings
	virtual void setSettings(QSettings *settings) = 0;
	virtual QSettings *settings() const = 0;

Q_SIGNALS:
	void objectAdded(QObject *obj);
	void aboutToRemoveObject(QObject *obj);

	void pluginsChanged();
};

}; // namespace ExtensionSystem

#endif // IPLUGINMANAGER_H
