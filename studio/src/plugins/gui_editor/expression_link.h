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

#ifndef EXPRESSION_LINK
#define EXPRESSION_LINK

#include <QGraphicsItem>

class ExpressionNode;

class ExpressionLink : public QGraphicsLineItem
{
public:
	ExpressionLink( QGraphicsItem *parent = NULL );
	~ExpressionLink();

	void link( ExpressionNode *from, ExpressionNode *to, int fromSlot, int toSlot );
	void unlink();

	void nodeMoved();

	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

	ExpressionNode* from() const{ return m_from; }
	ExpressionNode* to() const{ return m_to; }

private:
	ExpressionNode *m_from;
	ExpressionNode *m_to;

	int m_fromSlot;
	int m_toSlot;
};

#endif

