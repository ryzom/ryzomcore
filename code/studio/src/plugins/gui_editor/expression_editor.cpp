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

ExpressionEditor::ExpressionEditor( QWidget *parent ) :
QWidget( parent )
{
	m_ui.setupUi( this );

	m_hasSelection = false;

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

	if( m_hasSelection )
	{
		a = menu.addAction( "Remove" );
		connect( a, SIGNAL( triggered() ), this, SLOT( onDeleteSelection() ) );
	}

	menu.exec( e->globalPos() );
}

void ExpressionEditor::onAddRect()
{
	QGraphicsRectItem *item = new QGraphicsRectItem( 0, 0, 100, 100 );
	item->setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable );
	m_scene->addItem( item );
}

void ExpressionEditor::onDeleteSelection()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();
	QListIterator< QGraphicsItem* > itr( l );
	while( itr.hasNext() )
		m_scene->removeItem( itr.next() );
}

void ExpressionEditor::onSelectionChanged()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();
	if( l.isEmpty() )
		m_hasSelection = false;
	else
		m_hasSelection = true;
}



