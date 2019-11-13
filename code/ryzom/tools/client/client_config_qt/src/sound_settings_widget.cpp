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

	connect(autoRadioButton, SIGNAL(clicked(bool)), this, SLOT(onSomethingChanged()));
	connect(openalRadioButton, SIGNAL(clicked(bool)), this, SLOT(onSomethingChanged()));
	connect(fmodRadioButton, SIGNAL(clicked(bool)), this, SLOT(onSomethingChanged()));
	connect(xaudio2RadioButton, SIGNAL(clicked(bool)), this, SLOT(onSomethingChanged()));
	connect(directsoundRadioButton, SIGNAL(clicked(bool)), this, SLOT(onSomethingChanged()));

	connect( tracksSlider, SIGNAL( valueChanged( int ) ), this, SLOT( onTracksSliderChange() ) );
	connect( soundCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( eaxCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
	connect( softwareCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( onSomethingChanged() ) );
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

	updateTracksLabel();

	std::string soundDriver = NLMISC::toLower(s.config.getString("DriverSound"));

#ifdef Q_OS_WIN32
	fmodRadioButton->setEnabled(true);
	xaudio2RadioButton->setEnabled(true);
	directsoundRadioButton->setEnabled(true);
#else
	fmodRadioButton->setEnabled(false);
	xaudio2RadioButton->setEnabled(false);
	directsoundRadioButton->setEnabled(false);
#endif

	if (soundDriver.compare("openal") == 0)
	{
		openalRadioButton->setChecked(true);
	}
	else if (soundDriver.compare("fmod") == 0)
	{
		fmodRadioButton->setChecked(true);
	}
	else if (soundDriver.compare("xaudio2") == 0)
	{
		xaudio2RadioButton->setChecked(true);
	}
	else if (soundDriver.compare("directsound") == 0)
	{
		directsoundRadioButton->setChecked(true);
	}
	else
	{
		autoRadioButton->setChecked(true);
	}
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

	if (openalRadioButton->isChecked())
		s.config.setString("DriverSound", std::string("OpenAL"));
#ifdef Q_OS_WIN32
	else if (fmodRadioButton->isChecked())
		s.config.setString("DriverSound", std::string("FMod"));
	else if (xaudio2RadioButton->isChecked())
		s.config.setString("DriverSound", std::string("XAudio2"));
	else if (directsoundRadioButton->isChecked())
		s.config.setString("DriverSound", std::string("DirectSound"));
#endif
	else
		s.config.setString("DriverSound", std::string("Auto"));
}

void CSoundSettingsWidget::updateTracksLabel()
{
	tracksLabel->setText( tr( "%1 tracks" ).arg( tracksSlider->value() * 4 ) );
}