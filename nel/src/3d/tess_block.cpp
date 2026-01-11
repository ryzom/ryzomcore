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

#include "nel/3d/tess_block.h"
#include "nel/3d/patch_rdr_pass.h"
#include "nel/3d/landscape_face_vector_manager.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


CPlane	CTessBlock::CurrentPyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE];



// ***************************************************************************
CTessBlock::CTessBlock()
{
	_Patch= NULL;

	// init bounding info.
	Empty= true;
	// By default, the tessBlock is clipped.
	Clipped= true;
	FullFar1= false;
	EmptyFar1= false;

	// init vert/face list.
	for(sint i=0;i<NL3D_TESSBLOCK_TILESIZE;i++)
	{
		RdrTileRoot[i]=NULL;
	}

	// init LightMap.
	LightMapRefCount= 0;

	Far0FaceVector= NULL;
	Far1FaceVector= NULL;

	_PrecToModify= NULL;
	_NextToModify= NULL;

	FaceTileMaterialRefCount= 0;
	TileMaterialRefCount= 0;

	// Micro-vegetation.
	VegetableBlock= NULL;
}


// ***************************************************************************
CTessBlock::~CTessBlock()
{
	if(isInModifyList())
	{
		removeFromModifyList();
	}

	// LightMap should be released
	nlassert(LightMapRefCount==0);
}

// ***************************************************************************
void			CTessBlock::init(CPatch *patch)
{
	_Patch= patch;
}
// ***************************************************************************
CPatch			*CTessBlock::getPatch()
{
	return _Patch;
}



// ***************************************************************************
void			CTessBlock::extendSphereFirst(const CVector &vec)
{
	if(Empty)
	{
		Empty= false;
		BBox.setCenter(vec);
		BBox.setHalfSize(CVector::Null);
	}
	else
		extendSphereAdd(vec);
}

// ***************************************************************************
void			CTessBlock::extendSphereAdd(const CVector &vec)
{
	if( !BBox.include(vec) )
		BBox.extend(vec);
}

// ***************************************************************************
void			CTessBlock::extendSphereCompile()
{
	BSphere.Center= BBox.getCenter();
	BSphere.Radius= BBox.getRadius();
}


// ***************************************************************************
void			CTessBlock::resetClip()
{
	Clipped= false;
	FullFar1= false;
	EmptyFar1= false;
}


// ***************************************************************************
void			CTessBlock::forceClip()
{
	Clipped= true;
}


// ***************************************************************************
void			CTessBlock::clip()
{
	Clipped= false;
	for(sint i=0;i<NL3D_TESSBLOCK_NUM_CLIP_PLANE;i++)
	{
		// If entirely out.
		if(!BSphere.clipBack( CTessBlock::CurrentPyramid[i] ))
		{
			Clipped= true;
			break;
		}
	}
}
// ***************************************************************************
void			CTessBlock::clipFar(const CVector &refineCenter, float tileDistNear, float farTransition)
{
	float	r= (refineCenter-BSphere.Center).norm();
	if( (r-BSphere.Radius) > tileDistNear)
	{
		FullFar1= true;
	}
	else
	{
		if( (r+BSphere.Radius) < (tileDistNear-farTransition) )
			EmptyFar1= true;
	}
}



// ***************************************************************************
void			CTessBlock::refillFaceVectorFar0()
{
	// If size is not 0.
	if(FarFaceList.size()>0)
	{
		nlassert(Far0FaceVector!=NULL);

		// Fill this faceVector, with FarFaceList
		CTessFace	*pFace;
		TLandscapeIndexType	*dest= Far0FaceVector+1;
		for(pFace= FarFaceList.begin(); pFace; pFace= (CTessFace*)pFace->Next)
		{
			#if defined(NL_DEBUG) && defined(NL_LANDSCAPE_INDEX16)
				nlassert(pFace->FVBase->Index0 <= 0xffff);
				nlassert(pFace->FVLeft->Index0 <= 0xffff);
				nlassert(pFace->FVRight->Index0 <= 0xffff);
			#endif
			*(dest++)= (TLandscapeIndexType) pFace->FVBase->Index0;
			*(dest++)= (TLandscapeIndexType) pFace->FVLeft->Index0;
			*(dest++)= (TLandscapeIndexType) pFace->FVRight->Index0;
		}
	}
}


// ***************************************************************************
void			CTessBlock::createFaceVectorFar0(CLandscapeFaceVectorManager &mgr)
{
	nlassert(Far0FaceVector==NULL);
	// If size is not 0.
	if(FarFaceList.size()>0)
	{
		// Create a faceVector of the wanted triangles size.
		Far0FaceVector= mgr.createFaceVector(FarFaceList.size());

		// init.
		refillFaceVectorFar0();
	}

}
// ***************************************************************************
void			CTessBlock::deleteFaceVectorFar0(CLandscapeFaceVectorManager &mgr)
{
	if(Far0FaceVector)
	{
		mgr.deleteFaceVector(Far0FaceVector);
		Far0FaceVector= NULL;
	}
}

// ***************************************************************************
void			CTessBlock::refillFaceVectorFar1()
{
	// If size is not 0.
	if(FarFaceList.size()>0)
	{
		nlassert(Far1FaceVector!=NULL);
		// Fill this faceVector, with FarFaceList
		CTessFace	*pFace;
		TLandscapeIndexType	*dest= Far1FaceVector+1;
		for(pFace= FarFaceList.begin(); pFace; pFace= (CTessFace*)pFace->Next)
		{
			#if defined(NL_DEBUG) && defined(NL_LANDSCAPE_INDEX16)
				nlassert(pFace->FVBase->Index1 <= 0xffff);
				nlassert(pFace->FVLeft->Index1 <= 0xffff);
				nlassert(pFace->FVRight->Index1 <= 0xffff);
			#endif
			*(dest++)= (TLandscapeIndexType) pFace->FVBase->Index1;
			*(dest++)= (TLandscapeIndexType) pFace->FVLeft->Index1;
			*(dest++)= (TLandscapeIndexType) pFace->FVRight->Index1;
		}
	}
}

// ***************************************************************************
void			CTessBlock::createFaceVectorFar1(CLandscapeFaceVectorManager &mgr)
{
	nlassert(Far1FaceVector==NULL);
	// If size is not 0.
	if(FarFaceList.size()>0)
	{
		// Create a faceVector of the wanted triangles size.
		Far1FaceVector= mgr.createFaceVector(FarFaceList.size());

		// init.
		refillFaceVectorFar1();
	}
}
// ***************************************************************************
void			CTessBlock::deleteFaceVectorFar1(CLandscapeFaceVectorManager &mgr)
{
	if(Far1FaceVector)
	{
		mgr.deleteFaceVector(Far1FaceVector);
		Far1FaceVector= NULL;
	}
}


// ***************************************************************************
void			CTessBlock::refillFaceVectorTile()
{
	// For all tiles existing, and for all facePass existing, fill the faceVector.
	for(uint tileId=0; tileId<NL3D_TESSBLOCK_TILESIZE; tileId++)
	{
		// if tile exist.
		if(RdrTileRoot[tileId])
		{
			// For all Pass faces of the tile.
			for(uint facePass=0; facePass<NL3D_MAX_TILE_FACE; facePass++)
			{
				CTessList<CTileFace>	&faceList= RdrTileRoot[tileId]->TileFaceList[facePass];
				TLandscapeIndexType		*faceVector= RdrTileRoot[tileId]->TileFaceVectors[facePass];
				// If some triangles create them.
				if(faceList.size()>0)
				{
					nlassert( faceVector!=NULL );

					// Fill this faceVector, with the TileFaceList
					CTileFace	*pFace;
					TLandscapeIndexType	*dest= faceVector+1;
					for(pFace= faceList.begin(); pFace; pFace= (CTileFace*)pFace->Next)
					{
						#if defined(NL_DEBUG) && defined(NL_LANDSCAPE_INDEX16)
							nlassert(pFace->V[CTessFace::IdUvBase]->Index <= 0xffff);
							nlassert(pFace->V[CTessFace::IdUvLeft]->Index <= 0xffff);
							nlassert(pFace->V[CTessFace::IdUvRight]->Index <= 0xffff);
						#endif
						*(dest++)= (TLandscapeIndexType) pFace->V[CTessFace::IdUvBase]->Index;
						*(dest++)= (TLandscapeIndexType) pFace->V[CTessFace::IdUvLeft]->Index;
						*(dest++)= (TLandscapeIndexType) pFace->V[CTessFace::IdUvRight]->Index;
					}
				}
			}
		}
	}
}


// ***************************************************************************
void			CTessBlock::createFaceVectorTile(CLandscapeFaceVectorManager &mgr)
{
	// For all tiles existing, and for all facePass existing, create the faceVector.
	for(uint tileId=0; tileId<NL3D_TESSBLOCK_TILESIZE; tileId++)
	{
		// if tile exist.
		if(RdrTileRoot[tileId])
		{
			// For all Pass faces of the tile.
			for(uint facePass=0; facePass<NL3D_MAX_TILE_FACE; facePass++)
			{
				CTessList<CTileFace>	&faceList= RdrTileRoot[tileId]->TileFaceList[facePass];
				TLandscapeIndexType		*&faceVector= RdrTileRoot[tileId]->TileFaceVectors[facePass];
				// If some triangles create them.
				if(faceList.size()>0)
				{
					// Create a faceVector of the wanted triangles size.
					faceVector= mgr.createFaceVector(faceList.size());
				}
			}
		}
	}

	// init.
	refillFaceVectorTile();
}
// ***************************************************************************
void			CTessBlock::deleteFaceVectorTile(CLandscapeFaceVectorManager &mgr)
{
	// For all tiles existing, and for all facePass existing, delete the faceVector.
	for(uint tileId=0; tileId<NL3D_TESSBLOCK_TILESIZE; tileId++)
	{
		// if tile exist.
		if(RdrTileRoot[tileId])
		{
			// For all Pass faces of the tile.
			for(uint facePass=0; facePass<NL3D_MAX_TILE_FACE; facePass++)
			{
				TLandscapeIndexType	*&faceVector= RdrTileRoot[tileId]->TileFaceVectors[facePass];
				// If the faceVector exist, delete it.
				if(faceVector)
				{
					mgr.deleteFaceVector(faceVector);
					faceVector= NULL;
				}
			}
		}
	}
}


// ***************************************************************************
void			CTessBlock::appendToModifyListAndDeleteFaceVector(CTessBlock &root, CLandscapeFaceVectorManager &mgr)
{
	// If already appened, return.
	if(isInModifyList())
		return;

	// append to root.
	_PrecToModify= &root;
	_NextToModify= root._NextToModify;
	if(root._NextToModify)
		root._NextToModify->_PrecToModify= this;
	root._NextToModify= this;

	// Then delete All faceVector that may exist.
	deleteFaceVectorFar0(mgr);
	deleteFaceVectorFar1(mgr);
	deleteFaceVectorTile(mgr);
}
// ***************************************************************************
void			CTessBlock::removeFromModifyList()
{
	// If already removed, return.
	// _PrecToModify must be !NULL
	if(!isInModifyList())
		return;

	// unlink.
	_PrecToModify->_NextToModify= _NextToModify;
	if(_NextToModify)
		_NextToModify->_PrecToModify= _PrecToModify;
	_PrecToModify= NULL;
	_NextToModify= NULL;
}


} // NL3D
