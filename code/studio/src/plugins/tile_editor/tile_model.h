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

#include "tile_constants.h"

class Node;
class TileSetNode;
class TileItemNode;
class TileBank;

class TileModel : public QAbstractItemModel
{
	Q_OBJECT

public:
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
	static TileItemNode *createItemNode( TileConstants::TNodeTileType type, int id, TileConstants::TTileChannel channel, const QString &fileName );
	TileItemNode *createItemNode( int idx, TileConstants::TNodeTileType type, int id, TileConstants::TTileChannel channel, const QString &fileName );

	static const char *getTileTypeName(TileConstants::TNodeTileType type);
	static uint32 getTileTypeSize(TileConstants::TNodeTileType type);

	static TTileZoomFactor CurrentZoomFactor;

	bool hasTileSet( const QString &name );

	void clear();

	void addLand( const QString &name );
	void removeLand( int idx );
	void removeTileSet( int idx );
	void renameTileSet( int idx, const QString &newName );
	void setLandSets( int idx, const QStringList &l );
	void getLandSets( int idx, QStringList &l );
	void removeTile( int ts, int type, int tile );
	bool replaceImage( int ts, int type, int tile, TileConstants::TTileChannel channel, const QString &name );
	void clearImage( int ts, int type, int tile, TileConstants::TTileChannel channel );

	void setVegetation( int tileSet, const QString &vegetation );
	QString getVegetation( int tileSet ) const;

	void setOriented( int tileSet, bool b );
	bool getOriented( int tileSet ) const;

	void setSurfaceData( int tileSet, unsigned long data );
	unsigned long getSurfaceData( int tileSet ) const;

	void setTexturePath( const QString &path );
	QString getTexturePath() const;

	QString getLastError() const;
	bool hasError() const;

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

	TileBank *m_tileBank;
};

#endif // TILE_MODEL_H
