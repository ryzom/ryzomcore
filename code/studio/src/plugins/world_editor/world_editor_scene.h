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

#ifndef WORLD_EDITOR_SCENE_H
#define WORLD_EDITOR_SCENE_H

// Project includes
#include "world_editor_global.h"

#include "../landscape_editor/landscape_scene_base.h"

// NeL includes

// Qt includes
#include <QtGui/QUndoStack>

namespace WorldEditor
{
class PrimitivesTreeModel;
class AbstractWorldItem;

/*
@class WorldEditorScene
@brief The WorldEditorScene provides a surface for managing a large number of 2D world items(point/path/zone).
@details WorldEditorScene also provides 'selections model' functionality, which differs from standart selection model.
*/
class WORLD_EDITOR_EXPORT WorldEditorScene : public LandscapeEditor::LandscapeSceneBase
{
	Q_OBJECT

public:
	enum ModeEdit
	{
		SelectMode = 0,
		MoveMode,
		RotateMode,
		ScaleMode,
		TurnMode,
		RadiusMode
	};

	WorldEditorScene(int sizeCell, PrimitivesTreeModel *model,
					 QUndoStack *undoStack, QObject *parent = 0);
	virtual ~WorldEditorScene();

	/// Create WorldItemPoint and add in scene.
	AbstractWorldItem *addWorldItemPoint(const QPointF &point, const qreal angle,
										 const qreal radius, bool showArrow);

	/// Create WorldItemPath and add in scene.
	AbstractWorldItem *addWorldItemPath(const QPolygonF &polyline, bool showArrow);

	/// Create WorldItemZone and add in scene.
	AbstractWorldItem *addWorldItemZone(const QPolygonF &polygon);

	/// Remove a world item from the scene.
	void removeWorldItem(QGraphicsItem *item);

	/// Set current mode editing(select/move/rotate/scale/turn), above world items.
	void setModeEdit(WorldEditorScene::ModeEdit mode);

	WorldEditorScene::ModeEdit editMode() const;

	/// @return true if edit points mode is enabled, else false.
	bool isEnabledEditPoints() const;

Q_SIGNALS:
	/// This signal is emitted by WorldEditorScene when the selections changes.
	/// The @selected value contains a list of all selected items.
	void updateSelectedItems(const QList<QGraphicsItem *> &selected);

public Q_SLOTS:
	/// Enable/disable edit points mode (user can change shape of WorldItemZone and WorldItemPath)
	///
	void setEnabledEditPoints(bool enabled);

	/// Update of selections
	void updateSelection(const QList<QGraphicsItem *> &selected, const QList<QGraphicsItem *> &deselected);

protected:
	virtual void drawForeground(QPainter *painter, const QRectF &rect);

	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
	QRectF calcBoundingRect(const QList<QGraphicsItem *> &listItems);
	QPainterPath calcBoundingShape(const QList<QGraphicsItem *> &listItems);

	void updateSelectedWorldItems(bool value);
	void updateSelectedWorldItem(QGraphicsItem *item, bool value);
	void updateSelectedPointItems(bool value);
	void updateSelectedPointItem(QGraphicsItem *item, bool value);

	void updatePickSelection(const QPointF &point);
	void updatePickSelectionPoints(const QPointF &point);

	void checkUndo();
	void checkUndoPointsMode();

	void updateWorldItemsMove(QGraphicsSceneMouseEvent *mouseEvent);
	void updateWorldItemsScale(QGraphicsSceneMouseEvent *mouseEvent);
	void updateWorldItemsRotate(QGraphicsSceneMouseEvent *mouseEvent);
	void updateWorldItemsTurn(QGraphicsSceneMouseEvent *mouseEvent);
	void updateWorldItemsRadius(QGraphicsSceneMouseEvent *mouseEvent);

	QPen m_greenPen, m_purplePen;
	QBrush m_greenBrush, m_purpleBrush;

	QPointF m_firstPick, m_scaleFactor, m_pivot, m_offset;
	QRectF m_selectionArea;
	qreal m_firstPickX, m_firstPickY, m_angle;

	QList<QGraphicsItem *> m_selectedItems;
	QList<QGraphicsItem *> m_selectedPoints;
	QList<QPolygonF> m_polygons;

	bool m_editedSelectedItems, m_firstSelection;
	uint m_lastPickedPrimitive;
	ModeEdit m_mode;
	bool m_pointsMode;
	QUndoStack *m_undoStack;
	PrimitivesTreeModel *m_model;
};

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_SCENE_H
