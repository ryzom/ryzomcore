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

#ifndef CL_WATER_MAP_H
#define CL_WATER_MAP_H


#include "nel/3d/u_scene.h"
//
#include "nel/misc/polygon.h"
#include "nel/misc/vector_2f.h"



/** Provides an easy way to know is there's water at a given point, and what is it height
  * NB : for now, only water surfaces that have been instanciated once yet are taken in account
  * so this is useful only for testing if a visible point is in water
  *
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2/2004
  */
class CWaterMap : public NL3D::IWaterSurfaceAddedCallback
{
public:
	// ctor
	CWaterMap();
	/** Init the map with its corners coordinates.
	  * NB : This register callback to the main scene to know when a water surface is added
	  */
	void init(const NLMISC::CVector2f &minCorner, const NLMISC::CVector2f &maxCorner, float cellSize = 20.f);
	void release();
	/** Test if there's water at the given position
	  * if so, 'height' is filled with the water height
	  */
	bool getWaterHeight(const NLMISC::CVector2f &pos, float &height, bool &splashEnabled);
	// Dump water map as a tga file. For debug.
	void dump(const std::string &filename);
	// render water map on screen with boxes (for debug)
	void render(const NLMISC::CVector2f &camPos, float maxDist = 100.f);
private:
	struct CWaterInfo
	{
		float				Height; // height of surface
		NLMISC::CPolygon2D	Shape;
		bool				SplashEnabled;
	};
	NLMISC::CVector2f		_MinCorner;
	NLMISC::CVector2f		_MaxCorner;
	typedef std::vector<CWaterInfo> TWaterInfoVect;         // a vector of water surfaces overlapping at the same point.
	TWaterInfoVect          _WaterInfoVect;                 // list of surfaces that have been added yet.
	typedef std::vector<uint16>     TWaterInfoIndexVect;	// a vector of water surface that overlap at the same point
	std::vector<TWaterInfoIndexVect>	_WaterInfoIndexVectVect; // a vector of vector of surface infos (if the list of referenced water surface for a given cell are the same, then the list is shared between the two cells
	                                                        // so this contains the possible lists of surfaces contained by a cell)
	std::vector<uint16>			_Grid;                      // for each point in space, gives an ID into _WaterInfoVectVect. This gives access to a water surfaces list.
	float						_CellSize;
	uint16						_Width;
	uint16						_Height;
public:
	// for CWaterMap
	virtual void waterSurfaceAdded(const NLMISC::CPolygon2D &shape, const NLMISC::CMatrix &worldMatrix, bool splashEnabled, bool usesSceneWaterenvmap);
	virtual void waterSurfaceRemoved(bool usesSceneWaterenvmap);
};


// for debug
extern bool DisplayWaterMap;

#endif
