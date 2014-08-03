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
	void addLand( const QString &name );
	void setLandSets( int idx, const QStringList &l );

	bool addTileToSet( int idx, const QString &name, const QVariant &pixmap, TileConstants::TTileChannel channel, TileConstants::TNodeTileType type );

private:
	TileBankPvt *m_pvt;
};

#endif

