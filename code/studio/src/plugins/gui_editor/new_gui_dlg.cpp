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

NewGUIDlg::NewGUIDlg( QWidget *parent ) :
QDialog( parent )
{
	m_ui.setupUi( this );

	connect( m_ui.okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );
	connect( m_ui.cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );

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

	accept();
}

void NewGUIDlg::onCancelClicked()
{
	reject();
}


