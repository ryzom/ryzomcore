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

ExpressionNode::ExpressionNode( QGraphicsItem *parent ) :
QGraphicsItem( parent )
{
	m_link = NULL;
}

ExpressionNode::~ExpressionNode()
{
}

QRectF ExpressionNode::boundingRect() const
{
	return QRectF( 0, 0, 100, 100 );
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

	paintConnections( painter );
}


void ExpressionNode::mouseMoveEvent( QGraphicsSceneMouseEvent *e )
{
	if( m_link != NULL )
		m_link->nodeMoved();

	QGraphicsItem::mouseMoveEvent( e );
}

void ExpressionNode::paintConnections( QPainter *painter )
{
	QRectF rect = boundingRect();
	QBrush boxBrush;
	QPen p;

	boxBrush.setColor( Qt::black );
	boxBrush.setStyle( Qt::SolidPattern );
	p.setColor( Qt::black );

	QRectF box = rect;
	QRectF tbox = rect;
	qreal wh = 10.0;
	qreal tw = 25.0;
	qreal th = 12.0;

	box.setTopLeft( QPoint( 0, rect.height() * 0.5 ) );
	box.setHeight( wh );
	box.setWidth( wh );

	painter->fillRect( box, boxBrush );

	tbox.setTopLeft( QPoint( 15, rect.height() * 0.50 ) );
	tbox.setHeight( th );
	tbox.setWidth( tw );
	painter->setPen( p );
	painter->drawText( tbox, Qt::AlignCenter, "Out" );


	for( int i = 0; i < 3; i++ )
	{
		qreal x = rect.width() - wh;
		qreal y = 30 + i * 20;
		qreal tx = x - 5 - tw;
		qreal ty = y - 2;
		
		box.setTopLeft( QPoint( x, y ) );
		box.setHeight( wh );
		box.setWidth( wh );

		painter->fillRect( box, boxBrush );

		tbox.setTopLeft( QPoint( tx, ty ) );
		tbox.setHeight( th );
		tbox.setWidth( tw );

		QString text = 'A' + i;
		painter->drawText( tbox, Qt::AlignRight, text );
	}
}


