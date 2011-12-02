// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "tile_model.h"
#include "tile_item.h"

#include <QStringList>

TileModel::TileModel(const QStringList &headers, QObject *parent) : QAbstractItemModel(parent)
{
	QVector<QVariant> rootData;
	Q_FOREACH(QString header, headers)
		rootData << header;

	rootItem = new TileItem(rootData);
}

TileModel::~TileModel()
{
	delete rootItem;
}

TileItem *TileModel::getItem(const QModelIndex &index) const
{
	if(index.isValid())
	{
		TileItem *item = static_cast<TileItem*>(index.internalPointer());
		if(item) return item;
	}
	return rootItem;
}

QModelIndex TileModel::index(int row, int column, const QModelIndex &parent) const
{
	if(parent.isValid() && parent.column() != 0)
		return QModelIndex();

	TileItem *parentItem = getItem(parent);

	TileItem *childItem = parentItem->child(row);
	if(childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex TileModel::parent(const QModelIndex &index) const
{
	if(!index.isValid())
		return QModelIndex();

	TileItem *childItem = getItem(index);
	TileItem *parentItem = childItem->parent();

	if(parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TileModel::rowCount(const QModelIndex &parent) const
{
	TileItem *parentItem = getItem(parent);
	return parentItem->childCount();
}

int TileModel::columnCount(const QModelIndex &parent) const
{
	TileItem *parentItem = getItem(parent);
	return parentItem->columnCount();
}

QVariant TileModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	if(role != Qt::DisplayRole)
		return QVariant();

	TileItem *item = static_cast<TileItem*>(index.internalPointer());
	return item->data(index.column());
}

Qt::ItemFlags TileModel::flags(const QModelIndex &index) const
{
	if(!index.isValid())
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant TileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

void TileModel::appendRow(const QList<TileItem*> &items)
{
	rootItem->appendRow(items);
}


void TileModel::appendRow(TileItem *item)
{
	rootItem->appendRow(item);
}