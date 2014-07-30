#include "tilebank_loader.h"

#include "tile_model.h"
#include "tile_item.h"

#include "nel/3d/tile_bank.h"
#include "nel/misc/file.h"

class TileBankLoaderPvt
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

	void loadLands( QList< Land > &lands )
	{
		lands.clear();

		Land l;

		int c = bank.getLandCount();
		for( int i = 0; i < c; i++ )
		{
			NL3D::CTileLand *land = bank.getLand( i );
			l.name = land->getName().c_str();


			std::set< std::string > sets = land->getTileSets();
			std::set< std::string >::const_iterator itr = sets.begin();
			while( itr != sets.end() )
			{
				l.tilesets.push_back( itr->c_str() );
				++itr;
			}

			lands.push_back( l );
		}
	}

	void loadTiles128( NL3D::CTileSet *set, TileTypeNode *node )
	{
		int c = set->getNumTile128();
		for( int i = 0; i < c; i++ )
		{
			int idx = set->getTile128( i );
			NL3D::CTile *tile = bank.getTile( idx );

			TileItemNode *tin = new TileItemNode( i, TileModel::TileDiffuse, "" );

			for( int i = TileModel::TileDiffuse; i <= TileModel::TileAlpha; i++ )
			{
				tin->setTileFilename( TileModel::TTileChannel( i ), tile->getRelativeFileName( channelToTBitmap( TileModel::TTileChannel( i ) ) ).c_str() );
			}

			node->appendChild( tin );
		}
	}

	void loadTiles256( NL3D::CTileSet *set, TileTypeNode *node )
	{
		int c = set->getNumTile256();
		for( int i = 0; i < c; i++ )
		{
			int idx = set->getTile256( i );
			NL3D::CTile *tile = bank.getTile( idx );

			TileItemNode *tin = new TileItemNode( i, TileModel::TileDiffuse, "" );

			for( int i = TileModel::TileDiffuse; i <= TileModel::TileAlpha; i++ )
			{
				tin->setTileFilename( TileModel::TTileChannel( i ), tile->getRelativeFileName( channelToTBitmap( TileModel::TTileChannel( i ) ) ).c_str() );
			}

			node->appendChild( tin );
		}
	}

	void loadTilesTransition( NL3D::CTileSet *set, TileTypeNode *node )
	{
		for( int i = 0; i < NL3D::CTileSet::count; i++ )
		{
			const NL3D::CTileSetTransition *tr = set->getTransition( i );
			int idx = tr->getTile();
			NL3D::CTile *tile = bank.getTile( idx );

			TileItemNode *tin = static_cast< TileItemNode* >( node->child( i ) );

			for( int j = TileModel::TileDiffuse; j <= TileModel::TileAlpha; j++ )
			{
				tin->setTileFilename( TileModel::TTileChannel( i ), tile->getRelativeFileName( channelToTBitmap( TileModel::TTileChannel( i ) ) ).c_str() );
			}
		}
	}

	void loadTilesDisplacement( NL3D::CTileSet *set, TileTypeNode *node )
	{
		for( int i = 0; i < NL3D::CTileSet::CountDisplace; i++ )
		{
			uint did = set->getDisplacementTile( NL3D::CTileSet::TDisplacement( i ) );
			const char *fn = bank.getDisplacementMap( did );

			TileItemNode *tin = static_cast< TileItemNode* >( node->child( i ) );
			tin->setTileFilename( TileModel::TileDiffuse, fn );
		}
	}

	void loadTileSet( NL3D::CTileSet *set, TileSetNode *node )
	{
		loadTiles128( set, static_cast< TileTypeNode* >( node->child( 0 ) ) );
		loadTiles256( set, static_cast< TileTypeNode* >( node->child( 1 ) ) );
		loadTilesTransition( set, static_cast< TileTypeNode* >( node->child( 2 ) ) );
		loadTilesDisplacement( set, static_cast< TileTypeNode* >( node->child( 3 ) ) );

		node->setOriented( set->getOriented() );
	}

	void loadTileSets( TileModel *model )
	{
		model->clear();

		int c = bank.getTileSetCount();
		for( int i = 0; i < c; i++ )
		{
			NL3D::CTileSet *set = bank.getTileSet( i );
			TileSetNode *node = model->createTileSetNode( set->getName().c_str() );
			loadTileSet( set, node );
		}
	}
};


TileBankLoader::TileBankLoader()
{
	p = new TileBankLoaderPvt;
}

TileBankLoader::~TileBankLoader()
{
	delete p;
	p = NULL;
}

bool TileBankLoader::load( const char *filename, TileModel *model, QList< Land > &lands )
{
	NLMISC::CIFile file;
	if( !file.open( filename, false ) )
		return false;

	p->bank.serial( file );

	p->loadLands( lands );
	p->loadTileSets( model );

	return false;
}
