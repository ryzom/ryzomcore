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



#ifndef CL_CONTINENT_MANAGER_H
#define CL_CONTINENT_MANAGER_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
// std.
#include <vector>
#include <map>
#include <string>
// client
#include "continent.h"
// client sheets
#include "client_sheets/world_sheet.h"

///////////
// CLASS //
///////////

namespace NLMISC
{
	class CVectorD;
	class CVector;
	class IProgressCallback;
}

namespace NLGEORGES
{
	class UFormElm;
}

class CContinent;
struct CFogState;

/**
 * Class to manage all continents.
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2001
 * \warning When you modify this class be sure _Current is always valid.
 */
class CContinentManager
{
public:
	/// Constructor
	CContinentManager();

	/// preload continent sheets
	void preloadSheets();

	/// Load & setup all continent.
	void load();

	// reset all datas
	void reset();

	/**
	 * Select continent from a name.
	 * \param const string &name : name of the continent to select.
	 */
	void select(const std::string &name, const NLMISC::CVectorD &pos, NLMISC::IProgressCallback &progress);
	/// Select closest continent from a vector.
	void select(const NLMISC::CVectorD &pos, NLMISC::IProgressCallback &progress);

	/** Test whether the next call to updateStreamable will be blocking.
	  * This happen for example when the player is too near of a village and when asynchronous loading is not sufficient.
	  * \param pos player position
	  */
	bool isLoadingforced(const NLMISC::CVector &playerPos) const;

	/** Given the position of the player, load / unload objects, asynchronously when possible (these are village for now)
	  */
	void updateStreamable(const NLMISC::CVector &playerPos);
	/** Given the position of the player, load / unload objects, but always in a synchronous fashion.
	  */
	void forceUpdateStreamable(const NLMISC::CVector &playerPos, NLMISC::IProgressCallback &progress);

	void removeVillages();

	// get fog
	void getFogState(TFogType fogType, float dayNight, float duskRatio, CLightCycleManager::TLightState lightState, const NLMISC::CVectorD &pos, CFogState &result);

	// Return a pointer on the current continent.
	CContinent	*cur() {return _Current;}

	// Return a pointer on the desired continent
	CContinent	*get(const std::string &contName);

	const std::string &getCurrentContinentSelectName();

	// load / saves all user landMarks
	void serialUserLandMarks(NLMISC::IStream &f);

	// rebuild visible landmarks on current map
	void updateUserLandMarks();

	// load / saves all fow maps
	void serialFOWMaps();

	void reloadWeather();

	void reloadSky();

	std::string getRegionNameByAlias(uint32 i);

protected:

	void loadContinentLandMarks();
	void readLMConts(const std::string &dataPath);

protected:

	typedef std::map<std::string, CContinent *> TContinents;

	/// Map with all continents.
	TContinents	_Continents;

	/// Current continent selected.
	CContinent	*_Current;
	CContinent	*_Hibernated;

	/// Map to find region name by alias
	std::map<uint32, std::string> aliasToRegionMap;

public:
	// World Map Info
	std::vector<CContLandMark> WorldMap;

};


#endif // CL_CONTINENT_MANAGER_H

/* End of continent_manager.h */
