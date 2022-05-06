// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
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

// Project includes
#include "landscape_editor_plugin.h"
#include "landscape_editor_window.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>

namespace LandscapeEditor
{

LandscapeEditorPlugin::~LandscapeEditorPlugin()
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

bool LandscapeEditorPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;

	addAutoReleasedObject(new LandscapeEditorContext(this));
	return true;
}

void LandscapeEditorPlugin::extensionsInitialized()
{
}

void LandscapeEditorPlugin::shutdown()
{
}

void LandscapeEditorPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	m_libContext = new NLMISC::CLibraryContext(*nelContext);
}

void LandscapeEditorPlugin::addAutoReleasedObject(QObject *obj)
{
	m_plugMan->addObject(obj);
	m_autoReleaseObjects.prepend(obj);
}

LandscapeEditorContext::LandscapeEditorContext(QObject *parent)
	: IContext(parent),
	  m_landEditorWindow(0)
{
	m_landEditorWindow = new LandscapeEditorWindow();
}

QUndoStack *LandscapeEditorContext::undoStack()
{
	return m_landEditorWindow->undoStack();
}

void LandscapeEditorContext::open()
{
	m_landEditorWindow->open();
}

QWidget *LandscapeEditorContext::widget()
{
	return m_landEditorWindow;
}

}
Q_EXPORT_PLUGIN(LandscapeEditor::LandscapeEditorPlugin)
