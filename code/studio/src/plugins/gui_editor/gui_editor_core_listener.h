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


#ifndef GUI_EDITOR_CORE_LISTENER_H
#define GUI_EDITOR_CORE_LISTENER_H

#include "../core/icore_listener.h"

namespace GUIEditor
{
	class GUIEditorWindow;

	class GUIEditorCoreListener : public Core::ICoreListener
	{
		Q_OBJECT
	public:
		GUIEditorCoreListener( GUIEditorWindow *mainWindow, QObject *parent = NULL ){ this->mainWindow = mainWindow; }
		~GUIEditorCoreListener(){}
		bool closeMainWindow() const;

	private:
		GUIEditorWindow *mainWindow;
	};
}

#endif

