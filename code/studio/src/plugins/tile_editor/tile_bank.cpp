#include "tile_bank.h"
#include "nel/3d/tile_bank.h"

#include <QPixmap>

namespace{

	bool pixmapToCBGRA( QPixmap &pixmap, std::vector< NLMISC::CBGRA >& pixels )
	{
		QImage img = pixmap.toImage();
		if( img.format() != QImage::Format_ARGB32 )
			img = img.convertToFormat( QImage::Format_ARGB32 );

		if( img.format() != QImage::Format_ARGB32 )
			return false;

		int c = img.width() * img.height();

		const unsigned char *data = img.bits();
		const unsigned int *idata = reinterpret_cast< const unsigned int* >( data );

		NLMISC::CBGRA bgra;
		pixels.clear();

		int i = 0;
		while( i < c )
		{
			bgra.A = ( idata[ i ] & 0xFF000000 ) >> 24;
			bgra.R = ( idata[ i ] & 0x00FF0000 ) >> 16;
			bgra.G = ( idata[ i ] & 0x0000FF00 ) >> 8;
			bgra.B = ( idata[ i ] & 0x000000FF );
			pixels.push_back( bgra );

			i++;
		}

		return true;
	}


	NL3D::CTile::TBitmap channelToTBitmap( TileConstants::TTileChannel channel )
	{
		return NL3D::CTile::TBitmap( int( channel ) );
	}


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class TileBankPvt
{
public:

	bool checkSize( TileConstants::TNodeTileType type, TileConstants::TTileChannel channel, int w, int h )
	{
		int width = -1;

		switch( type )
		{
		case TileConstants::Tile128:
			width = 128;
			break;

		case TileConstants::Tile256:
			width = 256;
			break;
		
		case TileConstants::TileTransition:
			{
				if( channel == TileConstants::TileAlpha )
					width = 64;
				else
					width = 128;

			}
			break;

		case TileConstants::TileDisplacement:
			width = 32;
			break;
		}

		if( width == w )
			return true;
		
		return false;
	}

	NL3D::CTileSet::TError checkTile( NL3D::CTileSet *set, int tile, TileConstants::TNodeTileType type, NL3D::CTileBorder &border, NL3D::CTile::TBitmap bitmap, QString &msg )
	{
		NL3D::CTileSet::TError error;

		if( bitmap == NL3D::CTile::additive )
			return NL3D::CTileSet::ok;

		if( type == TileConstants::TileDisplacement )
			return NL3D::CTileSet::ok;

		int component;
		int pixel;
		int idx;
		
		switch( type )
		{
		case TileConstants::Tile128:
			error = set->checkTile128( bitmap, border, pixel, component );
			break;

		case TileConstants::Tile256:
			error = set->checkTile256( bitmap, border, pixel, component );
			break;

		case TileConstants::TileTransition:
			{
				if( bitmap != NL3D::CTile::alpha )
					error = set->checkTile128( bitmap, border, pixel, component );
				else
					error = set->checkTileTransition( NL3D::CTileSet::TTransition( tile ), bitmap, border, idx, pixel, component );

				break;
			}
		}

		if( ( error != NL3D::CTileSet::ok ) && ( error != NL3D::CTileSet::addFirstA128128 ) )
		{
			static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};
			
			msg = NL3D::CTileSet::getErrorMessage( error );
			msg += "\n";
			msg += " pixel %1 component %2";
			msg = msg.arg( pixel );
			msg = msg.arg( comp[ component ] );
		}

		return error;
	}

	void setTile( NL3D::CTileSet *set, int tile, int rotation, const QString &name, NL3D::CTile::TBitmap bm, TileConstants::TNodeTileType type, NL3D::CTileBorder &border )
	{
		switch( type )
		{
		case TileConstants::Tile128:
			set->setTile128( tile, name.toUtf8().constData(), bm, m_bank );
			break;

		case TileConstants::Tile256:
			set->setTile256( tile, name.toUtf8().constData(), bm, m_bank );
			break;

		case TileConstants::TileTransition:
			if( bm != NL3D::CTile::alpha )
				set->setTileTransition( NL3D::CTileSet::TTransition( tile ), name.toUtf8().constData(), bm, m_bank, border );
			else
				set->setTileTransitionAlpha( NL3D::CTileSet::TTransition( tile ), name.toUtf8().constData(), m_bank, border, rotation );
			break;

		case TileConstants::TileDisplacement:
			set->setDisplacement( NL3D::CTileSet::TDisplacement( tile ), name.toUtf8().constData(), m_bank );
			break;
		}

	}

	void buildBorder( QPixmap &pm, NL3D::CTileBorder &border )
	{
		std::vector< NLMISC::CBGRA > pixels;
		pixmapToCBGRA( pm, pixels );
		border.set( pm.width(), pm.height(), pixels );
	}

	NL3D::CTileBank m_bank;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TileBank::TileBank()
{
	m_pvt = new TileBankPvt();
	resetError();
}

TileBank::~TileBank()
{
	delete m_pvt;
}

void TileBank::addTileSet( const QString &name )
{
	m_pvt->m_bank.addTileSet( name.toUtf8().constData() );
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( 0 );
}

void TileBank::removeTileSet( int idx )
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( idx );
	if( set == NULL )
		return;

	int c = m_pvt->m_bank.getLandCount();
	for( int i = 0; i < c; i++ )
	{
		NL3D::CTileLand *land = m_pvt->m_bank.getLand( i );
		land->removeTileSet( set->getName() );
	}

	m_pvt->m_bank.removeTileSet( idx );
}

void TileBank::renameTileSet( int idx, const QString &newName )
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( idx );
	if( set == NULL )
		return;

	std::string oldName = set->getName();
	set->setName( newName.toUtf8().constData() );

	int c = m_pvt->m_bank.getLandCount();
	for( int i = 0; i < c; i++ )
	{
		NL3D::CTileLand *land = m_pvt->m_bank.getLand( i );
		land->removeTileSet( oldName );
		land->addTileSet( newName.toUtf8().constData() );
	}

}

void TileBank::addLand( const QString &name )
{
	m_pvt->m_bank.addLand( name.toUtf8().constData() );
}

void TileBank::removeLand( int idx )
{
	m_pvt->m_bank.removeLand( idx );
}

void TileBank::setLandSets( int idx, const QStringList &l )
{
	NL3D::CTileLand *land = m_pvt->m_bank.getLand( idx );
	land->clear();

	QStringListIterator itr( l );
	while( itr.hasNext() )
	{
		land->addTileSet( itr.next().toUtf8().constData() );
	}
}

void TileBank::getLandSets( int idx, QStringList &l )
{
	NL3D::CTileLand *land = m_pvt->m_bank.getLand( idx );
	if( land == NULL )
		return;

	l.clear();

	std::set< std::string> sets = land->getTileSets();
	std::set< std::string >::const_iterator itr = sets.begin();
	while( itr != sets.end() )
	{
		l.push_back( itr->c_str() );
		++itr;
	}
}

bool TileBank::addTile( int setIdx, const QString &name, const QVariant &pixmap, TileConstants::TTileChannel channel, TileConstants::TNodeTileType type )
{
	resetError();

	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( setIdx );

	int tile;
	switch( type )
	{
	case TileConstants::Tile128: set->addTile128( tile, m_pvt->m_bank ); break;
	case TileConstants::Tile256: set->addTile256( tile, m_pvt->m_bank ); break;
	}

	bool b = setTile( setIdx, tile, name, pixmap, channel, type );
	if( b )
		return true;
	
	// There was an error, roll back
	switch( type )
	{
	case TileConstants::Tile128: set->removeTile128( tile, m_pvt->m_bank ); break;
	case TileConstants::Tile256: set->removeTile128( tile, m_pvt->m_bank ); break;
	}

	return false;
}

void TileBank::removeTile( int ts, int type, int tile )
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( ts );
	
	switch( type )
	{
	case TileConstants::Tile128: set->removeTile128( tile, m_pvt->m_bank ); break;
	case TileConstants::Tile256: set->removeTile256( tile, m_pvt->m_bank ); break;
	}
}

bool TileBank::setTile( int tileset, int tile, const QString &name, const QVariant &pixmap, TileConstants::TTileChannel channel, TileConstants::TNodeTileType type )
{
	resetError();
	
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( tileset );
	if( set == NULL )
		return false;

	QPixmap pm = pixmap.value< QPixmap >();
	if( pm.isNull() )
	{
		m_hasError = true;
		m_lastError = "Failed to load image.";
		return false;
	}

	if( pm.width() != pm.height() )
	{
		m_hasError = true;
		m_lastError = "Image isn't square.";
		return false;
	}

	if( !m_pvt->checkSize( type, channel, pm.width(), pm.height() ) )
	{
		m_hasError = true;
		m_lastError = "Invalid image size.";
		return false;
	}

	NL3D::CTileBorder border;
	m_pvt->buildBorder( pm, border );

	QString msg;
	NL3D::CTileSet::TError error = m_pvt->checkTile( set, tile, type, border, channelToTBitmap( channel ), msg );

	// Tile checks out fine, set it
	if( ( error == NL3D::CTileSet::ok ) || ( error == NL3D::CTileSet::addFirstA128128 ) )
	{
		if( error == NL3D::CTileSet::addFirstA128128 )
			set->setBorder( channelToTBitmap( channel ), border );

		m_pvt->setTile( set, tile, 0 /* rotation */, name, channelToTBitmap( channel ), type, border );

		return true;
	}

	setError( msg );

	return false;
}

void TileBank::replaceImage( int ts, int type, int tile, TileConstants::TTileChannel channel, const QString &name, const QVariant &pixmap )
{
	setTile( ts, tile, name, pixmap, channel, TileConstants::TNodeTileType( type ) );
}

void TileBank::clearImage( int ts, int type, int tile, TileConstants::TTileChannel channel )
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( ts );

	int tileId;

	NL3D::CTile::TBitmap bm = channelToTBitmap( channel );
	
	switch( type )
	{
	case TileConstants::Tile128:
		set->clearTile128( tile, bm, m_pvt->m_bank );
		break;

	case TileConstants::Tile256:
		set->clearTile256( tile, bm, m_pvt->m_bank );
		break;	

	case TileConstants::TileTransition:
		set->clearTransition( NL3D::CTileSet::TTransition( tile ), bm, m_pvt->m_bank );
		break;

	case TileConstants::TileDisplacement:
		set->clearDisplacement( NL3D::CTileSet::TDisplacement( tile ), m_pvt->m_bank );
		break;
	}
	
}


void TileBank::setVegetation( int tileSet, const QString &vegetation )
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( tileSet );
	if( set == NULL )
		return;

	set->setTileVegetableDescFileName( vegetation.toUtf8().constData() );
}


QString TileBank::getVegetation( int tileSet ) const
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( tileSet );
	if( set == NULL )
		return "";

	return set->getTileVegetableDescFileName().c_str();
}

void TileBank::setOriented( int tileSet, bool b )
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( tileSet );
	if( set == NULL )
		return;

	set->setOriented( b );
}

bool TileBank::getOriented( int tileSet ) const
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( tileSet );
	if( set == NULL )
		return false;

	return set->getOriented();
}


void TileBank::setSurfaceData( int tileSet, unsigned long data )
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( tileSet );
	if( set == NULL )
		return;

	set->SurfaceData = data;
}

unsigned long TileBank::getSurfaceData( int tileSet ) const
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( tileSet );
	if( set == NULL )
		return 0;

	return set->SurfaceData;
}


