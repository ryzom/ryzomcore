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


#ifndef PRIMITIVES_VIEW_H
#define PRIMITIVES_VIEW_H

// Project includes
#include "primitive_item.h"

// NeL includes
#include <nel/ligo/primitive.h>

// Qt includes
#include <QtGui/QAction>
#include <QtGui/QTreeView>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>
#include <QtCore/QSignalMapper>
#include <QtGui/QUndoStack>
#include <QtGui/QItemSelection>

namespace LandscapeEditor
{
class ZoneBuilderBase;
}

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

public:
	explicit PrimitivesView(QWidget *parent = 0);
	~PrimitivesView();

	void setUndoStack(QUndoStack *undoStack);
	void setZoneBuilder(LandscapeEditor::ZoneBuilderBase *zoneBuilder);
	void setWorldScene(WorldEditorScene *worldEditorScene);
	virtual void setModel(PrimitivesTreeModel *model);

private Q_SLOTS:
	void loadLandscape();
	void loadRootPrimitive();
	void createRootPrimitive();
	void selectChildren();

	void save();
	void saveAs();
	void deletePrimitives();
	void unload();
	void showPrimitive();
	void hidePrimitive();
	void addNewPrimitiveByClass(int value);
	void generatePrimitives(int value);
	void openItem(int value);

protected:
	void contextMenuEvent(QContextMenuEvent *event);

private:
	void selectChildren(const QModelIndex &parent, QItemSelection &itemSelection);
	void fillMenu_WorldEdit(QMenu *menu);
	void fillMenu_Landscape(QMenu *menu);
	void fillMenu_RootPrimitive(QMenu *menu, const QModelIndex &index);
	void fillMenu_Primitive(QMenu *menu, const QModelIndex &index);

	QString m_lastDir;

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

	QUndoStack *m_undoStack;
	WorldEditorScene *m_worldEditorScene;
	LandscapeEditor::ZoneBuilderBase *m_zoneBuilder;
	PrimitivesTreeModel *m_primitivesTreeModel;
};

} /* namespace WorldEditor */

#endif // PRIMITIVES_VIEW_H
