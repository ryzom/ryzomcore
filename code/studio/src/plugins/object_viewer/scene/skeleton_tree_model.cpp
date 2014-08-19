/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdpch.h"
#include "skeleton_tree_model.h"

// NeL include
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_bone.h>

// Project includes
#include "modules.h"

namespace NLQT
{

CSkeletonTreeModel::CSkeletonTreeModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	QList<QVariant> rootData;
	rootData << "Skeleton";
	_rootItem = new CSkeletonTreeItem(rootData, -1);
}

CSkeletonTreeModel::~CSkeletonTreeModel()
{
	delete _rootItem;
}

int CSkeletonTreeModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<CSkeletonTreeItem *>(parent.internalPointer())->columnCount();
	else
		return _rootItem->columnCount();
}

QVariant CSkeletonTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	CSkeletonTreeItem *item = static_cast<CSkeletonTreeItem *>(index.internalPointer());

	return item->data(index.column());
}

Qt::ItemFlags CSkeletonTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant CSkeletonTreeModel::headerData(int section, Qt::Orientation orientation,
										int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return _rootItem->data(section);

	return QVariant();
}

QModelIndex CSkeletonTreeModel::index(int row, int column, const QModelIndex &parent)
const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	CSkeletonTreeItem *parentItem;

	if (!parent.isValid())
		parentItem = _rootItem;
	else
		parentItem = static_cast<CSkeletonTreeItem *>(parent.internalPointer());

	CSkeletonTreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex CSkeletonTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	CSkeletonTreeItem *childItem = static_cast<CSkeletonTreeItem *>(index.internalPointer());
	CSkeletonTreeItem *parentItem = childItem->parent();

	if (parentItem == _rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int CSkeletonTreeModel::rowCount(const QModelIndex &parent) const
{
	CSkeletonTreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = _rootItem;
	else
		parentItem = static_cast<CSkeletonTreeItem *>(parent.internalPointer());

	return parentItem->childCount();
}

void CSkeletonTreeModel::rebuildModel()
{
	std::string curObj = Modules::objView().getCurrentObject();

	NL3D::USkeleton skel = Modules::objView().getEntity(curObj).getSkeleton();

	if (skel.empty())
	{
		resetTreeModel();
		return;
	}

	beginResetModel();
	delete _rootItem;

	QList<QVariant> rootData;
	rootData << curObj.c_str();

	_rootItem = new CSkeletonTreeItem(rootData, -1);

	CSkeletonTreeItem *parentItem = _rootItem;

	for (uint i = 0; i < skel.getNumBones(); ++i)
	{
		NL3D::UBone bone =  skel.getBone(i);
		sint32 parentId = bone.getObjectPtr()->getFatherId();

		while(parentId != parentItem->getId())
			parentItem = parentItem->parent();

		QList<QVariant> boneData;
		boneData << bone.getObjectPtr()->getBoneName().c_str();
		CSkeletonTreeItem *item = new CSkeletonTreeItem(boneData, i, parentItem);
		parentItem->appendChild(item);
		parentItem = item;
	}

	endResetModel();
}

void CSkeletonTreeModel::resetTreeModel()
{
	beginResetModel();
	delete _rootItem;
	QList<QVariant> rootData;
	rootData << "Skeleton";
	_rootItem = new CSkeletonTreeItem(rootData, -1);
	endResetModel();
}

QModelIndex CSkeletonTreeModel::getIndexFromId(sint id, const QModelIndex &parent)
{
	QModelIndex currentIndex = parent;
	CSkeletonTreeItem *item = static_cast<CSkeletonTreeItem *>(parent.internalPointer());
	if (item->getId() != id)
		for (int i = 0; i < item->childCount(); ++i)
		{
			currentIndex = getIndexFromId(id, index(i, 0, parent));
			CSkeletonTreeItem *item = static_cast<CSkeletonTreeItem *>(currentIndex.internalPointer());
			if (item->getId() == id)
				return currentIndex;
		}
	return currentIndex;
}


} /* namespace NLQT */