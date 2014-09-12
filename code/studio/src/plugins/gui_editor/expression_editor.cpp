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

#include <QMessageBox>

ExpressionEditor::ExpressionEditor( QWidget *parent ) :
QWidget( parent )
{
	m_ui.setupUi( this );

	m_selectionCount = 0;

	m_scene = new QGraphicsScene( this );
	m_ui.view->setScene( m_scene );

	m_scene->addSimpleText( "Hello" );

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
	a = menu.addAction( "Add rect" );
	connect( a, SIGNAL( triggered() ), this, SLOT( onAddRect() ) );

	if( m_selectionCount > 0 )
	{
		a = menu.addAction( "Remove" );
		connect( a, SIGNAL( triggered() ), this, SLOT( onDeleteSelection() ) );

		if( m_selectionCount == 2 )
		{
			a = menu.addAction( "Link" );
			connect( a, SIGNAL( triggered() ), this, SLOT( onLinkItems() ) );
		}
	}

	menu.exec( e->globalPos() );
}

void ExpressionEditor::onAddRect()
{
	QGraphicsItem *item = new ExpressionNode();
	item->setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable );
	m_scene->addItem( item );
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
			ExpressionLink *link = node->link();
			if( link != NULL )
			{
				link->unlink();
				m_scene->removeItem( link );
				delete link;
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

	if( ( from->link() != NULL ) || ( to->link() != NULL ) )
	{
		QMessageBox::information( this,
									tr( "Failed to link nodes" ),
									tr( "Unfortunately those nodes are already linked." ) );
		return;
	}

	ExpressionLink *link = new ExpressionLink();
	link->link( from, to );

	m_scene->addItem( link );
}


