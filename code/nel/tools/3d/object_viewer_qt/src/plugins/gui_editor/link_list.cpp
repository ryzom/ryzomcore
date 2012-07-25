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


#include "link_list.h"
#include "link_editor.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/link_data.h"
#include <map>
#include <string>


namespace GUIEditor
{
	LinkList::LinkList( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		linkEditor = new LinkEditor();

		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditButtonClicked() ) );
	}

	LinkList::~LinkList()
	{
		delete linkEditor;
	}

	void LinkList::onGUILoaded()
	{
		const std::map< uint32, SLinkData > &linkMap =
			CWidgetManager::getInstance()->getParser()->getLinkMap();
		
		std::map< uint32, SLinkData >::const_iterator itr;
		for( itr = linkMap.begin(); itr != linkMap.end(); ++itr )
		{
			QTreeWidgetItem *item = new QTreeWidgetItem( linkTree );
			item->setText( 0, itr->second.parent.c_str() );
			item->setText( 1, itr->second.target.c_str() );
			item->setText( 2, itr->second.action.c_str() );
			item->setData( 3, Qt::UserRole, itr->first );
			linkTree->addTopLevelItem( item );
		}
		linkTree->sortByColumn( 0 );
	}

	void LinkList::onEditButtonClicked()
	{

		QTreeWidgetItem *item =
			linkTree->currentItem();
		if( item == NULL )
			return;

		bool ok;
		uint32 id = item->data( 3, Qt::UserRole ).toUInt( &ok );
		if( !ok )
			return;

		linkEditor->setLinkId( id );
		linkEditor->show();
	}
}

