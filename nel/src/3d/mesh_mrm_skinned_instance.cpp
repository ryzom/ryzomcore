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

#include "nel/3d/mesh_mrm_skinned_instance.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/raw_skinned.h"
#include "nel/3d/shifted_triangle_cache.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/scene.h"


using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
CMeshMRMSkinnedInstance::~CMeshMRMSkinnedInstance()
{
	// Auto detach me from skeleton. Must do it here, not in ~CTransform().
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// hrc and clip hierarchy is modified.
		_FatherSkeletonModel->detachSkeletonSon(this);
		nlassert(_FatherSkeletonModel==NULL);
		// If skinned, setApplySkin(false) should have been called through detachSkeletonSon()
		nlassert(_RawSkinCache== NULL);
		nlassert(_ShiftedTriangleCache== NULL);
	}
}


// ***************************************************************************
void		CMeshMRMSkinnedInstance::registerBasic()
{
	CScene::registerModel(MeshMRMSkinnedInstanceId, MeshBaseInstanceId, CMeshMRMSkinnedInstance::creator);
}


// ***************************************************************************
void		CMeshMRMSkinnedInstance::clearRawSkinCache()
{
	delete	_RawSkinCache;
	_RawSkinCache= NULL;
}

// ***************************************************************************
void		CMeshMRMSkinnedInstance::clearShiftedTriangleCache()
{
	delete	_ShiftedTriangleCache;
	_ShiftedTriangleCache= NULL;
}

// ***************************************************************************
void		CMeshMRMSkinnedInstance::setApplySkin(bool state)
{
	// Call parents method
	CMeshBaseInstance::setApplySkin (state);

	// Get a pointer on the shape
	CMeshMRMSkinned *pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);

	// Recompute the id
	if (state)
	{
		pMesh->computeBonesId (_FatherSkeletonModel);
	}

	// update the skeleton usage according to the mesh.
	pMesh->updateSkeletonUsage(_FatherSkeletonModel, state);

	// If unbinded, clean all the cache.
	if(state==false)
	{
		clearRawSkinCache();
		clearShiftedTriangleCache();
	}
}


// ***************************************************************************
void		CMeshMRMSkinnedInstance::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	if(Shape)
	{
		// Get a pointer on the shape.
		CMeshMRMSkinned *pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);
		// Affect the mesh directly.
		pMesh->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
	}
}


// ***************************************************************************
const std::vector<sint32>			*CMeshMRMSkinnedInstance::getSkinBoneUsage() const
{
	// Get a pointer on the shape
	CMeshMRMSkinned *pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);

	// Recompute the id, if needed
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get ids.
	return &pMesh->getMeshGeom().getSkinBoneUsage();
}


// ***************************************************************************
const std::vector<NLMISC::CBSphere>	*CMeshMRMSkinnedInstance::getSkinBoneSphere() const
{
	// Get a pointer on the shape
	CMeshMRMSkinned *pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);

	// Recompute the id, and skin spheres, if needed
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get it.
	return &pMesh->getMeshGeom().getSkinBoneSphere();
}


// ***************************************************************************
bool			CMeshMRMSkinnedInstance::getSkinBoneBBox(NLMISC::CAABBox &bbox, uint boneId)
{
	// Get a pointer on the shape
	CMeshMRMSkinned *pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);

	// Recompute the id, and skin spheres, if needed
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get it.
	return pMesh->getMeshGeom().getSkinBoneBBox(_FatherSkeletonModel, bbox, boneId);
}


// ***************************************************************************
bool	CMeshMRMSkinnedInstance::isSkinnable() const
{
	// true if the mesh is skinned
	return true;
}


// ***************************************************************************
void	CMeshMRMSkinnedInstance::renderSkin(float alphaMRM)
{
	// Don't setup lighting or matrix in Skin. Done by the skeleton

	if(Shape && getVisibility() != CHrcTrav::Hide)
	{
		// Get a pointer on the shape
		CMeshMRMSkinned *pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);

		// render the meshGeom
		CMeshMRMSkinnedGeom	&meshGeom= const_cast<CMeshMRMSkinnedGeom&>(pMesh->getMeshGeom ());
		meshGeom.renderSkin( this, alphaMRM );
	}
}


// ***************************************************************************
const	CMRMLevelDetail		*CMeshMRMSkinnedInstance::getMRMLevelDetail() const
{
	if(Shape)
	{
		CMeshMRMSkinned	*meshMrm= safe_cast<CMeshMRMSkinned*>((IShape*)Shape);
		return &meshMrm->getMeshGeom().getLevelDetail();
	}
	else
		return NULL;
}

// ***************************************************************************
bool			CMeshMRMSkinnedInstance::supportSkinGrouping() const
{
	if(Shape)
	{
		CMeshMRMSkinned	*meshMrm= safe_cast<CMeshMRMSkinned*>((IShape*)Shape);
		return	meshMrm->getMeshGeom().supportSkinGrouping();
	}
	else
		return false;
}
// ***************************************************************************
sint			CMeshMRMSkinnedInstance::renderSkinGroupGeom(float alphaMRM, uint remainingVertices, uint8 *dest)
{
	// Get a pointer on the shape
	CMeshMRMSkinned		*pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMSkinnedGeom	&meshGeom= const_cast<CMeshMRMSkinnedGeom&>(pMesh->getMeshGeom ());
	return meshGeom.renderSkinGroupGeom(this, alphaMRM, remainingVertices, dest);
}
// ***************************************************************************
void			CMeshMRMSkinnedInstance::renderSkinGroupPrimitives(uint baseVertex, std::vector<CSkinSpecularRdrPass> &specularRdrPasses, uint skinIndex)
{
	// Get a pointer on the shape
	CMeshMRMSkinned		*pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMSkinnedGeom	&meshGeom= const_cast<CMeshMRMSkinnedGeom&>(pMesh->getMeshGeom ());
	meshGeom.renderSkinGroupPrimitives(this, baseVertex, specularRdrPasses, skinIndex);
}
// ***************************************************************************
void			CMeshMRMSkinnedInstance::renderSkinGroupSpecularRdrPass(uint rdrPassId)
{
	// Get a pointer on the shape
	CMeshMRMSkinned		*pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMSkinnedGeom	&meshGeom= const_cast<CMeshMRMSkinnedGeom&>(pMesh->getMeshGeom ());
	meshGeom.renderSkinGroupSpecularRdrPass(this, rdrPassId);
}

// ***************************************************************************
void	CMeshMRMSkinnedInstance::initRenderFilterType()
{
	if(Shape)
	{
		// If the Shape has a VP or not...
		CMeshMRMSkinned *pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);

		if( pMesh->getMeshGeom().hasMeshVertexProgram() )
			_RenderFilterType= UScene::FilterMeshMRMVP;
		else
			_RenderFilterType= UScene::FilterMeshMRMNoVP;
	}
}


// ***************************************************************************
bool			CMeshMRMSkinnedInstance::supportShadowSkinGrouping() const
{
	if(Shape)
	{
		CMeshMRMSkinned	*meshMrm= safe_cast<CMeshMRMSkinned*>((IShape*)Shape);
		return	meshMrm->getMeshGeom().supportShadowSkinGrouping();
	}
	else
		return false;
}

// ***************************************************************************
sint			CMeshMRMSkinnedInstance::renderShadowSkinGeom(uint remainingVertices, uint8 *vbDest)
{
	// Get a pointer on the shape
	CMeshMRMSkinned		*pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMSkinnedGeom	&meshGeom= const_cast<CMeshMRMSkinnedGeom&>(pMesh->getMeshGeom ());
	return meshGeom.renderShadowSkinGeom(this, remainingVertices, vbDest);
}

// ***************************************************************************
void			CMeshMRMSkinnedInstance::renderShadowSkinPrimitives(CMaterial &castMat, IDriver *drv, uint baseVertex)
{
	// Get a pointer on the shape
	CMeshMRMSkinned		*pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMSkinnedGeom	&meshGeom= const_cast<CMeshMRMSkinnedGeom&>(pMesh->getMeshGeom ());
	meshGeom.renderShadowSkinPrimitives(this, castMat, drv, baseVertex);
}

// ***************************************************************************
bool			CMeshMRMSkinnedInstance::supportIntersectSkin() const
{
	CMeshMRMSkinned		*pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);
	CMeshMRMSkinnedGeom	&meshGeom= const_cast<CMeshMRMSkinnedGeom&>(pMesh->getMeshGeom ());
	return meshGeom.supportIntersectSkin();
}

// ***************************************************************************
bool			CMeshMRMSkinnedInstance::intersectSkin(const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D)
{
	// Get a pointer on the shape
	CMeshMRMSkinned		*pMesh = NLMISC::safe_cast<CMeshMRMSkinned *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMSkinnedGeom	&meshGeom= const_cast<CMeshMRMSkinnedGeom&>(pMesh->getMeshGeom ());
	return meshGeom.intersectSkin(this, toRaySpace, dist2D, distZ, computeDist2D);
}


} // NL3D
