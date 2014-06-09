// Object Viewer Qt - BNP Manager Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Roland Winklmeier <roland.m.winklmeier@googlemail.com>
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
#include "bnp_manager_plugin.h"
#include "bnp_manager_window.h"

#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/menu_manager.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMainWindow>

namespace BNPManager
{

	BNPManagerPlugin::BNPManagerPlugin()
	{
	}
	
	BNPManagerPlugin::~BNPManagerPlugin()
	{
		Q_FOREACH(QObject *obj, m_autoReleaseObjects)
		{
			m_plugMan->removeObject(obj);
		}
		qDeleteAll(m_autoReleaseObjects);
		m_autoReleaseObjects.clear();

		delete m_libContext;
		m_libContext = NULL;
	}

bool BNPManagerPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
    Q_UNUSED(errorString);
    m_plugMan = pluginManager;

    addAutoReleasedObject(new BNPManagerContext(this));
    return true;
}

void BNPManagerPlugin::extensionsInitialized()
{
}

void BNPManagerPlugin::shutdown()
{

}

void BNPManagerPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
    // Ensure that a context doesn't exist yet.
    // This only applies to platforms without PIC, e.g. Windows.
    nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
    m_libContext = new NLMISC::CLibraryContext(*nelContext);
}

void BNPManagerPlugin::addAutoReleasedObject(QObject *obj)
{
    m_plugMan->addObject(obj);
    m_autoReleaseObjects.prepend(obj);
}

/*void BNPManagerContext::open()
{
    m_BnpManagerWindow->open();
}*/
}

Q_EXPORT_PLUGIN(BNPManager::BNPManagerPlugin)
