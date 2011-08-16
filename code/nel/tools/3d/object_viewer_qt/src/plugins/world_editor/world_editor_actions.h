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

#ifndef WORLD_EDITOR_ACTIONS_H
#define WORLD_EDITOR_ACTIONS_H

// Project includes
#include "primitives_model.h"

// NeL includes

// Qt includes
#include <QtGui/QUndoCommand>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>

namespace LandscapeEditor
{
class ZoneBuilderBase;
}

namespace WorldEditor
{
class WorldEditorScene;
class AbstractWorldItem;

// Auxiliary operations

// Return QGraphicsItem if node contains it
QGraphicsItem *getGraphicsItem(Node *node);

// Scan primitives model for create/add necessary QGraphicsItems
void addNewGraphicsItems(const QModelIndex &primIndex, PrimitivesTreeModel *model, WorldEditorScene *scene);

// Recursive scan primitives model for delete Graphics Items
void removeGraphicsItems(const QModelIndex &primIndex, PrimitivesTreeModel *model, WorldEditorScene *scene);
QList<Path> graphicsItemsToPaths(const QList<QGraphicsItem *> &items, PrimitivesTreeModel *model);

QList<QPolygonF> polygonsFromItems(const QList<QGraphicsItem *> &items);

void updateGraphicsData(AbstractWorldItem *item);

/**
@class CreateWorldCommand
@brief
@details
*/
class CreateWorldCommand: public QUndoCommand
{
public:
	CreateWorldCommand(const QString &fileName, PrimitivesTreeModel *model, QUndoCommand *parent = 0);
	virtual ~CreateWorldCommand();

	virtual void undo();
	virtual void redo();
private:

	const QString m_fileName;
	PrimitivesTreeModel *const m_model;
};

/**
@class LoadLandscapeCommand
@brief
@details
*/
class LoadLandscapeCommand: public QUndoCommand
{
public:
	LoadLandscapeCommand(const QString &fileName, PrimitivesTreeModel *model,
						 LandscapeEditor::ZoneBuilderBase *zoneBuilder, QUndoCommand *parent = 0);
	virtual ~LoadLandscapeCommand();

	virtual void undo();
	virtual void redo();
private:

	Path landIndex;
	int m_id;
	const QString m_fileName;
	PrimitivesTreeModel *const m_model;
	LandscapeEditor::ZoneBuilderBase *const m_zoneBuilder;
};

class CreateRootPrimitiveCommand: public QUndoCommand
{
public:
	CreateRootPrimitiveCommand(const QString &fileName, PrimitivesTreeModel *model,
							   QUndoCommand *parent = 0);
	virtual ~CreateRootPrimitiveCommand();

	virtual void undo();
	virtual void redo();
private:

	const QString m_fileName;
	Path m_rootPrimIndex;
	PrimitivesTreeModel *const m_model;
};

/**
@class LoadPrimitiveCommand
@brief
@details
*/
class LoadRootPrimitiveCommand: public QUndoCommand
{
public:
	LoadRootPrimitiveCommand(const QString &fileName, WorldEditorScene *scene,
							 PrimitivesTreeModel *model, QUndoCommand *parent = 0);
	virtual ~LoadRootPrimitiveCommand();

	virtual void undo();
	virtual void redo();
private:

	Path m_rootPrimIndex;
	const QString m_fileName;
	WorldEditorScene *const m_scene;
	PrimitivesTreeModel *const m_model;
};

/**
@class AddPrimitiveCommand
@brief
@details
*/
class AddPrimitiveByClassCommand: public QUndoCommand
{
public:
	AddPrimitiveByClassCommand(const QString &className, const Path &parentIndex,
							   WorldEditorScene *scene, PrimitivesTreeModel *model,
							   QUndoCommand *parent = 0);
	virtual ~AddPrimitiveByClassCommand();

	virtual void undo();
	virtual void redo();
private:

	QPointF m_initPos;
	float m_delta;
	const QString m_className;
	Path m_parentIndex, m_newPrimIndex;
	WorldEditorScene *m_scene;
	PrimitivesTreeModel *m_model;
};

/**
@class MoveWorldItemsCommand
@brief
@details
*/
class MoveWorldItemsCommand: public QUndoCommand
{
public:
	MoveWorldItemsCommand(const QList<QGraphicsItem *> &items, const QPointF &offset,
						  WorldEditorScene *scene, PrimitivesTreeModel *model,
						  QUndoCommand *parent = 0);
	virtual ~MoveWorldItemsCommand();

	virtual void undo();
	virtual void redo();
private:

	const QList<Path> m_listPaths;
	const QPointF m_offset;
	PrimitivesTreeModel *const m_model;
	WorldEditorScene *m_scene;
	bool m_firstRun;
};

/**
@class RotateWorldItemsCommand
@brief
@details
*/
class RotateWorldItemsCommand: public QUndoCommand
{
public:
	RotateWorldItemsCommand(const QList<QGraphicsItem *> &items, const qreal angle,
							const QPointF &pivot, WorldEditorScene *scene,
							PrimitivesTreeModel *model, QUndoCommand *parent = 0);
	virtual ~RotateWorldItemsCommand();

	virtual void undo();
	virtual void redo();
private:

	const QList<Path> m_listPaths;
	const qreal m_angle;
	const QPointF m_pivot;
	PrimitivesTreeModel *const m_model;
	WorldEditorScene *m_scene;
	bool m_firstRun;
};

/**
@class ScaleWorldItemsCommand
@brief
@details
*/
class ScaleWorldItemsCommand: public QUndoCommand
{
public:
	ScaleWorldItemsCommand(const QList<QGraphicsItem *> &items, const QPointF &factor,
						   const QPointF &pivot, WorldEditorScene *scene,
						   PrimitivesTreeModel *model, QUndoCommand *parent = 0);
	virtual ~ScaleWorldItemsCommand();

	virtual void undo();
	virtual void redo();
private:

	const QList<Path> m_listPaths;
	const QPointF m_factor;
	const QPointF m_pivot;
	PrimitivesTreeModel *const m_model;
	WorldEditorScene *m_scene;
	bool m_firstRun;
};

/**
@class TurnWorldItemsCommand
@brief
@details
*/
class TurnWorldItemsCommand: public QUndoCommand
{
public:
	TurnWorldItemsCommand(const QList<QGraphicsItem *> &items, const qreal angle,
						  WorldEditorScene *scene, PrimitivesTreeModel *model,
						  QUndoCommand *parent = 0);
	virtual ~TurnWorldItemsCommand();

	virtual void undo();
	virtual void redo();
private:

	const QList<Path> m_listPaths;
	const qreal m_angle;
	PrimitivesTreeModel *const m_model;
	WorldEditorScene *m_scene;
	bool m_firstRun;
};

/**
@class TurnWorldItemsCommand
@brief
@details
*/
class ShapeWorldItemsCommand: public QUndoCommand
{
public:
	ShapeWorldItemsCommand(const QList<QGraphicsItem *> &items, const QList<QPolygonF> &polygons,
						   WorldEditorScene *scene, PrimitivesTreeModel *model,
						   QUndoCommand *parent = 0);
	virtual ~ShapeWorldItemsCommand();

	virtual void undo();
	virtual void redo();
private:

	const QList<Path> m_listPaths;
	const QList<QPolygonF> m_redoPolygons;
	const QList<QPolygonF> m_undoPolygons;
	PrimitivesTreeModel *const m_model;
	WorldEditorScene *m_scene;
	bool m_firstRun;
};

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_ACTIONS_H
