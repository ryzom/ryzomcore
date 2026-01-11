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

#ifndef NL_PATCH_H
#define NL_PATCH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/rgba.h"
#include "nel/3d/tessellation.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/bsphere.h"
#include "nel/misc/triangle.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/object_vector.h"
#include "nel/3d/tile_element.h"
#include "nel/3d/tile_color.h"
#include "nel/3d/tess_block.h"
#include "nel/3d/tile_light_influence.h"
#include "nel/3d/point_light_influence.h"


namespace NL3D {

#define NL_MAX_TILES_BY_PATCH_EDGE_SHIFT 4										// max 16x16 tiles by patch (shift version)
#define NL_MAX_TILES_BY_PATCH_EDGE (1<<NL_MAX_TILES_BY_PATCH_EDGE_SHIFT)		// max 16x16 tiles by patch
#define NL_PATCH_FAR0_ROTATED 0x1												// Flags far0 rotated
#define NL_PATCH_FAR1_ROTATED 0x2												// Flags far1 rotated
#define NL_PATCH_SMOOTH_FLAG_SHIFT 0x2											// Smooth flags shift
#define NL_PATCH_SMOOTH_FLAG_MASK 0x3c											// Smooth flags mask

#define NL_LUMEL_BY_TILE_SHIFT 2												// 4 lumels by tile
#define NL_LUMEL_BY_TILE (1<<NL_LUMEL_BY_TILE_SHIFT)							// 4 lumels by tile
#define NL_BLOCK_LUMEL_COMPRESSED_SIZE 8										// Compressed block size 8 bytes


#define NL_PATCH_BLOCK_MAX_QUAD 4												// Max quad per CPatchQuadBlock.
#define NL_PATCH_BLOCK_MAX_VERTEX (NL_PATCH_BLOCK_MAX_QUAD+1)					// Max vertex per CPatchQuadBlock.


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CAABBox;
using NLMISC::CBSphere;
using NLMISC::CRGBA;


class	CLandscape;
class	CZone;
class	CBezierPatch;
class	ITexture;
class	CVegetableClipBlock;
class	CVegetableManager;
class	CVegetableInstanceGroup;
class	CLandscapeVegetableBlock;
class	CLandscapeVegetableBlockCreateContext;
class	CPatchDLMContext;
class	CPatchDLMPointLight;


// ***************************************************************************
#define	NL3D_NOISE_MAX	1


// ***************************************************************************
/*
	NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK == 2 means that
	clipBlocks enclose 2*2 tessBlocks (hence 4*4 tiles).
*/
#define	NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK_SHIFT	1
#define	NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK		(1<<NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK_SHIFT)


// ***************************************************************************
class	CVector3s
{
public:
	sint16	x,y,z;

public:
	void	pack(const CVector &v, const CVector &bias, float scale)
	{
		float	xr,yr,zr;
		xr= (v.x - bias.x)/scale;
		yr= (v.y - bias.y)/scale;
		zr= (v.z - bias.z)/scale;
		NLMISC::clamp(xr, -32768, 32767);
		NLMISC::clamp(yr, -32768, 32767);
		NLMISC::clamp(zr, -32768, 32767);
		x= (sint16)xr;
		y= (sint16)yr;
		z= (sint16)zr;
	}
	void	unpack(CVector &v, const CVector &bias, float scale) const
	{
		v.x= x*scale + bias.x;
		v.y= y*scale + bias.y;
		v.z= z*scale + bias.z;
	}
	void	serial(NLMISC::IStream &f)
	{
		f.serial(x,y,z);
	}
};


// ***************************************************************************
/**
 * A landscape patch identifier (zone/patch).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CPatchIdent
{
	sint32		ZoneId;		// From which zone this patch come from...
	uint16		PatchId;	// Id of this patch.

	// default ctor
	CPatchIdent() {}
	//
	CPatchIdent(sint32 zoneId, uint16 patchId) : ZoneId(zoneId), PatchId(patchId) {}
public:
	bool			operator<(const CPatchIdent &p) const
	{
		if(ZoneId!=p.ZoneId) return ZoneId<p.ZoneId;
		return PatchId<p.PatchId;
	}

	bool			operator==(const CPatchIdent &p) const
	{
		return ZoneId==p.ZoneId && PatchId==p.PatchId;
	}
	bool			operator!=(const CPatchIdent &p) const
	{
		return !(*this==p);
	}

};


// ***************************************************************************
/**
 * A triangle from a patch identifier.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CTrianglePatch : public NLMISC::CTriangleUV
{
	/// from which patch this triangle comes from.
	CPatchIdent		PatchId;
};


// ***************************************************************************
/**
 * An descriptor of a group of tiles in a patch.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CPatchBlockIdent
{
public:
	/// the patch.
	CPatchIdent		PatchId;
	/// size of the patch, in tile coordinates.
	uint8			OrderS,OrderT;
	/// coordinate of the SubPart of the patch, in tile coordinates.
	uint8			S0,S1,T0,T1;

public:
	/// \name Operators.
	// @{
	bool			operator==(const CPatchBlockIdent &pb) const
	{
		return PatchId==pb.PatchId &&
			S0==pb.S0 && S1==pb.S1 &&
			T0==pb.T0 && T1==pb.T1;
	}
	bool			operator!=(const CPatchBlockIdent &pb) const
	{
		return !(*this==pb);
	}

	bool			operator<(const CPatchBlockIdent &pb) const
	{
		if(PatchId!=pb.PatchId)
			return PatchId<pb.PatchId;
		if(S0!=pb.S0)	return S0<pb.S0;
		if(S1!=pb.S1)	return S1<pb.S1;
		if(T0!=pb.T0)	return T0<pb.T0;
		return T1<pb.T1;
	}
	bool			operator<=(const CPatchBlockIdent &pb) const
	{
		return (*this<pb) || (*this==pb);
	}
	bool			operator>(const CPatchBlockIdent &pb) const
	{
		return !(*this<=pb);
	}
	bool			operator>=(const CPatchBlockIdent &pb) const
	{
		return !(*this<pb);
	}

	// @}
};


// ***************************************************************************
/**
 * A group of tiles faces in a patch. For speed/mem, max size of a CPatchQuadBlock is fixed.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CPatchQuadBlock
{
public:
	/// from what subPart of the patch those data comes from.
	CPatchBlockIdent	PatchBlockId;

	/// evaluated Vertices of this patch.
	CVector				Vertices[NL_PATCH_BLOCK_MAX_VERTEX*NL_PATCH_BLOCK_MAX_VERTEX];

public:

	/** build the 2 triangles from a quad of the CPatchQuadBlock.
	 * quadId is the number of the quad, relatively to the PatchQuadBlock.
	 */
	void		buildTileTriangles(uint8 quadId, CTrianglePatch  triangles[2]) const;
};



// ***************************************************************************
/**
 * A landscape patch.
 * QuadPatch layout (same notations as 3ds Max SDK).
 *
 *   A---> ad ---- da <---D
 *   |                    |
 *   |                    |
 *   v                    v
 *   ab    ia      id     dc
 *
 *   |                    |
 *   |                    |
 *
 *   ba    ib      ic     cd
 *   ^                    ^
 *   |                    |
 *   |                    |
 *   B---> bc ---- cb <---C
 *
 * NB: Patch 1x1 or 1xX are illegal: lot of problem: rectangular geomoprh, Son0 and Son1 must be setup as tile at beginning ...
 *
 * NB: Edges number are:
 *	- 0: AB.
 *	- 1: BC.
 *	- 2: CD.
 *	- 3: DA.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPatch
{
public:

	struct	CBindInfo
	{
		// The zone on this edge. NULL if not loaded (or if none).
		CZone			*Zone;

		// The number of patchs on this edge. 0,1, 2 or 4. if MultipleBindNum>1, NPatchs==1.
		sint			NPatchs;

		// Special case: on this edge, we are a small patch connected to a bigger: this is the X of 1/X (1,2 or 4).
		// 0 if this is not the case.
		uint8			MultipleBindNum;
		// valid only if MultipleBindNum>1. this tells our place in this MultipleBind: 0<=MultipleBindId<MultipleBindNum.
		uint8			MultipleBindId;


		CPatch			*Next[4];	// The neighbor patch i.
		sint			Edge[4];	// On which edge of Nexti we are binded.
	};

public:
	/// The patch coordinates (see CBezierPatch).
	CVector3s		Vertices[4];
	CVector3s		Tangents[8];
	CVector3s		Interiors[4];

	// Lumel array compressed.
	std::vector<uint8>			CompressedLumels;

	// There is OrderS*OrderT tiles. CZone build it at build() time.
	std::vector<CTileElement>	Tiles;

	// There is OrderS*OrderT tiles color. CZone build it at build() time.
	std::vector<CTileColor>		TileColors;

	// There is OrderS/2+1 * OrderT/2+1 tiles light influence. CZone build it at build() time.
	std::vector<CTileLightInfluence>		TileLightInfluences;


	/// Noise Data.
	// @{
	/// The orientation of the NoiseMap. 0,1,2,3. This represent a CCW rotation of the NoiseMap.
	uint8			NoiseRotation;

	/** setup Smooth flags for Noise on corner: used for Noise geometry and for lighting.
	 *	NB: convention: corner0==A, corner1==B ...
	 */
	void			setCornerSmoothFlag(uint corner, bool smooth);
	bool			getCornerSmoothFlag(uint corner) const;

private:
	/// Put here for packing with NoiseRotation.
	uint8			_CornerSmoothFlag;

public:
	// @}


public:

	/// Constructor
	CPatch();
	/// dtor
	~CPatch();

	/** compile a valid patch. (init)
	 * Call this method before any other. Zone and Control points must be valid before calling compile(). \n
	 * This is an \b ERROR to call compile() two times. \n
	 * \param z zone where the patch must be binded.
	 * \param orderS the Tile order in S direction: 2,4,8,16.
	 * \param orderT the Tile order in T direction: 2,4,8,16.
	 * \param errorSize if 0, setup() compute himself the errormetric of the patch. May be setup to surface of patch,
	 *  modulated by tangents and displacement map.
	 */
	void			compile(CZone *z, uint patchId, uint8 orderS, uint8 orderT, CTessVertex *baseVertices[4], float errorSize=0);
	/// Un-compile a patch. Tesselation is deleted. if patch is not compiled, no - op.
	void			release();


	///  Get the landscape in which is placed this patch.
	CLandscape		*getLandscape () const;
	CZone			*getZone() const {return Zone;}
	uint8			getOrderS() const {return OrderS;}
	uint8			getOrderT() const {return OrderT;}
	uint8			getOrderForEdge(sint8 edge) const;
	float			getErrorSize() const {return ErrorSize;}
	sint			getFar0() const {return Far0;}
	sint			getFar1() const {return Far1;}
	uint16			getPatchId () const {return PatchId;}
	/// return neighborhood information.
	void			getBindNeighbor(uint edge, CBindInfo &neighborEdge) const;

	/// Build the bbox of the patch, according to ctrl points, and displacement map max value.
	CAABBox			buildBBox() const;

	/** Compute a vertex.
	 * Compute a vertex according to:
	 *	- s,t
	 *	- patch control points uncompressed with zone Bias/Scale.
	 *  - Patch UV geometric correction.
	 *	- Patch noise (and noise of Patch neighbor).
	 */
	CVector			computeVertex(float s, float t) const;


	/**	Same as computeVertex, but special accurate version for CVisualCollisionEntity.
	 *	If on a corner of the patch (0,0), (0,1) ...., take directly the BaseVertices[] result
	 *	if on a border of a patch (ie s==0, s==1, t==0 or t==1), also
	 *	compute the vertex of the neighbor patch (if any), and then average the result.
	 *	This ensure that tesselation generated is perfectly continous, even across patchs of same or different zones.
	 *	This feature is very important for CVisualCollisionEntity::snapToGround()
	 */
	CVector			computeContinousVertex(float s, float t) const;


	/** unbind the patch from All neighbors. neighbors patchs links are modified too. The tesselation is forcemerged.
	 */
	void			unbind();

	/** bind the patch to 4 neighbors, given in this patch edge order (0,1,2,3). Tesselation is reseted (patch unbound first).
	 * NB: this patch and his neighborood must be compiled...
	 * NB: neighbor patchs must not be NULL (but according to NPatchs).
	 */
	void			bind(CBindInfo	Edges[4], bool rebind);

	/// For changing TileMaxSubdivision. force tesselation to be under tile.
	void			forceMergeAtTileLevel();

	/** This is especially for Pacs. see CLandscape desc.
	 */
	void			averageTesselationVertices();

	/// Refine this patch. Even if clipped. Refine all nodes.
	void			refineAll();


	/// \name Render
	//@{

	/// preRender this patch. Build Max faces / pass etc...
	void			preRender(const NLMISC::CBSphere &patchSphere);
	// like preRender(), but update TextureFar only. (you should call preRender() in this case).
	void			updateTextureFarOnly(const NLMISC::CBSphere &patchSphere);
	/// Render this patch, if not clipped. Call PatchCurrentDriver->renderSimpleTriangles().
	// Global Setup setup it in CLandscape::render().
	void			renderFar0();
	void			renderFar1();
	// NB: renderTile() is now in CTileMaterial.
	// For All Far0/Far1/Tile etc..., compute Geomorph and Alpha in software (no VertexShader).
	void			computeSoftwareGeomorphAndAlpha();

	// update VB according to new clip state.
	void			updateClipPatchVB(bool renderClipped);

	// get the next patch to render in the same RenderPass for Far0.
	CPatch			*getNextFar0ToRdr() const {return _NextRdrFar0;}
	// get the next patch to render in the same RenderPass for Far1.
	CPatch			*getNextFar1ToRdr() const {return _NextRdrFar1;}

	//@}


	// release Far render pass/reset Tile/Far render.
	void			resetRenderFar();



	// For CZone changePatchTexture only.
	void			deleteTileUvs();
	void			recreateTileUvs();
	// For CZone::refreshTesselationGeometry() only.
	void			refreshTesselationGeometry();


	// Serial just the un-compiled part.
	void			serial(NLMISC::IStream &f);

	// unpack the patch into a floating point one.
	void			unpack(CBezierPatch	&p) const;

	/// \name Lumels methods

	/**
	  *  Unpack the lumels of the patches. Lumels are classes that describe the lighting environnement at a given texel
	  *  of the lightmap. It is used to compute the shadow map of the patch, compress it and uncompress it.
	  *  This method uncompress the lumels stored in its Lumels member.
	  *
	  *  \param pShadow is a pointer on the destination lumel buffer. Size must be ((OrderS*4/ratio)+1)*((OrderT*4/ratio)+1).
	  *  \param ratio is the one over the ratio of the texture destination. Must be 1 or 2.
	  *  \see packShadowMap(), resetCompressedLumels()
	  */
	void			unpackShadowMap (uint8 *pShadow);

	/**
	  *  Pack the lumels of the patches. Lumels are classes that describe the lighting environnement at a given texel
	  *  of the lightmap. It is used to compute the shadow map of the patch, compress it and uncompress it.
	  *  This method compress the lumels passed in parameter and stored them in its Lumels member.
	  *
	  *  \param pShadow is a pointer on the destination lumel buffer. Size must be (OrderS*4+1)*(OrderS*4+1).
	  *  \see unpackShadowMap(), resetCompressedLumels()
	  */
	void			packShadowMap (const uint8 *pLumel);

	/**
	  *  Rebuild the packed lumels without shadow. Only the interpolated color will be used.
	  *
	  *  \see packShadowMap(), unpackShadowMap()
	  */
	void			resetCompressedLumels ();

	/** Debug purpose only : setup the colors of this patch so that it shows which tiles
	  * have vegetable disabled, or are above, below water.
	  * User provides a table with 4 colors for each state :
	  * color 0 = above water
	  * color 1 = underwater
	  * color 2 = intersect water
	  * color 3 = vegetable disabled
	  */
	void setupColorsFromTileFlags(const NLMISC::CRGBA colors[4]);

	/** Set this patch flags from another one.
	  * The patchs must match
	  */
	void copyTileFlagsFromPatch(const CPatch *src);

	// Count the number of tri needed to draw the patch
	uint32 countNumTriFar0() const;
	uint32 countNumTriFar1() const;

private:

	// Methods used internaly to compute shadowmaps

	/**
	  *	Pack a 4x4 lumel block
	  *
	  *  \see packShadowMap(), unpackShadowMap()
	  */
	void			packLumelBlock (uint8 *dest, const uint8 *source, uint8 alpha0, uint8 alpha1);

	/**
	  * Eval an uncompressed 4x4 block against the original
	  *
	  *  \see packShadowMap(), unpackShadowMap()
	  */
	uint			evalLumelBlock (const uint8 *original, const uint8 *unCompressed, uint width, uint height);

	/**
	  *	Unpack a 4x4 lumel block
	  *
	  *  \see packShadowMap(), unpackShadowMap()
	  */
	void			unpackLumelBlock (uint8 *dest, const uint8 *src);

public:

	/// \name Smooth flags methods

	/**
	  * Set the smooth flag for the n-th edge. flag is false if this edge must by smoothed, true else.
	  */
	void setSmoothFlag (uint edge, bool flag)
	{
		// Erase it
		Flags&=~(1<<(edge+NL_PATCH_SMOOTH_FLAG_SHIFT));

		// Set it
		Flags|=(((uint)flag)<<(edge+NL_PATCH_SMOOTH_FLAG_SHIFT));
	}

	/**
	  * Get the smooth flag for the n-th edge. Return true if this edge must by smoothed, false else.
	  */
	bool getSmoothFlag (uint edge) const
	{
		// Test it
		return ((Flags&(1<<(edge+NL_PATCH_SMOOTH_FLAG_SHIFT)))!=0);
	}


	/// \name Subdivision / ForCollision.
	// @{
	/** Add triangles to triangles array which intersect the bbox.
	 * NB: this method use a convex hull subdivion to search in O(logn) what part of the patch to insert.
	 * \param patchId the id of this patch, used to fill triangles.
	 * \param bbox the bbox to test against.
	 * \param triangles array to be filled (no clear performed, elements added).
	 * \param tileTessLevel 0,1 or 2  size of the triangles (2*2m, 1*1m or 0.5*0.5m). Level of subdivision of a tile.
	 */
	void		addTrianglesInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel) const;

	/** Fill a CPatchQuadBlock, from its required PatchId.
	 * nlassert(PatchId size is less than NL_PATCH_BLOCK_MAX_QUAD)
	 */
	void		fillPatchQuadBlock(CPatchQuadBlock &quadBlock) const;

	/** Add CPatchBlockIdent to CPatchBlockIdent array which intersect the bbox.
	 * NB: this method use a convex hull subdivion to search in O(logn) what part of the patch to insert.
	 * \param patchId the id of this patch, used to fill PatchBlockIdent.
	 * \param bbox the bbox to test against.
	 * \param paBlockIds array to be filled (no clear performed, elements added).
	 */
	void		addPatchBlocksInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds) const;


	/** From the current tesselation of this patch, and a UV in this patch, return tesselated position.
	 */
	CVector		getTesselatedPos(CUV uv) const;


	/** From the current tesselation of this patch, append to the list of leaves faces.
	 */
	void		appendTessellationLeaves(std::vector<const CTessFace*>  &leaves) const;


	// @}


	/// \name Lightmap get interface.
	// @{

	/// Get the lumel under the position.
	uint8		getLumel(const CUV &uv) const;

	/** Append lights under the position to pointLightList.
	 *	Notice that the PointLight are ensured to be actually CPointLightNamed.
	 */
	void		appendTileLightInfluences(const CUV &uv,
		std::vector<CPointLightInfluence> &pointLightList) const;

	/** For CTextureFar, compute current TLI Lightmap at tile level. array should be allocated of at least
	 *  sqr(NL_MAX_TILES_BY_PATCH_EDGE+1).
	 */
	void		computeCurrentTLILightmapDiv2(NLMISC::CRGBA *array) const;

	// @}


	/// \name Tiles get interface.
	// @{

	/// Get the lumel under the position.
	CTileElement	*getTileElement(const CUV &uv);

	// @}

public:

	// Is the patch clipped? true if not yte compiled
	bool			isRenderClipped() const;


	// get the according vertex for a corner. use wisely
	const CTessVertex	*getCornerVertex(uint corner)
	{
		return BaseVertices[corner];
	}


public:

	/// \name VB Allocator mgt.
	// @{

	// delete all VB allocated in VertexBuffers, according to Far0 and Far1. Do not Test isRenderClipped() state.
	// With VB, allocate to he faces array.
	void		deleteVBAndFaceVector();

	// allocate all VB, according to Far0 and Far1. Do not Test isRenderClipped() state.
	// With VB, allocate to he faces array.
	void		allocateVBAndFaceVector();

	// fill all VB, according to Far0, Far1 and CTessFace VBInfos. Do not Test isRenderClipped() state.
	// Do not fill a VB if reallocationOccurs().
	void		fillVB();

	// if isRenderClipped()==false, fillVB().
	void		fillVBIfVisible();

	// delete Far1 VB allocated in VertexBuffers. do it only if Far1==true. Do not Test isRenderClipped() state.
	// With VB, allocate to he faces array.
	void		deleteVBAndFaceVectorFar1Only();

	// allocate Far1 VB, . do it only if Far1==true. Do not Test isRenderClipped() state.
	// With VB, allocate to he faces array.
	void		allocateVBAndFaceVectorFar1Only();

	// fill Far0 VB according CTessFace VBInfos and Far0 (do not fill if !Far0). Do not Test isRenderClipped() state.
	// Do not fill a VB if reallocationOccurs().
	void		fillVBFar0Only();
	// same for Far1
	void		fillVBFar1Only();


	// fill DLM Uv (ie UV1) for Far0 and Far1 VB only. NB: do not fill Far0 if !Far0 (idem fro Far1).
	// Do not Test isRenderClipped() state. Do not fill a VB if reallocationOccurs().
	void		fillVBFarsDLMUvOnly();
	void		fillFar0DLMUvOnlyVertexListVB(CTessList<CTessFarVertex>  &vertList);
	void		fillFar1DLMUvOnlyVertexListVB(CTessList<CTessFarVertex>  &vertList);


	// For Debug only.
	void		debugAllocationMarkIndices(uint marker);


	// Because of refine... tessBlock FaceVector may have been deleted, this method do nothing if isRenderClipped().
	// If not, recreate FaceVector for this tessBlock only, according to Far0 and Far1.
	void		recreateTessBlockFaceVector(CTessBlock &block);


	// @}


public:

	/// \name MicroVegetation
	// @{

	/// Delete any vegetable Ig still existing in this patch.
	void		deleteAllVegetableIgs();

	/// Recreate any vegetable block (as possible) in this patch. (useful for edition)
	void		recreateAllVegetableIgs();

	// @}


	/// \name TileLightInfluences
	// @{
	/** make a valid empty array of TileLightInfluences (ie resized to good size, but with empty
	 *	light influences
	 */
	void		resetTileLightInfluences();
	// @}


	/// \name UpdateLighting Management
	// @{

	/// For lighting update, insert this before patchNext (CiruclarList). textNext must be !NULL
	void		linkBeforeNearUL(CPatch *patchNext);
	/// For lighting update, unlink (CiruclarList)
	void		unlinkNearUL();
	/// For lighting update, get Next (CiruclarList). If ==this, then list is empty
	CPatch		*getNextNearUL() const {return _ULNearNext;}

	/// get the number of Near TessBlocks. Actually OrderS/2*OrderT/2.
	uint		getNumNearTessBlocks() const {return TessBlocks.size();}

	/**
	 *	recompute the near lightmap of tessBlock "numTb".
	 *	return the number of pixels updated by computing of this tessBlock.
	 *	Actually 0 if the tessBlock lightmap is not computed, or 100
	 *	(NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE) pixels.
	 */
	uint		updateTessBlockLighting(uint numTb);

	// @}


	/// \name Dynamic Lighting Management
	// @{

	/** begin Dynamic light Process.
	 *	reset texture To Black (if needed)
	 *	Called by CLandscape. _DLMContext must exist
	 */
	void		beginDLMLighting();
	/** Process a Dynamic light, creating the DLMContext if necessary. Increment RefCount.
	 *	Called by CLandscape.
	 */
	void		processDLMLight(CPatchDLMPointLight &pl);
	/** end Dynamic light Process, deleting the DLMContext if necessary.
	 *	NB: _DLMContext->compileLighting() is not called, since done during render phase.
	 *	Called by CLandscape. _DLMContext must exist
	 */
	void		endDLMLighting();

	// @}


	/// Get number of TileMaterial created in this Patch
	uint		getTileMaterialRefCount() const {return MasterBlock.TileMaterialRefCount;}

// Private part.
private:
/*********************************/

	friend	class CTessFace;
	friend	class CZone;
	friend	class CLandscapeVegetableBlock;
	friend	class CPatchRdrPass;

	CZone			*Zone;

	// Number of this patch in the zone. valid at compile Time.
	uint16			PatchId;
	// Tile Order for the patch.
	uint8			OrderS, OrderT;

	// This is especially for Pacs. false by default, and used by CZone::refineAll() and CZone::excludePatchFromRefineAll().
	bool			ExcludeFromRefineAll;

	// For this patch, which level is required for a face to be inserted in the secondary TessBlocks (and not the masterblock)??
	sint			TessBlockLimitLevel;
	// For this patch, which level is required for a face to be a valid Tile??
	sint			TileLimitLevel;
	// For this patch, which level is required for a face to be a "square" face (not rectangular)??
	sint			SquareLimitLevel;
	// The Base Size*bumpiness of the patch (/2 at each subdivide).
	float			ErrorSize;
	// The root for tesselation.
	CTessFace		*Son0, *Son1;
	// The base vertices.
	CTessVertex		*BaseVertices[4];
	// The base Far vertices (always here!!).
	CTessFarVertex	BaseFarVertices[4];


	// Local info for CTessFace tiles. CPatch must setup them at the beginning at refine()/render().
	// For Far Texture coordinates.
	float			Far0UScale, Far0VScale, Far0UBias, Far0VBias;
	float			Far1UScale, Far1VScale, Far1UBias, Far1VBias;

	/**
	  * Flags NL_PATCH_FAR0_ROTATED and NL_PATCH_FAR1_ROTATED
	  *		NL_PATCH_FAR0_ROTATED for Far0, NL_PATCH_FAR1_ROTATED for Far1
	  *		If the flag is set, the far texture of the patch is rotated of 1
	  *		(to the left of course)
	  *
	  * Flags NL_PATCH_SMOOTH_FLAG_MASK
	  *		4 flag for smooth edge. Same as CPatchInfo::CBindInfo shifted by (<<NL_PATCH_SMOOTH_FLAG_SHIFT).
	  *		See CPatchInfo::CBindInfo::Flags for details.
	  */
	uint8			Flags;


	// Info for alpha transition with Far1.
	float			TransitionSqrMin;
	float			OOTransitionSqrDelta;

	/*
		Cache Optim Note:
		NB: for faster Cache Access during preRender(), you must leave those variables packed like this

		Far0
		Far1
		_PatchRdrPassFar0
		_NextRdrFar0
		_PatchRdrPassFar1
		_NextRdrFar1
		NumRenderableFaces
		TessBlocks (for size() )
		MasterBlock.Far0FaceVector
		MasterBlock.Far1FaceVector

		NB: Far0FaceVector and Far1FaceVector are accessed during renderFar*() (not preRender()) but they must be near
		to TessBlocks (for size() )
	*/

	// Current Far Level computed in preRender()
	sint			Far0;			// The level of First Far: 0,1,2 or 3. 0 means Tile.
	sint			Far1;			// The level of second Far, for transition: 1,2 or 3. 0 means none.

	// The render Pass of Far0 and Far1.
	CPatchRdrPass	*_PatchRdrPassFar0;
	CPatch			*_NextRdrFar0;
	CPatchRdrPass	*_PatchRdrPassFar1;
	CPatch			*_NextRdrFar1;

	/// \name Block renders.
	// @{
	// Tells how many Renderable Face this Patch has. updated in append/remove/FaceToRenderList()
	sint						NumRenderableFaces;
	// The 2*2 block render. For memory optimisation, none is allocated when no faces need it.
	// There is (OrderT/2)*(OrderS/2) TessBlocks.
	NLMISC::CObjectVector<CTessBlock>	TessBlocks;
	// The block render of far only. Only Far faces bigger than a block are inserted here.
public: // tmp
	CTessBlock					MasterBlock;
private:
	// The counter of faces which need TessBlocks (FarFaces, TileMaterial and FarVertices). When 0, the vector is contReset()-ed.
	sint						TessBlockRefCount;
	// @}


	/// \name Vegetables.
	// @{
	/// list of vegetable clipBlocks, created/destroyed at same time as TessBlocks.
	std::vector<CVegetableClipBlock*>	VegetableClipBlocks;
	// @}


	/**
	  *  Stream version of the class.
	  */
	static uint32	_Version;

private:
	// Guess.
	void			computeDefaultErrorSize();
	// based on BaseVertices, recompute positions, and Make Face roots Son0 and Son1.
	void			makeRoots();
	// Guess. For bind() reasons.
	CTessFace		*getRootFaceForEdge(sint edge) const;
	// Guess. For bind() reasons. return the vertex 0 of edge.
	CTessVertex		*getRootVertexForEdge(sint edge) const;
	void			changeEdgeNeighbor(sint edge, CTessFace *to);


	/// \name RenderList mgt.
	// @{

	// reset all list of MasterBlock.
	void			resetMasterBlock();
	// simply clear the tessnbloc array and reset cout to 0.
	void			clearTessBlocks();
	// add a ref to tess blocks, allocate them if necessary.
	void			addRefTessBlocks();
	// dec a ref to tess blocks, destroy them if necessary.
	void			decRefTessBlocks();
	// UGLY SIDE EFFECT: when refcount TessBlocks RefCount reach 0, tessblockas are deleted. think of it in tesselation.cpp.


	// Retrieve the tessblockId, depending on face info.
	uint			getNumTessBlock(CTessFace *face);
	// FarVertType.
	enum			TFarVertType {FVMasterBlock=0, FVTessBlock, FVTessBlockEdge};
	// Retrieve the tessblockId, depending on a ParamCoord.
	void			getNumTessBlock(CParamCoord pc, TFarVertType &type, uint &numtb);


	// If pathc is visible, force deletion of this TessBlock.
	void			dirtTessBlockFaceVector(CTessBlock &block);


	// For rdr. Insert in the GOOD TessBlock the face (depending on level, patchcoordinates etc...)
	// call appendFaceToTileRenderList() to insert his TileFaces into the good renderList.
	void			appendFaceToRenderList(CTessFace *face);
	// Remove a face and his tileface from the patch renderlist.
	// call removeFaceFromTileRenderList() to insert his TileFaces into the good renderList.
	void			removeFaceFromRenderList(CTessFace *face);
	// for changePatchTexture, insert just the TileFace into the good render List.
	void			appendFaceToTileRenderList(CTessFace *face);
	void			removeFaceFromTileRenderList(CTessFace *face);
	// For refreshTesselationGeometry() only. enlarge the TessBlock (if any) with face->V*->EndPos.
	void			extendTessBlockWithEndPos(CTessFace *face);

	// Set/Unset (to NULL) a TileMaterial from the TessBlocks. Material must exist for both functions.
	// And TileS/TileT must be OK.
	void			appendTileMaterialToRenderList(CTileMaterial *tm);
	void			removeTileMaterialFromRenderList(CTileMaterial *tm);

	// Add/Remove FarVertices. Use fv->PCoord to know where.
	void			appendFarVertexToRenderList(CTessFarVertex *fv);
	void			removeFarVertexFromRenderList(CTessFarVertex *fv);
	// Add/Remove NearVertices. Use tileMat to know where.
	void			appendNearVertexToRenderList(CTileMaterial *tileMat, CTessNearVertex *nv);
	void			removeNearVertexFromRenderList(CTileMaterial *tileMat, CTessNearVertex *nv);

	// @}



	/// \name Texture mgt.
	// @{
	// For CTessFace::computeMaterial(). Return the render pass for this material, given the number of the tile, and the
	// desired pass. NULL may be returned if the pass is not present (eg: no additive for this tile...).
	CPatchRdrPass	*getTileRenderPass(sint tileId, sint pass);
	// For CTessFace::computeMaterial(). Return the orient/scalebias for the tile in the patchtexture, and the
	// desired pass (and the desired stage: RGB/Alpha).
	void			getTileUvInfo(sint tileId, sint pass, bool alpha, uint8 &orient, CVector &uvScaleBias, bool &is256x256, uint8 &uvOff);
	// @}

	// Tile LightMap mgt.
	// @{
	// for a given tile (accessed from the (ts,tt) coordinates), compute a lightmap if necessary, and get a RenderPass.
	void		getTileLightMap(uint ts, uint tt, CPatchRdrPass *&rdrpass);
	// get uvInfo for tile. NB: ts,tt form because simpler.
	void		getTileLightMapUvInfo(uint ts, uint tt, CVector &uvScaleBias);
	// release the tile lightmap. NB: ts,tt form because simpler.
	void		releaseTileLightMap(uint ts, uint tt);

	// Compute the Lightmap for 2x2 tiles. => 10x10 pixels. ts, tt is in [0,OrderS], [0, OrderT].
	void		computeNearBlockLightmap(uint ts, uint tt, NLMISC::CRGBA	*lightText);
	void		computeTileLightmapPixelAroundCorner(const NLMISC::CVector2f &stIn, NLMISC::CRGBA *dest, bool lookAround);

	// Compute a lightmap for a tile (ts,tt). 4x4 lumels are processed. NB: result= lumel*userColor.
	void		computeTileLightmap(uint ts, uint tt, NLMISC::CRGBA *dest, uint stride);
	// Compute a lightmap for an edge of a tile. 1x4 lumels. "edge" say what edge of the tile to compute.
	// pixels will be written in (dest+i*stride), where i vary from 0 to 3 or 3 to 0 (according to "inverse").
	void		computeTileLightmapEdge(uint ts, uint tt, uint edge, NLMISC::CRGBA *dest, uint stride, bool inverse);
	// Compute a lightmap just for a pixel (s,t) of a tile (ts,tt). (s,t) E [0;3], [0;3].
	void		computeTileLightmapPixel(uint ts, uint tt, uint s, uint t, NLMISC::CRGBA *dest);


	// Methods for automatic Lighting. NB: result= lumel only (no TileColors).
	void		computeTileLightmapAutomatic(uint ts, uint tt, NLMISC::CRGBA *dest, uint stride);
	void		computeTileLightmapEdgeAutomatic(uint ts, uint tt, uint edge, NLMISC::CRGBA *dest, uint stride, bool inverse);
	void		computeTileLightmapPixelAutomatic(uint ts, uint tt, uint s, uint t, NLMISC::CRGBA *dest);
	// Methods for Precomputed Lighting. NB: result= lumel only (no TileColors).
	void		computeTileLightmapPrecomputed(uint ts, uint tt, NLMISC::CRGBA *dest, uint stride);
	void		computeTileLightmapEdgePrecomputed(uint ts, uint tt, uint edge, NLMISC::CRGBA *dest, uint stride, bool inverse);
	void		computeTileLightmapPixelPrecomputed(uint ts, uint tt, uint s, uint t, NLMISC::CRGBA *dest);
	// Methods to modulate dest with TileColors. NB: A unmodified.
	void		modulateTileLightmapWithTileColors(uint ts, uint tt, NLMISC::CRGBA *dest, uint stride);
	void		modulateTileLightmapEdgeWithTileColors(uint ts, uint tt, uint edge, NLMISC::CRGBA *dest, uint stride, bool inverse);
	void		modulateTileLightmapPixelWithTileColors(uint ts, uint tt, uint s, uint t, NLMISC::CRGBA *dest);
	// get the tileColors at the corners of the tile. corner order: 0,0; 1,0; 0,1; 1,1. NB: A undefined.
	void		getTileTileColors(uint ts, uint tt, NLMISC::CRGBA corners[4]);


	// get the current TLIColor given a TLI coordinate (in (0..OrderS/2+1, 0..OrderT/2+1) )
	// NB: returned color is modulated by landscape material and precomputed diffuse factor
	CRGBA		getCurrentTLIColor(uint x, uint y) const;
	// get the current TLIColors at the corners of the tile (according to pointLights current colors)
	// corner order: 0,0; 1,0; 0,1; 1,1. NB: A undefined.
	void		getCurrentTileTLIColors(uint ts, uint tt, NLMISC::CRGBA corners[4]);
	// Methods to add dest with result of TLI lighting. NB: A unmodified.
	void		addTileLightmapWithTLI(uint ts, uint tt, NLMISC::CRGBA *dest, uint stride);
	void		addTileLightmapEdgeWithTLI(uint ts, uint tt, uint edge, NLMISC::CRGBA *dest, uint stride, bool inverse);
	void		addTileLightmapPixelWithTLI(uint ts, uint tt, uint s, uint t, NLMISC::CRGBA *dest);

	// @}


	// Recompute of new Far Values, according to globals. Don't erase Far0 and Far1.
	void		computeNewFar(const NLMISC::CBSphere &patchSphere, sint &newFar0, sint &newFar1);


	// For Render. Those methods compute the vertices for Driver (in CTessFace::Current*VB).
	void		fillFar0VertexVB(CTessFarVertex *pVert);
	void		fillFar1VertexVB(CTessFarVertex *pVert);
	void		fillTileVertexVB(CTessNearVertex *pVert);
	void		fillFar0VertexListVB(CTessList<CTessFarVertex>  &vertList);
	void		fillFar1VertexListVB(CTessList<CTessFarVertex>  &vertList);
	void		fillTileVertexListVB(CTessList<CTessNearVertex> &vertList);
	// For Render. Those methods allocate/delete vertices in VB.
	void		updateFar0VBAlloc(CTessList<CTessFarVertex>  &vertList, bool alloc);
	void		updateFar1VBAlloc(CTessList<CTessFarVertex>  &vertList, bool alloc);
	void		updateTileVBAlloc(CTessList<CTessNearVertex>  &vertList, bool alloc);
	void		updateVBAlloc(bool alloc);
	// For Debug Allcoation only.
	void		debugAllocationMarkIndicesFarList(CTessList<CTessFarVertex>  &vertList, uint marker);
	void		debugAllocationMarkIndicesNearList(CTessList<CTessNearVertex>  &vertList, uint marker);
	// For Render. Allocate / Fill FaceVector, according to Far0/Far1.
	void		createFaceVectorFar1();
	void		deleteFaceVectorFar1();
	void		createFaceVectorFar0OrTile();
	void		deleteFaceVectorFar0OrTile();



	// For Refine. Those methods do all the good job, and test if they can allocate the VB.
	void		checkCreateVertexVBFar(CTessFarVertex *pVert);
	void		checkCreateVertexVBNear(CTessNearVertex	*pVert);
	// For Refine. Those methods do all the good job, and test if they can fill the VB.
	void		checkFillVertexVBFar(CTessFarVertex *pVert);
	void		checkFillVertexVBNear(CTessNearVertex	*pVert);
	// For Refine. Those methods do all the good job, and test if they have to delete the VB.
	void		checkDeleteVertexVBFar(CTessFarVertex *pVert);
	void		checkDeleteVertexVBNear(CTessNearVertex	*pVert);

	// For Render, geomorph / Alpha in software.
	void		computeGeomorphVertexList(CTessList<CTessFarVertex>  &vertList);
	void		computeGeomorphFar0VertexListVB(CTessList<CTessFarVertex>  &vertList);
	void		computeGeomorphAlphaFar1VertexListVB(CTessList<CTessFarVertex>  &vertList);
	void		computeGeomorphTileVertexListVB(CTessList<CTessNearVertex>  &vertList);

	/// \name Subdivision private.
	// @{
	/// build a bbox from the convex hull of a bezier patch, enlarged with noise.
	void		buildBBoxFromBezierPatch(const CBezierPatch &p, CAABBox &ret) const;
	/** recurse subdivide of the bezierPatch.
	 * 3 1st parameters are the parameter of addTrianglesInBBox(). \n
	 * pa is the bezier patch for this subdivision of this patch. \n
	 * s0, s1, t0, t1 represent the part of the bezier patch subdivided. At start, s0=0, s1=OrderS, t0=0, t1=OrderT.
	 */
	void		addTrianglesInBBoxRecurs(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tessLevel,
		const CBezierPatch &pa, uint8 s0, uint8 s1, uint8 t0, uint8 t1) const;
	/** called by addTrianglesInBBoxRecurs(). effective fill the array of triangles from 1 tile at tile coordinates s0,t0.
	 * depending of tessLevel (0,1,2), 2, 8 or 32 triangles are added.  (2*2m, 1*1*m or 0.5*0.5m).
	 * NB: only triangles of quad included in the bbox are added.
	 */
	void		addTileTrianglesInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tessLevel,
		uint8 s0, uint8 t0) const;

	/** recurse method of addPatchBlocksInBBox.
	 */
	void		addPatchBlocksInBBoxRecurs(CPatchIdent paId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds,
		const CBezierPatch &pa, uint8 s0, uint8 s1, uint8 t0, uint8 t1) const;

	/// Used by computeContinousVertex()
	CVector		computeVertexButCorner(float s, float t, bool &onCorner) const;

	// @}


private:



	/// Realtime Bind information.
	// @{

	/// The 4 neighbors zone of this patch (setuped at bind() time). NB: NULL if zone not loaded, or if no patch near us.
	CZone		*_BindZoneNeighbor[4];

	/** Used by bind(). Search in the tesselation the first face which own the edge uv0-uv1. link it with linkTo, and
	 *	return it. NULL if not found (obviously because not so tesselated)
	 */
	CTessFace	*linkTessFaceWithEdge(const NLMISC::CVector2f &uv0, const NLMISC::CVector2f &uv1, CTessFace *linkTo);

	// @}



	/// Noise Geometry.
	// @{


	float		computeDisplaceRawInteger(sint ts, sint tt, sint ms, sint mt) const;
	void		computeDisplaceRawCoordinates(float sTile, float tTile, float s, float t,
	sint &ts, sint &tt, sint &ms, sint &mt) const;
	/** compute the displacement for s,t ([0;OrderS], [0;OrderT])
	 *  (sTile, tTile) choose what NoiseMap to use, and (s,t) choose the coordinate in the patch to compute this NoiseMap.
	 *	Any rotation of the NoiseMap is included in this method.
	 *	NB: s,t does not have to be clamped to ([0;OrderS], [0;OrderT]).
	 */
	float		computeDisplaceRaw(float sTile, float tTile, float s, float t) const;
	/** useful only for computeDisplaceCornerSmooth().
	 *	This method, if nessecary (ie sTile or tTile <0 or >Order), look on his neighbor to compute the value.
	 */
	float		computeDisplaceRawOnNeighbor(float sTile, float tTile, float s, float t) const;


	/** compute the smoothed displacement for s,t ([0;OrderS], [0;OrderT]).
	 */
	float		computeDisplaceInteriorSmooth(float s, float t) const;
	/** compute the smoothed displacement for s,t ([0;OrderS], [0;OrderT]). Special case on edge.
	 */
	float		computeDisplaceEdgeSmooth(float s, float t, sint8 smoothBorderX, sint8 smoothBorderY) const;
	/** compute the smoothed displacement for s,t ([0;OrderS], [0;OrderT]). Special case on corner.
	 */
	float		computeDisplaceCornerSmooth(float s, float t, sint8 smoothBorderX, sint8 smoothBorderY) const;


	/** compute the smoothed normal for s,t ([0;OrderS], [0;OrderT]). Special case on edge.
	 */
	CVector		computeNormalEdgeSmooth(float s, float t, sint8 smoothBorderX, sint8 smoothBorderY) const;
	/** compute the smoothed normal for s,t ([0;OrderS], [0;OrderT]). Special case on corner.
	 */
	CVector		computeNormalCornerSmooth(float	s, float t, sint8 smoothBorderX, sint8 smoothBorderY) const;
	/** same reasoning as in computeDisplaceRawOnNeighbor().
	 */
	CVector		computeNormalOnNeighbor(float s, float t, uint edgeExclude) const;


	/** compute the Final displacement for s,t ([0;1], [0;1]). This is the top call.
	 *	displace.norm() should be <= NL3D_NOISE_MAX.
	 */
	void		computeNoise(float s, float t, CVector &displace) const;

	// @}


	// From tile coordinates, return the tessBlockId, and the id of the material in this tessBlock.
	void		computeTbTm(uint &numtb, uint &numtm, uint ts, uint tt);


	/// Micro-Vegetation.
	// @{

	/// Create / init the vegetableBlock in the corresponding TessBlock. TessBlocks must exist
	void		createVegetableBlock(uint numTb, uint ts, uint tt);
	/// release the vegetableBlock in the corresponding TessBlock. TessBlocks must exist
	void		releaseVegetableBlock(uint numTb);

	/*
		generate the vegetables for a given tile in the vegetable manager.
		instances are added to vegetIg.
		Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	*/
	void		generateTileVegetable(CVegetableInstanceGroup *vegetIg, uint distType, uint ts, uint tt,
		CLandscapeVegetableBlockCreateContext &vbCreateCtx);

	// same as computeTileLightmapPrecomputed(), but brut result, not modified by colorTable.
	void		getTileLumelmapPrecomputed(uint ts, uint tt, uint8 *dest, uint stride);
	/** same as computeTileLightmapPixelPrecomputed, but brut result, not modified by colorTable.
	 *	Actually used for Lightmap get interface.
	 */
	void		getTileLumelmapPixelPrecomputed(uint ts, uint tt, uint s, uint t, uint8 &dest) const;

	// @}


	/// \name UpdateLighting Management
	// @{
	CPatch		*_ULNearPrec;
	CPatch		*_ULNearNext;
	// @}


	/// \name Dynamic Lighting Management
	// @{

	/** The Dynamic LightMap context.
	 *	created only when compiled, AND (when in Near OR (when in Far AND touched by pointLight))
	 *	else NULL.
	 */
	CPatchDLMContext	*_DLMContext;

	/** The reference count for DLMContext. Each TileMaterial created add a reference. Each pointLight which
	 *	touch the patch too.
	 */
	sint				_DLMContextRefCount;

	/// Add a ref count to the DLMContext, creating it if necessary.
	void				addRefDLMContext();
	/// Dec a ref count to the DLMContext, deleting it if refCount== 0.
	void				decRefDLMContext(uint count= 1);

	// @}


private:
	// NB: All global render info are stored in CTessFace class static members....

	// The Patch cache (may be a short list/vector later...).
	static	CBezierPatch	CachePatch;
	// For cahcing.
	static	const CPatch	*LastPatch;

public:
	// unpack the patch into the cache.
	CBezierPatch	*unpackIntoCache() const;

};



} // NL3D


#endif // NL_PATCH_H

/* End of patch.h */
