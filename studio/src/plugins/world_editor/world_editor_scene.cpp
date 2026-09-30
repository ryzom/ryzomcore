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
#include "world_editor_scene.h"
#include "world_editor_scene_item.h"
#include "world_editor_actions.h"
#include "world_editor_constants.h"
#include "primitives_model.h"
#include "primitive_item.h"
#include "../landscape_editor/landscape_view.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsSimpleTextItem>
#include <QApplication>

namespace WorldEditor
{

WorldEditorScene::WorldEditorScene(int sizeCell, PrimitivesTreeModel *model, QUndoStack *undoStack, QObject *parent)
	: LandscapeEditor::LandscapeSceneBase(sizeCell, parent),
	  m_editedSelectedItems(false),
	  m_lastPickedPrimitive(0),
	  m_mode(SelectMode),
	  m_pointsMode(false),
	  m_snapToGrid(false),
	  m_selectionLocked(false),
	  m_visiblePointPrimitives(true),
	  m_visiblePathPrimitives(true),
	  m_visibleZonePrimitives(true),
	  m_undoStack(undoStack),
	  m_model(model)
{
	setItemIndexMethod(NoIndex);

	// TODO: get params from settings
	setSceneRect(QRectF(-20 * 160, -20 * 160, 256 * 160, 256 * 160));

	m_greenPen.setColor(QColor(50, 255, 155));
	m_greenPen.setWidth(0);
	m_greenBrush.setColor(QColor(50, 255, 155, 80));
	m_greenBrush.setStyle(Qt::SolidPattern);

	m_purplePen.setColor(QColor(100, 0, 255));
	m_purplePen.setWidth(0);
	m_purpleBrush.setColor(QColor(100, 0, 255, 80));
	m_purpleBrush.setStyle(Qt::SolidPattern);
}

WorldEditorScene::~WorldEditorScene()
{
}

AbstractWorldItem *WorldEditorScene::addWorldItemPoint(const QPointF &point, const qreal angle,
		const qreal radius, bool showArrow, const CollisionShape &collision)
{
	WorldItemPoint *item = new WorldItemPoint(point, angle, radius, showArrow, collision);
	addItem(item);
	item->setVisible(m_visiblePointPrimitives);
	return item;
}

AbstractWorldItem *WorldEditorScene::addWorldItemPath(const QPolygonF &polyline, bool showArrow)
{
	WorldItemPath *item = new WorldItemPath(polyline);
	addItem(item);
	item->setVisible(m_visiblePathPrimitives);
	return item;
}

AbstractWorldItem *WorldEditorScene::addWorldItemZone(const QPolygonF &polygon)
{
	WorldItemZone *item = new WorldItemZone(polygon);
	addItem(item);
	item->setVisible(m_visibleZonePrimitives);
	return item;
}

namespace
{
/// Re-apply the visibility of every item of type T in the list.
///
/// dynamic_cast, not qgraphicsitem_cast: the world items all inherit
/// AbstractWorldItem::Type unchanged, so qgraphicsitem_cast matches any of them for any
/// of the three classes - every switch would hide everything. Giving them separate Type
/// values is not an option either, that is what qgraphicsitem_cast<AbstractWorldItem *>
/// relies on everywhere else.
template <typename T>
void updateVisibleItems(WorldEditorScene *scene, const QList<QGraphicsItem *> &items)
{
	Q_FOREACH (QGraphicsItem *item, items)
	{
		if (dynamic_cast<T>(item) != 0)
			scene->updateItemVisibility(item);
	}
}
}

void WorldEditorScene::setVisiblePointPrimitives(bool visible)
{
	m_visiblePointPrimitives = visible;
	updateVisibleItems<WorldItemPoint *>(this, items());
}

void WorldEditorScene::setVisiblePathPrimitives(bool visible)
{
	m_visiblePathPrimitives = visible;
	updateVisibleItems<WorldItemPath *>(this, items());
}

void WorldEditorScene::setVisibleZonePrimitives(bool visible)
{
	m_visibleZonePrimitives = visible;
	updateVisibleItems<WorldItemZone *>(this, items());
}

void WorldEditorScene::updateItemVisibility(QGraphicsItem *item)
{
	// Two things decide: the show/hide switch for the kind of primitive, and whether
	// this one primitive was hidden from the tree. Switching a kind back on must not
	// bring back what was hidden one by one.
	bool visible = true;
	if (dynamic_cast<WorldItemPoint *>(item) != 0)
		visible = m_visiblePointPrimitives;
	else if (dynamic_cast<WorldItemPath *>(item) != 0)
		visible = m_visiblePathPrimitives;
	else if (dynamic_cast<WorldItemZone *>(item) != 0)
		visible = m_visibleZonePrimitives;
	else
		return;

	Node *node = qvariant_cast<Node *>(item->data(Constants::WORLD_EDITOR_NODE));
	if ((node != 0) && PrimitivesTreeModel::isHiddenNode(node))
		visible = false;

	item->setVisible(visible);
}

void WorldEditorScene::removeWorldItem(QGraphicsItem *item)
{
	updateSelectedWorldItems(true);
	m_selectedItems.clear();
	m_editedSelectedItems = false;
	m_firstSelection = false;
	delete item;
}

void WorldEditorScene::setModeEdit(WorldEditorScene::ModeEdit mode)
{
	if (mode == WorldEditorScene::SelectMode)
		m_editedSelectedItems = false;

	m_mode = mode;
}

WorldEditorScene::ModeEdit WorldEditorScene::editMode() const
{
	return m_mode;
}

bool WorldEditorScene::isEnabledEditPoints() const
{
	return m_pointsMode;
}

void WorldEditorScene::setEnabledEditPoints(bool enabled)
{
	if (m_pointsMode == enabled)
		return;

	m_pointsMode = enabled;

	Q_FOREACH(QGraphicsItem *item, m_selectedItems)
	{
		AbstractWorldItem *worldItem = qgraphicsitem_cast<AbstractWorldItem *>(item);
		if (worldItem != 0)
			worldItem->setEnabledSubPoints(enabled);
	}

	m_selectedPoints.clear();
}

void WorldEditorScene::updateSelection(const QList<QGraphicsItem *> &selected, const QList<QGraphicsItem *> &deselected)
{
	// Deselect and remove from list graphics items.
	Q_FOREACH(QGraphicsItem *item, deselected)
	{
		// Item is selected?
		int i = m_selectedItems.indexOf(item);
		if (i != -1)
		{
			updateSelectedWorldItem(item, false);
			m_selectedItems.takeAt(i);
		}
	}

	// Select and add from list graphics items.
	Q_FOREACH(QGraphicsItem *item, selected)
	{
		// Item is selected?
		int i = m_selectedItems.indexOf(item);
		if (i == -1)
		{
			updateSelectedWorldItem(item, true);
			m_selectedItems.push_back(item);
		}
	}

	update();
	m_firstSelection = true;
}

void WorldEditorScene::drawForeground(QPainter *painter, const QRectF &rect)
{
	QGraphicsScene::drawForeground(painter, rect);

	if ((m_selectionArea.left() != 0) && (m_selectionArea.right() != 0))
	{
		// Draw selection area
		if (m_selectionArea.left() < m_selectionArea.right())
		{
			painter->setPen(m_greenPen);
			painter->setBrush(m_greenBrush);
		}
		else
		{
			painter->setPen(m_purplePen);
			painter->setBrush(m_purpleBrush);
		}
		painter->drawRect(m_selectionArea);
	}
}

void WorldEditorScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	// While editing points the right button deletes sub-points, so no menu there.
	if (isEnabledEditPoints())
	{
		LandscapeEditor::LandscapeSceneBase::contextMenuEvent(event);
		return;
	}

	// Act on what is under the cursor. Anything already selected is left alone, so a
	// right click on the current selection does not silently pick a different primitive.
	bool hitSelected = false;
	Q_FOREACH (QGraphicsItem *item, items(event->scenePos(), Qt::ContainsItemShape,
										  Qt::AscendingOrder))
	{
		if ((qgraphicsitem_cast<AbstractWorldItem *>(item) != 0) &&
			m_selectedItems.contains(item))
		{
			hitSelected = true;
			break;
		}
	}

	if (!hitSelected)
		updatePickSelection(event->scenePos());

	if (m_selectedItems.isEmpty())
	{
		// Nothing under the cursor: offer what makes sense for a spot on the map.
		Q_EMIT emptyContextMenuRequested(event->scenePos(), event->screenPos());
		event->accept();
		return;
	}

	Q_EMIT contextMenuRequested(m_selectedItems.first(), event->screenPos());
	event->accept();
}

void WorldEditorScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	m_firstPick = mouseEvent->scenePos();

	if (isEnabledEditPoints())
	{
		m_polygons = polygonsFromItems(m_selectedItems);

		if (mouseEvent->button() == Qt::LeftButton)
		{
			// Create new sub-points
			// Call method mousePressEvent for sub-point located under mouse
			LandscapeEditor::LandscapeSceneBase::mousePressEvent(mouseEvent);

			if (!m_selectionLocked &&
					((!m_editedSelectedItems && m_selectedPoints.isEmpty()) ||
					 (!calcBoundingRect(m_selectedPoints).contains(mouseEvent->scenePos()))))
			{
				updatePickSelectionPoints(mouseEvent->scenePos());
				m_firstSelection = true;
			}
			m_pivot = calcBoundingRect(m_selectedPoints).center();
		}
		else if (mouseEvent->button() == Qt::RightButton)
		{
			updateSelectedPointItems(false);
			m_selectedPoints.clear();

			// Delete sub-points if it located under mouse
			// Call method mousePressEvent for sub-point located under mouse
			LandscapeEditor::LandscapeSceneBase::mousePressEvent(mouseEvent);
		}
	}
	else
	{
		LandscapeEditor::LandscapeSceneBase::mousePressEvent(mouseEvent);

		if (mouseEvent->button() != Qt::LeftButton)
			return;

		if (!m_selectionLocked &&
				((!m_editedSelectedItems && m_selectedItems.isEmpty()) ||
				 (!calcBoundingRect(m_selectedItems).contains(mouseEvent->scenePos()))))
		{
			updatePickSelection(mouseEvent->scenePos());
			m_firstSelection = true;
		}

		m_pivot = calcBoundingRect(m_selectedItems).center();
	}

	// The point that snaps: a point primitive's position, the first vertex of a path or
	// zone, or the first picked sub-point.
	const QList<QGraphicsItem *> &picked = isEnabledEditPoints() ? m_selectedPoints : m_selectedItems;
	m_snapAnchor = mouseEvent->scenePos();
	if (!picked.isEmpty())
	{
		QGraphicsItem *item = picked.first();
		AbstractWorldItem *worldItem = qgraphicsitem_cast<AbstractWorldItem *>(item);
		const QPolygonF polygon = worldItem ? worldItem->polygon() : QPolygonF();
		m_snapAnchor = polygon.isEmpty() ? item->scenePos() : item->mapToScene(polygon.first());
	}

	// Radius mode works on the points that have a radius to change.
	m_radiusItems.clear();
	m_radiusStart.clear();
	if ((m_mode == WorldEditorScene::RadiusMode) && !isEnabledEditPoints())
	{
		Q_FOREACH (QGraphicsItem *item, m_selectedItems)
		{
			WorldItemPoint *point = dynamic_cast<WorldItemPoint *>(item);
			Node *node = qvariant_cast<Node *>(item->data(Constants::WORLD_EDITOR_NODE));
			if ((point == 0) || (node == 0) || (node->type() != Node::PrimitiveNodeType))
				continue;
			std::string radius;
			if (!static_cast<PrimitiveNode *>(node)->primitive()->getPropertyByName("radius", radius))
				continue;
			m_radiusItems.append(point);
			m_radiusStart.append(point->radius());
		}
	}

	m_editedSelectedItems = false;
	m_offset = QPointF(0, 0);
	m_angle = 0;
	m_scaleFactor = QPointF(1.0, 1.0);

	if ((m_mode == WorldEditorScene::SelectMode) && !m_selectionLocked)
		m_selectionArea.setTopLeft(mouseEvent->scenePos());
}

void WorldEditorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	m_lastMouseScenePos = mouseEvent->scenePos();

	if (QApplication::mouseButtons() == Qt::LeftButton)
	{
		m_selectionArea.setBottomRight(mouseEvent->scenePos());
		switch (m_mode)
		{
		case WorldEditorScene::SelectMode:
			break;
		case WorldEditorScene::MoveMode:
			updateWorldItemsMove(mouseEvent);
			break;
		case WorldEditorScene::RotateMode:
			updateWorldItemsRotate(mouseEvent);
			break;
		case WorldEditorScene::ScaleMode:
			updateWorldItemsScale(mouseEvent);
			break;
		case WorldEditorScene::TurnMode:
			updateWorldItemsTurn(mouseEvent);
			break;
		case WorldEditorScene::RadiusMode:
			updateWorldItemsRadius(mouseEvent);
			break;
		};

		if (isEnabledEditPoints())
		{
			if ((editMode() != WorldEditorScene::SelectMode) && (!m_selectedPoints.isEmpty()))
				m_editedSelectedItems = true;
			else
				m_editedSelectedItems = false;
		}
		else
		{
			if ((editMode() != WorldEditorScene::SelectMode) && (!m_selectedItems.isEmpty()))
				m_editedSelectedItems = true;
			else
				m_editedSelectedItems = false;
		}
		// Update render (drawing selection area when enabled multiple selection mode)
		update();
	}

	LandscapeEditor::LandscapeSceneBase::mouseMoveEvent(mouseEvent);
}

void WorldEditorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (mouseEvent->button() == Qt::MidButton)
		return;

	if (mouseEvent->button() == Qt::LeftButton)
	{
		checkUndo();

		// Update selection
		if ((m_selectionArea.left() != 0) && (m_selectionArea.right() != 0))
		{
			QList<QGraphicsItem *> listItems;

			// Clear selection
			updateSelectedPointItems(false);
			m_selectedPoints.clear();

			// Return list of selected items
			if (m_selectionArea.left() < m_selectionArea.right())
				listItems = items(m_selectionArea, Qt::IntersectsItemShape, Qt::AscendingOrder);
			else
				listItems = items(m_selectionArea, Qt::ContainsItemShape, Qt::AscendingOrder);

			if (isEnabledEditPoints())
			{
				Q_FOREACH(QGraphicsItem *item, listItems)
				{
					if (qgraphicsitem_cast<WorldItemSubPoint *>(item) == 0)
						continue;
					m_selectedPoints.push_back(item);
				}
				updateSelectedPointItems(true);
			}
			else
			{
				Q_FOREACH(QGraphicsItem *item, listItems)
				{
					if (qgraphicsitem_cast<AbstractWorldItem *>(item) == 0)
						continue;
					m_selectedItems.push_back(item);
				}
				Q_EMIT updateSelectedItems(m_selectedItems);
				updateSelectedWorldItems(true);
			}
			m_selectionArea = QRectF();
			update();
		}
		else
		{
			if ((!m_editedSelectedItems) && (!m_firstSelection) && !m_selectionLocked)
			{
				if (isEnabledEditPoints())
					updatePickSelectionPoints(mouseEvent->scenePos());
				else
					updatePickSelection(mouseEvent->scenePos());
			}
			else
				m_firstSelection = false;
		}

		if (isEnabledEditPoints())
			checkUndoPointsMode();
	}
	m_selectionArea = QRectF();
	LandscapeEditor::LandscapeSceneBase::mouseReleaseEvent(mouseEvent);
}

QRectF WorldEditorScene::calcBoundingRect(const QList<QGraphicsItem *> &listItems)
{
	QRectF rect;
	Q_FOREACH(QGraphicsItem *item, listItems)
	{
		QRectF itemRect = item->boundingRect();
		rect = rect.united(itemRect.translated(item->scenePos()));
	}
	return rect;
}

QPainterPath WorldEditorScene::calcBoundingShape(const QList<QGraphicsItem *> &listItems)
{
	QPainterPath painterPath;
	Q_FOREACH(QGraphicsItem *item, listItems)
	{
		QPainterPath itemPath = item->shape();
		painterPath = painterPath.united(itemPath.translated(item->scenePos()));
	}
	return painterPath;
}

void WorldEditorScene::updateSelectedWorldItems(bool value)
{
	Q_FOREACH(QGraphicsItem *item, m_selectedItems)
	{
		updateSelectedWorldItem(item, value);
	}
	update();
}

void WorldEditorScene::updateSelectedWorldItem(QGraphicsItem *item, bool value)
{
	AbstractWorldItem *worldItem = qgraphicsitem_cast<AbstractWorldItem *>(item);
	if (worldItem != 0)
		worldItem->setActived(value);
}

void WorldEditorScene::updateSelectedPointItems(bool value)
{
	Q_FOREACH(QGraphicsItem *item, m_selectedPoints)
	{
		updateSelectedPointItem(item, value);
	}
	update();
}

void WorldEditorScene::updateSelectedPointItem(QGraphicsItem *item, bool value)
{
	WorldItemSubPoint *worldItem = qgraphicsitem_cast<WorldItemSubPoint *>(item);
	if (worldItem != 0)
		worldItem->setActived(value);
}

void WorldEditorScene::updatePickSelection(const QPointF &point)
{
	updateSelectedWorldItems(false);
	m_selectedItems.clear();

	QList<QGraphicsItem *> listItems = items(point, Qt::ContainsItemShape,
									   Qt::AscendingOrder);

	QList<AbstractWorldItem *> worldItemsItems;

	Q_FOREACH(QGraphicsItem *item, listItems)
	{
		AbstractWorldItem *worldItem = qgraphicsitem_cast<AbstractWorldItem *>(item);
		if (worldItem != 0)
			worldItemsItems.push_back(worldItem);
	}

	if (!worldItemsItems.isEmpty())
	{
		// Next primitives
		m_lastPickedPrimitive++;
		m_lastPickedPrimitive %= worldItemsItems.size();

		m_selectedItems.push_back(worldItemsItems.at(m_lastPickedPrimitive));
		updateSelectedWorldItems(true);
	}

	Q_EMIT updateSelectedItems(m_selectedItems);
}

void WorldEditorScene::updatePickSelectionPoints(const QPointF &point)
{
	updateSelectedPointItems(false);
	m_selectedPoints.clear();

	QList<QGraphicsItem *> listItems = items(point, Qt::IntersectsItemBoundingRect,
									   Qt::AscendingOrder);

	QList<WorldItemSubPoint *> subPointsItems;

	Q_FOREACH(QGraphicsItem *item, listItems)
	{
		WorldItemSubPoint *subPointItem = qgraphicsitem_cast<WorldItemSubPoint *>(item);
		if (subPointItem != 0)
		{
			if (subPointItem->subPointType() == WorldItemSubPoint::EdgeType)
				subPointsItems.push_back(subPointItem);
		}
	}

	if (!subPointsItems.isEmpty())
	{
		// Next primitives
		m_lastPickedPrimitive++;
		m_lastPickedPrimitive %= subPointsItems.size();

		m_selectedPoints.push_back(subPointsItems.at(m_lastPickedPrimitive));
		updateSelectedPointItems(true);
	}
}

void WorldEditorScene::checkUndo()
{
	if (m_editedSelectedItems && (!isEnabledEditPoints()))
	{
		switch (m_mode)
		{
		case WorldEditorScene::SelectMode:
			break;
		case WorldEditorScene::MoveMode:
			m_undoStack->push(new MoveWorldItemsCommand(m_selectedItems, m_offset, this, m_model));
			break;
		case WorldEditorScene::RotateMode:
			m_undoStack->push(new RotateWorldItemsCommand(m_selectedItems, m_angle, m_pivot, this, m_model));
			break;
		case WorldEditorScene::ScaleMode:
			m_undoStack->push(new ScaleWorldItemsCommand(m_selectedItems, m_scaleFactor, m_pivot, this, m_model));
			break;
		case WorldEditorScene::TurnMode:
			m_undoStack->push(new TurnWorldItemsCommand(m_selectedItems, m_angle, this, m_model));
			break;
		case WorldEditorScene::RadiusMode:
		{
			// Each changed radius becomes a property change, so it is undone, saved and
			// shown in the property form like one typed in there.
			QList<int> changed;
			for (int i = 0; i < m_radiusItems.size(); ++i)
			{
				if (m_radiusItems[i]->radius() != m_radiusStart[i])
					changed.append(i);
			}
			if (changed.isEmpty())
				break;

			m_undoStack->beginMacro(tr("Change radius"));
			Q_FOREACH (int i, changed)
			{
				Node *node = qvariant_cast<Node *>(m_radiusItems[i]->data(Constants::WORLD_EDITOR_NODE));
				m_undoStack->push(new SetPropertyCommand(m_model->pathFromNode(node), QLatin1String("radius"),
														 QString::number(m_radiusItems[i]->radius(), 'g', 6),
														 false, m_model, this));
			}
			m_undoStack->endMacro();
			break;
		}
		};
	}
}

void WorldEditorScene::checkUndoPointsMode()
{
	if (m_pointsMode)
	{
		QList<QGraphicsItem *> items;
		QList<QPolygonF> polygons;
		Q_FOREACH(QGraphicsItem *item, m_selectedItems)
		{
			AbstractWorldItem *worldItem = qgraphicsitem_cast<AbstractWorldItem *>(item);
			if (worldItem->isShapeChanged())
			{
				items.push_back(item);
				polygons.push_back(m_polygons.at(m_selectedItems.indexOf(item)));
				worldItem->setShapeChanged(false);
			}
		}
		if (!items.isEmpty())
		{
			m_undoStack->push(new ShapeWorldItemsCommand(items, polygons, this, m_model));
			m_polygons.clear();
		}
	}
}

void WorldEditorScene::updateWorldItemsMove(QGraphicsSceneMouseEvent *mouseEvent)
{
	// Where the selection should be now, relative to where the drag started. Worked out
	// from the start each time rather than summed up, so snapping can be switched while
	// dragging and nothing drifts.
	QPointF target = mouseEvent->scenePos() - m_firstPick;
	if (m_snapToGrid)
		target = snapToGrid(m_snapAnchor + target) - m_snapAnchor;
	const QPointF offset = target - m_offset;
	if (offset.isNull())
		return;
	m_offset = target;
	if (m_pointsMode)
		Q_FOREACH(QGraphicsItem *item, m_selectedPoints)
	{
		item->moveBy(offset.x(), offset.y());
	}
	else
		Q_FOREACH(QGraphicsItem *item, m_selectedItems)
	{
		item->moveBy(offset.x(), offset.y());
	}
}

void WorldEditorScene::updateWorldItemsScale(QGraphicsSceneMouseEvent *mouseEvent)
{
	QPointF offset(mouseEvent->scenePos() - mouseEvent->lastScenePos());

	qreal scaleRatio = 5000;

	// Calculate scale factor
	if (offset.x() > 0)
		offset.setX(1.0 + (offset.x() / scaleRatio));
	else
		offset.setX(1.0 / (1.0 + (-offset.x() / scaleRatio)));

	if (offset.y() < 0)
		offset.setY(1.0 - (offset.y() / scaleRatio));
	else
		offset.setY(1.0 / (1.0 + (offset.y() / scaleRatio)));

	m_scaleFactor.setX(offset.x() * m_scaleFactor.x());
	m_scaleFactor.setY(offset.y() * m_scaleFactor.y());

	if (m_pointsMode)
		Q_FOREACH(QGraphicsItem *item, m_selectedPoints)
	{
		qgraphicsitem_cast<WorldItemSubPoint *>(item)->scaleOn(m_pivot, offset);
	}
	else
		Q_FOREACH(QGraphicsItem *item, m_selectedItems)
	{
		qgraphicsitem_cast<AbstractWorldItem *>(item)->scaleOn(m_pivot, offset);
	}
}

void WorldEditorScene::updateWorldItemsRotate(QGraphicsSceneMouseEvent *mouseEvent)
{
	// Caluculate angle between two line
	QLineF firstLine(m_pivot, mouseEvent->lastScenePos());
	QLineF secondLine(m_pivot, mouseEvent->scenePos());
	qreal angle = secondLine.angleTo(firstLine);
	m_angle += angle;

	if (m_pointsMode)
		Q_FOREACH(QGraphicsItem *item, m_selectedPoints)
	{
		qgraphicsitem_cast<WorldItemSubPoint *>(item)->rotateOn(m_pivot, angle);
	}
	else
		Q_FOREACH(QGraphicsItem *item, m_selectedItems)
	{
		qgraphicsitem_cast<AbstractWorldItem *>(item)->rotateOn(m_pivot, angle);
	}
}

void WorldEditorScene::updateWorldItemsTurn(QGraphicsSceneMouseEvent *mouseEvent)
{
	// Caluculate angle between two line
	QLineF firstLine(m_pivot, mouseEvent->lastScenePos());
	QLineF secondLine(m_pivot, mouseEvent->scenePos());
	qreal angle = secondLine.angleTo(firstLine);
	m_angle += angle;

	Q_FOREACH(QGraphicsItem *item, m_selectedItems)
	{
		qgraphicsitem_cast<AbstractWorldItem *>(item)->turnOn(angle);
	}
}

void WorldEditorScene::updateWorldItemsRadius(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (m_radiusItems.isEmpty())
		return;

	// The first point leads: its circle grows by as much as the mouse moved away from
	// its centre since the press, so grabbing the circle anywhere does not make it jump.
	// The others keep their proportion to it.
	const QPointF centre = m_radiusItems.first()->scenePos();
	const qreal startLead = m_radiusStart.first();
	const qreal moved = QLineF(centre, mouseEvent->scenePos()).length() - QLineF(centre, m_firstPick).length();

	const qreal step = m_snapToGrid ? gridStep() : 0;
	const qreal minimum = (step > 0) ? step : 0.1;
	qreal lead = startLead + moved;
	if (step > 0)
		lead = qRound64(lead / step) * step;
	lead = qMax(minimum, lead);

	for (int i = 0; i < m_radiusItems.size(); ++i)
	{
		const qreal radius = (startLead > 0) ? m_radiusStart[i] * lead / startLead
											 : m_radiusStart[i] + (lead - startLead);
		m_radiusItems[i]->setRadius(qMax(minimum, radius));
	}
}

bool WorldEditorScene::isSnapToGrid() const
{
	return m_snapToGrid;
}

void WorldEditorScene::setSnapToGrid(bool enabled)
{
	m_snapToGrid = enabled;
}

bool WorldEditorScene::isSelectionLocked() const
{
	return m_selectionLocked;
}

void WorldEditorScene::setSelectionLocked(bool locked)
{
	m_selectionLocked = locked;
	m_selectionArea = QRectF();
	update();
}

qreal WorldEditorScene::gridStep() const
{
	if (views().isEmpty())
		return 0;
	LandscapeEditor::LandscapeView *view = qobject_cast<LandscapeEditor::LandscapeView *>(views().first());
	return view ? view->gridPointsStep() : 0;
}

QPointF WorldEditorScene::snapToGrid(const QPointF &scenePos) const
{
	if (!m_snapToGrid)
		return scenePos;

	// The step of the dots the user sees. The cell size is a multiple of every step, so
	// rounding in scene coordinates lands on whole steps in world coordinates as well.
	const qreal step = gridStep();
	if (step <= 0)
		return scenePos;

	return QPointF(qRound64(scenePos.x() / step) * step, qRound64(scenePos.y() / step) * step);
}

} /* namespace WorldEditor */
