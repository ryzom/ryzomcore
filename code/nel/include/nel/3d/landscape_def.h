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

#ifndef NL_LANDSCAPE_DEF_H
#define NL_LANDSCAPE_DEF_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bsphere.h"
#include "nel/3d/landscapevb_info.h"
#include "nel/3d/index_buffer.h"


namespace NL3D
{

using	NLMISC::CVector;


class	CLandscapeVBAllocator;
class	IDriver;


// ***************************************************************************
// 4th pass is always the Lightmapped one (Lightmap*clouds).
#define	NL3D_MAX_TILE_PASS 5
// There is no Face for lightmap, since lightmap pass share the RGB0 face.
#define	NL3D_MAX_TILE_FACE	NL3D_MAX_TILE_PASS-1

#define	NL3D_TILE_PASS_RGB0		0
#define	NL3D_TILE_PASS_RGB1		1
#define	NL3D_TILE_PASS_RGB2		2
#define	NL3D_TILE_PASS_ADD		3
#define	NL3D_TILE_PASS_LIGHTMAP	4
// NB: RENDER ORDER: CLOUD*LIGHTMAP is done BEFORE ADDITIVE.


// ***************************************************************************
// see CTessFace::updateRefineMerge()
#define	NL3D_REFINE_MERGE_THRESHOLD		2.0f


// ***************************************************************************
/// For Landscape Vegetable: AGP load (in number of vertices allocated )
#define	NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_UNLIT		50000
#define	NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_LIGHTED		5000
#define	NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_MAX	\
	(max(NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_UNLIT, NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_LIGHTED))

// For Landscape Vegetable
// Suppose a constant size for a tile of 2m*2m.
#define	NL3D_PATCH_TILE_AREA	4.f
// Suppose a constant size for a tile of 2m*2m. approx radius is 1m.
#define	NL3D_PATCH_TILE_RADIUS	1.f


// ***************************************************************************
// For Landscape dynamic lighting, size of the dynamic lightmap.
#define	NL3D_LANDSCAPE_DLM_WIDTH	512
#define	NL3D_LANDSCAPE_DLM_HEIGHT	256

// if NL_DLM_TILE_RES is defined, DLM texture precision are at Tile precision (ie 2m)
// If not defined then suppose TessBlock coarser precision (4m)
#define NL_DLM_TILE_RES



// ***************************************************************************
class	CLandscapeGlobals
{
public:
	// LANDSCAPE RENDERING CONTEXT.  Landscape must setup it at the beginning at refine()/render().
	// The current date of LandScape for refine only.
	static	sint	CurrentDate;
	// The current date of LandScape for render only.
	static	sint	CurrentRenderDate;
	// The center view for refinement.
	static	CVector RefineCenter;
	// What is the threshold for tessellation.
	static	float	RefineThreshold;
	// Guess.
	static	float	OORefineThreshold;

	// The center of the landscape (near CameraCenter), for better ZBuffer precision.
	static	CVector PZBModelPosition;


	// Tile Global Info.
	// What are the limit distances for Tile tesselation transition.
	static	float	TileDistNear, TileDistFar;
	// System, computed from prec.
	static	float	TileDistNearSqr, TileDistFarSqr;
	// System, computed from prec.
	static	float	OOTileDistDeltaSqr;
	// The tiles are not subdivided above this limit (but because of enforced splits). Default: 4 => 50cm.
	static	sint	TileMaxSubdivision;
	// The sphere for TileFar test.
	static	NLMISC::CBSphere	TileFarSphere;
	// The sphere for TileNear test.
	static	NLMISC::CBSphere	TileNearSphere;
	// The size of a 128x128 tile, in pixel. Useful for HalfPixel Scale/Bias.
	static	float		TilePixelSize;
	// HalfPixel Scale/Bias.
	static	float		TilePixelBias128;
	static	float		TilePixelScale128;
	static	float		TilePixelBias256;
	static	float		TilePixelScale256;


	// Render Global info. Used by Patch.
	// The distance transition for Far0 and Far1 (200m / 400m).
	static	float	Far0Dist, Far1Dist;
	// Distance for Alpha blend transition
	static	float	FarTransition;


	// This Tells if VertexProgram is activated for the current landscape.
	static	bool					VertexProgramEnabled;
	// The current VertexBuffer for Far0
	static	CFarVertexBufferInfo	CurrentFar0VBInfo;
	// The current VertexBuffer for Far1.
	static	CFarVertexBufferInfo	CurrentFar1VBInfo;
	// The current VertexBuffer for Tile.
	static	CNearVertexBufferInfo	CurrentTileVBInfo;

	// The current VertexBuffer Allocator for Far0
	static	CLandscapeVBAllocator	*CurrentFar0VBAllocator;
	// The current VertexBuffer Allocator for Far1.
	static	CLandscapeVBAllocator	*CurrentFar1VBAllocator;
	// The current VertexBuffer Allocator for Tile.
	static	CLandscapeVBAllocator	*CurrentTileVBAllocator;


	// PATCH GLOBAL INTERFACE.  patch must setup them at the beginning at refine()/render().
	// NO!!! REMIND: can't have any patch global, since a propagated split()/updateErrorMetric()
	// can arise. must use Patch pointer.

	// Render:
	// Globals for speed render.
	static IDriver				*PatchCurrentDriver;
	// The triangles array for the current pass rendered.
	static CIndexBuffer			PassTriArray;
	static CIndexBufferReadWrite	PassTriArrayIBA;

};


// ***************************************************************************
// Out of CLandscapeGlobals, because maybe used in __asm{}
extern	uint					NL3D_LandscapeGlobals_PassNTri;
extern	void					*NL3D_LandscapeGlobals_PassTriCurPtr;
extern  CIndexBuffer::TFormat	NL3D_LandscapeGlobals_PassTriFormat;



} // NL3D


#endif // NL_LANDSCAPE_DEF_H

/* End of landscape_def.h */
