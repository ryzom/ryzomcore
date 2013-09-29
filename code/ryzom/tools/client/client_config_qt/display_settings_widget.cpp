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
#include "display_settings_widget.h"

#include "system.h"
#include <QRegExpValidator>

CDisplaySettingsWidget::CDisplaySettingsWidget( QWidget *parent ) :
	CWidgetBase( parent )
{
	setupUi( this );
	widthLineEdit->setValidator( new QRegExpValidator( QRegExp( "[0-9]{1,6}" ), widthLineEdit ) );
	heightLineEdit->setValidator( new QRegExpValidator( QRegExp( "[0-9]{1,6}" ), heightLineEdit ) );
	xpositionLineEdit->setValidator( new QRegExpValidator( QRegExp( "[0-9]{1,6}" ), xpositionLineEdit ) );
	ypositionLineEdit->setValidator( new QRegExpValidator( QRegExp( "[0-9]{1,6}" ), ypositionLineEdit ) );
	load();

	connect( autoRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( openglRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( direct3dRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( fullscreenRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( windowedRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( widthLineEdit, SIGNAL( textChanged( const QString & ) ), this, SLOT( onSomethingChanged() ) );
	connect( heightLineEdit, SIGNAL( textChanged( const QString & ) ), this, SLOT( onSomethingChanged() ) );
	connect( xpositionLineEdit, SIGNAL( textChanged( const QString & ) ), this, SLOT( onSomethingChanged() ) );
	connect( ypositionLineEdit, SIGNAL( textChanged( const QString & ) ), this, SLOT( onSomethingChanged() ) );
	connect( videomodeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSomethingChanged() ) );
	connect( autoRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( updateVideoModes() ) );
	connect( openglRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( updateVideoModes() ) );
	connect( direct3dRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( updateVideoModes() ) );
}

CDisplaySettingsWidget::~CDisplaySettingsWidget()
{
}

void CDisplaySettingsWidget::load()
{
	CSystem &s = CSystem::GetInstance();

	std::string driver = s.config.getString( "Driver3D" );
	switch( getDriverFromConfigString( driver ) )
	{
	case DRV_AUTO:
		autoRadioButton->setChecked( true );
		break;
	case DRV_OPENGL:
		openglRadioButton->setChecked( true );
		break;
	case DRV_DIRECT3D:
		direct3dRadioButton->setChecked( true );
		break;
	}

	updateVideoModes();


	CVideoMode mode;
	mode.width = s.config.getInt( "Width" );
	mode.height = s.config.getInt( "Height" );
	mode.depth = s.config.getInt( "Depth" );
	mode.frequency = s.config.getInt( "Frequency" );

	if( s.config.getInt( "FullScreen" ) == 1 )
	{
		fullscreenRadioButton->setChecked( true );
		videomodeComboBox->setCurrentIndex( findVideoModeIndex( &mode ) );
	}
	else
	{
		windowedRadioButton->setChecked( true );
	}

	widthLineEdit->setText( QString( "%1" ).arg( mode.width ) );
	heightLineEdit->setText( QString( "%1" ).arg( mode.height ) );
	xpositionLineEdit->setText( QString( "%1" ).arg( s.config.getInt( "PositionX" ) ) );
	ypositionLineEdit->setText( QString( "%1" ).arg( s.config.getInt( "PositionY" ) ) );

}

void CDisplaySettingsWidget::save()
{
	CSystem &s = CSystem::GetInstance();

	if( openglRadioButton->isChecked() )
		s.config.setString( "Driver3D", std::string( "OpenGL" ) );
	else if( direct3dRadioButton->isChecked() )
		s.config.setString( "Driver3D", std::string( "Direct3D" ) );
	else
		s.config.setString( "Driver3D", std::string( "Auto" ) );

	if( fullscreenRadioButton->isChecked() )
	{
		s.config.setInt( "FullScreen", 1 );

		sint32 index = videomodeComboBox->currentIndex();
		CVideoMode mode;

		// OpenGL should be available everywhere!
		if( direct3dRadioButton->isChecked() )
			mode = s.d3dInfo.modes[ index ];
		else
			mode = s.openglInfo.modes[ index ];

		s.config.setInt( "Width", mode.width );
		s.config.setInt( "Height", mode.height );
		s.config.setInt( "Depth", mode.depth );
		s.config.setInt( "Frequency", mode.frequency );

	}
	else
	{
		s.config.setInt( "FullScreen", 0 );
		s.config.setInt( "Width", widthLineEdit->text().toInt() );
		s.config.setInt( "Height", heightLineEdit->text().toInt() );
	}

	s.config.setInt( "PositionX", xpositionLineEdit->text().toInt() );
	s.config.setInt( "PositionY", ypositionLineEdit->text().toInt() );
}

void CDisplaySettingsWidget::changeEvent( QEvent *event )
{
	if( event->type() == QEvent::LanguageChange )
	{
		disconnect( videomodeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSomethingChanged() ) );
		retranslateUi( this );
		connect( videomodeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSomethingChanged() ) );
	}
	QWidget::changeEvent( event );
}

void CDisplaySettingsWidget::updateVideoModes()
{
	CSystem &s = CSystem::GetInstance();

	videomodeComboBox->clear();

	if( direct3dRadioButton->isChecked() )
	{
		for( std::vector< CVideoMode >::iterator itr = s.d3dInfo.modes.begin(); itr != s.d3dInfo.modes.end(); ++itr )
		{
			std::stringstream ss;
			ss << itr->widht << "x" << itr->height << " " << itr->depth << " bit @" << itr->frequency;
			videomodeComboBox->addItem( ss.str().c_str() );
		}
	}
	else
	{
		// OpenGL should be available everywhere!
		for( std::vector< CVideoMode >::iterator itr = s.openglInfo.modes.begin(); itr != s.openglInfo.modes.end(); ++itr )
		{
			std::stringstream ss;
			ss << itr->widht << "x" << itr->height << " " << itr->depth << " bit @" << itr->frequency;
			videomodeComboBox->addItem( ss.str().c_str() );
		}
	}
}

uint32 CDisplaySettingsWidget::findVideoModeIndex( CVideoMode *mode )
{
	//////////////////////////////////////////////////////////////////////////////////
	// WARNING:
	// This part relies on that the video mode combo box is filled as the following:
	//
	//| --------------------------------------|
	//| Selected driver | Modes               |
	//| --------------------------------------|
	//| Auto            | OpenGL modes        |
	//| OpenGL          | OpenGL modes        |
	//| Direct3D        | Direct3d modes      |
	//| --------------------------------------|
	//
	//
	//////////////////////////////////////////////////////////////////////////////////

	CVideoMode &m = *mode;
	CSystem &s = CSystem::GetInstance();

	if( direct3dRadioButton->isChecked() )
	{
		for( uint32 i = 0; i < s.d3dInfo.modes.size(); i++ )
			if( s.d3dInfo.modes[ i ] == m )
				return i;
	}
	else
	{
		// Again OpenGL should be available everywhere!
		for( uint32 i = 0; i < s.openglInfo.modes.size(); i++ )
			if( s.openglInfo.modes[ i ] == m )
				return i;
	}

	return 0;
}

E3DDriver CDisplaySettingsWidget::getDriverFromConfigString(std::string &str) const
{
	if( str.compare( "0" ) == 0 )
		return DRV_AUTO;
	if( str.compare( "1" ) == 0 )
		return DRV_OPENGL;
	if( str.compare( "2" ) == 0 )
		return DRV_DIRECT3D;
	if( str.compare( "OpenGL" ) == 0 )
		return DRV_OPENGL;
	if( str.compare( "Direct3D" ) == 0)
		return DRV_DIRECT3D;


	return DRV_AUTO;
}
