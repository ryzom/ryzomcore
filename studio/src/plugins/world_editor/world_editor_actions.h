// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include "world_editor_misc.h"

// NeL includes

// STL includes
#include <set>
#include <string>

// Qt includes
#include <QStringList>
#include <QUndoCommand>
#include <QGraphicsScene>
#include <QTreeView>
#include <QGraphicsItem>
#include <QPersistentModelIndex>

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

QList<QPolygonF> polygonsFromItems(const QList<QGraphicsItem *> &items);


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


/// What the window shares with the load commands and Save
struct WorldFiles
{
	/// The files named in the loaded .worldedit that are not loaded, by their absolute names: the load
	/// commands keep it in step, and Save keeps their elements in the file
	Utils::FilePathSet NotLoaded;

	/// What the editor and the disk had for each file when it was loaded or last written, from which
	/// Save tells the files the editor changed and the files that changed on disk
	Utils::FileBaselines Baselines;

	/// The files that a redo of a load command did not load, which the window reports in one message
	/// once the redo is over
	QStringList RedoFailures;
};

class WorldSaver
{
public:
	WorldSaver(PrimitivesTreeModel *model, LandscapeEditor::ZoneBuilderBase *zoneBuilder, const std::string &dataDir, const std::string &context,
		WorldFiles *files);

	/// The .worldedit as it was loaded or last saved: Save then rewrites that file and keeps what it does
	/// not change, the elements of the files that are not loaded among them
	void setLoadedFile(const std::string &fileName, const std::string &content, const std::string &dataDir,
		const std::string &context);

	/// Makes the content of the .worldedit for the world as it is in the model, which worldEditContent
	/// then gives; false with the reason in getLastError when it cannot be made
	bool makeWorldEdit();

	/// Makes the content of every land and primitive file of the model and of the .worldedit, and finds
	/// what each file needs from its baseline in the world files; nothing is written. False with the
	/// reason in getLastError when a content cannot be made
	bool prepare();

	/// After prepare, the files the editor changed that also changed on disk since they were loaded or
	/// last written
	std::vector<std::string> conflicts() const;

	/// After prepare, writes the files the editor changed, the land and primitive files first and the
	/// .worldedit last, the conflicting ones only when replaceConflicts, and updates their baselines;
	/// false with the reason in getLastError at the first file that cannot be written
	bool write(bool replaceConflicts);

	/// The files that write wrote, also when it failed
	const std::vector<std::string> &writtenFiles() const { return m_writtenFiles; }

	/// After write, whether the .worldedit on disk has the content made for it, written or already there
	bool worldEditSaved() const { return m_worldEditSaved; }

	std::string getLastError() const{ return lastError; }

	/// After makeWorldEdit or prepare, the name and the content of the .worldedit
	const std::string &worldEditFileName() const { return m_worldEditFileName; }
	const std::string &worldEditContent() const { return m_worldEditContent; }

private:
	struct SavedFile
	{
		std::string FileName;
		std::string Content;
		Utils::FileAction Action;
		/// The land or root primitive node, NULL for the .worldedit
		Node *FileNode;
	};

	bool prepareLandscapeNode(Node *node);
	bool preparePrimitiveNode(Node *node);
	void addSavedFile(const std::string &fileName, const std::string &content, Node *node);
	bool makeWorldEditFile();
	void addDatabaseElement(struct _xmlNode *rootNode, const std::pair<std::string, std::string> &entry, const std::string &folder);
	struct _xmlDoc *updateLoadedFile(const std::string &fn);

	std::string m_dataDir;
	std::string m_context;

	std::string m_loadedFileName;
	std::string m_loadedContent;
	std::string m_loadedDataDir;
	std::string m_loadedContext;
	WorldFiles *m_worldFiles;
	PrimitivesTreeModel *m_model;
	LandscapeEditor::ZoneBuilderBase *m_zoneBuilder;

	std::vector< std::pair< std::string, std::string > > m_data;

	std::string m_worldEditFileName;
	std::string m_worldEditContent;

	std::vector<SavedFile> m_savedFiles;
	std::vector<std::string> m_writtenFiles;
	bool m_worldEditSaved;

	// the file whose content is being made, for the error
	std::string m_currentFile;

	std::string lastError;
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
						 LandscapeEditor::ZoneBuilderBase *zoneBuilder, WorldFiles *files, QUndoCommand *parent = 0);
	virtual ~LoadLandscapeCommand();

	virtual void undo();
	virtual void redo();

	/// False when the land file did not load, the command then added no node
	bool isLoaded() const { return m_loaded; }

	/// Why the land file did not load
	const QString &error() const { return m_error; }
private:

	Path landIndex;
	int m_id;
	const QString m_fileName;
	PrimitivesTreeModel *const m_model;
	LandscapeEditor::ZoneBuilderBase *const m_zoneBuilder;
	WorldFiles *const m_files;
	bool m_loaded;
	bool m_notLoadedChanged;
	bool m_firstRedo;
	QString m_error;
};

/**
@class UnloadLandscapeCommand
@brief
@details
*/
class UnloadLandscapeCommand: public QUndoCommand
{
public:
	UnloadLandscapeCommand(const QModelIndex &index, PrimitivesTreeModel *model,
						   LandscapeEditor::ZoneBuilderBase *zoneBuilder, QUndoCommand *parent = 0);
	virtual ~UnloadLandscapeCommand();

	virtual void undo();
	virtual void redo();
private:

	Path m_path;
	int m_id;
	QString m_fileName;
	PrimitivesTreeModel *const m_model;
	LandscapeEditor::ZoneBuilderBase *const m_zoneBuilder;
};

/**
@class CreateRootPrimitiveCommand
@brief
@details
*/
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
@class LoadRootPrimitiveCommand
@brief
@details
*/
class LoadRootPrimitiveCommand: public QUndoCommand
{
public:
	LoadRootPrimitiveCommand(const QString &fileName, WorldEditorScene *scene,
							 PrimitivesTreeModel *model, QTreeView *view,
							 WorldFiles *files, QUndoCommand *parent = 0);
	virtual ~LoadRootPrimitiveCommand();

	virtual void undo();
	virtual void redo();

	/// False when the primitive file did not load, the command then added no node
	bool isLoaded() const { return m_loaded; }
private:

	Path m_rootPrimIndex;
	const QString m_fileName;
	WorldEditorScene *const m_scene;
	PrimitivesTreeModel *const m_model;
	QTreeView *m_view;
	WorldFiles *const m_files;
	bool m_loaded;
	bool m_notLoadedChanged;
	bool m_firstRedo;
};

/**
@class UnloadRootPrimitiveCommand
@brief
@details
*/
class UnloadRootPrimitiveCommand: public QUndoCommand
{
public:
	UnloadRootPrimitiveCommand(const QModelIndex &index, WorldEditorScene *scene,
							   PrimitivesTreeModel *model, QTreeView *view,
							   QUndoCommand *parent = 0);
	virtual ~UnloadRootPrimitiveCommand();

	virtual void undo();
	virtual void redo();
private:

	Path m_path;
	QString m_fileName;
	NLLIGO::CPrimitives *m_primitives;
	WorldEditorScene *const m_scene;
	PrimitivesTreeModel *const m_model;
	QTreeView *m_view;
};

/**
@class AddPrimitiveByClassCommand
@brief
@details
*/
class AddPrimitiveByClassCommand: public QUndoCommand
{
public:
	AddPrimitiveByClassCommand(const QString &className, const Path &parentIndex,
							   WorldEditorScene *scene, PrimitivesTreeModel *model,
							   QTreeView *view, QUndoCommand *parent = 0);
	virtual ~AddPrimitiveByClassCommand();

	virtual void undo();
	virtual void redo();
private:

	QPointF m_initPos;
	float m_delta;
	const QString m_className;
	Path m_parentIndex, m_newPrimIndex;
	WorldEditorScene *const m_scene;
	PrimitivesTreeModel *const m_model;
	QTreeView *const m_view;
};

/**
@class DeletePrimitiveCommand
@brief
@details
*/
class DeletePrimitiveCommand: public QUndoCommand
{
public:
	DeletePrimitiveCommand(const QModelIndex &index, PrimitivesTreeModel *model,
						   WorldEditorScene *scene, QTreeView *view, QUndoCommand *parent = 0);
	virtual ~DeletePrimitiveCommand();

	virtual void undo();
	virtual void redo();
private:

	Path m_path, m_parentPath;
	uint m_posPrimitive;
	NLLIGO::IPrimitive *m_oldPrimitive;
	WorldEditorScene *const m_scene;
	PrimitivesTreeModel *const m_model;
	QTreeView *const m_view;
};

/**
@class AbstractWorldItemCommand
@brief
@details
*/
class AbstractWorldItemCommand: public QUndoCommand
{
public:
	AbstractWorldItemCommand(const QList<QGraphicsItem *> &items, WorldEditorScene *scene,
							 PrimitivesTreeModel *model, QUndoCommand *parent = 0);
	virtual ~AbstractWorldItemCommand();

	virtual void undo();
	virtual void redo();

protected:
	virtual void undoChangeItem(int i, AbstractWorldItem *item) = 0;
	virtual void redoChangeItem(int i, AbstractWorldItem *item) = 0;
	void updatePrimitiveData(AbstractWorldItem *item);

private:
	QList<Path> graphicsItemsToPaths(const QList<QGraphicsItem *> &items, PrimitivesTreeModel *model);

	const QList<Path> m_listPaths;
	PrimitivesTreeModel *const m_model;
	WorldEditorScene *const m_scene;
	bool m_firstRun;
};

/**
@class MoveWorldItemsCommand
@brief
@details
*/
class MoveWorldItemsCommand: public AbstractWorldItemCommand
{
public:
	MoveWorldItemsCommand(const QList<QGraphicsItem *> &items, const QPointF &offset,
						  WorldEditorScene *scene, PrimitivesTreeModel *model,
						  QUndoCommand *parent = 0);
	virtual ~MoveWorldItemsCommand();

protected:
	virtual void undoChangeItem(int i, AbstractWorldItem *item);
	virtual void redoChangeItem(int i, AbstractWorldItem *item);

private:

	const QPointF m_offset;
};

/**
@class RotateWorldItemsCommand
@brief
@details
*/
class RotateWorldItemsCommand: public AbstractWorldItemCommand
{
public:
	RotateWorldItemsCommand(const QList<QGraphicsItem *> &items, const qreal angle,
							const QPointF &pivot, WorldEditorScene *scene,
							PrimitivesTreeModel *model, QUndoCommand *parent = 0);
	virtual ~RotateWorldItemsCommand();

protected:
	virtual void undoChangeItem(int i, AbstractWorldItem *item);
	virtual void redoChangeItem(int i, AbstractWorldItem *item);

private:

	const qreal m_angle;
	const QPointF m_pivot;
};

/**
@class ScaleWorldItemsCommand
@brief
@details
*/
class ScaleWorldItemsCommand: public AbstractWorldItemCommand
{
public:
	ScaleWorldItemsCommand(const QList<QGraphicsItem *> &items, const QPointF &factor,
						   const QPointF &pivot, WorldEditorScene *scene,
						   PrimitivesTreeModel *model, QUndoCommand *parent = 0);
	virtual ~ScaleWorldItemsCommand();

protected:
	virtual void undoChangeItem(int i, AbstractWorldItem *item);
	virtual void redoChangeItem(int i, AbstractWorldItem *item);

private:

	const QPointF m_factor;
	const QPointF m_pivot;
};

/**
@class TurnWorldItemsCommand
@brief
@details
*/
class TurnWorldItemsCommand: public AbstractWorldItemCommand
{
public:
	TurnWorldItemsCommand(const QList<QGraphicsItem *> &items, const qreal angle,
						  WorldEditorScene *scene, PrimitivesTreeModel *model,
						  QUndoCommand *parent = 0);
	virtual ~TurnWorldItemsCommand();

protected:
	virtual void undoChangeItem(int i, AbstractWorldItem *item);
	virtual void redoChangeItem(int i, AbstractWorldItem *item);

private:

	const qreal m_angle;
};

/**
@class TurnWorldItemsCommand
@brief
@details
*/
class ShapeWorldItemsCommand: public AbstractWorldItemCommand
{
public:
	ShapeWorldItemsCommand(const QList<QGraphicsItem *> &items, const QList<QPolygonF> &polygons,
						   WorldEditorScene *scene, PrimitivesTreeModel *model,
						   QUndoCommand *parent = 0);
	virtual ~ShapeWorldItemsCommand();

protected:
	virtual void undoChangeItem(int i, AbstractWorldItem *item);
	virtual void redoChangeItem(int i, AbstractWorldItem *item);

private:

	const QList<QPolygonF> m_redoPolygons;
	const QList<QPolygonF> m_undoPolygons;
};

} /* namespace WorldEditor */

// Enable the use of QVariant with this class.
Q_DECLARE_METATYPE(QPersistentModelIndex *)

#endif // WORLD_EDITOR_ACTIONS_H
