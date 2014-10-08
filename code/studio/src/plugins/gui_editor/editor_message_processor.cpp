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
#include "widget_info_tree.h"

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

		CInterfaceElement *p = e->getParent();
		if( p == NULL )
			return;
		
		CInterfaceGroup *g = dynamic_cast< CInterfaceGroup* >( p );
		if( g == NULL )
			return;

		if( g->delElement( e ) )
		{
			CWidgetManager::getInstance()->setCurrentEditorSelection( "" );
		}
	}

	void CEditorMessageProcessor::onAdd( const QString &parentGroup, const QString &widgetType, const QString &name )
	{
		CWidgetInfoTreeNode *node = tree->findNodeByName( std::string( widgetType.toUtf8() ) );
		// No such widget
		if( node == NULL )
		{
			QMessageBox::critical(
				NULL,
				tr( "Error" ),
				tr( "Error adding the new widget! No such widget type!" ),
				QMessageBox::Ok
				);

			return;
		}

		// No class name defined
		std::string className = node->getInfo().className;
		if( className.empty() )
		{
			QMessageBox::critical(
				NULL,
				tr( "Error" ),
				tr( "Error adding the new widget! Missing classname!" ),
				QMessageBox::Ok
				);

			return;
		}

		std::string pgName = std::string( parentGroup.toUtf8() );
		std::string wName = std::string( name.toUtf8() );

		CInterfaceElement *e =
			CWidgetManager::getInstance()->addWidgetToGroup(
			pgName,
			className,
			wName
			);

		// Failed to add widget
		if( e == NULL )
		{
			QMessageBox::critical(
				NULL,
				tr( "Error" ),
				tr( "Error adding the new widget!" ),
				QMessageBox::Ok
				);

			return;
		}

		// Setting the defaults will override the Id too
		std::string id = e->getId();

		// Set up the defaults
		std::vector< SPropEntry >::const_iterator itr = node->getInfo().props.begin();
		while( itr != node->getInfo().props.end() )
		{
			e->setProperty( itr->propName, itr->propDefault );
			++itr;
		}

		// Restore the Id
		e->setId( id );
		// Make the widget aligned to the top left corner
		e->setParentPosRef( Hotspot_TL );
		e->setPosRef( Hotspot_TL );

		// Apply the new settings
		e->setActive( false );
		e->setActive( true );
	}

	void CEditorMessageProcessor::onSetGroupSelection( bool b )
	{
		CWidgetManager::getInstance()->setGroupSelection( b );
	}

	void CEditorMessageProcessor::onUngroup()
	{
		bool ok = CWidgetManager::getInstance()->unGroupSelection();

		if( !ok )
		{
			QMessageBox::critical( NULL,
									tr( "Ungrouping widgets" ),
									tr( "Couldn't ungroup widgets." ) );
		}
	}
}

