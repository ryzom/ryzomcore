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

void addNewGraphicsItems(const QModelIndex &primIndex, PrimitivesTreeModel *model, WorldEditorScene *scene);
void removeGraphicsItems(const QModelIndex &primIndex, PrimitivesTreeModel *model, WorldEditorScene *scene);

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
	CreateRootPrimitiveCommand(const QString &fileName, PrimitivesTreeModel *model, QUndoCommand *parent = 0);
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
							   PrimitivesTreeModel *model, QUndoCommand *parent = 0);
	virtual ~AddPrimitiveByClassCommand();

	virtual void undo();
	virtual void redo();
private:

	const QString m_className;
	Path m_parentIndex, m_newPrimIndex;
	PrimitivesTreeModel *m_model;
};

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_ACTIONS_H
