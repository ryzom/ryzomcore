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

PluginManager::PluginManager(QObject *parent)
	:IPluginManager(parent),
	 m_settings(0),
	 m_extension("xml")
{
}

PluginManager::~PluginManager()
{
	writeSettings();
	stopAll();
	deleteAll();
	qDeleteAll(m_pluginSpecs);
}

void PluginManager::addObject(QObject *obj)
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
	nlinfo("addObject: %s", obj->objectName().toUtf8().constData());

	m_allObjects.append(obj);

	Q_EMIT objectAdded(obj);
}

void PluginManager::removeObject(QObject *obj)
{
	if (obj == 0)
	{
		nlwarning("trying to remove null object");
		return;
	}

	if (!m_allObjects.contains(obj))
	{
		nlinfo("object not in list: %s", obj->objectName().toUtf8().constData());
		return;
	}
	nlinfo("removeObject: %s", obj->objectName().toUtf8().constData());

	Q_EMIT aboutToRemoveObject(obj);
	QWriteLocker lock(&m_lock);
	m_allObjects.removeAll(obj);
}

QList<QObject *> PluginManager::allObjects() const
{
	return m_allObjects;
}

void PluginManager::loadPlugins()
{
	Q_FOREACH (PluginSpec *spec, m_pluginSpecs)
	setPluginState(spec, State::Resolved);

	QList<PluginSpec *> queue = loadQueue();

	Q_FOREACH (PluginSpec *spec, queue)
	setPluginState(spec, State::Loaded);

	Q_FOREACH (PluginSpec *spec, queue)
	setPluginState(spec, State::Initialized);

	QListIterator<PluginSpec *> it(queue);
	it.toBack();
	while (it.hasPrevious())
		setPluginState(it.previous(), State::Running);

	Q_EMIT pluginsChanged();
}

QStringList PluginManager::getPluginPaths() const
{
	return m_pluginPaths;
}

void PluginManager::setPluginPaths(const QStringList &paths)
{
	m_pluginPaths = paths;
	readPluginPaths();
	readSettings();
}

QList<IPluginSpec *> PluginManager::plugins() const
{
	return m_ipluginSpecs;
}

void PluginManager::setSettings(QSettings *settings)
{
	m_settings = settings;
}

QSettings *PluginManager::settings() const
{
	return m_settings;
}

void PluginManager::readSettings()
{
	if (m_settings)
	{
		QStringList blackList;
		m_settings->beginGroup("PluginManager");
		blackList = m_settings->value("BlackList").toStringList();
		m_settings->endGroup();
		Q_FOREACH (PluginSpec *spec, m_pluginSpecs)
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

void PluginManager::writeSettings()
{
	if (m_settings)
	{
		QStringList blackList;
		Q_FOREACH(PluginSpec *spec, m_pluginSpecs)
		{
			if (!spec->isEnabled())
				blackList.push_back(spec->fileName());
		}
		m_settings->beginGroup("PluginManager");
		m_settings->setValue("BlackList", blackList);
		m_settings->endGroup();
		m_settings->sync();
	}
}

void PluginManager::readPluginPaths()
{
	qDeleteAll(m_pluginSpecs);
	m_pluginSpecs.clear();
	m_ipluginSpecs.clear();

	QStringList pluginsList;
	QStringList searchPaths = m_pluginPaths;
	while (!searchPaths.isEmpty())
	{
		const QDir dir(searchPaths.takeFirst());
		const QFileInfoList files = dir.entryInfoList(QStringList() << QString("studio_plugin_*.%1").arg(m_extension), QDir::Files);
		Q_FOREACH (const QFileInfo &file, files)
		pluginsList << file.absoluteFilePath();
		const QFileInfoList dirs = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
		Q_FOREACH (const QFileInfo &subdir, dirs)
		searchPaths << subdir.absoluteFilePath();
	}

	Q_FOREACH (const QString &pluginFile, pluginsList)
	{
		PluginSpec *spec = new PluginSpec;
		spec->m_pluginManager = this;
		spec->setSpecFileName(pluginFile);
		m_pluginSpecs.append(spec);
		m_ipluginSpecs.append(spec);
	}

	Q_EMIT pluginsChanged();
}

void PluginManager::setPluginState(PluginSpec *spec, int destState)
{
	if (spec->hasError() || spec->state() != destState-1)
		return;

	// plugin in black list
	if (!spec->isEnabledStartup())
		return;

	switch (destState)
	{
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
	Q_FOREACH (const PluginSpec *depSpec, spec->dependencySpecs())
	{
		if (depSpec->state() != destState)
		{
			spec->m_hasError = true;
			spec->m_errorString = tr("Cannot load plugin because dependency failed to load: %1")
								  .arg(depSpec->name());
			return;
		}
	}
	switch (destState)
	{
	case State::Loaded:
		spec->loadLibrary();
		return;
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

QList<PluginSpec *> PluginManager::loadQueue()
{
	QList<PluginSpec *> queue;
	Q_FOREACH(PluginSpec *spec, m_pluginSpecs)
	{
		QList<PluginSpec *> circularityCheckQueue;
		loadQueue(spec, queue, circularityCheckQueue);
	}
	return queue;
}

bool PluginManager::loadQueue(PluginSpec *spec, QList<PluginSpec *> &queue,
							  QList<PluginSpec *> &circularityCheckQueue)
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
	Q_FOREACH (PluginSpec *depSpec, spec->dependencySpecs())
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

void PluginManager::stopAll()
{
	QList<PluginSpec *> queue = loadQueue();
	Q_FOREACH (PluginSpec *spec, queue)
	setPluginState(spec, State::Stopped);
}

void PluginManager::deleteAll()
{
	QList<PluginSpec *> queue = loadQueue();
	QListIterator<PluginSpec *> it(queue);
	it.toBack();
	while (it.hasPrevious())
	{
		setPluginState(it.previous(), State::Deleted);
	}
}

}; // namespace ExtensionSystem