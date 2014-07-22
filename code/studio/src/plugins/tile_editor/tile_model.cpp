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

#include <nel/misc/debug.h>

// Initialize the static members
TileModel::TTileZoomFactor TileModel::CurrentZoomFactor;

TileModel::TileModel(const QStringList &headers, QObject *parent) : QAbstractItemModel(parent)
{
	QVector<QVariant> rootData;
	Q_FOREACH(QString header, headers)
		rootData << header;

	rootItem = new Node(rootData);

	TileModel::CurrentZoomFactor = TileModel::TileZoom100;
	m_indexDisplay = true;
	m_fileDisplay = true;
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

	Node *item = static_cast<Node*>(index.internalPointer());
	
	// Translate the display role to the settings-specific role.
	
	if(role == Qt::DisplayRole)
	{
		if(m_indexDisplay && m_fileDisplay)
			role = TileFilenameIndexRole;
		else if(m_fileDisplay)
			role = TileFilenameRole;
		else if(m_indexDisplay)
			role = TileIndexRole;
	}
	return item->data(index.column(), role);
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
		return rootItem->data(section,role);

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
	TileTypeNode *tile128= new TileTypeNode(Tile128);
	tileSet->appendRow(tile128);

	// child for 256x256 tiles
	TileTypeNode *tile256= new TileTypeNode(Tile256);
	tileSet->appendRow(tile256);

	// child for transition tiles.
	TileTypeNode *tileTrans= new TileTypeNode(TileTransition);
	tileSet->appendRow(tileTrans);

	// Add the default transition tiles.
	// TODO tie this to CTileSet::count from NeL
	for(int transPos=0; transPos<48; transPos++)
	{				
		TileItemNode *transTile= new TileItemNode(transPos, TileDiffuse, QString("empty"));
		tileTrans->appendRow(transTile);
	}

	// child for displacement tiles
	TileTypeNode *tileDisp= new TileTypeNode(TileDisplacement);
	tileSet->appendRow(tileDisp);

	// Add the default displacement tiles.
	// TODO tie this to CTileSet::CountDisplace from NeL
	for(int dispPos=0; dispPos<16; dispPos++)
	{
		TileItemNode *dispTile= new TileItemNode(dispPos, TileDiffuse, QString("empty"));
		tileDisp->appendRow(dispTile);
	}

	// Append them in the correct order to the tile set.
	this->appendRow(tileSet);

	return tileSet;
}

const char *TileModel::getTileTypeName(TileModel::TNodeTileType type)
{
	switch(type)
	{
	case Tile128:
		return "128";
	case Tile256:
		return "256";
	case TileTransition:
		return "Transition";
	case TileDisplacement:
		return "Displacement";
	default:
		break;
	}
	return "UNKNOWN";
}

uint32 TileModel::getTileTypeSize(TileModel::TNodeTileType type)
{
	switch(type)
	{
	case Tile128:
		return 128;
	case Tile256:
		return 256;
	case TileTransition:
		return 64;
	case TileDisplacement:
		return 32;
	default:
		break;
	}
	return 0;
}

bool TileModel::hasTileSet( const QString &name )
{
	for( int i = 0; i < rowCount(); i++ )
	{
		QModelIndex idx = index( i, 0 );
		if( !idx.isValid() )
		{
			continue;
		}

		TileSetNode *n = reinterpret_cast< TileSetNode* >( idx.internalPointer() );
		if( n->getTileSetName() == name )
			return true;
	}

	return false;
}

void TileModel::selectFilenameDisplay(bool selected)
{
	m_fileDisplay = selected;
}

void TileModel::selectIndexDisplay(bool selected)
{
	m_indexDisplay = selected;
}