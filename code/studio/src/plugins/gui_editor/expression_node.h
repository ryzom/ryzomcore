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

class ExpressionLink;

class ExpressionNode : public QGraphicsItem
{
public:
	ExpressionNode( QGraphicsItem *parent = NULL );
	~ExpressionNode();

	QRectF boundingRect() const;
	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

	void setLink( ExpressionLink *link ){ m_link = link; }
	ExpressionLink* link() const{ return m_link; }

protected:
	void mouseMoveEvent( QGraphicsSceneMouseEvent *e );

private:
	ExpressionLink *m_link;

};

#endif

