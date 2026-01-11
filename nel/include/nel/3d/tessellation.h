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

#ifndef NL_TESSELLATION_H
#define NL_TESSELLATION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/uv.h"
#include "nel/misc/bsphere.h"
#include "nel/misc/vector_2f.h"
#include "nel/3d/tess_list.h"
#include "nel/3d/tess_face_priority_list.h"
#include "nel/3d/landscape_def.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;
using NLMISC::CUV;


class	CPatch;
class	CTessFace;
class	CLandscapeVBAllocator;
class	CTessBlock;
class	CPatchRdrPass;
struct	CTileMaterial;

// ***************************************************************************
const	float	OO32768= 1.0f/0x8000;



// ***************************************************************************
/**
 * The parametric coordinates of the patch. 0x0000<=>0.0f. 0x8000<=> 1.0f.
 */
class	CParamCoord
{
public:
	uint16	S,T;

public:
	CParamCoord() {}
	CParamCoord(uint16 s, uint16 t) {S=s; T=t;}
	// Create at middle.
	CParamCoord(CParamCoord a, CParamCoord b)
	{
		S= (uint16) (((uint16)a.S + (uint16)b.S)>>1);
		T= (uint16) (((uint16)a.T + (uint16)b.T)>>1);
	}
	// Get s,t as floats. returned s,t E [0,1].
	float	getS() const {return S*OO32768;}
	float	getT() const {return T*OO32768;}
	// Set s,t as float. s,t E [0,1].
	void	setST(float s, float t) {S= (uint16)(s*32768);T= (uint16)(t*32768);}
	// vertex on the border?
	bool	onBorder() const {return (S==0 || S==0x8000 || T==0 || T==0x8000);}
};



// ***************************************************************************
/**
 * A Landscape Vertex
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTessVertex
{
public:
	// The current position, + geomorph.
	CVector			Pos;
	CVector			StartPos, EndPos;
	// Geomorph Information, for VertexProgram geomorph. NB: no RefineThreshold dependency here.
	float			MaxFaceSize;	// max(SizeFaceA, SizeFaceB)
	float			MaxNearLimit;	// max(NearLimitFaceA, NearLimitFaceB) with NearLimitFace= (1<<Patch->TileLimitLevel) / (1<<Level);

	CTessVertex()
	{
		// init to 0, so max always() works.
		MaxFaceSize= 0;
		MaxNearLimit= 0;
	}

	// Compute the geomorphed position, with MaxFaceSize, MaxNearLimit ..., and current globals.
	void			computeGeomPos();

};


// ***************************************************************************
struct	CTessFarVertex : public CTessNodeList
{
	CTessBlock		*OwnerBlock;
	sint32			Index0;		// The index of Far0 in the Far VB.
	sint32			Index1;		// The index of Far1 in the Far VB.
	CTessVertex		*Src;		// The src vertex to copy Pos, and compute alpha.
	CParamCoord		PCoord;		// The patch coord, to compute far UV.
};


// ***************************************************************************
struct	CTessNearVertex : public CTessNodeList
{
	CTessBlock		*OwnerBlock;
	sint32			Index;		// The index in the Tile VB.
	CTessVertex		*Src;		// The src vertex to copy Pos.
	CUV				PUv0;
	CUV				PUv1;
	// Uv For dynamic lightmap
	CUV				PUv2;

public:
	// Init this near vertex UVs, at middle of a and b.
	void		initMiddleUv(CTessNearVertex &a, CTessNearVertex &b)
	{
		PUv0= (a.PUv0+b.PUv0)*0.5;
		PUv1= (a.PUv1+b.PUv1)*0.5;
		PUv2= (a.PUv2+b.PUv2)*0.5;
	}
};


// ***************************************************************************
class	CRdrTileId
{
public:
	CPatchRdrPass	*PatchRdrPass;
	CTileMaterial	*TileMaterial;

	CRdrTileId();

	// For list reading.
	CRdrTileId		*getNext() {return _Next;}

private:
	friend	class	CPatchRdrPass;
	CRdrTileId		*_Next;
};


// ***************************************************************************
/** A tileface. There is one TileFace per pass (BUT Lightmap PASS!! same used for RGB0 and LIGHTMAP).
 *
 */
struct	CTileFace : public CTessNodeList
{
	CTessNearVertex		*V[3];
};


// ***************************************************************************
struct	CTileMaterial
{
	// The coordinates of the tile in the patch.
	uint8			TileS, TileT;
	// The rendering passes materials.
	CRdrTileId		Pass[NL3D_MAX_TILE_PASS];
	// Pass are:
	//  0: RGB general Tile.
	//  1: 1st alpha tile. 0: RGB.  1: Alpha.
	//  2: 2nd alpha tile. 0: RGB.  1: Alpha.
	//  3: additive pass.
	//	4: Cloud*Lightmap.
	// NB: BIG TRICK: the pass 0 has 2 Uvs: Uv0: RGB, and Uv1: Lightmap!!!! even, if they are not used at the same pass :o).
	// This simplifies, save memory, and save from copies of v3f.
	// NB: RENDER ORDER: CLOUD*LIGHTMAP is done BEFORE ADDITIVE.

	// The list of faces, for each pass. NB: no faces for lightmaps!! since share same face/vertices from RGB0.
	// The only thing not shared is the renderpass (Pass[0] and Pass[3] are different).
	CTessList<CTileFace>	TileFaceList[NL3D_MAX_TILE_FACE];

	// FaceVectors.
	TLandscapeIndexType		*TileFaceVectors[NL3D_MAX_TILE_FACE];


	// The global id of the little lightmap part for this tile.
	uint			LightMapId;

	// ptrs are Null by default.
	CTileMaterial();

	// For fast Tile clipping, may add only tiles which are visibles in RenderPass. (see preRender()).
	void			appendTileToEachRenderPass(uint patchNumRenderableFaces);

	// Render of this tile. code in patch_render.cpp. use pass to acces both Pass[] and TileFaceVectors[].
	// So don't work for lightmap.
	void			renderTile(uint pass);
	// For faster render of Pass RGB0 and Lightmap which are always setuped.
	void			renderTilePassRGB0();
	void			renderTilePassLightmap();

};


// ***************************************************************************
/**
 * A Landscape Triangle.
 *	MemSize: 28*4 octets. => for 100K faces, it takes 11.2 Mo.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTessFace : public CTessNodeList, public CTessFacePListNode
{
public:

	/// \name geometric tesselation.
	//@{
	// Can't be global, because of propagated splits.
	CPatch			*Patch;
	CTessVertex		*VBase, *VLeft, *VRight;
	CTessFace		*FBase, *FLeft, *FRight;
	CTessFace		*Father, *SonLeft, *SonRight;
	// The parametric coordinates of the patch.
	CParamCoord		PVBase, PVLeft, PVRight;
	// To know if we can split or not, and to compute tile errormetric.
	uint8			Level;			// Level of Detail of this face. (++ at each split).
	// Mark of recursion (see canMerge() ...).
	bool			RecursMarkCanMerge;
	bool			RecursMarkForceMerge;
	//@}

	/// \name Tile Material Infos (uvs...).
	// @{
	// The number of the tile relatively to the patch TileMap.
	// We CANNOT store this into CTileMaterial, because of sameTile() test.
	uint8			TileId;
	// The multi-pass tile Material.
	CTileMaterial	*TileMaterial;
	// The Tile Faces. There is no Face for lightmap, since use the one from RGB0.
	CTileFace		*TileFaces[NL3D_MAX_TILE_FACE];

	// Enum for UVs V[] array in TileFaces
	enum	TTileUvId {IdUvBase=0, IdUvLeft, IdUvRight};

	// @}


	/// \name Error metric.
	// @{
	// Size could be computed from TileSize: Size= Patch->BaseSize / (1<<(TileSize-Patch->BaseTileSize)). (or optimized)
	// But used by updateErrorMetric(), and must be as fast as possible.
	sint			ErrorMetricDate;		// The date of errormetric update.
	float			Size;					// /2 at each split.
	CVector			SplitPoint;				// Midle of VLeft/VRight. Used to compute the errorMetric.
	float			ErrorMetric;			// equal to projected size of face, but greater for the transition Far-Near.
	float			MaxDistToSplitPoint;	// Max Dist from SplitPoint to each of 3 vertices.
	// @}


	// For Render.
	CTessFarVertex	*FVBase, *FVLeft, *FVRight;
	// NB: herit from CTessNodeList to use Prec and Next ptrs.

	// Id For ShadowMap
	sint			ShadowMapTriId;

public:
	CTessFace();
	// do nothing dtor.
	~CTessFace();

	// Utilities.
	bool			isLeaf() const {return SonLeft==NULL;}
	bool			isRectangular() const;
	bool			hasVertex(CTessVertex *v) const {return VBase==v || VLeft==v || VRight==v;}
	bool			hasEdge(CTessVertex *v0, CTessVertex *v1) const {return hasVertex(v0) && hasVertex(v1);}
	void			changeNeighbor(CTessFace *from, CTessFace *to)
	{
		if(FBase==from) FBase=to;
		if(FLeft==from) FLeft=to;
		if(FRight==from) FRight=to;
	}

	// Fill all Tile Material infos (id, type...), according to CPatch tilemap, and CTessFace paramcoord.
	void			computeTileMaterial();
	// Release Tile Uvs.
	void			releaseTileMaterial();

	// compute the NearLimit for this face. NearLimit= (1<<Patch->TileLimitLevel) / (1<<Level)
	float			computeNearLimit();

	// update the error metric
	void			updateErrorMetric();
	// can split leaf only.
	void			split(bool propagateSplit=true);
	// can merge "short roots" only (roots which have leafs).
	bool			merge();
	/// refine the node, and his sons. Refine all nodes.
	void			refineAll();
	// update the refinement of the node, split if necessary (and then updateRefine() his splitted sons),
	// Then re-insert this face in the appropriate CLandscape priority list.
	void			updateRefineSplit();
	// Same for merge.
	void			updateRefineMerge();


	/** compute the SplitPoint. VBase / VLeft and VRight must be valid.
	 *	Also compute MaxDistToSplitPoint.
	 */
	void	computeSplitPoint();

	// Used by CPatch::unbind(). isolate the tesselation from other patchs.
	void			unbind();
	// Used by CPatch::unbind(). force the merging of face.
	void			forceMerge();
	// Used by CPatch::linkTessFaceWithEdge(). See it. NB: it assert if the link is NULL.
	CTessFace		*linkTessFaceWithEdge(const NLMISC::CVector2f &uv0, const NLMISC::CVector2f &uv1, CTessFace *linkTo);
	// Used by CPatch::bind(). Split if necessary, according to neighbors.
	bool			updateBindEdge(CTessFace	*&edgeFace, bool &splitWanted);
	void			updateBind();
	void			updateBindAndSplit();
	// Used by CPatch::forceMergeAtTileLevel(). force the merging of face (as possible).
	void			forceMergeAtTileLevel();

	// Used by CPatch::averageTesselationVertices(). See CLandscape doc. recurs call.
	void			averageTesselationVertices();
	// For CZone::refreshTesselationGeometry() only. recurs call.
	void			refreshTesselationGeometry();


	// Used by CZone::changePatchTexture().
	void			deleteTileUvs();
	void			recreateTileUvs();


	// Used by CPatch::getTesselatedPos(). recurs call.
	void			getTesselatedPos(const CUV &uv, bool verifInclusion, CVector &ret);


	// Used by CPatch::appendTessellationLeaves(). recurs call.
	void			appendTessellationLeaves(std::vector<const CTessFace*>  &leaves) const;



private:
	// Faces have the same tile???
	static bool		sameTile(const CTessFace *a, const CTessFace *b)
	{
		// WE CANNOT do this test, testing TileMaterial, because this one may be releleased by releaseMaterial() !!
		return (a->Patch==b->Patch && a->TileId==b->TileId);
	}

	/// \name UV mgt.
	// @{
	// Allocate a CTessNearVertex "id" (base, left or right) for each not NULL TileFace of "this" face.
	// Init with good CTessVertex::Src, and then, insert it into Patch RenderList.
	void	allocTileUv(TTileUvId id);
	// delete a CTessNearVertex "id", removing it from Patch RenderList, for each not NULL TileFace of "this" face.
	void	deleteTileUv(TTileUvId id);
	// Just ptr-copy a CTessNearVertex "id" from another face/vertex id. Do this for each not NULL TileFace.
	void	copyTileUv(TTileUvId id, CTessFace *srcFace, TTileUvId srcId);

	// The Base NearVertex must be allocated before.
	// for each not NULL TileFace of "this" face, make the Base NearVertex the middle of baseFace TileUvLeft, and
	// baseFace TileUvRight.
	void	heritTileUv(CTessFace *baseFace);

	// If patch !isRenderClipped(), and other state, create and fill VB for the vertex id of all faces of this tileFace.
	void	checkCreateFillTileVB(TTileUvId id);
	// If patch !isRenderClipped(), and other state, fill VB only for the vertex id of all faces of this tileFace.
	void	checkFillTileVB(TTileUvId id);


	// create The TileFaces, according to the TileMaterial passes. NO INSERTION IN RENDERLIST!!
	void	buildTileFaces();
	// delete The TileFaces, according to the TileMaterial passes. NO REMOVAL FROM RENDERLIST!!
	void	deleteTileFaces();
	// return true iff TileFaces are all NULL.
	bool	emptyTileFaces();

	// @}


	// see updateErrorMetric.
	void	computeTileErrorMetric();
	// see updateRefine().
	float	computeTileEMForUpdateRefine(float distSplitPoint, float distMinFace, float nearLimit);

	// See updateRefine() and merge().
	bool	canMerge(bool testEm);

	// see split().
	void	splitRectangular(bool propagateSplit);
	void	doMerge();
	void	heritTileMaterial();

	// see computeTileMaterial().
	void	initTileUvRGBA(sint pass, bool alpha, CParamCoord pointCoord, CParamCoord middleCoord, CUV &uv);
	// The same, but for the lightmap pass.
	void	initTileUvLightmap(CParamCoord pointCoord, CParamCoord middleCoord, CUV &uv);
	// The same, but for the dynamic lightmap (DLM) pass.
	void	initTileUvDLM(CParamCoord pointCoord, CUV &uv);

	// Far Vertices: update info (ptr, and PCoord).
	void			updateNearFarVertices();


	// Final copmute the 3D pos in a face, according to UV (with gradients).
	void			computeTesselatedPos(const CUV &uv, CVector &ret);

private:
	// Fake face are the only ones which have a NULL patch ptr (with mult face).
	// The fake face which indicates a "can't merge". Useful for bind 2/4 or 1/4.
	static	CTessFace	CantMergeFace;

public:
	// The fake face which indicates a multiple patch face. Used in updateBind(), for multiple bind.
	static	CTessFace	MultipleBindFace;
};


} // NL3D


#endif // NL_TESSELLATION_H

/* End of tessellation.h */
