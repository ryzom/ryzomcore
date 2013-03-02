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

#include <QMessageBox>
#include "editor_message_processor.h"

#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

namespace GUIEditor
{
	void CEditorMessageProcessor::onDelete()
	{
		std::string selection = CWidgetManager::getInstance()->getCurrentEditorSelection();
		if( selection.empty() )
			return;

		QMessageBox::StandardButton r =
			QMessageBox::question( NULL, 
									tr( "Deleting widget" ),
									tr( "Are you sure you want to delete %1?" ).arg( selection.c_str() ),
									QMessageBox::Yes | QMessageBox::No );
		if( r != QMessageBox::Yes )
			return;

		CInterfaceElement *e =
			CWidgetManager::getInstance()->getElementFromId( selection );
		if( e == NULL )
			return;

		CInterfaceElement *p = e;
		while( ( p != NULL ) && !p->isGroup() )
			p = p->getParent();
		
		CInterfaceGroup *g = dynamic_cast< CInterfaceGroup* >( p );
		if( g == NULL )
			return;

		if( g->delElement( e ) )
			CWidgetManager::getInstance()->setCurrentEditorSelection( "" );
	}
}

