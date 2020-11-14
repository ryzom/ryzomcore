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

#ifndef DISPLAYSETTINGSDETAILSWIDGET_H
#define DISPLAYSETTINGSDETAILSWIDGET_H

#include "ui_display_settings_details_widget.h"
#include "widget_base.h"
#include <nel/misc/types_nl.h>

enum
{
	QUALITY_LOW    = 0,
	QUALITY_MEDIUM = 1,
	QUALITY_NORMAL = 2,
	QUALITY_HIGH   = 3,
	QUALITY_STEP   = 4
};

enum
{
	TEXQUALITY_LOW    = 0,
	TEXQUALITY_NORMAL = 1,
	TEXQUALITY_HIGH   = 2
};

/**
 @brief The display details page of the configuration tool
*/
class CDisplaySettingsDetailsWidget : public CWidgetBase, public Ui::display_settings_details_widget
{
	Q_OBJECT
public:
	CDisplaySettingsDetailsWidget( QWidget *parent = NULL );
	virtual ~CDisplaySettingsDetailsWidget();

	void load();
	void save();

private slots:
	void onLandscapeSliderChange( int value );
	void onCharactersSliderChange( int value );
	void onFXSliderChange( int value );
	void onTexturesSliderChange( int value );

private:
	/**
	 @brief  Looks up and returns the "quality" ( see the enums on the top), that belongs to the specified float value.
	 @param  variable  -  The config variable.
	 @return Returns the "quality" that best fits the specified value.
	*/
	int getQualityPresetFloat(const std::string &variable);

	/**
	 @brief  Looks up and returns the "quality" ( see the enums on the top), that belongs to the specified integer value.
	 @param  variable  -  The config variable.
	 @return Returns the "quality" that best fits the specified value.
	*/
	int getQualityPresetInteger(const std::string &variable);

	/**
	 @brief  Return the float value of the variable depending on the preset.
	 @param  variable  -  The config variable.
	 @param  preset  -  The preset to use (0-3).
	 @return Returns the float value.
	*/
	float getPresetFloat(const std::string &variable, sint preset);

	/**
	 @brief  Return the integer value of the variable depending on the preset.
	 @param  variable  -  The config variable.
	 @param  preset  -  The preset to use (0-3).
	 @return Returns the integer value.
	*/
	int getPresetInteger(const std::string &variable, sint preset);

	/**
	 @brief  Define the float value of the variable depending on the predefined preset.
	 @param  variable  -  The config variable.
	 @param  preset  -  The preset to use (0-3).
	*/
	void setFloatPreset(const std::string &variable, int preset);

	/**
	 @brief  Define the integer value of the variable depending on the predefined preset.
	 @param  variable  -  The config variable.
	 @param  preset  -  The preset to use (0-3).
	*/
	void setIntegerPreset(const std::string &variable, int preset);

	/**
	 @brief Retrieves the string that belongs to the specified quality.
	 @param quality  -  The quality value
	 @return Returns a string describing the quality value, Returns an empty string if an invalid value is specified.
	*/
	static QString getQualityString( uint32 quality );


	/**
	 @brief Retrieves the string that belongs to the specified texture quality.
	 @param quality  -  The texture quality value
	 @return Returns a string describing the texture quality, Returns an empty string if an invalid value is specified.
	*/
	static QString getTextureQualityString( uint32 quality );
};

#endif // DISPLAYSETTINGSDETAILSWIDGET_H
