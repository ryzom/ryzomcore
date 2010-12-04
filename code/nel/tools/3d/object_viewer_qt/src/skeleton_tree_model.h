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

#ifndef SKELETON_TREE_MODEL_H
#define SKELETON_TREE_MODEL_H


// Qt includes
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtGui/QIcon>

// NeL includes
#include "nel/misc/types_nl.h"

// Projects includes

namespace NLQT
{

/**
@class CSkeletonTreeItem
@brief Basic elements tree model skeleton.
*/
class CSkeletonTreeItem
{
public:
	CSkeletonTreeItem(const QList<QVariant> &data, const sint32 id, CSkeletonTreeItem *parent = 0)
		: _itemData(data), _id(id), _parentItem(parent) {}

	~CSkeletonTreeItem()
	{
		qDeleteAll(_childItems);
	}

	void appendChild(CSkeletonTreeItem *child)
	{
		_childItems.append(child);
	}

	CSkeletonTreeItem *child(int row)
	{
		return _childItems.value(row);
	}
	int childCount() const
	{
		return _childItems.count();
	}
	int columnCount() const
	{
		return _itemData.count();
	}
	QVariant data(int column) const
	{
		return _itemData.value(column);
	}

	int row() const
	{
		if (_parentItem)
			return _parentItem->_childItems.indexOf(const_cast<CSkeletonTreeItem *>(this));
		return 0;
	}

	CSkeletonTreeItem *parent()
	{
		return _parentItem;
	}
	sint32 getId()
	{
		return _id;
	}

private:

	QList<CSkeletonTreeItem *> _childItems;
	QList<QVariant> _itemData;
	sint32 _id;
	CSkeletonTreeItem *_parentItem;
};

/**
@class CSkeletonTreeModel
@brief Tree model skeleton.
*/
class CSkeletonTreeModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	CSkeletonTreeModel(QObject *parent = 0);
	~CSkeletonTreeModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex getIndexFromId(sint id, const QModelIndex &parent);

	/// Clear tree model skeleton
	void resetTreeModel();

public Q_SLOTS:

	/// Update tree model skeleton
	void rebuildModel();

private:

	CSkeletonTreeItem *_rootItem;
};

} /* namespace NLQT */

#endif // SKELETON_TREE_MODEL_H
