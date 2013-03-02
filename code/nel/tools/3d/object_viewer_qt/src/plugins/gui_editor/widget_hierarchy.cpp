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


#include "widget_hierarchy.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

namespace
{
	std::string makeNodeName( const std::string &name )
	{
		std::string s = name;
		if( s.empty() )
			return s;

		std::string::size_type i = s.find_last_of( ":" );
		if( i == std::string::npos )
			return s;

		if( i == ( s.size() - 1 ) )
			return s;

		s = name.substr( i + 1, s.size() - 1 );
		return s;
	}

	std::string& makeFullName( QTreeWidgetItem *item, std::string &name )
	{
		if( item == NULL )
			return name;
		QString s;

		s = item->text( 0 );
		item = item->parent();

		while( item != NULL )
		{
			s.prepend( item->text( 0 ) + ":" );
			item = item->parent();
		}

		name = s.toUtf8().constData();
		return name;
	}
}

namespace GUIEditor
{
	WidgetHierarchy::WidgetHierarchy( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		connect( widgetHT, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ),
			this, SLOT( onItemDblClicked( QTreeWidgetItem* ) ) );
	}

	WidgetHierarchy::~WidgetHierarchy()
	{
	}

	void WidgetHierarchy::clearHierarchy()
	{
		widgetHT->clear();
		widgetHierarchyMap.clear();
	}

	void WidgetHierarchy::buildHierarchy( std::string &masterGroup )
	{
		clearHierarchy();

		CInterfaceGroup *mg = CWidgetManager::getInstance()->getMasterGroupFromId( masterGroup );
		if( mg != NULL )
		{
			QTreeWidgetItem *item = new QTreeWidgetItem( NULL );
			item->setText( 0, "ui" );
			widgetHT->addTopLevelItem( item );
			widgetHierarchyMap[ "ui" ] = item;

			buildHierarchy( item, mg );
		}
	}

	void WidgetHierarchy::buildHierarchy( QTreeWidgetItem *parent, CInterfaceGroup *group )
	{
		// First add ourselves
		QTreeWidgetItem *item = new QTreeWidgetItem( parent );
		
		item->setText( 0, makeNodeName( group->getId() ).c_str() );
		widgetHierarchyMap[ group->getId() ] = item;

		// Then add recursively our subgroups
		const std::vector< CInterfaceGroup* > &groups = group->getGroups();
		std::vector< CInterfaceGroup* >::const_iterator gitr;
		for( gitr = groups.begin(); gitr != groups.end(); ++gitr )
		{
			buildHierarchy( item, *gitr );
		}

		// Add our controls
		const std::vector< CCtrlBase* > &controls = group->getControls();
		std::vector< CCtrlBase* >::const_iterator citr;
		for( citr = controls.begin(); citr != controls.end(); ++citr )
		{
			QTreeWidgetItem *subItem = new QTreeWidgetItem( item );
			subItem->setText( 0, makeNodeName( (*citr)->getId() ).c_str() );
			widgetHierarchyMap[ (*citr)->getId() ] = subItem;
		}

		// Add our views
		const std::vector< CViewBase* > &views = group->getViews();
		std::vector< CViewBase* >::const_iterator vitr;
		for( vitr = views.begin(); vitr != views.end(); ++vitr )
		{
			QTreeWidgetItem *subItem = new QTreeWidgetItem( item );
			subItem->setText( 0, makeNodeName( (*vitr)->getId() ).c_str() );
			widgetHierarchyMap[ (*vitr)->getId() ] = subItem;
		}
	}

	void WidgetHierarchy::onGUILoaded()
	{
		if( masterGroup.empty() )
			return;
		buildHierarchy( masterGroup );
		currentSelection.clear();
	}

	void WidgetHierarchy::onSelectionChanged( std::string &newSelection )
	{
		if( newSelection == currentSelection )
			return;

		std::map< std::string, QTreeWidgetItem* >::iterator itr = 
			widgetHierarchyMap.find( newSelection );
		if( itr == widgetHierarchyMap.end() )
			return;

		if( widgetHT->currentItem() != NULL )
			widgetHT->currentItem()->setSelected( false );

		QTreeWidgetItem *item = itr->second;
		QTreeWidgetItem *currItem = item;
		while( currItem != NULL )
		{
			currItem->setExpanded( true );
			currItem = currItem->parent();
		}

		item->setSelected( true );
		widgetHT->setCurrentItem( item );
	}

	void WidgetHierarchy::onItemDblClicked( QTreeWidgetItem *item )
	{
		if( item->parent() == NULL )
			return;
		
		std::string n = item->text( 0 ).toUtf8().constData();
		currentSelection = makeFullName( item, n );
		CWidgetManager::getInstance()->setCurrentEditorSelection( currentSelection );
	}
}
