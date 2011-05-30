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

#ifndef PLUGINSPEC_H
#define PLUGINSPEC_H

#include "iplugin_spec.h"

#include "QtCore/QList"

namespace ExtensionSystem
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
	QList<CPluginSpec *> dependencySpecs() const;

private:
	CPluginSpec();

	bool setFileName(const QString &fileName);
	bool loadLibrary();
	bool resolveDependencies(const QList<CPluginSpec *> &specs);
	bool initializePlugin();
	bool initializeExtensions();
	void stop();
	void kill();

	bool reportError(const QString &err);

	QString m_location;
	QString m_filePath;
	QString m_fileName;

	QString m_name;
	QString m_version;
	QString m_vendor;
	QString m_description;

	int m_state;
	bool m_hasError;
	QString m_errorString;

	IPlugin *m_plugin;
	IPluginManager *m_pluginManager;
	QList<CPluginSpec *> m_dependencySpecs;

	friend class CPluginManager;
};

} // namespace ExtensionSystem

#endif // PLUGINSPEC_H

