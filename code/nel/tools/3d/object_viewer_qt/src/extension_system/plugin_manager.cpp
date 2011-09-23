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

#include "plugin_manager.h"
#include "plugin_spec.h"

#include <QtCore/QDir>

#include <nel/misc/debug.h>

namespace ExtensionSystem
{

CPluginManager::CPluginManager(QObject *parent)
	:IPluginManager(parent),
	 m_settings(0)
{
}

CPluginManager::~CPluginManager()
{
	writeSettings();
	stopAll();
	deleteAll();
	qDeleteAll(m_pluginSpecs);
}

void CPluginManager::addObject(QObject *obj)
{
	QWriteLocker lock(&m_lock);
	if (obj == 0)
	{
		nlwarning("trying to add null object");
		return;
	}
	if (m_allObjects.contains(obj))
	{
		nlwarning("trying to add duplicate object");
		return;
	}
	nlinfo(QString("addObject: " + obj->objectName()).toStdString().c_str());

	m_allObjects.append(obj);

	Q_EMIT objectAdded(obj);
}

void CPluginManager::removeObject(QObject *obj)
{
	if (obj == 0)
	{
		nlwarning("trying to remove null object");
		return;
	}

	if (!m_allObjects.contains(obj))
	{
		nlinfo(QString("object not in list: " + obj->objectName()).toStdString().c_str());
		return;
	}
	nlinfo(QString("removeObject: " + obj->objectName()).toStdString().c_str());

	Q_EMIT aboutToRemoveObject(obj);
	QWriteLocker lock(&m_lock);
	m_allObjects.removeAll(obj);
}

QList<QObject *> CPluginManager::allObjects() const
{
	return m_allObjects;
}

void CPluginManager::loadPlugins()
{
	Q_FOREACH (CPluginSpec *spec, m_pluginSpecs)
	setPluginState(spec, State::Loaded);

	Q_FOREACH (CPluginSpec *spec, m_pluginSpecs)
	setPluginState(spec, State::Resolved);

	QList<CPluginSpec *> queue = loadQueue();

	Q_FOREACH (CPluginSpec *spec, queue)
	setPluginState(spec, State::Initialized);

	QListIterator<CPluginSpec *> it(queue);
	it.toBack();
	while (it.hasPrevious())
		setPluginState(it.previous(), State::Running);

	Q_EMIT pluginsChanged();
}

QStringList CPluginManager::getPluginPaths() const
{
	return m_pluginPaths;
}

void CPluginManager::setPluginPaths(const QStringList &paths)
{
	m_pluginPaths = paths;
	readPluginPaths();
	readSettings();
}

QList<IPluginSpec *> CPluginManager::plugins() const
{
	return m_ipluginSpecs;
}

void CPluginManager::setSettings(QSettings *settings)
{
	m_settings = settings;
}

QSettings *CPluginManager::settings() const
{
	return m_settings;
}

void CPluginManager::readSettings()
{
	if (m_settings)
	{
		QStringList blackList;
		m_settings->beginGroup("PluginManager");
		blackList = m_settings->value("BlackList").toStringList();
		m_settings->endGroup();
		Q_FOREACH (CPluginSpec *spec, m_pluginSpecs)
		{
			QString pluginName = spec->fileName();

			if (blackList.contains(pluginName))
			{
				spec->setEnabled(false);
				spec->setEnabledStartup(false);
			}
		}
	}
}

void CPluginManager::writeSettings()
{
	if (m_settings)
	{
		QStringList blackList;
		Q_FOREACH(CPluginSpec *spec, m_pluginSpecs)
		{
			nlinfo(spec->fileName().toStdString().c_str());
			if (!spec->isEnabled())
				blackList.push_back(spec->fileName());
		}
		m_settings->beginGroup("PluginManager");
		m_settings->setValue("BlackList", blackList);
		m_settings->endGroup();
		m_settings->sync();
	}
}

void CPluginManager::readPluginPaths()
{
	qDeleteAll(m_pluginSpecs);
	m_pluginSpecs.clear();
	m_ipluginSpecs.clear();

	QStringList pluginsList;
	QStringList searchPaths = m_pluginPaths;
	while (!searchPaths.isEmpty())
	{
		const QDir dir(searchPaths.takeFirst());
#ifdef Q_OS_WIN
		const QFileInfoList files = dir.entryInfoList(QStringList() << QString("ovqt_plugin_*.dll"), QDir::Files);
#else
		const QFileInfoList files = dir.entryInfoList(QStringList() << QString("libovqt_plugin_*.so"), QDir::Files);
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
		spec->setFileName(pluginFile);
		spec->m_pluginManager = this;
		m_pluginSpecs.append(spec);
		m_ipluginSpecs.append(spec);
	}

	Q_EMIT pluginsChanged();
}

void CPluginManager::setPluginState(CPluginSpec *spec, int destState)
{
	if (spec->hasError() || spec->state() != destState-1)
		return;

	// plugin in black list
	if (!spec->isEnabledStartup())
		return;

	switch (destState)
	{
	case State::Loaded:
		spec->loadLibrary();
		return;
	case State::Resolved:
		spec->resolveDependencies(m_pluginSpecs);
		return;
	case State::Running:
		spec->initializeExtensions();
		return;
	case State::Deleted:
		spec->kill();
		return;
	default:
		break;
	}
	Q_FOREACH (const CPluginSpec *depSpec, spec->dependencySpecs())
	{
		if (depSpec->state() != destState)
		{
			spec->m_hasError = true;
			spec->m_errorString = tr("Cannot initializing plugin because dependency failed to load: %1\nReason: %2")
								  .arg(depSpec->name()).arg(depSpec->errorString());
			return;
		}
	}
	switch (destState)
	{
	case State::Initialized:
		spec->initializePlugin();
		break;
	case State::Stopped:
		spec->stop();
		break;
	default:
		break;
	}
}

QList<CPluginSpec *> CPluginManager::loadQueue()
{
	QList<CPluginSpec *> queue;
	Q_FOREACH(CPluginSpec *spec, m_pluginSpecs)
	{
		QList<CPluginSpec *> circularityCheckQueue;
		loadQueue(spec, queue, circularityCheckQueue);
	}
	return queue;
}

bool CPluginManager::loadQueue(CPluginSpec *spec, QList<CPluginSpec *> &queue,
							   QList<CPluginSpec *> &circularityCheckQueue)
{
	if (queue.contains(spec))
		return true;
	// check for circular dependencies
	if (circularityCheckQueue.contains(spec))
	{
		spec->m_hasError = true;
		spec->m_errorString = tr("Circular dependency detected:\n");
		int index = circularityCheckQueue.indexOf(spec);
		for (int i = index; i < circularityCheckQueue.size(); ++i)
		{
			spec->m_errorString.append(tr("%1(%2) depends on\n")
									   .arg(circularityCheckQueue.at(i)->name()).arg(circularityCheckQueue.at(i)->version()));
		}
		spec->m_errorString.append(tr("%1(%2)").arg(spec->name()).arg(spec->version()));
		return false;
	}
	circularityCheckQueue.append(spec);
	// check if we have the dependencies
	if (spec->state() == State::Invalid || spec->state() == State::Read)
	{
		queue.append(spec);
		return false;
	}

	// add dependencies
	Q_FOREACH (CPluginSpec *depSpec, spec->dependencySpecs())
	{
		if (!loadQueue(depSpec, queue, circularityCheckQueue))
		{
			spec->m_hasError = true;
			spec->m_errorString =
				tr("Cannot load plugin because dependency failed to load: %1(%2)\nReason: %3")
				.arg(depSpec->name()).arg(depSpec->version()).arg(depSpec->errorString());
			return false;
		}
	}
	// add self
	queue.append(spec);
	return true;
}

void CPluginManager::stopAll()
{
	QList<CPluginSpec *> queue = loadQueue();
	Q_FOREACH (CPluginSpec *spec, queue)
	setPluginState(spec, State::Stopped);
}

void CPluginManager::deleteAll()
{
	QList<CPluginSpec *> queue = loadQueue();
	QListIterator<CPluginSpec *> it(queue);
	it.toBack();
	while (it.hasPrevious())
	{
		setPluginState(it.previous(), State::Deleted);
	}
}

}; // namespace ExtensionSystem