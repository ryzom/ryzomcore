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



#ifndef CL_CONTINENT_H
#define CL_CONTINENT_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
// Game share
#include "game_share/season.h"
#include "game_share/dir_light_setup.h"
// Client sheets
#include "client_sheets/continent_sheet.h"
//
#include "streamable_entity_composite.h"
#include "fog_map.h"
#include "game_share/fog_of_war.h"
#include "game_share/fog_type.h"
#include "game_share/time_weather_season/weather_function.h"
#include "sky_material_setup.h"
#include "outpost.h"
//
#include "sky.h" // new style sky
//
#include "water_map.h"

#include "continent_manager_build.h"

struct IIGEnum;
struct IIGAdded;


///////////
// CLASS //
///////////
namespace NLMISC
{
	class CVectorD;
	class IProgressCallback;
}

namespace NL3D
{
	class UDriver;
}

struct CFogState;

// Landmark created by the user
class CUserLandMark
{

public:

	enum EUserLandMarkType
	{
		Misc = 0,
		Tribe,
		Bandit,
		Citizen,
		Fauna,
		FaunaExcel,
		FaunaSup,
		Forage,
		ForageExcel,
		ForageSup,
		Sap,
		Amber,
		Node,
		Fiber,
		Bark,
		Seed,
		Shell,
		Resin,
		Wood,
		Oil,
		Mission,
		Food,
		Construction,
		Goo,
		Insect,
		Kitin,
		Nocive,
		Preservative,
		Passage,
		Teleporter,
		UserLandMarkTypeCount
	};

	NLMISC::CVector2f	Pos; // Pos in local map
	ucstring			Title;
	uint8				Type;

	//User LandMarks Colors
	static NLMISC::CRGBA		_LandMarksColor[UserLandMarkTypeCount];

public:


	CUserLandMark()
	{
		Type = Misc;
	}

	/// Get user landmark color
	NLMISC::CRGBA	getColor () const;

	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(Pos, Title, Type);
	}

};

const uint	STANDARD_NUM_USER_LANDMARKS = 256; // not counting bonus landmarks


/**
 * Class to manage the fog of war over a continent
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date June 2004
 */
class CFogOfWar : public IFogOfWar
{

public:

	NL3D::UTextureMem *Tx; // Here store the real size

public:

	CFogOfWar()
	{
		Tx = NULL;
	}

	~CFogOfWar();

	void load(const std::string &contName);
	void save(const std::string &contName);

	// Implementation of IFogOfWar
	virtual uint8 *getData();
	virtual bool createData(sint16 w, sint16 h);
	virtual void explored(sint16 mapPosX, sint16 mapPosY);
	virtual sint16 getRealWidth();
	virtual sint16 getRealHeight();
};


/**
 * Class to manage a continent.
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2001
 */
class CContinent : public CContinentParameters
{
public:
	// name of the sheet used to create the continent
	std::string       SheetName;



	// backup setup of sky for day
	CSkyMaterialSetup DaySkySetup;

	// backup setup of sky for day
	CSkyMaterialSetup NightSkySetup;

	// Fog map
	CFogMap					FogMap;

	// Water map
	CWaterMap				WaterMap;

	// Selected season
	EGSPD::CSeason::TSeason		Season;

	// A group of village
	CStreamableEntityComposite	_Villages;

	// A group of outposts
	std::vector<COutpost>		_Outposts;

	// The current sky
	CSky                     CurrentSky;

	// Weather functions for each season
	CWeatherFunction	WeatherFunction[EGSPD::CSeason::Invalid];

	// user landmarks (saved)
	std::vector<CUserLandMark> UserLandMarks;

	// continent landmarks (not saved but static data)
	std::vector<CContLandMark> ContLandMarks;

	// Continent occupation zone
	NLLIGO::CPrimZone	Zone;

	// Center of the occupation zone
	NLMISC::CVector2f	ZoneCenter;

//	bool Newbie;

	CFogOfWar	FoW;

public:
	/// Constructor
	CContinent();

	/// setup the continent from a sheet (use the SheetName that MUST be set)
	void setup();

	/**
	 * Update global parameters like the texture for the micro veget.
	 */
	void select(const NLMISC::CVectorD &pos, NLMISC::IProgressCallback &progress, bool complete, bool unhibernate, EGSPD::CSeason::TSeason season);

	/// This will remove extra rsc used by the continent (fog maps ..)
	void unselect();

	/** Test whether the next call to updateStreamable will be blocking.
	  * This happen for example when the player is too near of a village and when asynchronous loading is not sufficient.
	  * \param pos player position
	  */
	bool isLoadingforced(const NLMISC::CVector &playerPos) const;

	/** Given the position of the player, load / unload objects (asynchronously when possible)
	  */
	void updateStreamable(const NLMISC::CVector &playerPos);

	/** Given the position of the player, load / unload objects (always in a synchronous fashion)
	  */
	void forceUpdateStreamable(const NLMISC::CVector &playerPos, NLMISC::IProgressCallback &progress);

	/** Remove all villages from the continents
	  */
	void removeVillages();

	// Get a outpost
	COutpost	*getOutpost (uint i);

	// get fog
	void getFogState(TFogType fogType, float dayNight, float duskRatio, CLightCycleManager::TLightState lightState, const NLMISC::CVectorD &pos, CFogState &result, bool overideByWeatherFogDist = true);



	/** Enum ig of the continent
	  * (for now, these are currently instanciated villages)
	  * \return false if the enumeration has been stopped
	  */
	bool		enumIGs(IIGEnum *callback);


	///\name ig added observers.
	//@{
		void registerObserver(IIGObserver *obs);
		void removeObserver(IIGObserver *obs);
		bool isObserver(IIGObserver *obs) const;
	//@}

	// load (or reload) the micro-life primitives
	void loadMicroLife();

	// init the water map
	void initWaterMap();

	/** get corners min / zone max
	  * \return true if success
	  */
	bool getCorners(NLMISC::CVector2f &cornerMin, NLMISC::CVector2f &cornerMax) const;

	// dump village loading zones in a bitmap
	void dumpVillagesLoadingZones(const std::string &filename);

	enum TChannel { ChannelR = 0, ChannelG = 1, ChannelB = 2, ChannelA = 3, ChannelRGBA };
	// dump fog map, blended with continent map, and with camera pos
	// If a single channel is used, then channelLookup is used pick up the final color
	void dumpFogMap(CFogMapBuild::TMapType mapType, const std::string &filename, TChannel channel = ChannelRGBA, const CRGBA channelLookup[256] = NULL);

	void reloadFogMap();

	// init release the sky
	void initSky();
	void releaseSky();

	/// Return the max number of user landmarks (standard + bonus ones)
	static uint getMaxNbUserLandMarks();

private:

	// Register outpost (collsions...)
	void		initOutpost();

	// Remove all outpost (collisions...)
	void		removeOutpost();

	// load the fog maps
	bool loadFogMaps();

	// Load the weather functions
	void loadWeatherFunctions(const NLGEORGES::UFormElm &item);

};


#endif // CL_CONTINENT_H

/* End of continent.h */
