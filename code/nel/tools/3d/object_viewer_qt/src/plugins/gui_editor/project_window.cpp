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
		filesChanged = false;

		connect( okButton, SIGNAL( clicked(bool) ), this, SLOT( onOKButtonClicked() ) );
		connect( cancelButton, SIGNAL( clicked(bool) ), this, SLOT( hide() ) );

		connect( addButton, SIGNAL( clicked(bool) ), this, SLOT( onAddButtonClicked() ) );
		connect( removeButton, SIGNAL( clicked(bool) ), this, SLOT( onRemoveButtonClicked() ) );
	}

	ProjectWindow::~ProjectWindow()
	{
	}

	void ProjectWindow::setupFiles( SProjectFiles &projectFiles )
	{
		QTreeWidgetItem *topItem = fileTree->topLevelItem( 0 );
		if( topItem != NULL )
		{
			QList< QTreeWidgetItem* > childList = topItem->takeChildren();
			QListIterator< QTreeWidgetItem* > it( childList );
			while( it.hasNext() )
				delete it.next();
			childList.clear();

			std::vector< std::string >::iterator itr;
			for( itr = projectFiles.guiFiles.begin(); itr != projectFiles.guiFiles.end(); ++itr )
			{
				QTreeWidgetItem *item = new QTreeWidgetItem( topItem );
				item->setText( 0, itr->c_str() );
			}
		}

		topItem = fileTree->topLevelItem( 1 );
		if( topItem != NULL )
		{
			QList< QTreeWidgetItem* > childList = topItem->takeChildren();
			QListIterator< QTreeWidgetItem* > it( childList );
			while( it.hasNext() )
				delete it.next();
			childList.clear();

			std::vector< std::string >::iterator itr;
			for( itr = projectFiles.mapFiles.begin(); itr != projectFiles.mapFiles.end(); ++itr )
			{
				QTreeWidgetItem *item = new QTreeWidgetItem( topItem );
				item->setText( 0, itr->c_str() );
			}
		}
	}

	void ProjectWindow::updateFiles( SProjectFiles &projectFiles )
	{
		projectFiles.clearFiles();
		
		QTreeWidgetItem *topItem;
		
		topItem = fileTree->topLevelItem( 0 );
		if( topItem != NULL )
		{
			int c = topItem->childCount();
			for( int i = 0; i < c; i++ )
			{
				QTreeWidgetItem *item = topItem->child( i );
				projectFiles.guiFiles.push_back( item->text( 0 ).toUtf8().constData() );
			}
		}

		topItem = fileTree->topLevelItem( 1 );
		if( topItem != NULL )
		{
			int c = topItem->childCount();
			for( int i = 0; i < c; i++ )
			{
				QTreeWidgetItem *item = topItem->child( i );
				projectFiles.mapFiles.push_back( item->text( 0 ).toUtf8().constData() );
			}
		}
	}

	void ProjectWindow::clear()
	{
		fileTree->clear();
	}

	void ProjectWindow::onAddButtonClicked()
	{
		if( fileTree->currentItem() == NULL )
			return;

		QTreeWidgetItem *item = fileTree->currentItem();
		QTreeWidgetItem *parent = item->parent();

		while( parent != NULL )
		{
			item = parent;
			parent = parent->parent();
		}

		bool ok;
		QString newFile = QInputDialog::getText( this,
												tr( "Adding file to project" ),
												tr( "Which file do you want to add?" ),
												QLineEdit::Normal,
												QString(),
												&ok );

		if( ok )
		{
			QTreeWidgetItem *newItem = new QTreeWidgetItem( item );
			newItem->setText( 0, newFile );
			filesChanged = true;
		}
	}

	void ProjectWindow::onRemoveButtonClicked()
	{
		if( fileTree->currentItem() == NULL )
			return;

		// Can't delete top-level item
		if( fileTree->currentItem()->parent() == NULL )
			return;

		QMessageBox::StandardButton reply;
		QString text = fileTree->currentItem()->text( 0 );

		reply = QMessageBox::question( this,
									tr( "Removing file from project" ),
									tr( "Are you sure you want to remove '%1' from the project?" ).arg( text ),
									QMessageBox::Yes | QMessageBox::Cancel );
		
		if( reply == QMessageBox::Yes )
		{
			fileTree->currentItem()->parent()->removeChild( fileTree->currentItem() );
			filesChanged = true;
		}

	}

	void ProjectWindow::onOKButtonClicked()
	{
		hide();

		if( filesChanged )
		{
			filesChanged = false;
			Q_EMIT projectFilesChanged();
		}
	}
}


