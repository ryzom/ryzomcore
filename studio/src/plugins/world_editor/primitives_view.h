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


#ifndef PRIMITIVES_VIEW_H
#define PRIMITIVES_VIEW_H

// Project includes
#include "primitive_item.h"

// NeL includes
#include <nel/ligo/primitive.h>

// Qt includes
#include <QtWidgets/QAction>
#include <QtWidgets/QTreeView>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>
#include <QtCore/QSignalMapper>
#include <QtWidgets/QUndoStack>
#include <QtCore/QItemSelection>

namespace LandscapeEditor
{
class ZoneBuilderBase;
}

class QGraphicsItem;

namespace WorldEditor
{
class PrimitivesTreeModel;
class WorldEditorScene;

/**
@class PrimitivesView
@brief
@details
*/
class PrimitivesView : public QTreeView
{
	Q_OBJECT

Q_SIGNALS:
	/// Emitted after one or more .land files finished loading, so the view can show them.
	void landscapeLoaded();

	/// "Close World" was picked on the world row.
	void closeWorldRequested();

	/// "Position..." was picked for these rows.
	void positionRequested(const QModelIndexList &indexes);

	/// Move the map view onto this area of the scene, picked through "Zoom in".
	void zoomToRectRequested(const QRectF &sceneRect);

public:
	/// Hide this one row on the map, or show it again - the eye in the tree. With
	/// children (Shift+click on the eye), the whole branch goes the same way.
	void toggleHidden(const QModelIndex &index, bool withChildren = false);

	/// Select the row, scroll it into view and move the map onto it.
	void focusIndex(const QModelIndex &index);

	explicit PrimitivesView(QWidget *parent = 0);
	~PrimitivesView();

	void setUndoStack(QUndoStack *undoStack);
	void setZoneBuilder(LandscapeEditor::ZoneBuilderBase *zoneBuilder);
	void setWorldScene(WorldEditorScene *worldEditorScene);
	virtual void setModel(PrimitivesTreeModel *model);

	/// Open every collapsed parent of index and scroll it into view, without touching
	/// the selection. Picking a primitive on the map is of little use while its row
	/// sits somewhere inside a folded tree.
	void revealIndex(const QModelIndex &index);

	/// Show the context menu that belongs to index, at a global position. Used by the
	/// tree's own right click and by a right click on the map, so both offer the same
	/// entries. fromScene adds "Show in Tree" on top, which inside the tree is pointless.
	void showContextMenu(const QModelIndex &index, const QPoint &globalPos,
						 bool fromScene = false);

	/// Edit actions with their shortcuts (Ctrl+C/X/V, Del, Ctrl+A, E, R). The window
	/// adds them to itself as well, so they work with the map in focus too.
	QList<QAction *> editActions() const;

public Q_SLOTS:
	void copyPrimitives();
	void cutPrimitives();
	void pastePrimitives();
	void selectAllPrimitives();
	void expandSelected();
	void collapseSelected();

private Q_SLOTS:
	void loadLandscape();
	void loadRootPrimitive();
	void createRootPrimitive();
	void selectChildren();
	void showInTree();
	void zoomToPrimitive();

	void save();
	void saveAs();
	void deletePrimitives();
	void unload();
	void showPrimitive();
	void hidePrimitive();
	void showAllPrimitives();
	void showPrimitiveWithChildren();
	void hidePrimitiveWithChildren();
	void requestPosition();
	void addNewPrimitiveByClass(int value);
	void generatePrimitives(int value);
	void openItem(int value);

protected:
	void contextMenuEvent(QContextMenuEvent *event);

private:
	/// Selected primitive rows without those whose parent is selected as well - they
	/// come along with it anyway.
	QModelIndexList topLevelSelectedPrimitives() const;

	/// Delete these rows as one undo step. Only rows that may be deleted.
	void deleteRows(const QModelIndexList &indexes, const QString &text);

	void selectAllPrimitives(const QModelIndex &parent, QItemSelection &selection);
	void collapseRecursively(const QModelIndex &index);

	void selectChildren(const QModelIndex &parent, QItemSelection &itemSelection);
	void fillMenu_WorldEdit(QMenu *menu);
	void fillMenu_Landscape(QMenu *menu, const QModelIndex &index);
	void fillMenu_RootPrimitive(QMenu *menu, const QModelIndex &index);
	void fillMenu_Primitive(QMenu *menu, const QModelIndex &index);

	/// Union of the map extents of this row and everything below it, in scene
	/// coordinates. Null when nothing in the subtree is drawn on the map.
	QRectF sceneRectOfSubtree(const QModelIndex &index) const;

	/// Area of a land on the map, null if the row is no land or the land is empty.
	QRectF landscapeSceneRect(const QModelIndex &index) const;


	QAction *m_unloadAction;
	QAction *m_saveAction;
	QAction *m_saveAsAction;
	QAction *m_loadLandAction;
	QAction *m_loadPrimitiveAction;
	QAction *m_newPrimitiveAction;
	QAction *m_deleteAction;
	QAction *m_selectChildrenAction;
	QAction *m_helpAction;
	QAction *m_showAction;
	QAction *m_hideAction;
	QAction *m_showAllAction;
	QAction *m_hideWithChildrenAction;
	QAction *m_showWithChildrenAction;
	QAction *m_positionAction;
	QAction *m_copyAction;
	QAction *m_cutAction;
	QAction *m_pasteAction;
	QAction *m_selectAllAction;
	QAction *m_expandAction;
	QAction *m_collapseAction;

	/// Rows Hide/Show act on, see hideTargets() in the source.
	QModelIndexList hideTargets() const;
	void setHidden(const QModelIndex &index, bool hidden, QItemSelection &deselect, bool withChildren);
	void setHiddenOnTargets(bool hidden, bool withChildren);
	void showAllPrimitives(const QModelIndex &parent);
	void addHideActions(QMenu *menu, const QModelIndex &index);
	QAction *m_showInTreeAction;
	QAction *m_zoomToAction;

	/// Index the context menu was last opened for, for "Show in Tree".
	QPersistentModelIndex m_contextMenuIndex;

	QUndoStack *m_undoStack;
	WorldEditorScene *m_worldEditorScene;
	LandscapeEditor::ZoneBuilderBase *m_zoneBuilder;
	PrimitivesTreeModel *m_primitivesTreeModel;
};

} /* namespace WorldEditor */

#endif // PRIMITIVES_VIEW_H
