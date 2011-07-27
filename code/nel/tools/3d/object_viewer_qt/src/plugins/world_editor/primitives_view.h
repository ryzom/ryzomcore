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

// NeL includes
#include <nel/ligo/primitive.h>

// Qt includes
#include <QtGui/QAction>
#include <QtGui/QTreeView>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>
#include <QtCore/QSignalMapper>
#include <QPersistentModelIndex>

namespace WorldEditor
{

class BaseTreeItem;
class PrimitivesTreeModel;

/**
@class PrimitivesView
@brief
@details
*/
class PrimitivesView : public QTreeView
{
	Q_OBJECT

public:
	PrimitivesView(QWidget *parent = 0);
	~PrimitivesView();

	virtual void setModel(PrimitivesTreeModel *model);

private Q_SLOTS:
	void clickedItem(const QModelIndex &index);
	void deletePrimitives();
	void addNewPrimitive(int value);
	void generatePrimitives(int value);
	void openItem(int value);

protected:
	void contextMenuEvent(QContextMenuEvent *event);

	QAction *m_deleteAction;
	QAction *m_selectChildrenAction;
	QAction *m_helpAction;
	QAction *m_showAction;
	QAction *m_hideAction;

	PrimitivesTreeModel *m_primitivesTreeModel;
};

} /* namespace WorldEditor */

#endif // PRIMITIVES_VIEW_H
