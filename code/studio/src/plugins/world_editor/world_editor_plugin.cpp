// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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
#include "world_editor_plugin.h"
#include "world_editor_window.h"
#include "world_editor_settings_page.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include "nel/misc/debug.h"
#include <nel/misc/path.h>
#include <nel/ligo/primitive_utils.h>
#include <nel/ligo/primitive.h>

// Qt includes
#include <QtCore/QObject>

namespace WorldEditor
{

WorldEditorPlugin::~WorldEditorPlugin()
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

bool WorldEditorPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	m_plugMan = pluginManager;

	WorldEditorSettingsPage *weSettings = new WorldEditorSettingsPage(this);
	addAutoReleasedObject(weSettings);
	addAutoReleasedObject(new WorldEditorContext(this));
	return true;
}

void WorldEditorPlugin::extensionsInitialized()
{
}

void WorldEditorPlugin::shutdown()
{
}

void WorldEditorPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	m_libContext = new NLMISC::CLibraryContext(*nelContext);
}

void WorldEditorPlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

WorldEditorContext::WorldEditorContext(QObject *parent)
	: IContext(parent),
	  m_worldEditorWindow(0)
{
	QSettings *settings = Core::ICore::instance()->settings();
    settings->beginGroup(Constants::WORLD_EDITOR_SECTION);
    m_ligoConfig.CellSize = settings->value(Constants::WORLD_EDITOR_CELL_SIZE, "160").toFloat();
    m_ligoConfig.Snap = settings->value(Constants::WORLD_EDITOR_SNAP, "1").toFloat();
    m_ligoConfig.ZoneSnapShotRes = settings->value(Constants::ZONE_SNAPSHOT_RES, "128").toUInt();
    QString fileName = settings->value(Constants::PRIMITIVE_CLASS_FILENAME, "world_editor_classes.xml").toString();
    settings->endGroup();
    try
    {
        // Search path of file world_editor_classes.xml
        std::string ligoPath = NLMISC::CPath::lookup(fileName.toUtf8().constData());
        // Init LIGO
        m_ligoConfig.readPrimitiveClass(ligoPath.c_str(), true);
        NLLIGO::Register();
        NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &m_ligoConfig;
    }
    catch (NLMISC::Exception &e)
    {
        nlinfo( "Error starting LIGO." );
    }

    // Reset
    m_ligoConfig.resetPrimitiveConfiguration ();

    // TODO: get file names! from settings
    m_ligoConfig.readPrimitiveClass("world_editor_primitive_configuration.xml", true);

	m_worldEditorWindow = new WorldEditorWindow();
}

QUndoStack *WorldEditorContext::undoStack()
{
	return m_worldEditorWindow->undoStack();
}

void WorldEditorContext::onActivated()
{
    NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &m_ligoConfig;
}

void WorldEditorContext::open()
{
	m_worldEditorWindow->open();
}

QWidget *WorldEditorContext::widget()
{
	return m_worldEditorWindow;
}

}

Q_EXPORT_PLUGIN(WorldEditor::WorldEditorPlugin)
