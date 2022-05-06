// Ryzom Core Studio - GUI Editor Plugin
//
// Copyright (C) 2010-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#include "expression_node.h"
#include "expression_link.h"
#include <QPainter>
#include <QStyleOption>

struct NodeSlotInfo
{
	// top-left
	QPoint tl;

	// text top-left
	QPoint ttl;

	// The text displayed
	QString text;

	// text width
	qreal tw;

	// text height
	qreal th;

	// width-height of the box
	qreal wh;
};

class NodeSlot
{
public:
	NodeSlot( const NodeSlotInfo &info )
	{
		m_info = info;
	}

	~NodeSlot()
	{
	}

	QPointF pos() const{
		QPointF p;		
		p.setX( m_info.tl.x() + m_info.wh / 2.0 );
		p.setY( m_info.tl.y() + m_info.wh / 2.0 );

		return p;
	}

	void paint( QPainter *painter )
	{
		QBrush boxBrush;
		QPen p;
		
		boxBrush.setColor( Qt::black );
		boxBrush.setStyle( Qt::SolidPattern );
		p.setColor( Qt::black );
		painter->setPen( p );

		QRectF box;
		QRectF tbox;
		
		box.setTopLeft( m_info.tl );
		box.setHeight( m_info.wh );
		box.setWidth( m_info.wh );

		painter->fillRect( box, boxBrush );

		tbox.setTopLeft( m_info.ttl );
		tbox.setHeight( m_info.th );
		tbox.setWidth( m_info.tw );

		painter->drawText( tbox, Qt::AlignRight, m_info.text );
	}

	QString text() const{ return m_info.text; }
	void setText( const QString &text ){ m_info.text = text; }

private:
	NodeSlotInfo m_info;
};



ExpressionNode::ExpressionNode( const QString &name, int slotCount, QGraphicsItem *parent ) :
QGraphicsItem( parent )
{
	setFlags( QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsScenePositionChanges );

	m_w = 100;
	m_h = 100;
	m_hh = 20.0;

	m_variable = false;
	m_isValue = false;
	m_isRoot = false;

	m_name = name;

	if( slotCount > 3 )
		m_h = m_h + 20.0 * ( slotCount - 3 );

	createSlots( slotCount );
}

ExpressionNode::~ExpressionNode()
{
	clearLinks();
	clearSlots();
}

QRectF ExpressionNode::boundingRect() const
{
	return QRectF( 0, 0, m_w, m_h );
}

void ExpressionNode::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QBrush br;
	QBrush boxBrush;
	QPen p;
	QColor c;

	QRectF rect = boundingRect();
	QRectF header = rect;
	header.setHeight( m_hh );

	// Draw filled rectangle, header
	if( !m_isRoot )
	{
		c.setRed( 44 );
		c.setGreen( 169 );
		c.setBlue( 232 );
	}
	else
	{
		c.setRed( 255 );
		c.setGreen( 0 );
		c.setBlue( 0 );
	}
	br.setColor( c );
	br.setStyle( Qt::SolidPattern );
	p.setColor( c );
	painter->setPen( p );
	painter->fillRect( header, br );

	// Draw header text
	p.setColor( Qt::black );
	painter->setPen( p );
	painter->drawText( header, Qt::AlignCenter, m_name );

	// Draw value if applicable
	if( m_isValue )
	{
		QRectF vbox;
		vbox.setTopLeft( QPoint( 0.0, 20.0 ) );
		vbox.setWidth( header.width() );
		vbox.setHeight( header.height() );
		QPen vpen;
		vpen.setColor( Qt::red );
		painter->setPen( vpen );
		painter->drawText( vbox, Qt::AlignCenter, m_value );
		painter->setPen( p );
	}

	if( option->state & QStyle::State_Selected )
	{
		p.setStyle( Qt::DotLine );
		p.setColor( Qt::red );
	}

	// Draw outline of the entire thing + header
	painter->setPen( p );
	painter->drawRect( rect );
	painter->drawRect( header );

	paintSlots( painter );
}


QPointF ExpressionNode::slotPos( int slot ) const
{
	const NodeSlot *s = m_slots[ slot ];
	QPointF sp = s->pos();
	QPointF mp = pos();

	mp += sp;
	return mp;
}

void ExpressionNode::changeSlotCount( int count )
{
	clearSlots();
	clearLinks();
	m_links.clear();

	if( count <= 3 )
		m_h = 100.0;
	else
		m_h = 100.0 + 20.0 * ( count - 3 );

	createSlots( count );

	update();
}

void ExpressionNode::clearSlots()
{
	qDeleteAll( m_slots );
	m_slots.clear();
}

bool ExpressionNode::slotEmpty( int slot ) const
{
	if( m_links[ 0 ] == NULL )
		return true;
	else
		return false;
}

void ExpressionNode::getSlots( QList< SlotInfo > &l )
{
	SlotInfo info;

	for( int i = 0; i < m_slots.count(); i++ )
	{
		if( m_links[ i ] != NULL )
			continue;

		info.name = m_slots[ i ]->text();
		info.slot = i;
		l.push_back( info );
	}
}

void ExpressionNode::setLink( ExpressionLink *link, int slot )
{
	m_links[ slot ] = link;
}

ExpressionLink* ExpressionNode::link( int slot ) const
{
	return m_links[ slot ];
}

void ExpressionNode::setSlotNames( const QList< QString > &l )
{
	int c = l.count();
	for( int i = 0; i < c; i++ )
	{
		// "Out" slot is at position 0, so set the names with an offset of 1
		m_slots[ i + 1 ]->setText( l[ i ] );
	}
}

void ExpressionNode::setValue( const QString &value )
{
	m_value = value;

	int c = m_value.count();
	if( c < 15 )
		m_w = 100.0;
	else
		m_w = m_w + ( 100.0 / 15.0 ) * ( c - 15.0 );

	update();
}

void ExpressionNode::setRoot( bool b )
{
	m_isRoot = b;
	update();
}

QString ExpressionNode::build() const
{
	QString result;

	if( isValue() )
		return m_value;

	QStringList l = m_name.split( ' ' );
	result = l[ 0 ];

	int c = m_links.count();
	if( c == 1 )
	{
		result += "()";
		return result;
	}

	result += "( ";

	for( int i = 1; i < c; i++ )
	{
		ExpressionLink *link = m_links[ i ];
		if( link == NULL )
			continue;

		ExpressionNode *node = NULL;

		if( link->from() == this )
			node = link->to();
		else
			node = link->from();

		result += node->build();

		if( i != ( c - 1 ) )
			result += ", ";
	}

	result += " )";
	return result;
}

QVariant ExpressionNode::itemChange( GraphicsItemChange change, const QVariant &value )
{
	if( change == ItemScenePositionHasChanged )
	{
		onNodeMove();
	}

	return QGraphicsItem::itemChange( change, value );
}

void ExpressionNode::onNodeMove()
{
	for( int i = 0; i < m_links.count(); i++ )
	{
		ExpressionLink *link = m_links[ i ];
		if( link == NULL )
			continue;

		link->nodeMoved();
	}
}

void ExpressionNode::createSlots( int count)
{
	// Out nodes
	m_links.push_back( NULL );

	for( int i = 0; i < count; i++ )
		m_links.push_back( NULL );
	
	// First create the "Out" slot
	NodeSlotInfo info;
	info.tw = 25.0;
	info.th = 12.0;
	info.wh = 10.0;

	qreal x = 0.0;
	qreal y = m_h * 0.5;
	qreal tx = info.wh;
	qreal ty = m_h * 0.5 - 2;

	info.tl = QPoint( x, y );
	info.ttl = QPoint( tx, ty );
	info.text = "Out";

	m_slots.push_back( new NodeSlot( info ) );

	// Then the rest of them
	for( int i = 0; i < count; i++ )
	{
		x = m_w - info.wh;
		y = 30 + i * 20.0;
		tx = x - 5 - info.tw;
		ty = y - 2;

		info.tl = QPoint( x, y );
		info.ttl = QPoint( tx, ty );
		info.text = QString( 'A' + i );

		m_slots.push_back( new NodeSlot( info ) );
	}
}

void ExpressionNode::paintSlots( QPainter *painter )
{
	for( int i = 0; i < m_slots.count(); i++ )
	{
		NodeSlot *slot = m_slots[ i ];
		slot->paint( painter );
	}
}


void ExpressionNode::clearLinks()
{
	for( int i = 0; i < m_links.count(); i++ )
	{
		ExpressionLink *link = m_links[ i ];
		if( link == NULL )
			continue;

		link->unlink();
	}
}

