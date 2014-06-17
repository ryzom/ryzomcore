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

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamReader>

class QPluginLoader;

namespace ExtensionSystem
{

class PluginSpec: public IPluginSpec
{
public:
	~PluginSpec();
	virtual QString name() const;
	virtual QString version() const;
	virtual QString vendor() const;
	virtual QString description() const;

	virtual QString location() const;
	virtual QString filePath() const;
	virtual QString fileName() const;

	virtual IPlugin *plugin() const;

	// state
	virtual int state() const;
	virtual bool hasError() const;
	virtual QString errorString() const;
	QList<PluginSpec *> dependencySpecs() const;

	/// Enables/disables load this plugin after restart the program
	virtual void setEnabled(bool enabled);
	virtual bool isEnabled() const;

private:
	PluginSpec();
	
	bool setFileName(const QString &fileName);
	bool setSpecFileName(const QString &specFileName);
	bool readSpec();
	void parseSpec(QXmlStreamReader &reader);
	void parseDependency(QXmlStreamReader &reader);
	bool loadLibrary();
	bool resolveDependencies(const QList<PluginSpec *> &specs);
	bool initializePlugin();
	bool initializeExtensions();
	void stop();
	void kill();

	/// Enables/disables load this plugin on startup the program
	/// Method is used for disabling startup plugin by pluginmanager
	void setEnabledStartup(bool enabled);
	bool isEnabledStartup() const;

	bool reportError(const QString &err);

	QString m_location;
	QString m_filePath;
	QString m_fileName;

	QString m_name;
	QString m_version;
	QString m_vendor;
	QString m_description;

	QString m_nameSpecFile;
	QString m_prefix;
	QString m_suffix;
	int m_state;
	bool m_enabled, m_enabledStartup;
	bool m_hasError;
	QString m_errorString;
	QStringList m_dependencies;

	IPlugin *m_plugin;
	IPluginManager *m_pluginManager;
	QList<PluginSpec *> m_dependencySpecs;

	QPluginLoader *loader;

	friend class PluginManager;
};

} // namespace ExtensionSystem

#endif // PLUGINSPEC_H

