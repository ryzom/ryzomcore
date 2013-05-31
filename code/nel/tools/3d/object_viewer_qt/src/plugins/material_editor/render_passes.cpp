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

#include "render_passes.h"

namespace MaterialEditor
{
	RenderPassesWidget::RenderPassesWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setupConnections();
	}

	RenderPassesWidget::~RenderPassesWidget()
	{
	}

	void RenderPassesWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );		
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditClicked() ) );
		connect( upButton, SIGNAL( clicked( bool ) ), this, SLOT( onUpClicked() ) );
		connect( downButton, SIGNAL( clicked( bool ) ), this, SLOT( onDownClicked() ) );
	}

	void RenderPassesWidget::onOKClicked()
	{
		close();
	}

	void RenderPassesWidget::onCancelClicked()
	{
		close();
	}

	void RenderPassesWidget::onAddClicked()
	{
	}

	void RenderPassesWidget::onRemoveClicked()
	{
	}

	void RenderPassesWidget::onEditClicked()
	{
	}

	void RenderPassesWidget::onUpClicked()
	{
	}

	void RenderPassesWidget::onDownClicked()
	{
	}
}

