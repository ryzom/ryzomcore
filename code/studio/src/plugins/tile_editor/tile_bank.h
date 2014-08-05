// Ryzom Core Studio - Tile Editor plugin
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


#ifndef TILE_BANK_H
#define TILE_BANK_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QList>

#include "tile_constants.h"
#include "tile_images.h"

namespace NLMISC
{
	class IStream;
}

class TileBankPvt;

class TileBank
{
public:
	TileBank();
	~TileBank();

	void addTileSet( const QString &name );
	void removeTileSet( int idx );
	void renameTileSet( int idx, const QString &newName );
	void getTileSets( QStringList &l );

	void addLand( const QString &name );
	void removeLand( int idx );
	void getLands( QStringList &l );
	void setLandSets( int idx, const QStringList &l );
	void getLandSets( int idx, QStringList &l );

	bool addTile( int setIdx, const QString &name, const QVariant &pixmap, TileConstants::TTileChannel channel, TileConstants::TNodeTileType type );
	void removeTile( int ts, int type, int tile );
	bool setTile( int tileset, int tile, const QString &name, const QVariant &pixmap, TileConstants::TTileChannel channel, TileConstants::TNodeTileType type );
	void replaceImage( int ts, int type, int tile, TileConstants::TTileChannel channel, const QString &name, const QVariant &pixmap );
	void clearImage( int ts, int type, int tile, TileConstants::TTileChannel channel );


	int getTileCount( int tileSet, TileConstants::TNodeTileType type );
	int getRealTileId( int tileSet, TileConstants::TNodeTileType type, int tileIdInSet );
	void getTileImages( int tileSet, TileConstants::TNodeTileType type, int tileId, TileImages &images );
	void getTileImages( int tileSet, TileConstants::TNodeTileType type, QList< TileImages > &l );

	void setVegetation( int tileSet, const QString &vegetation );
	QString getVegetation( int tileSet ) const;

	void setOriented( int tileSet, bool b );
	bool getOriented( int tileSet ) const;

	void setSurfaceData( int tileSet, unsigned long data );
	unsigned long getSurfaceData( int tileSet ) const;

	void setTexturePath( const QString &path );
	QString getTexturePath() const;

	void setRotation( int rotation );

	void serial( NLMISC::IStream &f );
	
	bool hasError() const{ return m_hasError; }
	QString getLastError() const{ return m_lastError; }
	void resetError(){
		m_hasError = false;
		m_lastError = "";
	}

	void setError( const QString &msg )
	{
		m_hasError = true;
		m_lastError = msg;
	}

private:
	TileBankPvt *m_pvt;
	QString m_lastError;
	bool m_hasError;
	int m_rotation;
};

#endif

