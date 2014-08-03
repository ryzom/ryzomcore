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
			NL3D::CTile *tile = addTileToSet( set, type );

			for( int j = TileModel::TileDiffuse; j < TileModel::TileAlpha; j++ )
			{
				QString fn = tin->getTileFilename( TileModel::TTileChannel( j ) );
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

	void setupTiles128( NL3D::CTileSet *set, TileTypeNode *node )
	{
		TileItemNode *tileNode = NULL;
		for( int i = 0; i < node->childCount(); i++ )
		{
			tileNode = static_cast< TileItemNode* >( node->child( i ) );

			for( int j = TileModel::TileDiffuse; j < TileModel::TileAlpha; j++ )
			{
				TileModel::TTileChannel channel = TileModel::TTileChannel( j );
				NL3D::CTile::TBitmap bm = channelToTBitmap( channel );
				const NL3D::CTileBorder &border = tileNode->border( channel );

				if( tileNode->borderFirst( channel ) )
					set->setBorder( bm, border );

				set->setTile128( i, tileNode->getTileFilename( channel ).toUtf8().constData(), bm, bank );
			}
		}
	}

	void setupTiles256( NL3D::CTileSet *set, TileTypeNode *node )
	{
		TileItemNode *tileNode = NULL;
		for( int i = 0; i < node->childCount(); i++ )
		{
			tileNode = static_cast< TileItemNode* >( node->child( i ) );

			for( int j = TileModel::TileDiffuse; j < TileModel::TileAlpha; j++ )
			{
				TileModel::TTileChannel channel = TileModel::TTileChannel( j );
				NL3D::CTile::TBitmap bm = channelToTBitmap( channel );
				const NL3D::CTileBorder &border = tileNode->border( channel );

				if( tileNode->borderFirst( channel ) )
					set->setBorder( bm, border );

				set->setTile256( i, tileNode->getTileFilename( channel ).toUtf8().constData(), bm, bank );
			}
		}
	}

	void setupTransitionTile( NL3D::CTileSet *set, TileItemNode *node, int idx )
	{
		TileModel::TTileChannel channel;
		NL3D::CTile::TBitmap bm;
		NL3D::CTileSet::TTransition tr;

		// Diffuse, Additive
		for( int i = TileModel::TileDiffuse; i < TileModel::TileAlpha; i++ )
		{
			channel =TileModel::TTileChannel( i );
			bm = channelToTBitmap( channel );
			tr = NL3D::CTileSet::TTransition( idx );
			const NL3D::CTileBorder &border = node->border( channel );

			if( node->borderFirst( channel ) )
				set->setBorder( bm, border );
			set->setTileTransition( tr, node->getTileFilename( channel ).toUtf8().constData(), bm, bank, border );
		}

		// Alpha
		{
			channel = TileModel::TileAlpha;
			bm = channelToTBitmap( channel );
			tr = NL3D::CTileSet::TTransition( idx );
			const NL3D::CTileBorder &border = node->border( channel );
			int rot = node->alphaRot();

			set->setTileTransitionAlpha( tr, node->getTileFilename( channel ).toUtf8().constData(), bank, border, rot );
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

	void setupTiles( NL3D::CTileSet *set, TileSetNode *node )
	{
		TileTypeNode *tn128 = static_cast< TileTypeNode* >( node->child( 0 ) );
		TileTypeNode *tn256 = static_cast< TileTypeNode* >( node->child( 1 ) );

		setupTiles128( set, tn128 );
		setupTiles256( set, tn256 );
		setupTransitionTiles( set, node );
		setupDisplacementTiles( set, node );
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
			setupTiles( set, n );
			set->setOriented( n->isOriented() );
			set->setTileVegetableDescFileName( n->vegetSet().toUtf8().constData() );
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

	p->bank.setAbsPath( model->texturePath().toUtf8().constData() );

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

