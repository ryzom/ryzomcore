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

namespace GUIEditor
{
	WidgetHierarchy::WidgetHierarchy( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
	}

	WidgetHierarchy::~WidgetHierarchy()
	{
	}

	void WidgetHierarchy::clearHierarchy()
	{
		widgetHT->clear();
	}

	void WidgetHierarchy::buildHierarchy( std::string &masterGroup )
	{
		clearHierarchy();

		CInterfaceGroup *mg = CWidgetManager::getInstance()->getMasterGroupFromId( masterGroup );
		if( mg != NULL )
		{
			QTreeWidgetItem *item = new QTreeWidgetItem( NULL );
			item->setText( 0, "root" );
			widgetHT->addTopLevelItem( item );

			buildHierarchy( item, mg );
		}
	}

	void WidgetHierarchy::buildHierarchy( QTreeWidgetItem *parent, CInterfaceGroup *group )
	{
		// First add ourselves
		QTreeWidgetItem *item = new QTreeWidgetItem( parent );
		item->setText( 0, group->getId().c_str() );

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
			subItem->setText( 0, (*citr)->getId().c_str() );
		}

		// Add our views
		const std::vector< CViewBase* > &views = group->getViews();
		std::vector< CViewBase* >::const_iterator vitr;
		for( vitr = views.begin(); vitr != views.end(); ++vitr )
		{
			QTreeWidgetItem *subItem = new QTreeWidgetItem( item );
			subItem->setText( 0, (*vitr)->getId().c_str() );
		}
	}
}
