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

#ifndef NL_TEXTURE_FAR_H
#define NL_TEXTURE_FAR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rect.h"
#include "nel/3d/texture.h"
#include "nel/3d/tile_far_bank.h"

/* NB: those Values work only if NL_MAX_TILES_BY_PATCH_EDGE is 16.
	asserted in the cpp.
	Else must change NL_MAX_FAR_PATCH_EDGE, NL_NUM_FAR_PATCH_EDGE_LEVEL, and NL_NUM_FAR_RECTANGLE_RATIO
*/

// The number of different Level to allocate. only possible to allocate 16, 8, 4 or 2 tiles patch (1x? is impossible)
#define	NL_NUM_FAR_PATCH_EDGE_LEVEL		(4+NL_NUM_PIXELS_ON_FAR_TILE_EDGE_SHIFT)

// Define the max number of pixel per edge for a far texture
#define NL_MAX_FAR_PATCH_EDGE (16*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)					// Max is 16x16 tiles for 4 pix/tiles.

// Define the min number of pixel per edge for a far texture
#define NL_MIN_FAR_PATCH_EDGE 2														// Min is 2x2 tiles for 1 pix/tile

// The max Difference of level between Height and Width
#define	NL_NUM_FAR_RECTANGLE_RATIO	4												// 16x16, 16x8, 16x4, 16x2
#define	NL_MAX_FAR_RECTANGLE_RATIO_SHIFT	(NL_NUM_RECTANGLE_ASPECT_RATIO-1)
#define	NL_MAX_FAR_RECTANGLE_RATIO	(1<<NL_MAX_RECTANGLE_RATIO_SHIFT)				// Max is 16 tiles fo 2 tiles => 8.

// The number of Square of MaxFarEdge.
#define	NL_NUM_FAR_BIGGEST_PATCH_PER_EDGE			8
#define	NL_NUM_FAR_BIGGEST_PATCH_PER_TEXTURE		(NL_NUM_FAR_BIGGEST_PATCH_PER_EDGE*NL_NUM_FAR_BIGGEST_PATCH_PER_EDGE)

// The size of the texture.  8*64 => textureFar of 512*512.
#define	NL_FAR_TEXTURE_EDGE_SIZE					(NL_MAX_FAR_PATCH_EDGE*NL_NUM_FAR_BIGGEST_PATCH_PER_EDGE)


namespace NLMISC
{
	class CRGBA;
}

namespace NL3D
{

class CPatch;
class CTileFarBank;
class CTileColor;

/**
 * A CTextureFar is a set of texture used to map a whole patch when it is in far Mode. (ie not in tile mode).
 * A CTextureFar handle several patch texture.\\
 *
 * TODO: keeping the far texture level1 alive when the tile pass in level0 (tile mode), don't erase it.
 * TODO: add an hysteresis to avoid swap of far texture on boundaries of levels
 * TODO: set the upload format in rgba565
 *
 * \author Cyril Corvazier - Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTextureFar : public ITexture
{
public:
	/// Constructor
	CTextureFar();
	virtual ~CTextureFar();


	/** Try to allocate a patch.
	 *	return -1 if error (not enough space)
	 *	return 0 if OK, and success to allocate a place without splitting any square
	 *	else return the size of the max space to split (eg if must a split a 32x32 to allocate a 16x8)
	 */
	sint						tryAllocatePatch (CPatch *pPatch, uint farIndex);


	/**
	 *  Allocate a patch in the CTextureFar, according to its size. nlstop if fails. Must call before tryAllocate()
	 *
	 *  \param pPatch is the pointer to the patch to add in the landscape
	 *  \param farUVScale will receive the scale to use to compute the UV coordinates
	 *  \param farUBias will receive the U Bias to use to compute the UV coordinates
	 *  \param farVBias will receive the V Bias to use to compute the UV coordinates
	 *  \param bRot will receive true if the texture is rotated of 90deg to the left or false.
	 *         You should take care of this value to compute UV coordinates.
	 */
	void						allocatePatch (CPatch *pPatch, uint farIndex, float& farUScale, float& farVScale, float& farUBias, float& farVBias, bool& bRot);

	/**
	 *  Remove a patch in the CTexture Far.
	 */
	void						removePatch (CPatch *pPatch, uint farIndex);

	/**
	 *  Generate the texture. See ITexture::doGenerate().
	 *
	 *  \see ITexture::doGenerate()
	 */
	virtual void				doGenerate(bool async = false);

	/**
	 *	Touch a patch (call touchRect()) and iterate to next .
	 *	\return number of pixels touched. 0 if end() (empty).
	 */
	uint						touchPatchULAndNext();
	void						startPatchULTouch();
	bool						endPatchULTouch() const;


	/// For lighting update, insert this before textNext (CiruclarList). textNext must be !NULL
	void						linkBeforeUL(CTextureFar *textNext);
	/// For lighting update, unlink (CiruclarList)
	void						unlinkUL();
	/// For lighting update, get Next (CiruclarList). If ==this, then list is empty
	CTextureFar					*getNextUL() const {return _ULNext;}


	/// A pointer on the far bank.
	CTileFarBank*				_Bank;


private:

	struct	CPatchIdent
	{
		CPatch	*Patch;
		uint	FarIndex;

		bool operator < (const CPatchIdent &rhs) const
		{
			return (Patch != rhs.Patch) ? Patch < rhs.Patch : FarIndex < rhs.FarIndex;
		}
	};

	struct	CVector2s
	{
		uint16	x, y;

		CVector2s() {}
		CVector2s(uint16 _x, uint16 _y) {x= _x; y= _y;}

		bool operator < (const CVector2s &rhs) const
		{
			return (x != rhs.x) ? x < rhs.x : y < rhs.y;
		}
	};

	/**
	 *  Map of Patchs stored in this texture Far.
	 *  Should be == to _WidthPatches*_HeightPatches
	 */
	typedef	std::map<CPatchIdent, CVector2s>	TPatchToPosMap;
	typedef	std::map<CVector2s, CPatchIdent>	TPosToPatchMap;
	TPatchToPosMap						_PatchToPosMap;
	TPosToPatchMap						_PosToPatchMap;

	/** Lists of empty Space. One for each possible size (64x64, 64x8 etc, but not 64x4 since not possible...)
	 */
	std::list<CVector2s>				_FreeSpaces[NL_NUM_FAR_PATCH_EDGE_LEVEL * NL_NUM_FAR_RECTANGLE_RATIO];


	// allocate search
	bool	getUpperSize(uint &width, uint &height);
	uint	getFreeListId(uint width, uint height);
	void	recursSplitNext(uint width, uint height);

	/**
	 *  Rebuild the patch passed in parameter
	 */
	void rebuildPatch  (const CVector2s texturePos, const CPatchIdent &pid);

	/// From IStreamable
	virtual void	serial(NLMISC::IStream &/* f */) {}

	// Some static buffers
	static NLMISC::CRGBA	_LightmapExpanded[];
	static uint8			_LumelExpanded[];
	static NLMISC::CRGBA	_TileTLIColors[];

	NLMISC_DECLARE_CLASS(CTextureFar);

	/// UpdateLighting. CiruclarList
	CTextureFar					*_ULPrec;
	CTextureFar					*_ULNext;
	// Iterator to the next patch to update.
	TPatchToPosMap::iterator	_ItULPatch;

};

} // NL3D

// For NL3D_drawFarTileInFarTexture external call
struct NL3D_CComputeTileFar
{
public:
	// TileFar pixels
	const NLMISC::CRGBA*		SrcDiffusePixels;

	// TileFar pixels
	const NLMISC::CRGBA*		SrcAdditivePixels;

	// Source deltaY
	sint32						SrcDeltaX;

	// Source deltaY
	sint32						SrcDeltaY;

	// Source lighting
	const NLMISC::CRGBA*		SrcLightingPixels;

	// Delta Y for lighting
	sint32						SrcLightingDeltaY;

	// TileFar pixels
	NLMISC::CRGBA*				DstPixels;

	// Destination deltaX
	sint32						DstDeltaX;

	// Destination deltaY
	sint32						DstDeltaY;

	// Size
	sint32						Size;

	// Can the compute be done in MMX
	bool						AsmMMX;
};

// For NL3D_expandLightmap external call
struct NL3D_CExpandLightmap
{
public:
	// CTileColor array.
	const NL3D::CTileColor*		ColorTile;
	// TLI Color array.
	const NLMISC::CRGBA*		TLIColor;

	// Lumel array. 4x4 lumels by tile.
	const uint8*				LumelTile;

	// Width of the array
	uint32						Width;

	// Height of the array
	uint32						Height;

	// Mul factor for the size (1, 2 or 4)
	uint32						MulFactor;

	// Static light color
	const NLMISC::CRGBA*		StaticLightColor;

	// Destination array
	NLMISC::CRGBA*				DstPixels;
};

// Extern ASM functions
extern "C" void NL3D_expandLightmap (const NL3D_CExpandLightmap* pLightmap);
extern "C" void NL3D_drawFarTileInFarTexture (const NL3D_CComputeTileFar* pTileFar);
extern "C" void NL3D_drawFarTileInFarTextureAdditive (const NL3D_CComputeTileFar* pTileFar);
extern "C" void NL3D_drawFarTileInFarTextureAlpha (const NL3D_CComputeTileFar* pTileFar);
extern "C" void NL3D_drawFarTileInFarTextureAdditiveAlpha (const NL3D_CComputeTileFar* pTileFar);

#endif // NL_TEXTURE_FAR_H

/* End of texture_far.h */
