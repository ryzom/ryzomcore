// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef WORLD_EDITOR_SCENE_ITEM_H
#define WORLD_EDITOR_SCENE_ITEM_H

// Project includes
#include "world_editor_global.h"

// NeL includes

// Qt includes
#include <QtCore/QPair>
#include <QtGui/QGraphicsObject>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsPolygonItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsSceneMouseEvent>

namespace WorldEditor
{
class GraphicsItemZone;
class GraphicsItemNode;

typedef QPair<GraphicsItemNode *, GraphicsItemNode *> LineNode;

const int SELECTED_LAYER = 200;
const int UNSELECTED_LAYER = 100;
const int WORLD_ZONE_LAYER = 100;
const int WORLD_POINT_LAYER = 200;
const int WORLD_PATH_LAYER = 200;
const int MIDDLE_POINT_LAYER = 201;
const int EDGE_POINT_LAYER = 201;

const int SIZE_ARROW = 20;
/*
// Deprecated
class GraphicsItemNode: public QGraphicsObject
{
	Q_OBJECT
	Q_PROPERTY(QColor colorNode READ colorNode WRITE setColorNode)
public:
	enum NodeType
	{
		EdgeType = 0,
		MiddleType
	};

	GraphicsItemNode(GraphicsItemZone *itemZone, QGraphicsItem *parent = 0);
	virtual ~GraphicsItemNode();

	void setColorNode(const QColor &color);
	QColor colorNode() const
	{
		return m_color;
	}

	void setNodeType(NodeType nodeType);

	virtual QRectF boundingRect() const;
	//QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:

	NodeType m_type;
	GraphicsItemZone *m_itemZone;
	QColor m_color;
};

// Deprecated
class GraphicsItemZone: public QGraphicsPolygonItem
{
public:
	GraphicsItemZone(QGraphicsScene *scene, QGraphicsItem *parent = 0);
	virtual ~GraphicsItemZone();

	void scanPolygon(const QPolygonF &polygon);
	void updateMiddleNode(GraphicsItemNode *node);
	bool deleteEdgePoint(GraphicsItemNode *node);
	//void addNode(GraphicsItemNode *node);
	void updateZone();
	//QRectF boundingRect() const;
	//void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	// QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	struct LineItem
	{
		GraphicsItemNode *itemPoint;
		LineNode lineNode;
	};
	QGraphicsScene *m_scene;
	QColor m_color;
	QList<GraphicsItemNode *> m_listItems;
	QList<LineItem> m_listLines;
};
*/

/*
@class WorldItemPoint
@brief
@details
*/
class AbstractWorldItem: public QGraphicsItem
{
public:
	AbstractWorldItem(QGraphicsItem *parent = 0);
	virtual ~AbstractWorldItem();

	enum { Type = QGraphicsItem::UserType + 1 };

	virtual void moveOn(const QPointF &offset) = 0;
	virtual void rotateOn(const QPointF &pivot, const qreal deltaAngle) = 0;
	// TODO: add modes: IgnoreAspectRatio, KeepAspectRatio
	virtual void scaleOn(const QPointF &pivot, const QPointF &factor) = 0;
	virtual void turnOn(const qreal angle) = 0;
	virtual void radiusOn(const qreal radius) = 0;

	virtual void setColor(const QColor &color) = 0;

	// Enable the use of qgraphicsitem_cast with this item.
	int type() const;
};

/*
@class WorldItemPoint
@brief
@details
*/
class WorldItemPoint: public AbstractWorldItem
{
public:
	WorldItemPoint(const QPointF &point, const qreal angle, QGraphicsItem *parent = 0);
	virtual ~WorldItemPoint();

	virtual void moveOn(const QPointF &offset);
	virtual void rotateOn(const QPointF &pivot, const qreal deltaAngle);
	virtual void scaleOn(const QPointF &pivot, const QPointF &factor);
	virtual void turnOn(const qreal angle);
	virtual void radiusOn(const qreal radius);

	virtual void setColor(const QColor &color);

	virtual QRectF boundingRect() const;
	virtual QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:

	// TODO
	static const int SIZE_POINT = 4;

	QPen m_pen, m_selectedPen;
	QBrush m_brush, m_selectedBrush;

	QRectF m_rect;
	qreal m_angle;
};

/*
@class WorldItemPath
@brief
@details
*/
class WorldItemPath: public AbstractWorldItem
{
public:
	WorldItemPath(const QPolygonF &polygon, QGraphicsItem *parent = 0);
	virtual ~WorldItemPath();

	virtual void moveOn(const QPointF &offset);
	virtual void rotateOn(const QPointF &pivot, const qreal deltaAngle);
	virtual void scaleOn(const QPointF &pivot, const QPointF &factor);
	virtual void turnOn(const qreal angle);
	virtual void radiusOn(const qreal radius);

	virtual void setColor(const QColor &color);

	virtual QRectF boundingRect() const;
	virtual QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	QPen m_pen, m_selectedPen;
	QPolygonF m_polygon;
};

/*
@class WorldItemZone
@brief
@details
*/
class WorldItemZone: public AbstractWorldItem
{
public:
	WorldItemZone(const QPolygonF &polygon, QGraphicsItem *parent = 0);
	virtual ~WorldItemZone();

	virtual void moveOn(const QPointF &offset);
	virtual void rotateOn(const QPointF &pivot, const qreal deltaAngle);
	virtual void scaleOn(const QPointF &pivot, const QPointF &factor);
	virtual void turnOn(const qreal angle);
	virtual void radiusOn(const qreal radius);

	virtual void setColor(const QColor &color);

	virtual QRectF boundingRect() const;
	virtual QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	static const int TRANSPARENCY = 28;

	QPen m_pen, m_selectedPen;
	QBrush m_brush, m_selectedBrush;
	QPolygonF m_polygon;
};

} /* namespace WorldEditor */

Q_DECLARE_METATYPE(WorldEditor::AbstractWorldItem *)

#endif // WORLD_EDITOR_SCENE_ITEM_H
