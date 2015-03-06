// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "gui_editor_plugin.h"
#include "gui_editor_window.h"
#include "gui_editor_context.h"
#include "gui_editor_core_listener.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

#include "nel/misc/debug.h"

#include <QtCore/QObject>

namespace GUIEditor
{
	GUIEditorPlugin::~GUIEditorPlugin()
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
	
	bool GUIEditorPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
	{
		Q_UNUSED(errorString);
		m_plugMan = pluginManager;
		GUIEditorContext *context = new GUIEditorContext( this );
		GUIEditorWindow *window = static_cast< GUIEditorWindow* >( context->widget() );
		
		addAutoReleasedObject( context );
		addAutoReleasedObject( new GUIEditorCoreListener( window, this ) );

		return true;
	}
	
	void GUIEditorPlugin::extensionsInitialized()
	{
	}
	
	void GUIEditorPlugin::shutdown()
	{
	}
	
	void GUIEditorPlugin::setNelContext(NLMISC::INelContext *nelContext)
	{

#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	m_libContext = new NLMISC::CLibraryContext(*nelContext);
	}

	void GUIEditorPlugin::addAutoReleasedObject(QObject *obj)
	{
		m_plugMan->addObject(obj);
		m_autoReleaseObjects.prepend(obj);
	}

}

Q_EXPORT_PLUGIN(GUIEditor::GUIEditorPlugin)