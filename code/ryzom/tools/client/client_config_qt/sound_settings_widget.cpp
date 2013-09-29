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
#include "sound_settings_widget.h"
#include "system.h"

CSoundSettingsWidget::CSoundSettingsWidget( QWidget *parent ) :
	CWidgetBase( parent )
{
	setupUi( this );
	load();

	connect( tracksSlider, SIGNAL( valueChanged( int ) ), this, SLOT( onTracksSliderChange() ) );
	connect( soundCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( eaxCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( softwareCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( fmodCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
}

CSoundSettingsWidget::~CSoundSettingsWidget()
{
}

void CSoundSettingsWidget::onTracksSliderChange()
{
	updateTracksLabel();
	emit changed();
}

void CSoundSettingsWidget::load()
{
	CSystem &s = CSystem::GetInstance();

	if( s.config.getInt( "SoundOn" ) == 1 )
		soundCheckBox->setChecked( true );

	if( s.config.getInt( "UseEax" ) == 1 )
		eaxCheckBox->setChecked( true );

	if( s.config.getInt( "SoundForceSoftwareBuffer" ) == 1 )
		softwareCheckBox->setChecked( true );

	sint32 tracks = s.config.getInt( "MaxTrack" );
	if( tracks < 4 )
		tracks = 4;
	if( tracks > 32 )
		tracks = 32;
	tracksSlider->setValue( tracks / 4 );

	if( s.config.getString( "DriverSound" ).compare( "FMod" ) == 0 )
		fmodCheckBox->setChecked( true );
}

void CSoundSettingsWidget::save()
{
	CSystem &s = CSystem::GetInstance();

	if( soundCheckBox->isChecked() )
		s.config.setInt( "SoundOn", 1 );

	if( eaxCheckBox->isChecked() )
		s.config.setInt( "UseEax", 1 );

	if( softwareCheckBox->isChecked() )
		s.config.setInt( "SoundForceSoftwareBuffer", 1 );

	s.config.setInt( "MaxTrack", tracksSlider->value() * 4 );

	if( fmodCheckBox->isChecked() )
		s.config.setString( "DriverSound", std::string( "FMod" ) );
}

void CSoundSettingsWidget::changeEvent( QEvent *event )
{
	if( event->type() == QEvent::LanguageChange )
	{
		retranslateUi( this );
		updateTracksLabel();
	}
	QWidget::changeEvent( event );
}

void CSoundSettingsWidget::updateTracksLabel()
{
	tracksLabel->setText( tr( "%1 tracks" ).arg( tracksSlider->value() * 4 ) );
}