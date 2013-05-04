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

#include "nel/3d/landscape_vegetable_block.h"
#include "nel/3d/vegetable_manager.h"
#include "nel/3d/vegetable_clip_block.h"
#include "nel/3d/vegetable_instance_group.h"
#include "nel/3d/patch.h"
#include "nel/3d/bezier_patch.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
/*
	Distances type squared.
*/
class	CLVBSqrDistLUT
{
public:
	static	float		Array[NL3D_VEGETABLE_BLOCK_NUMDIST+1];

	CLVBSqrDistLUT()
	{
		// 0, 10, 20, 30, 40, 50
		for(uint i=0;i<NL3D_VEGETABLE_BLOCK_NUMDIST+1;i++)
		{
			Array[i]= i*NL3D_VEGETABLE_BLOCK_ELTDIST;
			Array[i]= sqr(Array[i]);
		}
	}

};


CLVBSqrDistLUT	NL3D_InitSqrDistLUT;
float		CLVBSqrDistLUT::Array[NL3D_VEGETABLE_BLOCK_NUMDIST+1];


// ***************************************************************************
CLandscapeVegetableBlock::CLandscapeVegetableBlock()
{
	_VegetableClipBlock= NULL;
	_CurDistType= NL3D_VEGETABLE_BLOCK_NUMDIST;

	for(uint j=0;j<NL3D_TESSBLOCK_TILESIZE;j++)
	{
		_VegetableSortBlock[j]= NULL;
		for(uint i=0;i<NL3D_VEGETABLE_BLOCK_NUMDIST;i++)
		{
			_VegetableIG[j][i]= NULL;
		}
	}
}



// ***************************************************************************
void			CLandscapeVegetableBlock::init(const CVector &center, CVegetableManager *vegetManager,
	CVegetableClipBlock *vegetableClipBlock, CPatch *patch, uint ts, uint tt, CTessList<CLandscapeVegetableBlock> &vblist)
{
	nlassert(patch);
	_Center= center;
	_VegetableClipBlock= vegetableClipBlock;
	_Patch= patch;
	_Ts= uint8(ts);
	_Tt= uint8(tt);

	// Create the Vegetable SortBlocks
	sint	tms,tmt;
	// for all tiles
	nlassert(NL3D_TESSBLOCK_TILESIZE==4);
	for(tmt= _Tt; tmt<_Tt+2; tmt++)
	{
		for(tms= _Ts; tms<_Ts+2; tms++)
		{
			// compute approximate center of the tile.
			float	s= (tms + 0.5f) / _Patch->getOrderS();
			float	t= (tmt + 0.5f) / _Patch->getOrderT();
			CBezierPatch	*bpatch= _Patch->unpackIntoCache();
			CVector	center= bpatch->eval(s, t);

			// create the sortBlock. NB: very approximate SortBlock radius....
			_VegetableSortBlock[(tmt-_Tt)*2 + (tms-_Ts)]= vegetManager->createSortBlock(_VegetableClipBlock, center, NL3D_PATCH_TILE_RADIUS);
		}
	}

	// append to list.
	vblist.append(this);
}


// ***************************************************************************
void			CLandscapeVegetableBlock::release(CVegetableManager *vegeManager, CTessList<CLandscapeVegetableBlock> &vblist)
{
	// release all Igs, and all Sbs.
	for(uint j=0;j<NL3D_TESSBLOCK_TILESIZE;j++)
	{
		// release IGs first.
		for(uint i=0;i<NL3D_VEGETABLE_BLOCK_NUMDIST;i++)
		{
			if(_VegetableIG[j][i])
			{
				vegeManager->deleteIg(_VegetableIG[j][i]);
				_VegetableIG[j][i]= NULL;
			}
		}

		// release SB
		if(_VegetableSortBlock[j])
		{
			vegeManager->deleteSortBlock(_VegetableSortBlock[j]);
			_VegetableSortBlock[j]= NULL;
		}
	}

	// reset state.
	_CurDistType= NL3D_VEGETABLE_BLOCK_NUMDIST;

	// remove from list.
	vblist.remove(this);
}

// ***************************************************************************
void			CLandscapeVegetableBlock::update(const CVector &viewCenter, CVegetableManager *vegeManager)
{
	float	sqrDist= (viewCenter-_Center).sqrnorm();

	// compute new distance type. Incremental mode.
	uint	newDistType= _CurDistType;
	while(sqrDist<CLVBSqrDistLUT::Array[newDistType])
	{
		newDistType--;
	}
	while(newDistType<NL3D_VEGETABLE_BLOCK_NUMDIST && sqrDist>CLVBSqrDistLUT::Array[newDistType+1])
	{
		newDistType++;
	}
	/*
		NB: to test but may be better than
			newDistType= floor()(delta.norm() / NL3D_VEGETABLE_BLOCK_ELTDIST);
	*/


	// Change of distance type??
	if(newDistType!=_CurDistType)
	{
		// Erase or create IGs.
		if(newDistType>_CurDistType)
		{
			// For all tiles
			for(uint j=0;j<NL3D_TESSBLOCK_TILESIZE;j++)
			{
				// Erase no more needed Igs.
				for(uint i=_CurDistType; i<newDistType; i++)
				{
					if(_VegetableIG[j][i])
					{
						vegeManager->deleteIg(_VegetableIG[j][i]);
						_VegetableIG[j][i]= NULL;
					}
				}

				// update the sort block for this tile
				_VegetableSortBlock[j]->updateSortBlock(*vegeManager);
			}
		}
		else
		{
			// Create a context for creation.
			CLandscapeVegetableBlockCreateContext	ctx;
			ctx.init(_Patch, _Ts, _Tt);

			// create new Igs, for all tiles
			for(uint i=newDistType; i<_CurDistType; i++)
			{
				createVegetableIGForDistType(i, vegeManager, ctx);
			}

			// For all tiles
			for(uint j=0;j<NL3D_TESSBLOCK_TILESIZE;j++)
			{
				// update the sort block for this tile
				_VegetableSortBlock[j]->updateSortBlock(*vegeManager);
			}
		}

		// copy new dist type.
		_CurDistType= uint8(newDistType);
	}


}


// ***************************************************************************
void			CLandscapeVegetableBlock::createVegetableIGForDistType(uint i, CVegetableManager *vegeManager,
	CLandscapeVegetableBlockCreateContext &vbCreateCtx)
{
	// check
	nlassert(NL3D_TESSBLOCK_TILESIZE==4);
	nlassert(_VegetableIG[0][i]==NULL);
	nlassert(_VegetableIG[1][i]==NULL);
	nlassert(_VegetableIG[2][i]==NULL);
	nlassert(_VegetableIG[3][i]==NULL);

	// Create vegetables instances per tile_material.
	sint	tms,tmt;
	// for all tiles
	nlassert(NL3D_TESSBLOCK_TILESIZE==4);
	for(tmt= _Tt; tmt<_Tt+2; tmt++)
	{
		for(tms= _Ts; tms<_Ts+2; tms++)
		{
			uint	tileId= tms-_Ts + (tmt-_Tt)*2;

			// create the instance group in the good sortBlock
			CVegetableInstanceGroup		*vegetIg= vegeManager->createIg(_VegetableSortBlock[tileId]);
			_VegetableIG[tileId][i]= vegetIg;

			// generate
			_Patch->generateTileVegetable(vegetIg, i, tms, tmt, vbCreateCtx);

			// If the ig is empty, delete him. This optimize rendering because no useless ig are
			// tested for rendering. This speed up some 1/10 of ms...
			if(vegetIg->isEmpty())
			{
				vegeManager->deleteIg(vegetIg);
				_VegetableIG[tileId][i]= NULL;
			}

			// NB: vegtable SortBlock is updated in CLandscapeVegetableBlock::update() after
		}
	}

}


// ***************************************************************************
// ***************************************************************************
// CLandscapeVegetableIGCreateContext
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLandscapeVegetableBlockCreateContext::CLandscapeVegetableBlockCreateContext()
{
	_Patch= NULL;
}


// ***************************************************************************
void			CLandscapeVegetableBlockCreateContext::init(CPatch *patch, uint ts, uint tt)
{
	nlassert(patch);
	_Empty= true;
	_Patch= patch;
	_Ts= ts;
	_Tt= tt;
}


// ***************************************************************************
void			CLandscapeVegetableBlockCreateContext::eval(uint ts, uint tt, float x, float y, CVector &pos)
{
	nlassert(NL3D_TESSBLOCK_TILESIZE==4);

	// If never created, do it now
	// ==================
	if(_Empty)
	{
		// Eval Position and normals for the 9 vertices (around the 2x2 tiles)
		for(uint j=0; j<3;j++)
		{
			float	t= (float)(_Tt+j)/_Patch->getOrderT();
			for(uint i=0; i<3;i++)
			{
				float	s= (float)(_Ts+i)/_Patch->getOrderS();
				// eval position.
				// use computeVertex() and not bpatch->eval() because vegetables must follow the
				// noise (at least at tile precision...). It is slower but necessary.
				_Pos[j*3+i]= _Patch->computeVertex(s, t);
			}
		}

		// Ok, it's done
		_Empty= false;
	}


	// Eval, with simple bilinear
	// ==================
	nlassert(ts==_Ts || ts==_Ts+1);
	nlassert(tt==_Tt || tt==_Tt+1);
	uint	ds= ts-_Ts;
	uint	dt= tt-_Tt;
	// Indices.
	uint	v00= dt*3 + ds;
	uint	v10= v00 + 1;
	uint	v01= v00 + 3;
	uint	v11= v00 + 4;
	// BiLinearFactor.
	float	dxdy= (1-x)*(1-y);
	float	dx2dy= x*(1-y);
	float	dxdy2= (1-x)*y;
	float	dx2dy2= x*y;

	// Compute Pos.
	pos = _Pos[v00] * dxdy;
	pos+= _Pos[v10] * dx2dy;
	pos+= _Pos[v01] * dxdy2;
	pos+= _Pos[v11] * dx2dy2;
}


} // NL3D
