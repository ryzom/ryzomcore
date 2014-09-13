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

private:
	NodeSlotInfo m_info;
};



ExpressionNode::ExpressionNode( QGraphicsItem *parent ) :
QGraphicsItem( parent )
{
	m_w = 100;
	m_h = 100;

	createSlots();

	for( int i = 0; i < 4; i++ )
		m_links.push_back( NULL );
}

ExpressionNode::~ExpressionNode()
{
	qDeleteAll( m_slots );
	m_slots.clear();
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
	header.setHeight( header.height() * 0.2 );

	// Draw filled rectangle, header
	c.setRed( 44 );
	c.setGreen( 169 );
	c.setBlue( 232 );
	br.setColor( c );
	br.setStyle( Qt::SolidPattern );
	p.setColor( c );
	painter->setPen( p );
	painter->fillRect( header, br );

	// Draw header text
	p.setColor( Qt::black );
	painter->setPen( p );
	painter->drawText( header, Qt::AlignCenter, "Something" );

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

void ExpressionNode::setLink( ExpressionLink *link, int slot )
{
	m_links[ slot ] = link;
}

ExpressionLink* ExpressionNode::link( int slot ) const
{
	return m_links[ slot ];
}

void ExpressionNode::mouseMoveEvent( QGraphicsSceneMouseEvent *e )
{
	for( int i = 0; i < 4; i++ )
	{
		ExpressionLink *link = m_links[ i ];
		if( link == NULL )
			continue;

		link->nodeMoved();
	}

	QGraphicsItem::mouseMoveEvent( e );
}

void ExpressionNode::createSlots()
{
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
	for( int i = 0; i < 3; i++ )
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
	for( int i = 0; i < 4; i++ )
	{
		NodeSlot *slot = m_slots[ i ];
		slot->paint( painter );
	}
}


