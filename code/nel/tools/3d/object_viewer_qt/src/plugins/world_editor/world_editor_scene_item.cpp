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

// Project includes
#include "world_editor_scene_item.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtCore/QRectF>
#include <QPolygonF>
#include <QTransform>
#include <QStyleOptionGraphicsItem>
#include <QPropertyAnimation>

namespace WorldEditor
{

static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
	// We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
	// if we pass a value of 0.0 to QPainterPathStroker::setWidth()
	const qreal penWidthZero = qreal(0.00000001);

	if (path == QPainterPath())
		return path;
	QPainterPathStroker ps;
	ps.setCapStyle(pen.capStyle());
	if (pen.widthF() <= 0.0)
		ps.setWidth(penWidthZero);
	else
		ps.setWidth(pen.widthF());
	ps.setJoinStyle(pen.joinStyle());
	ps.setMiterLimit(pen.miterLimit());
	QPainterPath p = ps.createStroke(path);
	p.addPath(path);
	return p;
}

AbstractWorldItem::AbstractWorldItem(QGraphicsItem *parent)
	: QGraphicsItem(parent),
	  m_active(false),
	  m_shapeChanged(false)
{
}

AbstractWorldItem::~AbstractWorldItem()
{
}

int AbstractWorldItem::type() const
{
	return Type;
}

void AbstractWorldItem::setActived(bool actived)
{
	m_active = actived;
}

bool AbstractWorldItem::isActived() const
{
	return m_active;
}

void AbstractWorldItem::setShapeChanged(bool value)
{
	m_shapeChanged = value;
}

bool AbstractWorldItem::isShapeChanged() const
{
	return m_shapeChanged;
}

WorldItemPoint::WorldItemPoint(const QPointF &point, const qreal angle, const qreal radius,
							   bool showArrow, QGraphicsItem *parent)
	: AbstractWorldItem(parent),
	  m_angle((2 * NLMISC::Pi - angle) * 180 / NLMISC::Pi),
	  m_radius(radius),
	  m_showArrow(showArrow)
{
	setZValue(WORLD_POINT_LAYER);

	//setFlag(ItemIgnoresTransformations);

	setPos(point);

	m_rect.setCoords(-SIZE_POINT, -SIZE_POINT, SIZE_POINT, SIZE_POINT);

	m_pen.setColor(QColor(255, 100, 10));
	//m_pen.setWidth(0);

	m_selectedPen.setColor(Qt::white);
	//m_selectedPen.setWidth(0);

	m_brush.setColor(QColor(255, 100, 10));
	m_brush.setStyle(Qt::SolidPattern);

	m_selectedBrush.setColor(Qt::white);
	m_selectedBrush.setStyle(Qt::SolidPattern);

	createCircle();

	// Create arrow
	if (showArrow)
	{
		m_arrow.push_back(QLine(0, 0, SIZE_ARROW, 0));
		m_arrow.push_back(QLine(SIZE_ARROW - 2, -2, SIZE_ARROW, 0));
		m_arrow.push_back(QLine(SIZE_ARROW - 2, 2, SIZE_ARROW, 0));
	}

	updateBoundingRect();
}

WorldItemPoint::~WorldItemPoint()
{
}

void WorldItemPoint::rotateOn(const QPointF &pivot, const qreal deltaAngle)
{
	prepareGeometryChange();

	QPolygonF rotatedPolygon(m_rect);

	rotatedPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.rotate(deltaAngle);
	rotatedPolygon = trans.map(rotatedPolygon);
	rotatedPolygon.translate(pivot);

	setPos(rotatedPolygon.boundingRect().center());
}

void WorldItemPoint::scaleOn(const QPointF &pivot, const QPointF &factor)
{
	prepareGeometryChange();

	QPolygonF scaledPolygon(m_rect);

	scaledPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.scale(factor.x(), factor.y());
	scaledPolygon = trans.map(scaledPolygon);
	scaledPolygon.translate(pivot);

	setPos(scaledPolygon.boundingRect().center());
}

void WorldItemPoint::turnOn(const qreal angle)
{
	m_angle += angle;
	update();
}

void WorldItemPoint::radiusOn(const qreal radius)
{
	if (m_radius == 0)
		return;
}

void WorldItemPoint::setColor(const QColor &color)
{
	m_pen.setColor(color);
	m_brush.setColor(color);
}

void WorldItemPoint::createCircle()
{
	if (m_radius != 0)
	{
		// Create circle
		int segmentCount = 30;
		QPointF circlePoint(m_radius, 0);
		m_circle << circlePoint;
		for (int i = 1; i < segmentCount + 1; ++i)
		{
			qreal angle = i * (2 * NLMISC::Pi / segmentCount);
			circlePoint.setX(cos(angle) * m_radius);
			circlePoint.setY(sin(angle) * m_radius);
			m_circle << circlePoint;
		}
	}
}

void WorldItemPoint::updateBoundingRect()
{
	m_boundingRect.setCoords(-SIZE_POINT, -SIZE_POINT, SIZE_POINT, SIZE_POINT);
	QRectF circleBoundingRect;
	circleBoundingRect.setCoords(-m_radius, -m_radius, m_radius, m_radius);
	m_boundingRect = m_boundingRect.united(circleBoundingRect);
}

QPainterPath WorldItemPoint::shape() const
{
	QPainterPath path;

	path.addRect(m_boundingRect);
	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

QRectF WorldItemPoint::boundingRect() const
{
	return m_boundingRect;
}

void WorldItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	// Here comes the magic:
	//painter->setClipRect(option->exposedRect);

	painter->setPen(m_pen);

	// Draw circle
	// Draws artefacts with using opengl painter
	// painter->drawEllipse(-m_radius / 2, -m_radius / 2, m_radius, m_radius);
	painter->drawPolygon(m_circle);

	painter->rotate(m_angle);

	// Draw arrow
	painter->drawLines(m_arrow);

	painter->setPen(Qt::NoPen);
//	if (option->state & QStyle::State_Selected)
	if (isActived())
		painter->setBrush(m_selectedBrush);
	else
		painter->setBrush(m_brush);

	// Draw point
	painter->drawRect(m_rect);
}

WorldItemPath::WorldItemPath(const QPolygonF &polygon, QGraphicsItem *parent)
	: AbstractWorldItem(parent),
	  m_polygon(polygon),
	  m_pointEdit(false)
{
	//setFlag(ItemIsSelectable);

	setZValue(WORLD_PATH_LAYER);

	m_pen.setColor(Qt::black);
	m_pen.setWidth(3);
	m_pen.setJoinStyle(Qt::MiterJoin);

	m_selectedPen.setColor(Qt::white);
	m_selectedPen.setWidth(3);
	m_selectedPen.setJoinStyle(Qt::MiterJoin);

	QPointF center = m_polygon.boundingRect().center();
	m_polygon.translate(-center);
	setPos(center);
}

WorldItemPath::~WorldItemPath()
{
}

void WorldItemPath::rotateOn(const QPointF &pivot, const qreal deltaAngle)
{
	prepareGeometryChange();

	QPolygonF rotatedPolygon(m_polygon);
	rotatedPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.rotate(deltaAngle);
	m_polygon = trans.map(rotatedPolygon);

	m_polygon.translate(pivot - pos());
}

void WorldItemPath::scaleOn(const QPointF &pivot, const QPointF &factor)
{
	prepareGeometryChange();

	QPolygonF scaledPolygon(m_polygon);
	scaledPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.scale(factor.x(), factor.y());
	m_polygon = trans.map(scaledPolygon);

	m_polygon.translate(pivot - pos());
}

void WorldItemPath::setColor(const QColor &color)
{
	m_pen.setColor(color);
}


void WorldItemPath::setEnabledSubPoints(bool enabled)
{
	m_pointEdit = enabled;
	if (m_pointEdit)
		createSubPoints();
	else
		removeSubPoints();
}

void WorldItemPath::moveSubPoint(WorldItemSubPoint *subPoint)
{
	prepareGeometryChange();

	QPolygonF polygon;

	// Update polygon
	Q_FOREACH(WorldItemSubPoint *node, m_listItems)
	{
		polygon << node->pos();
	}

	// Update middle points
	for (int i = 0; i < m_listLines.size(); ++i)
		m_listLines.at(i).itemPoint->setPos((m_listLines.at(i).lineItem.first->pos() + m_listLines.at(i).lineItem.second->pos()) / 2);

	m_polygon = polygon;
	update();
}

void WorldItemPath::addSubPoint(WorldItemSubPoint *subPoint)
{
	prepareGeometryChange();

	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (subPoint == m_listLines.at(i).itemPoint)
		{
			LineStruct oldLineItem = m_listLines[i];

			// Create the first middle sub-point
			WorldItemSubPoint *firstItem = new WorldItemSubPoint(WorldItemSubPoint::MiddleType, this);
			firstItem->setPos((oldLineItem.lineItem.first->pos() + subPoint->pos()) / 2);

			// Create the second middle sub-point
			WorldItemSubPoint *secondItem = new WorldItemSubPoint(WorldItemSubPoint::MiddleType, this);
			secondItem->setPos((oldLineItem.lineItem.second->pos() + subPoint->pos()) / 2);

			// Add first line in the list
			LineStruct firstNewLineItem;
			firstNewLineItem.itemPoint = firstItem;
			firstNewLineItem.lineItem = LineItem(oldLineItem.lineItem.first, subPoint);
			m_listLines.push_back(firstNewLineItem);

			// Add second line in the list
			LineStruct secondNewLineItem;
			secondNewLineItem.itemPoint = secondItem;
			secondNewLineItem.lineItem = LineItem(subPoint, oldLineItem.lineItem.second);
			m_listLines.push_back(secondNewLineItem);

			m_listLines.removeAt(i);

			int pos = m_listItems.indexOf(oldLineItem.lineItem.second);
			m_listItems.insert(pos, subPoint);
			subPoint->setFlag(ItemSendsScenePositionChanges);

			break;
		}
	}
}

bool WorldItemPath::removeSubPoint(WorldItemSubPoint *subPoint)
{
	int pos = m_listItems.indexOf(subPoint);

	// First and second points can not be removed
	if ((pos == 0) || (pos == m_listItems.size() - 1))
		return false;

	prepareGeometryChange();

	m_listItems.takeAt(pos);

	LineStruct newLineItem;

	newLineItem.itemPoint = subPoint;

	// Delete first line
	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (subPoint == m_listLines.at(i).lineItem.first)
		{
			// Saving second point for new line
			newLineItem.lineItem.second = m_listLines.at(i).lineItem.second;
			delete m_listLines.at(i).itemPoint;
			m_listLines.removeAt(i);
			break;
		}
	}

	// Delete second line
	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (subPoint == m_listLines.at(i).lineItem.second)
		{
			// Saving first point for new line
			newLineItem.lineItem.first = m_listLines.at(i).lineItem.first;
			delete m_listLines.at(i).itemPoint;
			m_listLines.removeAt(i);
			break;
		}
	}
	subPoint->setPos((newLineItem.lineItem.first->pos() + newLineItem.lineItem.second->pos()) / 2);
	m_listLines.push_back(newLineItem);
	subPoint->setFlag(ItemSendsScenePositionChanges, false);

	return true;
}

void WorldItemPath::setPolygon(const QPolygonF &polygon)
{
	prepareGeometryChange();

	m_polygon = polygon;

	update();
}

QPolygonF WorldItemPath::polygon() const
{
	return m_polygon;
}

QPainterPath WorldItemPath::shape() const
{
	QPainterPath path;

	path.moveTo(m_polygon.first());
	for (int i = 1; i < m_polygon.count(); ++i)
		path.lineTo(m_polygon.at(i));

	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

QRectF WorldItemPath::boundingRect() const
{
	return m_polygon.boundingRect();
}

void WorldItemPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	// Here comes the magic:
	//painter->setClipRect(option->exposedRect);

	//if (option->state & QStyle::State_Selected)
	if (isActived())
		painter->setPen(m_selectedPen);
	else
		painter->setPen(m_pen);

	painter->drawPolyline(m_polygon);
}

void WorldItemPath::createSubPoints()
{
	WorldItemSubPoint *firstPoint;
	firstPoint = new WorldItemSubPoint(WorldItemSubPoint::EdgeType, this);
	firstPoint->setPos(m_polygon.front());
	firstPoint->setFlag(ItemSendsScenePositionChanges);
	m_listItems.push_back(firstPoint);

	for (int i = 1; i < m_polygon.count(); ++i)
	{
		WorldItemSubPoint *secondPoint = new WorldItemSubPoint(WorldItemSubPoint::EdgeType, this);
		secondPoint->setPos(m_polygon.at(i));
		secondPoint->setFlag(ItemSendsScenePositionChanges);

		WorldItemSubPoint *middlePoint = new WorldItemSubPoint(WorldItemSubPoint::MiddleType, this);
		middlePoint->setPos((firstPoint->pos() + secondPoint->pos()) / 2);

		LineStruct newLineItem;
		newLineItem.itemPoint = middlePoint;
		newLineItem.lineItem = LineItem(firstPoint, secondPoint);
		m_listLines.push_back(newLineItem);

		firstPoint = secondPoint;
		m_listItems.push_back(firstPoint);
	}
}

void WorldItemPath::removeSubPoints()
{
	for (int i = 0; i < m_listLines.count(); ++i)
		delete m_listLines.at(i).itemPoint;

	for (int i = 0; i < m_listItems.count(); ++i)
		delete m_listItems.at(i);

	m_listItems.clear();
	m_listLines.clear();
}

WorldItemZone::WorldItemZone(const QPolygonF &polygon, QGraphicsItem *parent)
	: AbstractWorldItem(parent),
	  m_polygon(polygon),
	  m_pointEdit(false)
{
	//setFlag(ItemIsSelectable);

	setZValue(WORLD_ZONE_LAYER);

	m_pen.setColor(QColor(20, 100, 255));
	m_pen.setWidth(0);

	m_selectedPen.setColor(Qt::white);
	m_selectedPen.setWidth(0);

	m_brush.setColor(QColor(20, 100, 255, TRANSPARENCY));
	m_brush.setStyle(Qt::SolidPattern);

	m_selectedBrush.setColor(QColor(255, 255, 255, 100));
	m_selectedBrush.setStyle(Qt::SolidPattern);

	QPointF center = m_polygon.boundingRect().center();
	m_polygon.translate(-center);
	setPos(center);
}

WorldItemZone::~WorldItemZone()
{
}

void WorldItemZone::rotateOn(const QPointF &pivot, const qreal deltaAngle)
{
	prepareGeometryChange();

	QPolygonF rotatedPolygon(m_polygon);
	rotatedPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.rotate(deltaAngle);
	m_polygon = trans.map(rotatedPolygon);

	m_polygon.translate(pivot - pos());
}

void WorldItemZone::scaleOn(const QPointF &pivot, const QPointF &factor)
{
	prepareGeometryChange();

	QPolygonF scaledPolygon(m_polygon);
	scaledPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.scale(factor.x(), factor.y());
	m_polygon = trans.map(scaledPolygon);

	m_polygon.translate(pivot - pos());
}

void WorldItemZone::setColor(const QColor &color)
{
	m_pen.setColor(color);

	QColor brushColor(color);
	brushColor.setAlpha(TRANSPARENCY);

	m_brush.setColor(brushColor);
}

void WorldItemZone::setEnabledSubPoints(bool enabled)
{
	m_pointEdit = enabled;
	if (m_pointEdit)
		createSubPoints();
	else
		removeSubPoints();

	setShapeChanged(false);
}

void WorldItemZone::moveSubPoint(WorldItemSubPoint *subPoint)
{
	prepareGeometryChange();

	QPolygonF polygon;

	// Update polygon
	Q_FOREACH(WorldItemSubPoint *node, m_listItems)
	{
		polygon << node->pos();
	}

	// Update middle points
	for (int i = 0; i < m_listLines.size(); ++i)
		m_listLines.at(i).itemPoint->setPos((m_listLines.at(i).lineItem.first->pos() + m_listLines.at(i).lineItem.second->pos()) / 2);

	m_polygon = polygon;
	update();

	setShapeChanged(true);
}

void WorldItemZone::addSubPoint(WorldItemSubPoint *subPoint)
{
	prepareGeometryChange();

	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (subPoint == m_listLines.at(i).itemPoint)
		{
			LineStruct oldLineItem = m_listLines[i];

			// Create the first middle sub-point
			WorldItemSubPoint *firstItem = new WorldItemSubPoint(WorldItemSubPoint::MiddleType, this);
			firstItem->setPos((oldLineItem.lineItem.first->pos() + subPoint->pos()) / 2);

			// Create the second middle sub-point
			WorldItemSubPoint *secondItem = new WorldItemSubPoint(WorldItemSubPoint::MiddleType, this);
			secondItem->setPos((oldLineItem.lineItem.second->pos() + subPoint->pos()) / 2);

			// Add first line in the list
			LineStruct firstNewLineItem;
			firstNewLineItem.itemPoint = firstItem;
			firstNewLineItem.lineItem = LineItem(oldLineItem.lineItem.first, subPoint);
			m_listLines.push_back(firstNewLineItem);

			// Add second line in the list
			LineStruct secondNewLineItem;
			secondNewLineItem.itemPoint = secondItem;
			secondNewLineItem.lineItem = LineItem(subPoint, oldLineItem.lineItem.second);
			m_listLines.push_back(secondNewLineItem);

			m_listLines.removeAt(i);

			int pos = m_listItems.indexOf(oldLineItem.lineItem.second);
			m_listItems.insert(pos, subPoint);
			subPoint->setFlag(ItemSendsScenePositionChanges);

			break;
		}
	}
	setShapeChanged(true);
}

bool WorldItemZone::removeSubPoint(WorldItemSubPoint *subPoint)
{
	prepareGeometryChange();

	if (m_listItems.size() < 4)
		return false;

	int pos = m_listItems.indexOf(subPoint);
	m_listItems.takeAt(pos);

	LineStruct newLineItem;

	newLineItem.itemPoint = subPoint;

	// Delete first line
	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (subPoint == m_listLines.at(i).lineItem.first)
		{
			// Saving second point for new line
			newLineItem.lineItem.second = m_listLines.at(i).lineItem.second;
			delete m_listLines.at(i).itemPoint;
			m_listLines.removeAt(i);
			break;
		}
	}

	// Delete second line
	for (int i = 0; i < m_listLines.size(); ++i)
	{
		if (subPoint == m_listLines.at(i).lineItem.second)
		{
			// Saving first point for new line
			newLineItem.lineItem.first = m_listLines.at(i).lineItem.first;
			delete m_listLines.at(i).itemPoint;
			m_listLines.removeAt(i);
			break;
		}
	}
	m_listLines.push_back(newLineItem);
	subPoint->setPos((newLineItem.lineItem.first->pos() + newLineItem.lineItem.second->pos()) / 2);
	subPoint->setFlag(ItemSendsScenePositionChanges, false);

	setShapeChanged(true);

	return true;
}

void WorldItemZone::setPolygon(const QPolygonF &polygon)
{
	prepareGeometryChange();

	m_polygon = polygon;

	update();
}

QPolygonF WorldItemZone::polygon() const
{
	return m_polygon;
}

QRectF WorldItemZone::boundingRect() const
{
	return m_polygon.boundingRect();
}

QPainterPath WorldItemZone::shape() const
{
	QPainterPath path;
	path.addPolygon(m_polygon);
	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

void WorldItemZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
//	if (option->state & QStyle::State_Selected)
	if (isActived())
	{
		painter->setPen(m_selectedPen);
		painter->setBrush(m_selectedBrush);
	}
	else
	{
		painter->setPen(m_pen);
		painter->setBrush(m_brush);
	}

	painter->drawPolygon(m_polygon);
}

void WorldItemZone::createSubPoints()
{
	WorldItemSubPoint *firstPoint;
	firstPoint = new WorldItemSubPoint(WorldItemSubPoint::EdgeType, this);
	firstPoint->setPos(m_polygon.front());
	firstPoint->setFlag(ItemSendsScenePositionChanges);
	m_listItems.push_back(firstPoint);

	for (int i = 1; i < m_polygon.count(); ++i)
	{
		WorldItemSubPoint *secondPoint = new WorldItemSubPoint(WorldItemSubPoint::EdgeType, this);
		secondPoint->setPos(m_polygon.at(i));
		secondPoint->setFlag(ItemSendsScenePositionChanges);

		WorldItemSubPoint *middlePoint = new WorldItemSubPoint(WorldItemSubPoint::MiddleType, this);
		middlePoint->setPos((firstPoint->pos() + secondPoint->pos()) / 2);

		LineStruct newLineItem;
		newLineItem.itemPoint = middlePoint;
		newLineItem.lineItem = LineItem(firstPoint, secondPoint);
		m_listLines.push_back(newLineItem);

		firstPoint = secondPoint;
		m_listItems.push_back(firstPoint);
	}

	LineStruct endLineItem;
	endLineItem.itemPoint = new WorldItemSubPoint(WorldItemSubPoint::MiddleType, this);
	endLineItem.itemPoint->setPos((m_listItems.first()->pos() + m_listItems.last()->pos()) / 2);
	endLineItem.lineItem = LineItem(m_listItems.last(), m_listItems.first());
	m_listLines.push_back(endLineItem);
}

void WorldItemZone::removeSubPoints()
{
	for (int i = 0; i < m_listLines.count(); ++i)
		delete m_listLines.at(i).itemPoint;

	for (int i = 0; i < m_listItems.count(); ++i)
		delete m_listItems.at(i);

	m_listItems.clear();
	m_listLines.clear();
}

//*******************************************

WorldItemSubPoint::WorldItemSubPoint(SubPointType pointType, AbstractWorldItem *parent)
	: QGraphicsObject(parent),
	  m_type(pointType),
	  m_active(false),
	  m_parent(parent)
{
	setZValue(WORLD_POINT_LAYER);

	m_brush.setColor(QColor(20, 100, 255));
	m_brush.setStyle(Qt::SolidPattern);

	m_brushMiddle.setColor(QColor(255, 25, 100));
	m_brushMiddle.setStyle(Qt::SolidPattern);

	m_selectedBrush.setColor(QColor(255, 255, 255, 100));
	m_selectedBrush.setStyle(Qt::SolidPattern);

	m_rect.setCoords(-SIZE_POINT, -SIZE_POINT, SIZE_POINT, SIZE_POINT);
	updateBoundingRect();

	//setFlag(ItemIgnoresTransformations);
	//setFlag(ItemSendsScenePositionChanges);
}

WorldItemSubPoint::~WorldItemSubPoint()
{
}

void WorldItemSubPoint::setSubPointType(SubPointType nodeType)
{
	m_type = nodeType;
	setFlag(ItemSendsScenePositionChanges);
}

WorldItemSubPoint::SubPointType WorldItemSubPoint::subPointType() const
{
	return m_type;
}

void WorldItemSubPoint::rotateOn(const QPointF &pivot, const qreal deltaAngle)
{
	prepareGeometryChange();

	QPolygonF rotatedPolygon(m_rect);

	// TODO
	rotatedPolygon.translate(scenePos() - pivot);

	QTransform trans;
	trans = trans.rotate(deltaAngle);
	rotatedPolygon = trans.map(rotatedPolygon);
	rotatedPolygon.translate(pivot);

	setPos(m_parent->mapFromParent(rotatedPolygon.boundingRect().center()));
}

void WorldItemSubPoint::scaleOn(const QPointF &pivot, const QPointF &factor)
{
	prepareGeometryChange();

	QPolygonF scaledPolygon(m_rect);

	// TODO
	scaledPolygon.translate(scenePos() - pivot);
	//scaledPolygon.translate(-pivot);

	QTransform trans;
	trans = trans.scale(factor.x(), factor.y());
	scaledPolygon = trans.map(scaledPolygon);
	scaledPolygon.translate(pivot);

	setPos(m_parent->mapFromParent(scaledPolygon.boundingRect().center()));
}

QRectF WorldItemSubPoint::boundingRect() const
{
	return m_boundingRect;
}

void WorldItemSubPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->setPen(Qt::NoPen);

	if (m_type == WorldItemSubPoint::EdgeType)
	{
		if (isActived())
			painter->setBrush(m_selectedBrush);
		else
			painter->setBrush(m_brush);
	}
	else
	{
		painter->setBrush(m_brushMiddle);
	}

	// Draw point
	painter->drawRect(m_rect);
}

int WorldItemSubPoint::type() const
{
	return Type;
}

void WorldItemSubPoint::setActived(bool actived)
{
	m_active = actived;
}

bool WorldItemSubPoint::isActived() const
{
	return m_active;
}

QVariant WorldItemSubPoint::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionHasChanged)
		m_parent->moveSubPoint(this);
	return QGraphicsItem::itemChange(change, value);
}

void WorldItemSubPoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if ((m_type == MiddleType) && (event->button() == Qt::LeftButton))
	{
		m_parent->addSubPoint(this);
		setSubPointType(EdgeType);
	}
	else if ((m_type == EdgeType) && (event->button() == Qt::RightButton))
	{
		if (m_parent->removeSubPoint(this))
			setSubPointType(MiddleType);
	}
	update();
}

void WorldItemSubPoint::updateBoundingRect()
{
	m_boundingRect.setCoords(-SIZE_POINT, -SIZE_POINT, SIZE_POINT, SIZE_POINT);
}

} /* namespace WorldEditor */
