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



#ifndef CL_MICRO_LIFE_MANAGER_H
#define CL_MICRO_LIFE_MANAGER_H

#include "timed_fx_manager.h"
//
#include "game_share/season.h"
//
#include "nel/3d/u_landscape.h"
#include "nel/misc/polygon.h"
#include "nel/misc/triangle.h"
//

class CFloraSheet;

namespace NLLIGO
{
	class CPrimVector;
}

/** Spawn/despawn microlife fx on the landscape. Those fx are created in the world editor using 'patatoid' primitives.
  * This manager look in which primitive the player is, and add/remove microlife with the given density as necessary.
  * Microlife FXs are a special case of timed fx, and this manager relies in fact on CTimedFXManager
  * \TODO The container used is basically a quadgrid tailored for polygons, so it may be good to put the container part in NL3D for reuse (test for water intersection are similar)
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 1/2004
  */

class CMicroLifeManager : public NL3D::ULandscapeTileCallback
{
public:
	// get the unique instance of that class
	static CMicroLifeManager &getInstance();
	/** Init the manager
	  * \param minCorner minimum coordinate of continent
	  * \param maxCorner maximum coordinate of continent
	  * \param cellSize size of grid cells used to test if a tile intersect a primitive (also limit precision of tests)
	  */
	void init(const NLMISC::CVector2f &minCorner, const NLMISC::CVector2f &maxCorner, float cellSize = 30.f);
	// build from a list of primitive names
	void build(const std::vector<std::string> &fileNames);
	// release all managed primitive / created fxs
	void release();
	// Dump grid as a tga file. For debug
	void dumpMLGrid(const std::string &filename);
	// render the primitive that contains microlife on screen as boxes (for debug)
	void renderMLZones(const NLMISC::CVector2f &camPos, float maxDist = 1000.f);
	#ifdef NL_DEBUG
		// render active tiles
		void renderActiveTiles();
	#endif
private:
	// infos for a zone polygon overlapping a given grid cell
	class CGridOverlapPolyInfo
	{
	public:
		NLMISC::CPolygon2D  Poly;
		bool				IsExcludePoly;  // this is an exclusion polygon
		bool				IsFullyCovered; // the grid cell is fully contained into that polygon so fast test can be performed
		const CFloraSheet	*Sheet;
		uint				PrimitiveIndex;
	public:
		bool operator < (const CGridOverlapPolyInfo &other) const;
		bool operator == (const CGridOverlapPolyInfo &other) const;
	};
	// a vector of CGridOverlapPolyInfo
	class CGridOverlapPolyInfoVector
	{
	public:
		std::vector<CGridOverlapPolyInfo> V;
	public:
		bool operator < (const CGridOverlapPolyInfoVector &other) const;
		bool operator == (const CGridOverlapPolyInfoVector &other) const;
	};
	// temporary grid used for build
	typedef std::vector<CGridOverlapPolyInfoVector>	 TBuildGrid;
	 // possible list of polygons that can overlap a grid cells
	typedef std::vector<CGridOverlapPolyInfoVector>  TPossibleOverlapingPolyLists;
	TPossibleOverlapingPolyLists					_PossibleOverlapPolyLists;
	/** A grid that cover the continent and that give the list of primitives polygons that cover each cell
	  * List of primitives are shared between each cell.
	  * Each cell contains an index into _PossibleOverlapPolyLists.
	  */
	std::vector<uint16>  _Grid;
	float				 _CellSize;		// size of a grid cell in meters
	uint				 _GridWidth;	// width of the grid (a number of cells)
	uint				 _GridHeight;	// height of the grid (a number of cells)
	NLMISC::CVector2f	 _MinCorner;
	NLMISC::CNoiseValue	 _Noise;
	//
	typedef CHashMap<uint64, CTimedFXManager::TFXGroupHandle> TTileIDToFX;
	TTileIDToFX			_ActiveFXs; // active fxs, sorted by tiles ids

#ifdef NL_DEBUG
		CHashMap<uint64, NL3D::CTileAddedInfo> _ActiveTiles;
		CHashMap<uint64, NL3D::CTileAddedInfo> _ActiveTilesWithFX;
#endif

private:
	// prevent construction from outside
	CMicroLifeManager();
	// from NL3D::ULandscapeTileCallback
	virtual void tileAdded(const NL3D::CTileAddedInfo &infos);
	virtual void tileRemoved(uint64 id);
	// add / remove a poly of a zone to a build grid. Poly coordinates are in world.
	void drawPolyInBuildGrid(const std::vector<NLLIGO::CPrimVector> &primPoly,
							 uint primitiveIndex,
						     TBuildGrid &buildGrid,
							 const CFloraSheet *sheet,
							 bool isExcludeTri);
	// pack the build grid so that each cell only contains index into a small set of poly list, instead of a full poly list
	// (a lot of cells share the same poly list)
	void packBuildGrid(TBuildGrid			&buildGrid,
					   TPossibleOverlapingPolyLists &possibleLists
					  );
	// add a primitive file in the build grid
	// the primitive file 'primitiveIndex' is incremented
	void addPrimitiveToBuildGrid(const std::string	&fileName, uint &primitiveIndex, TBuildGrid &buildGrid);
	//
	friend class CCleanFullyCoveredGridCellPred;
};


// for debug : boolean which indicates if micro_life zones should
#if !FINAL_VERSION
	extern bool DisplayMicroLifeZones;
#endif

#ifdef NL_DEBUG
	extern bool DisplayMicroLifeActiveTiles;
#endif



#endif
