// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "project_window.h"
#include <QInputDialog>
#include <QMessageBox>

namespace GUIEditor
{
	ProjectWindow::ProjectWindow( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		connect( okButton, SIGNAL( clicked(bool) ), this, SLOT( hide() ) );
		connect( cancelButton, SIGNAL( clicked(bool) ), this, SLOT( hide() ) );

		connect( addButton, SIGNAL( clicked(bool) ), this, SLOT( onAddButtonClicked() ) );
		connect( removeButton, SIGNAL( clicked(bool) ), this, SLOT( onRemoveButtonClicked() ) );
	}

	ProjectWindow::~ProjectWindow()
	{
	}

	void ProjectWindow::setupFileList( const std::vector< std::string > &fileNames )
	{
		fileList->clear();

		std::vector< std::string >::const_iterator itr;
		for( itr = fileNames.begin(); itr != fileNames.end(); ++itr )
		{
			const std::string &s = *itr;
			fileList->addItem( s.c_str() );
		}
		fileList->sortItems();
	}

	void ProjectWindow::onAddButtonClicked()
	{
		bool ok;
		QString newFile = QInputDialog::getText( this,
												tr( "Adding file to project" ),
												tr( "Which file do you want to add?" ),
												QLineEdit::Normal,
												QString(),
												&ok );

		if( ok )
		{
			fileList->addItem( newFile );
			fileList->sortItems();
		}
	}

	void ProjectWindow::onRemoveButtonClicked()
	{
		if( fileList->count() == 0 )
			return;

		QMessageBox::StandardButton reply;
		QString text;
		if( fileList->currentRow() >= 0 )
			text = fileList->item( fileList->currentRow() )->text();

		reply = QMessageBox::question( this,
									tr( "Removing file from project" ),
									tr( "Are you sure you want to remove '%1' from the project?" ).arg( text ),
									QMessageBox::Yes | QMessageBox::Cancel );
		
		QListWidgetItem *item;
		if( reply == QMessageBox::Yes )
			item = fileList->takeItem( fileList->currentRow() );
		delete item;
	}
}


