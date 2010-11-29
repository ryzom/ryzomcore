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

#include "plugin_manager.h"

#include <QtCore/QDir>

#include <nel/misc/debug.h>

#include "plugin_spec.h"

namespace NLQT
{

CPluginManager::CPluginManager(QObject *parent):
	IPluginManager(parent)
{
}

CPluginManager::~CPluginManager()
{
	stopAll();
	qDeleteAll(_pluginSpecs);
}

void CPluginManager::addObject(QObject *obj)
{
	QWriteLocker lock(&_lock);
	if (obj == 0) 
	{
		nlwarning("trying to add null object");
        return;
	}
    if (_allObjects.contains(obj)) 
	{
		nlwarning("trying to add duplicate object");
		return;
    }
	nlinfo(QString("addObject:" + obj->objectName()).toStdString().c_str());
	
	_allObjects.append(obj);

    Q_EMIT objectAdded(obj);
}

void CPluginManager::removeObject(QObject *obj)
{
	if (obj == 0) 
	{
		nlwarning("trying to remove null object");
        return;
    }

    if (!_allObjects.contains(obj)) 
	{
		nlinfo(QString("object not in list:" + obj->objectName()).toStdString().c_str());
		return;
    }
	nlinfo(QString("removeObject:" + obj->objectName()).toStdString().c_str());
   
    Q_EMIT aboutToRemoveObject(obj);
	QWriteLocker lock(&_lock);
    _allObjects.removeAll(obj);
}

QList<QObject *> CPluginManager::allObjects() const
{
	return _allObjects;
}

void CPluginManager::loadPlugins()
{
	Q_FOREACH (CPluginSpec *spec, _pluginSpecs) 
		setPluginState(spec, State::Loaded);

	Q_FOREACH (CPluginSpec *spec, _pluginSpecs) 
        setPluginState(spec, State::Initialized);

	Q_FOREACH (CPluginSpec *spec, _pluginSpecs) 
		setPluginState(spec, State::Running);

    Q_EMIT pluginsChanged();
}

QStringList CPluginManager::getPluginPaths() const
{
	return _pluginPaths;
}

void CPluginManager::setPluginPaths(const QStringList &paths)
{
	_pluginPaths = paths;
	readPluginPaths();
}

QList<CPluginSpec *> CPluginManager::plugins() const
{
	return _pluginSpecs;
}

void CPluginManager::readPluginPaths()
{
	qDeleteAll(_pluginSpecs);
    _pluginSpecs.clear();

	QStringList pluginsList;
    QStringList searchPaths = _pluginPaths;
    while (!searchPaths.isEmpty()) 
	{
        const QDir dir(searchPaths.takeFirst());
#ifdef Q_OS_WIN
		const QFileInfoList files = dir.entryInfoList(QStringList() << QString("*.dll"), QDir::Files);
#elif defined(Q_OS_MAC)
		const QFileInfoList files = dir.entryInfoList(QStringList() << QString("*.dylib"), QDir::Files);
#else
		const QFileInfoList files = dir.entryInfoList(QStringList() << QString("*.so"), QDir::Files);
#endif
		Q_FOREACH (const QFileInfo &file, files)
		    pluginsList << file.absoluteFilePath();
		const QFileInfoList dirs = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
		Q_FOREACH (const QFileInfo &subdir, dirs)
		    searchPaths << subdir.absoluteFilePath();
    }
	
    Q_FOREACH (const QString &pluginFile, pluginsList) 
	{
        CPluginSpec *spec = new CPluginSpec;
		if (spec->setFileName(pluginFile))
			_pluginSpecs.append(spec);
		else
			delete spec;
    }

	 Q_EMIT pluginsChanged();
}

CPluginSpec *CPluginManager::pluginByName(const QString &name) const
{
    Q_FOREACH (CPluginSpec *spec, _pluginSpecs)
        if (spec->name() == name)
            return spec;
    return 0;
}

void CPluginManager::setPluginState(CPluginSpec *spec, int destState)
{
	if (spec->hasError())
		return;
    if (destState == State::Running) 
	{
		spec->initializeExtensions();
        return;
	} 
	else if (destState == State::Deleted) 
	{
        spec->kill();
        return;
    }
 
    if (destState == State::Loaded)
        spec->loadLibrary();
    else if (destState == State::Initialized)
        spec->initializePlugin();
    else if (destState == State::Stopped)
        spec->stop();
}

void CPluginManager::stopAll()
{
    Q_FOREACH (CPluginSpec *spec,  _pluginSpecs) 
		setPluginState(spec, State::Stopped);

	Q_FOREACH (CPluginSpec *spec,  _pluginSpecs) 
		setPluginState(spec, State::Deleted);
}

}; // namespace NLQT