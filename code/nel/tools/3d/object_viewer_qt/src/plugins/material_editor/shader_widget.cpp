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
		for( int i = 0; i < shaderList->count(); i++ )
		{
			if( shaderList->item( i )->text() == name ) 
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

	void ShaderWidget::onAddClicked()
	{
		QString name =
			QInputDialog::getText(
			this,
			tr( "New shader" ),
			tr( "Enter the new shader's name" )
			);

		if( name.isEmpty() )
		{
			QMessageBox::critical( 
				this,
				tr( "New shader" ),
				tr( "You must enter a name for the new shader" ),
				QMessageBox::Ok 
				);
			return;
		}

		if( nameExists( name ) )
		{
			nameExistsMessage();
			return;
		}

		shaderList->addItem( name );
	}

	void ShaderWidget::onRemoveClicked()
	{
		int i = shaderList->currentRow();
		if( i < 0 )
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
		{
			QListWidgetItem *item = shaderList->takeItem( i );
			delete item;
		}
	}

	void ShaderWidget::onEditClicked()
	{
		int i = shaderList->currentRow();
		if( i < 0 )
			return;

		QString name = shaderList->item( i )->text();

		shaderEditorWidget->reset();
		shaderEditorWidget->setName( name );

		QString sname;
		bool ok;
		int res;
		do{
			ok = true;
			res = shaderEditorWidget->exec();
			if( res == QDialog::Rejected )
				return;

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

		shaderList->item( i )->setText( sname );

		// save

	}
}

