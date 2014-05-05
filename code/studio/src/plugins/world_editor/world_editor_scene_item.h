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
class WorldItemSubPoint;

typedef QPair<WorldItemSubPoint *, WorldItemSubPoint *> LineItem;

struct LineStruct
{
	WorldItemSubPoint *itemPoint;
	LineItem lineItem;
};

const int SELECTED_LAYER = 200;
const int UNSELECTED_LAYER = 100;
const int WORLD_ZONE_LAYER = 100;
const int WORLD_POINT_LAYER = 200;
const int WORLD_PATH_LAYER = 200;
const int MIDDLE_POINT_LAYER = 201;
const int EDGE_POINT_LAYER = 201;

const int SIZE_ARROW = 20;

/*
@class AbstractWorldItem
@brief Abstract class for graphics item
@details
*/
class AbstractWorldItem: public QGraphicsItem
{
public:
	AbstractWorldItem(QGraphicsItem *parent = 0);
	virtual ~AbstractWorldItem();

	enum { Type = QGraphicsItem::UserType + 1 };

	/// Rotate item around @pivot point on &deltaAngle (deg).
	virtual void rotateOn(const QPointF &pivot, const qreal deltaAngle) {}

	/// Scales item relatively @pivot point
	// TODO: add modes: IgnoreAspectRatio, KeepAspectRatio
	virtual void scaleOn(const QPointF &pivot, const QPointF &factor) {}

	/// Rotate arrow on angle (deg). (only for WorldItemPoint)
	virtual void turnOn(const qreal angle) {}
	virtual void radiusOn(const qreal radius) {}

	/// Change color
	virtual void setColor(const QColor &color) {}

	/// Enable/disable the mode edit shape (only for WorldItemPath and WorldItemPath)
	virtual void setEnabledSubPoints(bool enabled) {}

	virtual void moveSubPoint(WorldItemSubPoint *subPoint) {}
	virtual void addSubPoint(WorldItemSubPoint *subPoint) {}
	virtual bool removeSubPoint(WorldItemSubPoint *subPoint)
	{
		return false;
	}

	virtual void setPolygon(const QPolygonF &polygon) {}
	virtual QPolygonF polygon() const
	{
		return QPolygonF();
	}

	void setActived(bool actived);
	bool isActived() const;

	void setShapeChanged(bool value);
	bool isShapeChanged() const;

	// Enable the use of qgraphicsitem_cast with this item.
	int type() const;

protected:

	bool m_active, m_shapeChanged;
};

/*
@class WorldItemPoint
@brief WorldItemPoint class provides a dot item with arrow and circle(@radius)
that you can add to a WorldEditorScene.
@details
*/
class WorldItemPoint: public AbstractWorldItem
{
public:
	WorldItemPoint(const QPointF &point, const qreal angle, const qreal radius,
				   bool showArrow, QGraphicsItem *parent = 0);
	virtual ~WorldItemPoint();

	qreal angle() const;

	virtual void rotateOn(const QPointF &pivot, const qreal deltaAngle);
	virtual void scaleOn(const QPointF &pivot, const QPointF &factor);
	virtual void turnOn(const qreal angle);
	virtual void radiusOn(const qreal radius);

	virtual void setColor(const QColor &color);
	virtual void setEnabledSubPoints(bool enabled) {}

	virtual void setPolygon(const QPolygonF &polygon);
	virtual QPolygonF polygon() const;

	virtual QRectF boundingRect() const;
	virtual QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
	void createCircle();
	void updateBoundingRect();

	static const int SIZE_POINT = 2;

	QPen m_pen, m_selectedPen;
	QBrush m_brush, m_selectedBrush;

	QPolygonF m_circle;
	QVector<QLine> m_arrow;
	QRectF m_rect, m_boundingRect;
	qreal m_angle, m_radius;
	bool m_showArrow;
};

/*
@class BaseWorldItemPolyline
@brief
@details
*/
class BaseWorldItemPolyline: public AbstractWorldItem
{
public:
	BaseWorldItemPolyline(const QPolygonF &polygon, QGraphicsItem *parent = 0);
	virtual ~BaseWorldItemPolyline();

	virtual void rotateOn(const QPointF &pivot, const qreal deltaAngle);
	virtual void scaleOn(const QPointF &pivot, const QPointF &factor);

	virtual void setEnabledSubPoints(bool enabled);
	virtual void moveSubPoint(WorldItemSubPoint *subPoint);
	virtual void addSubPoint(WorldItemSubPoint *subPoint);
	virtual bool removeSubPoint(WorldItemSubPoint *subPoint);

	virtual void setPolygon(const QPolygonF &polygon);
	virtual QPolygonF polygon() const;

	virtual QRectF boundingRect() const;

protected:
	virtual void createSubPoints();
	virtual void removeSubPoints();

	bool m_pointEdit;
	QPolygonF m_polyline;
	QPen m_pen, m_selectedPen;

	QList<WorldItemSubPoint *> m_listItems;
	QList<LineStruct> m_listLines;
};

/*
@class WorldItemPath
@brief WorldItemPath class provides a polyline item that you can add to a WorldEditorScene.
@details
*/
class WorldItemPath: public BaseWorldItemPolyline
{
public:
	WorldItemPath(const QPolygonF &polygon, QGraphicsItem *parent = 0);
	virtual ~WorldItemPath();

	virtual void setColor(const QColor &color);
	virtual bool removeSubPoint(WorldItemSubPoint *subPoint);
	virtual QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:

	QPen m_pen, m_selectedPen;
};

/*
@class WorldItemZone
@brief The WorldItemZone class provides a polygon item that you can add to a WorldEditorScene.
@details
*/
class WorldItemZone: public BaseWorldItemPolyline
{
public:
	WorldItemZone(const QPolygonF &polygon, QGraphicsItem *parent = 0);
	virtual ~WorldItemZone();

	virtual void setColor(const QColor &color);
	virtual bool removeSubPoint(WorldItemSubPoint *subPoint);
	virtual QPainterPath shape() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
	virtual void createSubPoints();

private:
	static const int TRANSPARENCY = 38;

	QPen m_pen, m_selectedPen;
	QBrush m_brush, m_selectedBrush;
};

/*
@class WorldItemSubPoint
@brief
@details
*/
class WorldItemSubPoint: public QGraphicsObject
{
	Q_OBJECT
public:
	enum SubPointType
	{
		EdgeType = 0,
		MiddleType
	};

	enum { Type = QGraphicsItem::UserType + 2 };

	WorldItemSubPoint(SubPointType pointType, AbstractWorldItem *parent = 0);
	virtual ~WorldItemSubPoint();

	void setSubPointType(SubPointType nodeType);
	SubPointType subPointType() const;

	void rotateOn(const QPointF &pivot, const qreal deltaAngle);
	void scaleOn(const QPointF &pivot, const QPointF &factor);

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	void setActived(bool actived);
	bool isActived() const;

	// Enable the use of qgraphicsitem_cast with this item.
	int type() const;

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
	void updateBoundingRect();

	static const int SIZE_POINT = 6;

	QBrush m_brush, m_brushMiddle, m_selectedBrush;

	QRectF m_rect, m_boundingRect;
	SubPointType m_type;
	bool m_active;
	AbstractWorldItem *m_parent;
};

} /* namespace WorldEditor */

// Enable the use of QVariant with this class.
Q_DECLARE_METATYPE(WorldEditor::AbstractWorldItem *)

#endif // WORLD_EDITOR_SCENE_ITEM_H
