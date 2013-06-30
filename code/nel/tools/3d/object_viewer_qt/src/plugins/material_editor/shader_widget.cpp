// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#include "shader_widget.h"
#include "shader_editor.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

namespace MaterialEditor
{
	ShaderWidget::ShaderWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setupConnections();

		shaderEditorWidget = new ShaderEditorWidget();
	}

	ShaderWidget::~ShaderWidget()
	{
		delete shaderEditorWidget;
		shaderEditorWidget = NULL;
	}

	void ShaderWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );

		connect( newButton, SIGNAL( clicked( bool ) ), this, SLOT( onNewClicked() ) );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditClicked() ) );
	}

	void ShaderWidget::onOKClicked()
	{
		close();
	}

	bool ShaderWidget::nameExists( const QString &name )
	{
		QTreeWidgetItem *item = NULL;
		for( int i = 0; i < shaderListWidget->topLevelItemCount(); i++ )
		{
			item = shaderListWidget->topLevelItem( i );
			if( item->text( 0 ) == name )
				return true;
		}

		return false;
	}

	void ShaderWidget::nameExistsMessage()
	{
		QMessageBox::critical(
			this,
			tr( "Shader already exists" ),
			tr( "A shader with that name already exists!" ),
			QMessageBox::Ok
			);
	}

	void ShaderWidget::onNewClicked()
	{
		bool ok = false;
		QString def;
		QString name =
			QInputDialog::getText( 
				this,
				tr( "Shader name" ),
				tr( "New shader's name?" ),
				QLineEdit::Normal,
				def,
				&ok
				);

		if( nameExists( name ) )
		{
			nameExistsMessage();
			return;
		}

		QString fname = QFileDialog::getSaveFileName(
							this,
							tr( "New shader's path" ),
							"/",
							tr( "Shader files ( *.nelshader )" )
							);

		shaderEditorWidget->reset();
		shaderEditorWidget->setName( name );

		QString sname;
		do{
			ok = true;
			shaderEditorWidget->exec();
			shaderEditorWidget->getName( sname );

			if( sname != name )
			{
				if( nameExists( sname ) )
				{
					ok = false;
					nameExistsMessage();
				}
			}

		}while( !ok );

		QString descr;
		shaderEditorWidget->getDescription( descr );

		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setText( 0, sname );
		item->setText( 1, descr );
		item->setText( 2, fname );
		shaderListWidget->addTopLevelItem( item );
		shaderListWidget->sortItems( 0, Qt::AscendingOrder );

		// save it
	}

	void ShaderWidget::onAddClicked()
	{
		QString fn = 
			QFileDialog::getOpenFileName(
			this,
			tr( "Load shader" ),
			"/",
			tr( "Shader files ( *.nelshader )" )
			);

	}

	void ShaderWidget::onRemoveClicked()
	{
		QTreeWidgetItem *item = shaderListWidget->currentItem();
		if( item == NULL )
			return;

		int selection =
			QMessageBox::question(
				this,
				tr( "Removing shader" ),
				tr( "Are you sure you want to remove this shader?" ),
				QMessageBox::Yes,
				QMessageBox::Cancel
				);

		if( selection == QMessageBox::Yes )
			delete item;
	}

	void ShaderWidget::onEditClicked()
	{
		QTreeWidgetItem *item = shaderListWidget->currentItem();
		if( item == NULL )
			return;

		QString name = item->text( 0 );
		QString description = item->text( 1 );

		shaderEditorWidget->reset();
		shaderEditorWidget->setName( name );
		shaderEditorWidget->setDescription( description );

		QString sname;
		bool ok;
		do{
			ok = true;
			shaderEditorWidget->exec();
			shaderEditorWidget->getName( sname );

			if( sname != name )
			{
				if( nameExists( sname ) )
				{
					ok = false;
					nameExistsMessage();
				}
			}

		}while( !ok );

		shaderEditorWidget->getDescription( description );
		item->setText( 0, sname );
		item->setText( 1, description );

		// save

	}
}

