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



#ifndef CL_FOG_MAP_H
#define CL_FOG_MAP_H

#include "nel/misc/bitmap.h"
#include "nel/misc/vector_2f.h"
//
#include "light_cycle_manager.h"
//
#include "game_share/fog_map_build.h"


namespace NL3D
{
	class UDriver;
}

// Fog state, can be setup in a driver
struct CFogState
{
	CFogState()	: FogEnabled(false)
	{
	}
	bool  FogEnabled;
	NLMISC::CRGBA FogColor;
	float FogStartDist;
	float FogEndDist;
	void setupInDriver(NL3D::UDriver &drv);
};


// A class that encodes fog color and distances
class CFogMap
{
public:
	typedef CFogMapBuild::TMapType TMapType; // enumeration of various fog map
public:
	// default ctor
	CFogMap();
	/** Init the fog map from a descriptor, and load the maps
	  * NB : lookup are performed on file names
	  * \return true if the initialisation succeed
	  */
	bool			init(const CFogMapBuild &desc);
	// init with explicit corners
	bool			init(const CFogMapBuild &desc, const NLMISC::CVector &minCoord, const NLMISC::CVector &maxCoord);
	// Release datas
	void			release();
	/// Compute the fog parameters at the given position.
	void			getFogParams(float referenceStartDist, float referenceEndDist, float x, float y, float dayNight, float duskRatio, CLightCycleManager::TLightState lightState, CFogState &result);
	/** Get a map value with a world (x, y) position
	  * Default value is returned if no map was specified or if map wasn't loaded
	  */
	NLMISC::CRGBAF	getMapValue(TMapType type, float x, float y, NLMISC::CRGBAF defaultValue) const;
	//
	NLMISC::CBitmap	   &getMap(TMapType type) { nlassert(type < CFogMapBuild::NumMap); return _Map[type]; }
	const NLMISC::CVector2f &getMinCoord() const { return _MinCoord; }
	const NLMISC::CVector2f &getMaxCoord() const { return _MaxCoord; }
	// convert a wortld pos into a map pos
	void  worldPosToMapPos(float inX, float inY, float &outX, float &outY) const;
private:
	NLMISC::CVector2f	_MinCoord;
	NLMISC::CVector2f	_MaxCoord;
	NLMISC::CBitmap		_Map[CFogMapBuild::NumMap];
private:
	void				load(NLMISC::CBitmap &bm, const std::string &filename);
	NLMISC::CRGBAF		getMapValueFromMapCoord(TMapType type, float x, float y, NLMISC::CRGBAF defaultValue) const;
};

#endif




