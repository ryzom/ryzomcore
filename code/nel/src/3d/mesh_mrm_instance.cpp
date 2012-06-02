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

#include "nel/3d/mesh_mrm_instance.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/raw_skin.h"
#include "nel/3d/shifted_triangle_cache.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/scene.h"


using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
CMeshMRMInstance::~CMeshMRMInstance()
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
void		CMeshMRMInstance::registerBasic()
{
	CScene::registerModel(MeshMRMInstanceId, MeshBaseInstanceId, CMeshMRMInstance::creator);
}


// ***************************************************************************
void		CMeshMRMInstance::clearRawSkinCache()
{
	delete	_RawSkinCache;
	_RawSkinCache= NULL;
}

// ***************************************************************************
void		CMeshMRMInstance::clearShiftedTriangleCache()
{
	delete	_ShiftedTriangleCache;
	_ShiftedTriangleCache= NULL;
}

// ***************************************************************************
void		CMeshMRMInstance::setApplySkin(bool state)
{
	// Call parents method
	CMeshBaseInstance::setApplySkin (state);

	// Get a pointer on the shape
	CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);

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
void		CMeshMRMInstance::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	if(Shape)
	{
		// Get a pointer on the shape.
		CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);
		// Affect the mesh directly.
		pMesh->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
	}
}


// ***************************************************************************
const std::vector<sint32>			*CMeshMRMInstance::getSkinBoneUsage() const
{
	// Get a pointer on the shape
	CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);

	// Recompute the id, if needed
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get ids.
	return &pMesh->getMeshGeom().getSkinBoneUsage();
}


// ***************************************************************************
const std::vector<NLMISC::CBSphere>	*CMeshMRMInstance::getSkinBoneSphere() const
{
	// Get a pointer on the shape
	CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);

	// Recompute the id, and skin spheres, if needed
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get it.
	return &pMesh->getMeshGeom().getSkinBoneSphere();
}


// ***************************************************************************
bool			CMeshMRMInstance::getSkinBoneBBox(NLMISC::CAABBox &bbox, uint boneId)
{
	// Get a pointer on the shape
	CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);

	// Recompute the id, and skin spheres, if needed
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get it.
	return pMesh->getMeshGeom().getSkinBoneBBox(_FatherSkeletonModel, bbox, boneId);
}


// ***************************************************************************
bool	CMeshMRMInstance::isSkinnable() const
{
	if(Shape==NULL)
		return false;

	// Get a pointer on the shape
	CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);

	// true if the mesh is skinned
	return pMesh->getMeshGeom().isSkinned();
}


// ***************************************************************************
void	CMeshMRMInstance::renderSkin(float alphaMRM)
{
	// Don't setup lighting or matrix in Skin. Done by the skeleton

	if(Shape && getVisibility() != CHrcTrav::Hide)
	{
		// Get a pointer on the shape
		CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);

		// render the meshGeom
		CMeshMRMGeom	&meshGeom= const_cast<CMeshMRMGeom&>(pMesh->getMeshGeom ());
		meshGeom.renderSkin( this, alphaMRM );
	}
}


// ***************************************************************************
const	CMRMLevelDetail		*CMeshMRMInstance::getMRMLevelDetail() const
{
	if(Shape)
	{
		CMeshMRM	*meshMrm= safe_cast<CMeshMRM*>((IShape*)Shape);
		return &meshMrm->getMeshGeom().getLevelDetail();
	}
	else
		return NULL;
}

// ***************************************************************************
bool			CMeshMRMInstance::supportSkinGrouping() const
{
	if(Shape)
	{
		CMeshMRM	*meshMrm= safe_cast<CMeshMRM*>((IShape*)Shape);
		return	meshMrm->getMeshGeom().supportSkinGrouping();
	}
	else
		return false;
}
// ***************************************************************************
sint			CMeshMRMInstance::renderSkinGroupGeom(float alphaMRM, uint remainingVertices, uint8 *dest)
{
	// Get a pointer on the shape
	CMeshMRM		*pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMGeom	&meshGeom= const_cast<CMeshMRMGeom&>(pMesh->getMeshGeom ());
	return meshGeom.renderSkinGroupGeom(this, alphaMRM, remainingVertices, dest);
}
// ***************************************************************************
void			CMeshMRMInstance::renderSkinGroupPrimitives(uint baseVertex, std::vector<CSkinSpecularRdrPass> &specularRdrPasses, uint skinIndex)
{
	// Get a pointer on the shape
	CMeshMRM		*pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMGeom	&meshGeom= const_cast<CMeshMRMGeom&>(pMesh->getMeshGeom ());
	meshGeom.renderSkinGroupPrimitives(this, baseVertex, specularRdrPasses, skinIndex);
}
// ***************************************************************************
void			CMeshMRMInstance::renderSkinGroupSpecularRdrPass(uint rdrPassId)
{
	// Get a pointer on the shape
	CMeshMRM		*pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMGeom	&meshGeom= const_cast<CMeshMRMGeom&>(pMesh->getMeshGeom ());
	meshGeom.renderSkinGroupSpecularRdrPass(this, rdrPassId);
}

// ***************************************************************************
void	CMeshMRMInstance::initRenderFilterType()
{
	if(Shape)
	{
		// If the Shape has a VP or not...
		CMeshMRM *pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);

		if( pMesh->getMeshGeom().hasMeshVertexProgram() )
			_RenderFilterType= UScene::FilterMeshMRMVP;
		else
			_RenderFilterType= UScene::FilterMeshMRMNoVP;
	}
}


// ***************************************************************************
bool			CMeshMRMInstance::supportShadowSkinGrouping() const
{
	if(Shape)
	{
		CMeshMRM	*meshMrm= safe_cast<CMeshMRM*>((IShape*)Shape);
		return	meshMrm->getMeshGeom().supportShadowSkinGrouping();
	}
	else
		return false;
}

// ***************************************************************************
sint			CMeshMRMInstance::renderShadowSkinGeom(uint remainingVertices, uint8 *vbDest)
{
	// Get a pointer on the shape
	CMeshMRM		*pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMGeom	&meshGeom= const_cast<CMeshMRMGeom&>(pMesh->getMeshGeom ());
	return meshGeom.renderShadowSkinGeom(this, remainingVertices, vbDest);
}

// ***************************************************************************
void			CMeshMRMInstance::renderShadowSkinPrimitives(CMaterial &castMat, IDriver *drv, uint baseVertex)
{
	// Get a pointer on the shape
	CMeshMRM		*pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMGeom	&meshGeom= const_cast<CMeshMRMGeom&>(pMesh->getMeshGeom ());
	meshGeom.renderShadowSkinPrimitives(this, castMat, drv, baseVertex);
}

// ***************************************************************************
bool			CMeshMRMInstance::supportIntersectSkin() const
{
	CMeshMRM		*pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);
	CMeshMRMGeom	&meshGeom= const_cast<CMeshMRMGeom&>(pMesh->getMeshGeom ());
	return meshGeom.supportIntersectSkin();
}

// ***************************************************************************
bool			CMeshMRMInstance::intersectSkin(const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D)
{
	// Get a pointer on the shape
	CMeshMRM		*pMesh = NLMISC::safe_cast<CMeshMRM *>((IShape*)Shape);
	// render the meshGeom
	CMeshMRMGeom	&meshGeom= const_cast<CMeshMRMGeom&>(pMesh->getMeshGeom ());
	return meshGeom.intersectSkin(this, toRaySpace, dist2D, distZ, computeDist2D);
}


} // NL3D
