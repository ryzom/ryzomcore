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



#ifndef RY_CONTINENT_SHEET
#define RY_CONTINENT_SHEET


#include "nel/misc/vector_2f.h"
//
#include "game_share/dir_light_setup.h"
#include "game_share/season.h"
#include "game_share/fog_map_build.h"
#include "entity_sheet.h"
//
#include "village_sheet.h"
#include "game_share/time_weather_season/weather_function_sheet.h"


// a location in a continent
/*class CLandMark
{
public:
	NLMISC::CVector2f Pos;
	std::string       TitleTextID; // should be converted with CI18N to get the actual title in ucstring
public:
	void build(const NLGEORGES::UFormElm &item);
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};*/

// Parameters common to continent image in the client and to its sheet
class CContinentParameters
{
public:
	// ctor/dtor
	CContinentParameters();
	virtual ~CContinentParameters() {}

	/// Name of the continent.
	std::string		Name;

	/// PACS RBank filename.
	std::string		PacsRBank;

	/// PACS GR filename.
	std::string		PacsGR;

	/// LandscapeIG filename.
	std::string		LandscapeIG;

	// New Sky system : gives name of the sky sheet per season. If present, this bypass the SkyDay & SkyNight fields
	std::string		SkySheet[EGSPD::CSeason::Invalid];

	/// SkyDay filename.
	std::string		SkyDay;

	/// SkyNight filename.
	std::string		SkyNight;

	// SkyFogPart filename
	std::string		SkyFogPartName;

	/// Background IG filename.
	std::string		BackgroundIGName;

	// Name of IGs for canopy, depending on season. If the string is empty, then the value in BackgroundIGName is used instead
	std::string     CanopyIGfileName[EGSPD::CSeason::Invalid];

	/// MicroVeg filename.
	std::string		MicroVeget;

	/// SmallBank filename.
	std::string		SmallBank;

	/// FarBank filename.
	std::string		FarBank;

	/// CoarseMeshMap filename.
	std::string		CoarseMeshMap;

	// corner zones
	std::string		ZoneMin;
	std::string		ZoneMax;

	/// Entity sun contribution power
	float			EntitySunContributionPower;

	/// Entity sun contribution max threshold
	float			EntitySunContributionMaxThreshold;

	// Lanscape - Day
	CDirLightSetup	LandscapeLightDay;

	// Lanscape - Dusk
	CDirLightSetup	LandscapeLightDusk;

	// Lanscape - Night
	CDirLightSetup	LandscapeLightNight;

	// Lanscape - PointLight Material.
	NLMISC::CRGBA	LandscapePointLightMaterial;

	// Entity - Day
	CDirLightSetup	EntityLightDay;

	// Entity - Dusk
	CDirLightSetup	EntityLightDusk;

	// Entity - Night
	CDirLightSetup	EntityLightNight;

	// Root - Day
	CDirLightSetup	RootLightDay;

	// Root - Dusk
	CDirLightSetup	RootLightDusk;

	// Root - Night
	CDirLightSetup	RootLightNight;

	// Zone constructible
	class CZC
	{
	public:
		std::string	Name;
		float       ForceLoadDist;
		float		LoadDist;
		float		UnloadDist;
		bool		EnableRuins;		// Allow the display of shape of ruins (else building shape displayed through bot objects)
		void serial (class NLMISC::IStream &f) throw (NLMISC::EStream);

		CZC()
		{
			ForceLoadDist = 0.0f;
			LoadDist = 0.0f;
			UnloadDist = 0.0f;
			EnableRuins = false;
		}
	};
	std::vector<CZC>	ZCList;

	// fog params
	CFogMapBuild			FogMapBuild;
	float					FogStart;
	float					FogEnd;
	float					RootFogStart;
	float					RootFogEnd;

	// Indoor continent ?
	bool					Indoor;

	// name of the texture for the world map
	std::string				WorldMap;

	// position of landmarks
	//std::vector<CLandMark>	LandMarks;

	// Localized name
	std::string				LocalizedName;

	// list of .primitive containing micro-life zones
	std::vector<std::string> MicroLifeZones;

	// Is the landscape tile colors on/off
	bool					TileColorMono[EGSPD::CSeason::Invalid];
	float					TileColorFactor[EGSPD::CSeason::Invalid];
	// Landscape lightmap factor
	float					StaticLightingFactor[EGSPD::CSeason::Invalid];

	// Mapping from Logical season, to visual season
	EGSPD::CSeason::TSeason	ForceDisplayedSeason[EGSPD::CSeason::Invalid];


	void build(const NLGEORGES::UFormElm &item);
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

private:

	void buildFogMapBuild(const NLGEORGES::UFormElm &item);
};

class CContinentSheet : public CEntitySheet
{
public:
	CContinentParameters Continent; // base parameters for continents
	// Villages
	std::vector<CVillageSheet> Villages;
	// Weather functions
	CWeatherFunctionSheet	   WeatherFunction[EGSPD::CSeason::Invalid];
public:
	// ctor
	CContinentSheet();
	// from CEntitySheet
	virtual void build(const NLGEORGES::UFormElm &item);
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

#endif
