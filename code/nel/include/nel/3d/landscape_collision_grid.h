// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_LANDSCAPE_COLLISION_GRID_H
#define NL_LANDSCAPE_COLLISION_GRID_H

#include "nel/misc/types_nl.h"
#include "nel/3d/patch.h"
#include "nel/3d/visual_collision_manager.h"
#include <vector>


namespace NL3D
{


class	CVisualCollisionManager;


// must be a power of 2.
#define NL_COLGRID_SIZE	32									// Size of the grid.
// NB: for speedup, one elt is always size of 2 meters.


// ***************************************************************************
/**
 * A grid of Tiles Id. Each CVisualCollisionEntity has such a grid. This looks like CQuadGrid, but with
 * special features.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeCollisionGrid
{
public:

	/// Constructor
	CLandscapeCollisionGrid(CVisualCollisionManager *owner);
	~CLandscapeCollisionGrid();


	/// clear the chainlist in the quadgrid.
	void			clear();
	/** Build the quadgrid with a array of patchblock. delta is the vector to apply to tiles coordinate,
	 * before insertion in the quadgrid (for precision consideration).
	 */
	void			build(const std::vector<CPatchQuadBlock*> &quadBlocks, const CVector &delta);


	/// select one entry in the chainQuad. pos is a position in World.
	CVisualTileDescNode			*select(const NLMISC::CVector &pos);


// ***********************
private:
	CVisualCollisionManager		*_Owner;


	/// Array of list of CVisualTileDescNode.
	bool						_Cleared;
	CVisualTileDescNode			*_Grid[NL_COLGRID_SIZE*NL_COLGRID_SIZE];
	uint						_SizePower;
	CVector						_Delta;


private:
	void			addQuadToGrid(uint16 paBlockId, uint16 quadId, sint minx, sint maxx, sint miny, sint maxy);


};


} // NL3D


#endif // NL_LANDSCAPE_COLLISION_GRID_H

/* End of landscape_collision_grid.h */
