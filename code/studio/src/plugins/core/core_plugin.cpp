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

// Project includes
#include "core_plugin.h"
#include "settings_dialog.h"
#include "core_constants.h"
#include "search_paths_settings_page.h"
#include "general_settings_page.h"
#include "../../extension_system/iplugin_spec.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

using namespace Core;

CorePlugin::CorePlugin()
	: m_plugMan(0),
	  m_mainWindow(0)
{
}

CorePlugin::~CorePlugin()
{
	Q_FOREACH(QObject *obj, m_autoReleaseObjects)
	{
		m_plugMan->removeObject(obj);
	}
	qDeleteAll(m_autoReleaseObjects);
	m_autoReleaseObjects.clear();

	if (m_mainWindow)
		delete m_mainWindow;
}

bool CorePlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;

	m_mainWindow = new MainWindow(pluginManager);
	bool success = m_mainWindow->initialize(errorString);

	GeneralSettingsPage *generalSettings = new GeneralSettingsPage(this);
	SearchPathsSettingsPage *searchPathPage = new SearchPathsSettingsPage(false, this);
	SearchPathsSettingsPage *recureseSearchPathPage = new SearchPathsSettingsPage(true, this);

	generalSettings->applyGeneralSettings();
	searchPathPage->applySearchPaths();
	recureseSearchPathPage->applySearchPaths();
	addAutoReleasedObject(generalSettings);
	addAutoReleasedObject(searchPathPage);
	addAutoReleasedObject(recureseSearchPathPage);
	return success;
}

void CorePlugin::extensionsInitialized()
{
	m_mainWindow->extensionsInitialized();
}

void CorePlugin::shutdown()
{
}

void CorePlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	m_libContext = new NLMISC::CLibraryContext(*nelContext);
}

void CorePlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

Q_EXPORT_PLUGIN(CorePlugin)
