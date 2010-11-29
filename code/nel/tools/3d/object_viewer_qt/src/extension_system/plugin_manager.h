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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

// Qt Include
#include <QtCore/QReadWriteLock>

// Project include
#include "iplugin_manager.h"

namespace NLQT
{

class IPlugin;

class CPluginManager : public IPluginManager
{
    Q_OBJECT

public:
    CPluginManager(QObject *parent = 0);
    ~CPluginManager();

    // Object pool operations
    virtual void addObject(QObject *obj);
    virtual void removeObject(QObject *obj);
    virtual QList<QObject *> allObjects() const;

    // Plugin operations
    virtual void loadPlugins();
    virtual QStringList getPluginPaths() const;
    virtual void setPluginPaths(const QStringList &paths);
    virtual QList<CPluginSpec *> plugins() const;

	CPluginSpec *pluginByName(const QString &name) const;

private:
	void setPluginState(CPluginSpec *spec, int destState);
	void readPluginPaths();
	void stopAll();

	mutable QReadWriteLock _lock;

    QList<CPluginSpec *> _pluginSpecs;
    QStringList _pluginPaths;
    QList<QObject *> _allObjects;
 
}; // class CPluginManager

} // namespace NLQT

#endif // PLUGINMANAGER_H
