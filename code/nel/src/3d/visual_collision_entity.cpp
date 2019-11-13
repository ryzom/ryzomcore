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

#include "nel/3d/u_visual_collision_entity.h"

#include "nel/3d/visual_collision_entity.h"
#include "nel/3d/landscape.h"
#include "nel/3d/dru.h"
#include "nel/3d/driver.h"
#include "nel/3d/tile_bank.h"
#include "nel/misc/hierarchical_timer.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// should be at least 2 meters.
const	float	CVisualCollisionEntity::BBoxRadius= 10;
const	float	CVisualCollisionEntity::BBoxRadiusZ= 20;
const	uint32	CVisualCollisionEntity::_StartPatchQuadBlockSize= 64;	// 256 octets per entity minimum.
vector<CPatchBlockIdent>		CVisualCollisionEntity::_TmpBlockIds;
vector<CPatchQuadBlock*>		CVisualCollisionEntity::_TmpPatchQuadBlocks;


// ***************************************************************************
CVisualCollisionEntity::CVisualCollisionEntity(CVisualCollisionManager *owner) : _LandscapeQuadGrid(owner)
{
	_Owner= owner;
	_PatchQuadBlocks.reserve(_StartPatchQuadBlockSize);

	_CurrentBBoxValidity.setHalfSize(CVector::Null);

	_GroundMode= true;
	_CeilMode= false;
	_SnapToRenderedTesselation= true;

	_LastGPTValid= false;
}


// ***************************************************************************
CVisualCollisionEntity::~CVisualCollisionEntity()
{
	// delete the _PatchQuadBlocks.
	for(sint i=0; i<(sint)_PatchQuadBlocks.size(); i++)
	{
		_Owner->deletePatchQuadBlock(_PatchQuadBlocks[i]);
	}
	_PatchQuadBlocks.clear();

	// delete the quadgrid.
	_LandscapeQuadGrid.clear();
}


// ***************************************************************************
bool		CVisualCollisionEntity::snapToGround(CVector &pos)
{
	CVector		normal;
	// Not optimized, but doesn't matter (one cross product is negligible).
	return snapToGround(pos, normal);
}


// ***************************************************************************
CTrianglePatch		*CVisualCollisionEntity::getPatchTriangleUnderUs(const CVector &pos, CVector &res)
{
	// verify if landscape (refptr) is here.
	if(_Owner->_Landscape==NULL)
	{
		_LastGPTValid= false;
		return NULL;
	}


	// Test GPT cache.
	//==================
	// If last call was valid, and if same pos (input or output), return cached information
	if(_LastGPTValid && (pos==_LastGPTPosInput || pos==_LastGPTPosOutput) )
	{
		// copy from cache.
		res= _LastGPTPosOutput;
		/* don't modify _LastGPTPosInput cache, for best cache behavior in all cases.
			1/ this is not necessary (here, if pos!=_LastGPTPosInput, THEN pos==_LastGPTPosOutput, no other possibilities)
			2/ it causes problems when getPatchTriangleUnderUs() is called randomly with the unsnapped or snapped position:
				1st Time: zin:9.0  =>  zout:9.5	cache fail (ok, 1st time...)
				2nd Time: zin:9.0  =>  zout:9.5	cache OK (_LastGPTPosInput.z==zin)
				3rd Time: zin:9.5  =>  zout:9.5	cache OK (_LastGPTPosOutput.z==zin)
				4th Time: zin:9.0  =>  zout:9.5	cache FAILS (_LastGPTPosInput.z= 9.5 => !=zin)
		*/
		// and return ptr on cache.
		return &_LastGPTTrianglePatch;
	}


	// update the cache of tile info near this position.
	// =================
	testComputeLandscape(pos);


	// find possible faces under the entity.
	// =================
	CVisualTileDescNode		*ptr= _LandscapeQuadGrid.select(pos);


	// find the better face under the entity.
	// =================
	float	sqrBestDist= sqr(1000.f);
	CVector	hit;
	// build the vertical ray.
	CVector		segP0= pos - CVector(0,0,100);
	CVector		segP1= pos + CVector(0,0,100);


	// triangles builded from this list.
	static	vector<CTrianglePatch>		testTriangles;
	// NB: not so many reallocation here, because static.
	testTriangles.clear();
	sint	bestTriangle= 0;


	// For all the faces in this quadgrid node.
	while(ptr)
	{
		// what is the quad block of this tile Id.
		sint				qbId= ptr->PatchQuadBlocId;
		nlassert(qbId>=0 && qbId<(sint)_PatchQuadBlocks.size());
		CPatchQuadBlock		&qb= *_PatchQuadBlocks[qbId];

		// Build the 2 triangles of this tile Id.
		sint	idStart= (sint)testTriangles.size();
		testTriangles.resize(idStart+2);
		qb.buildTileTriangles((uint8)ptr->QuadId, &testTriangles[idStart]);

		// Test the 2 triangles.
		for(sint i=0; i<2; i++)
		{
			CTrianglePatch	&tri= testTriangles[idStart+i];
			// test if the ray intersect.
			// NB: triangleIntersect() is faster than CTriangle::intersect().
			if(triangleIntersect(tri, segP0, segP1, hit))
			{
				// find the nearest triangle.
				float sqrdist= (hit-pos).sqrnorm();
				if(sqrdist<sqrBestDist)
				{
					bestTriangle= idStart+i;
					res= hit;
					sqrBestDist= sqrdist;
				}
			}
		}


		// Next in the list.
		ptr= ptr->Next;
	}


	// found ??
	if(sqrBestDist<sqr(1000))
	{
		// copy into cache
		_LastGPTValid= true;
		_LastGPTTrianglePatch= testTriangles[bestTriangle];
		_LastGPTPosInput= pos;
		_LastGPTPosOutput= res;
		// and return ptr on cache.
		return &_LastGPTTrianglePatch;
	}
	else
	{
		_LastGPTValid= false;
		return NULL;
	}

}


// ***************************************************************************
bool		CVisualCollisionEntity::snapToGround(CVector &pos, CVector &normal)
{
	// Get Patch Triangle Under Us
	CVector		res;
	CTrianglePatch	*tri= getPatchTriangleUnderUs(pos, res);

	// result. NB: if not found, dot not modify.
	if( tri )
	{
		if(_SnapToRenderedTesselation)
		{
			// snap the position to the nearest tesselation.
			pos= res;

			// snap the position to the current rendered tesselation.
			snapToLandscapeCurrentTesselation(pos, *tri);
		}
		else
		{
			// just snap to the accurate tile tesselation.
			pos= res;
		}


		// compute the normal.
		normal= (tri->V1-tri->V0)^(tri->V2-tri->V0);
		normal.normalize();

		return true;
	}
	else
		return false;
}


// ***************************************************************************
void		CVisualCollisionEntity::computeUvForPos(const CTrianglePatch &tri, const CVector &pos, CUV &uv)
{
	// compute UV gradients.
	CVector		Gu;
	CVector		Gv;
	tri.computeGradient(tri.Uv0.U, tri.Uv1.U, tri.Uv2.U, Gu);
	tri.computeGradient(tri.Uv0.V, tri.Uv1.V, tri.Uv2.V, Gv);
	// interpolate
	uv.U= tri.Uv0.U + Gu*(pos-tri.V0);
	uv.V= tri.Uv0.V + Gv*(pos-tri.V0);
}


// ***************************************************************************
void		CVisualCollisionEntity::snapToLandscapeCurrentTesselation(CVector &pos, const CTrianglePatch &tri)
{
	// compute UV for position.
	CUV		uv;
	computeUvForPos(tri, pos, uv);

	// Ask pos to landscape.
	CVector		posLand;
	posLand= _Owner->_Landscape->getTesselatedPos(tri.PatchId, uv);

	// just keep Z.
	pos.z= posLand.z;
}


// ***************************************************************************
// TestYoyo. For Precision problem.
/*static bool	testLine(CVector &p1, CVector &p0, const CVector &pos0)
{
	float	epsilon=	0.1f;

	float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
	float		norm;
	// Line p0-p1.
	a= -(p1.y-p0.y);
	b= (p1.x-p0.x);
	norm= sqrtf(sqr(a) + sqr(b));
	a/= norm;
	b/= norm;
	c= -(p0.x*a + p0.y*b);
	if( (a*pos0.x + b*pos0.y + c) < -epsilon)
		return false;
	else
		return true;
}*/


// ***************************************************************************
bool		CVisualCollisionEntity::triangleIntersect2DGround(CTriangle &tri, const CVector &pos0)
{
	CVector		&p0= tri.V0;
	CVector		&p1= tri.V1;
	CVector		&p2= tri.V2;

	// TestYoyo. Test for precision problems.
	/*if( testLine(p1, p0, pos0) && testLine(p2, p1, pos0) && testLine(p0, p2, pos0) )
	{
		nlinfo("Found Tri For Pos: %.07f, %.07f\n   P0: %.07f, %.07f\n   P1: %.07f, %.07f\n   P2: %.07f, %.07f",
			pos0.x, pos0.y, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
	}*/

	// Test if the face enclose the pos in X/Y plane.
	// NB: compute and using a BBox to do a rapid test is not a very good idea, since it will
	// add an overhead which is NOT negligeable compared to the following test.
	float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
	// Line p0-p1.
	a= -(p1.y-p0.y);
	b= (p1.x-p0.x);
	c= -(p0.x*a + p0.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;
	// Line p1-p2.
	a= -(p2.y-p1.y);
	b= (p2.x-p1.x);
	c= -(p1.x*a + p1.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;
	// Line p2-p0.
	a= -(p0.y-p2.y);
	b= (p0.x-p2.x);
	c= -(p2.x*a + p2.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;

	return true;
}

// ***************************************************************************
bool		CVisualCollisionEntity::triangleIntersect2DCeil(CTriangle &tri, const CVector &pos0)
{
	CVector		&p0= tri.V0;
	CVector		&p1= tri.V1;
	CVector		&p2= tri.V2;

	// Test if the face enclose the pos in X/Y plane.
	// NB: compute and using a BBox to do a rapid test is not a very good idea, since it will
	// add an overhead which is NOT negligeable compared to the following test.
	float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
	// Line p0-p1.
	a= -(p1.y-p0.y);
	b= (p1.x-p0.x);
	c= -(p0.x*a + p0.y*b);
	if( (a*pos0.x + b*pos0.y + c) > 0)	return false;
	// Line p1-p2.
	a= -(p2.y-p1.y);
	b= (p2.x-p1.x);
	c= -(p1.x*a + p1.y*b);
	if( (a*pos0.x + b*pos0.y + c) > 0)	return false;
	// Line p2-p0.
	a= -(p0.y-p2.y);
	b= (p0.x-p2.x);
	c= -(p2.x*a + p2.y*b);
	if( (a*pos0.x + b*pos0.y + c) > 0)	return false;

	return true;
}

// ***************************************************************************
bool		CVisualCollisionEntity::triangleIntersect(CTriangle &tri, const CVector &pos0, const CVector &pos1, CVector &hit)
{
	CVector		&p0= tri.V0;
	CVector		&p1= tri.V1;
	CVector		&p2= tri.V2;

	bool	ok= false;
	if( _GroundMode && triangleIntersect2DGround(tri, pos0) )
		ok= true;
	if(!ok && _CeilMode && triangleIntersect2DCeil(tri, pos0) )
		ok= true;
	if(!ok)
		return false;


	// Compute the possible height.
	CVector		tmp;
	// build the plane
	CPlane plane;
	plane.make (p0, p1, p2);
	// intersect the vertical line with the plane.
	tmp= plane.intersect(pos0, pos1);

	float		h= tmp.z;
	// Test if it would fit in the wanted field.
	if(h>pos1.z)	return false;
	if(h<pos0.z)	return false;

	// OK!!
	// For cache completness, ensure that X and Y don't move, take same XY than pos0
	hit.x= pos0.x;
	hit.y= pos0.y;
	hit.z= h;
	return true;
}

// ***************************************************************************
void		CVisualCollisionEntity::testComputeLandscape(const CVector &pos)
{
	// if new position is out of the bbox surounding the entity.
	if(_CurrentBBoxValidity.getHalfSize()==CVector::Null || !_CurrentBBoxValidity.include(pos))
	{
		// must recompute the data around the entity.
		doComputeLandscape(pos);
	}
}

// ***************************************************************************
void		CVisualCollisionEntity::doComputeLandscape(const CVector &pos)
{
	sint	i;

	// setup new bbox.
	//==================
	// compute the bbox which must includes the patchQuadBlocks
	CAABBox		bboxToIncludePatchs;
	bboxToIncludePatchs.setCenter(pos);
	bboxToIncludePatchs.setHalfSize(CVector(BBoxRadius, BBoxRadius, BBoxRadiusZ));
	// setup the _CurrentBBoxValidity with same values, but BBoxRadiusZ/2
	_CurrentBBoxValidity.setCenter(pos);
	_CurrentBBoxValidity.setHalfSize(CVector(BBoxRadius, BBoxRadius, BBoxRadiusZ/2));


	// Search landscape blocks which are in the bbox.
	//==================
	_Owner->_Landscape->buildPatchBlocksInBBox(bboxToIncludePatchs, _TmpBlockIds);



	// Recompute PatchQuadBlockcs.
	//==================
	// This parts try to keeps old patch blocks so they are not recomputed if they already here.

	// sort PatchBlockIdent.
	sort(_TmpBlockIds.begin(), _TmpBlockIds.end());

	// Copy old array of ptr (ptr copy only).
	_TmpPatchQuadBlocks= _PatchQuadBlocks;

	// allocate dest array.
	_PatchQuadBlocks.resize(_TmpBlockIds.size());

	// Traverse all current patchBlocks, deleting old ones no longer needed, and creating new ones.
	// this algorithm suppose both array are sorted.
	uint	iOld=0;
	// parse until dest is filled.
	for(i=0; i<(sint)_PatchQuadBlocks.size();)
	{
		// get requested new BlockIdent.
		CPatchBlockIdent	newBi= _TmpBlockIds[i];

		// get requested old BlockIdent in the array.
		bool				oldEnd= false;
		CPatchBlockIdent	oldBi;
		if(iOld==_TmpPatchQuadBlocks.size())
			oldEnd= true;
		else
			oldBi= _TmpPatchQuadBlocks[iOld]->PatchBlockId;

		// if no more old blocks, or if new Block is < than current, we must create a new block, and insert it.
		if(oldEnd || newBi < oldBi)
		{
			// allocate the patch block.
			_PatchQuadBlocks[i]= _Owner->newPatchQuadBlock();
			// fill the patch block.
			_PatchQuadBlocks[i]->PatchBlockId= _TmpBlockIds[i];
			_Owner->_Landscape->fillPatchQuadBlock(*_PatchQuadBlocks[i]);

			// next new patch block.
			i++;
		}
		// else if current new Block is same than old block, just copy ptr.
		else if(newBi==oldBi)
		{
			// just copy ptr with the old one.
			_PatchQuadBlocks[i]= _TmpPatchQuadBlocks[iOld];

			// next new and old patch block.
			i++;
			iOld++;
		}
		// else, this old block is no longer used, delete it.
		else
		{
			_Owner->deletePatchQuadBlock(_TmpPatchQuadBlocks[iOld]);
			// next old patch block.
			iOld++;
		}
	}
	// Here, must delete old blocks not yet processed.
	for(;iOld<_TmpPatchQuadBlocks.size(); iOld++)
	{
		_Owner->deletePatchQuadBlock(_TmpPatchQuadBlocks[iOld]);
	}
	_TmpPatchQuadBlocks.clear();


	// Compute the quadGrid.
	//==================

	// Compute a delta so elt position for CLandscapeCollisionGrid are positive, and so fastFloor() used will work.
	CVector		delta;
	// center the position on 0.
	// floor may be important for precision when the delta is applied.
	delta.x= (float)floor(-pos.x);
	delta.y= (float)floor(-pos.y);
	delta.z= 0;
	// We must always have positive values for patchBlocks vertices.
	float	val= (float)ceil(BBoxRadius + 256);
	// NB: 256 is a security. Because of size of patchs, a value of 32 at max should be sufficient (64 for bigger patch (gfx))
	// we are large because doesn't matter, the CLandscapeCollisionGrid tiles.
	delta.x+= val;
	delta.y+= val;

	// rebuild the quadGrid.
	_LandscapeQuadGrid.build(_PatchQuadBlocks, delta);

}


// ***************************************************************************
bool		CVisualCollisionEntity::getStaticLightSetup(NLMISC::CRGBA sunAmbient, const CVector &pos,
	std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient)
{
	// Get Patch Triangle Under Us
	CVector		res;
	CTrianglePatch	*tri= getPatchTriangleUnderUs(pos, res);

	// For now, no special ambient support on landscape => take sunAmbient
	localAmbient= sunAmbient;

	// result. NB: if not found, dot not modify.
	if( tri )
	{
		// compute UV for position.
		CUV		uv;
		computeUvForPos(*tri, pos, uv);

		// get the sunContribution
		sunContribution= _Owner->_Landscape->getLumel(tri->PatchId, uv);
		// see getStaticLightSetup.
		sunContribution= _Owner->_SunContributionLUT[sunContribution];

		// append any lights of interest.
		_Owner->_Landscape->appendTileLightInfluences(tri->PatchId, uv, pointLightList);

		return true;
	}
	else
	{
		// Suppose full Sun Contribution, and don't add any pointLight
		sunContribution= 255;

		return false;
	}
}


// ***************************************************************************
bool		CVisualCollisionEntity::getSurfaceInfo(const CVector &pos, CSurfaceInfo &surfaceInfo)
{
	H_AUTO( NL3D_CVisualCollisionEntity_getSurfaceInfo )
	// Get Patch Triangle Under Us
	CVector		res;
	CTrianglePatch	*tri= getPatchTriangleUnderUs(pos, res);

	// result. NB: if not found, dot not modify.
	if( tri )
	{
		// compute UV for position.
		CUV		uv;
		computeUvForPos(*tri, pos, uv);

		// get the tileelement
		CTileElement *tileElm = _Owner->_Landscape->getTileElement(tri->PatchId, uv);
		if (tileElm)
		{
			// Valid tile ?
			uint16 tileId = tileElm->Tile[0];
			if (tileId != NL_TILE_ELM_LAYER_EMPTY)
			{
				// The tilebank
				CTileBank &tileBank = _Owner->_Landscape->TileBank;

				// Get xref info for this tile
				int tileSet;
				int number;
				CTileBank::TTileType type;
				tileBank.getTileXRef ((int)tileId, tileSet, number, type);

				// Get the tileset from layer 0
				const CTileSet* tileSetPtr = tileBank.getTileSet (tileSet);

				// Fill the surface
				surfaceInfo.UserSurfaceData = tileSetPtr->SurfaceData;

				// Ok
				return true;
			}
		}
	}
	return false;
}


// ***************************************************************************
void		CVisualCollisionEntity::displayDebugGrid(IDriver &drv) const
{
	// static to not reallocate each frame
	static	CMaterial	mat;
	static	bool		inited= false;
	if(!inited)
	{
		inited= true;
		mat.initUnlit();
	}
	static	vector<CLine>	lineList;
	lineList.clear();

	// Build lines for all patch quad blocks.
	for(uint i=0;i<_PatchQuadBlocks.size();i++)
	{
		CPatchQuadBlock		&pqb= *_PatchQuadBlocks[i];
		// Parse all quads of this patch block.
		CPatchBlockIdent	&pbid= pqb.PatchBlockId;
		for(uint t= pbid.T0; t<pbid.T1; t++)
		{
			for(uint s= pbid.S0; s<pbid.S1; s++)
			{
				// compute quad coordinate in pqb.
				uint	sd0= (s-pbid.S0);
				uint	td0= (t-pbid.T0);
				uint	sd1= sd0+1;
				uint	td1= td0+1;
				// get 4 vertex coord of quad
				const CVector	&p0= pqb.Vertices[sd0 + td0*NL_PATCH_BLOCK_MAX_VERTEX];
				const CVector	&p1= pqb.Vertices[sd0 + td1*NL_PATCH_BLOCK_MAX_VERTEX];
				const CVector	&p2= pqb.Vertices[sd1 + td1*NL_PATCH_BLOCK_MAX_VERTEX];
				const CVector	&p3= pqb.Vertices[sd1 + td0*NL_PATCH_BLOCK_MAX_VERTEX];

				// build the 5 lines
				lineList.push_back(CLine(p0, p1));
				lineList.push_back(CLine(p1, p2));
				lineList.push_back(CLine(p2, p3));
				lineList.push_back(CLine(p3, p0));
				lineList.push_back(CLine(p0, p2));
			}
		}
	}

	// Display the lines.
	CDRU::drawLinesUnlit(lineList, mat, drv);
}



} // NL3D
