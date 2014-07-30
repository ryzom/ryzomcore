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

#ifndef TILE_MODEL_H
#define TILE_MODEL_H

#include "nel/misc/types_nl.h"

#include <QAbstractListModel>

class Node;
class TileSetNode;

class TileModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	enum TTileChannel
	{
		TileDiffuse = 0,
		TileAdditive = 1,
		TileAlpha = 2,
	};

	enum TNodeTileType
	{
		Tile128 = 0,
		Tile256 = 1,
		TileTransition = 2,
		TileDisplacement = 3
	};

	enum TTileItemRole
	{
		TilePixmapRole = Qt::UserRole+1,
		TileFilenameRole = Qt::UserRole+2,
		TileIndexRole = Qt::UserRole+3,
		TileFilenameIndexRole = Qt::UserRole+4
	};

	enum TTileZoomFactor
	{
		TileZoom50 = 0,
		TileZoom100 = 1,
		TileZoom200 = 2
	};

	static const int TILE_DISPLACE_BASE_SIZE = 32;
	static const int TILE_TRANSITION_BASE_SIZE = 64;
	static const int TILE_128_BASE_SIZE = 128;
	static const int TILE_256_BASE_SIZE = 256;

	TileModel(const QStringList &headers, QObject *parent = NULL);
	~TileModel();

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent= QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	// Tile Model specific functions
	void appendRow(const QList<Node*> &items);
	void appendRow(Node *item);

	bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );

	void swapRows( int a, int b );

	TileSetNode *createTileSetNode(QString tileSetName);
	static Node *createItemNode( int id, TTileChannel channel, const QString &fileName );

	static const char *getTileTypeName(TNodeTileType type);
	static uint32 getTileTypeSize(TileModel::TNodeTileType type);

	static TTileZoomFactor CurrentZoomFactor;

	bool hasTileSet( const QString &name );

	void clear();

public Q_SLOTS:
	void selectFilenameDisplay(bool selected);
	void selectIndexDisplay(bool selected);	

private:
	Node *getItem(const QModelIndex &index) const;

	bool m_fileDisplay;
	bool m_indexDisplay;
	//TTileZoomFactor m_tileZoomFactor;

	//QList<TileItem*> m_tiles;
	//int m_activeEditChannel;
	Node *rootItem;
};

#endif // TILE_MODEL_H
