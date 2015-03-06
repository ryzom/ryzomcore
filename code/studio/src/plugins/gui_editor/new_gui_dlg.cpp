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

#include "new_gui_dlg.h"
#include <QMessageBox>
#include <QFileDialog>

NewGUIDlg::NewGUIDlg( QWidget *parent ) :
QDialog( parent )
{
	m_ui.setupUi( this );

	// Login texture map - temporaty measure until we add default textures for widgets to use
	m_ui.mapList->addItem( "texture_interfaces_v3_login.tga" );

	connect( m_ui.okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );
	connect( m_ui.cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
	connect( m_ui.projectDirTB, SIGNAL( clicked( bool ) ), this, SLOT( onProjectDirTBClicked() ) );
	connect( m_ui.addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	connect( m_ui.removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );

}

NewGUIDlg::~NewGUIDlg()
{
}

QString NewGUIDlg::getProjectName() const
{
	return m_ui.projectEdit->text();
}

QString NewGUIDlg::getWindowName() const
{
	return m_ui.windowEdit->text();
}

QString NewGUIDlg::getProjectDirectory() const
{
	return m_ui.projectDirEdit->text();
}

void NewGUIDlg::getMapList( QList< QString > &l )
{
	l.clear();

	for( int i = 0; i < m_ui.mapList->count(); i++ )
	{
		l.push_back( m_ui.mapList->item( i )->text() );
	}
}

void NewGUIDlg::onOKClicked()
{
	if( m_ui.projectEdit->text().isEmpty() )
	{
		QMessageBox::information( this,
									tr( "New project" ),
									tr( "You must specify a project name!" ) );
		return;
	}

	if( m_ui.windowEdit->text().isEmpty() )
	{
		QMessageBox::information( this,
									tr( "New project" ),
									tr( "You must specify a window name!" ) );
		return;
	}

	if( m_ui.projectDirEdit->text().isEmpty() )
	{
		QMessageBox::information( this,
									tr( "New project" ),
									tr( "You must specify a project directory!" ) );
		return;
	}

	accept();
}

void NewGUIDlg::onCancelClicked()
{
	reject();
}

void NewGUIDlg::onProjectDirTBClicked()
{
	QString dir = QFileDialog::getExistingDirectory( this,
													tr( "Specify project directory" ),
													"." );
	if( dir.isEmpty() )
		return;

	m_ui.projectDirEdit->setText( dir );
}

void NewGUIDlg::onAddClicked()
{
	if( m_ui.mapEdit->text().isEmpty() )
		return;

	QList< QListWidgetItem* > l = m_ui.mapList->findItems( m_ui.mapEdit->text(), Qt::MatchContains );
	if( !l.isEmpty() )
	{
		return;
	}

	m_ui.mapList->addItem( m_ui.mapEdit->text() );
	m_ui.mapEdit->clear();
}

void NewGUIDlg::onRemoveClicked()
{
	QListWidgetItem *item = m_ui.mapList->currentItem();
	if( item == NULL )
		return;

	delete item;
}



