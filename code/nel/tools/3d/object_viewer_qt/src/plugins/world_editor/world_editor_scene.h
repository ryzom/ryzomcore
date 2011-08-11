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

/*
@class WorldEditorScene
@brief
@details
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

	QGraphicsItem *addWorldItemPoint(const QPointF &point, const float angle);
	QGraphicsItem *addWorldItemPath(const QPolygonF &polyline);
	QGraphicsItem *addWorldItemZone(const QPolygonF &polygon);

	void removeWorldItem(QGraphicsItem *item);

	void setModeEdit(WorldEditorScene::ModeEdit mode);
	WorldEditorScene::ModeEdit editMode() const;

	bool isEnabledEditPoint() const;

public Q_SLOTS:
	void setEnabledEditPoint(bool enabled);

protected:
	virtual void drawForeground(QPainter *painter, const QRectF &rect);

	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:

	QRectF calcBoundingRect(const QList<QGraphicsItem *> &listItems);
	QPainterPath calcBoundingShape(const QList<QGraphicsItem *> &listItems);
	void updateSelectedItems(bool value);

	void updatePickSelection(const QPointF &point);

	QPen m_pen1, m_pen2;
	QBrush m_brush1, m_brush2;

	QPointF m_firstPick, m_scaleFactor;
	QRectF m_selectionArea;
	qreal m_firstPickX, m_firstPickY;
	QList<QGraphicsItem *> m_selectedItems;
	bool m_editedSelectedItems, m_firstSelection;
	uint m_lastPickedPrimitive;
	ModeEdit m_mode;
	bool m_editMode;
	QUndoStack *m_undoStack;
	PrimitivesTreeModel *m_model;
};

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_SCENE_H
