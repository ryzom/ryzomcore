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


#include "gui_editor_context.h"
#include "gui_editor_window.h"

namespace GUIEditor
{
	GUIEditorContext::GUIEditorContext(QObject *parent) :
	IContext(parent),
	m_guiEditorWindow(0)
	{
		m_guiEditorWindow = new GUIEditorWindow();
	}
	
	QUndoStack *GUIEditorContext::undoStack()
	{
		return m_guiEditorWindow->undoStack();
	}
	
	void GUIEditorContext::open()
	{
		m_guiEditorWindow->open();
	}

	void GUIEditorContext::newDocument()
	{
		m_guiEditorWindow->newDocument();
	}

	void GUIEditorContext::save()
	{
		m_guiEditorWindow->save();
	}

	void GUIEditorContext::saveAs()
	{
		m_guiEditorWindow->saveAs();
	}

	void GUIEditorContext::close()
	{
		m_guiEditorWindow->close();
	}
		
	QWidget *GUIEditorContext::widget()
	{
		return m_guiEditorWindow;
	}

}

