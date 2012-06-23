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

#ifndef NL_ZONE_H
#define NL_ZONE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/stream.h"
#include "nel/misc/debug.h"
#include "nel/misc/bit_set.h"
#include "nel/3d/tessellation.h"
#include "nel/3d/patch.h"
#include "nel/3d/bezier_patch.h"
#include "nel/3d/point_light_named.h"
#include "nel/3d/point_light_named_array.h"
#include <cstdio>
#include <vector>
#include <map>


namespace NL3D
{


class CZone;
class CLandscape;
class CZoneSymmetrisation;
class CTileBank;

using NLMISC::CAABBoxExt;


// ***************************************************************************
typedef	std::map<uint16, CZone*>			TZoneMap;
typedef	std::map<uint16, CZone*>::iterator	ItZoneMap;


// ***************************************************************************
/**
 * The struct for connectivity of zone vertices.
 */
struct	CBorderVertex
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// The index of vertex in the current zone to bind.
	uint16			CurrentVertex;
	// The neighbor zone Id.
	uint16			NeighborZoneId;
	// The index of vertex in the neighbor zone to bind to CurrentVertex.
	uint16			NeighborVertex;

	void			serial(NLMISC::IStream &f);
};


// ***************************************************************************
/**
 * The struct for building a patch.
 * NB: Different from the one which is stored.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CPatchInfo
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
public:

	/** A bind Info on a edge of a patch.
	 * Entry 0 is only useful for Bind One/One+
	 * Entry 1 is only useful for Bind One/Two+
	 * Entry 2/3 is only useful for Bind One/Four.
	 */
	struct	CBindInfo
	{
		/** The number of patchs on this edge. 0,1, 2 or 4.  0 means no neigbor on this edge. \b 5 is a special code,
		 * which means the same thing than NPatchs==1, but "I am one of the little patch connected to the bigger neigbor".
		 * Like when NPatchs==1, ZoneId, Next[0] and Edge[0] must be valid.
		 */
		uint8			NPatchs;

		/// The neighbor zone of all neigbor patch. Often the same zone as the patch (but on zone border).
		uint16			ZoneId;
		/// The neighbor patch i.
		uint16			Next[4];
		/// On which edge of Nexti we are binded.
		uint8			Edge[4];

	public:
		void			serial(NLMISC::IStream &f);
		CBindInfo() {NPatchs=0;}
	};


public:
	/// \name Patch geometry.
	// @{
	/// The patch coordinates.
	CBezierPatch	Patch;
	/// Tile Order for the patch.
	uint8			OrderS, OrderT;
	/// The Base Size*bumpiness of the patch (/2 at each subdivide). Set to 0, if you want CZone to compute it for you.
	float			ErrorSize;
	/// The base corner vertices indices in the current zone. Used for patch connectivity.
	uint16			BaseVertices[4];

	/** "The don't smooth" flags. For each edge of the patch (0~3), the flag means that this patch mustn't be smoothed
	with its neightbor. The n-th edge links the vertices "n" and "(n+1)%4". The flag for the n-th edge is (1<<n). */
	uint8			Flags;


	/// The orientation of the NoiseMap. 0,1,2,3. This represent a CCW rotation of the NoiseMap.
	uint8			NoiseRotation;

	/// setup NoiseSmooth flags: used for Noise geometry and lighting. NB: convention: corner0==A, corner1==B ...
	void			setCornerSmoothFlag(uint corner, bool smooth);
	bool			getCornerSmoothFlag(uint corner) const;

	// @}


	/// \name Patch texture.
	// @{

	/** The Tiles for this patch. There must be OrderS*OrderT tiles.
	 * They are stored in line first order, from S=0 to 1, and T=0 to 1.
	 */
	std::vector<CTileElement>	Tiles;

	/** The Tile colors for this patch. There must be (OrderS+1)*(OrderT+1) tile colors. Those are the colors at
	 * the corners of the tiles.
	 * They are stored in line first order, from S=0 to 1, and T=0 to 1.
	 */
	std::vector<CTileColor>		TileColors;

	/** The Tile lumels for this patch. There must be (OrderS*4+1)*(OrderT*4+1) tile lumels. Those are lumel value
	 *  in tiles. There is 4x4 lumels by tiles plus last lumels.
	 *  They are stored in line first order, from S=0 to 1, and T=0 to 1.
	 */
	std::vector<uint8>			Lumels;

	/** There is (OrderS/2+1) * (OrderT/2+1) tiles light influence.
	 *	It indicates which static pointLight influence each corner of a TessBlock (block of 2*2 tiles).
	 *
	 *	If size()==0, suppose no light influence. but CZone::retrieve() always return a
	 *	size() == (OrderS/2+1) * (OrderT/2+1).
	 *
	 * They are stored in line first order, from S=0 to 1, and T=0 to 1.
	 */
	std::vector<CTileLightInfluence>		TileLightInfluences;

	// @}

	/// \name Smooth flags methods

	/**
	  * Set the smooth flag for the n-th edge. flag is false if this edge must by smoothed, true else.
	  */
	void setSmoothFlag (uint edge, bool flag)
	{
		// Erase it
		Flags&=~(1<<edge);

		// Set it
		Flags|=(((uint)flag)<<edge);
	}

	/**
	  * Get the smooth flag for the n-th edge. Return true if this edge must by smoothed, false else.
	  */
	bool getSmoothFlag (uint edge)
	{
		// Test it
		return ((Flags&(1<<edge))!=0);
	}

	/** Get neighbor tile across a edge
	  *
	  * \param patchid is the id of this patch
	  * \param edge is the edge shared with the neigbor
	  * \param position is the position over the edge in CCW across the patch.
	  * So if edge == 0, position is oriented like OT
	  * So if edge == 1, position is oriented like OS
	  * So if edge == 2, position is oriented like -OT
	  * So if edge == 3, position is oriented like -OS
	  * \param patchOut will be filled with the output patch id
	  * \param sOut will be filled with the output patch s coordinate
	  * \param tOut will be filled with the output patch t coordinate
	  * \param patchInfos is the vector of all patch info
	  * \return false if no neighbor has been found or tile ratio is not the same than in this patch.
	  */
	bool getNeighborTile (uint patchId, uint edge, sint position, uint &patchOut, sint &sOut, sint &tOut, const std::vector<CPatchInfo> &patchInfos) const;

	/** Adjusts a CPatchInfo array to get a symmetrized / rotated zone with matching oriented tiles.
	  * This method only adjuste tile and vertex color array, does'nt transform vertices.
	  *
	  * Transform an array of patchInfo by a symmetry on OY axis followed by a 90deg CCW rotation (0, 1, 2, 3).
	  *
	  * The method doesn't transform vertices.
	  * If symmetry, the method invert 0-3 and 1-2 vertices indexes to get CCW oriented patches. It will fix bind information.
	  * The method fixes tile and color vertex arrays.
	  * The method fixes tile rotation, 256 cases and tile transistions.
	  *
	  * Return false if something wrong.
	  */
	static bool transform (std::vector<CPatchInfo> &patchInfo, NL3D::CZoneSymmetrisation &zoneSymmetry, const NL3D::CTileBank &bank, bool symmetry, uint rotate, float snapCell, float weldThreshold, const NLMISC::CMatrix &toOriginalSpace);

	// Transform tile and 256 case with rotation and symmetry parameters
	static bool getTileSymmetryRotate (const NL3D::CTileBank &bank, uint tile, bool &symmetry, uint &rotate);
	static bool transformTile (const NL3D::CTileBank &bank, uint &tile, uint &tileRotation, bool symmetry, uint rotate, bool goofy);
	static void transform256Case (const NL3D::CTileBank &bank, uint8 &case256, uint tileRotation, bool symmetry, uint rotate, bool goofy);

	/// \name Patch Binding.
	// @{
	CBindInfo		BindEdges[4];
	// @}


public:
	CPatchInfo()
	{
		ErrorSize= 0;
		// No Rotation / not smooth by default.
		NoiseRotation= 0;
		_CornerSmoothFlag= 0;
	}

private:
	// Noise Smooth flags.
	uint8			_CornerSmoothFlag;

};


// ***************************************************************************
/**
 * The struct for building a zone.
 * NB: Different from the one which is stored.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CZoneInfo
{
	/// zoneId the Unique ID of this zone.
	uint16						ZoneId;
	/// patchs the PatchInfo of this zone.
	std::vector<CPatchInfo>		Patchs;
	/** borderVertices vertices connectivity for this zone. NB: borderVertices must contains the connectivity
	 *	across zones. It is VERY IMPORTANT to setup zone corner connectivity too. A "corner borderVertex" may appear
	 *	3 times here. One for each other zone of the corner.
	 */
	std::vector<CBorderVertex>	BorderVertices;

	/** List of PointLights that may influences Patchs and objects walking on them.
	 *	Must be of good size regarding to Patchs[i].TileLightInfluences data
	 */
	std::vector<CPointLightNamed>	PointLights;
};


// ***************************************************************************
/**
 * A landscape zone.
 * There is 2 ways for building a zone:
 *	- use build(). (then you can use serial to save the zone, don't need to compile() the zone).
 *	- use serial() for loading this zone.
 *
 * Before a zone may be rendered, it must be compile()-ed, to compile and bind patch, to make vertices etc...
 *
 * NB: you must call release() before deleting a compiled zone. (else assert in destruction).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CZone
{
public:
	// The stored patch structure for compile() - ation.
	struct	CPatchConnect
	{
		// NB: same meanings than in CPatchInfo.
		uint8			OldOrderS, OldOrderT;
		float			ErrorSize;
		uint16			BaseVertices[4];
		CPatchInfo::CBindInfo		BindEdges[4];

	public:
		void			serial(NLMISC::IStream &f);
	};

public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/// Constructor
	CZone();
	/// Destructor
	~CZone();


	/** Build a zone.
	 * This method do:
	 *	- compress the patchs coordinates.
	 *	- build the patchs of the zone, but doesn't compile() them.
	 *	- compress Lumels.
	 *	- sort PointLights by name, and hence remap TileLightInfluences coordinates.
	 *
	 * NB: cannot build on a compiled zone. must release the zone before....
	 *
	 * \param numVertices maximize the numgber of vertices used by this zone with this value.
	 */
	void			build(const CZoneInfo &zoneInfo, uint32 numVertices=0);


	/** Build a zone. Deprecated.
	 *	Should use build(CZoneInfo &) instead. see this method
	 */
	void			build(uint16 zoneId, const std::vector<CPatchInfo> &patchs, const std::vector<CBorderVertex> &borderVertices, uint32 numVertices=0);


	/** Build a copy of a zone.
	 * This method do a copy of zone (should be builded but maybe not compiled).
	 *
	 * NB: cannot build on a compiled zone. must release the zone before....
	 */
	void			build(const CZone &zone);


	/** Retrieve zone patchinfo.
	 * This method uncompress the patchs coordinates and all info into the patch info/borderVertices.
	 * Warning!!! Due to compression, data won't be the same as those given in build().
	 * Same remark for TileLightInfluences, due to light sorting.
	 *
	 */
	void			retrieve(CZoneInfo &zoneInfo);


	/** Retrieve zone patchinfo. Deprecated.
	 *	Should use retrieve(CZoneInfo &) instead. see this method.
	 */
	void			retrieve(std::vector<CPatchInfo> &patchs, std::vector<CBorderVertex> &borderVertices);



	/** Debug a zone, print binds in display.
	 *
	 */
	void			debugBinds(FILE *f= stdout);


	/** Compile a zone. Make it usable for clip()/refine()/render().
	 * This method do:
	 *	- attach this to loadedZones.
	 *	- create/link the base vertices (internal..), according to present neigbor zones.
	 *	- compile() the patchs.
	 *	- bind() the patchs.
	 *	- rebindBorder() on neighbor zones.
	 *
	 * A zone must keep a pointer on a landscape, for texture management.
	 * NB: assert if already compiled.
	 * assert if zone already exist in loadedZones.
	 */
	void			compile(CLandscape *landscape, TZoneMap &loadedZones);

	/** Release a zone.
	 * This method do:
	 *	- detach this zone to loadedZones.
	 *	- destroy/unlink the base vertices (internal..), according to present neigbor zones.
	 *	- unbind() the patchs.
	 *	- release() the patchs.
	 *	- rebindBorder() on neighbor zones.
	 *
	 * NB: no-op if not compiled.
	 */
	void			release(TZoneMap &loadedZones);


	/** Load/save a zone.
	 * Save work even if zone is not compiled, but load must be done on a not compiled zone...
	 */
	void			serial(NLMISC::IStream &f);


	/**
	 * Update and refresh a patch texture.
	 * Useful for Tile edition. Even if patch is in tile mode, it is refreshed...
	 * \param numPatch the index of patch in this zone which will receive his new texture. assert if bad id.
	 * \param tiles the patch texture. assert if not of good size (OrderS*OrderT). Can be NULL if you don't want to change the patch texture.
	 * \param colors the patch texture. assert if not of good size ((OrderS+1)*(OrderT+1)). Can be NULL if you don't want to change the patch colors.
	 */
	void			changePatchTextureAndColor (sint numPatch, const std::vector<CTileElement> *tiles, const std::vector<CTileColor> *colors);


	/**
	 * refresh the geometry (re-compute vertices).
	 * Useful for Tile Noise edition. Do it after calling changePatchTextureAndColor().
	 *	NB: a refreshTesselationGeometry() should be done on All patchs, and all direct neighbors of this patch (including
	 *	patchs on corners).
	 *	WARNING: specially coded for Tile edition. Result is not perfect:
	 *		- only EndPos of tesselation is modified. because Tile edition always subdivide at max.
	 *		- Pos=EndPos. because TileEdition always subdivide at max, and don't refine...
	 *		- TessBlocks BSphere may be too big. Clip is worse (too big), but doesn't matter.
	 * \param numPatch the index of patch in this zone. assert if bad id.
	 */
	void			refreshTesselationGeometry(sint numPatch);


	/**
	 * Get a patch texture.
	 * Return the tile array.
	 * \param numPatch the index of patch in this zone which will get his texture. assert if bad id.
	 * \param
	 * \return The tiles the patch texture. The size should be OrderS*OrderT.
	 * \see getPatch()
	 */
	const std::vector<CTileElement> &getPatchTexture(sint numPatch) const;

	/**
	 * Get a patch colors
	 * Return the color array.
	 * \param numPatch the index of patch in this zone which will get his colors. assert if bad id.
	 * \param
	 * \return The tiles the patch colors. The size should be (OrderS+1)*(OrderT+1).
	 * \see getPatch()
	 */
	const std::vector<CTileColor> &getPatchColor(sint numPatch) const;

	/**
	 * Set the zone tile color to monochrome or not and apply multiplier factor
	 * Set all tile colors of all patch of this zone to monochrome or not and apply a multiplier factor
	 * convertion to monochrome is done by that formula : newR = newG = newB = 0.28 * R + 0.59 * G + 0.13 * B;
	 * for monochrome the factor is applied like for the color mode
	 */
	void setTileColor(bool monochrome, float factor);

	/**
	 *  Get the landscape in which is placed this zone. If no landscape, return NULL.
	 *
	 *	\return the pointer of the landscape of the zone or NULL if the zone hasn't be compiled.
	 */
	CLandscape*		getLandscape () const
	{
		return Landscape;
	}

	// NB: for all those function, CTessFace static rendering context must be setup.
	/// Clip a zone. To know if must be rendered etc... A zone is IN if in BACK of at least one plane of the pyramid.
	void			clip(const std::vector<CPlane>	&pyramid);
	/// PreRender a zone (if needed).
	void			preRender();
	// release Far render pass/reset Tile/Far render. Delete also VB, and FaceVectors
	void			resetRenderFarAndDeleteVBFV();
	/// For changing TileMaxSubdivision. force tesselation to be under tile.
	void			forceMergeAtTileLevel();

	/// force Refine a zone.
	void			refineAll();
	/// This is especially for Pacs. exlude a patch to be refineAll()ed.
	void			excludePatchFromRefineAll(uint patch, bool exclude);
	/** This is especially for Pacs. see CLandscape desc.
	 */
	void			averageTesselationVertices();

	// Accessors.
	const CVector	&getPatchBias() const {return PatchBias;}
	float			getPatchScale() const {return PatchScale;}
	bool			compiled() const {return Compiled;}
	uint16			getZoneId() const {return ZoneId;}
	sint			getNumPatchs() const {return (sint)Patchs.size();}
	// Return the Bounding Box of the zone.
	const CAABBoxExt	&getZoneBB() const {return ZoneBB;}

	/**
	 * Get a read only patch pointer.
	 *
	 * \param patch the index of patch to get.
	 * \return A patch pointer in read only.
	 */
	const CPatch	*getPatch(sint patch) const {nlassert(patch>=0 && patch<(sint)Patchs.size()); return &(Patchs[patch]);}

	/**
	 * Get a read only patch connect pointer.
	 *
	 * \param patch the index of patch to get.
	 * \return A patch pointer in read only.
	 */
	const CPatchConnect	*getPatchConnect(sint patch) const
		{nlassert(patch>=0 && patch<(sint)Patchs.size()); return &(PatchConnects[patch]);}


	/** apply a landscape heightfield on a zone (modification of Z control points values).
	 *  NB: this is done in Landscape addZone(), before compile().
	 * \param landScape the landscape which gives Z delta values, for a x,y point.
	 */
	void			applyHeightField(const CLandscape &landScape);

	/** Debug purpose only : setup the colors of the patch of this zone so that it shows which tiles
	  * have vegetable disabled, or are above, below water.
	  * User provides a table with 4 colors for each state :
	  * color 0 = above water
	  * color 1 = underwater
	  * color 2 = intersect water
	  * color 3 = vegetable disabled
	  */
	void setupColorsFromTileFlags(const NLMISC::CRGBA colors[4]);

	/** Copy the tiles flags from a src patch to a patch of this zone.
	  * the patch must match of course...
	  */
	void copyTilesFlags(sint destPatchId, const CPatch *srcPatch);

	/** Get the Bounding spehere of a patch. Stored in Zone as an array and not in CPatch for Fast Memory access
		consideration during clipping.
		Work only when zone compiled.
	*/
	const CBSphere	&getPatchBSphere(uint patch) const;

	/// Is the patch clipped (ie not visible). crash if bad Id.
	bool			isPatchRenderClipped(uint patch) const {return _PatchRenderClipped.get(patch);}

// Private part.
private:
/*********************************/
	// A smartptrisable vertex.
	struct	CTessBaseVertex : public NLMISC::CRefCount
	{
		CTessVertex		Vert;
	};

	// Zone vertices.
	typedef	NLMISC::CSmartPtr<CTessBaseVertex>	PBaseVertex;
	typedef	std::vector<PBaseVertex>			TBaseVerticesVec;


private:
	// The lanscape which own this zone. Useful for texture management.
	// Filled at compilation only.
	CLandscape		*Landscape;

	// Misc.
	uint16			ZoneId;
	bool			Compiled;
	CAABBoxExt		ZoneBB;
	CVector			PatchBias;
	float			PatchScale;

	// The number of vertices she access (maybe on border).
	sint32				NumVertices;
	// The smartptr on zone vertices.
	TBaseVerticesVec	BaseVertices;
	// The list of border vertices.
	std::vector<CBorderVertex>	BorderVertices;
	// NB: No problem on corners, since zones are compile()-ed with knowledge of neighbors.

	// The patchs.
	std::vector<CPatch>			Patchs;
	std::vector<CPatchConnect>	PatchConnects;
	// Clipped States and BSphere stored here and not in CPatch for faster memory access during clip
	std::vector<CBSphere>		_PatchBSpheres;
	NLMISC::CBitSet				_PatchRenderClipped;
	NLMISC::CBitSet				_PatchOldRenderClipped;


	/** List of PointLights that may influences Patchs and objects walking on them.
	 */
	CPointLightNamedArray			_PointLightArray;


private:
	friend	class CLandscape;
	friend	class CTessFace;
	// Should do this, for texture mgt.
	friend	class CPatch;

	sint			ClipResult;
	enum	TClipResult {ClipIn= 0, ClipOut= 1, ClipSide= 2};


private:
	/**
	 * Force border patchs (those who don't bind to current zone) to re bind() them, using new neighborood.
	 * no-op if zone is not compiled.
	 */
	void			rebindBorder(TZoneMap &loadedZones);

	PBaseVertex		getBaseVertex(sint vert) const {return BaseVertices[vert];}
	CPatch			*getPatch(sint patch) {nlassert(patch>=0 && patch<(sint)Patchs.size()); return &(Patchs[patch]);}
	static CPatch	*getZonePatch(TZoneMap &loadedZones, sint zoneId, sint patch);
	// Bind the patch with ones which are loaded...
	static void		unbindPatch(CPatch &pa);
	static void		bindPatch(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc, bool rebind);
	// Is the patch on a border of this zone???
	bool			patchOnBorder(const CPatchConnect &pc) const;

	// compute AABBox, PatchBias and PatchScale, from a bbox.
	void			computeBBScaleBias(const CAABBox	&bb);


	// For CPatch: build a bindInfo.
	void			buildBindInfo(uint patchId, uint edge, CZone *neighborZone, CPatch::CBindInfo	&paBind);

	// Patch Array Clip
	void			clipPatchs(const std::vector<CPlane>	&pyramid);
};


} // NL3D


#endif // NL_ZONE_H

/* End of zone.h */
