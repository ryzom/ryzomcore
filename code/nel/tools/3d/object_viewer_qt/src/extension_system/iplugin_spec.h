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

#ifndef IPLUGINSPEC_H
#define IPLUGINSPEC_H

#include <QtCore/QString>

namespace ExtensionSystem
{
class IPlugin;
class IPluginManager;

/**
@struct State
@details The plugin goes through several steps while being loaded.
The state gives a hint on what went wrong in case of an error.
*/
struct State
{
	enum List
	{
		Invalid = 1,
		Read,
		Resolved,
		Loaded,
		Initialized,
		Running,
		Stopped,
		Deleted
	};
};

/**
@interface IPluginSpec
@brief Interface for plugin spec contains the information of the plugins and
information about the plugin's current state.
@details The plugin spec is also filled with more information as the plugin
goes through its loading process (see State).
If an error occurs, the plugin spec is the place to look for the error details.
*/
class IPluginSpec
{
public:
	virtual ~IPluginSpec() {}

	virtual QString name() const = 0;
	virtual QString version() const = 0;
	virtual QString vendor() const = 0;
	virtual QString description() const = 0;

	virtual QString location() const = 0;
	virtual QString filePath() const = 0;
	virtual QString fileName() const = 0;

	virtual IPlugin *plugin() const = 0;

	// state
	virtual int state() const = 0;
	virtual bool hasError() const = 0;
	virtual QString errorString() const = 0;

	/// Enables/disables load this plugin after restart the program
	virtual void setEnabled(bool enabled) = 0;
	virtual bool isEnabled() const = 0;
};

} // namespace ExtensionSystem

#endif // IPLUGINSPEC_H

