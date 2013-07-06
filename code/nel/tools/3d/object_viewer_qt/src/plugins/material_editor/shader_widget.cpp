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
#include "nel3d_interface.h"
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
		nl3dIface = NULL;
	}

	ShaderWidget::~ShaderWidget()
	{
		delete shaderEditorWidget;
		shaderEditorWidget = NULL;
	}

	void ShaderWidget::load()
	{
		std::vector< std::string > v;

		nl3dIface->getShaderList( v );

		shaderList->clear();

		QString name;

		std::vector< std::string >::const_iterator itr = v.begin();
		while( itr != v.end() )
		{
			name = itr->c_str();
			shaderList->addItem( name );
			Q_EMIT shaderAdded( name );

			++itr;
		}
	}

	void ShaderWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );

		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditClicked() ) );

		connect( shaderList, SIGNAL( currentRowChanged( int ) ), this, SLOT( onRowChanged( int ) ) );
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

		SShaderInfo info;
		info.name = name.toUtf8().data();
		bool ok = nl3dIface->addShader( info );
		nl3dIface->saveShader( info.name );
		if( !ok )
		{
			QMessageBox::critical( 
				this,
				tr( "Error adding shader" ),
				tr( "There was an error while trying to add the shader" )
				);

			return;
		}

		shaderList->addItem( name );

		Q_EMIT shaderAdded( name );
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
			QString name = item->text();
			std::string n = name.toUtf8().data();
			delete item;
			
			nl3dIface->removeShader( n );
			nl3dIface->deleteShader( n );
			
			if( shaderList->count() == 0 )
				description->setPlainText( "" );

			Q_EMIT shaderRemoved( name );
		}
		
	}

	void ShaderWidget::onEditClicked()
	{
		int i = shaderList->currentRow();
		if( i < 0 )
			return;

		QString name = shaderList->item( i )->text();

		shaderEditorWidget->reset();
		
		SShaderInfo info;
		std::string n = name.toUtf8().data();
		bool ok = nl3dIface->getShaderInfo( n, info );
		if( !ok )
		{
			QMessageBox::critical(
				this,
				tr( "Error retrieving shader data" ),
				tr( "There was an error while trying to retrieve shader data!" )
				);

			return;
		}

		shaderEditorWidget->setName( info.name.c_str() );
		shaderEditorWidget->setDescription( info.description.c_str() );
		shaderEditorWidget->setVertexShader( info.vp.c_str() );
		shaderEditorWidget->setFragmentShader( info.fp.c_str() );

		int res = shaderEditorWidget->exec();
		if( res == QDialog::Rejected )
			return;

		// save
		QString s;

		shaderEditorWidget->getDescription( s );
		info.description = s.toUtf8().data();

		shaderEditorWidget->getVertexShader( s );
		info.vp = s.toUtf8().data();

		shaderEditorWidget->getFragmentShader( s );
		info.fp = s.toUtf8().data();

		ok = nl3dIface->updateShaderInfo( info );
		if( !ok )
		{
			QMessageBox::critical(
				this,
				tr( "Error saving shader data" ),
				tr( "There was an error while trying to save shader data!" )
				);
		}
		nl3dIface->saveShader( info.name );

		description->setPlainText( info.description.c_str() );
	}

	void ShaderWidget::onRowChanged( int i )
	{
		if( i < 0 )
			return;

		std::string s = shaderList->item( i )->text().toUtf8().data();

		SShaderInfo info;
		bool ok = nl3dIface->getShaderInfo( s, info );
		if( !ok )
			return;

		description->setPlainText( info.description.c_str() );
	}
}

