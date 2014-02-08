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


#ifndef PRIMITIVES_MODEL_H
#define PRIMITIVES_MODEL_H

// NeL includes
#include <nel/misc/vector.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_class.h>
#include <nel/ligo/ligo_config.h>

// Qt includes
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace WorldEditor
{
class Node;
class WorldEditNode;

const int AtTheEnd = -1;

typedef QPair<int, int> PathItem;
/*
@typedef Path
@brief It store a list of row and column numbers which have to walk through from the root index of the model to reach the need item.
It is used for undo/redo commands.
*/
typedef QList<PathItem> Path;

/**
@class PrimitivesTreeModel
@brief
@details
*/
class PrimitivesTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit PrimitivesTreeModel(QObject *parent = 0);
	~PrimitivesTreeModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	/// Convert QModelIndex to the persistent index - @Path.
	/// @Path is a list of [row,column] pairs showing us the way through the model.
	Path pathFromIndex(const QModelIndex &index);
	QModelIndex pathToIndex(const Path &path);

	Path pathFromNode(Node *node);
	Node *pathToNode(const Path &path);

	void createWorldEditNode(const QString &fileName);
	void deleteWorldEditNode();
	bool isWorldEditNodeLoaded() const;

	/// Add new landscape node in tree model.
	Path createLandscapeNode(const QString &fileName, int id, int pos = AtTheEnd);

	/// Add new root primitive node and all sub-primitives in the tree model.
	Path createRootPrimitiveNode(const QString &fileName, NLLIGO::CPrimitives *primitives, int pos = AtTheEnd);

	/// Add new primitive node and all sub-primitives in the tree model.
	Path createPrimitiveNode(NLLIGO::IPrimitive *primitive, const Path &parent, int pos = AtTheEnd);

	/// Delete node and all child nodes from the tree model
	void deleteNode(const Path &path);

private:
	void createChildNodes(NLLIGO::IPrimitive *primitive, int pos, const QModelIndex &parent);
	void removeChildNodes(Node *node, const QModelIndex &parent);

	Node *m_rootNode;
	WorldEditNode *m_worldEditNode;
};

} /* namespace WorldEditor */

#endif // PRIMITIVES_MODEL_H
