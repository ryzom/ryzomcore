#include "texture_chooser.h"
#include "nel/misc/path.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/file.h"
#include <vector>
#include <string>
#include <QPixMap>
#include <QImage>
#include <QListWidget>

#include "nel/gui/view_renderer.h"

struct TextureChooserPrivate
{
	QListWidget *fileTextures;
	QListWidget *atlasTextures;

	TextureChooserPrivate()
	{
		fileTextures = new QListWidget();
		atlasTextures = new QListWidget();
	}
};

TextureChooser::TextureChooser( QDialog *parent ) : 
QDialog( parent )
{
	setupUi( this );

	d_ptr = new TextureChooserPrivate;
	this->tabWidget->clear();
	this->tabWidget->addTab( d_ptr->fileTextures, tr( "File textures" ) );
	this->tabWidget->addTab( d_ptr->atlasTextures, tr( "Atlas texture" ) );

	setupConnections();
}

TextureChooser::~TextureChooser()
{
	delete d_ptr;
	d_ptr = NULL;
}


void TextureChooser::load()
{
	// Load the file textures
	d_ptr->fileTextures->clear();

	std::vector< std::string > textures;
	//NLMISC::CPath::getFileList( "tga", textures );
	NLMISC::CPath::getFileListByPath( "dds", "interfaces", textures );
	NLMISC::CPath::getFileListByPath( "dds", "gamedev", textures );

	std::sort( textures.begin(), textures.end() );

	std::vector< std::string >::const_iterator itr = textures.begin();
	while( itr != textures.end() )
	{
		d_ptr->fileTextures->addItem( itr->c_str() );
		++itr;
	}

	// Now load the atlas textures
	d_ptr->atlasTextures->clear();
	textures.clear();

	NLGUI::CViewRenderer::getInstance()->getTextureNames( textures );
	itr = textures.begin();
	while( itr != textures.end() )
	{
		d_ptr->atlasTextures->addItem( itr->c_str() );
		++itr;
	}

	// set the file textures row after the atlas, because they are shown first
	d_ptr->atlasTextures->setCurrentRow( 0 );
	d_ptr->fileTextures->setCurrentRow( 0 );
}

void TextureChooser::accept()
{
	QListWidgetItem *item = d_ptr->fileTextures->currentItem();
	if( item == NULL )
		return;

	selection = item->text();
	QDialog::accept();
}

void TextureChooser::reject()
{
	selection = "";

	QDialog::reject();
}

void TextureChooser::onFileTxtRowChanged( int row )
{
	if( row < 0 )
		return;

	QListWidgetItem *item = d_ptr->fileTextures->item( row );
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
	b = bm.convertToType( NLMISC::CBitmap::RGBA );
	if( !b )
	{
		return;
	}

	setPreviewImage( bm );
}

void TextureChooser::onAtlasTxtRowChanged( int row )
{
	if( row < 0 )
		return;

	QListWidgetItem *item = d_ptr->atlasTextures->item( row );
	QString fn = item->text();

	std::string rfn = fn.toUtf8().constData();

	NLMISC::CBitmap bm;

	bool b = NLGUI::CViewRenderer::getInstance()->getTexture( bm, rfn );
	if( !b )
		return;
	
	setPreviewImage( bm );
}


void TextureChooser::setupConnections()
{
	connect( d_ptr->fileTextures, SIGNAL( currentRowChanged( int ) ), this, SLOT( onFileTxtRowChanged( int ) ) );
	connect( d_ptr->atlasTextures, SIGNAL( currentRowChanged( int ) ), this, SLOT( onAtlasTxtRowChanged( int ) ) );
}

void TextureChooser::setPreviewImage( NLMISC::CBitmap &bm )
{
	// should be depth, but CBitmap always uses 32 bit to store the image
	uint32 size = bm.getSize() * ( 32 / 8 );
	uint8 *data = new uint8[ size ];
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

	delete data;
	data = NULL;
}



