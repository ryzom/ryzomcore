#include "land_edit_dialog.h"

LandEditDialog::LandEditDialog( QWidget *parent ) :
QDialog( parent )
{
	setupUi( this );
	setupConnections();
}

LandEditDialog::~LandEditDialog()
{
}

void LandEditDialog::getSelectedTileSets( QStringList &l ) const
{
	int c = tilesetLV->count();
	for( int i = 0; i < c; i++ )
	{
		l.push_back( tilesetLV->item( i )->text() );
	}
}

void LandEditDialog::setTileSets( const QStringList &l )
{
	tilesetCB->clear();

	QStringListIterator itr( l );
	while( itr.hasNext() )
	{
		tilesetCB->addItem( itr.next() );
	}
}

void LandEditDialog::setupConnections()
{
	connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOkClicked() ) );
	connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
	connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
}

void LandEditDialog::onOkClicked()
{
	accept();
}

void LandEditDialog::onCancelClicked()
{
	reject();
}

void LandEditDialog::onAddClicked()
{
	if( tilesetCB->currentIndex() < 0 )
		return;

	QString text = tilesetCB->currentText();

	int c = tilesetLV->count();
	for( int i = 0; i < c; i++ )
	{
		if( text == tilesetLV->item( i )->text() )
			return;
	}

	tilesetLV->addItem( text );
}

void LandEditDialog::onRemoveClicked()
{
	if( tilesetLV->currentItem() == NULL )
		return;

	QListWidgetItem *item = tilesetLV->currentItem();
	delete item;
}

