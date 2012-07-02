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

#include "nel/3d/mesh_instance.h"
#include "nel/3d/mesh.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/scene.h"
#include <list>

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
CMeshInstance::CMeshInstance()
{
	_ShadowMap= NULL;

	// LoadBalancing is not useful for Mesh, because meshs cannot be reduced in faces.
	// Override CTransformShape state.
	CTransform::setIsLoadbalancable(false);

	// Mesh support shadow map casting only
	CTransform::setIsShadowMapCaster(true);
}

// ***************************************************************************
CMeshInstance::~CMeshInstance()
{
	// Auto detach me from skeleton. Must do it here, not in ~CTransform().
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// hrc and clip hierarchy is modified.
		_FatherSkeletonModel->detachSkeletonSon(this);
		nlassert(_FatherSkeletonModel==NULL);
	}

	// delete the shadowMap
	deleteShadowMap();
}


// ***************************************************************************
void		CMeshInstance::registerBasic()
{
	CScene::registerModel(MeshInstanceId, MeshBaseInstanceId, CMeshInstance::creator);
}

// ***************************************************************************
void		CMeshInstance::setApplySkin(bool state)
{
	// Call parents method
	CMeshBaseInstance::setApplySkin (state);

	// Get a pointer on the shape
	CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// Recompute the id
	if (state)
	{
		pMesh->computeBonesId (_FatherSkeletonModel);
	}

	// update the skeleton usage according to the mesh.
	pMesh->updateSkeletonUsage(_FatherSkeletonModel, state);
}


// ***************************************************************************
const std::vector<sint32>	*CMeshInstance::getSkinBoneUsage() const
{
	// Get a pointer on the shape
	CMesh	*pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// Recompute the id
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get ids.
	return &pMesh->getMeshGeom().getSkinBoneUsage();
}


// ***************************************************************************
const std::vector<NLMISC::CBSphere>	*CMeshInstance::getSkinBoneSphere() const
{
	// Get a pointer on the shape
	CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// Recompute the id, and skin spheres, if needed
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get it.
	return &pMesh->getMeshGeom().getSkinBoneSphere();
}


// ***************************************************************************
bool	CMeshInstance::isSkinnable() const
{
	if(Shape==NULL)
		return false;

	// Get a pointer on the shape
	CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// true if the mesh is skinned
	return pMesh->getMeshGeom().isSkinned();
}


// ***************************************************************************
void	CMeshInstance::renderSkin(float alphaMRM)
{
	// Don't setup lighting or matrix in Skin. Done by the skeleton

	if(Shape && getVisibility() != CHrcTrav::Hide)
	{
		// Get a pointer on the shape
		CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

		// render the meshGeom
		CMeshGeom	&meshGeom= const_cast<CMeshGeom&>(pMesh->getMeshGeom ());
		meshGeom.renderSkin( this, alphaMRM );
	}
}


// ***************************************************************************
void	CMeshInstance::initRenderFilterType()
{
	if(Shape)
	{
		// If the Shape has a VP or not...
		CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

		if( pMesh->getMeshGeom().hasMeshVertexProgram() )
			_RenderFilterType= UScene::FilterMeshVP;
		else
			_RenderFilterType= UScene::FilterMeshNoVP;
	}
}

// ***************************************************************************
bool	CMeshInstance::supportIntersectSkin() const
{
	CMesh		*pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);
	CMeshGeom	&meshGeom= const_cast<CMeshGeom&>(pMesh->getMeshGeom ());
	return meshGeom.supportIntersectSkin();
}

// ***************************************************************************
bool	CMeshInstance::intersectSkin(const CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D)
{
	// Get a pointer on the shape
	CMesh		*pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);
	// render the meshGeom
	CMeshGeom	&meshGeom= const_cast<CMeshGeom&>(pMesh->getMeshGeom ());
	return meshGeom.intersectSkin(this, toRaySpace, dist2D, distZ, computeDist2D);
}

// ***************************************************************************
// ***************************************************************************
// ShadowMapping
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CMeshInstance::generateShadowMap(const CVector &lightDir)
{
	// get the driver for Texture Render
	CScene			*scene= getOwnerScene();
	CRenderTrav		&renderTrav= scene->getRenderTrav();
	IDriver			*driver= renderTrav.getAuxDriver();

	if(!Shape)
		return;

	// update ShadowMap data if needed.
	// ****
	updateShadowMap(driver);

	if(!_ShadowMap)
		return;

	// compute the ProjectionMatrix.
	// ****

	// get the BBox and localPosMatrix
	CAABBox		bbShape;
	Shape->getAABBox(bbShape);
	CMatrix		localPosMatrix= getWorldMatrix();
	localPosMatrix.setPos(CVector::Null);

	// setup cameraMatrix with BBox and Enlarge For 1 pixel
	CMatrix		cameraMatrix;
	_ShadowMap->buildCasterCameraMatrix(lightDir, localPosMatrix, bbShape, cameraMatrix);


	// Render.
	// ****
	// setup the orhtogonal frustum and viewMatrix to include all the object.
	driver->setFrustum(0,1,0,1,0,1,false);
	driver->setupViewMatrix(cameraMatrix.inverted());
	driver->setupModelMatrix(localPosMatrix);

	// render the Mesh
	CMaterial	&castMat= renderTrav.getShadowMapManager().getCasterShadowMaterial();
	CMesh		*mesh= (CMesh*)(IShape*)Shape;
	driver->activeVertexBuffer( const_cast<CVertexBuffer&>(mesh->getVertexBuffer()) );
	for(uint mb=0;mb<mesh->getNbMatrixBlock();mb++)
	{
		for(uint rp=0;rp<mesh->getNbRdrPass(mb);rp++)
		{
			const CIndexBuffer	&pb= mesh->getRdrPassPrimitiveBlock(mb, rp);
			driver->activeIndexBuffer(const_cast<CIndexBuffer&>(pb));
			driver->renderTriangles(castMat, 0, pb.getNumIndexes()/3);
		}
	}

	// Infos.
	// ****

	// Compute the BackPoint: the first point to be shadowed.
	CVector		backPoint= bbShape.getCenter();
	// get the 3/4 bottom of the shape
	backPoint.z-= bbShape.getHalfSize().z/2;
	backPoint= localPosMatrix * backPoint;
	// Use the 3/4 bottom of the BBox minus the light direction in XY. NB: little hack:
	// suppose no Rotate (but Z) and no scale
	CVector	ldir= lightDir;
	ldir.z= 0;
	ldir.normalize();
	float	lenXY= (CVector(bbShape.getHalfSize().x, bbShape.getHalfSize().y, 0)).norm();
	backPoint-= ldir*lenXY;

	// Compute LocalProjectionMatrix and other infos from cameraMatrix and backPoint?
	_ShadowMap->buildProjectionInfos(cameraMatrix, backPoint, getShadowMapMaxDepth());
}

// ***************************************************************************
CShadowMap	*CMeshInstance::getShadowMap()
{
	return _ShadowMap;
}

// ***************************************************************************
void		CMeshInstance::createShadowMap()
{
	// create the shadowMap
	if(!_ShadowMap)
	{
		_ShadowMap= new CShadowMap(&getOwnerScene()->getRenderTrav().getShadowMapManager());
		getOwnerScene()->registerShadowCasterToList(this);
	}
}

// ***************************************************************************
void		CMeshInstance::deleteShadowMap()
{
	if(_ShadowMap)
	{
		delete _ShadowMap;
		_ShadowMap= NULL;
		getOwnerScene()->unregisterShadowCasterToList(this);
	}
}

// ***************************************************************************
void		CMeshInstance::updateShadowMap(IDriver * /* driver */)
{
	nlassert(_ShadowMap);

	// create/update texture
	if(_ShadowMap->getTextureSize()!=getOwnerScene()->getShadowMapTextureSize())
	{
		_ShadowMap->initTexture(getOwnerScene()->getShadowMapTextureSize());
	}
}


// ***************************************************************************
bool	CMeshInstance::computeWorldBBoxForShadow(NLMISC::CAABBox &worldBB)
{
	// If even not visible or empty, no-op
	if(!isHrcVisible() || !Shape)
		return false;

	// get the shape bbox
	Shape->getAABBox(worldBB);
	// transform into world
	worldBB= CAABBox::transformAABBox(getWorldMatrix(), worldBB);

	return true;
}

// ***************************************************************************
void	CMeshInstance::renderIntoSkeletonShadowMap(CSkeletonModel *rootSkeleton, CMaterial	&castMat)
{
	/*
		Yoyo: Not Very Robuts here:
			- suppose the MeshInstance don't have sons.
			- suppose that the VertexBuffer doesn't have VertexColor (else castMat.color unused).
	*/

	// If even not visible or empty, no-op
	if(!isHrcVisible() || !Shape)
		return;

	// render into aux Driver
	IDriver			*driver= getOwnerScene()->getRenderTrav().getAuxDriver();

	// **** Render the Skeleton Skins
	// The model Matrix is special here. It must be the Skeleton World Matrix, minus The Root Skeleton pos.
	CMatrix		localPosMatrix;
	localPosMatrix.setRot( getWorldMatrix() );
	// NB: if this==rootSkeleton, then the final pos will be CVector::Null
	localPosMatrix.setPos( getWorldMatrix().getPos() - rootSkeleton->getWorldMatrix().getPos() );
	driver->setupModelMatrix(localPosMatrix);

	// render the Mesh
	CMesh	*mesh= (CMesh*)(IShape*)Shape;
	driver->activeVertexBuffer( const_cast<CVertexBuffer&>(mesh->getVertexBuffer()) );
	for(uint mb=0;mb<mesh->getNbMatrixBlock();mb++)
	{
		for(uint rp=0;rp<mesh->getNbRdrPass(mb);rp++)
		{
			const CIndexBuffer	&pb= mesh->getRdrPassPrimitiveBlock(mb, rp);
			driver->activeIndexBuffer(const_cast<CIndexBuffer&>(pb));
			driver->renderTriangles(castMat, 0, pb.getNumIndexes()/3);
		}
	}

}


} // NL3D
