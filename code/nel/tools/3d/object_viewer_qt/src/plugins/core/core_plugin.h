// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2010  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef CORE_PLUGIN_H
#define CORE_PLUGIN_H

#include "../../extension_system/iplugin.h"
#include "main_window.h"

#include "nel/misc/app_context.h"

#include <QtCore/QObject>

namespace NLMISC
{
class CLibraryContext;
}

namespace ExtensionSystem
{
class IPluginSpec;
}

namespace Core
{

class CorePlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void shutdown();

	void setNelContext(NLMISC::INelContext *nelContext);

	QString name() const;
	QString version() const;
	QString vendor() const;
	QString description() const;
	QList<QString> dependencies() const;

	QObject *objectByName(const QString &name) const;
	ExtensionSystem::IPluginSpec *pluginByName(const QString &name) const;
	ExtensionSystem::IPluginManager *pluginManager() const
	{
		return _plugMan;
	}

	template <typename Type>
	QList<Type *> getObjects() const
	{
		QList<QObject *> all = _plugMan->allObjects();
		QList<Type *> objects;
		Q_FOREACH(QObject *obj, all)
		{
			Type *typeObj = qobject_cast<Type *>(obj);
			if (typeObj)
				objects.append(typeObj);
		}
		return objects;
	}

protected:
	NLMISC::CLibraryContext *_LibContext;

private Q_SLOTS:
	void execSettings();

private:
	ExtensionSystem::IPluginManager *_plugMan;
	ExtensionSystem::CPluginView *_pluginView;
	CMainWindow *_mainWindow;

	bool oldOVQT;
};

} // namespace Core

#endif // CORE_PLUGIN_H
