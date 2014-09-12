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
	if( option->state & QStyle::State_Selected )
	{
		QPen outline;
		outline.setStyle( Qt::DotLine );
		painter->setPen( outline );

	}

	painter->drawRect( boundingRect() );
}


void ExpressionNode::mouseMoveEvent( QGraphicsSceneMouseEvent *e )
{
	if( m_link != NULL )
		m_link->nodeMoved();

	QGraphicsItem::mouseMoveEvent( e );
}


