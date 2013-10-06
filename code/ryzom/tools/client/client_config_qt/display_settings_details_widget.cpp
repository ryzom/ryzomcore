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
#include "display_settings_details_widget.h"

#include "system.h"

CDisplaySettingsDetailsWidget::CDisplaySettingsDetailsWidget( QWidget *parent ) :
	CWidgetBase( parent )
{
	setupUi( this );
	connect( landscapeSlider, SIGNAL( valueChanged( int ) ), this, SLOT( onLandscapeSliderChange( int ) ) );
	connect( charactersSlider, SIGNAL( valueChanged( int ) ), this, SLOT( onCharactersSliderChange( int ) ) );
	connect( fxSlider, SIGNAL( valueChanged( int ) ), this, SLOT( onFXSliderChange( int ) ) );
	connect( texturesSlider, SIGNAL( valueChanged( int ) ), this, SLOT( onTexturesSliderChange( int ) ) );
	load();
}

CDisplaySettingsDetailsWidget::~CDisplaySettingsDetailsWidget()
{
}

void CDisplaySettingsDetailsWidget::load()
{
	CSystem &s = CSystem::GetInstance();

	landscapeSlider->setValue( getQuality( qualityToLandscapeThreshold, s.config.getFloat( "LandscapeThreshold" ) ) );
	landscapeSlider->setValue( std::min( landscapeSlider->value(), getQuality( qualityToZFar, s.config.getFloat( "Vision" ) ) ) );
	landscapeSlider->setValue( std::min( landscapeSlider->value(), getQuality( qualityToLandscapeTileNear, s.config.getFloat( "LandscapeTileNear" ) ) ) );
	landscapeSlider->setValue( std::min( landscapeSlider->value(), getQuality( qualityToMicrovegetDensity, s.config.getFloat( "MicroVegetDensity" ) ) ) );

	charactersSlider->setValue( getQuality( qualityToSkinNbMaxPoly, s.config.getInt( "SkinNbMaxPoly" ) ) );
	charactersSlider->setValue( std::min( charactersSlider->value(), getQuality( qualityToNbMaxSkeletonNotCLod, s.config.getInt( "NbMaxSkeletonNotCLod" ) ) ) );
	charactersSlider->setValue( std::min( charactersSlider->value(), getQuality( qualityToCharacterFarClip, s.config.getFloat( "CharacterFarClip" ) ) ) );

	fxSlider->setValue( getQuality( qualityToFxNbMaxPoly, s.config.getInt( "FxNbMaxPoly" ) ) );

	int hdTextureInstalled = s.config.getInt( "HDTextureInstalled" );
	if( hdTextureInstalled == 1 )
		texturesSlider->setMaximum( QUALITY_NORMAL );
	else
		texturesSlider->setMaximum( QUALITY_MEDIUM );

	// Comment taken from the original config tool:
	// NB: if the player changes its client.cfg, mixing HDEntityTexture=1 and DivideTextureSizeBy2=1, it will
	// result to a low quality!
	if( s.config.getInt( "DivideTextureSizeBy2" ) )
		texturesSlider->setValue( QUALITY_LOW );
	else if( s.config.getInt( "HDEntityTexture" ) && ( hdTextureInstalled == 1 ) )
		texturesSlider->setValue( QUALITY_NORMAL );
	else
		texturesSlider->setValue( QUALITY_MEDIUM );
}

void CDisplaySettingsDetailsWidget::save()
{
	CSystem &s = CSystem::GetInstance();

	s.config.setFloat( "Vision", qualityToZFar[ landscapeSlider->value() ] );
	s.config.setFloat( "LandscapeTileNear", qualityToLandscapeTileNear[ landscapeSlider->value() ] );
	s.config.setFloat( "LandscapeThreshold", qualityToLandscapeThreshold[ landscapeSlider->value() ] );

	if( landscapeSlider->value() > QUALITY_LOW )
		s.config.setInt( "MicroVeget", 1 );
	else
		s.config.setInt( "MicroVeget", 0 );

	s.config.setFloat( "MicroVegetDensity", qualityToMicrovegetDensity[ landscapeSlider->value() ] );


	s.config.setInt( "SkinNbMaxPoly", qualityToSkinNbMaxPoly[ charactersSlider->value() ] );
	s.config.setInt( "NbMaxSkeletonNotCLod", qualityToNbMaxSkeletonNotCLod[ charactersSlider->value() ] );
	s.config.setFloat( "CharacterFarClip", qualityToCharacterFarClip[ charactersSlider->value() ] );


	s.config.setInt( "FxNbMaxPoly", qualityToFxNbMaxPoly[ fxSlider->value() ] );
	if( fxSlider->value() > QUALITY_LOW )
	{
		s.config.setInt( "Shadows", 1 );
		s.config.setInt( "Bloom", 1 );
		s.config.setInt( "SquareBloom", 1 );
	}
	else
	{
		s.config.setInt( "Shadows", 0 );
		s.config.setInt( "Bloom", 0 );
		s.config.setInt( "SquareBloom", 0 );
	}


	if( texturesSlider->value() == QUALITY_NORMAL )
		s.config.setInt( "HDEntityTexture", 1 );
	else if( texturesSlider->value() == QUALITY_LOW )
		s.config.setInt( "DivideTextureSizeBy2", 1 );
}

void CDisplaySettingsDetailsWidget::changeEvent( QEvent *event )
{
	if( event->type() == QEvent::LanguageChange )
	{
		retranslateUi( this );

		landscapeLabel->setText( getQualityString( landscapeSlider->value() ) );
		characterLabel->setText( getQualityString( charactersSlider->value() ) );
		fxLabel->setText( getQualityString( fxSlider->value() ) );
		textureLabel->setText( getTextureQualityString( texturesSlider->value() ) );
	}
	QWidget::changeEvent( event );
}


void CDisplaySettingsDetailsWidget::onLandscapeSliderChange( int value )
{
	if( ( value < 0 ) || ( value > 3 ) )
		return;

	landscapeLabel->setText( getQualityString( value ) );
	emit changed();
}

void CDisplaySettingsDetailsWidget::onCharactersSliderChange( int value )
{
	if( ( value < 0 ) || ( value > 3 ) )
		return;

	characterLabel->setText( getQualityString( value ) );
	emit changed();
}

void CDisplaySettingsDetailsWidget::onFXSliderChange( int value )
{
	if( ( value < 0 ) || ( value > 3 ) )
		return;

	fxLabel->setText( getQualityString( value ) );
	emit changed();
}

void CDisplaySettingsDetailsWidget::onTexturesSliderChange( int value )
{
	if( ( value < 0 ) || ( value > 3 ) )
		return;

	textureLabel->setText( getTextureQualityString( value ) );
	emit changed();
}

const float CDisplaySettingsDetailsWidget::qualityToZFar[ QUALITY_STEP ] =
{
	200.0f,
	400.0f,
	500.0f,
	800.0f
};

const float CDisplaySettingsDetailsWidget::qualityToLandscapeTileNear[ QUALITY_STEP ] =
{
	20.0f,
	100.0f,
	150.0f,
	200.0f
};

const float CDisplaySettingsDetailsWidget::qualityToLandscapeThreshold[ QUALITY_STEP ] =
{
	100.0f,
	1000.0f,
	2000.0f,
	3000.0f
};


const float CDisplaySettingsDetailsWidget::qualityToMicrovegetDensity[ QUALITY_STEP ] =
{
	10.0f,
	30.0f,
	80.0f,
	100.0f
};


const sint32 CDisplaySettingsDetailsWidget::qualityToSkinNbMaxPoly[ QUALITY_STEP ] =
{
	10000,
	70000,
	100000,
	200000
};

const sint32 CDisplaySettingsDetailsWidget::qualityToNbMaxSkeletonNotCLod[ QUALITY_STEP ] =
{
	10,
	50,
	125,
	255
};

const float CDisplaySettingsDetailsWidget::qualityToCharacterFarClip[ QUALITY_STEP ] =
{
	50.0f,
	100.0f,
	200.0f,
	500.0f
};

const sint32 CDisplaySettingsDetailsWidget::qualityToFxNbMaxPoly[ QUALITY_STEP ] =
{
	2000,
	10000,
	20000,
	50000
};

QString CDisplaySettingsDetailsWidget::getQualityString( uint32 quality )
{
	switch( quality )
	{
	case QUALITY_LOW:
		return tr( "Low" );
		break;
	case QUALITY_MEDIUM:
		return tr( "Medium" );
		break;
	case QUALITY_NORMAL:
		return tr( "Normal" );
		break;
	case QUALITY_HIGH:
		return tr( "High" );
		break;
	default:
		return "";
		break;
	}
}

QString CDisplaySettingsDetailsWidget::getTextureQualityString( uint32 quality )
{
	switch( quality )
	{
	case TEXQUALITY_LOW:
		return tr( "Low (32 MB)" );
		break;

	case TEXQUALITY_NORMAL:
		return tr( "Normal (64 MB)" );
		break;

	case TEXQUALITY_HIGH:
		return tr( "High (128 MB)" );
		break;

	default:
		return "";
		break;
	}
}