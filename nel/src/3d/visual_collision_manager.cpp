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

#include "nel/3d/visual_collision_manager.h"
#include "nel/3d/visual_collision_entity.h"
#include "nel/3d/landscape.h"
#include "nel/3d/camera_col.h"
#include "nel/3d/shadow_map.h"
#include "nel/3d/light.h"
#include "nel/misc/common.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// Those blocks size are computed to be approximatively one block for 10 entities.
const	uint	TileDescNodeAllocatorBlockSize= 40000;
const	uint	PatchQuadBlockAllocatorBlockSize= 160;
// For Mesh QuadGrid
const	uint	MeshColQuadGridSize= 64;
const	float	MeshColQuadGridEltSize= 20;


// ***************************************************************************
CVisualCollisionManager::CVisualCollisionManager() :
	_TileDescNodeAllocator(TileDescNodeAllocatorBlockSize),
	_PatchQuadBlockAllocator(PatchQuadBlockAllocatorBlockSize)
{
	_Landscape= NULL;

	// Default.
	setSunContributionPower(0.5f, 0.5f);

	// init the mesh quadGrid
	_MeshQuadGrid.create(MeshColQuadGridSize, MeshColQuadGridEltSize);
	// valid id start at 1
	_MeshIdPool= 1;

	_PlayerInside= false;
	_ShadowIndexBuffer.setFormat(NL_DEFAULT_INDEX_BUFFER_FORMAT);
	NL_SET_IB_NAME(_ShadowIndexBuffer, "CVisualCollisionManager");
}
// ***************************************************************************
CVisualCollisionManager::~CVisualCollisionManager()
{
	_Landscape= NULL;
}


// ***************************************************************************
void					CVisualCollisionManager::setLandscape(CLandscape *landscape)
{
	_Landscape= landscape;
}


// ***************************************************************************
CVisualCollisionEntity		*CVisualCollisionManager::createEntity()
{
	return new CVisualCollisionEntity(this);
}


// ***************************************************************************
void						CVisualCollisionManager::deleteEntity(CVisualCollisionEntity	*entity)
{
	delete entity;
}


// ***************************************************************************
CVisualTileDescNode		*CVisualCollisionManager::newVisualTileDescNode()
{
	return _TileDescNodeAllocator.allocate();
}

// ***************************************************************************
void					CVisualCollisionManager::deleteVisualTileDescNode(CVisualTileDescNode *ptr)
{
	_TileDescNodeAllocator.freeBlock(ptr);
}

// ***************************************************************************
CPatchQuadBlock			*CVisualCollisionManager::newPatchQuadBlock()
{
	return _PatchQuadBlockAllocator.allocate();
}

// ***************************************************************************
void					CVisualCollisionManager::deletePatchQuadBlock(CPatchQuadBlock *ptr)
{
	_PatchQuadBlockAllocator.freeBlock(ptr);
}


// ***************************************************************************
void					CVisualCollisionManager::setSunContributionPower(float power, float maxThreshold)
{
	NLMISC::clamp(power, 0.f, 1.f);

	for(uint i=0; i<256; i++)
	{
		float	f= i/255.f;
		f = powf(f/maxThreshold, power);
		sint	uf= (sint)floor(255*f);
		NLMISC::clamp(uf, 0, 255);
		_SunContributionLUT[i]= uf;
	}

}

// ***************************************************************************
// ***************************************************************************
// Camera collision
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void					CVisualCollisionManager::setPlayerInside(bool state)
{
	_PlayerInside= state;
}


// ***************************************************************************
float					CVisualCollisionManager::getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone)
{
	float	minCol= 1;

	// try col with landscape
	if(_Landscape)
	{
		minCol= _Landscape->getCameraCollision(start, end, radius, cone);
	}

	// try col with meshes
	CCameraCol		camCol;
	camCol.build(start, end, radius, cone);
	_MeshQuadGrid.select(camCol.getBBox().getMin(), camCol.getBBox().getMax());
	// try to intersect with any instance meshs
	CQuadGrid<CMeshInstanceCol*>::CIterator		it;
	for(it= _MeshQuadGrid.begin();it!=_MeshQuadGrid.end();it++)
	{
		// Skip this mesh according to special flag (IBBR problem)
		if((*it)->AvoidCollisionWhenPlayerInside && _PlayerInside)
			continue;
		if((*it)->AvoidCollisionWhenPlayerOutside && !_PlayerInside)
			continue;

		// collide
		float	meshCol= (*it)->getCameraCollision(camCol);
		// Keep only yhe smallest value
		minCol= min(minCol, meshCol);
	}

	return minCol;
}


// ***************************************************************************
bool					CVisualCollisionManager::getRayCollision(const NLMISC::CVector &start, const NLMISC::CVector &end, bool landscapeOnly)
{
	// try col with landscape
	if(_Landscape)
	{
		if( _Landscape->getRayCollision(start, end) < 1.f)
			return true;
	}

	// try col with meshes, if wanted
	if(!landscapeOnly)
	{
		CCameraCol		camCol;
		camCol.buildRay(start, end);
		_MeshQuadGrid.select(camCol.getBBox().getMin(), camCol.getBBox().getMax());
		// try to intersect with any instance meshs
		CQuadGrid<CMeshInstanceCol*>::CIterator		it;
		for(it= _MeshQuadGrid.begin();it!=_MeshQuadGrid.end();it++)
		{
			// Skip this mesh according to special flag (IBBR problem)
			if((*it)->AvoidCollisionWhenPlayerInside && _PlayerInside)
				continue;
			if((*it)->AvoidCollisionWhenPlayerOutside && !_PlayerInside)
				continue;

			// collide
			float	meshCol= (*it)->getCameraCollision(camCol);
			if(meshCol<1.f)
				return true;
		}
	}

	return false;
}


// ***************************************************************************
float		CVisualCollisionManager::CMeshInstanceCol::getCameraCollision(CCameraCol &camCol)
{
	// if mesh still present (else it s may be an error....)
	if(Mesh)
	{
		// first test if intersect with the bboxes
		if(!camCol.getBBox().intersect(WorldBBox))
			return 1;

		// get the collision with the mesh
		return Mesh->getCameraCollision(WorldMatrix, camCol);
	}
	else
		// no collision
		return 1;
}


// ***************************************************************************
uint					CVisualCollisionManager::addMeshInstanceCollision(CVisualCollisionMesh *mesh, const CMatrix &instanceMatrix, bool avoidCollisionWhenInside, bool avoidCollisionWhenOutside)
{
	if(!mesh)
		return 0;

	// allocate a new id
	uint32	id= _MeshIdPool++;

	// insert in map
	CMeshInstanceCol	&meshInst= _Meshs[id];

	// Build the col mesh instance
	meshInst.Mesh= mesh;
	meshInst.WorldMatrix= instanceMatrix;
	meshInst.WorldBBox= mesh->computeWorldBBox(instanceMatrix);
	meshInst.AvoidCollisionWhenPlayerInside= avoidCollisionWhenInside;
	meshInst.AvoidCollisionWhenPlayerOutside= avoidCollisionWhenOutside;
	meshInst.ID = id;

	// insert in quadGrid
	meshInst.QuadGridIt= _MeshQuadGrid.insert(meshInst.WorldBBox.getMin(), meshInst.WorldBBox.getMax(), &meshInst);

	return id;
}

// ***************************************************************************
void					CVisualCollisionManager::removeMeshCollision(uint id)
{
	// find in map
	TMeshColMap::iterator	it= _Meshs.find(id);
	if(it!=_Meshs.end())
	{
		// remove from the quadgrid
		_MeshQuadGrid.erase(it->second.QuadGridIt);
		// remove from the map
		_Meshs.erase(it);
	}
}


// ***************************************************************************
void					CVisualCollisionManager::receiveShadowMap(IDriver *drv, CShadowMap *shadowMap, const CVector &casterPos, CMaterial &shadowMat, CShadowMapProjector &smp)
{
	// Build a shadow context
	CVisualCollisionMesh::CShadowContext	shadowContext(shadowMat, _ShadowIndexBuffer, smp);
	shadowContext.Driver= drv;
	shadowContext.ShadowMap= shadowMap;
	shadowContext.CasterPos= casterPos;
	shadowContext.ShadowWorldBB= shadowMap->LocalBoundingBox;
	shadowContext.ShadowWorldBB.setCenter(shadowContext.ShadowWorldBB.getCenter() + casterPos);

	// bkup shadowColor
	CRGBA	shadowColor= shadowContext.ShadowMaterial.getColor();

	// try to intersect with any instance meshs in quadgrid
	_MeshQuadGrid.select(shadowContext.ShadowWorldBB.getMin(), shadowContext.ShadowWorldBB.getMax());
	CQuadGrid<CMeshInstanceCol*>::CIterator		it;
	bool		lightSetupFaked= false;
	for(it= _MeshQuadGrid.begin();it!=_MeshQuadGrid.end();it++)
	{
		/* NB: this is a collision flag, but the problem is exactly the same for shadows:
			If the player is outside, then an "outside entity" can cast shadow only on "outside mesh"
		*/
		// Skip this mesh according to special flag (IBBR problem)
		if((*it)->AvoidCollisionWhenPlayerInside && _PlayerInside)
			continue;
		if((*it)->AvoidCollisionWhenPlayerOutside && !_PlayerInside)
			continue;


		/* Avoid BackFace Shadowing on CVisualCollisionMesh. To do this simply and smoothly, use a trick:
			Use a FakeLight: a directional light in reverse direction of the shadow direction
			The material is now a lighted material, with
				Emissive= ShadowColor so we get correct dark color, for frontfaces agst shadow direction
				(which are actually backfaces agst FakeLight)
				Diffuse= White so shadow is off, for pure backFace agst shadow direction
				(which are actually frontFaces agst FakeLight)
			NB: CShadowMapManager::renderProject() has called CRenderTrav::resetLighting() and enableLight(0, true) so we get clean setup here
		*/
		// need to do it only one time per shadowMap reception
		if(!lightSetupFaked)
		{
			lightSetupFaked= true;
			// Build the light direction as the opposite to shadow direction (in LocalProjectionMatrix.getJ())
			CLight	fakeLight;
			fakeLight.setupDirectional(CRGBA::Black, CRGBA::White, CRGBA::Black, -shadowMap->LocalProjectionMatrix.getJ());
			drv->setLight(0, fakeLight);
			// setup the material
			shadowContext.ShadowMaterial.setLighting(true, shadowColor, CRGBA::Black, CRGBA::White, CRGBA::Black);
		}

		// cast shadow
		(*it)->receiveShadowMap(shadowContext);
	}

	// if the material has been faked, reset
	if(lightSetupFaked)
	{
		shadowContext.ShadowMaterial.setLighting(false);
		shadowContext.ShadowMaterial.setColor(shadowColor);
	}
}


// ***************************************************************************
void		CVisualCollisionManager::CMeshInstanceCol::receiveShadowMap(const CVisualCollisionMesh::CShadowContext &shadowContext)
{
	// if mesh still present (else it s may be an error....)
	if(Mesh)
	{
		// first test if intersect with the bboxes
		if(!shadowContext.ShadowWorldBB.intersect(WorldBBox))
			return;

		// get the collision with the mesh
		Mesh->receiveShadowMap(WorldMatrix, shadowContext);
	}
}

// ***************************************************************************
void CVisualCollisionManager::getMeshs(const NLMISC::CAABBox &aabbox, std::vector<CMeshInstanceColInfo> &dest)
{
	_MeshQuadGrid.select(aabbox.getMin(), aabbox.getMax());
	dest.clear();
	CQuadGrid<CMeshInstanceCol*>::CIterator it = _MeshQuadGrid.begin();
	CQuadGrid<CMeshInstanceCol*>::CIterator endIt = _MeshQuadGrid.end();
	while (it != endIt)
	{
		if ((*it)->WorldBBox.intersect(aabbox))
		{
			CMeshInstanceColInfo infos;
			infos.Mesh = (*it)->Mesh;
			infos.ID = (*it)->ID;
			infos.WorldBBox = &((*it)->WorldBBox);
			infos.WorldMatrix = &((*it)->WorldMatrix);
			dest.push_back(infos);
		}
		++ it;
	}
}



} // NL3D

