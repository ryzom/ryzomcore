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
#include "expression_store.h"
#include "expression_info.h"

#include <QMessageBox>
#include <QInputDialog>

class ExpressionEditorPvt
{
public:

	ExpressionEditorPvt()
	{
		m_root = NULL;
	}

	ExpressionStore store;
	ExpressionNode *m_root;
};

ExpressionEditor::ExpressionEditor( QWidget *parent ) :
QMainWindow( parent )
{
	m_ui.setupUi( this );

	m_pvt = new ExpressionEditorPvt();
	
	m_selectionCount = 0;

	m_scene = new QGraphicsScene( this );
	m_ui.view->setScene( m_scene );

	connect( m_scene, SIGNAL( selectionChanged() ), this, SLOT( onSelectionChanged() ) );
	connect( m_ui.tree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( onItemDblClicked( QTreeWidgetItem* ) ) );

	m_nodeCount = 0;
}

ExpressionEditor::~ExpressionEditor()
{
	delete m_pvt;
	m_pvt = NULL;
	m_scene = NULL;
}

void ExpressionEditor::load()
{
	m_pvt->store.load();

	QList< const ExpressionInfo* > l;
	m_pvt->store.getExpressions( l );

	QListIterator< const ExpressionInfo* > itr( l );
	while( itr.hasNext() )
	{
		addExpression( itr.next() );
	}

	l.clear();
}

void ExpressionEditor::contextMenuEvent( QContextMenuEvent *e )
{	
	QMenu menu;
	QAction *a = NULL;

	if( m_selectionCount > 0 )
	{
		a = menu.addAction( "Remove" );
		connect( a, SIGNAL( triggered() ), this, SLOT( onDeleteSelection() ) );

		if( m_selectionCount == 1 )
		{
			QList< QGraphicsItem* > l = m_scene->selectedItems();
			ExpressionNode *node = dynamic_cast< ExpressionNode* >( l[ 0 ] );
			if( node != NULL )
			{
				if( node->variable() )
				{
					a = menu.addAction( "Change slot count" );
					connect( a, SIGNAL( triggered() ), this, SLOT( onChangeSlotCount() ) );
				}
				else
				if( node->isValue() )
				{
					a = menu.addAction( "Change value" );
					connect( a, SIGNAL( triggered() ), this, SLOT( onChangeValue() ) );
				}

				a = menu.addAction( "Set as root" );
				connect( a, SIGNAL( triggered() ), this, SLOT( onSetRoot() ) );
			}
		}
		else
		if( m_selectionCount == 2 )
		{
			a = menu.addAction( "Link" );
			connect( a, SIGNAL( triggered() ), this, SLOT( onLinkItems() ) );
		}

		a = menu.addAction( "Unlink" );
		connect( a, SIGNAL( triggered() ), this, SLOT( onUnLinkItems() ) );
	}
	else
	{
		a = menu.addAction( "Build expression" );
		connect( a, SIGNAL( triggered() ), this, SLOT( onBuildExpression() ) );
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
	d.load( froml, tol, from->name(), to->name() );
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

void ExpressionEditor::onItemDblClicked( QTreeWidgetItem *item )
{
	QString name = item->text( 0 );

	const ExpressionInfo *info = m_pvt->store.getInfo( name );

	QString n = name;
	n += " #";
	n += QString::number( m_nodeCount );
	m_nodeCount++;

	ExpressionNode *node = new ExpressionNode( n, info->slotNames.count() );
	node->setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable );
	node->setSlotNames( info->slotNames );
	node->setVariable( info->variable );
	node->setIsValue( info->value );
	if( node->isValue() )
	{
		node->setValue( "Value" );
	}
	m_scene->addItem( node );
}

void ExpressionEditor::onChangeSlotCount()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();
	ExpressionNode *node = static_cast< ExpressionNode* >( l[ 0 ] );
	int oldc = node->slotCount();

	int c = QInputDialog::getInt( this,
								tr( "Change slot count" ),
								tr( "Enter new slot count" ),
								oldc,
								1,
								26 );

	if( oldc == c )
		return;

	node->changeSlotCount( c );
}

void ExpressionEditor::onChangeValue()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();
	ExpressionNode *node = static_cast< ExpressionNode* >( l[ 0 ] );

	QString oldValue = node->getValue();

	QString newValue = QInputDialog::getText( this,
										tr( "Change value" ),
										tr( "Enter new value" ),
										QLineEdit::Normal,
										oldValue );

	if( newValue.isEmpty() )
		return;
	if( newValue == oldValue )
		return;

	node->setValue( newValue );
}

void ExpressionEditor::onSetRoot()
{
	QList< QGraphicsItem* > l = m_scene->selectedItems();
	ExpressionNode *node = static_cast< ExpressionNode* >( l[ 0 ] );

	if( m_pvt->m_root != NULL )
		m_pvt->m_root->setRoot( false );

	m_pvt->m_root = node;
	node->setRoot( true );
}

void ExpressionEditor::onBuildExpression()
{
	if( m_pvt->m_root == NULL )
	{
		QMessageBox::information( this,
									tr( "Building expression" ),
									tr( "Failed to build expression: You must set a root node." ) );
		return;
	}

	QString result = m_pvt->m_root->build();
	QMessageBox::information( this,
								tr( "Building expression" ),
								tr( "The result is\n" ) + result  );
}


void ExpressionEditor::addExpression( const ExpressionInfo *info )
{
	QTreeWidgetItem *item = findTopLevelItem( info->category );
	if( item == NULL )
	{
		item = new QTreeWidgetItem();
		item->setText( 0, info->category );
		m_ui.tree->addTopLevelItem( item );
	}

	QTreeWidgetItem *citem = new QTreeWidgetItem();
	citem->setText( 0, info->name );
	item->addChild( citem );
}

QTreeWidgetItem* ExpressionEditor::findTopLevelItem( const QString &text )
{
	int c = m_ui.tree->topLevelItemCount();
	for( int i = 0; i < c; i++ )
	{
		QTreeWidgetItem *item = m_ui.tree->topLevelItem( i );
		if( item->text( 0 ) == text )
			return item;
	}

	return NULL;
}

