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

void LandEditDialog::setSelectedTileSets( QStringList &l )
{
	tilesetLV->clear();

	QStringListIterator itr( l );
	while( itr.hasNext() )
	{
		tilesetLV->addItem( itr.next() );
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

