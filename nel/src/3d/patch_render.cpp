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
#include "nel/3d/landscape_profile.h"
#include "nel/3d/patchdlm_context.h"
#include "nel/3d/debug_vb.h"

#include "nel/misc/vector.h"
#include "nel/misc/common.h"

using	namespace	std;
using	namespace	NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// Patch Texture computation.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CPatch::computeNewFar(const NLMISC::CBSphere &patchSphere, sint &newFar0, sint &newFar1)
{
	// Classify the patch.
	//========================
	float	r= (CLandscapeGlobals::RefineCenter-patchSphere.Center).norm() - patchSphere.Radius;
	float	rr=0.0;
	if(r<CLandscapeGlobals::TileDistNear)
		rr= r-CLandscapeGlobals::TileDistNear, newFar0= 0;
	else if(r<CLandscapeGlobals::Far0Dist)
		rr= r-CLandscapeGlobals::Far0Dist, newFar0= 1;
	else if(r<CLandscapeGlobals::Far1Dist)
		rr= r-CLandscapeGlobals::Far1Dist, newFar0= 2;
	else
		newFar0= 3;
	// Transition with the next level.
	newFar1=0;
	if(newFar0<3 && rr>-(CLandscapeGlobals::FarTransition+2*patchSphere.Radius))
	{
		newFar1= newFar0+1;
	}


	// Update Texture Info.
	//========================
	if(newFar0!=Far0 || newFar1!=Far1)
	{
		// Backup old pass0
		CPatchRdrPass	*oldPass0=_PatchRdrPassFar0;
		CPatchRdrPass	*oldPass1=_PatchRdrPassFar1;

		// Checks
		if (oldPass0==NULL)
			nlassert (Far0<=0);
		if (oldPass1==NULL)
			nlassert (Far1<=0);

		float oldFar0UScale=Far0UScale;
		float oldFar0VScale=Far0VScale;
		float oldFar0UBias=Far0UBias;
		float oldFar0VBias=Far0VBias;
		uint8 oldFlags=Flags;


		// Don't delete the pass0 if the new newFar1 will use it
		if ((newFar1==Far0)&&(Far0>0))
			_PatchRdrPassFar0=NULL;

		// Don't delete the pass1 if the new newFar0 will use it
		if ((newFar0==Far1)&&(Far1>0))
			_PatchRdrPassFar1=NULL;

		// Pass0 have changed ?
		if (newFar0!=Far0)
		{
			// Compute / get the texture Far.
			if(newFar0>0)
			{
				// Free the old pass, don't used any more
				if (_PatchRdrPassFar0)
					Zone->Landscape->freeFarRenderPass (this, _PatchRdrPassFar0, Far0);

				// Can we use the old pass1 ?
				if (newFar0==Far1)
				{
					// Yes, recycle it!
					_PatchRdrPassFar0=oldPass1;

					// Copy uv coordinates
					Far0UScale=Far1UScale;
					Far0VScale=Far1VScale;
					Far0UBias=Far1UBias;
					Far0VBias=Far1VBias;

					// Copy rotation flag
					Flags&=~NL_PATCH_FAR0_ROTATED;			// erase it
					if (Flags&NL_PATCH_FAR1_ROTATED)
						Flags|=NL_PATCH_FAR0_ROTATED;			// copy it
				}
				else	// get a new render pass
				{
					// Rotation boolean
					bool bRot;
					_PatchRdrPassFar0=Zone->Landscape->getFarRenderPass(this, newFar0, Far0UScale, Far0VScale, Far0UBias, Far0VBias, bRot);

					// Flags is set if the far texture is rotated of 90deg to the left
					if (bRot)
						Flags|=NL_PATCH_FAR0_ROTATED;
					else
						Flags&=~NL_PATCH_FAR0_ROTATED;
				}
			}
			else	// no more far pass0
			{
				if (_PatchRdrPassFar0)
				{
					Zone->Landscape->freeFarRenderPass (this, _PatchRdrPassFar0, Far0);
					_PatchRdrPassFar0=NULL;
				}
			}
		}

		// Pass1 have changed ?
		if (newFar1!=Far1)
		{
			// Now let's go with pass1
			if(newFar1>0)
			{
				// Delete the pass1 if not used any more
				if (_PatchRdrPassFar1)
					Zone->Landscape->freeFarRenderPass (this, _PatchRdrPassFar1, Far1);

				// Can we use the old pass1 ?
				if (newFar1==Far0)
				{
					// Yes, recycle it!
					_PatchRdrPassFar1= oldPass0;
					nlassert (_PatchRdrPassFar1);

					// Copy uv coordinates
					Far1UScale=oldFar0UScale;
					Far1VScale=oldFar0VScale;
					Far1UBias=oldFar0UBias;
					Far1VBias=oldFar0VBias;

					// Copy rotation flag
					Flags&=~NL_PATCH_FAR1_ROTATED;			// erase it
					if (oldFlags&NL_PATCH_FAR0_ROTATED)
						Flags|=NL_PATCH_FAR1_ROTATED;			// copy it
				}
				else	// get a new render pass
				{
					// Rotation boolean
					bool bRot;
					_PatchRdrPassFar1=Zone->Landscape->getFarRenderPass(this, newFar1, Far1UScale, Far1VScale, Far1UBias, Far1VBias, bRot);
					nlassert (_PatchRdrPassFar1);

					// Flags is set if the far texture is rotated of 90deg to the left
					if (bRot)
						Flags|=NL_PATCH_FAR1_ROTATED;
					else
						Flags&=~NL_PATCH_FAR1_ROTATED;
				}

				// Compute info for transition.
				float	farDist = 0.f;
				switch(newFar1)
				{
					case 1: farDist= CLandscapeGlobals::TileDistNear; break;
					case 2: farDist= CLandscapeGlobals::Far0Dist; break;
					case 3: farDist= CLandscapeGlobals::Far1Dist; break;
					default: nlstop;
				};
				TransitionSqrMin= sqr(farDist-CLandscapeGlobals::FarTransition);
				OOTransitionSqrDelta= 1.0f/(sqr(farDist)-TransitionSqrMin);
			}
			else	// no more far pass1
			{
				if (_PatchRdrPassFar1)
				{
					Zone->Landscape->freeFarRenderPass (this, _PatchRdrPassFar1, Far1);
					_PatchRdrPassFar1=NULL;
				}
			}
		}

	}

	// Don't copy Far0 and Far1.
}


// ***************************************************************************
void			CPatch::updateTextureFarOnly(const NLMISC::CBSphere &patchSphere)
{
	sint	newFar0,newFar1;
	// compute new Far0 and new Far1.
	computeNewFar(patchSphere, newFar0, newFar1);

	// Far change.
	//------------------
	// Set new far values
	Far0= newFar0;
	Far1= newFar1;
}



// ***************************************************************************
// ***************************************************************************
// Patch / Face rendering.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CPatch::preRender(const NLMISC::CBSphere &patchSphere)
{
	// 0. Classify the patch, and update TextureInfo.
	//=======================
	sint	newFar0,newFar1;
	// compute new Far0 and new Far1.
	computeNewFar(patchSphere, newFar0, newFar1);


	// 2. Update vertices in VB
	//========================
	sint	oldFar0= Far0, oldFar1= Far1;

	// Test if go in or go out tileMode.
	bool	changeTileMode;
	// this is true if a change is made, and if old or current is in TileMode.
	changeTileMode= (newFar0!=oldFar0) && (newFar0==0 || oldFar0==0);
	// Or this is true if Far0 / Far1 was invalid before.
	changeTileMode= changeTileMode || oldFar0==-1 || oldFar1==-1;

	// Pre VB change.
	//------------------
	// In this case, major change: delete all VB, then recreate after.
	if(changeTileMode)
	{
		// delete the old VB (NB: isRenderClipped()==false here).
		// NB: Far0 and Far1 are still unmodified, so deleteVB() will do the good job.
		deleteVBAndFaceVector();
	}
	else
	{
		// minor change: Far0 UV, or Far1.
		// Far0 UV: do nothing here.
		// If change in Far1, must delete Far1
		if(newFar1!=oldFar1)
		{
			// NB: Far1 is still unmodified, so deleteVB() will do the good job.
			deleteVBAndFaceVectorFar1Only();
		}
	}

	// Far change.
	//------------------
	// Set new far values
	Far0= newFar0;
	Far1= newFar1;


	// Post VB change.
	//------------------
	if(changeTileMode)
	{
		// major change: recreate all the VB.
		allocateVBAndFaceVector();
		// Then try to refill if possible.
		fillVB();
	}
	else
	{
		// minor change: Far0 UV, or Far1.
		// If change in Far0
		if(newFar0!=oldFar0)
		{
			// Then must recompute VB with good UVs.
			// An optimisation is to check if UVs had really change (see UScale ...)
			// but the case they don't change is very rare: 1/16.
			fillVBFar0Only();
		}

		// If change in Far1, must allcoate and recompute UVs.
		if(newFar1!=oldFar1)
		{
			allocateVBAndFaceVectorFar1Only();
			// try to fill if no reallocation problem
			fillVBFar1Only();
		}
	}


	// 3. Append patch to renderList.
	//=====================
	// This patch is visible. So if good far, append.
	if(Far0>0)
	{
		_PatchRdrPassFar0->appendRdrPatchFar0(this);
	}
	// Same for Far1.
	if(Far1>0)
	{
		_PatchRdrPassFar1->appendRdrPatchFar1(this);
	}


	// 4. Clip tess blocks.
	//=====================
	// If we are in Tile/FarTransition
	bool	doClipFar= Far0==0 && Far1==1;
	// Parse all TessBlocks.
	uint			nTessBlock= TessBlocks.size();
	CTessBlock		*pTessBlock= nTessBlock>0? &TessBlocks[0]: NULL ;
	for(; nTessBlock>0; pTessBlock++, nTessBlock--)
	{
		CTessBlock		&tblock= *pTessBlock;

		// bkup the old result for clipping
		bool	oldVisibleFar0= tblock.visibleFar0();
		bool	oldVisibleTile= tblock.visibleTile();
		bool	oldVisibleFar1= tblock.visibleFar1();

		// If this TessBlock is empty, do not need to clip
		if(tblock.FaceTileMaterialRefCount==0)
		{
			// Simply force the clip.
			tblock.forceClip();
		}
		else
		{
			// clip the tessBlock.
			tblock.resetClip();
			tblock.clip();
			// If we are in Tile/FarTransition
			if(doClipFar)
				tblock.clipFar(CLandscapeGlobals::RefineCenter, CLandscapeGlobals::TileDistNear, CLandscapeGlobals::FarTransition);

			// If TileMode, and if tile visible
			if(Far0==0 && tblock.visibleTile() )
			{
				// Append all tiles (if any) to the renderPass list!
				for(uint j=0;j<NL3D_TESSBLOCK_TILESIZE; j++)
				{
					// If tile exist
					if(tblock.RdrTileRoot[j])
						// add it to the renderList
						tblock.RdrTileRoot[j]->appendTileToEachRenderPass(NumRenderableFaces);
				}
			}
		}

		// If change of clip in tessBlock, must update the tessBlock
		if( Far0> 0 && oldVisibleFar0 != tblock.visibleFar0() )
		{
			if( tblock.visibleFar0() )
			{
				// allocate
				updateFar0VBAlloc(tblock.FarVertexList, true);
				// fill only if possible.
				if(!CLandscapeGlobals::CurrentFar0VBAllocator->reallocationOccurs())
					fillFar0VertexListVB(tblock.FarVertexList);
				// rebuild triangles index list.
				tblock.refillFaceVectorFar0();
			}
			else
			{
				// delete
				updateFar0VBAlloc(tblock.FarVertexList, false);
			}
		}
		if( Far0==0 && oldVisibleTile != tblock.visibleTile() )
		{
			if( tblock.visibleTile() )
			{
				// allocate
				updateTileVBAlloc(tblock.NearVertexList, true);
				// fill only if possible.
				if(!CLandscapeGlobals::CurrentTileVBAllocator->reallocationOccurs())
					fillTileVertexListVB(tblock.NearVertexList);
				// rebuild triangles index list.
				tblock.refillFaceVectorTile();
			}
			else
			{
				// delete
				updateTileVBAlloc(tblock.NearVertexList, false);
			}
		}
		if( Far1> 0 && oldVisibleFar1 != tblock.visibleFar1() )
		{
			if( tblock.visibleFar1() )
			{
				// allocate
				updateFar1VBAlloc(tblock.FarVertexList, true);
				// fill only if possible.
				if(!CLandscapeGlobals::CurrentFar1VBAllocator->reallocationOccurs())
					fillFar1VertexListVB(tblock.FarVertexList);
				// rebuild triangles index list.
				tblock.refillFaceVectorFar1();
			}
			else
			{
				// delete
				updateFar1VBAlloc(tblock.FarVertexList, false);
			}
		}
	}

}


// Special profiling.
#ifdef	NL3D_PROFILE_LAND
#define	NL3D_PROFILE_LAND_ADD_FACE_VECTOR(_x, _fv)		\
	if(_fv)											\
	{												\
		NL3D_PROFILE_LAND_ADD(_x, *_fv);		\
	}
#else
#define	NL3D_PROFILE_LAND_ADD_FACE_VECTOR(_x, _fv)
#endif


// ***************************************************************************
static	inline	void	renderFaceVector(TLandscapeIndexType *fv)
{
	/*
		Remind that structure of fv is:
			uint32 NumTri
			uint32 index0Tri0
			uint32 index1Tri0
			uint32 index2Tri0
			uint32 index0Tri1
			uint32 index1Tri1
			.....
	*/

	// If not NULL (ie not empty).
	if(fv)
	{
		// TestYoyo: Bench.
		/*
		extern	uint	TEMP_NRFV;
		extern	uint	TEMP_NRFVTri;
		extern	float	TEMP_NRFVMeanTri;
		extern	float	TEMP_NRFVDeltaTri;
		TEMP_NRFV++;
		TEMP_NRFVTri+= *fv;
		TEMP_NRFVDeltaTri+= sqr(*fv-TEMP_NRFVMeanTri);
		*/

		// here we have NumTri>0, because fv!=NULL.

		// making lot of render() is slower than copy a block, and render it.
		//CLandscapeGlobals::PatchCurrentDriver->renderSimpleTriangles(fv+1, *fv);
		CHECK_IBA_RANGE(CLandscapeGlobals::PassTriArrayIBA, NL3D_LandscapeGlobals_PassTriCurPtr,  *fv*3 * sizeof(uint32));
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
		#ifndef NL_LANDSCAPE_INDEX16
			__asm
			{
				mov		ebx, fv
				mov		edi, NL3D_LandscapeGlobals_PassTriCurPtr

				mov		edx, NL3D_LandscapeGlobals_PassNTri
				xor		eax, eax		// Avoid AGI stall.

				mov		ecx, [ebx]
				lea		esi, [ebx+4]

				mov		eax, ecx				// eax= bkup NumTris
				lea		ecx, [ecx + ecx*2]		// ecx= nTriIndex= NumTris*3

				// copy tri indices
				rep		movsd

				add		edx, eax				// edx= NL3D_LandscapeGlobals_PassNTri + NumTri;

				// NL3D_LandscapeGlobals_PassTriCurPtr= edi= new ptr after copy
				mov		NL3D_LandscapeGlobals_PassTriCurPtr, edi
				mov		NL3D_LandscapeGlobals_PassNTri, edx
			}
		#else
			__asm
			{
				mov		ebx, fv
				mov		edi, NL3D_LandscapeGlobals_PassTriCurPtr

				mov		edx, NL3D_LandscapeGlobals_PassNTri
				xor		eax, eax		// Avoid AGI stall.

				movzx	ecx,  word ptr [ebx]
				lea		esi, [ebx+2]

				mov		eax, ecx				// eax= bkup NumTris
				lea		ecx, [ecx + ecx*2]		// ecx= nTriIndex= NumTris*3

				test	ecx, 1
				jne		odd_number
				shr     ecx, 1
				// for alignment, first copy a single word
				movsw
				dec	ecx
				rep movsd
				movsw
				jmp		even_number_done
	odd_number:
				shr ecx, 1
				// for alignment, first copy a single word
				movsw
				rep movsd
	even_number_done:

				add		edx, eax				// edx= NL3D_LandscapeGlobals_PassNTri + NumTri;

				// NL3D_LandscapeGlobals_PassTriCurPtr= edi= new ptr after copy
				mov		NL3D_LandscapeGlobals_PassTriCurPtr, edi
				mov		NL3D_LandscapeGlobals_PassNTri, edx
			}
	#endif
#else
		uint	nTriIndex= *fv*3;
		// Fill and increment the array.
		#ifndef NL_LANDSCAPE_INDEX16
			memcpy( NL3D_LandscapeGlobals_PassTriCurPtr, fv+1, nTriIndex * sizeof(uint32) );
			NL3D_LandscapeGlobals_PassTriCurPtr= (uint32*)NL3D_LandscapeGlobals_PassTriCurPtr + nTriIndex;
		#else
			memcpy( NL3D_LandscapeGlobals_PassTriCurPtr, fv+1, nTriIndex * sizeof(uint16) );
			NL3D_LandscapeGlobals_PassTriCurPtr= (uint16*)NL3D_LandscapeGlobals_PassTriCurPtr + nTriIndex;
		#endif
		NL3D_LandscapeGlobals_PassNTri+= *fv;
#endif
	}
}


// ***************************************************************************
void			CPatch::renderFar0()
{
	NL3D_PROFILE_LAND_ADD(ProfNPatchRdrFar0, 1);

#ifdef NL_DEBUG
	// Must be visible, and must be called only if the RdrPass is enabled.
	nlassert(!isRenderClipped() && _PatchRdrPassFar0);
#endif

	// Render tris of MasterBlock.
	renderFaceVector(MasterBlock.Far0FaceVector);
	// profile
	NL3D_PROFILE_LAND_ADD_FACE_VECTOR(ProfNRdrFar0, MasterBlock.Far0FaceVector);

	// Render tris of TessBlocks.
	uint			nTessBlock= TessBlocks.size();
	CTessBlock		*pTessBlock= nTessBlock>0? &TessBlocks[0]: NULL ;
	for(; nTessBlock>0; pTessBlock++, nTessBlock--)
	{
		CTessBlock		&tblock= *pTessBlock;
		// if block visible, render
		if( tblock.visibleFar0() )
		{
			renderFaceVector(tblock.Far0FaceVector);
			// profile
			NL3D_PROFILE_LAND_ADD_FACE_VECTOR(ProfNRdrFar0, tblock.Far0FaceVector);
		}
	}
}


// ***************************************************************************
void			CPatch::renderFar1()
{
	NL3D_PROFILE_LAND_ADD(ProfNPatchRdrFar1, 1);

#ifdef NL_DEBUG
	// Must be visible, and must be called only if the RdrPass is enabled.
	nlassert(!isRenderClipped() && _PatchRdrPassFar1);
#endif

	// Render tris of MasterBlock.
	renderFaceVector(MasterBlock.Far1FaceVector);
	// profile.
	NL3D_PROFILE_LAND_ADD_FACE_VECTOR(ProfNRdrFar1, MasterBlock.Far1FaceVector);

	// Render tris of TessBlocks.
	uint			nTessBlock= TessBlocks.size();
	CTessBlock		*pTessBlock= nTessBlock>0? &TessBlocks[0]: NULL ;
	for(; nTessBlock>0; pTessBlock++, nTessBlock--)
	{
		CTessBlock		&tblock= *pTessBlock;
		// if block visible, render
		if( tblock.visibleFar1() )
		{
			renderFaceVector(tblock.Far1FaceVector);
			// profile.
			NL3D_PROFILE_LAND_ADD_FACE_VECTOR(ProfNRdrFar1, tblock.Far1FaceVector);
		}
	}
}


// ***************************************************************************
void			CTileMaterial::renderTile(uint pass)
{
	// because precisely inserted in preRender(), and correctly tested, this tile is to be rendered,
	// If the pass is enabled.
	if(Pass[pass].PatchRdrPass)
	{
		// render tris of the good faceList.
		renderFaceVector(TileFaceVectors[pass]);

		// profile.
		NL3D_PROFILE_LAND_ADD_FACE_VECTOR(ProfNRdrTile[pass], TileFaceVectors[pass]);
	}
}

// ***************************************************************************
void			CTileMaterial::renderTilePassRGB0()
{
	// because precisely inserted in preRender(), and correctly tested, this tile is to be rendered,
	// this pass must be enabled!
	nlassert(Pass[NL3D_TILE_PASS_RGB0].PatchRdrPass);
	// render tris of the good faceList.
	renderFaceVector(TileFaceVectors[NL3D_TILE_PASS_RGB0]);

	// profile.
	NL3D_PROFILE_LAND_ADD_FACE_VECTOR(ProfNRdrTile[NL3D_TILE_PASS_RGB0], TileFaceVectors[NL3D_TILE_PASS_RGB0]);
}

// ***************************************************************************
void			CTileMaterial::renderTilePassLightmap()
{
	// because precisely inserted in preRender(), and correctly tested, this tile is to be rendered,
	// this pass must be enabled!
	nlassert(Pass[NL3D_TILE_PASS_LIGHTMAP].PatchRdrPass);
	// render tris of the good faceList, ie the one of PassRGB0, because vertices are reused.
	renderFaceVector(TileFaceVectors[NL3D_TILE_PASS_RGB0]);

	// profile.
	NL3D_PROFILE_LAND_ADD_FACE_VECTOR(ProfNRdrTile[NL3D_TILE_PASS_LIGHTMAP], TileFaceVectors[NL3D_TILE_PASS_RGB0]);
}


// ***************************************************************************
// ***************************************************************************
// FaceVector Allocation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CPatch::createFaceVectorFar1()
{
	if(Far1>0)
	{
		// Create the face for all TessBlocks.
		MasterBlock.createFaceVectorFar1(getLandscape()->_FaceVectorManager);
		for(uint i=0; i<TessBlocks.size(); i++)
			TessBlocks[i].createFaceVectorFar1(getLandscape()->_FaceVectorManager);
	}
}
// ***************************************************************************
void		CPatch::deleteFaceVectorFar1()
{
	if(Far1>0)
	{
		// delete the face for all TessBlocks.
		MasterBlock.deleteFaceVectorFar1(getLandscape()->_FaceVectorManager);
		for(uint i=0; i<TessBlocks.size(); i++)
			TessBlocks[i].deleteFaceVectorFar1(getLandscape()->_FaceVectorManager);
	}
}
// ***************************************************************************
void		CPatch::createFaceVectorFar0OrTile()
{
	// If Far Mode.
	if(Far0>0)
	{
		// Create the face for all TessBlocks.
		MasterBlock.createFaceVectorFar0(getLandscape()->_FaceVectorManager);
		for(uint i=0; i<TessBlocks.size(); i++)
			TessBlocks[i].createFaceVectorFar0(getLandscape()->_FaceVectorManager);
	}
	// Or If Tile Mode.
	else if(Far0==0)
	{
		// Create the face for all TessBlocks.
		// No tiles in MasterBlock!
		for(uint i=0; i<TessBlocks.size(); i++)
			TessBlocks[i].createFaceVectorTile(getLandscape()->_FaceVectorManager);
	}
}
// ***************************************************************************
void		CPatch::deleteFaceVectorFar0OrTile()
{
	// If Far Mode.
	if(Far0>0)
	{
		// delete the face for all TessBlocks.
		MasterBlock.deleteFaceVectorFar0(getLandscape()->_FaceVectorManager);
		for(uint i=0; i<TessBlocks.size(); i++)
			TessBlocks[i].deleteFaceVectorFar0(getLandscape()->_FaceVectorManager);
	}
	// Or If Tile Mode.
	else if(Far0==0)
	{
		// delete the face for all TessBlocks.
		// No tiles in MasterBlock!
		for(uint i=0; i<TessBlocks.size(); i++)
			TessBlocks[i].deleteFaceVectorTile(getLandscape()->_FaceVectorManager);
	}
}


// ***************************************************************************
void		CPatch::recreateTessBlockFaceVector(CTessBlock &block)
{
	// Do it Only if patch is visible.
	if(!isRenderClipped())
	{
		// Far0.
		// If Far Mode.
		if(Far0>0)
		{
			// Create the face for this TessBlock only.
			block.createFaceVectorFar0(getLandscape()->_FaceVectorManager);
		}
		// Or If Tile Mode.
		else if(Far0==0)
		{
			// No tiles in MasterBlock! So no need to call createFaceVectorTile(), if this block is the MasterBlock.
			if(&block != &MasterBlock)
				block.createFaceVectorTile(getLandscape()->_FaceVectorManager);
		}

		// Far1.
		if(Far1>0)
		{
			// Create the face for this TessBlock only.
			block.createFaceVectorFar1(getLandscape()->_FaceVectorManager);
		}
	}

}


// ***************************************************************************
// ***************************************************************************
// VB Allocation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CPatch::updateFar0VBAlloc(CTessList<CTessFarVertex>  &vertList, bool alloc)
{
	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		if(alloc)
			pVert->Index0= CLandscapeGlobals::CurrentFar0VBAllocator->allocateVertex();
		else
			CLandscapeGlobals::CurrentFar0VBAllocator->deleteVertex(pVert->Index0);
	}
}


// ***************************************************************************
void		CPatch::updateFar1VBAlloc(CTessList<CTessFarVertex>  &vertList, bool alloc)
{
	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		if(alloc)
			pVert->Index1= CLandscapeGlobals::CurrentFar1VBAllocator->allocateVertex();
		else
			CLandscapeGlobals::CurrentFar1VBAllocator->deleteVertex(pVert->Index1);
	}
}


// ***************************************************************************
void		CPatch::updateTileVBAlloc(CTessList<CTessNearVertex>  &vertList, bool alloc)
{
	// Traverse the vertList.
	CTessNearVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessNearVertex*)pVert->Next)
	{
		if(alloc)
			pVert->Index= CLandscapeGlobals::CurrentTileVBAllocator->allocateVertex();
		else
			CLandscapeGlobals::CurrentTileVBAllocator->deleteVertex(pVert->Index);
	}
}


// ***************************************************************************
void			CPatch::updateVBAlloc(bool alloc)
{
	// update Far0.
	//=======
	if(Far0>0)
	{
		// alloc Far0 VB.
		updateFar0VBAlloc(MasterBlock.FarVertexList, alloc);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// need update VB only if tessBlock is visible.
			if( tblock.visibleFar0() )
				updateFar0VBAlloc(tblock.FarVertexList, alloc);
		}
	}
	else if (Far0==0)
	{
		// alloc Tile VB.
		// No Tiles in MasterBlock!!
		// Traverse the TessBlocks to add vertices.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// Add the vertices.
			// need update VB only if tessBlock is visible.
			if( tblock.visibleTile() )
				updateTileVBAlloc(tblock.NearVertexList, alloc);
		}
	}

	// update Far1.
	//=======
	if(Far1>0)
	{
		// alloc VB.
		updateFar1VBAlloc(MasterBlock.FarVertexList, alloc);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// need update VB only if tessBlock is visible.
			if( tblock.visibleFar1() )
				updateFar1VBAlloc(tblock.FarVertexList, alloc);
		}
	}
}

// ***************************************************************************
void			CPatch::deleteVBAndFaceVector()
{
	updateVBAlloc(false);
	deleteFaceVectorFar1();
	deleteFaceVectorFar0OrTile();
}

// ***************************************************************************
void			CPatch::allocateVBAndFaceVector()
{
	updateVBAlloc(true);
	createFaceVectorFar1();
	createFaceVectorFar0OrTile();
}


// ***************************************************************************
void		CPatch::deleteVBAndFaceVectorFar1Only()
{
	if(Far1>0)
	{
		// alloc VB.
		updateFar1VBAlloc(MasterBlock.FarVertexList, false);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// need update VB only if tessBlock is visible.
			if( tblock.visibleFar1() )
				updateFar1VBAlloc(tblock.FarVertexList, false);
		}
	}

	deleteFaceVectorFar1();
}

// ***************************************************************************
void		CPatch::allocateVBAndFaceVectorFar1Only()
{
	if(Far1>0)
	{
		// alloc VB.
		updateFar1VBAlloc(MasterBlock.FarVertexList, true);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// need update VB only if tessBlock is visible.
			if( tblock.visibleFar1() )
				updateFar1VBAlloc(tblock.FarVertexList, true);
		}
	}

	createFaceVectorFar1();
}


// ***************************************************************************
void		CPatch::debugAllocationMarkIndicesFarList(CTessList<CTessFarVertex>  &vertList, uint marker)
{
	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		pVert->Index0= marker;
		pVert->Index1= marker;
	}
}

void		CPatch::debugAllocationMarkIndicesNearList(CTessList<CTessNearVertex>  &vertList, uint marker)
{
	// Traverse the vertList.
	CTessNearVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessNearVertex*)pVert->Next)
	{
		pVert->Index= marker;
	}
}

void		CPatch::debugAllocationMarkIndices(uint marker)
{
	sint i;

	// Do it For Far.
	debugAllocationMarkIndicesFarList(MasterBlock.FarVertexList, marker);
	for(i=0; i<(sint)TessBlocks.size(); i++)
	{
		CTessBlock	&tblock= TessBlocks[i];
		debugAllocationMarkIndicesFarList(tblock.FarVertexList, marker);
	}
	// Do it For Near.
	// No Tiles in MasterBlock!!
	for(i=0; i<(sint)TessBlocks.size(); i++)
	{
		CTessBlock	&tblock= TessBlocks[i];
		debugAllocationMarkIndicesNearList(tblock.NearVertexList, marker);
	}

}



// ***************************************************************************
// ***************************************************************************
// VB Filling.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// NB: need to be inlined only for fillFar0VB() in this file.
inline void		CPatch::fillFar0VertexVB(CTessFarVertex *pVert)
{
	// The Buffers must have been locked
	nlassert(CLandscapeGlobals::CurrentFar0VBAllocator);
	nlassert(CLandscapeGlobals::CurrentFar0VBAllocator->bufferLocked());
	// VBInfo must be OK.
	nlassert(!CLandscapeGlobals::CurrentFar0VBAllocator->reallocationOccurs());

	static	uint8	*CurVBPtr;
	// Compute/build the new vertex.
	CurVBPtr= (uint8*)CLandscapeGlobals::CurrentFar0VBInfo.VertexCoordPointer;
	CurVBPtr+= pVert->Index0 * CLandscapeGlobals::CurrentFar0VBInfo.VertexSize;

	// NB: the filling order of data is important, for AGP write combiners.

	// compute Uvs.
	static CUV	uv;
	CParamCoord	pc= pVert->PCoord;
	if (Flags&NL_PATCH_FAR0_ROTATED)
	{
		uv.U= pc.getT()* Far0UScale + Far0UBias;
		uv.V= (1.f-pc.getS())* Far0VScale + Far0VBias;
	}
	else
	{
		uv.U= pc.getS()* Far0UScale + Far0UBias;
		uv.V= pc.getT()* Far0VScale + Far0VBias;
	}

	// compute Dynamic lightmap Uv.
	static CUV	uvDLM;
	if(_DLMContext)		//  (NB: Suppose BTB kill this test).
	{
		// compute UV with DLM context info.
		uvDLM.U= pc.getS()* _DLMContext->DLMUScale + _DLMContext->DLMUBias;
		uvDLM.V= pc.getT()* _DLMContext->DLMVScale + _DLMContext->DLMVBias;
	}
	else
	{
		// just set UV so the vertex point to a black pixel (see CTextureDLM).
		uvDLM.U= 1;
		uvDLM.V= 1;
	}

	// If not VertexProgram (NB: Suppose BTB kill this test).
	if( !CLandscapeGlobals::VertexProgramEnabled )
	{
		// Set Pos. Set it local to the current center of landscape
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr, sizeof(CVector));
		*(CVector*)CurVBPtr= pVert->Src->Pos - CLandscapeGlobals::PZBModelPosition;
		// Set Uvs.
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff0, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff0)= uv;
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff1, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff1)= uvDLM;
	}
	else
	{
		// Else must setup Vertex program inputs
		// v[0]== StartPos.
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr, sizeof(CVector));
		*(CVector*)CurVBPtr= pVert->Src->StartPos;
		// v[8]== Tex0
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff0, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff0)= uv;
		// v[9]== Tex1
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff1, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff1)= uvDLM;

		// v[10]== GeomInfo.
		static CUV	geomInfo;
		geomInfo.U= pVert->Src->MaxFaceSize * CLandscapeGlobals::OORefineThreshold;
		geomInfo.V= pVert->Src->MaxNearLimit;
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.GeomInfoOff, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.GeomInfoOff)= geomInfo;

		// v[11]== EndPos - StartPos
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.DeltaPosOff, sizeof(CVector))
		*(CVector*)(CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.DeltaPosOff)=
			pVert->Src->EndPos - pVert->Src->StartPos;
	}
}
// ***************************************************************************
// NB: need to be inlined only for fillFar1VB() in this file.
inline void		CPatch::fillFar1VertexVB(CTessFarVertex *pVert)
{
	// The Buffers must have been locked
	nlassert(CLandscapeGlobals::CurrentFar1VBAllocator);
	nlassert(CLandscapeGlobals::CurrentFar1VBAllocator->bufferLocked());
	// VBInfo must be OK.
	nlassert(!CLandscapeGlobals::CurrentFar1VBAllocator->reallocationOccurs());

	static	uint8	*CurVBPtr;
	// Compute/build the new vertex.
	CurVBPtr= (uint8*)CLandscapeGlobals::CurrentFar1VBInfo.VertexCoordPointer;
	CurVBPtr+= pVert->Index1 * CLandscapeGlobals::CurrentFar1VBInfo.VertexSize;

	// NB: the filling order of data is important, for AGP write combiners.

	// compute Uvs.
	static CUV	uv;
	CParamCoord	pc= pVert->PCoord;
	if (Flags&NL_PATCH_FAR1_ROTATED)
	{
		uv.U= pc.getT()* Far1UScale + Far1UBias;
		uv.V= (1.f-pc.getS())* Far1VScale + Far1VBias;
	}
	else
	{
		uv.U= pc.getS()* Far1UScale + Far1UBias;
		uv.V= pc.getT()* Far1VScale + Far1VBias;
	}

	// compute Dynamic lightmap Uv.
	static CUV	uvDLM;
	if(_DLMContext)		//  (NB: Suppose BTB kill this test).
	{
		// compute UV with DLM context info.
		uvDLM.U= pc.getS()* _DLMContext->DLMUScale + _DLMContext->DLMUBias;
		uvDLM.V= pc.getT()* _DLMContext->DLMVScale + _DLMContext->DLMVBias;
	}
	else
	{
		// just set UV so the vertex point to a black pixel (see CTextureDLM).
		uvDLM.U= 1;
		uvDLM.V= 1;
	}

	// If not VertexProgram (NB: Suppose BTB kill this test).
	if( !CLandscapeGlobals::VertexProgramEnabled )
	{
		// Set Pos. Set it local to the current center of landscape
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr, sizeof(CVector));
		*(CVector*)CurVBPtr= pVert->Src->Pos - CLandscapeGlobals::PZBModelPosition;
		// Set Uvs.
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff0, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff0)= uv;
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff1, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff1)= uvDLM;
		// Set default color.
		static CRGBA	col(255,255,255,255);
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.ColorOff, sizeof(CRGBA));
		*(CRGBA*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.ColorOff)= col;
	}
	else
	{
		// Else must setup Vertex program inputs
		// v[0]== StartPos.
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr, sizeof(CVector));
		*(CVector*)CurVBPtr= pVert->Src->StartPos;
		// v[8]== Tex0
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff0, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff0)= uv;
		// v[9]== Tex1
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff1, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff1)= uvDLM;

		// v[10]== GeomInfo.
		static CUV	geomInfo;
		geomInfo.U= pVert->Src->MaxFaceSize * CLandscapeGlobals::OORefineThreshold;
		geomInfo.V= pVert->Src->MaxNearLimit;
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.GeomInfoOff, sizeof(CUV));
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.GeomInfoOff)= geomInfo;

		// v[11]== EndPos - StartPos
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.DeltaPosOff, sizeof(CVector))
		*(CVector*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.DeltaPosOff)=
			pVert->Src->EndPos - pVert->Src->StartPos;

		// v[12]== Alpha information
		// Hopefully, fillVBFar1Only() is called each Time the Far1 change, in preRender().
		// So TransitionSqrMin and OOTransitionSqrDelta in CPath are valid.
		geomInfo.U= TransitionSqrMin;
		geomInfo.V= OOTransitionSqrDelta;
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.AlphaInfoOff, sizeof(CUV))
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.AlphaInfoOff)= geomInfo;

	}
}
// ***************************************************************************
// NB: need to be inlined only for fillTileVB() in this file.
inline void		CPatch::fillTileVertexVB(CTessNearVertex *pVert)
{
	// The Buffers must have been locked
	nlassert(CLandscapeGlobals::CurrentTileVBAllocator);
	nlassert(CLandscapeGlobals::CurrentTileVBAllocator->bufferLocked());
	// VBInfo must be OK.
	nlassert(!CLandscapeGlobals::CurrentTileVBAllocator->reallocationOccurs());

	static	uint8	*CurVBPtr;
	// Compute/build the new vertex.
	CurVBPtr= (uint8*)CLandscapeGlobals::CurrentTileVBInfo.VertexCoordPointer;
	CurVBPtr+= pVert->Index * CLandscapeGlobals::CurrentTileVBInfo.VertexSize;


	// NB: the filling order of data is important, for AGP write combiners.

	// If not VertexProgram (NB: Suppose BTB kill this test).
	if( !CLandscapeGlobals::VertexProgramEnabled )
	{
		// Set Pos. Set it local to the current center of landscape
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr, sizeof(CVector))
		*(CVector*)CurVBPtr= pVert->Src->Pos - CLandscapeGlobals::PZBModelPosition;
		// Set Uvs.
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff0, sizeof(CUV))
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff0)= pVert->PUv0;
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff1, sizeof(CUV))
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff1)= pVert->PUv1;
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff2, sizeof(CUV))
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff2)= pVert->PUv2;
	}
	else
	{
		// Else must setup Vertex program inputs
		// v[0]== StartPos.
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr, sizeof(CVector))
		*(CVector*)CurVBPtr= pVert->Src->StartPos;
		// v[8]== Tex0
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff0, sizeof(CUV))
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff0)= pVert->PUv0;
		// v[9]== Tex1
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff1, sizeof(CUV))
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff1)= pVert->PUv1;
		// v[12]== Tex2
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff2, sizeof(CUV))
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.TexCoordOff2)= pVert->PUv2;

		// v[10]== GeomInfo.
		static CUV	geomInfo;
		geomInfo.U= pVert->Src->MaxFaceSize * CLandscapeGlobals::OORefineThreshold;
		geomInfo.V= pVert->Src->MaxNearLimit;
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.GeomInfoOff, sizeof(CUV))
		*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.GeomInfoOff)= geomInfo;

		// v[11]== EndPos - StartPos
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.DeltaPosOff, sizeof(CVector))
		*(CVector*)(CurVBPtr + CLandscapeGlobals::CurrentTileVBInfo.DeltaPosOff)=
			pVert->Src->EndPos - pVert->Src->StartPos;
	}
}


// ***************************************************************************
void		CPatch::fillFar0VertexListVB(CTessList<CTessFarVertex>  &vertList)
{
	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		fillFar0VertexVB(pVert);
	}
}


// ***************************************************************************
void		CPatch::fillFar1VertexListVB(CTessList<CTessFarVertex>  &vertList)
{
	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		fillFar1VertexVB(pVert);
	}
}


// ***************************************************************************
void		CPatch::fillTileVertexListVB(CTessList<CTessNearVertex>  &vertList)
{
	// Traverse the vertList.
	CTessNearVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessNearVertex*)pVert->Next)
	{
		fillTileVertexVB(pVert);
	}
}



// ***************************************************************************
void			CPatch::fillVB()
{
	// Fill Far0.
	//=======
	// fill only if no reallcoation occurs
	if(Far0>0 && !CLandscapeGlobals::CurrentFar0VBAllocator->reallocationOccurs() )
	{
		// Fill Far0 VB.
		fillFar0VertexListVB(MasterBlock.FarVertexList);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// fill only if tblock visible.
			if( tblock.visibleFar0() )
				fillFar0VertexListVB(tblock.FarVertexList);
		}
	}
	else if(Far0==0 && !CLandscapeGlobals::CurrentTileVBAllocator->reallocationOccurs() )
	{
		// Fill Tile VB.
		// No Tiles in MasterBlock!!
		// Traverse the TessBlocks to add vertices.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// fill only if tblock visible.
			if( tblock.visibleTile() )
				fillTileVertexListVB(tblock.NearVertexList);
		}
	}

	// Fill Far1.
	//=======
	if(Far1>0 && !CLandscapeGlobals::CurrentFar1VBAllocator->reallocationOccurs() )
	{
		// Fill VB.
		fillFar1VertexListVB(MasterBlock.FarVertexList);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// fill only if tblock visible.
			if( tblock.visibleFar1() )
				fillFar1VertexListVB(tblock.FarVertexList);
		}
	}

}


// ***************************************************************************
void		CPatch::fillVBIfVisible()
{
	if(isRenderClipped()==false)
		fillVB();
}


// ***************************************************************************
void		CPatch::fillVBFar0Only()
{
	if(Far0>0 && !CLandscapeGlobals::CurrentFar0VBAllocator->reallocationOccurs() )
	{
		// Fill Far0 VB.
		fillFar0VertexListVB(MasterBlock.FarVertexList);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// fill only if tblock visible.
			if( tblock.visibleFar0() )
				fillFar0VertexListVB(tblock.FarVertexList);
		}
	}
}


// ***************************************************************************
void		CPatch::fillVBFar1Only()
{
	if(Far1>0 && !CLandscapeGlobals::CurrentFar1VBAllocator->reallocationOccurs() )
	{
		// Fill VB.
		fillFar1VertexListVB(MasterBlock.FarVertexList);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// fill only if tblock visible.
			if( tblock.visibleFar1() )
				fillFar1VertexListVB(tblock.FarVertexList);
		}
	}
}

// ***************************************************************************
// ***************************************************************************
// VB Software Geomorph / Alpha.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CPatch::computeGeomorphVertexList(CTessList<CTessFarVertex>  &vertList)
{
	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		// compute geomorph.
		pVert->Src->computeGeomPos();
	}
}


// ***************************************************************************
void		CPatch::computeGeomorphFar0VertexListVB(CTessList<CTessFarVertex>  &vertList)
{
	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		static	uint8	*CurVBPtr;
		// Compute/build the new vertex.
		CurVBPtr= (uint8*)CLandscapeGlobals::CurrentFar0VBInfo.VertexCoordPointer;
		CurVBPtr+= pVert->Index0 * CLandscapeGlobals::CurrentFar0VBInfo.VertexSize;

		// Set Geomorphed Position. Set it local to the current center of landscape
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr, sizeof(CVector))
		*(CVector*)CurVBPtr= pVert->Src->Pos - CLandscapeGlobals::PZBModelPosition;
	}
}


// ***************************************************************************
void		CPatch::computeGeomorphAlphaFar1VertexListVB(CTessList<CTessFarVertex>  &vertList)
{
	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		static	uint8	*CurVBPtr;
		// Compute/build the new vertex.
		CurVBPtr= (uint8*)CLandscapeGlobals::CurrentFar1VBInfo.VertexCoordPointer;
		CurVBPtr+= pVert->Index1 * CLandscapeGlobals::CurrentFar1VBInfo.VertexSize;

		// NB: the filling order of data is important, for AGP write combiners.

		// Set Geomorphed Position. Set it local to the current center of landscape
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr, sizeof(CVector))
		*(CVector*)CurVBPtr= pVert->Src->Pos - CLandscapeGlobals::PZBModelPosition;

		// Set Alpha color.
		static CRGBA	col(255,255,255,255);
		// For Far1, use alpha fro transition.
		// Prefer Use Pos, because of caching. So little difference between Soft and VertexProgram mode.
		float	f= (pVert->Src->Pos - CLandscapeGlobals::RefineCenter).sqrnorm();
		f= (f-TransitionSqrMin) * OOTransitionSqrDelta;
		clamp(f,0,1);
		col.A= (uint8)(f*255);
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.ColorOff, sizeof(CRGBA))
		*(CRGBA*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.ColorOff)= col;
	}
}


// ***************************************************************************
void		CPatch::computeGeomorphTileVertexListVB(CTessList<CTessNearVertex>  &vertList)
{
	// Traverse the vertList.
	CTessNearVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessNearVertex*)pVert->Next)
	{
		static	uint8	*CurVBPtr;
		// Compute/build the new vertex.
		CurVBPtr= (uint8*)CLandscapeGlobals::CurrentTileVBInfo.VertexCoordPointer;
		CurVBPtr+= pVert->Index * CLandscapeGlobals::CurrentTileVBInfo.VertexSize;

		// Set Geomorphed Position. Set it local to the current center of landscape
		CHECK_VBA_RANGE(CLandscapeGlobals::CurrentTileVBInfo.Accessor, CurVBPtr, sizeof(CVector))
		*(CVector*)CurVBPtr= pVert->Src->Pos - CLandscapeGlobals::PZBModelPosition;
	}
}



// ***************************************************************************
void		CPatch::computeSoftwareGeomorphAndAlpha()
{
	if(isRenderClipped())
		return;

	// Compute Geomorph.
	//=======
	// Need only to fill CTessVertex, so do it only for FarVertices
	// Hence Geomorph is done twice on edges of patches!!.
	// If not too near for precise, fast compute of geomorph.
	if( TessBlocks.empty() )
	{
		// Just update all vertices of master block.
		computeGeomorphVertexList(MasterBlock.FarVertexList);
	}
	else
	{
		// update all vertices of master block.
		computeGeomorphVertexList(MasterBlock.FarVertexList);
		// update vertices of others block.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// Precise Clip.
			if(!tblock.getClipped())
			{
				// compute the geomorph of the vertices in the tessblock.
				computeGeomorphVertexList(tblock.FarVertexList);
			}
		}
	}


	// Fill Far0.
	//=======
	if(Far0>0)
	{
		// Fill Far0 VB.
		computeGeomorphFar0VertexListVB(MasterBlock.FarVertexList);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// Precise Clip.
			if( tblock.visibleFar0() )
				computeGeomorphFar0VertexListVB(tblock.FarVertexList);
		}
	}
	else if(Far0==0)
	{
		// Fill Tile VB.
		// No Tiles in MasterBlock!!
		// Traverse the TessBlocks to compute vertices.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// Precise Clip.
			if( tblock.visibleTile() )
				computeGeomorphTileVertexListVB(tblock.NearVertexList);
		}
	}

	// Fill Far1.
	//=======
	if(Far1>0)
	{
		// Fill VB.
		computeGeomorphAlphaFar1VertexListVB(MasterBlock.FarVertexList);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// Precise Clip.
			if( tblock.visibleFar1() )
				computeGeomorphAlphaFar1VertexListVB(tblock.FarVertexList);
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// VB clip Allocate/Filling.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CPatch::updateClipPatchVB(bool renderClipped)
{
	// If now the patch is invisible
	if(renderClipped)
	{
		// Then delete vertices.
		deleteVBAndFaceVector();

		// Now, all vertices in VB are deleted.
		// Force clip state of all TessBlocks, so no allocation will be done on Vertices in VB.
		if(!TessBlocks.empty())
		{
			for(uint i=0; i<TessBlocks.size();i++)
			{
				CTessBlock	&tblock= TessBlocks[i];
				tblock.forceClip();
			}
		}
	}
	else
	{
		// else allocate / fill them.
		allocateVBAndFaceVector();
		// NB: fillVB() test if any reallocation occurs.
		fillVB();
	}
}


// ***************************************************************************
// ***************************************************************************
// VB refine Allocate/Filling.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void		CPatch::checkCreateVertexVBFar(CTessFarVertex *pVert)
{
	nlassert(pVert);
	// If visible, and Far0 in !Tile Mode, allocate.
	// NB: must test Far0>0 because vertices are reallocated in preRender() if a change of Far occurs.
	if(!isRenderClipped() && Far0>0 && pVert->OwnerBlock->visibleFar0() )
	{
		pVert->Index0= CLandscapeGlobals::CurrentFar0VBAllocator->allocateVertex();
	}

	// Idem for Far1
	if(!isRenderClipped() && Far1>0 && pVert->OwnerBlock->visibleFar1())
	{
		pVert->Index1= CLandscapeGlobals::CurrentFar1VBAllocator->allocateVertex();
	}

}


// ***************************************************************************
void		CPatch::checkFillVertexVBFar(CTessFarVertex *pVert)
{
	nlassert(pVert);
	// If visible, and Far0 in !Tile Mode, try to fill.
	// NB: must test Far0>0 because vertices are reallocated in preRender() if a change of Far occurs.
	if(!isRenderClipped() && Far0>0 && pVert->OwnerBlock->visibleFar0())
	{
		if( !CLandscapeGlobals::CurrentFar0VBAllocator->reallocationOccurs() )
			fillFar0VertexVB(pVert);
	}

	// Idem for Far1
	if(!isRenderClipped() && Far1>0 && pVert->OwnerBlock->visibleFar1())
	{
		if( !CLandscapeGlobals::CurrentFar1VBAllocator->reallocationOccurs() )
			fillFar1VertexVB(pVert);
	}
}


// ***************************************************************************
void		CPatch::checkCreateVertexVBNear(CTessNearVertex	*pVert)
{
	nlassert(pVert);
	// If visible, and Far0 in Tile Mode, allocate.
	// NB: must test Far0==0 because vertices are reallocated in preRender() if a change of Far occurs.
	if(!isRenderClipped() && Far0==0 && pVert->OwnerBlock->visibleTile())
	{
		pVert->Index= CLandscapeGlobals::CurrentTileVBAllocator->allocateVertex();
	}
}


// ***************************************************************************
void		CPatch::checkFillVertexVBNear(CTessNearVertex	*pVert)
{
	nlassert(pVert);
	// If visible, and Far0 in Tile Mode, try to fill.
	// NB: must test Far0==0 because vertices are reallocated in preRender() if a change of Far occurs.
	if(!isRenderClipped()&& Far0==0 && pVert->OwnerBlock->visibleTile() )
	{
		// try to fill.
		if( !CLandscapeGlobals::CurrentTileVBAllocator->reallocationOccurs() )
			fillTileVertexVB(pVert);
	}
}


// ***************************************************************************
void		CPatch::checkDeleteVertexVBFar(CTessFarVertex *pVert)
{
	nlassert(pVert);
	// If visible, and Far0 in !Tile Mode, ok, the vertex exist in VB, so delete.
	// NB: must test Far0>0 because vertices are deleted in preRender() if a change of Far occurs.
	if(!isRenderClipped() && Far0>0 && pVert->OwnerBlock->visibleFar0() )
	{
		CLandscapeGlobals::CurrentFar0VBAllocator->deleteVertex(pVert->Index0);
	}

	// Idem for Far1
	if(!isRenderClipped() && Far1>0  && pVert->OwnerBlock->visibleFar1() )
	{
		CLandscapeGlobals::CurrentFar1VBAllocator->deleteVertex(pVert->Index1);
	}
}

// ***************************************************************************
void		CPatch::checkDeleteVertexVBNear(CTessNearVertex	*pVert)
{
	nlassert(pVert);
	// If visible, and Far0 in Tile Mode, ok, the vertex exist in VB, so delete.
	// NB: must test Far0==0 because vertices are deleted in preRender() if a change of Far occurs.
	if(!isRenderClipped() && Far0==0 && pVert->OwnerBlock->visibleTile() )
	{
		CLandscapeGlobals::CurrentTileVBAllocator->deleteVertex(pVert->Index);
	}
}


// ***************************************************************************
// ***************************************************************************
// VB DLM Filling
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CPatch::fillFar0DLMUvOnlyVertexListVB(CTessList<CTessFarVertex>  &vertList)
{
	// The Buffers must have been locked
	nlassert(CLandscapeGlobals::CurrentFar0VBAllocator);
	nlassert(CLandscapeGlobals::CurrentFar0VBAllocator->bufferLocked());
	// VBInfo must be OK.
	nlassert(!CLandscapeGlobals::CurrentFar0VBAllocator->reallocationOccurs());

	static	uint8	*CurVBPtr;
	static CUV		uvDLM;

	// If the DLMContext exist
	if(_DLMContext)
	{
		// Traverse the vertList, to compute new uvDLM
		CTessFarVertex	*pVert;
		for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
		{
			// Compute/build the new vertex.
			CurVBPtr= (uint8*)CLandscapeGlobals::CurrentFar0VBInfo.VertexCoordPointer;
			CurVBPtr+= pVert->Index0 * CLandscapeGlobals::CurrentFar0VBInfo.VertexSize;

			// compute Uvs.
			CParamCoord	pc= pVert->PCoord;

			// compute Dynamic lightmap Uv with DLM context info.
			uvDLM.U= pc.getS()* _DLMContext->DLMUScale + _DLMContext->DLMUBias;
			uvDLM.V= pc.getT()* _DLMContext->DLMVScale + _DLMContext->DLMVBias;

			// Set Uv DLM only (NB: same code for VertexProgram or not, only TexCoordOff1 may change).
			CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff1, sizeof(CUV))
			*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff1)= uvDLM;
		}
	}
	// else, reset all Uvs
	else
	{
		// just set UV so the vertex point to a black pixel (see CTextureDLM).
		uvDLM.U= 1;
		uvDLM.V= 1;

		// Traverse the vertList, to reset uv
		CTessFarVertex	*pVert;
		for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
		{
			// Compute/build the new vertex.
			CurVBPtr= (uint8*)CLandscapeGlobals::CurrentFar0VBInfo.VertexCoordPointer;
			CurVBPtr+= pVert->Index0 * CLandscapeGlobals::CurrentFar0VBInfo.VertexSize;

			// Set Uv DLM only (NB: same code for VertexProgram or not, only TexCoordOff1 may change).
			CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar0VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff1, sizeof(CUV))
			*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar0VBInfo.TexCoordOff1)= uvDLM;
		}
	}
}

// ***************************************************************************
void		CPatch::fillFar1DLMUvOnlyVertexListVB(CTessList<CTessFarVertex>  &vertList)
{
	// The Buffers must have been locked
	nlassert(CLandscapeGlobals::CurrentFar1VBAllocator);
	nlassert(CLandscapeGlobals::CurrentFar1VBAllocator->bufferLocked());
	// VBInfo must be OK.
	nlassert(!CLandscapeGlobals::CurrentFar1VBAllocator->reallocationOccurs());

	static	uint8	*CurVBPtr;
	static CUV		uvDLM;

	// If the DLMContext exist
	if(_DLMContext)
	{
		// Traverse the vertList, to compute new uvDLM
		CTessFarVertex	*pVert;
		for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
		{
			// Compute/build the new vertex.
			CurVBPtr= (uint8*)CLandscapeGlobals::CurrentFar1VBInfo.VertexCoordPointer;
			CurVBPtr+= pVert->Index1 * CLandscapeGlobals::CurrentFar1VBInfo.VertexSize;

			// compute Uvs.
			CParamCoord	pc= pVert->PCoord;

			// compute Dynamic lightmap Uv with DLM context info.
			uvDLM.U= pc.getS()* _DLMContext->DLMUScale + _DLMContext->DLMUBias;
			uvDLM.V= pc.getT()* _DLMContext->DLMVScale + _DLMContext->DLMVBias;

			// Set Uv DLM only (NB: same code for VertexProgram or not, only TexCoordOff1 may change).
			CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff1, sizeof(CUV))
			*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff1)= uvDLM;
		}
	}
	// else, reset all Uvs
	else
	{
		// just set UV so the vertex point to a black pixel (see CTextureDLM).
		uvDLM.U= 1;
		uvDLM.V= 1;

		// Traverse the vertList, to reset uv
		CTessFarVertex	*pVert;
		for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
		{
			// Compute/build the new vertex.
			CurVBPtr= (uint8*)CLandscapeGlobals::CurrentFar1VBInfo.VertexCoordPointer;
			CurVBPtr+= pVert->Index1 * CLandscapeGlobals::CurrentFar1VBInfo.VertexSize;

			// Set Uv DLM only (NB: same code for VertexProgram or not, only TexCoordOff1 may change).
			CHECK_VBA_RANGE(CLandscapeGlobals::CurrentFar1VBInfo.Accessor, CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff1, sizeof(CUV))
			*(CUV*)(CurVBPtr + CLandscapeGlobals::CurrentFar1VBInfo.TexCoordOff1)= uvDLM;
		}
	}
}


// ***************************************************************************
void		CPatch::fillVBFarsDLMUvOnly()
{
	// Do it for Far0.
	if(Far0>0 && !CLandscapeGlobals::CurrentFar0VBAllocator->reallocationOccurs() )
	{
		// Fill Far0 VB.
		fillFar0DLMUvOnlyVertexListVB(MasterBlock.FarVertexList);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// fill only if tblock visible.
			if( tblock.visibleFar0() )
				fillFar0DLMUvOnlyVertexListVB(tblock.FarVertexList);
		}
	}

	// Do it for Far1.
	if(Far1>0 && !CLandscapeGlobals::CurrentFar1VBAllocator->reallocationOccurs() )
	{
		// Fill VB.
		fillFar1DLMUvOnlyVertexListVB(MasterBlock.FarVertexList);
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			// fill only if tblock visible.
			if( tblock.visibleFar1() )
				fillFar1DLMUvOnlyVertexListVB(tblock.FarVertexList);
		}
	}
}



} // NL3D
