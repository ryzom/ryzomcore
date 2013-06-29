// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "material_editor_context.h"
#include "material_editor_window.h"

namespace MaterialEditor
{
	MaterialEditorContext::MaterialEditorContext( QObject *parent ) :
	IContext( parent ),
	m_materialEditorWindow( 0 )
	{
		m_materialEditorWindow = new MaterialEditorWindow();
	}
	
	void MaterialEditorContext::open()
	{
		m_materialEditorWindow->onOpenClicked();
	}

	void MaterialEditorContext::newDocument()
	{
	}

	void MaterialEditorContext::save()
	{
	}

	void MaterialEditorContext::saveAs()
	{
	}

	void MaterialEditorContext::close()
	{
		m_materialEditorWindow->close();
	}
		
	QWidget *MaterialEditorContext::widget()
	{
		return m_materialEditorWindow;
	}

}

