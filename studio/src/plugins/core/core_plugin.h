// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

	CorePlugin();
	virtual ~CorePlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void shutdown();

	void setNelContext(NLMISC::INelContext *nelContext);

	void addAutoReleasedObject(QObject *obj);

	ExtensionSystem::IPluginManager *pluginManager() const
	{
		return m_plugMan;
	}

protected:
	NLMISC::CLibraryContext *m_libContext;

private:
	ExtensionSystem::IPluginManager *m_plugMan;
	MainWindow *m_mainWindow;
	QList<QObject *> m_autoReleaseObjects;
};

} // namespace Core

#endif // CORE_PLUGIN_H
