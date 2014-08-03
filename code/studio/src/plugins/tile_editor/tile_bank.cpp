#include "tile_bank.h"
#include "nel/3d/tile_bank.h"

#include <QPixmap>

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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class TileBankPvt
{
public:
	NL3D::CTileBank m_bank;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TileBank::TileBank()
{
	m_pvt = new TileBankPvt();
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

void TileBank::addLand( const QString &name )
{
	m_pvt->m_bank.addLand( name.toUtf8().constData() );
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

bool TileBank::addTileToSet( int idx, const QString &name, const QVariant &pixmap, TileConstants::TTileChannel channel, TileConstants::TNodeTileType type )
{
	NL3D::CTileSet *set = m_pvt->m_bank.getTileSet( idx );
	if( set == NULL )
		return false;

	QPixmap pm = pixmap.value< QPixmap >();
	if( pm.isNull() )
		return false;

	if( pm.width() != pm.height() )
		return false;

	std::vector< NLMISC::CBGRA > pixels;
	pixmapToCBGRA( pm, pixels );

	int tile;
	set->addTile128( tile, m_pvt->m_bank );

	NL3D::CTileBorder border;
	border.set( pm.width(), pm.height(), pixels );

	

	return true;
}