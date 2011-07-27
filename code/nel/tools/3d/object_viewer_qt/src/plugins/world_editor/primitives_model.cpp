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

// NeL includes
#include <nel/misc/debug.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_utils.h>

// Qt includes
#include <QtGui>

namespace WorldEditor
{

PrimitivesTreeModel::PrimitivesTreeModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	QList<QVariant> rootData;
	rootData << "Name" << "Class" << "Class";
	m_rootItem = new BaseTreeItem(rootData);
}

PrimitivesTreeModel::~PrimitivesTreeModel()
{
	delete m_rootItem;
}

int PrimitivesTreeModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<BaseTreeItem *>(parent.internalPointer())->columnCount();
	else
		return m_rootItem->columnCount();
}

QVariant PrimitivesTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	BaseTreeItem *item = static_cast<BaseTreeItem *>(index.internalPointer());
	switch (role)
	{
//	case Qt::TextAlignmentRole:
//		return int(Qt::AlignLeft | Qt::AlignVCenter);
	case Qt::DisplayRole:
		return item->data(index.column() + 1);
	case Qt::DecorationRole:
	{
		if (index.column() == 0)
			return qVariantFromValue(item->data(0));
		else
			return QVariant();
	}
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

QVariant PrimitivesTreeModel::headerData(int section, Qt::Orientation orientation,
		int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return m_rootItem->data(section);

	return QVariant();
}

QModelIndex PrimitivesTreeModel::index(int row, int column, const QModelIndex &parent)
const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	BaseTreeItem *parentItem;

	if (!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<BaseTreeItem *>(parent.internalPointer());

	BaseTreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex PrimitivesTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	BaseTreeItem *childItem = static_cast<BaseTreeItem *>(index.internalPointer());
	BaseTreeItem *parentItem = childItem->parent();

	if (parentItem == m_rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int PrimitivesTreeModel::rowCount(const QModelIndex &parent) const
{
	BaseTreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<BaseTreeItem *>(parent.internalPointer());

	return parentItem->childCount();
}

NLLIGO::IPrimitive *PrimitivesTreeModel::primitive(const QModelIndex &index)
{
	NLLIGO::IPrimitive *prim = 0;
	if (index.isValid())
	{
		PrimitiveItem *item = static_cast<PrimitiveItem *>(index.internalPointer());
		prim = item->primitive();
	}
	return prim;
}

const NLLIGO::CPrimitiveClass *PrimitivesTreeModel::primitiveClass(const QModelIndex &index)
{
	if (index.isValid())
	{
		NLLIGO::IPrimitive *prim = primitive(index);
		return ligoConfig()->getPrimitiveClass(*prim);
	}
	return 0;
}

void PrimitivesTreeModel::loadPrimitive(const QString &fileName)
{
	NLLIGO::CPrimitives *primitives = new NLLIGO::CPrimitives();

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = primitives;

	NLLIGO::loadXmlPrimitiveFile(*primitives, fileName.toStdString(), *NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig);

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

	addRootPrimitive(fileName, primitives);
}

void PrimitivesTreeModel::newPrimitiveWithoutUndo(const QString &className, uint id, const QModelIndex &parent)
{
	const NLLIGO::CPrimitiveClass *primClass = primitiveClass(parent);
	float delta = 10;

	// TODO: Set the context
	//CPrimitiveContext::instance().CurrentPrimitive = &_DataHierarchy[locator._LocateStack[0]].Primitives;

	NLLIGO::IPrimitive *newPrimitive = createPrimitive(className.toStdString().c_str(), className.toStdString().c_str()
									   , NLMISC::CVector(), delta, primClass->DynamicChildren[id].Parameters, primitive(parent));

	// unset the context
	//CPrimitiveContext::instance().CurrentPrimitive = NULL;

	if (newPrimitive != 0)
	{
		scanPrimitive(newPrimitive, parent);
	}
}

void PrimitivesTreeModel::deletePrimitiveWithoutUndo(const QModelIndex &index)
{
	deletePrimitive(primitive(index));
	removeRows(index.row(), index.parent());
}

void PrimitivesTreeModel::addRootPrimitive(const QString &name, NLLIGO::CPrimitives *primitives)
{
	beginResetModel();

	// Create root primitive
	RootPrimitiveItem *newPrimitives = new RootPrimitiveItem(name, primitives, m_rootItem);
	m_rootItem->appendChild(newPrimitives);

	// Scan childs items and add in tree model
	for (uint i = 0; i < primitives->RootNode->getNumChildren(); ++i)
	{
		NLLIGO::IPrimitive *childPrim;
		primitives->RootNode->getChild(childPrim, i);
		scanPrimitive(childPrim, newPrimitives);
	}
	endResetModel();
}

void PrimitivesTreeModel::scanPrimitive(NLLIGO::IPrimitive *prim, const QModelIndex &parentIndex)
{
	PrimitiveItem *parent = static_cast<PrimitiveItem *>(parentIndex.internalPointer());

	// Add in tree model
	beginInsertRows(parentIndex, parent->childCount(), parent->childCount());
	PrimitiveItem *newItem = new PrimitiveItem(prim, parent);
	parent->appendChild(newItem);
	endInsertRows();

	// Scan childs items and add in tree model
	QModelIndex childIndex = index(parent->childCount() - 1, 0, parentIndex);
	for (uint i = 0; i < prim->getNumChildren(); ++i)
	{
		NLLIGO::IPrimitive *childPrim;
		prim->getChild(childPrim, i);
		scanPrimitive(childPrim, childIndex);
	}
}

void PrimitivesTreeModel::scanPrimitive(NLLIGO::IPrimitive *prim, BaseTreeItem *parent)
{
	// Add in tree model
	PrimitiveItem *newItem = new PrimitiveItem(prim, parent);
	parent->appendChild(newItem);

	// Scan childs items and add in tree model
	for (uint i = 0; i < prim->getNumChildren(); ++i)
	{
		NLLIGO::IPrimitive *childPrim;
		prim->getChild(childPrim, i);
		scanPrimitive(childPrim, newItem);
	}
}

void PrimitivesTreeModel::removeRows(int position, const QModelIndex &parent)
{
	BaseTreeItem *item = static_cast<BaseTreeItem *>(parent.internalPointer())->child(position);

	// Delete all child items from tree model
	while (item->childCount() != 0)
		removeRows(0, parent.child(position, 0));

	// Delete item
	beginRemoveRows(parent, position, position);
	static_cast<BaseTreeItem *>(parent.internalPointer())->deleteChild(position);
	endRemoveRows();
}

NLLIGO::CLigoConfig *PrimitivesTreeModel::ligoConfig() const
{
	return NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig;
}

} /* namespace WorldEditor */