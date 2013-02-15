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

#include "std3d.h"

#include "nel/3d/landscape_collision_grid.h"
#include "nel/misc/fast_floor.h"
#include <algorithm>


using namespace std;
using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
CLandscapeCollisionGrid::CLandscapeCollisionGrid(CVisualCollisionManager *owner)
{
	_Owner= owner;
	// reset list to NULL.
	memset(_Grid, 0, NL_COLGRID_SIZE*NL_COLGRID_SIZE * sizeof(CVisualTileDescNode*));
	_Cleared= true;

	// sizepower.
	nlassert(isPowerOf2(NL_COLGRID_SIZE));
	_SizePower= getPowerOf2(NL_COLGRID_SIZE);
}

// ***************************************************************************
CLandscapeCollisionGrid::~CLandscapeCollisionGrid()
{
	clear();
}

// ***************************************************************************
void			CLandscapeCollisionGrid::clear()
{
	// already cleared? do nothing.
	if(_Cleared)
		return;

	// Parse all quads.
	sint	i;
	for(i=0;i<NL_COLGRID_SIZE*NL_COLGRID_SIZE;i++)
	{
		CVisualTileDescNode			*ptr, *next;
		ptr= _Grid[i];

		// delete list of node.
		while(ptr)
		{
			next= ptr->Next;
			_Owner->deleteVisualTileDescNode(ptr);
			ptr= next;
		}

		// reset root.
		_Grid[i]= NULL;
	}

	_Cleared= true;
}


// ***************************************************************************
struct	CVector2i
{
	sint	x,y;

};


// ***************************************************************************
void			CLandscapeCollisionGrid::build(const std::vector<CPatchQuadBlock*>	&quadBlocks, const CVector &delta)
{
	sint	x,y;
	static CVector2i	floorVals[NL_PATCH_BLOCK_MAX_VERTEX*NL_PATCH_BLOCK_MAX_VERTEX];

	// first clear
	clear();

	// init for fast floor.
	OptFastFloorBegin();

	// then fill.
	_Cleared= false;
	_Delta= delta;
	// parse all quad blocks.
	for(sint i=0; i<(sint)quadBlocks.size();i++)
	{
		CPatchQuadBlock		&qb= *quadBlocks[i];
		sint		lenS= qb.PatchBlockId.S1 - qb.PatchBlockId.S0;
		sint		lenT= qb.PatchBlockId.T1 - qb.PatchBlockId.T0;

		// First, floor all vertices of interest.
		for(y=0; y<lenT+1; y++)
		{
			for(x=0; x<lenS+1; x++)
			{
				sint	id= y*NL_PATCH_BLOCK_MAX_VERTEX + x;
				// Add delta, and floor to sint.
				floorVals[id].x= OptFastFloor(qb.Vertices[id].x + delta.x);
				floorVals[id].y= OptFastFloor(qb.Vertices[id].y + delta.y);
			}
		}

		// Then compute min max for all quads, and insert quad id in the quadGrid.
		for(y=0; y<lenT; y++)
		{
			for(x=0; x<lenS; x++)
			{
				sint	minx, maxx, miny, maxy;
				sint	id= y*NL_PATCH_BLOCK_MAX_VERTEX + x;
				// Compute min max of the 4 vertices.
				minx= floorVals[id].x; maxx= floorVals[id].x;
				miny= floorVals[id].y; maxy= floorVals[id].y;
				id++;
				minx= min(minx, floorVals[id].x); maxx= max(maxx, floorVals[id].x);
				miny= min(miny, floorVals[id].y); maxy= max(maxy, floorVals[id].y);
				id+= NL_PATCH_BLOCK_MAX_VERTEX;
				minx= min(minx, floorVals[id].x); maxx= max(maxx, floorVals[id].x);
				miny= min(miny, floorVals[id].y); maxy= max(maxy, floorVals[id].y);
				id--;
				minx= min(minx, floorVals[id].x); maxx= max(maxx, floorVals[id].x);
				miny= min(miny, floorVals[id].y); maxy= max(maxy, floorVals[id].y);

				// store minmax in the quad.
				sint	quadId= y*NL_PATCH_BLOCK_MAX_QUAD + x;
				addQuadToGrid(i, quadId, minx, maxx, miny, maxy);
			}
		}
	}

	// init for fast floor.
	OptFastFloorEnd();
}


// ***************************************************************************
void			CLandscapeCollisionGrid::addQuadToGrid(uint16 paBlockId, uint16 quadId, sint x0, sint x1, sint y0, sint y1)
{
	// coordinate should be positive.
	nlassert(x0>=0 && x1>=x0);
	nlassert(y0>=0 && y1>=y0);

	// first, transform coordinate (in meters) in quadgrid eltSize (ie 2 meters).
	x0= (x0>>1);		// floor().
	x1= (x1>>1) + 1;	// equivalent of ceil().
	y0= (y0>>1);		// floor().
	y1= (y1>>1) + 1;	// equivalent of ceil().

	// setup bounds in quadgrid coordinate.
	if(x1-x0>=NL_COLGRID_SIZE)
		x0=0, x1= NL_COLGRID_SIZE;
	else
	{
		x0&= NL_COLGRID_SIZE-1;
		x1&= NL_COLGRID_SIZE-1;
		if(x1<=x0)
			x1+=NL_COLGRID_SIZE;
	}
	if(y1-y0>=NL_COLGRID_SIZE)
		y0=0, y1= NL_COLGRID_SIZE;
	else
	{
		y0&= NL_COLGRID_SIZE-1;
		y1&= NL_COLGRID_SIZE-1;
		if(y1<=y0)
			y1+=NL_COLGRID_SIZE;
	}

	// fill all cases with element.
	sint	x,y;
	for(y= y0;y<y1;y++)
	{
		sint	xe,ye;
		ye= y &(NL_COLGRID_SIZE-1);
		for(x= x0;x<x1;x++)
		{
			xe= x &(NL_COLGRID_SIZE-1);
			// which case we add the element.
			sint gridId= (ye<<_SizePower)+xe;

			// allocate element.
			CVisualTileDescNode		*elt= _Owner->newVisualTileDescNode();

			// fill elt.
			elt->PatchQuadBlocId= paBlockId;
			elt->QuadId= quadId;

			// bind elt to the list.
			elt->Next= _Grid[gridId];
			_Grid[gridId]= elt;
		}
	}
}


// ***************************************************************************
CVisualTileDescNode			*CLandscapeCollisionGrid::select(const NLMISC::CVector &pos)
{
	// compute pos in the quadgrid.
	CVector		localPos;
	localPos= pos + _Delta;
	// cases are 2x2 meters.
	localPos/=2;

	// floor, bound in quadgrid coordinate.
	sint	x,y;
	x= (sint)floor(localPos.x);
	y= (sint)floor(localPos.y);
	x&= NL_COLGRID_SIZE-1;
	y&= NL_COLGRID_SIZE-1;

	return _Grid[y*NL_COLGRID_SIZE+x];
}



} // NL3D
