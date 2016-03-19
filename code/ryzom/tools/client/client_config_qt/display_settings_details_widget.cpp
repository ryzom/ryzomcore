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

	// landscape
	landscapeSlider->setValue(getQualityPresetFloat("LandscapeTileNear"));
	landscapeSlider->setValue(std::min(landscapeSlider->value(), getQualityPresetFloat("LandscapeThreshold")));
	landscapeSlider->setValue(std::min(landscapeSlider->value(), getQualityPresetFloat("Vision")));
	landscapeSlider->setValue(std::min(landscapeSlider->value(), getQualityPresetFloat("MicroVegetDensity")));

	// FX
	fxSlider->setValue(getQualityPresetInteger("FxNbMaxPoly"));

	// characters
	charactersSlider->setValue(getQualityPresetInteger("SkinNbMaxPoly"));
	charactersSlider->setValue(std::min(charactersSlider->value(), getQualityPresetInteger("NbMaxSkeletonNotCLod")));
	charactersSlider->setValue(std::min(charactersSlider->value(), getQualityPresetFloat("CharacterFarClip")));

	int hdTextureInstalled = s.config.getInt("HDTextureInstalled");
	if (hdTextureInstalled == 1)
		texturesSlider->setMaximum(QUALITY_NORMAL);
	else
		texturesSlider->setMaximum(QUALITY_MEDIUM);

	// Comment taken from the original config tool:
	// NB: if the player changes its client.cfg, mixing HDEntityTexture=1 and DivideTextureSizeBy2=1, it will
	// result to a low quality!
	if (s.config.getInt("DivideTextureSizeBy2"))
		texturesSlider->setValue( QUALITY_LOW );
	else if( s.config.getInt("HDEntityTexture") && ( hdTextureInstalled == 1 ) )
		texturesSlider->setValue( QUALITY_NORMAL );
	else
		texturesSlider->setValue( QUALITY_MEDIUM );
}

void CDisplaySettingsDetailsWidget::save()
{
	CSystem &s = CSystem::GetInstance();

	// landscape
	setFloatPreset("LandscapeTileNear", landscapeSlider->value());
	setFloatPreset("LandscapeThreshold", landscapeSlider->value());
	setFloatPreset("Vision", landscapeSlider->value());
	setIntegerPreset("MicroVeget", landscapeSlider->value());
	setFloatPreset("MicroVegetDensity", landscapeSlider->value());

	// FX
	setIntegerPreset("FxNbMaxPoly", fxSlider->value());
	setIntegerPreset("Cloud", fxSlider->value());
	setFloatPreset("CloudQuality", fxSlider->value());
	setIntegerPreset("CloudUpdate", fxSlider->value());
	setIntegerPreset("Shadows", fxSlider->value());
	setIntegerPreset("FXAA", fxSlider->value());
	setIntegerPreset("Bloom", fxSlider->value());
	setIntegerPreset("SquareBloom", fxSlider->value());
	setFloatPreset("DensityBloom", fxSlider->value());

	// characters
	setIntegerPreset("SkinNbMaxPoly", charactersSlider->value());
	setIntegerPreset("NbMaxSkeletonNotCLod", charactersSlider->value());
	setFloatPreset("CharacterFarClip", charactersSlider->value());

	// misc
	if( texturesSlider->value() == QUALITY_NORMAL )
		s.config.setInt( "HDEntityTexture", 1 );
	else if( texturesSlider->value() == QUALITY_LOW )
		s.config.setInt( "DivideTextureSizeBy2", 1 );
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

float CDisplaySettingsDetailsWidget::getPresetFloat(const std::string &variable, sint preset)
{
	CSystem &s = CSystem::GetInstance();

	// preset name
	std::string varName = variable + NLMISC::toString("_ps%d", preset);

	return s.config.getFloat(varName.c_str());
}

int CDisplaySettingsDetailsWidget::getPresetInteger(const std::string &variable, sint preset)
{
	CSystem &s = CSystem::GetInstance();

	// preset name
	std::string varName = variable + NLMISC::toString("_ps%d", preset);

	return s.config.getInt(varName.c_str());
}

void CDisplaySettingsDetailsWidget::setFloatPreset(const std::string &variable, int preset)
{
	CSystem &s = CSystem::GetInstance();

	// take value of the preset and set it to variable
	s.config.setFloat(variable.c_str(), getPresetFloat(variable, preset));
}

void CDisplaySettingsDetailsWidget::setIntegerPreset(const std::string &variable, int preset)
{
	CSystem &s = CSystem::GetInstance();

	// take value of the preset and set it to variable
	s.config.setInt(variable.c_str(), getPresetInteger(variable, preset));
}

int CDisplaySettingsDetailsWidget::getQualityPresetFloat(const std::string &variable)
{
	CSystem &s = CSystem::GetInstance();

	float value = s.config.getFloat(variable.c_str());

	// ascending order
	if (getPresetFloat(variable, 0) < getPresetFloat(variable, QUALITY_STEP-1))
	{
		uint32 i = 0;
		while((i < QUALITY_STEP) && (getPresetFloat(variable, i) < value) )
			i++;
		return i;
	}
	// descending order
	else
	{
		uint32 i = 0;
		while((i < QUALITY_STEP) && (getPresetFloat(variable, i) > value))
			i++;
		return i;
	}
}

int CDisplaySettingsDetailsWidget::getQualityPresetInteger(const std::string &variable)
{
	CSystem &s = CSystem::GetInstance();

	float value = s.config.getFloat(variable.c_str());

	// ascending order
	if (getPresetFloat(variable, 0) < getPresetFloat(variable, QUALITY_STEP-1))
	{
		uint32 i = 0;
		while((i < QUALITY_STEP) && (getPresetFloat(variable, i) < value) )
			i++;
		return i;
	}
	// descending order
	else
	{
		uint32 i = 0;
		while((i < QUALITY_STEP) && (getPresetFloat(variable, i) > value))
			i++;
		return i;
	}
}

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
		return tr( "High (more than 128 MB)" );
		break;

	default:
		return "";
		break;
	}
}