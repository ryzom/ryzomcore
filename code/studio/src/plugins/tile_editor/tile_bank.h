#ifndef TILE_BANK_H
#define TILE_BANK_H

#include <QString>
#include <QStringList>
#include <QVariant>

#include "tile_constants.h"

class TileBankPvt;

class TileBank
{
public:
	TileBank();
	~TileBank();

	void addTileSet( const QString &name );
	void removeTileSet( int idx );
	void renameTileSet( int idx, const QString &newName );

	void addLand( const QString &name );
	void removeLand( int idx );
	void setLandSets( int idx, const QStringList &l );
	void getLandSets( int idx, QStringList &l );

	bool addTile( int setIdx, const QString &name, const QVariant &pixmap, TileConstants::TTileChannel channel, TileConstants::TNodeTileType type );
	void removeTile( int ts, int type, int tile );
	bool setTile( int tileset, int tile, const QString &name, const QVariant &pixmap, TileConstants::TTileChannel channel, TileConstants::TNodeTileType type );
	void replaceImage( int ts, int type, int tile, TileConstants::TTileChannel channel, const QString &name, const QVariant &pixmap );
	void clearImage( int ts, int type, int tile, TileConstants::TTileChannel channel );
	
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
};

#endif

