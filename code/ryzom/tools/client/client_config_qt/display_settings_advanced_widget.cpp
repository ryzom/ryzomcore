// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "display_settings_advanced_widget.h"
#include "system.h"

CDisplaySettingsAdvancedWidget::CDisplaySettingsAdvancedWidget( QWidget *parent ) :
	CWidgetBase( parent )
{
	setupUi( this );
	load();

	connect( texcompressionCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( vertexshaderCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( verticesagpCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( pixelshadersCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
}

CDisplaySettingsAdvancedWidget::~CDisplaySettingsAdvancedWidget()
{
}

void CDisplaySettingsAdvancedWidget::load()
{
	CSystem &s = CSystem::GetInstance();

	if( s.config.getInt( "ForceDXTC" ) == 1 )
		texcompressionCheckBox->setChecked( true );

	if( s.config.getInt( "DisableVtxProgram" ) == 1 )
		vertexshaderCheckBox->setChecked( true );

	if( s.config.getInt( "DisableVtxAGP" ) == 1 )
		verticesagpCheckBox->setChecked( true );

	if( s.config.getInt( "DisableTextureShdr" ) == 1 )
		pixelshadersCheckBox->setChecked( true );
}

void CDisplaySettingsAdvancedWidget::save()
{
	CSystem &s = CSystem::GetInstance();

	if( texcompressionCheckBox->isChecked() )
		s.config.setInt( "ForceDXTC", 1 );
	else
		s.config.setInt( "ForceDXTC", 0 );

	if( vertexshaderCheckBox->isChecked() )
		s.config.setInt( "DisableVtxProgram", 1 );
	else
		s.config.setInt( "DisableVtxProgram", 0 );

	if( verticesagpCheckBox->isChecked() )
		s.config.setInt( "DisableVtxAGP", 1 );
	else
		s.config.setInt( "DisableVtxAGP", 0 );

	if( pixelshadersCheckBox->isChecked() )
		s.config.setInt( "DisableTextureShdr", 1 );
	else
		s.config.setInt( "DisableTextureShdr", 0 );
}

void CDisplaySettingsAdvancedWidget::changeEvent( QEvent *event )
{
	if( event->type() == QEvent::LanguageChange )
	{
		retranslateUi( this );
	}

	QWidget::changeEvent( event );
}
