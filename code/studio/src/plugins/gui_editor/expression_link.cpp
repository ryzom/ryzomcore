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

#include "expression_link.h"
#include "expression_node.h"
#include <QGraphicsItem>
#include <QPen>

ExpressionLink::ExpressionLink( QGraphicsItem *parent ) :
QGraphicsLineItem( parent )
{
	m_from = NULL;
	m_to = NULL;

	setFlags( QGraphicsItem::ItemIsSelectable );
}

ExpressionLink::~ExpressionLink()
{
	unlink();
}

void ExpressionLink::link( ExpressionNode *from, ExpressionNode *to, int fromSlot, int toSlot )
{
	m_from = from;
	m_to = to;
	m_from->setLink( this, fromSlot );
	m_to->setLink( this, toSlot );

	m_fromSlot = fromSlot;
	m_toSlot = toSlot;

	nodeMoved();
}

void ExpressionLink::unlink()
{
	if( m_from == NULL )
		return;

	m_from->setLink( NULL, m_fromSlot );
	m_to->setLink( NULL, m_toSlot );

	m_from = NULL;
	m_to = NULL;
}

void ExpressionLink::nodeMoved()
{
	setLine( QLineF( m_from->slotPos( m_fromSlot ), m_to->slotPos( m_toSlot ) ) );
}

void ExpressionLink::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QPen p;
	p.setColor( Qt::black );
	p.setWidth( 5 );
	setPen( p );

	QGraphicsLineItem::paint( painter, option, widget );
}


