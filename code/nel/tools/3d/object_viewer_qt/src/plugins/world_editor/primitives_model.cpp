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
#include "primitive_item.h"
#include "primitives_model.h"
#include "world_editor_misc.h"
#include "world_editor_constants.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_utils.h>

// Qt includes
#include <QtGui>

namespace WorldEditor
{

PrimitivesTreeModel::PrimitivesTreeModel(QObject *parent)
	: QAbstractItemModel(parent),
	  m_worldEditNode(0)
{
	m_rootNode = new Node();
	m_rootNode->setData(Qt::DisplayRole, "Name");
}

PrimitivesTreeModel::~PrimitivesTreeModel()
{
	delete m_rootNode;
}

int PrimitivesTreeModel::columnCount(const QModelIndex &parent) const
{
	/*	if (parent.isValid())
			return static_cast<BaseTreeItem *>(parent.internalPointer())->columnCount();
		else
			return m_rootItem->columnCount();
		*/
	return 1;
}

QVariant PrimitivesTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	Node *item = static_cast<Node *>(index.internalPointer());
	switch (role)
	{
//	case Qt::TextAlignmentRole:
//		return int(Qt::AlignLeft | Qt::AlignVCenter);
	case Qt::DisplayRole:
		return item->data(Qt::DisplayRole);
	case Qt::DecorationRole:
		return item->data(Qt::DecorationRole);
	default:
		return QVariant();
	}
}

Qt::ItemFlags PrimitivesTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PrimitivesTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
//		return m_rootNode->data(section);
		return m_rootNode->data(Qt::DisplayRole);

	return QVariant();
}

QModelIndex PrimitivesTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	Node *parentNode;

	if (!parent.isValid())
		parentNode = m_rootNode;
	else
		parentNode = static_cast<Node *>(parent.internalPointer());

	Node *childNode = parentNode->child(row);
	if (childNode)
		return createIndex(row, column, childNode);
	else
		return QModelIndex();
}

QModelIndex PrimitivesTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	Node *childNode = static_cast<Node *>(index.internalPointer());
	Node *parentNode = childNode->parent();

	if (parentNode == m_rootNode)
		return QModelIndex();

	return createIndex(parentNode->row(), 0, parentNode);
}

int PrimitivesTreeModel::rowCount(const QModelIndex &parent) const
{
	Node *parentNode;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentNode = m_rootNode;
	else
		parentNode = static_cast<Node *>(parent.internalPointer());

	return parentNode->childCount();
}

Path PrimitivesTreeModel::pathFromIndex(const QModelIndex &index)
{
	QModelIndex iter = index;
	Path path;
	while(iter.isValid())
	{
		path.prepend(PathItem(iter.row(), iter.column()));
		iter = iter.parent();
	}
	return path;
}

Path PrimitivesTreeModel::pathFromNode(Node *node)
{
	Node *iter = node;
	Path path;
	while(iter != 0)
	{
		path.prepend(PathItem(iter->row(), 0));
		iter = iter->parent();
	}
	return path;
}

QModelIndex PrimitivesTreeModel::pathToIndex(const Path &path)
{
	QModelIndex iter;
	for(int i = 0; i < path.size(); i++)
	{
		iter = index(path[i].first, path[i].second, iter);
	}
	return iter;
}

Node *PrimitivesTreeModel::pathToNode(const Path &path)
{
	Node *node = m_rootNode;
	for(int i = 1; i < path.size(); i++)
	{
		node = node->child(path[i].first);
	}
	return node;
}

void PrimitivesTreeModel::createWorldEditNode(const QString &fileName)
{
	// World edit node already is created, if yes is showing error message box
	if (m_worldEditNode != 0)
		nlerror("World edit node already is created.");

	beginResetModel();
	m_worldEditNode = new WorldEditNode(fileName);
	m_rootNode->appendChildNode(m_worldEditNode);
	endResetModel();
}

void PrimitivesTreeModel::deleteWorldEditNode()
{
	beginResetModel();
	if (m_worldEditNode != 0)
	{
		delete m_worldEditNode;
		m_worldEditNode = 0;
	}
	endResetModel();
}

bool PrimitivesTreeModel::isWorldEditNodeLoaded() const
{
	if (m_worldEditNode == 0)
		return false;
	else
		return true;
}

Path PrimitivesTreeModel::createLandscapeNode(const QString &fileName, int id, int pos)
{
	if (m_worldEditNode == 0)
		createWorldEditNode("NewWorldEdit");

	QModelIndex parentIndex = index(0, 0, QModelIndex());
	int insPos = pos;
	if (pos == -1)
		insPos = 0;

	beginInsertRows(parentIndex, insPos, insPos);
	LandscapeNode *newNode = new LandscapeNode(fileName, id);
	m_worldEditNode->insertChildNode(insPos, newNode);
	endInsertRows();
	return pathFromIndex(index(0, 0, index(insPos, 0, QModelIndex())));
}


Path PrimitivesTreeModel::createRootPrimitiveNode(const QString &fileName, NLLIGO::CPrimitives *primitives, int pos)
{
	if (m_worldEditNode == 0)
		createWorldEditNode("NewWorldEdit");

	QString name = "NewPrimitive";
	if (!fileName.isEmpty())
		name = fileName;

	int insPos = pos;

	// Get position
	if (pos == AtTheEnd)
		insPos = m_worldEditNode->childCount();

	QModelIndex parentIndex = index(0, 0, QModelIndex());

	// Add root node in tree model
	beginInsertRows(parentIndex, insPos, insPos);
	RootPrimitiveNode *newNode = new RootPrimitiveNode(name, primitives);
	m_worldEditNode->insertChildNode(insPos, newNode);
	endInsertRows();

	newNode->setData(Constants::PRIMITIVE_FILE_IS_CREATED, !fileName.isEmpty());
	newNode->setData(Constants::PRIMITIVE_IS_MODIFIED, false);

	QModelIndex rootPrimIndex = index(insPos, 0, parentIndex);

	// Scan childs items and add in the tree model
	for (uint i = 0; i < primitives->RootNode->getNumChildren(); ++i)
	{
		NLLIGO::IPrimitive *childPrim;
		primitives->RootNode->getChild(childPrim, i);
		createChildNodes(childPrim, i, rootPrimIndex);
	}

	return pathFromIndex(rootPrimIndex);
}

Path PrimitivesTreeModel::createPrimitiveNode(NLLIGO::IPrimitive *primitive, const Path &parent, int pos)
{
	QModelIndex parentIndex = pathToIndex(parent);
	Node *parentNode = static_cast<Node *>(parentIndex.internalPointer());

	int insPos = pos;
	if (pos == AtTheEnd)
		insPos = parentNode->childCount();

	createChildNodes(primitive, insPos, parentIndex);

	return pathFromIndex(index(insPos, 0, parentIndex));
}

void PrimitivesTreeModel::deleteNode(const Path &path)
{
	QModelIndex nodeIndex = pathToIndex(path);
	QModelIndex parentIndex = nodeIndex.parent();
	Node *node = static_cast<Node *>(nodeIndex.internalPointer());

	// Scan childs items and delete from the tree model
	removeChildNodes(node, parentIndex);
}

void PrimitivesTreeModel::createChildNodes(NLLIGO::IPrimitive *primitive, int pos, const QModelIndex &parent)
{
	Node *parentNode = static_cast<Node *>(parent.internalPointer());

	// Add node in the tree model
	beginInsertRows(parent, pos, pos);
	PrimitiveNode *newNode = new PrimitiveNode(primitive);
	parentNode->insertChildNode(pos, newNode);
	endInsertRows();

	// Scan childs items and add in the tree model
	QModelIndex childIndex = index(pos, 0, parent);
	for (uint i = 0; i < primitive->getNumChildren(); ++i)
	{
		NLLIGO::IPrimitive *childPrim;
		primitive->getChild(childPrim, i);
		createChildNodes(childPrim, i, childIndex);
	}
}

void PrimitivesTreeModel::removeChildNodes(Node *node, const QModelIndex &parent)
{
	// Delete all child nodes from the tree model
	while (node->childCount() != 0)
		removeChildNodes(node->child(node->childCount() - 1), parent.child(node->row(), 0));

	// Delete node from the tree model
	beginRemoveRows(parent, node->row(), node->row());
	delete node;
	endRemoveRows();
}

} /* namespace WorldEditor */