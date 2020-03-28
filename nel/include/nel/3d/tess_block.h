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

#ifndef NL_TESS_BLOCK_H
#define NL_TESS_BLOCK_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tessellation.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/bsphere.h"
#include <list>


namespace NL3D
{


// clip Only with Left/Right and Top/Bottom planes, for faster clip.
#define	NL3D_TESSBLOCK_NUM_CLIP_PLANE	4


// block of 2*2 tiles, for accurate clipping.
#define	NL3D_TESSBLOCK_TILESIZE	4


class	CPatchRdrPass;
class	CLandscapeFaceVectorManager;
class	CLandscapeVegetableBlock;

// ***************************************************************************
/**
 * A block of 2*2 tiles, for accurate clipping.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTessBlock
{
public:
	/*
		Cache optim:
		Important to store them here for better Cache access in CPatch::MasterBlock (near too TessBlocks)
	*/
	// FaceVectors.
	TLandscapeIndexType	*Far0FaceVector;
	TLandscapeIndexType	*Far1FaceVector;
	// tiles one are stored in material.

private:
	// Clip info.
	NLMISC::CAABBox		BBox;
	NLMISC::CBSphere	BSphere;
	bool		Empty;			// empty bbox? (first extend).

	// Clip result.
	bool		Clipped;
	bool		FullFar1;		// block Entirely IN transition. (all alpha of Far1 should be 255)
	bool		EmptyFar1;		// block Entirely OUT transition. (all alpha of Far1 should be 0)

public:

	// Vertices.
	CTessList<CTessFarVertex>	FarVertexList;
	CTessList<CTessNearVertex>	NearVertexList;

	// Faces.
	CTessList<CTessFace>		FarFaceList;
	CTileMaterial				*RdrTileRoot[NL3D_TESSBLOCK_TILESIZE];
	// contains FarFaceList.size() + "RdrTileRoot.size()"
	uint						FaceTileMaterialRefCount;
	// contains only "RdrTileRoot.size()"
	uint						TileMaterialRefCount;


	// For memory optimisation, Info for Lightmap per block of 2x2 tiles are stored here too.
	uint						LightMapId;
	uint						LightMapRefCount;
	CPatchRdrPass				*LightMapRdrPass;


	// For micro-vegetation, little tessBlocks got InstanceGroups
	CLandscapeVegetableBlock	*VegetableBlock;


public:
	CTessBlock();
	// in dtor, remove me from list of TessBlock to modify, if needed.
	~CTessBlock();

	// Must have a patch, for good TessBlock FaceVector (re)construction.
	void			init(CPatch *patch);
	CPatch			*getPatch();

	// BBox.
	void			extendSphereFirst(const CVector &vec);
	void			extendSphereAdd(const CVector &vec);
	void			extendSphereCompile();

	// Reset clip to no clipped.
	void			resetClip();
	// force clip to clipped.
	void			forceClip();
	// Clip. (set Clipped to true/false). Use the CurrentPyramid static.
	static CPlane	CurrentPyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE];
	void			clip();
	// Clip Tile/Far.
	void			clipFar(const CVector &refineCenter, float tileDistNear, float farTransition);

	// Clip result.
	bool			getClipped() const {return Clipped;}
	bool			visibleFar0() const {return !Clipped && !FullFar1;}
	bool			visibleTile() const {return !Clipped && !FullFar1;}
	bool			visibleFar1() const {return !Clipped && !EmptyFar1;}

	// create and fill FaceVector, from the FaceList.
	void			createFaceVectorFar0(CLandscapeFaceVectorManager &mgr);
	void			deleteFaceVectorFar0(CLandscapeFaceVectorManager &mgr);
	void			createFaceVectorFar1(CLandscapeFaceVectorManager &mgr);
	void			deleteFaceVectorFar1(CLandscapeFaceVectorManager &mgr);
	void			createFaceVectorTile(CLandscapeFaceVectorManager &mgr);
	void			deleteFaceVectorTile(CLandscapeFaceVectorManager &mgr);
	// suppose FV is created, and refill it.
	void			refillFaceVectorFar0();
	void			refillFaceVectorFar1();
	void			refillFaceVectorTile();



	// append/remove to modify list. This is for FaceVector update durint refine(), addZone() ....
	bool			isInModifyList() const {return _PrecToModify!=NULL;}
	// append to modify list has the side effect od deleting ALL FaceVector.
	void			appendToModifyListAndDeleteFaceVector(CTessBlock &root, CLandscapeFaceVectorManager &mgr);
	void			removeFromModifyList();


public:
	// For CLandscape only.
	CTessBlock		*getNextToModify() {return _NextToModify;}

private:
	CPatch			*_Patch;

	// If in list, prec is !NULL.
	CTessBlock		*_PrecToModify;
	CTessBlock		*_NextToModify;
};


} // NL3D


#endif // NL_TESS_BLOCK_H

/* End of tess_block.h */
