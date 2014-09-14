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


#ifndef EXPRESSION_NODE
#define EXPRESSION_NODE

#include <QGraphicsItem>
#include <QList>
#include "expr_slot_info.h"

class ExpressionLink;
class NodeSlot;

class ExpressionNode : public QGraphicsItem
{
public:
	ExpressionNode( QGraphicsItem *parent = NULL );
	~ExpressionNode();

	QRectF boundingRect() const;
	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

	void setLink( ExpressionLink *link, int slot );
	ExpressionLink* link( int slot ) const;

	QPointF slotPos( int slot ) const;

	int slotCount() const{ return m_slots.count(); }

	bool slotEmpty( int slot ) const;

	void getSlots( QList< SlotInfo > &l );

	void clearLinks();

protected:
	void mouseMoveEvent( QGraphicsSceneMouseEvent *e );

private:
	void createSlots();
	void paintSlots( QPainter *painter );

	qreal m_w;
	qreal m_h;

	QList< NodeSlot* > m_slots;
	QList< ExpressionLink* > m_links;
};

#endif

