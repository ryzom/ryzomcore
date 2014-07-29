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


#include "tilebank_saver.h"
#include "tile_model.h"
#include "tile_item.h"

#include "nel/3d/tile_bank.h"
#include "nel/misc/file.h"

class TileBankSaverPvt
{
public:
	NL3D::CTileBank bank;

	static NL3D::CTile::TBitmap channelToTBitmap( TileModel::TTileChannel channel )
	{
		NL3D::CTile::TBitmap b = NL3D::CTile::bitmapCount;

		switch( channel )
		{
		case TileModel::TileDiffuse: b = NL3D::CTile::diffuse; break;
		case TileModel::TileAdditive: b = NL3D::CTile::additive; break;
		case TileModel::TileAlpha: b = NL3D::CTile::alpha; break;
		}

		return b;
	}

	NL3D::CTile* addTileToSet( NL3D::CTileSet *set, TileModel::TNodeTileType type )
	{
		int idx = -1;
		int bidx = -1;

		switch( type )
		{
		case TileModel::Tile128:
			{
				set->addTile128( idx, bank );
				bidx = set->getTile128( idx );
				break;
			}

		case TileModel::Tile256:
			{
				set->addTile256( idx, bank );
				bidx = set->getTile256( idx );
				break;
			}
		}

		if( idx == -1 )
			return NULL;

		return bank.getTile( bidx );
	}

	void addTilesToSet( NL3D::CTileSet *set, TileTypeNode *node )
	{
		TileModel::TNodeTileType type = node->getTileType();

		for( int i = 0; i < node->childCount(); i++ )
		{
			TileItemNode *tin = static_cast< TileItemNode* >( node->child( i ) );
			for( int j = TileModel::TileDiffuse; j <= TileModel::TileAlpha; j++ )
			{
				QString fn = tin->getTileFilename( TileModel::TTileChannel( j ) );

				NL3D::CTile *tile = addTileToSet( set, type );
				tile->setFileName( channelToTBitmap( TileModel::TTileChannel( j ) ) , fn.toUtf8().constData() );
			}
		}
	}

	void addTilesToSet( NL3D::CTileSet *set, TileSetNode *node )
	{
		for( int i = TileModel::Tile128; i <= TileModel::Tile256; i++ )
		{
			TileTypeNode *tn = static_cast< TileTypeNode* >( node->child( i ) );
			
			addTilesToSet( set, tn );
		}
	}

	void setupTransitionTile( NL3D::CTileSet *set, TileItemNode *node, int idx )
	{
		NL3D::CTileSetTransition *tr = set->getTransition( idx );
		int tid = tr->getTile();
		NL3D::CTile *tile = bank.getTile( tid );

		if( tile == NULL )
			return;

		for( int i = TileModel::TileDiffuse; i <= TileModel::TileAlpha; i++ )
		{
			QString fn = node->getTileFilename( TileModel::TTileChannel( i ) );
			tile->setFileName( channelToTBitmap( TileModel::TTileChannel( i ) ), fn.toUtf8().constData() );
		}
	}

	void setupTransitionTiles( NL3D::CTileSet *set, TileTypeNode *node )
	{
		for( int i = 0; i < node->childCount(); i++ )
		{
			TileItemNode *tin = static_cast< TileItemNode* >( node->child( i ) );
			setupTransitionTile( set, tin, i );
		}
	}

	void setupTransitionTiles( NL3D::CTileSet *set, TileSetNode *node )
	{
		TileTypeNode *tn = static_cast< TileTypeNode* >( node->child( 2 ) );
		setupTransitionTiles( set, tn );
	}

	void setupDisplacementTile( NL3D::CTileSet *set, TileItemNode *node, int idx )
	{
		set->setDisplacement( NL3D::CTileSet::TDisplacement( idx ), 
							node->getTileFilename( TileModel::TileDiffuse ).toUtf8().constData(),
							bank );
	}

	void setupDisplacementTiles( NL3D::CTileSet *set, TileTypeNode *node )
	{
		for( int i = 0; i < node->childCount(); i++ )
		{
			TileItemNode *tin = static_cast< TileItemNode* >( node->child( i ) );
			setupDisplacementTile( set, tin, i );
		}
	}

	void setupDisplacementTiles( NL3D::CTileSet *set, TileSetNode *node )
	{
		TileTypeNode *tn = static_cast< TileTypeNode* >( node->child( 3 ) );
		setupDisplacementTiles( set, tn );
	}

	void addLands( const QList< Land > &lands )
	{
		QListIterator< Land > itr( lands );
		while( itr.hasNext() )
		{
			bank.addLand( itr.next().name.toUtf8().constData() );
		}
	}


	void addTileSets( const TileModel* model, const QList< Land > &lands )
	{
		// Add the tilesets
		for( int i = 0; i < model->rowCount(); i++ )
		{
			QModelIndex idx = model->index( i, 0 );
			if( !idx.isValid() )
				continue;

			TileSetNode *n = reinterpret_cast< TileSetNode* >( idx.internalPointer() );
			QString set = n->getTileSetName();
			bank.addTileSet( set.toUtf8().constData() );
		}

		// Set the data to tilesets
		for( int i = 0; i < bank.getTileSetCount(); i++ )
		{
			NL3D::CTileSet *set = bank.getTileSet( i );

			QModelIndex idx = model->index( i, 0 );
			if( !idx.isValid() )
				continue;

			TileSetNode *n = reinterpret_cast< TileSetNode* >( idx.internalPointer() );

			addTilesToSet( set, n );
			setupTransitionTiles( set, n );
			setupDisplacementTiles( set, n );
		}

		// Add tilesets to lands
		for( int i = 0; i < bank.getLandCount(); i++ )
		{
			NL3D::CTileLand *land = bank.getLand( i );
			const Land &l = lands[ i ];

			for( int j = 0; j < l.tilesets.count(); j++ )
			{
				land->addTileSet( l.tilesets[ j ].toUtf8().constData() );
			}
		}
	}
};

TileBankSaver::TileBankSaver()
{
	p = new TileBankSaverPvt();
}

TileBankSaver::~TileBankSaver()
{
	delete p;
	p = NULL;
}

bool TileBankSaver::save( const char *fileName, const TileModel* model, const QList< Land > &lands )
{
	p->addLands( lands );
	p->addTileSets( model, lands );

	// Save to file
	NLMISC::COFile f;
	bool b = f.open( fileName, false, false, false );
	if( !b )
		return false;

	p->bank.serial( f );

	f.flush();
	f.close();

	return true;
}

