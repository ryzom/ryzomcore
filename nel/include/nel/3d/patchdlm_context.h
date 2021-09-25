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

#ifndef NL_PATCHDLM_CONTEXT_H
#define NL_PATCHDLM_CONTEXT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/object_vector.h"
#include "nel/3d/tess_list.h"
#include "nel/misc/bsphere.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/rgba.h"
#include "nel/3d/landscape_def.h"


namespace NL3D
{


class	CPatch;
class	CTextureDLM;
class	CPointLight;
class	CPatchDLMContextList;

// ***************************************************************************
/**
 * A PointLight for Dynamic LightMap (DLM) context for a patch.
 *	It contains precomputed values.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CPatchDLMPointLight
{
public:
	// Diffuse Color of the Spot, pre-modulated with landscape PointLightDiffuseMaterial. 0..255
	float		R, G, B;
	// Is this a spot? NB: if false, cosMin/cosMax are still well computed for correctLighting (cosMax=-1, cosMin= -2).
	bool		IsSpot;
	// World Position of the spot
	CVector		Pos;
	// Direction of the spot, normalized
	CVector		Dir;
	// cosMax, where influence==1.
	float		CosMax;
	// cosMin, where influence==0. NB: cosMax>cosMin (ie angleMax<angleMin)
	float		CosMin;
	// 1.f / (cosMax-cosMin);
	float		OOCosDelta;
	// Attenuation distance, where influence==0.
	float		AttMax;
	// Attenuation distance, where influence==1. NB: attMax>attMin
	float		AttMin;
	// 1.f / (attMin-attMax);
	float		OOAttDelta;


	// The estimated sphere which englobe the light. NB: approximated for SpotLight
	NLMISC::CBSphere	BSphere;
	// The BBox which englobe the light. NB: Sphere and Box are best fit to the light, ie bbox may not
	// englobe the sphere and vice versa
	NLMISC::CAABBox		BBox;


public:
	// compile from a pointlight. NB: attenuation end is clamped to maxAttEnd (must be >0)
	void		compile(const CPointLight &pl, NLMISC::CRGBA landDiffMat, float maxAttEnd= 30.f);
};


// ***************************************************************************
/**
 * A Dynamic LightMap (DLM) context for a patch.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CPatchDLMContext : public CTessNodeList
{
public:

	struct	CVertex
	{
		CVector		Pos;
		CVector		Normal;
	};

	/// see compileLighting()
	enum	TCompileType	{ModulateTileColor=0, ModulateTextureFar, ModulateConstant, NoModulate};

public:

	/// DLM info
	/// The position and size of the DLM in the texture, in pixels.
	uint			TextPosX, TextPosY, Width, Height;
	/// Mapping to this rectangle from 0-1 basis
	float			DLMUScale, DLMVScale, DLMUBias, DLMVBias;
	/// texture coordinate bound in 8 bits. Important for Vegetable special DLM mapping
	uint8			MinU8, MaxU8;
	uint8			MinV8, MaxV8;

	/** Lighting Process: number of light contribution to this patch in last rneder, and in cur Render.
	 *	Modified by CPatch and CLandscape
	 */
	uint			OldPointLightCount;
	uint			CurPointLightCount;


public:

	/// Constructor
	CPatchDLMContext();
	/// Destructor: lightmap is released from _DLMTexture
	~CPatchDLMContext();

	/** generate: link to patch and allocate texture space, compile vertices etc...
	 *	The context is also linked to the list, and removed automaticllay at dtor
	 *	Texture space is filled with black (RAM only)
	 *	\return false if cannot allocate texture space for this patch
	 */
	bool			generate(CPatch *patch, CTextureDLM *textureDLM, CPatchDLMContextList *ctxList);

	/**	Fill texture space with Black: RAM texture is updated.
	 *	NB: full src blackness is cached.
	 */
	void			clearLighting();

	/**	Add a pointLight influence to the lighting: RAM texture is updated.
	 *	NB: full src blackness is reseted.
	 */
	void			addPointLightInfluence(const CPatchDLMPointLight &pl);

	/**	update VRAM texture with RAM texture. Uploaded in 16 bits format.
	 *	NB: full dst blackness is cached.
	 *	\param compType say if, before writing to the texture, the lightmap is modulated with _Patch TileColor,
	 *	_Patch textureFar (precomputed in the context), modulate with a constant or not modulated at all
	 *	\param modulateCte used only if compType==ModulateConstant. this is the cte to be modulate by lightmap
	 *	before copy to texture
	 */
	void			compileLighting(TCompileType compType, NLMISC::CRGBA modulateCte= NLMISC::CRGBA::White);

	CPatch			*getPatch() const {return _Patch;}

	// For Bench. Get the size in memory this class use.
	uint			getMemorySize() const;

// *************************
private:

	/// The patch which owns us.
	CPatch							*_Patch;
	/// The DLM texture (only one per landscape)
	CTextureDLM						*_DLMTexture;
	// The ctx list where this context is appened.
	CPatchDLMContextList			*_DLMContextList;

	/// Bezier Patch Array information: Width*Height.
	NLMISC::CObjectVector<CVertex>	_Vertices;

	/// The computed lightmap: Width*Height.
	NLMISC::CObjectVector<CRGBA>	_LightMap;

	/// A clip cluster, for quadTree of clusters.
	struct	CCluster
	{
		// The bounding sphere of the cluster
		NLMISC::CBSphere			BSphere;
		// If cluster not clipped, how many cluster to skip. NB: if NSkips==0, then it is a leaf cluster.
		uint						NSkips;
		// For leaf cluster: logical position of the cluster
		uint16						X, Y;
	};

	/// Bounding Sphere QuadTree (with NSkips paradigm)
	NLMISC::CObjectVector<CCluster>	_Clusters;

	// Tells if all _LightMap[] is all black.
	bool							_IsSrcTextureFullBlack;
	// Tells if all dst texture in _DLMTexture is black.
	bool							_IsDstTextureFullBlack;


// If resolution of the lightmap is every 2x2 tiles, must bkup tileColors.
#ifndef NL_DLM_TILE_RES
	// The tileColors at resolution of tessBlock
	NLMISC::CObjectVector<uint16, false>	_LowResTileColors;
#endif


	/** The TextureFar (at tile level or less), which is always computed at generate().
	 *	NB: Real compute is TextureFar*UserColor, so it need only to be modulated by lightmap each frame.
	 */
	NLMISC::CObjectVector<CRGBA>	_TextureFar;


private:

	// called at generate.
	void			computeTextureFar();

	// Tile at 2x2 resolution method
	static const CRGBA	*computeTileFarSrcDeltas(sint nRot, bool is256x256, uint8 uvOff, const CRGBA *srcPixel, sint &srcDeltaX, sint &srcDeltaY);
	static void		copyTileToTexture(const CRGBA *srcPixel, sint srcDeltaX, sint srcDeltaY, CRGBA *dstPixel, uint dstStride);
	static void		blendTileToTexture(const CRGBA *srcPixel, sint srcDeltaX, sint srcDeltaY, CRGBA *dstPixel, uint dstStride);

};


// ***************************************************************************
/// A List of CPatchDLMContext.
class	CPatchDLMContextList : public CTessList<CPatchDLMContext>
{
};



} // NL3D


#endif // NL_PATCHDLM_CONTEXT_H

/* End of patchdlm_context.h */
