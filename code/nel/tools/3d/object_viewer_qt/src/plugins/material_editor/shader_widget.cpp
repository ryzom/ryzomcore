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

namespace MaterialEditor
{
	ShaderWidget::ShaderWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setupConnections();
	}

	ShaderWidget::~ShaderWidget()
	{
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

	void ShaderWidget::onNewClicked()
	{
	}

	void ShaderWidget::onAddClicked()
	{
	}

	void ShaderWidget::onRemoveClicked()
	{
	}

	void ShaderWidget::onEditClicked()
	{
	}
}

