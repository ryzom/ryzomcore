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

#ifndef GUI_EDITOR_PLUGIN_H
#define GUI_EDITOR_PLUGIN_H

#include "gui_editor_constants.h"
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"

#include "nel/misc/app_context.h"

#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace NLMISC
{
	class CLibraryContext;
}

namespace ExtensionSystem
{
	class IPluginSpec;
}

namespace GUIEditor
{
	class GUIEditorWindow;
	
	class GUIEditorPlugin : public QObject, public ExtensionSystem::IPlugin
	{
		Q_OBJECT
		Q_INTERFACES(ExtensionSystem::IPlugin)
	public:
		virtual ~GUIEditorPlugin();
		
		bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
		
		void extensionsInitialized();
		
		void shutdown();
		
		void setNelContext(NLMISC::INelContext *nelContext);
		
		void addAutoReleasedObject(QObject *obj);
		
	protected:
		NLMISC::CLibraryContext *m_libContext;
	
	private:
		ExtensionSystem::IPluginManager *m_plugMan;
		QList<QObject *> m_autoReleaseObjects;
	};	
}

#endif
