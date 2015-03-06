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
#include <QMessageBox>
#include <QInputDialog>
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
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddButtonClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveButtonClicked() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditButtonClicked() ) );

		connect( linkTree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ),
			this, SLOT( onItemDblClicked( QTreeWidgetItem* ) ) );

		connect( linkEditor, SIGNAL( okClicked() ), this, SLOT( onEditorFinished() ) );
	}

	LinkList::~LinkList()
	{
		delete linkEditor;
	}

	void LinkList::clear()
	{
		linkTree->clear();
		linkEditor->clear();
	}

	void LinkList::onGUILoaded()
	{
		linkTree->clear();

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

	void LinkList::onAddButtonClicked()
	{
		bool ok;
		QString parent =
			QInputDialog::getText( this, 
								tr( "Adding a new link" ),
								tr( "Please specify the parent group's full id" ),
								QLineEdit::Normal,
								QString(),
								&ok );
		if( ok )
		{
			if( CWidgetManager::getInstance()->getElementFromId( parent.toUtf8().constData() ) == NULL )
			{
				QMessageBox::critical( this,
									tr( "Parent group doesn't exist" ),
									tr( "The specified parent group '%1' doesn't exist!" ).arg( parent ) );
				return;
			}
			SLinkData data;
			data.parent = parent.toUtf8().constData();
			
			uint32 id = CWidgetManager::getInstance()->getParser()->addLinkData( data );

			QTreeWidgetItem *item = new QTreeWidgetItem( linkTree );
			item->setText( 0, parent );
			item->setData( 3, Qt::UserRole, id );
			linkTree->addTopLevelItem( item );
		}
	}

	void LinkList::onRemoveButtonClicked()
	{
		QTreeWidgetItem *item =
			linkTree->currentItem();
		if( item == NULL )
			return;

		bool ok;
		uint32 id = item->data( 3, Qt::UserRole ).toUInt( &ok );
		if( !ok )
			return;

		QMessageBox::StandardButton reply =
			QMessageBox::question( this,
								tr( "Removing a link" ),
								tr( "Are you sure you want to remove this link?" ),
								QMessageBox::Yes | QMessageBox::Cancel );
		
		if( reply != QMessageBox::Yes )
			return;
		
		CWidgetManager::getInstance()->getParser()->removeLinkData( id );
		linkTree->takeTopLevelItem( linkTree->indexOfTopLevelItem( item ) );
		delete item;
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

	void LinkList::onItemDblClicked( QTreeWidgetItem *item )
	{
		bool ok;
		uint32 id = item->data( 3, Qt::UserRole ).toUInt( &ok );
		if( !ok )
			return;

		linkEditor->setLinkId( id );
		linkEditor->show();
	}

	void LinkList::onEditorFinished()
	{
		QTreeWidgetItem *item =
			linkTree->currentItem();
		if( item == NULL )
			return;

		bool ok;
		uint32 id = item->data( 3, Qt::UserRole ).toUInt( &ok );
		if( !ok )
			return;

		SLinkData data;
		if( !CWidgetManager::getInstance()->getParser()->getLinkData( id, data ) )
			return;
		
		item->setText( 1, data.target.c_str() );
		item->setText( 2, data.action.c_str() );
	}
}

