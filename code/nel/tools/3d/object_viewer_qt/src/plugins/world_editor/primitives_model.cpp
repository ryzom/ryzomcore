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

#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>
#include <nel/ligo/primitive_class.h>

#include <QtGui>

#include "primitive_item.h"
#include "primitives_model.h"

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

void PrimitivesTreeModel::addPrimitives(const QString &name, NLLIGO::CPrimitives *primitives)
{
	beginResetModel();
	PrimitivesItem *newPrimitives = new PrimitivesItem(name, primitives, m_rootItem);
	m_rootItem->appendChild(newPrimitives);
	for (uint i = 0; i < primitives->RootNode->getNumChildren(); ++i)
	{
		NLLIGO::IPrimitive *childPrim;
		primitives->RootNode->getChild(childPrim, i);
		scanPrimitive(childPrim, newPrimitives);
	}
	endResetModel();
}

void PrimitivesTreeModel::scanPrimitive(NLLIGO::IPrimitive *prim, BaseTreeItem *parent)
{
//	const NLLIGO::CPrimitiveClass *primClass = NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig->getPrimitiveClass(*prim);
//	nlassert (primClass);
//	if (primClass->Type == NLLIGO::CPrimitiveClass::Alias)
//		return;
	if (prim->getClassName() == "CPrimAlias")
		return;

	PrimitiveItem *newItem = new PrimitiveItem(prim, parent);
	parent->appendChild(newItem);
	for (uint i = 0; i < prim->getNumChildren(); ++i)
	{
		NLLIGO::IPrimitive *childPrim;
		prim->getChild(childPrim, i);
		scanPrimitive(childPrim, newItem);
	}
}

} /* namespace WorldEditor */