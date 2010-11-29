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

#ifndef PLUGINSPEC_H
#define PLUGINSPEC_H

#include <QtCore/QString>
#include <QtCore/QList>

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

class CPluginSpec
{
public:
	~CPluginSpec();

	QString name() const;
	QString version() const;
	QString vendor() const;
	QString description() const;

	QString location() const;
	QString filePath() const;
	QString fileName() const;

	IPlugin *plugin() const;

	// state
	int getState() const;
	bool hasError() const;
	QString errorString() const;

private:
	CPluginSpec();

	bool setFileName(const QString &fileName);
	bool loadLibrary();
	bool initializePlugin();
	bool initializeExtensions();
	void stop();
	void kill();

	bool reportError(const QString &err);

	QString _location;
	QString _filePath;
	QString _fileName;

	QString _name;
	QString _version;
	QString _vendor;
	QString _description;

	int _state;
	bool _hasError;
	QString _errorString;

	IPlugin *_plugin;
	IPluginManager *_pluginManager;

	friend class CPluginManager;
};

} // namespace NLQT

#endif // PLUGINSPEC_H

