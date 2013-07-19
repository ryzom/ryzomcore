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
	~CDisplaySettingsDetailsWidget();

	void load();
	void save();

protected:
	void changeEvent( QEvent *event );

private slots:
	void onLandscapeSliderChange( int value );
	void onCharactersSliderChange( int value );
	void onFXSliderChange( int value );
	void onTexturesSliderChange( int value );

private:
	/**
	 @brief  Looks up and returns the "quality" ( see the enums on the top), that belongs to the specified value.
	 @param  table  -  The lookup table you want to use.
	 @param  value  -  The value that we want to look up.
	 @return Returns the "quality" that best fits the specified value.
	*/
	template< typename T >
	int getQuality( const T *table, T value )
	{
		if( table[ 0 ] < table[ QUALITY_STEP - 1 ] )
		{
			uint32 i = 0;
			while( ( i < QUALITY_STEP ) && ( table[ i ] < value ) )
				i++;
			return i;
		}
		else
		{
			uint32 i = 0;
			while( ( i < QUALITY_STEP ) && ( table[ i ] > value ) )
				i++;
			return i;
		}
	}


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


	///////////////////////// Landscape values ///////////////////////
	static const float qualityToZFar[ QUALITY_STEP ];
	static const float qualityToLandscapeTileNear[ QUALITY_STEP ];
	static const float qualityToLandscapeThreshold[ QUALITY_STEP ];
	static const float qualityToMicrovegetDensity[ QUALITY_STEP ];

	//////////////////////// Character values ////////////////////////
	static const sint32 qualityToSkinNbMaxPoly[ QUALITY_STEP ];
	static const sint32 qualityToNbMaxSkeletonNotCLod[ QUALITY_STEP ];
	static const float qualityToCharacterFarClip[ QUALITY_STEP ];

	/////////////////////// FX values ////////////////////////////////
	static const sint32 qualityToFxNbMaxPoly[ QUALITY_STEP ];

};

#endif // DISPLAYSETTINGSDETAILSWIDGET_H
