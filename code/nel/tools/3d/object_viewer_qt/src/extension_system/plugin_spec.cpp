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

#include "plugin_spec.h"

#include <QtCore/QList>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QPluginLoader>
#include <QtCore/QCoreApplication>

#include "nel/misc/app_context.h"

#include "iplugin.h"
#include "iplugin_manager.h"

namespace NLQT
{

CPluginSpec::CPluginSpec():
	_state(State::Invalid),
	_hasError(false),
	_plugin(NULL),
	_pluginManager(NULL)
{
}

QString CPluginSpec::name() const
{
	return _name;
}

QString CPluginSpec::version() const
{
	return _version;
}

QString CPluginSpec::vendor() const
{
	return _vendor;
}

QString CPluginSpec::description() const
{
	return _description;
}

QString CPluginSpec::location() const
{
	return _location;
}

QString CPluginSpec::filePath() const
{
	return _filePath;
}

QString CPluginSpec::fileName() const
{
	return _fileName;
}

IPlugin* CPluginSpec::plugin() const
{
	return _plugin;
}

int CPluginSpec::getState() const
{
	return _state;
}

bool CPluginSpec::hasError() const
{
	return _hasError;
}

QString CPluginSpec::errorString() const
{
	return _errorString;
}

bool CPluginSpec::setFileName(const QString &fileName)
{
	_name = _version
		= _vendor
		= _description
		= _location
		= _filePath
		= _fileName
		= "";
	_state = State::Invalid;
	_hasError = false;
	_errorString = "";
	QFile file(fileName);
	if (!file.exists())
		return reportError(QCoreApplication::translate("CPluginSpec", "File does not exist: %1").arg(file.fileName()));
	if (!file.open(QIODevice::ReadOnly))
		return reportError(QCoreApplication::translate("CPluginSpec", "Could not open file for read: %1").arg(file.fileName()));

	QFileInfo fileInfo(file);
	_location = fileInfo.absolutePath();
	_filePath = fileInfo.absoluteFilePath();
	_fileName = fileInfo.fileName();

	_state = State::Read;
	return true;
}

bool CPluginSpec::loadLibrary()
{
	if (_hasError)
		return false;
	if (_state != State::Read)
	{
		if (_state == State::Loaded)
			return true;
		return reportError(QCoreApplication::translate("CPluginSpec", "Loading the library failed because state != Resolved"));
	}
	
	QPluginLoader loader(_filePath);
	if (!loader.load())
		return reportError(loader.errorString());

	IPlugin *pluginObject = qobject_cast<IPlugin *>(loader.instance());
	if (!pluginObject)
	{
		loader.unload();
		return reportError(QCoreApplication::translate("CPluginSpec", "Plugin is not valid (does not derive from IPlugin)"));
	}

	pluginObject->setNelContext(&NLMISC::INelContext::getInstance());

	_name = pluginObject->name();
	_version = pluginObject->version();
	_vendor = pluginObject->vendor();
	_description = pluginObject->description();

	_state = State::Loaded;
	_plugin = pluginObject;
	return true;
}

bool CPluginSpec::initializePlugin()
{
	if (_hasError)
		return false;
	if (_state != State::Loaded)
	{
		if (_state == State::Initialized)
			return true;
		return reportError(QCoreApplication::translate("CPluginSpec", "Initializing the plugin failed because state != Loaded)"));
	}
	if (!_plugin)
		return reportError(QCoreApplication::translate("CPluginSpec", "Internal error: have no plugin instance to initialize"));

	QString err;
	if (!_plugin->initialize(_pluginManager, &err))
		return reportError(QCoreApplication::translate("CPluginSpec", "Plugin initialization failed: %1").arg(err));

	_state = State::Initialized;
	return true;
}

bool CPluginSpec::initializeExtensions()
{
	if (_hasError)
		return false;
	if (_state != State::Initialized)
	{
		if (_state == State::Running)
			return true;
		return reportError(QCoreApplication::translate("CPluginSpec", "Cannot perform extensionsInitialized because state != Initialized"));
	}
	if (!_plugin)
		return reportError(QCoreApplication::translate("CPluginSpec", "Internal error: have no plugin instance to perform extensionsInitialized"));

	_plugin->extensionsInitialized();
	_state = State::Running;
	return true;
}

void CPluginSpec::stop()
{
	if (!_plugin)
		return;
	_plugin->shutdown();
	_state = State::Stopped;
}

void CPluginSpec::kill()
{
	if (!_plugin)
		return;
	delete _plugin;
	_plugin = NULL;
	_state = State::Deleted;
}

bool CPluginSpec::reportError(const QString &err)
{
	_errorString = err;
	_hasError = true;
	return false;
}

} // namespace NLQT