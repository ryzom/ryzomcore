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

#include "material_widget.h"
#include "render_passes.h"
#include "shader_editor.h"

namespace MaterialEditor
{

	MaterialWidget::MaterialWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		renderPassesWidget = new RenderPassesWidget();
		shaderEditorWidget = new ShaderEditorWidget();
		setupConnections();
	}

	MaterialWidget::~MaterialWidget()
	{
		delete renderPassesWidget;
		renderPassesWidget = NULL;
		delete shaderEditorWidget;
		shaderEditorWidget = NULL;
	}

	void MaterialWidget::setupConnections()
	{
		connect( passButton, SIGNAL( clicked( bool ) ), this, SLOT( onPassEditClicked() ) );
		connect( shaderButton, SIGNAL( clicked( bool ) ), this, SLOT( onShaderEditClicked() ) );

		connect( renderPassesWidget, SIGNAL( passAdded( const QString& ) ),
			this, SLOT( onPassAdded( const QString& ) ) );

		connect( renderPassesWidget, SIGNAL( passRemoved( const QString& ) ),
			this, SLOT( onPassRemoved( const QString& ) ) );

		connect( renderPassesWidget, SIGNAL( passRenamed( const QString&, const QString& ) ),
			this, SLOT( onPassRenamed( const QString&, const QString& ) ) );
		
		connect( renderPassesWidget, SIGNAL( passPushedUp( const QString& ) ),
			this, SLOT( onPassPushedUp( const QString& ) ) );

		connect( renderPassesWidget, SIGNAL( passPushedDown( const QString& ) ),
			this, SLOT( onPassPushedDown( const QString& ) ) );

		connect( shaderEditorWidget, SIGNAL( okClicked() ), this, SLOT( onShaderEditOKClicked() ) );
	}

	void MaterialWidget::onPassEditClicked()
	{
		renderPassesWidget->show();
	}

	void MaterialWidget::onShaderEditClicked()
	{
		shaderEditorWidget->show();
	}

	void MaterialWidget::onPassAdded( const QString &pass )
	{
		passCB->addItem( pass );
	}

	void MaterialWidget::onPassRemoved( const QString &pass )
	{
		int i = passCB->findText( pass );
		if( i == -1 )
			return;

		passCB->removeItem( i );
	}

	void MaterialWidget::onPassRenamed( const QString &from, const QString &to )
	{
		int i = passCB->findText( from );
		if( i == -1 )
			return;
		passCB->setItemText( i, to );
	}

	void MaterialWidget::onPassPushedUp( const QString &pass )
	{
		int i = passCB->findText( pass );
		if( i == -1 )
			return;

		if( i == 0 )
			return;

		QString temp = passCB->itemText( i - 1 );
		passCB->setItemText( i - 1, pass );
		passCB->setItemText( i, temp );

		if( passCB->currentIndex() == i )
			passCB->setCurrentIndex( i - 1 );
	}

	void MaterialWidget::onPassPushedDown( const QString &pass )
	{
		int i = passCB->findText( pass );
		if( i == -1 )
			return;

		if( i == ( passCB->count() - 1 ) )
			return;

		QString temp = passCB->itemText( i + 1 );
		passCB->setItemText( i + 1, pass );
		passCB->setItemText( i, temp );

		if( passCB->currentIndex() == i )
			passCB->setCurrentIndex( i + 1 );
	}

	void MaterialWidget::onShaderEditOKClicked()
	{
	}

}

