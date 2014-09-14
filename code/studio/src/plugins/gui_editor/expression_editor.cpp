// Ryzom Core Studio - Georges Editor Plugin
//
// Copyright (C) 2014 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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


#include "expression_editor.h"
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMenu>
#include <qevent.h>

#include "expression_node.h"
#include "expression_link.h"
#include "expr_link_dlg.h"

#include <QMessageBox>

ExpressionEditor::ExpressionEditor( QWidget *parent ) :
QWidget( parent )
{
	m_ui.setupUi( this );

	m_selectionCount = 0;

	m_scene = new QGraphicsScene( this );
	m_ui.view->setScene( m_scene );

	connect( m_scene, SIGNAL( selectionChanged() ), this, SLOT( onSelectionChanged() ) );
}

ExpressionEditor::~ExpressionEditor()
{
	m_scene = NULL;
}

void ExpressionEditor::contextMenuEvent( QContextMenuEvent *e )
{	
	QMenu menu;

	QAction *a = NULL;
	QMenu *mm = menu.addMenu( "Add node" );
	a = mm->addAction( "1 slot" );
	connect( a, SIGNAL( triggered() ), this, SLOT( onAddNode1() ) );

	a = mm->addAction( "2 slots" );
	connect( a, SIGNAL( triggered() ), this, SLOT( onAddNode2() ) );

	a = mm->addAction( "3 slots" );
	connect( a, SIGNAL( triggered() ), this, SLOT( onAddNode3() ) );

	if( m_selectionCount > 0 )
	{
		a = menu.addAction( "Remove" );
		connect( a, SIGNAL( triggered() ), this, SLOT( onDeleteSelection() ) );

		if( m_selectionCount == 2 )
		{
			a = menu.addAction( "Link" );
			connect( a, SIGNAL( triggered() ), this, SLOT( onLinkItems() ) );
		}

		a = menu.addAction( "Unlink" );
		connect( a, SIGNAL( triggered() ), this, SLOT( onUnLinkItems() ) );
	}

	menu.exec( e->globalPos() );
}

void ExpressionEditor::onDeleteSelection()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();
	QListIterator< QGraphicsItem* > itr( l );
	while( itr.hasNext() )
	{
		QGraphicsItem *item = itr.next();

		ExpressionNode *node = dynamic_cast< ExpressionNode* >( item );
		if( node != NULL )		
		{
			ExpressionLink *link = NULL;

			int c = node->slotCount();
			for( int i = 0; i < c; i++ )
			{
				link = node->link( i );
				if( link != NULL )
				{
					link->unlink();
					m_scene->removeItem( link );
					delete link;
				}
			}
		}

		m_scene->removeItem( item );
		delete item;
	}
}

void ExpressionEditor::onSelectionChanged()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();
	m_selectionCount = l.count();
}

void ExpressionEditor::onLinkItems()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();
	ExpressionNode *from = static_cast< ExpressionNode* >( l[ 0 ] );
	ExpressionNode *to = static_cast< ExpressionNode* >( l[ 1 ] );

	QList< SlotInfo > froml;
	QList< SlotInfo > tol;

	from->getSlots( froml );
	to->getSlots( tol );

	// If there are no free slots, or both "Out" slots are taken we can't link
	if( froml.isEmpty() || tol.isEmpty() || ( !from->slotEmpty( 0 ) && !to->slotEmpty( 0 ) ) )
	{
		QMessageBox::information( this,
									tr( "Failed to link nodes" ),
									tr( "Unfortunately those nodes are full." ) );
		return;
	}

	ExprLinkDlg d;
	d.load( froml, tol );
	int result = d.exec();
	if( result == QDialog::Rejected )
		return;

	int slotA = d.getSlotA();
	int slotB = d.getSlotB();

	ExpressionLink *link = new ExpressionLink();
	link->link( from, to, slotA, slotB );

	m_scene->addItem( link );
}

void ExpressionEditor::onUnLinkItems()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();

	for( int i = 0; i < l.count(); i++ )
	{
		ExpressionNode *node = dynamic_cast< ExpressionNode* >( l[ i ] );
		if( node == NULL )
			continue;

		node->clearLinks();
	}
}

void ExpressionEditor::addNode( int slotCount )
{
	QGraphicsItem *item = new ExpressionNode( slotCount );
	item->setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable );
	m_scene->addItem( item );
}

void ExpressionEditor::onAddNode1()
{
	addNode( 1 );
}

void ExpressionEditor::onAddNode2()
{
	addNode( 2 );
}

void ExpressionEditor::onAddNode3()
{
	addNode( 3 );
}

