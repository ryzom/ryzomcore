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
#include "world_editor_scene.h"
#include "world_editor_scene_item.h"
#include "world_editor_actions.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QApplication>

namespace WorldEditor
{

WorldEditorScene::WorldEditorScene(int sizeCell, PrimitivesTreeModel *model, QUndoStack *undoStack, QObject *parent)
	: LandscapeEditor::LandscapeSceneBase(sizeCell, parent),
	  m_editedSelectedItems(false),
	  m_lastPickedPrimitive(0),
	  m_mode(SelectMode),
	  m_pointsMode(false),
	  m_undoStack(undoStack),
	  m_model(model)
{
	setItemIndexMethod(NoIndex);

	// TODO: get params from settings
	setSceneRect(QRectF(-20 * 160, -20 * 160, 256 * 160, 256 * 160));

	m_pen1.setColor(QColor(50, 255, 155));
	m_pen1.setWidth(0);

	m_brush1.setColor(QColor(50, 255, 155, 80));
	m_brush1.setStyle(Qt::SolidPattern);

	m_pen2.setColor(QColor(100, 0, 255));
	m_pen2.setWidth(0);

	m_brush2.setColor(QColor(100, 0, 255, 80));
	m_brush2.setStyle(Qt::SolidPattern);
}

WorldEditorScene::~WorldEditorScene()
{
}

AbstractWorldItem *WorldEditorScene::addWorldItemPoint(const QPointF &point, const qreal angle,
		const qreal radius, bool showArrow)
{
	WorldItemPoint *item = new WorldItemPoint(point, angle, radius, showArrow);
	addItem(item);
	return item;
}

AbstractWorldItem *WorldEditorScene::addWorldItemPath(const QPolygonF &polyline, bool showArrow)
{
	WorldItemPath *item = new WorldItemPath(polyline);
	addItem(item);
	return item;
}

AbstractWorldItem *WorldEditorScene::addWorldItemZone(const QPolygonF &polygon)
{
	WorldItemZone *item = new WorldItemZone(polygon);
	addItem(item);
	return item;
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
		if (m_selectionArea.left() < m_selectionArea.right())
		{
			painter->setPen(m_pen1);
			painter->setBrush(m_brush1);
		}
		else
		{
			painter->setPen(m_pen2);
			painter->setBrush(m_brush2);
		}
		painter->drawRect(m_selectionArea);

	}
}

void WorldEditorScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	m_firstPick = mouseEvent->scenePos();

	if (m_pointsMode)
	{
		m_polygons = polygonsFromItems(m_selectedItems);

		if (mouseEvent->button() == Qt::LeftButton)
		{
			// Create new sub-points
			// Call method mousePressEvent for point located under mouse
			LandscapeEditor::LandscapeSceneBase::mousePressEvent(mouseEvent);

			if ((!m_editedSelectedItems && m_selectedPoints.isEmpty()) ||
					(!calcBoundingRect(m_selectedPoints).contains(mouseEvent->scenePos())))
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
			// Call method mousePressEvent for point located under mouse
			LandscapeEditor::LandscapeSceneBase::mousePressEvent(mouseEvent);
		}
	}
	else
	{
		LandscapeEditor::LandscapeSceneBase::mousePressEvent(mouseEvent);

		if (mouseEvent->button() != Qt::LeftButton)
			return;

		if ((!m_editedSelectedItems && m_selectedItems.isEmpty()) ||
				(!calcBoundingRect(m_selectedItems).contains(mouseEvent->scenePos())))
		{
			updatePickSelection(mouseEvent->scenePos());
			m_firstSelection = true;
		}

		m_pivot = calcBoundingRect(m_selectedItems).center();
	}

	m_editedSelectedItems = false;
	m_angle = 0;
	m_scaleFactor = QPointF(1.0, 1.0);

	if (m_mode == WorldEditorScene::SelectMode)
		m_selectionArea.setTopLeft(mouseEvent->scenePos());

//	if (m_selectedItems.isEmpty())
//		m_selectionArea.setTopLeft(mouseEvent->scenePos());
}

void WorldEditorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (QApplication::mouseButtons() == Qt::LeftButton)
	{
		QPointF offset(mouseEvent->scenePos() - mouseEvent->lastScenePos());

		m_selectionArea.setBottomRight(mouseEvent->scenePos());

		switch (m_mode)
		{
		case WorldEditorScene::SelectMode:
			break;
		case WorldEditorScene::MoveMode:
		{
			if (m_pointsMode)
			{
				Q_FOREACH(QGraphicsItem *item, m_selectedPoints)
				{
					item->moveBy(offset.x(), offset.y());
				}
			}
			else
			{
				Q_FOREACH(QGraphicsItem *item, m_selectedItems)
				{
					item->moveBy(offset.x(), offset.y());
				}
			}
			break;
		}
		case WorldEditorScene::RotateMode:
		{
			// Caluculate angle between two line
			QLineF firstLine(m_pivot, mouseEvent->lastScenePos());
			QLineF secondLine(m_pivot, mouseEvent->scenePos());
			qreal angle = secondLine.angleTo(firstLine);

			m_angle += angle;

			if (m_pointsMode)
			{
				Q_FOREACH(QGraphicsItem *item, m_selectedPoints)
				{
					qgraphicsitem_cast<WorldItemSubPoint *>(item)->rotateOn(m_pivot, angle);
				}
			}
			else
			{
				Q_FOREACH(QGraphicsItem *item, m_selectedItems)
				{
					qgraphicsitem_cast<AbstractWorldItem *>(item)->rotateOn(m_pivot, angle);
				}
			}
			break;
		}
		case WorldEditorScene::ScaleMode:
		{
			// Calculate scale factor
			if (offset.x() > 0)
				offset.setX(1.0 + (offset.x() / 5000));
			else
				offset.setX(1.0 / (1.0 + (-offset.x() / 5000)));

			if (offset.y() < 0)
				offset.setY(1.0 + (-offset.y() / 5000));
			else
				offset.setY(1.0 / (1.0 + (offset.y() / 5000)));

			m_scaleFactor.setX(offset.x() * m_scaleFactor.x());
			m_scaleFactor.setY(offset.y() * m_scaleFactor.y());

			if (m_pointsMode)
			{
				Q_FOREACH(QGraphicsItem *item, m_selectedPoints)
				{
					qgraphicsitem_cast<WorldItemSubPoint *>(item)->scaleOn(m_pivot, offset);
				}
			}
			else
			{
				Q_FOREACH(QGraphicsItem *item, m_selectedItems)
				{
					qgraphicsitem_cast<AbstractWorldItem *>(item)->scaleOn(m_pivot, offset);
				}
			}
			break;
		}
		case WorldEditorScene::TurnMode:
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

			break;
		}
		case WorldEditorScene::RadiusMode:
			break;
		};

		if (m_pointsMode)
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

		update();
	}

	LandscapeEditor::LandscapeSceneBase::mouseMoveEvent(mouseEvent);
}

void WorldEditorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (m_pointsMode)
	{
		if (mouseEvent->button() == Qt::LeftButton)
		{
			if ((m_selectionArea.left() != 0) && (m_selectionArea.right() != 0))
			{
				QList<QGraphicsItem *> listItems;

				// Clear selection
				updateSelectedPointItems(false);
				m_selectedPoints.clear();

				if (m_selectionArea.left() < m_selectionArea.right())
					listItems = items(m_selectionArea, Qt::IntersectsItemShape, Qt::AscendingOrder);
				else
					listItems = items(m_selectionArea, Qt::ContainsItemShape, Qt::AscendingOrder);

				Q_FOREACH(QGraphicsItem *item, listItems)
				{
					if (qgraphicsitem_cast<WorldItemSubPoint *>(item) == 0)
						continue;

					m_selectedPoints.push_back(item);
				}
				updateSelectedPointItems(true);
				m_selectionArea = QRectF();
				update();
			}
			else
			{
				if ((!m_editedSelectedItems) && (!m_firstSelection))
					updatePickSelectionPoints(mouseEvent->scenePos());
				else
					m_firstSelection = false;
			}
		}
		checkUndo();
	}
	else
	{
		if (mouseEvent->button() != Qt::LeftButton)
			return;

		if (m_editedSelectedItems)
		{
			switch (m_mode)
			{
			case WorldEditorScene::SelectMode:
				break;

			case WorldEditorScene::MoveMode:
			{
				QPointF offset = mouseEvent->scenePos() - m_firstPick;
				m_undoStack->push(new MoveWorldItemsCommand(m_selectedItems, offset, this, m_model));
				break;
			}
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
				break;
			};
		}

		if ((m_selectionArea.left() != 0) && (m_selectionArea.right() != 0))
		{
			QList<QGraphicsItem *> listItems;

			// Clear selection
			updateSelectedWorldItems(false);
			m_selectedItems.clear();

			if (m_selectionArea.left() < m_selectionArea.right())
				listItems = items(m_selectionArea, Qt::IntersectsItemShape, Qt::AscendingOrder);
			else
				listItems = items(m_selectionArea, Qt::ContainsItemShape, Qt::AscendingOrder);

			Q_FOREACH(QGraphicsItem *item, listItems)
			{
				if (qgraphicsitem_cast<AbstractWorldItem *>(item) == 0)
					continue;

				m_selectedItems.push_back(item);
			}
			updateSelectedWorldItems(true);
			m_selectionArea = QRectF();
			update();
		}
		else
		{
			if ((!m_editedSelectedItems) && (!m_firstSelection))
				updatePickSelection(mouseEvent->scenePos());
			else
				m_firstSelection = false;
		}
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

} /* namespace WorldEditor */
