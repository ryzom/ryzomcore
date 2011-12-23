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

	rootItem = new Node(rootData);
}

TileModel::~TileModel()
{
	delete rootItem;
}

Node *TileModel::getItem(const QModelIndex &index) const
{
	if(index.isValid())
	{
		Node *item = static_cast<Node*>(index.internalPointer());
		if(item) return item;
	}
	return rootItem;
}

QModelIndex TileModel::index(int row, int column, const QModelIndex &parent) const
{
	if(parent.isValid() && parent.column() != 0)
		return QModelIndex();

	Node *parentItem = getItem(parent);

	Node *childItem = parentItem->child(row);
	if(childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex TileModel::parent(const QModelIndex &index) const
{
	if(!index.isValid())
		return QModelIndex();

	Node *childItem = getItem(index);
	Node *parentItem = childItem->parent();

	if(parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TileModel::rowCount(const QModelIndex &parent) const
{
	Node *parentItem = getItem(parent);
	return parentItem->childCount();
}

int TileModel::columnCount(const QModelIndex &parent) const
{
	Node *parentItem = getItem(parent);
	return parentItem->columnCount();
}

QVariant TileModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	if(role != Qt::DisplayRole)
		return QVariant();

	Node *item = static_cast<Node*>(index.internalPointer());
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

void TileModel::appendRow(const QList<Node*> &items)
{
	rootItem->appendRow(items);
}


void TileModel::appendRow(Node *item)
{
	rootItem->appendRow(item);
}

TileSetNode *TileModel::createTileSetNode(QString tileSetName)
{
	// Create the new tile set.
	TileSetNode *tileSet = new TileSetNode(tileSetName);
			
	// child for 128x128 tiles
	TileTypeNode *tile128= new TileTypeNode(TileTypeNode::Tile128);
	
	// child for 256x256 tiles
	TileTypeNode *tile256= new TileTypeNode(TileTypeNode::Tile256);

	// child for transition tiles.
	TileTypeNode *tileTrans= new TileTypeNode(TileTypeNode::TileTransition);

	// Add the default transition tiles.
	// TODO tie this to CTileSet::count from NeL
	for(int transPos=0; transPos<48; transPos++)
	{				
		TileItemNode *transTile= new TileItemNode(transPos, TileItemNode::TileDiffuse, QString("filename").append(QString::number(transPos+1)));
		tileTrans->appendRow(transTile);
	}

	// child for displacement tiles
	TileTypeNode *tileDisp= new TileTypeNode(TileTypeNode::TileDisplacement);

	// Add the default displacement tiles.
	// TODO tie this to CTileSet::CountDisplace from NeL
	for(int dispPos=0; dispPos<16; dispPos++)
	{
		TileItemNode *dispTile= new TileItemNode(dispPos, TileItemNode::TileDiffuse, QString("filename").append(QString::number(dispPos+1)));
		tileDisp->appendRow(dispTile);
	}

	// Append them in the correct order to the tile set.
	tileSet->appendRow(tile128);
	tileSet->appendRow(tile256);
	tileSet->appendRow(tileTrans);
	tileSet->appendRow(tileDisp);

	this->appendRow(tileSet);

	return tileSet;
}