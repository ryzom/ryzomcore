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
	  m_editMode(false),
	  m_undoStack(undoStack),
	  m_model(model)
{
	setItemIndexMethod(NoIndex);

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

QGraphicsItem *WorldEditorScene::addWorldItemPoint(const QPointF &point, const float angle)
{
	WorldItemPoint *item = new WorldItemPoint(point, angle);
	addItem(item);
	return item;
}

QGraphicsItem *WorldEditorScene::addWorldItemPath(const QPolygonF &polyline)
{
	WorldItemPath *item = new WorldItemPath(polyline);
	addItem(item);
	return item;
}

QGraphicsItem *WorldEditorScene::addWorldItemZone(const QPolygonF &polygon)
{
	WorldItemZone *item = new WorldItemZone(polygon);
	addItem(item);
	return item;
}

void WorldEditorScene::removeWorldItem(QGraphicsItem *item)
{
	updateSelectedItems(true);
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

bool WorldEditorScene::isEnabledEditPoint() const
{
	return m_editMode;
}

void WorldEditorScene::setEnabledEditPoint(bool enabled)
{
	m_editMode = enabled;
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
	LandscapeEditor::LandscapeSceneBase::mousePressEvent(mouseEvent);

	qreal x = mouseEvent->scenePos().x();
	qreal y = mouseEvent->scenePos().y();

	if (mouseEvent->button() != Qt::LeftButton)
		return;

	m_firstPick = mouseEvent->scenePos();

//	if ((!m_editedSelectedItems) && (m_mode != WorldEditorScene::SelectMode))
	if ((!m_editedSelectedItems && m_selectedItems.isEmpty()) ||
			(!calcBoundingRect(m_selectedItems).contains(mouseEvent->scenePos())))
	{
		updatePickSelection(mouseEvent->scenePos());
		m_firstSelection = true;
	}

	m_editedSelectedItems = false;

	switch (m_mode)
	{
	case WorldEditorScene::SelectMode:
	{
		m_selectionArea.setTopLeft(mouseEvent->scenePos());
		break;
	}
	case WorldEditorScene::MoveMode:
	{
		break;
	}
	case WorldEditorScene::RotateMode:
		break;
	case WorldEditorScene::ScaleMode:
		m_scaleFactor = QPointF(1.0, 1.0);
		break;
	case WorldEditorScene::TurnMode:
		break;
	case WorldEditorScene::RadiusMode:
		break;
	};

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
			Q_FOREACH(QGraphicsItem *item, m_selectedItems)
			{
				qgraphicsitem_cast<AbstractWorldItem *>(item)->moveOn(offset);
			}
			break;
		}
		case WorldEditorScene::RotateMode:
		{
			QPointF pivot = calcBoundingRect(m_selectedItems).center();

			// Caluculate angle between two line
			QLineF firstLine(pivot, mouseEvent->lastScenePos());
			QLineF secondLine(pivot, mouseEvent->scenePos());
			qreal angle = secondLine.angleTo(firstLine);

			Q_FOREACH(QGraphicsItem *item, m_selectedItems)
			{
				qgraphicsitem_cast<AbstractWorldItem *>(item)->rotateOn(pivot, angle);
			}
			break;
		}
		case WorldEditorScene::ScaleMode:
		{
			// TODO: perfomance
			QPointF pivot = calcBoundingRect(m_selectedItems).center();

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

			Q_FOREACH(QGraphicsItem *item, m_selectedItems)
			{
				qgraphicsitem_cast<AbstractWorldItem *>(item)->scaleOn(pivot, offset);
			}
			break;
		}
		case WorldEditorScene::TurnMode:
			break;
		case WorldEditorScene::RadiusMode:
			break;
		};

		if ((editMode() != WorldEditorScene::SelectMode) && (!m_selectedItems.isEmpty()))
			m_editedSelectedItems = true;
		else
			m_editedSelectedItems = false;

		update();
	}
	/*m_mouseX = mouseEvent->scenePos().x();
	m_mouseY = mouseEvent->scenePos().y() - m_cellSize;
	*/
	LandscapeEditor::LandscapeSceneBase::mouseMoveEvent(mouseEvent);
}

void WorldEditorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
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

			m_undoStack->push(new MoveWorldItemsCommand(m_selectedItems, offset, m_model));
			break;
		}
		case WorldEditorScene::RotateMode:
		{
			QPointF pivot = calcBoundingRect(m_selectedItems).center();

			// Caluculate angle between two line
			QLineF firstLine(pivot, m_firstPick);
			QLineF secondLine(pivot, mouseEvent->scenePos());
			qreal angle = secondLine.angleTo(firstLine);

			m_undoStack->push(new RotateWorldItemsCommand(m_selectedItems, angle, pivot, m_model));
			break;
		}
		case WorldEditorScene::ScaleMode:
		{
			QPointF pivot = calcBoundingRect(m_selectedItems).center();
			m_undoStack->push(new ScaleWorldItemsCommand(m_selectedItems, m_scaleFactor, pivot, m_model));

			break;
		}
		case WorldEditorScene::TurnMode:
			break;
		case WorldEditorScene::RadiusMode:
			break;
		};
	}

	if ((m_selectionArea.left() != 0) && (m_selectionArea.right() != 0))
	{
		QList<QGraphicsItem *> listItems;

		// Clear selection
		updateSelectedItems(false);
		m_selectedItems.clear();

		if (m_selectionArea.left() < m_selectionArea.right())
		{
			listItems = items(m_selectionArea, Qt::IntersectsItemShape,
							  Qt::AscendingOrder);
		}
		else
		{
			listItems = items(m_selectionArea, Qt::ContainsItemShape,
							  Qt::AscendingOrder);
		}

		Q_FOREACH(QGraphicsItem *item, listItems)
		{
			if (qgraphicsitem_cast<AbstractWorldItem *>(item) == 0)
				continue;

			m_selectedItems.push_back(item);
		}
		updateSelectedItems(true);
		m_selectionArea = QRectF();
		update();
	}
	else
	{
		if ((!m_editedSelectedItems) && (!m_firstSelection))
			updatePickSelection(mouseEvent->scenePos());
		else
			m_firstSelection = false;

		m_selectionArea = QRectF();
	}

	LandscapeEditor::LandscapeSceneBase::mouseReleaseEvent(mouseEvent);
}

QRectF WorldEditorScene::calcBoundingRect(const QList<QGraphicsItem *> &listItems)
{
	QRectF rect;
	Q_FOREACH(QGraphicsItem *item, listItems)
	{
		QRectF itemRect = item->boundingRect();
		rect = rect.united(itemRect.translated(item->pos()));
	}
	return rect;
}

QPainterPath WorldEditorScene::calcBoundingShape(const QList<QGraphicsItem *> &listItems)
{
	QPainterPath painterPath;
	Q_FOREACH(QGraphicsItem *item, listItems)
	{
		QPainterPath itemPath = item->shape();
		painterPath = painterPath.united(itemPath.translated(item->pos()));
	}
	return painterPath;
}

void WorldEditorScene::updateSelectedItems(bool value)
{
	Q_FOREACH(QGraphicsItem *item, m_selectedItems)
	{
		if (value)
		{
			item->setFlag(QGraphicsItem::ItemIsSelectable);
			//item->setZValue(SELECTED_LAYER);
		}
		else
		{
			item->setFlag(QGraphicsItem::ItemIsSelectable, false);
			//item->setZValue(UNSELECTED_LAYER);
		}
		item->setSelected(value);
	}
}

void WorldEditorScene::updatePickSelection(const QPointF &point)
{
	//clearSelection();
	updateSelectedItems(false);
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
		updateSelectedItems(true);
	}
}

} /* namespace WorldEditor */
