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
#include "shader_editor.h"
#include "material_properties.h"

namespace MaterialEditor
{

	MaterialWidget::MaterialWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		shaderEditorWidget = new ShaderEditorWidget();
		matPropWidget = new MatPropWidget();
		setupConnections();
	}

	MaterialWidget::~MaterialWidget()
	{
		delete shaderEditorWidget;
		shaderEditorWidget = NULL;
		delete matPropWidget;
		matPropWidget = NULL;
	}

	void MaterialWidget::setupConnections()
	{
		connect( passButton, SIGNAL( clicked( bool ) ), this, SLOT( onPassEditClicked() ) );
		connect( shaderButton, SIGNAL( clicked( bool ) ), this, SLOT( onShaderEditClicked() ) );

		connect( shaderEditorWidget, SIGNAL( okClicked() ), this, SLOT( onShaderEditOKClicked() ) );
	}

	void MaterialWidget::onPassEditClicked()
	{
		matPropWidget->show();
	}

	void MaterialWidget::onShaderEditClicked()
	{
		shaderEditorWidget->show();
	}

	void MaterialWidget::onShaderEditOKClicked()
	{
	}

}

