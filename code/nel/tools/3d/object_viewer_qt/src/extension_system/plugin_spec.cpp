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
#include "plugin_spec.h"

#include <QtCore/QList>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QPluginLoader>
#include <QtCore/QCoreApplication>

#include "nel/misc/app_context.h"

#include "iplugin.h"
#include "iplugin_manager.h"

namespace ExtensionSystem
{

CPluginSpec::CPluginSpec()
	: m_location(""),
	  m_filePath(""),
	  m_fileName(""),
	  m_name(""),
	  m_version(""),
	  m_vendor(""),
	  m_description(""),
	  m_state(State::Invalid),
	  m_hasError(false),
	  m_errorString(""),
	  m_plugin(0),
	  m_pluginManager(0)
{
}

QString CPluginSpec::name() const
{
	return m_name;
}

QString CPluginSpec::version() const
{
	return m_version;
}

QString CPluginSpec::vendor() const
{
	return m_vendor;
}

QString CPluginSpec::description() const
{
	return m_description;
}

QString CPluginSpec::location() const
{
	return m_location;
}

QString CPluginSpec::filePath() const
{
	return m_filePath;
}

QString CPluginSpec::fileName() const
{
	return m_fileName;
}

IPlugin* CPluginSpec::plugin() const
{
	return m_plugin;
}

int CPluginSpec::getState() const
{
	return m_state;
}

bool CPluginSpec::hasError() const
{
	return m_hasError;
}

QString CPluginSpec::errorString() const
{
	return m_errorString;
}

QList<CPluginSpec *> CPluginSpec::dependencySpecs() const
{
	return m_dependencySpecs;
}

bool CPluginSpec::setFileName(const QString &fileName)
{
	QFile file(fileName);
	if (!file.exists())
		return reportError(QCoreApplication::translate("CPluginSpec", "File does not exist: %1").arg(file.fileName()));
	if (!file.open(QIODevice::ReadOnly))
		return reportError(QCoreApplication::translate("CPluginSpec", "Could not open file for read: %1").arg(file.fileName()));

	QFileInfo fileInfo(file);
	m_location = fileInfo.absolutePath();
	m_filePath = fileInfo.absoluteFilePath();
	m_fileName = fileInfo.fileName();

	m_state = State::Read;
	return true;
}

bool CPluginSpec::loadLibrary()
{
	if (m_hasError)
		return false;
	if (m_state != State::Read)
	{
		if (m_state == State::Loaded)
			return true;
		return reportError(QCoreApplication::translate("CPluginSpec", "Loading the library failed because state != Resolved"));
	}

	QPluginLoader loader(m_filePath);
	if (!loader.load())
		return reportError(loader.errorString());

	IPlugin *pluginObject = qobject_cast<IPlugin *>(loader.instance());
	if (!pluginObject)
	{
		loader.unload();
		return reportError(QCoreApplication::translate("CPluginSpec", "Plugin is not valid (does not derive from IPlugin)"));
	}

	pluginObject->setNelContext(&NLMISC::INelContext::getInstance());

	m_name = pluginObject->name();
	m_version = pluginObject->version();
	m_vendor = pluginObject->vendor();
	m_description = pluginObject->description();

	m_state = State::Loaded;
	m_plugin = pluginObject;
	return true;
}

bool CPluginSpec::resolveDependencies(const QList<CPluginSpec *> &specs)
{
	if (m_hasError)
		return false;
	if (m_state != State::Loaded)
	{
		m_errorString = QCoreApplication::translate("CPluginSpec", "Resolving dependencies failed because state != Read");
		m_hasError = true;
		return false;
	}
	QList<CPluginSpec *> resolvedDependencies;
	QStringList dependencies = m_plugin->dependencies();
	Q_FOREACH(const QString &dependency, dependencies)
	{
		CPluginSpec *found = 0;

		Q_FOREACH(CPluginSpec *spec, specs)
		{
			if (QString::compare(dependency, spec->name(), Qt::CaseInsensitive) == 0)
			{
				found = spec;
				break;
			}
		}
		if (!found)
		{
			m_hasError = true;
			if (!m_errorString.isEmpty())
				m_errorString.append(QLatin1Char('\n'));
			m_errorString.append(QCoreApplication::translate("CPluginSpec", "Could not resolve dependency '%1'")
								 .arg(dependency));
			continue;
		}
		resolvedDependencies.append(found);
	}
	if (m_hasError)
		return false;

	m_dependencySpecs = resolvedDependencies;

	m_state = State::Resolved;

	return true;
}

bool CPluginSpec::initializePlugin()
{
	if (m_hasError)
		return false;
	if (m_state != State::Resolved)
	{
		if (m_state == State::Initialized)
			return true;
		return reportError(QCoreApplication::translate("CPluginSpec", "Initializing the plugin failed because state != Resolved)"));
	}
	if (!m_plugin)
		return reportError(QCoreApplication::translate("CPluginSpec", "Internal error: have no plugin instance to initialize"));

	QString err;
	if (!m_plugin->initialize(m_pluginManager, &err))
		return reportError(QCoreApplication::translate("CPluginSpec", "Plugin initialization failed: %1").arg(err));

	m_state = State::Initialized;
	return true;
}

bool CPluginSpec::initializeExtensions()
{
	if (m_hasError)
		return false;
	if (m_state != State::Initialized)
	{
		if (m_state == State::Running)
			return true;
		return reportError(QCoreApplication::translate("CPluginSpec", "Cannot perform extensionsInitialized because state != Initialized"));
	}
	if (!m_plugin)
		return reportError(QCoreApplication::translate("CPluginSpec", "Internal error: have no plugin instance to perform extensionsInitialized"));

	m_plugin->extensionsInitialized();
	m_state = State::Running;
	return true;
}

void CPluginSpec::stop()
{
	if (!m_plugin)
		return;
	m_plugin->shutdown();
	m_state = State::Stopped;
}

void CPluginSpec::kill()
{
	if (!m_plugin)
		return;
	delete m_plugin;
	m_plugin = 0;
	m_state = State::Deleted;
}

bool CPluginSpec::reportError(const QString &err)
{
	m_errorString = err;
	m_hasError = true;
	return false;
}

} // namespace NLQT