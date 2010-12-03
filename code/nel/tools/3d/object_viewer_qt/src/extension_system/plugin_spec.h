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

#include "iplugin_spec.h"

namespace NLQT
{

class CPluginSpec: public IPluginSpec
{
public:
	virtual QString name() const;
	virtual QString version() const;
	virtual QString vendor() const;
	virtual QString description() const;

	virtual QString location() const;
	virtual QString filePath() const;
	virtual QString fileName() const;

	virtual IPlugin *plugin() const;

	// state
	virtual int getState() const;
	virtual bool hasError() const;
	virtual QString errorString() const;

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

