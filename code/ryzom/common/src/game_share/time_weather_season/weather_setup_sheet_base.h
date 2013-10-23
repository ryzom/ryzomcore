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



#ifndef RY_WEATHER_SETUP_SHEET_BASE_H
#define RY_WEATHER_SETUP_SHEET_BASE_H


#include "../fog_type.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/rgba.h"

namespace NLGEORGES
{
	class UForm;
	class UFormElm;
}

namespace NLMISC
{
	class CSheetId;
}

// state of weather, not including clouds
class CWeatherStateSheet
{
public:
	// Best aprox setup name when blended
	std::string BestSetupName;
	struct CFXInfos
	{
		std::string Name;
		float		Ratio;

		CFXInfos() { Ratio = 0.0f; }

		void		serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial(Name, Ratio);
		}
	};
	// Fog (main & canopy)
	float		  FogRatio;
	NLMISC::CRGBA FogColorDay;
	NLMISC::CRGBA FogColorDusk;
	NLMISC::CRGBA FogColorNight;
	float         FogNear[NumFogType];
	float		  FogFar[NumFogType];
	float		  FogGradientFactor; // factor for the fog gradient
	// Lighting
	float		  Lighting;
	/** Bg read from the sheet
 	  */
	std::string   DayBackground;
	std::string   DuskBackground;
	std::string   NightBackground;

	// Wind
	float		  WindIntensity;
	// FX
	std::vector<CFXInfos> FXInfos;
	// Thunder
	float		  ThunderIntensity;
	NLMISC::CRGBA ThunderColor;

	std::string   LocalizedName;

public:
	// default ctor
	CWeatherStateSheet();
	/** Build that weather state from a georges sheet
	  */
	void				 build(const NLGEORGES::UFormElm &item);
	//
	void				 serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

// state of weather, including clouds
class CCloudStateSheet
{
public:
	NLMISC::CRGBA AmbientDay;
	NLMISC::CRGBA DiffuseDay;
	NLMISC::CRGBA AmbientNight;
	NLMISC::CRGBA DiffuseNight;
	NLMISC::CRGBA AmbientDusk;
	NLMISC::CRGBA DiffuseDusk;
	uint32		  NumClouds;
	float		  DiffusionSpeed;
public:
	CCloudStateSheet();
	// Build from a georges sheet
	void				 build(const NLGEORGES::UFormElm &item);
	//
	void				serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};


/**
 * Class to manage weather setup sheets
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */
class CWeatherSetupSheetBase
{
public:
	CWeatherStateSheet WeatherState;
	CCloudStateSheet   CloudState;
	NLMISC::TStringId  SetupName;
public:
	// ctor
	CWeatherSetupSheetBase();
	// build from an external sheet
	void build(const NLGEORGES::UFormElm &item);
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	//
	void readGeorges (const NLGEORGES::UForm *form, const NLMISC::CSheetId &sheetId);
	void removed() const {}
	static uint32 getVersion() { return 0; }
};





#endif
