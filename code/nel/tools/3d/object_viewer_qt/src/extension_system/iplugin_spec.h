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

#ifndef IPLUGINSPEC_H
#define IPLUGINSPEC_H

#include <QtCore/QString>

namespace NLQT
{
class IPlugin;
class IPluginManager;

struct State
{
	enum List
	{
		Invalid = 1,
		Read,
		Loaded,
		Initialized,
		Running,
		Stopped,
		Deleted
	};
};

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
	virtual int getState() const = 0;
	virtual bool hasError() const = 0;
	virtual QString errorString() const = 0;
};

} // namespace NLQT

#endif // IPLUGINSPEC_H

