// Ryzom Core Studio - GUI Editor Plugin
//
// Copyright (C) 2014 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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


#include "expr_link_dlg.h"
#include <QMessageBox>

ExprLinkDlg::ExprLinkDlg( QWidget *parent ) :
QDialog( parent )
{
	m_ui.setupUi( this );

	connect( m_ui.okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );
	connect( m_ui.cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
}

ExprLinkDlg::~ExprLinkDlg()
{
}

void ExprLinkDlg::load( const QList< SlotInfo > &a, const QList< SlotInfo > &b, const QString &aname, const QString &bname )
{
	QListIterator< SlotInfo > itra( a );
	QListIterator< SlotInfo > itrb( b );

	while( itra.hasNext() )
	{
		const SlotInfo &info = itra.next();

		QListWidgetItem *item = new QListWidgetItem();
		item->setText( info.name );
		item->setData( Qt::UserRole, info.slot );

		m_ui.list1->addItem( item );
	}

	while( itrb.hasNext() )
	{
		const SlotInfo &info = itrb.next();

		QListWidgetItem *item = new QListWidgetItem();
		item->setText( info.name );
		item->setData( Qt::UserRole, info.slot );

		m_ui.list2->addItem( item );
	}

	m_ui.groupBox1->setTitle( aname );
	m_ui.groupBox2->setTitle( bname );
}

int ExprLinkDlg::getSlotA() const
{
	QListWidgetItem *item = m_ui.list1->currentItem();
	if( item == NULL )
		return -1;

	int slot = item->data( Qt::UserRole ).toInt();
	return slot;
}

int ExprLinkDlg::getSlotB() const
{
	QListWidgetItem *item = m_ui.list2->currentItem();
	if( item == NULL )
		return -1;

	int slot = item->data( Qt::UserRole ).toInt();
	return slot;
}

void ExprLinkDlg::onOKClicked()
{
	int slotA = getSlotA();
	int slotB = getSlotB();

	if( ( slotA == -1 ) || ( slotB == -1 ) )
	{
		QMessageBox::information( this,
									tr( "No slots selected" ),
									tr( "You need to select a slot on both sides." ) );
		return;
	}

	if( ( slotA == 0 ) && ( slotB == 0 ) )
	{
		QMessageBox::information( this,
									tr( "Wrong slots selected" ),
									tr( "You can only select the 'Out' slot on one of the sides." ) );
		return;
	}

	if( ( slotA != 0 ) && ( slotB != 0 ) )
	{
		QMessageBox::information( this,
									tr( "Wrong slots selected" ),
									tr( "One of the slots selected must be the 'Out' slot!" ) );
		return;
	}

	accept();
}

void ExprLinkDlg::onCancelClicked()
{
	reject();
}


