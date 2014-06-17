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

// Project includes
#include "plugin_spec.h"
#include "iplugin.h"
#include "iplugin_manager.h"

#include "nel/misc/app_context.h"
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QList>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QPluginLoader>
#include <QtCore/QCoreApplication>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_OVQT_CONFIG_H
#include "ovqt_config.h"
#endif

namespace ExtensionSystem
{
const char *const PLUGIN_SPEC_NAME = "name";
const char *const PLUGIN_SPEC_VENDOR = "vendor";
const char *const PLUGIN_SPEC_VERSION = "version";
const char *const PLUGIN_SPEC_LIBRARY_NAME = "library-name";
const char *const PLUGIN_SPEC_DESCRIPTION = "description";
const char *const PLUGIN_SPEC_DEPENDENCIES = "dependencies";
const char *const PLUGIN_SPEC_DEPENDENCY = "dependency";
const char *const PLUGIN_SPEC_DEPENDENCY_NAME = "plugin-name";
const char *const PLUGIN_SPEC_DEPENDENCY_VERSION = "version";

PluginSpec::PluginSpec()
	: m_location(""),
	  m_filePath(""),
	  m_fileName(""),
	  m_name(""),
	  m_version(""),
	  m_vendor(""),
	  m_description(""),
	  m_nameSpecFile(""),
	  m_suffix(""),
	  m_state(State::Invalid),
	  m_enabled(true),
	  m_enabledStartup(true),
	  m_hasError(false),
	  m_errorString(""),
	  m_plugin(0),
	  m_pluginManager(0)
{
// Compilation mode specific suffixes
#ifdef NL_OS_WINDOWS
#	if defined(NL_DEBUG)
	m_suffix = "_d.dll";
#	elif defined(NL_RELEASE)
	m_suffix = "_r.dll";
#	else
#		error "Unknown compilation mode, can't build suffix"
#	endif
#elif defined (NL_OS_UNIX)
	m_prefix = "lib";
	m_suffix = ".so";
#else
#	error "You must define the lib suffix for your platform"
#endif
	loader = NULL;
}

PluginSpec::~PluginSpec()
{
	delete loader;
	loader = NULL;
}

QString PluginSpec::name() const
{
	return m_name;
}

QString PluginSpec::version() const
{
	return m_version;
}

QString PluginSpec::vendor() const
{
	return m_vendor;
}

QString PluginSpec::description() const
{
	return m_description;
}

QString PluginSpec::location() const
{
	return m_location;
}

QString PluginSpec::filePath() const
{
	return m_filePath;
}

QString PluginSpec::fileName() const
{
	return m_fileName;
}

IPlugin *PluginSpec::plugin() const
{
	return m_plugin;
}

int PluginSpec::state() const
{
	return m_state;
}

bool PluginSpec::hasError() const
{
	return m_hasError;
}

QString PluginSpec::errorString() const
{
	return m_errorString;
}

QList<PluginSpec *> PluginSpec::dependencySpecs() const
{
	return m_dependencySpecs;
}

bool PluginSpec::setFileName(const QString &fileName)
{
	m_fileName = m_prefix + fileName + m_suffix;
	m_filePath = m_location + "/" + m_fileName;

	QFile file;
	file.setFileName(m_filePath);

	bool exists = file.exists();

#ifdef NL_OS_UNIX

#ifdef PLUGINS_DIR
	if (!exists)
	{
		// if plugin can't be found in the same directory as spec file,
		// looks for it in PLUGINS_DIR
		m_filePath = QString("%1/%2").arg(PLUGINS_DIR).arg(m_fileName);

		file.setFileName(m_filePath);

		exists = file.exists();
	}
#endif

#ifdef NL_LIB_PREFIX
	if (!exists)
	{
		// if plugin can't be found in the same directory as spec file or PLUGINS_DIR,
		// looks for it in NL_LIB_PREFIX
		m_filePath = QString("%1/%2").arg(NL_LIB_PREFIX).arg(m_fileName);

		file.setFileName(m_filePath);

		exists = file.exists();
	}
#endif

#endif

	nlinfo(m_filePath.toUtf8().constData());

	if (!exists)
		return reportError(QCoreApplication::translate("PluginSpec", "File does not exist: %1").arg(file.fileName()));
	if (!file.open(QIODevice::ReadOnly))
		return reportError(QCoreApplication::translate("PluginSpec", "Could not open file for read: %1").arg(file.fileName()));
	return true;
}

bool PluginSpec::setSpecFileName(const QString &specFileName)
{
	m_nameSpecFile = specFileName;

	QFile file(specFileName);
	if (!file.exists())
		return reportError(QCoreApplication::translate("PluginSpec", "Spec file does not exist: %1").arg(file.fileName()));

	QFileInfo fileInfo(file);
	m_location = fileInfo.absolutePath();
	readSpec();
	return true;
}

bool PluginSpec::readSpec()
{
	QFile file(m_nameSpecFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return reportError(QCoreApplication::translate("PluginSpec", "Could not open spec file for read: %1").arg(file.fileName()));

	QXmlStreamReader reader(&file);
	while (!reader.atEnd())
	{
		if (reader.isStartElement())
			parseSpec(reader);
		reader.readNext();
	}
	if (reader.hasError())
		return reportError(QCoreApplication::translate("PluginSpec", "Error parsing file %1: %2, at line %3, column %4")
						   .arg(file.fileName())
						   .arg(reader.errorString())
						   .arg(reader.lineNumber())
						   .arg(reader.columnNumber()));
	m_state = State::Read;
	return true;
}

void PluginSpec::parseSpec(QXmlStreamReader &reader)
{
	QString elemName = reader.name().toString();
	reader.readNext();
	if (reader.isCharacters())
	{
		QString elemText = reader.text().toString();
		if (elemName == PLUGIN_SPEC_LIBRARY_NAME)
			setFileName(elemText);
		if (elemName == PLUGIN_SPEC_NAME)
			m_name = elemText;
		if (elemName == PLUGIN_SPEC_VERSION)
			m_version = elemText;
		if (elemName == PLUGIN_SPEC_VENDOR)
			m_vendor = elemText;
		if (elemName == PLUGIN_SPEC_DESCRIPTION)
			m_description = elemText;
		if (elemName == PLUGIN_SPEC_DEPENDENCIES)
			parseDependency(reader);
	}
}

void PluginSpec::parseDependency(QXmlStreamReader &reader)
{
	QString elemName;
	while (!reader.atEnd() && (elemName != PLUGIN_SPEC_DEPENDENCIES))
	{
		reader.readNext();
		elemName = reader.name().toString();
		if (reader.isStartElement() && (elemName == PLUGIN_SPEC_DEPENDENCY))
		{
			// Read name dependency plugin
			QString dependencyName = reader.attributes().value(PLUGIN_SPEC_DEPENDENCY_NAME).toString();
			if (dependencyName.isEmpty())
			{
				reader.raiseError(QCoreApplication::translate("CPluginSpec", "'%1' misses attribute '%2'")
								  .arg(PLUGIN_SPEC_DEPENDENCY)
								  .arg(PLUGIN_SPEC_DEPENDENCY_NAME));
				return;
			}
			// TODO: Read version dependency plugin
			QString dependencyVersion = reader.attributes().value(PLUGIN_SPEC_DEPENDENCY_VERSION).toString();

			m_dependencies.push_back(dependencyName);
		}
	}
}

void PluginSpec::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

bool PluginSpec::isEnabled() const
{
	return m_enabled;
}

bool PluginSpec::loadLibrary()
{
	nlassert( loader == NULL );

	if (m_hasError)
		return false;
	if (m_state != State::Resolved)
	{
		if (m_state == State::Loaded)
			return true;
		return reportError(QCoreApplication::translate("PluginSpec", "Loading the library failed because state != Resolved"));
	}

	loader = new QPluginLoader( m_filePath );
	if (!loader->load())
		return reportError(loader->errorString());

	IPlugin *pluginObject = qobject_cast<IPlugin *>(loader->instance());
	if (!pluginObject)
	{
		loader->unload();
		delete loader;
		loader = NULL;
		return reportError(QCoreApplication::translate("PluginSpec", "Plugin is not valid (does not derive from IPlugin)"));
	}

	pluginObject->setNelContext(&NLMISC::INelContext::getInstance());

	m_state = State::Loaded;
	m_plugin = pluginObject;
	return true;
}

bool PluginSpec::resolveDependencies(const QList<PluginSpec *> &specs)
{
	if (m_hasError)
		return false;
	if (m_state != State::Read)
	{
		m_errorString = QCoreApplication::translate("PluginSpec", "Resolving dependencies failed because state != Read");
		m_hasError = true;
		return false;
	}
	QList<PluginSpec *> resolvedDependencies;
	Q_FOREACH(const QString &dependency, m_dependencies)
	{
		PluginSpec *found = 0;

		Q_FOREACH(PluginSpec *spec, specs)
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
			m_errorString.append(QCoreApplication::translate("PluginSpec", "Could not resolve dependency '%1'")
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

bool PluginSpec::initializePlugin()
{
	if (m_hasError)
		return false;
	if (m_state != State::Loaded)
	{
		if (m_state == State::Initialized)
			return true;
		return reportError(QCoreApplication::translate("PluginSpec", "Initializing the plugin failed because state != Loaded)"));
	}
	if (!m_plugin)
		return reportError(QCoreApplication::translate("PluginSpec", "Internal error: have no plugin instance to initialize"));

	QString err;
	if (!m_plugin->initialize(m_pluginManager, &err))
		return reportError(QCoreApplication::translate("PluginSpec", "Plugin initialization failed: %1").arg(err));

	m_state = State::Initialized;
	return true;
}

bool PluginSpec::initializeExtensions()
{
	if (m_hasError)
		return false;
	if (m_state != State::Initialized)
	{
		if (m_state == State::Running)
			return true;
		return reportError(QCoreApplication::translate("PluginSpec", "Cannot perform extensionsInitialized because state != Initialized"));
	}
	if (!m_plugin)
		return reportError(QCoreApplication::translate("PluginSpec", "Internal error: have no plugin instance to perform extensionsInitialized"));

	m_plugin->extensionsInitialized();
	m_state = State::Running;
	return true;
}

void PluginSpec::stop()
{
	if (!m_plugin)
		return;
	m_plugin->shutdown();
	m_state = State::Stopped;
}

void PluginSpec::kill()
{
	if (!m_plugin)
		return;

	bool b = loader->unload();
	if( !b )
	{
		nlinfo( "Plugin %s couldn't be unloaded.", this->m_name.toAscii().data() );
	}

	//delete m_plugin;
	m_plugin = NULL;
	delete loader;
	loader = NULL;
	m_state = State::Deleted;
}

void PluginSpec::setEnabledStartup(bool enabled)
{
	m_enabledStartup = enabled;
}

bool PluginSpec::isEnabledStartup() const
{
	return m_enabledStartup;
}

bool PluginSpec::reportError(const QString &err)
{
	m_errorString = err;
	m_hasError = true;
	return false;
}

} // namespace ExtensionSystem
