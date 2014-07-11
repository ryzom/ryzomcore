// Ryzom Core Studio World Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "const_string_array_editor.h"

ConstStrArrEditDialog::ConstStrArrEditDialog( QDialog *parent ) :
QDialog( parent )
{
	setupUi( this );
	setupConnections();
}

ConstStrArrEditDialog::~ConstStrArrEditDialog()
{
}

void ConstStrArrEditDialog::setStrings( const QStringList &strings )
{
	cb->clear();

	QStringListIterator itr( strings );
	while( itr.hasNext() )
	{
		cb->addItem( itr.next() );
	}
	
	cb->setCurrentIndex( 0 );
}

void ConstStrArrEditDialog::setValue( const QString &value )
{
	listWidget->clear();

	if( value.isEmpty() )
		return;

	QStringList l = value.split( ';' );
	
	QStringListIterator itr( l );
	while( itr.hasNext() )
	{
		listWidget->addItem( itr.next() );
	}
}

QString ConstStrArrEditDialog::getValue() const
{
	QString value;

	for( int i = 0; i < listWidget->count(); i++ )
	{
		QListWidgetItem *item = listWidget->item( i );
		value += item->text();

		if( i < ( listWidget->count() - 1 ) )
			value += ';';
	}

	return value;
}

void ConstStrArrEditDialog::accept()
{
	QDialog::accept();
}

void ConstStrArrEditDialog::reject()
{
	QDialog::reject();
}

void ConstStrArrEditDialog::onAddClicked()
{
	listWidget->addItem( cb->currentText() );
}

void ConstStrArrEditDialog::onRemoveClicked()
{
	QListWidgetItem *item = listWidget->currentItem();
	if( item == NULL )
		return;

	delete item;
}

void ConstStrArrEditDialog::setupConnections()
{
	connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
}


