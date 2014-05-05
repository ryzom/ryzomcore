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

#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QtCore/QtPlugin>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "iplugin_manager.h"

namespace NLMISC
{
class INelContext;
}

namespace ExtensionSystem
{

/**
@interface IPlugin
@brief Base class for all plugins.
@details The IPlugin class is an abstract class that must be implemented
once for each plugin. The IPlugin implementation must be exported and
made known to Qt's plugin system via the Q_EXPORT_PLUGIN macro,
see the Qt documentation for details on that.
*/
class IPlugin
{
public:
	virtual ~IPlugin() {}

	/**
	@brief Called after the plugin has been loaded and the IPlugin instance has been created.

	@details The initialize methods of plugins that depend
	on this plugin are called after the initialize method of this plugin
	has been called. Plugins should initialize their internal state in this
	method. Returns if initialization of successful. If it wasn't successful,
	the \a errorString should be set to a user-readable message
	describing the reason.
	*/
	virtual bool initialize(IPluginManager *pluginManager, QString *errorString) = 0;

	/**
	@brief Called after the IPlugin::initialize() method has been called,
	and after both the IPlugin::initialize() and IPlugin::extensionsInitialized()
	methods of plugins that depend on this plugin have been called.

	@details In this method, the plugin can assume that plugins that depend on
	this plugin are fully 'up and running'. It is a good place to
	look in the plugin manager's object pool for objects that have
	been provided by dependent plugins.
	*/
	virtual void extensionsInitialized() = 0;

	/**
	@brief Called during a shutdown sequence in the same order as initialization
	before the plugins get deleted in reverse order.

	@details This method should be used to disconnect from other plugins,
	hide all UI, and optimize shutdown in general.
	*/
	virtual void shutdown() { }

	/**
	@brief This method should be implemented to work properly NeL singletons.
	Called immediately after loading the plugin.
	@code
	void Plugin::setNelContext(NLMISC::INelContext *nelContext)
	{
	#ifdef NL_OS_WINDOWS
		// Ensure that a context doesn't exist yet.
		// This only applies to platforms without PIC, e.g. Windows.
		nlassert(!NLMISC::INelContext::isContextInitialised());
	#endif // NL_OS_WINDOWS
		_LibContext = new NLMISC::CLibraryContext(*nelContext);
	}
	@endcode
	*/
	virtual void setNelContext(NLMISC::INelContext *nelContext) = 0;
};

}; //namespace ExtensionSystem

Q_DECLARE_INTERFACE(ExtensionSystem::IPlugin, "dev.ryzom.com.ObjectViewerQt.IPlugin/0.9.2")

#endif // IPLUGIN_H
