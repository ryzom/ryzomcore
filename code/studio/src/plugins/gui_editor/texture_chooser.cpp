#include "texture_chooser.h"
#include "nel/misc/path.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/file.h"
#include <vector>
#include <string>
#include <QPixMap>
#include <QImage>

TextureChooser::TextureChooser( QDialog *parent ) : 
QDialog( parent )
{
	setupUi( this );
	setupConnections();
	data = NULL;
}

TextureChooser::~TextureChooser()
{
	delete data;
	data = NULL;
}


void TextureChooser::load()
{
	listWidget->clear();

	std::vector< std::string > textures;
	NLMISC::CPath::getFileList( "tga", textures );

	std::vector< std::string >::const_iterator itr = textures.begin();
	while( itr != textures.end() )
	{
		listWidget->addItem( itr->c_str() );
		++itr;
	}
}

void TextureChooser::onCurrentRowChanged( int row )
{
	if( row < 0 )
		return;

	QListWidgetItem *item = listWidget->item( row );
	QString fn = item->text();

	std::string rfn = fn.toUtf8().constData();
	rfn = NLMISC::CPath::lookup( rfn );
	
	NLMISC::CIFile f;
	bool b = f.open( rfn );
	if( !b )
	{
		return;
	}

	NLMISC::CBitmap bm;
	uint8 depth = bm.load( f );
	f.close();

	uint32 size = bm.getSize() * ( 32 / 8 );  // should be depth, but CBitmap always uses 32 bit to store the image

	if( data != NULL )
		delete data;

	data = new uint8[ size ];
	bm.getData( data );

	/// Convert from ABGR to ARGB
	{
		int i = 0;
		while( i < size )
		{
			uint8 t = 0;

			/// ABGR
			t = data[ i ];
			data[ i ] = data[ i + 2 ];
			data[ i + 2 ] = t;

			i += 4;
		}
	}

	QImage img( data, bm.getWidth(), bm.getHeight(), QImage::Format_ARGB32 );
	label->setPixmap( QPixmap::fromImage( img ) );

}


void TextureChooser::setupConnections()
{
	connect( listWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( onCurrentRowChanged( int ) ) );
}



