// Ryzom Core Studio - GUI Editor Plugin
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
	ExpressionNode( const QString &name, int slotCount = 3, QGraphicsItem *parent = NULL );
	~ExpressionNode();

	QRectF boundingRect() const;
	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

	void setLink( ExpressionLink *link, int slot );
	ExpressionLink* link( int slot ) const;

	QPointF slotPos( int slot ) const;

	int slotCount() const{ return m_slots.count(); }
	void changeSlotCount( int count );
	void clearSlots();

	bool slotEmpty( int slot ) const;

	void getSlots( QList< SlotInfo > &l );

	void clearLinks();

	QString name() const{ return m_name; }

	void setSlotNames( const QList< QString > &l );

	void setVariable( bool b ){ m_variable = b; }
	bool variable() const{ return m_variable; }

	void setValue( const QString &value );
	QString getValue() const{ return m_value; }

	bool isValue() const{ return m_isValue; }
	void setIsValue( bool b ){ m_isValue = b; }
	void setRoot( bool b );

	QString build() const;

protected:
	QVariant itemChange( GraphicsItemChange change, const QVariant &value );

private:
	void onNodeMove();
	void createSlots( int count = 3 );
	void paintSlots( QPainter *painter );

	qreal m_w; // node width
	qreal m_h; // node height
	qreal m_hh; // header height

	QList< NodeSlot* > m_slots;
	QList< ExpressionLink* > m_links;

	QString m_name;

	bool m_variable;

	QString m_value;
	bool m_isValue;
	bool m_isRoot;
};

#endif

