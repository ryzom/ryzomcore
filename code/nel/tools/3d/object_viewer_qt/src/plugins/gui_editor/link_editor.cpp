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


#include "link_editor.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

namespace GUIEditor
{
	LinkEditor::LinkEditor( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setup();
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
	}

	LinkEditor::~LinkEditor()
	{
	}

	void LinkEditor::setup()
	{
		expressionEdit->clear();
		groupCB->setCheckable( true );
		groupCB->setChecked( false );
		groupCB->setDisabled( true );
		ahCB->setCheckable( true );
		ahCB->setChecked( false );
		ahCB->setDisabled( true );
		ahEdit->clear();
		ahParamEdit->clear();
		ahParamEdit->setDisabled( true );
	}

	void LinkEditor::setLinkId( uint32 linkId )
	{
		setup();
		currentLinkId = linkId;

		const std::map< uint32, SLinkData > &linkMap =
			CWidgetManager::getInstance()->getParser()->getLinkMap();

		std::map< uint32, SLinkData >::const_iterator itr =
			linkMap.find( currentLinkId );

		if( itr == linkMap.end() )
			return;
		SLinkData data = itr->second;

		expressionEdit->setPlainText( data.expr.c_str() );
		if( !data.target.empty() )
		{
			groupCB->setEnabled( true );
			groupCB->setChecked( true );
			ahEdit->setText( data.target.c_str() );
		}
		else
		{
			ahCB->setEnabled( true );
			ahCB->setChecked( true );
			ahEdit->setText( data.action.c_str() );
			ahParamEdit->setEnabled( true );
			ahParamEdit->setText( data.params.c_str() );
		}
	}
}
