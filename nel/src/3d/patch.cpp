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


#include "nel/3d/patch.h"
#include "nel/3d/tessellation.h"
#include "nel/3d/bezier_patch.h"
#include "nel/3d/zone.h"
#include "nel/3d/landscape.h"
#include "nel/misc/vector.h"
#include "nel/misc/common.h"
#include "nel/3d/patchuv_locator.h"
#include "nel/3d/vegetable_manager.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/light_influence_interpolator.h"
#include "nel/3d/patchdlm_context.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/u_landscape.h"

using	namespace	std;
using	namespace	NLMISC;

// Define this to remove user color (debug)
// #define NEL_FORCE_NO_USER_COLOR

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
CBezierPatch	CPatch::CachePatch;
const CPatch	*CPatch::LastPatch= NULL;
uint32			CPatch::_Version=7;


// ***************************************************************************
CPatch::CPatch()
{
	Zone= NULL;
	OrderS=0;
	OrderT=0;
	Son0=NULL;
	Son1=NULL;
	TessBlockRefCount=0;
	NumRenderableFaces= 0;

	// for Pacs process. By default, false.
	ExcludeFromRefineAll= false;

	// Init Passes.
	// DO NOT FILL Patch here, because of operator= problem. do it in compile().
	// By default, RdrPasses are NULL.

	// To force computation of texture info on next preRender().
	Far0= -1;
	Far1= -1;

	// Default: not binded.
	_BindZoneNeighbor[0]= NULL;
	_BindZoneNeighbor[1]= NULL;
	_BindZoneNeighbor[2]= NULL;
	_BindZoneNeighbor[3]= NULL;
	NoiseRotation= 0;
	// No smooth by default.
	_CornerSmoothFlag= 0;

	// MasterBlock never clipped.
	MasterBlock.resetClip();

	// Init UL circular list to NULL (not compiled)
	_ULNearPrec= NULL;
	_ULNearNext= NULL;

	// Dynamic LightMap
	_DLMContext= NULL;
	_DLMContextRefCount= 0;

	// Render Passes
	_PatchRdrPassFar0= NULL;
	_NextRdrFar0= NULL;
	_PatchRdrPassFar1= NULL;
	_NextRdrFar1= NULL;
}
// ***************************************************************************
CPatch::~CPatch()
{
	release();
}

// ***************************************************************************
void			CPatch::release()
{
	if(Zone)
	{
		// First, delete the VB if the zone was removed while the patch is visible.
		if(!isRenderClipped())
		{
			// release VertexBuffer.
			deleteVBAndFaceVector();
			// Flag RenderClipped in Zone
			Zone->_PatchRenderClipped.set(PatchId, true);
		}

		// THIS PATCH MSUT BE UNBOUND FIRST!!!!!
		nlassert(Son0 && Son1);
		nlassert(Son0->isLeaf() && Son1->isLeaf());
		nlassert(Son0->FLeft == NULL);
		nlassert(Son0->FRight == NULL);
		nlassert(Son1->FLeft == NULL);
		nlassert(Son1->FRight == NULL);

		// Free renderPass of landscape, and maybe force computation of texture info on next preRender().
		// Must do it here, before deletion of Zone, OrderS/T etc...
		resetRenderFar();

		getLandscape()->deleteTessFace(Son0);
		getLandscape()->deleteTessFace(Son1);
		// Vertices are smartptr/deleted in zone.
	}

	// Flag the fact that this patch can't be rendered.
	OrderS=0;
	OrderT=0;
	Son0=NULL;
	Son1=NULL;
	clearTessBlocks();
	resetMasterBlock();
	// Flag RenderClipped in Zone
	if(Zone)
	{
		Zone->_PatchRenderClipped.set( PatchId, true);
		Zone->_PatchOldRenderClipped.set( PatchId, true);
	}

	// the pathc is uncompiled. must do it after clearTessBlocks(), because may use it
	// for vegetable manager, and for updateLighting
	Zone= NULL;

	// uncompile: reset UpdateLighting circular list to NULL.
	if(_ULNearPrec!= NULL)
	{
		// verify the patch is correctly unlinked from any ciruclar list.
		nlassert(_ULNearPrec==this && _ULNearNext==this);
	}
	_ULNearPrec= NULL;
	_ULNearNext= NULL;

	// DynamciLightMap: release the _DLMContext if still exist.
	if(_DLMContext)
		delete _DLMContext;
	// reset
	_DLMContext= NULL;
	_DLMContextRefCount= 0;
}


// ***************************************************************************
CBezierPatch	*CPatch::unpackIntoCache() const
{
	if(LastPatch!=this)
	{
		unpack(CachePatch);
		LastPatch=this;
	}
	return &CachePatch;
}
// ***************************************************************************
void			CPatch::unpack(CBezierPatch	&p) const
{
	sint	i;
	const	CVector	&bias= Zone->getPatchBias();
	float	scale= Zone->getPatchScale();

	for(i=0;i<4;i++)
		Vertices[i].unpack(p.Vertices[i], bias, scale);
	for(i=0;i<8;i++)
		Tangents[i].unpack(p.Tangents[i], bias, scale);
	for(i=0;i<4;i++)
		Interiors[i].unpack(p.Interiors[i], bias, scale);
}
// ***************************************************************************
void			CPatch::computeDefaultErrorSize()
{
	CBezierPatch	&p= *unpackIntoCache();
	CVector			&v0= p.Vertices[0];
	CVector			&v1= p.Vertices[1];
	CVector			&v2= p.Vertices[2];

	// \todo yoyo: TODO_NOISE: modulate this value with tangents (roundiness of patch), and with the displacement map.
	ErrorSize= ((v1 - v0)^(v2 - v0)).norm();

}



// ***************************************************************************
void			CPatch::buildBBoxFromBezierPatch(const CBezierPatch &p, CAABBox &ret) const
{
	// Because of the structure of CAABBox, extend() is not fast enough for us. first compute bmin, bmax,
	// then compute the bbox.
	CVector		bmin= p.Vertices[0];
	CVector		bmax= bmin;

	sint			i;
	for(i=0;i<4;i++)
	{
		bmin.minof(bmin, p.Vertices[i]);
		bmax.maxof(bmax, p.Vertices[i]);
	}
	for(i=0;i<8;i++)
	{
		bmin.minof(bmin, p.Tangents[i]);
		bmax.maxof(bmax, p.Tangents[i]);
	}
	for(i=0;i<4;i++)
	{
		bmin.minof(bmin, p.Interiors[i]);
		bmax.maxof(bmax, p.Interiors[i]);
	}

	// Modulate with the maximum displacement map (useful for patch clipping).
	static	CVector		vectorNoiseMax(NL3D_NOISE_MAX, NL3D_NOISE_MAX, NL3D_NOISE_MAX);
	bmin-= vectorNoiseMax;
	bmax+= vectorNoiseMax;
	// NB: this is not very optimal, since the BBox may be very too big. eg: patch 16mx16m => bbox 18mx18m.
	// But remind that tessblocks do not use this BBox, and are computed with the real geometry.

	ret.setMinMax(bmin, bmax);
}


// ***************************************************************************
CAABBox			CPatch::buildBBox() const
{
	CBezierPatch	&p= *unpackIntoCache();

	// Compute Bounding Box. (easiest way...)
	CAABBox		ret;
	buildBBoxFromBezierPatch(p, ret);

	return ret;
}


// ***************************************************************************
void		CPatch::addTrianglesInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel) const
{
	CBezierPatch	&bpatch= *unpackIntoCache();

	// call with the whole root patch.
	addTrianglesInBBoxRecurs(paId, bbox, triangles, tileTessLevel, bpatch, 0, OrderS, 0, OrderT);
}


// ***************************************************************************
void		CPatch::addTrianglesInBBoxRecurs(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tessLevel,
		const CBezierPatch &pa, uint8 s0, uint8 s1, uint8 t0, uint8 t1) const
{
	uint8	lenS=s1-s0, lenT=t1-t0;
	nlassert(lenS>0);
	nlassert(lenT>0);

	// compute and compare bbox of the subdivision patch against request bbox.
	//========================
	// NB: this compute includes possible noise.
	CAABBox		paBBox;
	buildBBoxFromBezierPatch(pa, paBBox);
	// if do not intersect, stop here.
	if( !paBBox.intersect(bbox) )
		return;
	// else if at tile level, then just computeTriangles.
	//========================
	else if( lenS==1 && lenT==1 )
	{
		addTileTrianglesInBBox(paId, bbox, triangles, tessLevel, s0, t0);
	}
	// else subdiv and reccurs.
	//========================
	else
	{
		// Subdivide along the bigger side.
		if(lenS>lenT)
		{
			// subdivide.
			CBezierPatch	left, right;
			pa.subdivideS(left, right);
			uint8	sMiddle= (uint8)( ((uint)s0+(uint)s1) /2 );
			// recurs left.
			addTrianglesInBBoxRecurs(paId, bbox, triangles, tessLevel, left, s0, sMiddle, t0, t1);
			// recurs right.
			addTrianglesInBBoxRecurs(paId, bbox, triangles, tessLevel, right, sMiddle, s1, t0, t1);
		}
		else
		{
			// subdivide.
			CBezierPatch	top, bottom;
			pa.subdivideT(top, bottom);
			uint8	tMiddle= (uint8)( ((uint)t0+(uint)t1) /2 );
			// recurs top.
			addTrianglesInBBoxRecurs(paId, bbox, triangles, tessLevel, top, s0, s1, t0, tMiddle);
			// recurs bottom.
			addTrianglesInBBoxRecurs(paId, bbox, triangles, tessLevel, bottom, s0, s1, tMiddle, t1);
		}
	}


}


// ***************************************************************************
void		CPatch::addTileTrianglesInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tessLevel, uint8 s0, uint8 t0) const
{
	nlassert(s0<OrderS);
	nlassert(t0<OrderT);
	nlassert(tessLevel<=2);
	uint	tessLen= 1<<tessLevel;

	// some preca.
	float	startS0= (float)s0 / (float)(OrderS);
	float	startT0= (float)t0 / (float)(OrderT);
	float	ds= 1.0f/(float)(OrderS*tessLen);
	float	dt= 1.0f/(float)(OrderT*tessLen);

	// Parse all quads.
	uint	sl,tl;
	for(tl=0; tl<tessLen; tl++)
	{
		float	fs0, fs1, ft0, ft1;
		// compute t patch coordinates.
		ft0= startT0 + (float)tl * dt ;
		ft1= ft0 + dt;
		for(sl=0; sl<tessLen; sl++)
		{
			// compute s patch coordinates.
			fs0= startS0 + (float)sl * ds ;
			fs1= fs0 + ds;

			// Compute Quad vectors (in CCW).
			CVector		p0, p1, p2, p3;
			CUV			uv0, uv1, uv2, uv3;
			uv0.U= fs0; uv0.V= ft0;
			uv1.U= fs0; uv1.V= ft1;
			uv2.U= fs1; uv2.V= ft1;
			uv3.U= fs1; uv3.V= ft0;
			// evaluate patch (with noise). (NB: because of cache, patch decompression cost nothing).
			p0= computeVertex(uv0.U, uv0.V);
			p1= computeVertex(uv1.U, uv1.V);
			p2= computeVertex(uv2.U, uv2.V);
			p3= computeVertex(uv3.U, uv3.V);

			// build the bbox of this quad, and test with request bbox.
			CAABBox		quadBBox;
			quadBBox.setCenter(p0);
			quadBBox.extend(p1);
			quadBBox.extend(p2);
			quadBBox.extend(p3);

			// insert only if intersect with the bbox.
			if(quadBBox.intersect(bbox))
			{
				// build triangles (in CCW).
				CTrianglePatch	tri;
				tri.PatchId= paId;

				// first tri.
				tri.V0= p0; tri.V1= p1; tri.V2= p2;
				tri.Uv0= uv0; tri.Uv1= uv1; tri.Uv2= uv2;
				triangles.push_back(tri);

				// second tri.
				tri.V0= p2; tri.V1= p3; tri.V2= p0;
				tri.Uv0= uv2; tri.Uv1= uv3; tri.Uv2= uv0;
				triangles.push_back(tri);

				// NB: this is not the same tesselation than in tesselation.cpp.
				// But this looks like Ben's NLPACS::CLocalRetriever tesselation.
			}
		}
	}

}



// ***************************************************************************
void		CPatchQuadBlock::buildTileTriangles(uint8 quadId, CTrianglePatch  triangles[2]) const
{
	// copute coordinate of the tile we want.
	uint	sd0= quadId&(NL_PATCH_BLOCK_MAX_QUAD-1);
	uint	td0= quadId/(NL_PATCH_BLOCK_MAX_QUAD);
	uint	sd1= sd0+1;
	uint	td1= td0+1;
	uint	s= PatchBlockId.S0+sd0;
	uint	t= PatchBlockId.T0+td0;
	nlassert(s<PatchBlockId.S1);
	nlassert(t<PatchBlockId.T1);

	// Compute UV coord.
	float	fs0= (float)s / (float)(PatchBlockId.OrderS);
	float	ft0= (float)t / (float)(PatchBlockId.OrderT);
	float	fs1= (float)(s+1) / (float)(PatchBlockId.OrderS);
	float	ft1= (float)(t+1) / (float)(PatchBlockId.OrderT);
	CUV			uv0, uv1, uv2, uv3;
	uv0.U= fs0; uv0.V= ft0;
	uv1.U= fs0; uv1.V= ft1;
	uv2.U= fs1; uv2.V= ft1;
	uv3.U= fs1; uv3.V= ft0;

	// get vertex coord.
	const CVector	&p0= Vertices[sd0 + td0*NL_PATCH_BLOCK_MAX_VERTEX];
	const CVector	&p1= Vertices[sd0 + td1*NL_PATCH_BLOCK_MAX_VERTEX];
	const CVector	&p2= Vertices[sd1 + td1*NL_PATCH_BLOCK_MAX_VERTEX];
	const CVector	&p3= Vertices[sd1 + td0*NL_PATCH_BLOCK_MAX_VERTEX];

	// build triangles.
	// first tri.
	{
		CTrianglePatch &tri= triangles[0];
		tri.PatchId= PatchBlockId.PatchId;
		tri.V0= p0; tri.V1= p1; tri.V2= p2;
		tri.Uv0= uv0; tri.Uv1= uv1; tri.Uv2= uv2;
	}

	// second tri.
	{
		CTrianglePatch &tri= triangles[1];
		tri.PatchId= PatchBlockId.PatchId;
		tri.V0= p2; tri.V1= p3; tri.V2= p0;
		tri.Uv0= uv2; tri.Uv1= uv3; tri.Uv2= uv0;
	}

}


// ***************************************************************************
void		CPatch::fillPatchQuadBlock(CPatchQuadBlock &quadBlock)  const
{
	CPatchBlockIdent	&pbId= quadBlock.PatchBlockId;
	uint	lenS= pbId.S1-pbId.S0;
	uint	lenT= pbId.T1-pbId.T0;
	nlassert( pbId.OrderS==OrderS );
	nlassert( pbId.OrderT==OrderT );
	nlassert( pbId.S1<=OrderS );
	nlassert( pbId.T1<=OrderT );
	nlassert( pbId.S0<pbId.S1 );
	nlassert( pbId.T0<pbId.T1 );
	nlassert( lenS<=NL_PATCH_BLOCK_MAX_QUAD );
	nlassert( lenT<=NL_PATCH_BLOCK_MAX_QUAD );

	// Fill vertices.
	uint	s0= pbId.S0;
	uint	t0= pbId.T0;
	// some preca.
	float	startS0= (float)s0 / (float)(OrderS);
	float	startT0= (float)t0 / (float)(OrderT);
	float	ds= 1.0f/(float)(OrderS);
	float	dt= 1.0f/(float)(OrderT);

	// Parse all quads vertices corner.
	uint	sl,tl;
	for(tl=0; tl<lenT+1; tl++)
	{
		float	fs, ft;
		// compute t patch coordinates.
		ft= startT0 + (float)tl * dt ;
		for(sl=0; sl<lenS+1; sl++)
		{
			// compute s patch coordinates.
			fs= startS0 + (float)sl * ds ;

			// Must use computeContinousVertex, to ensure continous coordinate on patch edges
			quadBlock.Vertices[sl + tl*NL_PATCH_BLOCK_MAX_VERTEX]= computeContinousVertex(fs, ft);
		}
	}

}


// ***************************************************************************
void		CPatch::addPatchBlocksInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds) const
{
	CBezierPatch	&bpatch= *unpackIntoCache();

	// call with the whole root patch.
	addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, bpatch, 0, OrderS, 0, OrderT);
}


// ***************************************************************************
void		CPatch::addPatchBlocksInBBoxRecurs(CPatchIdent paId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds,
		const CBezierPatch &pa, uint8 s0, uint8 s1, uint8 t0, uint8 t1) const
{
	uint8	lenS=s1-s0, lenT=t1-t0;
	nlassert(lenS>0);
	nlassert(lenT>0);

	// compute and compare bbox of the subdivision patch against request bbox.
	//========================
	// NB: this compute includes possible noise.
	CAABBox		paBBox;
	buildBBoxFromBezierPatch(pa, paBBox);
	// if do not intersect, stop here.
	if( !paBBox.intersect(bbox) )
		return;
	// else if at CPatchQuadBlock tile level, then just add this Id.
	//========================
	else if( lenS<=NL_PATCH_BLOCK_MAX_QUAD && lenT<=NL_PATCH_BLOCK_MAX_QUAD )
	{
		// Add this PatchBlock desctiptor to the list.
		CPatchBlockIdent	pbId;
		// Fill struct from this and result of recursion.
		pbId.PatchId= paId;
		pbId.OrderS= OrderS;
		pbId.OrderT= OrderT;
		pbId.S0= s0;
		pbId.S1= s1;
		pbId.T0= t0;
		pbId.T1= t1;
		// Add to list.
		paBlockIds.push_back(pbId);
	}
	// else subdiv and reccurs.
	//========================
	else
	{
		// Subdivide along the bigger side.
		if(lenS>lenT)
		{
			// subdivide.
			CBezierPatch	left, right;
			pa.subdivideS(left, right);
			uint8	sMiddle= (uint8)( ((uint)s0+(uint)s1) /2 );
			// recurs left.
			addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, left, s0, sMiddle, t0, t1);
			// recurs right.
			addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, right, sMiddle, s1, t0, t1);
		}
		else
		{
			// subdivide.
			CBezierPatch	top, bottom;
			pa.subdivideT(top, bottom);
			uint8	tMiddle= (uint8)( ((uint)t0+(uint)t1) /2 );
			// recurs top.
			addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, top, s0, s1, t0, tMiddle);
			// recurs bottom.
			addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, bottom, s0, s1, tMiddle, t1);
		}
	}

}


// ***************************************************************************
CVector		CPatch::getTesselatedPos(CUV uv) const
{
	// clamp values.
	clamp(uv.U, 0, 1);
	clamp(uv.V, 0, 1);
	// recurs down the 2 sons.
	CVector		ret= CVector::Null;
	Son0->getTesselatedPos(uv, true, ret);
	Son1->getTesselatedPos(uv, true, ret);

	return ret;
}


// ***************************************************************************
bool		CPatch::isRenderClipped() const
{
	if(Zone)
		return Zone->isPatchRenderClipped(PatchId);
	else
		return true;
}


// ***************************************************************************
// ***************************************************************************
// RENDER LIST.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CPatch::addRefTessBlocks()
{
	uint	i;

	TessBlockRefCount++;

	if (TessBlocks.empty())
	{
		// Allocate the tessblocks.
		//==========

		nlassert(NL3D_TESSBLOCK_TILESIZE==4);
		// A tessblock is 2*2 tiles.
		sint os= OrderS>>1;
		sint ot= OrderT>>1;
		nlassert(os>0);
		nlassert(ot>0);
		TessBlocks.resize(os*ot);
		// init all tessBlocks with the Patch ptr.
		for(i=0; i<TessBlocks.size(); i++)
			TessBlocks[i].init(this);


		// Vegetable management
		//==========
		CVegetableManager	*vegetableManager= getLandscape()->_VegetableManager;

		// Create ClipBlocks.
		uint	nTbPerCb= NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK;
		uint	wCB= (os + nTbPerCb-1) >> NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK_SHIFT;
		uint	hCB= (ot + nTbPerCb-1) >> NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK_SHIFT;
		VegetableClipBlocks.resize(wCB * hCB);
		// allocate ClipBlocks
		for(i=0; i<VegetableClipBlocks.size(); i++)
		{
			VegetableClipBlocks[i]= vegetableManager->createClipBlock();
		}


		// updateLighting management.
		//==========
		// append patch for Near updateLighting, since TessBlock lightmap may/will exist.
		getLandscape()->linkPatchToNearUL(this);

	}
}

// ***************************************************************************
void			CPatch::decRefTessBlocks()
{
	TessBlockRefCount--;
	// If no loinger need the tessblocks, delete them.
	if(TessBlockRefCount==0)
		clearTessBlocks();
	nlassert(TessBlockRefCount>=0);
}


// ***************************************************************************
void			CPatch::clearTessBlocks()
{
	uint	i;

	// Vegetable management
	//==========
	// if compiled.
	if(Zone)
	{
		CVegetableManager	*vegetableManager= getLandscape()->_VegetableManager;

		// delete still existing vegetable Igs.
		deleteAllVegetableIgs();

		// delete ClipBlocks.
		for(i=0; i<VegetableClipBlocks.size(); i++)
		{
			vegetableManager->deleteClipBlock(VegetableClipBlocks[i]);
		}
		contReset(VegetableClipBlocks);
	}


	// updateLighting management.
	//==========
	if(Zone)
	{
		// remove patch from Near updateLighting, since no more TessBlock lightmap can exist.
		getLandscape()->unlinkPatchFromNearUL(this);
	}


	// Delete TessBlocks
	//==========
	TessBlockRefCount=0;
	contReset(TessBlocks);
}


// ***************************************************************************
void			CPatch::resetMasterBlock()
{
	// We should be not visible so FaceVector no more exist.
	nlassert(isRenderClipped());

	MasterBlock.FarVertexList.clear();
	MasterBlock.FarFaceList.clear();
	NumRenderableFaces= 0;
	// no tiles should be here!!
	nlassert(MasterBlock.NearVertexList.size()==0);
}

// ***************************************************************************
uint			CPatch::getNumTessBlock(CTessFace *face)
{
	// To which tessBlocks link the face?
	// compute an approx middle of the face.
	CParamCoord	edgeMid(face->PVLeft, face->PVRight);
	CParamCoord	middle(edgeMid, face->PVBase);
	// Coordinate of the tessblock (2*2 a tile!! so the >>1).
	uint ts= ((uint)middle.S * (uint)(OrderS>>1)) / 0x8000;
	uint tt= ((uint)middle.T * (uint)(OrderT>>1)) / 0x8000;
	uint numtb= tt*(uint)(OrderS>>1) + ts;

	return numtb;
}


// ***************************************************************************
void			CPatch::getNumTessBlock(CParamCoord pc, TFarVertType &type, uint &numtb)
{
	uint	tboS= (uint)(OrderS>>1);
	uint	tboT= (uint)(OrderT>>1);

	// Coordinate of the tessblock (2*2 a tile!! so the >>1).
	uint ts= ((uint)pc.S * tboS) / 0x8000;
	uint tt= ((uint)pc.T * tboT) / 0x8000;
	numtb= tt*tboS + ts;

	bool	edgeS= (ts*0x8000) == ((uint)pc.S * tboS);
	bool	edgeT= (tt*0x8000) == ((uint)pc.T * tboT);

	// Does this vertex lies on a corner of a TessBlock?
	if(edgeS && edgeT)
		type= FVMasterBlock;
	// Does this vertex lies on a edge of a TessBlock?
	else if(edgeS || edgeT)
		type= FVTessBlockEdge;
	// Else it lies exclusively IN a TessBlock.
	else
		type= FVTessBlock;

}



// ***************************************************************************
void			CPatch::extendTessBlockWithEndPos(CTessFace *face)
{
	if(face->Level>=TessBlockLimitLevel)
	{
		// get the tessBlock of the face.
		uint	numtb= getNumTessBlock(face);

		// Must enlarge the BSphere of the tesblock!!
		TessBlocks[numtb].extendSphereFirst(face->VBase->EndPos);
		TessBlocks[numtb].extendSphereAdd(face->VLeft->EndPos);
		TessBlocks[numtb].extendSphereAdd(face->VRight->EndPos);
		TessBlocks[numtb].extendSphereCompile();
	}
}


// ***************************************************************************
void			CPatch::dirtTessBlockFaceVector(CTessBlock &tb)
{
	// If patch is visible, block's faceVector should exist, but are no more valid.
	if(!isRenderClipped())
	{
		// If this tessBlock not already notified to modification.
		if(!tb.isInModifyList())
		{
			// Then append, and delete all FaceVector.
			// NB: delete FaceVector now, because the TessBlock himself may disapear soon.
			tb.appendToModifyListAndDeleteFaceVector(getLandscape()->_TessBlockModificationRoot, getLandscape()->_FaceVectorManager);
		}
	}
}


// ***************************************************************************
void			CPatch::appendFaceToRenderList(CTessFace *face)
{
	// Update the number of renderable Faces
	NumRenderableFaces++;

	// Update Gnal render.
	//====================
	if(face->Level<TessBlockLimitLevel)
	{
		MasterBlock.FarFaceList.append(face);
		MasterBlock.FaceTileMaterialRefCount++;

		// The facelist is modified, so we must update the faceVector, if visible.
		dirtTessBlockFaceVector(MasterBlock);
	}
	else
	{
		// Alloc if necessary the TessBlocks.
		addRefTessBlocks();

		// link the face to the good tessblock.
		uint	numtb= getNumTessBlock(face);
		TessBlocks[numtb].FarFaceList.append(face);
		TessBlocks[numtb].FaceTileMaterialRefCount++;

		// The facelist is modified, so we must update the faceVector, if visible.
		dirtTessBlockFaceVector(TessBlocks[numtb]);

		// Must enlarge the BSphere of the tesblock!!
		// We must do it on a per-face approach, because of tessblocks 's corners which are outside of tessblocks.
		TessBlocks[numtb].extendSphereFirst(face->VBase->EndPos);
		TessBlocks[numtb].extendSphereAdd(face->VLeft->EndPos);
		TessBlocks[numtb].extendSphereAdd(face->VRight->EndPos);
		// I think this should be done too on StartPos, for geomorph (rare??...) problems.
		// \todo yoyo: is this necessary???
		TessBlocks[numtb].extendSphereAdd(face->VBase->StartPos);
		TessBlocks[numtb].extendSphereAdd(face->VLeft->StartPos);
		TessBlocks[numtb].extendSphereAdd(face->VRight->StartPos);
		TessBlocks[numtb].extendSphereCompile();


		// Update Tile render (no need to do it if face not at least at tessblock level).
		appendFaceToTileRenderList(face);
	}
}


// ***************************************************************************
void			CPatch::removeFaceFromRenderList(CTessFace *face)
{
	// Update the number of renderable Faces
	NumRenderableFaces--;
	nlassert(NumRenderableFaces>=0);

	// Update Gnal render.
	//====================
	if(face->Level<TessBlockLimitLevel)
	{
		MasterBlock.FarFaceList.remove(face);
		MasterBlock.FaceTileMaterialRefCount--;

		// The facelist is modified, so we must update the faceVector, if visible.
		dirtTessBlockFaceVector(MasterBlock);
	}
	else
	{
		// link the face to the good tessblock.
		uint	numtb= getNumTessBlock(face);
		TessBlocks[numtb].FarFaceList.remove(face);
		TessBlocks[numtb].FaceTileMaterialRefCount--;

		// The facelist is modified, so we must update the faceVector, if visible.
		dirtTessBlockFaceVector(TessBlocks[numtb]);

		// Update Tile render (no need to do it if face not at least at tessblock level).
		removeFaceFromTileRenderList(face);

		// Destroy if necessary the TessBlocks.
		decRefTessBlocks();
	}
}


// ***************************************************************************
void			CPatch::appendFaceToTileRenderList(CTessFace *face)
{
	if(face->TileMaterial)
	{
		// For all valid faces, update their links.
		// Do not do this for lightmap, since it use same face from RGB0 pass.
		for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
		{
			CPatchRdrPass	*tilePass= face->TileMaterial->Pass[i].PatchRdrPass;
			// If tile i enabled.
			if(tilePass)
			{
				// a face should have created for this pass.
				nlassert(face->TileFaces[i]);
				face->TileMaterial->TileFaceList[i].append(face->TileFaces[i]);
			}
		}

		// The facelist is modified, so we must update the faceVector, if visible.
		uint	numtb= getNumTessBlock(face);
		dirtTessBlockFaceVector(TessBlocks[numtb]);

		// Shadow: append it to the Shadow Triangles.
		getLandscape()->appendToShadowPolyReceiver(face);
	}
}


// ***************************************************************************
void			CPatch::removeFaceFromTileRenderList(CTessFace *face)
{
	if(face->TileMaterial)
	{
		// For all valid faces, update their links.
		// Do not do this for lightmap, since it use same face from RGB0 pass.
		for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
		{
			CPatchRdrPass	*tilePass= face->TileMaterial->Pass[i].PatchRdrPass;
			// If tile i enabled.
			if(tilePass)
			{
				// a face should have created for this pass.
				nlassert(face->TileFaces[i]);
				face->TileMaterial->TileFaceList[i].remove(face->TileFaces[i]);
			}
		}

		// The facelist is modified, so we must update the faceVector, if visible.
		uint	numtb= getNumTessBlock(face);
		dirtTessBlockFaceVector(TessBlocks[numtb]);

		// Shadow: remove it from the Shadow Triangles.
		getLandscape()->removeFromShadowPolyReceiver(face);
	}
}


// ***************************************************************************
void			CPatch::computeTbTm(uint &numtb, uint &numtm, uint ts, uint tt)
{
	sint	is= ts&1;
	sint	it= tt&1;
	ts>>=1;
	tt>>=1;

	numtb= tt*(uint)(OrderS>>1) + ts;
	numtm= it*2+is;
}


// ***************************************************************************
void			CPatch::appendTileMaterialToRenderList(CTileMaterial *tm)
{
	nlassert(tm);

	// Alloc if necessary the TessBlocks.
	addRefTessBlocks();

	uint	numtb, numtm;
	computeTbTm(numtb, numtm, tm->TileS, tm->TileT);
	TessBlocks[numtb].RdrTileRoot[numtm]= tm;
	TessBlocks[numtb].FaceTileMaterialRefCount++;
	TessBlocks[numtb].TileMaterialRefCount++;
	// The master block contains the whole patch TileMaterialRefCount
	MasterBlock.TileMaterialRefCount++;

	// DynamicLighting. When in near, must compute the context, to have good UVs.
	//==========
	// inc ref to the context, creating it if needed.
	addRefDLMContext();
	// NB: do it before creating the vegetableBlock, because vegetables use DLM Uvs.

	// if was no tiles before in this tessBlock, create a Vegetable block.
	//==========
	// one Tile <=> was 0 before
	if( TessBlocks[numtb].TileMaterialRefCount == 1 && getLandscape()->isVegetableActive() )
	{
		createVegetableBlock(numtb, tm->TileS, tm->TileT);
	}
	const std::vector<ULandscapeTileCallback *> &tc = getLandscape()->getTileCallbacks();
	if (!tc.empty())
	{
		CBezierPatch	*bpatch= unpackIntoCache();
		CTileAddedInfo tai;
		//
		tai.Corners[0] = bpatch->eval((float) tm->TileS / OrderS, (float) tm->TileT / OrderT);
		tai.Corners[1] = bpatch->eval((tm-> TileS + 1.f) / OrderS, (float) tm->TileT / OrderT);
		tai.Corners[2] = bpatch->eval( (tm-> TileS + 1.f) / OrderS, (tm->TileT + 1.f) / OrderT);
		tai.Corners[3] = bpatch->eval( (float) tm-> TileS / OrderS, (tm->TileT + 1.f) / OrderT);
		tai.Center = bpatch->eval( (tm-> TileS + 0.5f) / OrderS, (tm->TileT + 0.5f) / OrderT);
		//
		tai.Normal = bpatch->evalNormal( (tm-> TileS + 0.5f) / OrderS, (tm->TileT + 0.5f) / OrderT);
		tai.TileID = (uint64) tm; // pointer to tile material serves as a unique identifier
		//
		for(std::vector<ULandscapeTileCallback *>::const_iterator it = tc.begin(); it != tc.end(); ++it)
		{
			(*it)->tileAdded(tai);
		}
	}
}
// ***************************************************************************
void			CPatch::removeTileMaterialFromRenderList(CTileMaterial *tm)
{
	nlassert(tm);

	uint	numtb, numtm;
	computeTbTm(numtb, numtm, tm->TileS, tm->TileT);
	TessBlocks[numtb].RdrTileRoot[numtm]= NULL;
	TessBlocks[numtb].FaceTileMaterialRefCount--;
	TessBlocks[numtb].TileMaterialRefCount--;
	// The master block contains the whole patch TileMaterialRefCount
	MasterBlock.TileMaterialRefCount--;

	// if no more tiles in this tessBlock, delete the vegetable Block.
	//==========
	// if no more tiles in this tessBlock
	if( TessBlocks[numtb].TileMaterialRefCount==0 )
	{
		// release the vegetableBlock (if any)
		releaseVegetableBlock(numtb);
	}

	// Destroy if necessary the TessBlocks.
	decRefTessBlocks();

	// DynamicLighting. When in near, must compute the context, to have good UVs.
	//==========
	// dec ref the context, deleting it if needed.
	decRefDLMContext();

	const std::vector<ULandscapeTileCallback *> &tc = getLandscape()->getTileCallbacks();
	if (!tc.empty())
	{
		//
		for(std::vector<ULandscapeTileCallback *>::const_iterator it = tc.begin(); it != tc.end(); ++it)
		{
			(*it)->tileRemoved((uint64) tm); // pointer to tile material serves as a unique identifier
		}
	}
}


// ***************************************************************************
void			CPatch::appendFarVertexToRenderList(CTessFarVertex *fv)
{
	TFarVertType	type;
	uint			numtb;
	getNumTessBlock(fv->PCoord, type, numtb);


	if(type==FVMasterBlock || type==FVTessBlockEdge)
	{
		fv->OwnerBlock= &MasterBlock;
		MasterBlock.FarVertexList.append(fv);
	}
	else
	{
		// Alloc if necessary the TessBlocks.
		addRefTessBlocks();

		fv->OwnerBlock= &TessBlocks[numtb];
		TessBlocks[numtb].FarVertexList.append(fv);
	}
}
// ***************************************************************************
void			CPatch::removeFarVertexFromRenderList(CTessFarVertex *fv)
{
	TFarVertType	type;
	uint			numtb;
	getNumTessBlock(fv->PCoord, type, numtb);


	if(type==FVMasterBlock || type==FVTessBlockEdge)
	{
		MasterBlock.FarVertexList.remove(fv);
		fv->OwnerBlock= NULL;
	}
	else
	{
		TessBlocks[numtb].FarVertexList.remove(fv);
		fv->OwnerBlock= NULL;

		// Destroy if necessary the TessBlocks.
		decRefTessBlocks();
	}
}


// ***************************************************************************
void			CPatch::appendNearVertexToRenderList(CTileMaterial *tileMat, CTessNearVertex *nv)
{
	nlassert(tileMat);

	// Alloc if necessary the TessBlocks.
	addRefTessBlocks();

	uint	numtb, numtm;
	computeTbTm(numtb, numtm, tileMat->TileS, tileMat->TileT);
	nv->OwnerBlock= &TessBlocks[numtb];
	TessBlocks[numtb].NearVertexList.append(nv);
}
// ***************************************************************************
void			CPatch::removeNearVertexFromRenderList(CTileMaterial *tileMat, CTessNearVertex *nv)
{
	nlassert(tileMat);

	uint	numtb, numtm;
	computeTbTm(numtb, numtm, tileMat->TileS, tileMat->TileT);
	TessBlocks[numtb].NearVertexList.remove(nv);
	nv->OwnerBlock= NULL;

	// Destroy if necessary the TessBlocks.
	decRefTessBlocks();
}



// ***************************************************************************
// ***************************************************************************
// BASIC BUILD.
// ***************************************************************************
// ***************************************************************************




// ***************************************************************************
void			CPatch::makeRoots()
{
	CTessVertex *a= BaseVertices[0];
	CTessVertex *b= BaseVertices[1];
	CTessVertex *c= BaseVertices[2];
	CTessVertex *d= BaseVertices[3];

	// Set positions.
	a->Pos= a->StartPos= a->EndPos= computeVertex(0,0);
	b->Pos= b->StartPos= b->EndPos= computeVertex(0,1);
	c->Pos= c->StartPos= c->EndPos= computeVertex(1,1);
	d->Pos= d->StartPos= d->EndPos= computeVertex(1,0);

	// Init Far vetices.
	CTessFarVertex *fa= &BaseFarVertices[0];
	CTessFarVertex *fb= &BaseFarVertices[1];
	CTessFarVertex *fc= &BaseFarVertices[2];
	CTessFarVertex *fd= &BaseFarVertices[3];
	fa->Src= a;
	fa->PCoord.setST(0,0);
	fb->Src= b;
	fb->PCoord.setST(0,1);
	fc->Src= c;
	fc->PCoord.setST(1,1);
	fd->Src= d;
	fd->PCoord.setST(1,0);

	// We don't have to fill the Far vertices VB here, because this patch is still not visible.
	// NB: we can't because we don't have any driver here.
	nlassert(isRenderClipped()==true);


	// Make Roots.
	/*
		Tesselation layout. For Square Face, and if OrderS>=OrderT.

		A-------D
		|\ Son1 |
		|  \    |
		|    \  |
		| Son0 \|
		B-------C

		For rectangles whith OrderT>OrderS. It is VERY IMPORTANT, for splitRectangular() reasons.

		A-------D
		| Son0 /|
		|    /  |
		|  /    |
		|/ Son1 |
		B-------C

	*/
	nlassert(Son0==NULL);
	nlassert(Son1==NULL);
	Son0= getLandscape()->newTessFace();
	Son1= getLandscape()->newTessFace();

	// Son0.
	Son0->Patch= this;
	Son0->Level= 0;
	if(OrderS>=OrderT)
	{
		Son0->VBase= b;
		Son0->VLeft= c;
		Son0->VRight= a;
		Son0->FVBase= fb;
		Son0->FVLeft= fc;
		Son0->FVRight= fa;
		Son0->PVBase.setST(0, 1);
		Son0->PVLeft.setST(1, 1);
		Son0->PVRight.setST(0, 0);
	}
	else
	{
		Son0->VBase= a;
		Son0->VLeft= b;
		Son0->VRight= d;
		Son0->FVBase= fa;
		Son0->FVLeft= fb;
		Son0->FVRight= fd;
		Son0->PVBase.setST(0, 0);
		Son0->PVLeft.setST(0, 1);
		Son0->PVRight.setST(1, 0);
	}
	Son0->FBase= Son1;
	Son0->FLeft= NULL;
	Son0->FRight= NULL;
	// No tile info.
	Son0->Size= ErrorSize/2;
	Son0->computeSplitPoint();

	// Son1.
	Son1->Patch= this;
	Son1->Level= 0;
	if(OrderS>=OrderT)
	{
		Son1->VBase= d;
		Son1->VLeft= a;
		Son1->VRight= c;
		Son1->FVBase= fd;
		Son1->FVLeft= fa;
		Son1->FVRight= fc;
		Son1->PVBase.setST(1, 0);
		Son1->PVLeft.setST(0, 0);
		Son1->PVRight.setST(1, 1);
	}
	else
	{
		Son1->VBase= c;
		Son1->VLeft= d;
		Son1->VRight= b;
		Son1->FVBase= fc;
		Son1->FVLeft= fd;
		Son1->FVRight= fb;
		Son1->PVBase.setST(1, 1);
		Son1->PVLeft.setST(1, 0);
		Son1->PVRight.setST(0, 1);
	}
	Son1->FBase= Son0;
	Son1->FLeft= NULL;
	Son1->FRight= NULL;
	// No tile info.
	Son1->Size= ErrorSize/2;
	Son1->computeSplitPoint();


	// Prepare the render list...
	clearTessBlocks();
	resetMasterBlock();
	appendFarVertexToRenderList(fa);
	appendFarVertexToRenderList(fb);
	appendFarVertexToRenderList(fc);
	appendFarVertexToRenderList(fd);
	appendFaceToRenderList(Son0);
	appendFaceToRenderList(Son1);

	// Useful for geomorph: Init 2 root faces MaxNearLimit, and MaxFaceSize
	// NB: since no geomorph is made on endpoints (StartPos==EndPos) of patchs, this is not useful.
	// but it is important to ensure the VP or software geomorph won't crash with bad float values.
	// Init MaxFaceSize.
	Son0->VBase->MaxFaceSize= 1;
	Son0->VLeft->MaxFaceSize= 1;
	Son0->VRight->MaxFaceSize= 1;
	Son1->VBase->MaxFaceSize= 1;
	Son1->VLeft->MaxFaceSize= 1;
	Son1->VRight->MaxFaceSize= 1;
	// Init MaxNearLimit.
	Son0->VBase->MaxNearLimit= 1;
	Son0->VLeft->MaxNearLimit= 1;
	Son0->VRight->MaxNearLimit= 1;
	Son1->VBase->MaxNearLimit= 1;
	Son1->VLeft->MaxNearLimit= 1;
	Son1->VRight->MaxNearLimit= 1;

}


// ***************************************************************************
void			CPatch::compile(CZone *z, uint patchId, uint8 orderS, uint8 orderT, CTessVertex *baseVertices[4], float errorSize)
{
	nlassert(z);
	Zone= z;

	// For updateLighting, get the correct pointer now.
	// Init UL circular list to me
	_ULNearPrec= this;
	_ULNearNext= this;


	// Once the patch is inserted and compiled in a zone, it is ready to be rendered.
	// init the MasterBlock.
	MasterBlock.init(this);

	// only 65536 patch per zone allowed.
	nlassert(patchId<0x10000);
	PatchId= (uint16)patchId;

	if(errorSize==0)
		computeDefaultErrorSize();
	else
		ErrorSize= errorSize;

	nlassert(orderS==2 || orderS==4 || orderS==8 || orderS==16);
	nlassert(orderT==2 || orderT==4 || orderT==8 || orderT==16);
	nlassert (OrderS==orderS);
	nlassert (OrderT==orderT);

	// Compile additional infos.
	sint	ps= getPowerOf2(orderS) , pt= getPowerOf2(orderT);
	sint	pmin= min(ps,pt);
	sint	pmax= max(ps,pt);
	// Rectangular patch OK.
	// Work, since patch 1xX are illegal. => The TileLimitLevel is at least 2 level distant from the time where
	// the rectangular patch is said "un-rectangular-ed" (tesselation looks like square). Hence, there is no problem
	// with rectangular UV geomorph (well don't bother me, make a draw :) ).
	TileLimitLevel= pmin*2 + pmax-pmin;
	// A TessBlock is a 2*2 tile. This simple formula works because patch 1xX are illegal.
	TessBlockLimitLevel= TileLimitLevel-2;
	// This tell us when the tess face is "un-rectangular-ed" (to say a square). Before, it is a "rectangular" face,
	// which has a strange fxxxxxg split.
	// If patch is square, then SquareLimitLevel=0 (ok!!).
	SquareLimitLevel= pmax-pmin;

	// Bind vertices, to zone base vertices.
	BaseVertices[0]= baseVertices[0];
	BaseVertices[1]= baseVertices[1];
	BaseVertices[2]= baseVertices[2];
	BaseVertices[3]= baseVertices[3];

	// build Sons.
	makeRoots();
}
// ***************************************************************************
CVector			CPatch::computeVertex(float s, float t) const
{
	// \todo yoyo: TODO_UVCORRECT: use UV correction.

	if(getLandscape()->getNoiseMode())
	{
		// compute displacement map to disturb result.
		CVector		displace;
		computeNoise(s,t, displace);

		// return patch(s,t) + dispalcement result.
		// unpack. Do it after computeNoise(), because this last may change the cache.
		CBezierPatch	*patch= unpackIntoCache();
		return patch->eval(s,t) + displace;
	}
	else
	{
		// unpack and return patch(s,t).
		CBezierPatch	*patch= unpackIntoCache();
		return patch->eval(s,t);
	}
}


// ***************************************************************************
CVector			CPatch::computeContinousVertex(float s, float t) const
{
	// must be compiled
	nlassert(Zone);

	// Test is on a edge/corner of the patch
	sint	edgeIdS= -1;
	sint	edgeIdT= -1;
	if(s==0)		edgeIdS= 0;
	else if(s==1)	edgeIdS= 2;
	if(t==0)		edgeIdT= 3;
	else if(t==1)	edgeIdT= 1;

	// test if on a corner
	if(edgeIdS>=0 && edgeIdT>=0)
	{
		// return the baseVertex according to edge falgs
		if(edgeIdS==0 && edgeIdT==3)	return BaseVertices[0]->EndPos;
		if(edgeIdS==0 && edgeIdT==1)	return BaseVertices[1]->EndPos;
		if(edgeIdS==2 && edgeIdT==1)	return BaseVertices[2]->EndPos;
		if(edgeIdS==2 && edgeIdT==3)	return BaseVertices[3]->EndPos;
		nlstop;
		// Error, but Avoid warning
		return CVector::Null;
	}
	// test if on an edge
	else if(edgeIdS>=0 || edgeIdT>=0)
	{
		// Yes, must compute myslef
		CVector		vertexOnMe= computeVertex(s,t);

		// Then, must compute my neighbor.
		sint	edgeId;
		if(edgeIdS>=0)
			edgeId= edgeIdS;
		else
			edgeId= edgeIdT;

		// Get my neighbor, if any.
		CBindInfo	bindInfo;
		getBindNeighbor(edgeId, bindInfo);
		// Fast reject: if no neighbor on the edge, just return my pos.
		if(!bindInfo.Zone)
		{
			return vertexOnMe;
		}
		// else must get vertex pos of my neighbor, and average.
		else
		{
			// use a CPatchUVLocator to get UV in neighbor patch
			CPatchUVLocator		uvLocator;
			uvLocator.build(this, edgeId, bindInfo);

			// UVlocator use OrderS/OrderT coordinate system.
			CVector2f	stTileIn, stTileOut;
			stTileIn.x= s * getOrderS();
			stTileIn.y= t * getOrderT();

			// transform coordinate to get into the neigbhor patch
			uint	pid= uvLocator.selectPatch(stTileIn);
			CPatch	*nebPatch;
			uvLocator.locateUV(stTileIn, pid, nebPatch, stTileOut);

			// transform neigbhor coord in 0..1 coordinate space.
			stTileOut.x/= nebPatch->getOrderS();
			stTileOut.y/= nebPatch->getOrderT();

			// and compute vertex. NB: if binded on 2 or 4 patch, it is then possible that stTileOut is on a
			// a corner ( (0,0), (0,1) ...).
			// In this case, we must use the precomputed Vertex pos, for completeness.
			bool		onCorner;
			CVector		vertexOnNeb= nebPatch->computeVertexButCorner(stTileOut.x, stTileOut.y, onCorner);

			// If the neighbor is on a corner, then use its corner value.
			if(onCorner)
				return vertexOnNeb;
			else
			{
				// Average the 2 and return this result.
				vertexOnMe+= vertexOnNeb;
				vertexOnMe/= 2;
				return vertexOnMe;
			}
		}

	}
	// else, std case
	else
		return computeVertex(s, t);
}


// ***************************************************************************
CVector			CPatch::computeVertexButCorner(float s, float t, bool &onCorner) const
{
	// must be compiled
	nlassert(Zone);

	// Test is on a edge/corner of the patch
	sint	edgeIdS= -1;
	sint	edgeIdT= -1;
	if(s==0)		edgeIdS= 0;
	else if(s==1)	edgeIdS= 2;
	if(t==0)		edgeIdT= 3;
	else if(t==1)	edgeIdT= 1;

	// test if on a corner
	if(edgeIdS>=0 && edgeIdT>=0)
	{
		// indicate that yes, we are on a corner
		onCorner= true;
		// return the baseVertex according to edge falgs
		if(edgeIdS==0 && edgeIdT==3)	return BaseVertices[0]->EndPos;
		if(edgeIdS==0 && edgeIdT==1)	return BaseVertices[1]->EndPos;
		if(edgeIdS==2 && edgeIdT==1)	return BaseVertices[2]->EndPos;
		if(edgeIdS==2 && edgeIdT==3)	return BaseVertices[3]->EndPos;
		nlstop;
		// Error, but Avoid warning
		return CVector::Null;
	}
	// else, std case
	else
	{
		onCorner= false;
		return computeVertex(s, t);
	}
}


// ***************************************************************************
void			CPatch::refineAll()
{
	// refineAll.
	nlassert(Son0);
	nlassert(Son1);
	Son0->refineAll();
	Son1->refineAll();
}



// ***************************************************************************
void			CPatch::averageTesselationVertices()
{
	nlassert(Son0);
	nlassert(Son1);

	// Recompute the BaseVertices. This is useful for Pacs.
	// Because CLandscape::averageTesselationVertices() is made on a strict order for patchs (map of zones, then
	// array of patchs), we are sure to overwrite BaseVertices in this order.
	CTessVertex *a= BaseVertices[0];
	CTessVertex *b= BaseVertices[1];
	CTessVertex *c= BaseVertices[2];
	CTessVertex *d= BaseVertices[3];
	// Set positions.
	a->Pos= a->StartPos= a->EndPos= computeVertex(0,0);
	b->Pos= b->StartPos= b->EndPos= computeVertex(0,1);
	c->Pos= c->StartPos= c->EndPos= computeVertex(1,1);
	d->Pos= d->StartPos= d->EndPos= computeVertex(1,0);


	// Average the tesselation of sons.
	Son0->averageTesselationVertices();
	Son1->averageTesselationVertices();
}


// ***************************************************************************
void			CPatch::refreshTesselationGeometry()
{
	nlassert(Son0);
	nlassert(Son1);

	// Recompute the BaseVertices.
	CTessVertex *a= BaseVertices[0];
	CTessVertex *b= BaseVertices[1];
	CTessVertex *c= BaseVertices[2];
	CTessVertex *d= BaseVertices[3];
	// Set positions.
	a->Pos= a->StartPos= a->EndPos= computeVertex(0,0);
	b->Pos= b->StartPos= b->EndPos= computeVertex(0,1);
	c->Pos= c->StartPos= c->EndPos= computeVertex(1,1);
	d->Pos= d->StartPos= d->EndPos= computeVertex(1,0);


	// refresh the tesselation of sons.
	Son0->refreshTesselationGeometry();
	Son1->refreshTesselationGeometry();
}


// ***************************************************************************
void			CPatch::resetRenderFar()
{
	if (_PatchRdrPassFar0)
	{
		// free the render pass.
		Zone->Landscape->freeFarRenderPass (this, _PatchRdrPassFar0, Far0);
		_PatchRdrPassFar0= NULL;
	}
	if (_PatchRdrPassFar1)
	{
		// free the render pass.
		Zone->Landscape->freeFarRenderPass (this, _PatchRdrPassFar1, Far1);
		_PatchRdrPassFar1= NULL;
	}

	Far0= -1;
	Far1= -1;
}


// ***************************************************************************
void			CPatch::serial(NLMISC::IStream &f)
{
	/*
	Version 7:
		- remove unused information from CTileColor. just keep 565 color
	Version 6:
		- default UnderWater flags for tileElements before version 6.
	Version 5:
		- TileLightInfluences serialized.
	Version 4:
		- Smooth flag serialized
	Version 3:
		- NoiseRotation.
		- NoiseSmooth.
	Version 2:
		- Lumels.
	Version 1:
		- Tile color.
	Version 0:
		- base version.
	*/
	uint	ver= f.serialVersion(_Version);

	// No more compatibility before version 2, because OrderS / OrderT not serialized in preceding version
	// Doens't matter since CZone::serial() do not support it too.
	if (ver<2)
	{
		throw EOlderStream(f);
	}

	f.xmlSerial (Vertices[0], Vertices[1], Vertices[2], Vertices[3], "VERTICIES");

	f.xmlPush ("TANGENTS");
	f.serial (Tangents[0], Tangents[1], Tangents[2], Tangents[3]);
	f.serial (Tangents[4], Tangents[5], Tangents[6], Tangents[7]);
	f.xmlPop ();

	f.xmlSerial (Interiors[0], Interiors[1], Interiors[2], Interiors[3], "INTERIORS");

	f.xmlPush ("TILES");
	f.serialCont(Tiles);
	f.xmlPop ();

	if(ver>=1)
	{
		// Read/Write TileColors.
		if(ver<=6)
		{
			nlassert(f.isReading());

			// read old version of tilesColors (ie with LightX/LightY/LightZ, which are deprecated now)
			vector<CTileColorOldPatchVersion6>	tmpArray;
			f.xmlPush ("TILE_COLORS");
			f.serialCont(tmpArray);
			f.xmlPop ();

			// then just copy to TileColors.
			TileColors.resize(tmpArray.size());
			if(!TileColors.empty())
			{
				memcpy(&TileColors[0], &tmpArray[0], TileColors.size()*sizeof(CTileColor));
			}
		}
		else
		{
			// version >=7, just serial array of TileColors (16 bits TileColor only)
			f.xmlPush ("TILE_COLORS");
			f.serialCont(TileColors);
#ifdef NEL_FORCE_NO_USER_COLOR
			CTileColor color;
			color.Color565 = 0xffff;
			uint size = TileColors.size ();
			TileColors.resize (0);
			TileColors.resize (size, color);
#endif // NEL_FORCE_NO_USER_COLOR
			f.xmlPop ();
		}
	}
	if(ver>=2)
	{
		f.xmlSerial (OrderS, "ORDER_S");
		f.xmlSerial (OrderT, "ORDER_T");

		f.xmlPush ("COMPRESSED_LUMELS");
			f.serialCont(CompressedLumels);
		f.xmlPop ();
	}
	// Else cannot create here the TileColors, because we need the OrderS/OrderT information... Done into CZone serial.
	if(ver>=3)
	{
		f.xmlSerial (NoiseRotation, "NOISE_ROTATION");
		f.xmlSerial (_CornerSmoothFlag, "CORNER_SMOOTH_FLAG");
	}
	else
	{
		// No Rotation / not smooth by default.
		NoiseRotation= 0;
		_CornerSmoothFlag= 0;
	}
	if(ver>=4)
	{
		f.xmlSerial(Flags, "FLAGS");
	}
	else
	{
		Flags=NL_PATCH_SMOOTH_FLAG_MASK;
	}

	// Serialize TileLightInfluences
	if(ver>=5)
	{
		f.xmlPush ("TILE_LIGHT_INFLUENCES");
		f.serialCont(TileLightInfluences);
		f.xmlPop ();
	}
	else
	{
		if(f.isReading())
		{
			// Fill default.
			resetTileLightInfluences();
		}
	}

	// if read a too old version,
	if(ver<6 && f.isReading())
	{
		// reset tileElements vegetableState to AboveWater.
		for(uint i=0; i<Tiles.size(); i++)
		{
			Tiles[i].setVegetableState(CTileElement::AboveWater);
		}
	}


}



// ***************************************************************************
// ***************************************************************************
// Bind / UnBind.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void			CPatch::unbind()
{
	nlassert(Son0 && Son1);

	Son0->unbind();
	Son1->unbind();
	Son0->forceMerge();
	Son1->forceMerge();
	// forcemerge should have be completed.
	nlassert(Son0->isLeaf() && Son1->isLeaf());
	// unbind should have be completed.
	nlassert(Son0->FLeft == NULL);
	nlassert(Son0->FRight == NULL);
	nlassert(Son1->FLeft == NULL);
	nlassert(Son1->FRight == NULL);


	// unbind Noise.
	_BindZoneNeighbor[0]= NULL;
	_BindZoneNeighbor[1]= NULL;
	_BindZoneNeighbor[2]= NULL;
	_BindZoneNeighbor[3]= NULL;

}


// ***************************************************************************
CTessFace		*CPatch::getRootFaceForEdge(sint edge) const
{
	nlassert(edge>=0 && edge<=3);

	// See tessellation rules.
	if(OrderS>=OrderT)
	{
		if(edge==0 || edge==1)
			return Son0;
		else
			return Son1;
	}
	else
	{
		if(edge==0 || edge==3)
			return Son0;
		else
			return Son1;
	}
}

// ***************************************************************************
CTessVertex		*CPatch::getRootVertexForEdge(sint edge) const
{
	// Return the vertex which is the start of edge.
	nlassert(edge>=0 && edge<=3);

	// See tessellation rules.
	if(OrderS>=OrderT)
	{
		switch(edge)
		{
			case 0: return Son0->VRight;
			case 1: return Son0->VBase;
			case 2: return Son0->VLeft;
			case 3: return Son1->VBase;
			default: return NULL;
		}
	}
	else
	{
		switch(edge)
		{
			case 0: return Son0->VBase;
			case 1: return Son0->VLeft;
			case 2: return Son1->VBase;
			case 3: return Son0->VRight;
			default: return NULL;
		}
	}
}


// ***************************************************************************
void			CPatch::changeEdgeNeighbor(sint edge, CTessFace *to)
{
	nlassert(edge>=0 && edge<=3);

	// See tessellation rules.
	if(OrderS>=OrderT)
	{
		switch(edge)
		{
			case 0: Son0->FRight= to; break;
			case 1: Son0->FLeft= to; break;
			case 2: Son1->FRight= to; break;
			case 3: Son1->FLeft= to; break;
		}
	}
	else
	{
		switch(edge)
		{
			case 0: Son0->FLeft= to; break;
			case 1: Son1->FRight= to; break;
			case 2: Son1->FLeft= to; break;
			case 3: Son0->FRight= to; break;
		}
	}
}


// ***************************************************************************
CTessFace		*CPatch::linkTessFaceWithEdge(const CVector2f &uv0, const CVector2f &uv1, CTessFace *linkTo)
{
	nlassert(Son0 && Son1);
	// Try to link with Root Son0
	CTessFace	*face= Son0->linkTessFaceWithEdge(uv0, uv1, linkTo);
	// if Failed Try to link with Root Son1
	if(!face)
		face= Son1->linkTessFaceWithEdge(uv0, uv1, linkTo);
	return face;
}


// ***************************************************************************
void			CPatch::bind(CBindInfo	Edges[4], bool rebind)
{
	// The multiple Patch Face.
	// By default, Patch==NULL, FLeft, FRight and FBase==NULL so ok!
	static	CTessFace	bind1_2[4];
	static	CTessFace	bind1_4[8];


	// THIS PATCH MUST BE UNBOUND FIRST!!!!!
	nlassert(Son0 && Son1);
	nlassert(Son0->isLeaf() && Son1->isLeaf());
	nlassert(Son0->FLeft == NULL);
	nlassert(Son0->FRight == NULL);
	nlassert(Son1->FLeft == NULL);
	nlassert(Son1->FRight == NULL);


	// bind the Realtime bind info here (before any computeVertex, and before bind too).
	sint	i;
	for(i=0;i<4;i++)
	{
		// just Copy zone (if not NULL).
		_BindZoneNeighbor[i]= Edges[i].Zone;
	}


	if(!rebind)
	{
		// Just recompute base vertices.
		CTessVertex *a= BaseVertices[0];
		CTessVertex *b= BaseVertices[1];
		CTessVertex *c= BaseVertices[2];
		CTessVertex *d= BaseVertices[3];
		// Set positions.
		a->Pos= a->StartPos= a->EndPos= computeVertex(0,0);
		b->Pos= b->StartPos= b->EndPos= computeVertex(0,1);
		c->Pos= c->StartPos= c->EndPos= computeVertex(1,1);
		d->Pos= d->StartPos= d->EndPos= computeVertex(1,0);
		// NB: no propagation problem, since the patch has root only (since has to be unbound!!!)
		// Recompute centers.
		Son0->computeSplitPoint();
		Son1->computeSplitPoint();
	}
	else
	{
		// Keep old Vertices as computed from neighbors, but reFill the 4 FarVertices.
		// NB: don't do it on NearVertices because suppose that Near is Off when a bind occurs (often far away).
		checkFillVertexVBFar(Son0->FVBase);
		checkFillVertexVBFar(Son0->FVLeft);
		checkFillVertexVBFar(Son0->FVRight);
		checkFillVertexVBFar(Son1->FVBase);
	}


	// Bind the roots.
	for(i=0;i<4;i++)
	{
		CBindInfo	&bind= Edges[i];

		nlassert(bind.NPatchs==0 || bind.NPatchs==1 || bind.NPatchs==2 || bind.NPatchs==4 || bind.NPatchs==5);
		if(bind.NPatchs==1)
		{
			// Bind me on Next.
			this->changeEdgeNeighbor(i, bind.Next[0]->getRootFaceForEdge(bind.Edge[0]));
			// Bind Next on me.
			bind.Next[0]->changeEdgeNeighbor(bind.Edge[0], this->getRootFaceForEdge(i));
		}
		else if(bind.NPatchs==2)
		{
			// Setup multiple bind.
			this->changeEdgeNeighbor(i, bind1_2+i);
			bind1_2[i].FBase= this->getRootFaceForEdge(i);
			// Setup the multiple face.
			// Follow the conventions! Make a draw for understand. Small Patchs are numbered in CCW.
			bind1_2[i].SonRight= bind.Next[0]->getRootFaceForEdge(bind.Edge[0]);
			bind1_2[i].SonLeft= bind.Next[1]->getRootFaceForEdge(bind.Edge[1]);
			bind1_2[i].VBase= bind.Next[0]->getRootVertexForEdge(bind.Edge[0]);
			// Set a "flag" to neighbors, so they know what edge is to be bind on me.
			bind.Next[0]->changeEdgeNeighbor(bind.Edge[0], &CTessFace::MultipleBindFace);
			bind.Next[1]->changeEdgeNeighbor(bind.Edge[1], &CTessFace::MultipleBindFace);
		}
		else if(bind.NPatchs==4)
		{
			// Setup multiple bind level 0.
			this->changeEdgeNeighbor(i, bind1_2+i);
			bind1_2[i].FBase= this->getRootFaceForEdge(i);

			// Setup multiple bind level 1.
			// Follow the conventions! Make a draw for understand. Small Patchs are numbered in CCW.
			bind1_2[i].SonRight= bind1_4 + 2*i+0;
			bind1_2[i].SonLeft= bind1_4 + 2*i+1;
			bind1_2[i].VBase= bind.Next[1]->getRootVertexForEdge(bind.Edge[1]);
			// Make first multiple face bind level1.
			bind1_4[2*i+0].FBase= &CTessFace::MultipleBindFace;	// to link correctly when the root face will be splitted.
			bind1_4[2*i+0].SonRight= bind.Next[0]->getRootFaceForEdge(bind.Edge[0]);
			bind1_4[2*i+0].SonLeft= bind.Next[1]->getRootFaceForEdge(bind.Edge[1]);
			bind1_4[2*i+0].VBase= bind.Next[0]->getRootVertexForEdge(bind.Edge[0]);
			// Make second multiple face bind level1.
			bind1_4[2*i+1].FBase= &CTessFace::MultipleBindFace;	// to link correctly when the root face will be splitted.
			bind1_4[2*i+1].SonRight= bind.Next[2]->getRootFaceForEdge(bind.Edge[2]);
			bind1_4[2*i+1].SonLeft= bind.Next[3]->getRootFaceForEdge(bind.Edge[3]);
			bind1_4[2*i+1].VBase= bind.Next[2]->getRootVertexForEdge(bind.Edge[2]);

			// Set a "flag" to neighbors, so they know what edge is to be bind on me.
			bind.Next[0]->changeEdgeNeighbor(bind.Edge[0], &CTessFace::MultipleBindFace);
			bind.Next[1]->changeEdgeNeighbor(bind.Edge[1], &CTessFace::MultipleBindFace);
			bind.Next[2]->changeEdgeNeighbor(bind.Edge[2], &CTessFace::MultipleBindFace);
			bind.Next[3]->changeEdgeNeighbor(bind.Edge[3], &CTessFace::MultipleBindFace);
		}
		else if(bind.NPatchs==5)
		{
			/* I am binded to a bigger patch. There is 2 cases:
				- rebind=false. This is an original Bind of all patch of a zone.
					If my bigger patch has not be bound, I CANNOT do rebind faces, since my bigger neighbor patch is not
					correctly tesselated.
					Wait for the BiggerPatch do the correct bind (see above)
				- rebind=true. This is possible for a patch in border of zone to have some bind 1/X (even if Bind 1/X
					is not possible across zone, it IS possible that a patch containing a bind 1/X NOT ON EDGE OF ZONE
					exist (and they do exist...))

				If neighbor bind has been done (must be the case for rebind), MUST do the rebind
					(because of CZoneBindPatch() which first unbind() this, then bind())
			*/
			// if rebind, my neigbhor bind should be done
			if(rebind==true)
			{
				nlassert(bind.Next[0]->_BindZoneNeighbor[bind.Edge[0]]);
			}
			// if my neighbor is bound, i have to do the bind (since CZoneBindPatch() had call unbind)
			if(bind.Next[0]->_BindZoneNeighbor[bind.Edge[0]])
			{
				// First, make the link with the face to which I must connect.
				// -----------------

				// Get the coordinate of the current edge of this patch
				CVector2f	uvi0, uvi1;
				switch(i)
				{
				case 0: uvi0.set(0,0); uvi1.set(0,1);  break;
				case 1: uvi0.set(0,1); uvi1.set(1,1);  break;
				case 2: uvi0.set(1,1); uvi1.set(1,0);  break;
				case 3: uvi0.set(1,0); uvi1.set(0,0);  break;
				};
				// mul by OrderS/OrderT for CPatchUVLocator
				uvi0.x*= OrderS;
				uvi0.y*= OrderT;
				uvi1.x*= OrderS;
				uvi1.y*= OrderT;
				// build a CPatchUVLocator to transpose coorindate ot this edge in coordinate on the bigger Neighbor patch.
				CBindInfo	bindInfo;
				getBindNeighbor(i, bindInfo);
				nlassert(bindInfo.Zone!=NULL && bindInfo.NPatchs==1);
				CPatchUVLocator		puvloc;
				puvloc.build(this, i, bindInfo);

				// transpose from this patch coord in neighbor patch coord.
				CVector2f	uvo0, uvo1;
				uint	pid;
				CPatch	*patchNeighbor;
				// Do it for uvi0
				pid= puvloc.selectPatch(uvi0);
				puvloc.locateUV(uvi0, pid, patchNeighbor, uvo0);
				nlassert(patchNeighbor == bindInfo.Next[0]);
				// Do it for uvi1
				pid= puvloc.selectPatch(uvi1);
				puvloc.locateUV(uvi1, pid, patchNeighbor, uvo1);
				nlassert(patchNeighbor == bindInfo.Next[0]);
				// Rescale to have uv in 0,1 basis.
				uvo0.x/= patchNeighbor->OrderS;
				uvo0.y/= patchNeighbor->OrderT;
				uvo1.x/= patchNeighbor->OrderS;
				uvo1.y/= patchNeighbor->OrderT;

				// Now, traverse the tesselation and find the first CTessFace which use this edge.
				CTessFace	*faceNeighbor;
				faceNeighbor= patchNeighbor->linkTessFaceWithEdge(uvo0, uvo1, this->getRootFaceForEdge(i));
				nlassert(faceNeighbor);
				// Bind me on Next.
				this->changeEdgeNeighbor(i, faceNeighbor);
			}
		}

	}

	// Propagate the binds to sons.
	Son0->updateBind();
	Son1->updateBind();

}


// ***************************************************************************
void			CPatch::forceMergeAtTileLevel()
{
	nlassert(Son0 && Son1);

	Son0->forceMergeAtTileLevel();
	Son1->forceMergeAtTileLevel();
}



// ***************************************************************************
// ***************************************************************************
// Texturing.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CPatchRdrPass	*CPatch::getTileRenderPass(sint tileId, sint pass)
{
	// All but lightmap.
	nlassert(pass==NL3D_TILE_PASS_RGB0 || pass==NL3D_TILE_PASS_RGB1 || pass==NL3D_TILE_PASS_RGB2 ||
		pass==NL3D_TILE_PASS_ADD);

	bool	additive= (pass==NL3D_TILE_PASS_ADD);
	sint	passNum= pass-NL3D_TILE_PASS_RGB0;
	// If additive, take the additve tile of bigger existing pass.
	if(additive)
	{
		// Default: take addtive of pass 0.
		passNum= 0;
		// If the pass1 is not empty, may take its tile.
		if(Tiles[tileId].Tile[1]!=0xFFFF)
		{
			passNum= 1;
			// If the pass2 is not empty, take its tile.
			if(Tiles[tileId].Tile[2]!=0xFFFF)
				passNum= 2;
		}
	}

	sint	tileNumber= Tiles[tileId].Tile[passNum];
	if(tileNumber==0xFFFF)
	{
		// Display a "fake" only if pass 0.
		if(pass==NL3D_TILE_PASS_RGB0)
			return Zone->Landscape->getTileRenderPass(0xFFFF, false);
		// Else, this tile do not have such a pass (not a transition).
		return NULL;
	}
	else
	{
		// return still may be NULL, in additive case.
		return Zone->Landscape->getTileRenderPass(uint16(tileNumber), additive);
	}
}

// ***************************************************************************
void			CPatch::getTileUvInfo(sint tileId, sint pass, bool alpha, uint8 &orient, CVector &uvScaleBias, bool &is256x256, uint8 &uvOff)
{
	// All but lightmap.
	nlassert(pass==NL3D_TILE_PASS_RGB0 || pass==NL3D_TILE_PASS_RGB1 || pass==NL3D_TILE_PASS_RGB2 ||
		pass==NL3D_TILE_PASS_ADD);

	bool	additive= (pass==NL3D_TILE_PASS_ADD);
	sint	passNum= pass-NL3D_TILE_PASS_RGB0;
	// If additive, take the additve tile of bigger existing pass.
	if(additive)
	{
		// Default: take addtive of pass 0.
		passNum= 0;
		// If the pass1 is not empty, may take its tile.
		if(Tiles[tileId].Tile[1]!=0xFFFF)
		{
			passNum= 1;
			// If the pass2 is not empty, take its tile.
			if(Tiles[tileId].Tile[2]!=0xFFFF)
				passNum= 2;
		}
	}

	sint	tileNumber= Tiles[tileId].Tile[passNum];
	if(tileNumber==0xFFFF)
	{
		// dummy... Should not be called here.
		orient= 0;
		uvScaleBias.x=0;
		uvScaleBias.y=0;
		uvScaleBias.z=1;
		is256x256=false;
		uvOff=0;
	}
	else
	{
		orient= Tiles[tileId].getTileOrient(passNum);
		Tiles[tileId].getTile256Info(is256x256, uvOff);
		CTile::TBitmap type;
		// If alpha wanted, return its UV info (works either for Alpha in Diffuse Pass and Alpha in Additive Pass)
		if(alpha)
			type= CTile::alpha;
		else
		{
			if(additive)
				type= CTile::additive;
			else
				type= CTile::diffuse;
		}

		uint8	rotalpha;
		Zone->Landscape->getTileUvScaleBiasRot(uint16(tileNumber), type, uvScaleBias, rotalpha);

		// Add the special rotation of alpha.
		if(alpha)
			orient= (orient+rotalpha)&3;
	}

}


// ***************************************************************************
void			CPatch::deleteTileUvs()
{
	Son0->deleteTileUvs();
	Son1->deleteTileUvs();
}


// ***************************************************************************
void			CPatch::recreateTileUvs()
{
	// Reset the Tile rdr list.
	for(sint tb=0; tb<(sint)TessBlocks.size();tb++)
	{
		// Vertices must all be reseted.
		TessBlocks[tb].NearVertexList.clear();
		for(sint i=0;i<NL3D_TESSBLOCK_TILESIZE;i++)
		{
			CTileMaterial	*tm= TessBlocks[tb].RdrTileRoot[i];
			if(tm)
			{
				for(sint pass=0;pass<NL3D_MAX_TILE_FACE;pass++)
				{
					tm->TileFaceList[pass].clear();
				}
			}
		}
	}

	Son0->recreateTileUvs();
	Son1->recreateTileUvs();
}


// ***************************************************************************
void		CPatch::appendTessellationLeaves(std::vector<const CTessFace*>  &leaves) const
{
	nlassert(Son0);
	nlassert(Son1);
	Son0->appendTessellationLeaves(leaves);
	Son1->appendTessellationLeaves(leaves);
}


// ***************************************************************************
CLandscape		*CPatch::getLandscape () const
{
	return Zone->getLandscape();
}





// ***************************************************************************
uint8			CPatch::getOrderForEdge(sint8 edge) const
{
	uint	e= ((sint)edge + 256)&3;
	// If an horizontal edge.
	if( e&1 )	return OrderS;
	else		return OrderT;
}


// ***************************************************************************
// ***************************************************************************
// Realtime Bind info.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CPatch::getBindNeighbor(uint edge, CBindInfo &neighborEdge) const
{
	nlassert(edge<4);

	if(_BindZoneNeighbor[edge]!=NULL)
	{
		getZone()->buildBindInfo(PatchId, edge, _BindZoneNeighbor[edge], neighborEdge);
	}
	else
	{
		neighborEdge.Zone= NULL;
		neighborEdge.NPatchs= 0;
		neighborEdge.MultipleBindNum= 0;
	}
}

// ***************************************************************************
/// debug coloring
void CPatch::setupColorsFromTileFlags(const NLMISC::CRGBA colors[4])
{
	for (uint s = 0; s <= OrderS; ++s)
	{
		for (uint t = 0; t <= OrderT; ++t)
		{
			uint index = std::min(t, (uint) (OrderT - 1)) * OrderS
				         + std::min(s, (uint) (OrderS - 1));
			TileColors[s + t * (OrderS + 1)].Color565 = colors[(uint) (Tiles[index].getVegetableState())].get565();
		}
	}
}

// ***************************************************************************
void CPatch::copyTileFlagsFromPatch(const CPatch *src)
{
	nlassert(OrderS == src->OrderS
			 && OrderT == src->OrderT);

	for (uint k = 0; k  < Tiles.size(); ++k)
	{
		Tiles[k].copyFlagsFromOther(src->Tiles[k]);
	}
}


// ***************************************************************************
// ***************************************************************************
// Tiles get interface.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTileElement *CPatch::getTileElement(const CUV &uv)
{
	// compute tile coord and lumel coord.
	sint	ts, tt;

	// fastFloor: use a precision of 256 to avoid doing OptFastFloorBegin.
	// add 128, to round and get cneter of lumel.
	ts= NLMISC::OptFastFloor(uv.U* (OrderS<<8) + 128);	ts>>=8;
	tt= NLMISC::OptFastFloor(uv.V* (OrderT<<8) + 128);	tt>>=8;
	clamp(ts, 0, OrderS-1);
	clamp(tt, 0, OrderT-1);

	// get the lumel
	return &(Tiles[ts+tt*OrderS]);
}

// ***************************************************************
uint32	CPatch::countNumTriFar0() const
{
	uint32 numIndex = MasterBlock.Far0FaceVector ? *MasterBlock.Far0FaceVector : 0;
	uint			nTessBlock= TessBlocks.size();
	const CTessBlock		*pTessBlock= nTessBlock>0? &TessBlocks[0]: NULL;
	for(; nTessBlock>0; pTessBlock++, nTessBlock--)
	{
		const CTessBlock		&tblock= *pTessBlock;
		// if block visible, render
		if( tblock.visibleFar0() )
		{
			numIndex += *(tblock.Far0FaceVector);
		}
	}
	return numIndex;
}

// ***************************************************************
uint32	CPatch::countNumTriFar1() const
{
	uint32 numIndex = MasterBlock.Far1FaceVector ? *MasterBlock.Far1FaceVector : 0;
	uint			nTessBlock= TessBlocks.size();
	const CTessBlock		*pTessBlock= nTessBlock>0? &TessBlocks[0]: NULL;
	for(; nTessBlock>0; pTessBlock++, nTessBlock--)
	{
		const CTessBlock		&tblock= *pTessBlock;
		// if block visible, render
		if( tblock.visibleFar1() )
		{
			numIndex += *(tblock.Far1FaceVector);
		}
	}
	return numIndex;
}





} // NL3D

