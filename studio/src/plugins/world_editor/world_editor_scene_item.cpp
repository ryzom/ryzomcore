// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
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
#include "world_editor_constants.h"

#include "../core/icore.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtCore/QRectF>
#include <QPolygonF>
#include <QTransform>
#include <QStyleOptionGraphicsItem>
#include <QPropertyAnimation>
#include <QSettings>

#include <cmath>

namespace WorldEditor
{

namespace
{
/// Default when the setting is absent: a strong magenta. It has to stand out from the
/// sandy zone bitmaps as well as from the orange points, the red paths and the green
/// collision shapes, and white did not.
const char *const DEFAULT_SELECTION_COLOR = "#ff28c8";

bool g_visibleCollisions = true;
}

QColor selectionColor()
{
	// Read once. Changing it takes a restart, which is fine for a colour.
	static QColor color;
	if (!color.isValid())
	{
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(Constants::WORLD_EDITOR_SECTION);
		const QString name = settings->value(Constants::SELECTION_COLOR,
											 QString(DEFAULT_SELECTION_COLOR)).toString();
		settings->endGroup();

		color = QColor(name);
		if (!color.isValid())
		{
			nlwarning("World Editor: [%s] %s is not a colour ('%s'), using %s.",
					  Constants::WORLD_EDITOR_SECTION, Constants::SELECTION_COLOR,
					  name.toUtf8().constData(), DEFAULT_SELECTION_COLOR);
			color = QColor(QString(DEFAULT_SELECTION_COLOR));
		}
	}
	return color;
}

bool isVisibleCollisions()
{
	return g_visibleCollisions;
}

void setVisibleCollisions(bool visible)
{
	g_visibleCollisions = visible;
}

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
							   bool showArrow, const CollisionShape &collision,
							   QGraphicsItem *parent)
	: AbstractWorldItem(parent),
	  m_angle(angle),
	  m_radius(radius),
	  m_showArrow(showArrow),
	  m_collision(collision),
	  m_lastSymbolScale(1.0)
{
	setZValue(WORLD_POINT_LAYER);

	//setFlag(ItemIgnoresTransformations);

	setPos(point);

	m_rect.setCoords(-SIZE_POINT, -SIZE_POINT, SIZE_POINT, SIZE_POINT);

	m_pen.setColor(QColor(255, 100, 10));
	m_pen.setWidth(0);

	m_selectedPen.setColor(selectionColor());
	m_selectedPen.setWidth(0);

	m_brush.setColor(QColor(255, 100, 10));
	m_brush.setStyle(Qt::SolidPattern);

	m_selectedBrush.setColor(selectionColor());
	m_selectedBrush.setStyle(Qt::SolidPattern);

	m_collisionPen.setColor(QColor(120, 230, 90));
	m_collisionPen.setWidth(0);
	m_collisionBrush.setColor(QColor(120, 230, 90, 40));
	m_collisionBrush.setStyle(Qt::SolidPattern);

	createCircle();
	createCollisionShape();

	updateBoundingRect();
}

WorldItemPoint::~WorldItemPoint()
{
}

qreal WorldItemPoint::angle() const
{
	return m_angle;
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
	m_angle -= floor(m_angle / 360) * 360;
	update();
}

void WorldItemPoint::radiusOn(const qreal radius)
{
	if (m_radius == 0)
		return;

	// TODO: implement
}

qreal WorldItemPoint::radius() const
{
	return m_radius;
}

void WorldItemPoint::setRadius(qreal radius)
{
	if (radius == m_radius)
		return;

	prepareGeometryChange();
	m_radius = qMax(qreal(0), radius);
	m_circle.clear();
	createCircle();
	updateBoundingRect();
	update();
}

void WorldItemPoint::setColor(const QColor &color)
{
	m_pen.setColor(color);
	m_brush.setColor(color);
}

void WorldItemPoint::setPolygon(const QPolygonF &polygon)
{
}

QPolygonF WorldItemPoint::polygon() const
{
	QPolygonF polygon;
	polygon << QPointF(0, 0);
	return polygon;
}

void WorldItemPoint::createCollisionShape()
{
	// Built as a polygon for the same reason as the radius circle above: drawEllipse()
	// leaves artefacts with the OpenGL painter.
	if (m_collision.Kind_ == CollisionShape::Circle)
	{
		const int segmentCount = 24;
		for (int i = 0; i < segmentCount + 1; ++i)
		{
			const qreal angle = i * (2 * NLMISC::Pi / segmentCount);
			m_collisionShape << QPointF(cos(angle) * m_collision.Radius,
										sin(angle) * m_collision.Radius);
		}
	}
	else if (m_collision.Kind_ == CollisionShape::Box)
	{
		// Length runs along the direction the entity faces, width across it.
		const qreal halfLength = m_collision.Length / 2.0;
		const qreal halfWidth = m_collision.Width / 2.0;
		m_collisionShape << QPointF(-halfLength, -halfWidth)
						 << QPointF(halfLength, -halfWidth)
						 << QPointF(halfLength, halfWidth)
						 << QPointF(-halfLength, halfWidth)
						 << QPointF(-halfLength, -halfWidth);
	}
}

void WorldItemPoint::createCircle()
{
	if (m_radius != 0)
	{
		// Create circle
		int segmentCount = 20;
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
	// The marker and the arrow grow by up to MAX_SYMBOL_SCALE when zoomed out, and the
	// arrow turns with the item, so reserve its full length in every direction. Leaving
	// the arrow out of the rectangle - as this did before - smears it across the view.
	const qreal symbolExtent = m_showArrow
			? qMax(qreal(SIZE_ARROW) * MAX_SYMBOL_SCALE, arrowWorldReach())
			: qreal(SIZE_POINT) * MAX_SYMBOL_SCALE;
	m_boundingRect.setCoords(-symbolExtent, -symbolExtent, symbolExtent, symbolExtent);

	QRectF circleBoundingRect;
	circleBoundingRect.setCoords(-m_radius, -m_radius, m_radius, m_radius);
	m_boundingRect = m_boundingRect.united(circleBoundingRect);

	if (!m_collisionShape.isEmpty())
	{
		// Rotated with the item, so a square around the longest extent covers every angle.
		const QRectF shapeRect = m_collisionShape.boundingRect();
		const qreal reach = qMax(qAbs(shapeRect.left()), qMax(qAbs(shapeRect.right()),
						   qMax(qAbs(shapeRect.top()), qAbs(shapeRect.bottom()))));
		QRectF collisionBoundingRect;
		collisionBoundingRect.setCoords(-reach, -reach, reach, reach);
		m_boundingRect = m_boundingRect.united(collisionBoundingRect);
	}
}

qreal WorldItemPoint::arrowWorldReach() const
{
	if (m_collisionShape.isEmpty())
		return 0;
	const QRectF shapeRect = m_collisionShape.boundingRect();
	const qreal reach = qMax(qAbs(shapeRect.left()), qMax(qAbs(shapeRect.right()),
					   qMax(qAbs(shapeRect.top()), qAbs(shapeRect.bottom()))));
	return reach * 1.5;
}

QPainterPath WorldItemPoint::shape() const
{
	// Only what the user actually sees: the marker at the size it was last drawn with,
	// plus the radius circle if the primitive has one. boundingRect() also reserves room
	// for the direction arrow at its largest, and using that here meant a click anywhere
	// within eight arrow lengths picked whichever point came first in z order.
	QPainterPath path;

	const qreal markerExtent = qreal(SIZE_POINT) * m_lastSymbolScale;
	QRectF markerRect;
	markerRect.setCoords(-markerExtent, -markerExtent, markerExtent, markerExtent);
	path.addRect(markerRect);

	if (m_radius != 0)
		path.addEllipse(QPointF(0.0, 0.0), m_radius, m_radius);

	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

QRectF WorldItemPoint::boundingRect() const
{
	return m_boundingRect;
}

QRectF WorldItemPoint::worldRect() const
{
	// What the primitive really covers: its radius, and the footprint from the sheet.
	// The marker and the arrow are drawn at a fixed size on screen and say nothing about
	// how much room the thing takes in the world, so they are left out.
	QRectF rect;

	if (m_radius != 0)
		rect.setCoords(-m_radius, -m_radius, m_radius, m_radius);

	if (!m_collisionShape.isEmpty())
		rect = rect.isNull() ? m_collisionShape.boundingRect()
							 : rect.united(m_collisionShape.boundingRect());

	return rect;
}

void WorldItemPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	painter->setPen(m_pen);

	// The radius is a real world size and therefore scales along.
	// Draws artefacts with using opengl painter
	// painter->drawEllipse(-m_radius / 2, -m_radius / 2, m_radius, m_radius);
	painter->drawPolygon(m_circle);

	// The collision footprint from the sheet is a real world size, so it is drawn
	// unscaled - seeing how much room the object really takes is the point of it. The
	// box turns with the entity, the circle does not need to.
	if (!m_collisionShape.isEmpty() && isVisibleCollisions())
	{
		painter->save();
		if (m_collision.Kind_ == CollisionShape::Box)
			painter->rotate(m_angle);
		painter->setPen(m_collisionPen);
		painter->setBrush(m_collisionBrush);
		painter->drawPolygon(m_collisionShape);
		painter->restore();
	}

	// The marker and the direction arrow, in contrast, are pure control symbols and should
	// keep the same size on screen at every zoom level. Scaling by 1/lod does that in both
	// directions; the cap keeps them inside the room boundingRect() reserves.
	const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
	const qreal scale = qMin(1.0 / lod, MAX_SYMBOL_SCALE);
	m_lastSymbolScale = scale;

	painter->save();
	painter->rotate(m_angle);

	if (m_showArrow)
	{
		// SIZE_ARROW pixels up to 1 pixel per metre; zoomed in further it grows to twice
		// that, and it always reaches past the collision footprint. The head keeps a
		// readable size on screen - it used to be 2 pixels and gone next to the marker.
		const qreal zoomIn = qBound(1.0, 1.0 + std::log(lod) / std::log(2.0) / 4.0, 2.0);
		const qreal length = qMax(SIZE_ARROW * zoomIn * scale, arrowWorldReach());
		const qreal head = 7.0 * zoomIn * scale;
		const QLineF arrow[3] =
		{
			QLineF(0, 0, length, 0),
			QLineF(length - head, -head * 0.6, length, 0),
			QLineF(length - head, head * 0.6, length, 0)
		};
		QPen arrowPen = painter->pen();
		arrowPen.setCosmetic(true);
		arrowPen.setWidthF(lod > 1.0 ? 2.0 : 1.0);
		painter->setPen(arrowPen);
		painter->drawLines(arrow, 3);
	}

	painter->scale(scale, scale);
	painter->setPen(Qt::NoPen);
	if (isActived())
		painter->setBrush(m_selectedBrush);
	else
		painter->setBrush(m_brush);

	// Draw point
	painter->drawRect(m_rect);
	painter->restore();
}

BaseWorldItemPolyline::BaseWorldItemPolyline(const QPolygonF &polygon, QGraphicsItem *parent)
	: AbstractWorldItem(parent),
	  m_polyline(polygon),
	  m_pointEdit(false)
{
	//setFlag(ItemIsSelectable);
	QPointF center = m_polyline.boundingRect().center();
	m_polyline.translate(-center);
	setPos(center);
}

BaseWorldItemPolyline::~BaseWorldItemPolyline()
{
}

void BaseWorldItemPolyline::rotateOn(const QPointF &pivot, const qreal deltaAngle)
{
	prepareGeometryChange();

	QPolygonF rotatedPolygon(m_polyline);
	rotatedPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.rotate(deltaAngle);
	m_polyline = trans.map(rotatedPolygon);

	m_polyline.translate(pivot - pos());
}

void BaseWorldItemPolyline::scaleOn(const QPointF &pivot, const QPointF &factor)
{
	prepareGeometryChange();

	QPolygonF scaledPolygon(m_polyline);
	scaledPolygon.translate(pos() - pivot);

	QTransform trans;
	trans = trans.scale(factor.x(), factor.y());
	m_polyline = trans.map(scaledPolygon);

	m_polyline.translate(pivot - pos());
}

QRectF BaseWorldItemPolyline::worldRect() const
{
	return m_polyline.boundingRect();
}

void BaseWorldItemPolyline::setEnabledSubPoints(bool enabled)
{
	m_pointEdit = enabled;
	if (m_pointEdit)
		createSubPoints();
	else
		removeSubPoints();

	setShapeChanged(false);
}

void BaseWorldItemPolyline::moveSubPoint(WorldItemSubPoint *subPoint)
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

	m_polyline = polygon;
	setShapeChanged(true);
	update();
}

void BaseWorldItemPolyline::addSubPoint(WorldItemSubPoint *subPoint)
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

bool BaseWorldItemPolyline::removeSubPoint(WorldItemSubPoint *subPoint)
{
	prepareGeometryChange();

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
	subPoint->setPos((newLineItem.lineItem.first->pos() + newLineItem.lineItem.second->pos()) / 2);
	m_listLines.push_back(newLineItem);
	subPoint->setFlag(ItemSendsScenePositionChanges, false);
	setShapeChanged(true);
	return true;
}

void BaseWorldItemPolyline::setPolygon(const QPolygonF &polygon)
{
	prepareGeometryChange();
	m_polyline = polygon;
	update();
}

QPolygonF BaseWorldItemPolyline::polygon() const
{
	return m_polyline;
}

QRectF BaseWorldItemPolyline::boundingRect() const
{
	return m_polyline.boundingRect();
}

void BaseWorldItemPolyline::createSubPoints()
{
	WorldItemSubPoint *firstPoint;
	firstPoint = new WorldItemSubPoint(WorldItemSubPoint::EdgeType, this);
	firstPoint->setPos(m_polyline.front());
	firstPoint->setFlag(ItemSendsScenePositionChanges);
	m_listItems.push_back(firstPoint);

	for (int i = 1; i < m_polyline.count(); ++i)
	{
		WorldItemSubPoint *secondPoint = new WorldItemSubPoint(WorldItemSubPoint::EdgeType, this);
		secondPoint->setPos(m_polyline.at(i));
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

void BaseWorldItemPolyline::removeSubPoints()
{
	for (int i = 0; i < m_listLines.count(); ++i)
		delete m_listLines.at(i).itemPoint;

	for (int i = 0; i < m_listItems.count(); ++i)
		delete m_listItems.at(i);

	m_listItems.clear();
	m_listLines.clear();
}

WorldItemPath::WorldItemPath(const QPolygonF &polygon, QGraphicsItem *parent)
	: BaseWorldItemPolyline(polygon, parent)
{
	setZValue(WORLD_PATH_LAYER);

	m_pen.setColor(Qt::black);
	// Width 0 = cosmetic pen: always one screen line wide, however far the view is
	// zoomed in. A width of 3 means 3 world metres, and zooming in
	// turns paths and routes into wide bands.
	m_pen.setWidth(0);
	m_pen.setJoinStyle(Qt::MiterJoin);

	m_selectedPen.setColor(selectionColor());
	m_selectedPen.setWidth(0);
	m_selectedPen.setJoinStyle(Qt::MiterJoin);
}

WorldItemPath::~WorldItemPath()
{
}

void WorldItemPath::setColor(const QColor &color)
{
	m_pen.setColor(color);
}

bool WorldItemPath::removeSubPoint(WorldItemSubPoint *subPoint)
{
	int pos = m_listItems.indexOf(subPoint);

	// First and second points can not be removed
	if ((pos == 0) || (pos == m_listItems.size() - 1))
		return false;

	return BaseWorldItemPolyline::removeSubPoint(subPoint);
}

QPainterPath WorldItemPath::shape() const
{
	QPainterPath path;

	path.moveTo(m_polyline.first());
	for (int i = 1; i < m_polyline.count(); ++i)
		path.lineTo(m_polyline.at(i));

	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

void WorldItemPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	if (isActived())
		painter->setPen(m_selectedPen);
	else
		painter->setPen(m_pen);

	painter->drawPolyline(m_polyline);
}


WorldItemZone::WorldItemZone(const QPolygonF &polygon, QGraphicsItem *parent)
	: BaseWorldItemPolyline(polygon, parent)
{
	setZValue(WORLD_ZONE_LAYER);

	m_pen.setColor(QColor(20, 100, 255));
	m_pen.setWidth(0);
	m_selectedPen.setColor(selectionColor());
	m_selectedPen.setWidth(0);
	m_brush.setColor(QColor(20, 100, 255, TRANSPARENCY));
	m_brush.setStyle(Qt::SolidPattern);
	m_selectedBrush.setColor(QColor(selectionColor().red(), selectionColor().green(),
									selectionColor().blue(), 100));
	m_selectedBrush.setStyle(Qt::SolidPattern);
}

WorldItemZone::~WorldItemZone()
{
}

void WorldItemZone::setColor(const QColor &color)
{
	m_pen.setColor(color);
	QColor brushColor(color);
	brushColor.setAlpha(TRANSPARENCY);
	m_brush.setColor(brushColor);
}

bool WorldItemZone::removeSubPoint(WorldItemSubPoint *subPoint)
{
	if (m_listItems.size() < 4)
		return false;

	return BaseWorldItemPolyline::removeSubPoint(subPoint);
}

QPainterPath WorldItemZone::shape() const
{
	QPainterPath path;
	path.addPolygon(m_polyline);
	return qt_graphicsItem_shapeFromPath(path, m_pen);
}

void WorldItemZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
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

	painter->drawPolygon(m_polyline);
}

void WorldItemZone::createSubPoints()
{
	BaseWorldItemPolyline::createSubPoints();

	LineStruct endLineItem;
	endLineItem.itemPoint = new WorldItemSubPoint(WorldItemSubPoint::MiddleType, this);
	endLineItem.itemPoint->setPos((m_listItems.first()->pos() + m_listItems.last()->pos()) / 2);
	endLineItem.lineItem = LineItem(m_listItems.last(), m_listItems.first());
	m_listLines.push_back(endLineItem);
}

//*******************************************

WorldItemSubPoint::WorldItemSubPoint(SubPointType pointType, AbstractWorldItem *parent)
	: QGraphicsObject(parent),
	  m_type(pointType),
	  m_active(false),
	  m_parent(parent),
	  m_lastSymbolScale(1.0)
{
	setZValue(WORLD_POINT_LAYER);

	m_brush.setColor(QColor(20, 100, 255));
	m_brush.setStyle(Qt::SolidPattern);

	m_brushMiddle.setColor(QColor(255, 25, 100));
	m_brushMiddle.setStyle(Qt::SolidPattern);

	// Opaque here: a half transparent handle on a pale zone bitmap was hard to make out.
	m_selectedBrush.setColor(selectionColor());
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
	scaledPolygon.translate(scenePos() - pivot);

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

QPainterPath WorldItemSubPoint::shape() const
{
	// The handle at its drawn size, not the room boundingRect() reserves - see
	// WorldItemPoint::shape().
	const qreal extent = qreal(SIZE_POINT) * m_lastSymbolScale;
	QRectF rect;
	rect.setCoords(-extent, -extent, extent, extent);

	QPainterPath path;
	path.addRect(rect);
	return path;
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
		painter->setBrush(m_brushMiddle);

	// A control handle, not a world object - constant size on screen (see WorldItemPoint).
	const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
	const qreal scale = qMin(1.0 / lod, MAX_SYMBOL_SCALE);
	m_lastSymbolScale = scale;

	painter->save();
	painter->scale(scale, scale);

	// Draw point
	painter->drawRect(m_rect);
	painter->restore();
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
	// Grows by up to MAX_SYMBOL_SCALE when zoomed out, see WorldItemPoint.
	const qreal extent = qreal(SIZE_POINT) * MAX_SYMBOL_SCALE;
	m_boundingRect.setCoords(-extent, -extent, extent, extent);
}

} /* namespace WorldEditor */