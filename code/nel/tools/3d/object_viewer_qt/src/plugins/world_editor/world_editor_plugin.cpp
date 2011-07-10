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

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include "nel/misc/debug.h"

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
}

bool WorldEditorPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;

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

QString WorldEditorPlugin::name() const
{
	return tr("WorldEditor");
}

QString WorldEditorPlugin::version() const
{
	return "0.0.1";
}

QString WorldEditorPlugin::vendor() const
{
	return "GSoC2011_dnk-88";
}

QString WorldEditorPlugin::description() const
{
	return "World editor ovqt plugin.";
}

QStringList WorldEditorPlugin::dependencies() const
{
	QStringList list;
	list.append(Core::Constants::OVQT_CORE_PLUGIN);
	return list;
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
	m_worldEditorWindow = new WorldEditorWindow();
}

QUndoStack *WorldEditorContext::undoStack()
{
	return m_worldEditorWindow->undoStack();
}

void WorldEditorContext::open()
{
	//m_worldEditorWindow->open();
}

QWidget *WorldEditorContext::widget()
{
	return m_worldEditorWindow;
}

}

Q_EXPORT_PLUGIN(WorldEditor::WorldEditorPlugin)