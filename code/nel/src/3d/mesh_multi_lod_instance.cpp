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

#include "nel/3d/mesh_multi_lod_instance.h"
#include "nel/3d/mesh_multi_lod.h"
#include "nel/3d/coarse_mesh_manager.h"
#include "nel/3d/scene.h"

#include "nel/misc/debug.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************

	CMeshMultiLodInstance::CMeshMultiLodInstance ()
{
	// No flags
	Flags=0;
	_CoarseMeshDistance = -1.f;
	_LastCoarseMesh= NULL;
	_LastCoarseMeshNumVertices= 0;
}

// ***************************************************************************

CMeshMultiLodInstance::~CMeshMultiLodInstance ()
{
}


// ***************************************************************************

void		CMeshMultiLodInstance::registerBasic()
{
	CScene::registerModel (MeshMultiLodInstanceId, MeshBaseInstanceId, CMeshMultiLodInstance::creator);
}


// ***************************************************************************
CRGBA		CMeshMultiLodInstance::getCoarseMeshLighting()
{
	CScene	*scene= getOwnerScene();
	nlassert(scene);

	// compute his sun contribution result, and update
	CRGBA	sunContrib= scene->getSunDiffuse();
	// simulate/average diffuse lighting over the mesh by dividing diffuse by 2.
	sunContrib.modulateFromuiRGBOnly(sunContrib, getLightContribution().SunContribution/2 );
	// Add Ambient
	sunContrib.addRGBOnly(sunContrib, scene->getSunAmbient());
	sunContrib.A= 255;

	return sunContrib;
}


// ***************************************************************************

void		CMeshMultiLodInstance::traverseLoadBalancing()
{
	// Call previous
	CMeshBaseInstance::traverseLoadBalancing ();

	// If this is the second pass of LoadBalancing, choose the Lods, according to getNumTrianglesAfterLoadBalancing()
	CLoadBalancingTrav		&loadTrav= getOwnerScene()->getLoadBalancingTrav();
	if(loadTrav.getLoadPass()==1)
	{
		// Get a pointer on the shape
		CMeshMultiLod *shape=safe_cast<CMeshMultiLod*> ((IShape*)Shape);

		// Reset render pass
		if	(!getBypassLODOpacityFlag())
		{
			setTransparency(false);
			setOpacity(false);
		}

		// Get the wanted number of polygons
		float polygonCount= getNumTrianglesAfterLoadBalancing ();

		// Look for the good slot
		uint meshCount=(uint)shape->_MeshVector.size();
		Lod0=0;
		if (meshCount>1)
		{
			// Look for good i
			while ( polygonCount < shape->_MeshVector[Lod0].EndPolygonCount )
			{
				Lod0++;
				if (Lod0==meshCount-1)
					break;
			}
		}

		// The slot
		CMeshMultiLod::CMeshSlot	&slot=shape->_MeshVector[Lod0];

		// Get the distance with polygon count
		float distance=(polygonCount-slot.B)/slot.A;

		// Get the final polygon count
		if (slot.MeshGeom)
			PolygonCountLod0=slot.MeshGeom->getNumTriangles (distance);

		// Second slot in use ?
		Lod1=0xffffffff;

		// The next slot
		CMeshMultiLod::CMeshSlot	*nextSlot=NULL;

		// Next slot exist ?
		if (Lod0!=meshCount-1)
		{
			nextSlot=&(shape->_MeshVector[Lod0+1]);
		}

		// Max dist before blend
		float startBlend;
		if (nextSlot)
			startBlend=slot.DistMax-nextSlot->BlendLength;
		else
			startBlend=slot.DistMax-slot.BlendLength;

		// In blend zone ?
		if ( startBlend < distance )
		{
			// Alpha factor for main Lod
			BlendFactor = (slot.DistMax-distance)/(slot.DistMax-startBlend);
			if (BlendFactor<0)
				BlendFactor=0;
			nlassert (BlendFactor<=1);

			// Render this mesh
			if (slot.MeshGeom)
			{
				if (slot.Flags&CMeshMultiLod::CMeshSlot::BlendOut)
				{
					// Render the geom mesh with alpha blending with goodPolyCount
					if (!getBypassLODOpacityFlag()) setTransparency(true);
					Flags|=CMeshMultiLodInstance::Lod0Blend;
				}
				else
				{
					// Render the geom mesh without alpha blending with goodPolyCount
					if (!getBypassLODOpacityFlag()) setTransparency (slot.isTransparent());
					setOpacity (slot.isOpaque());
					Flags&=~CMeshMultiLodInstance::Lod0Blend;
				}
			}
			else
				Lod0=0xffffffff;

			// Next mesh, BlendIn actived ?
			if (nextSlot && shape->_MeshVector[Lod0+1].MeshGeom && (nextSlot->Flags&CMeshMultiLod::CMeshSlot::BlendIn))
			{
				// Render the geom mesh with alpha blending with nextSlot->BeginPolygonCount
				PolygonCountLod1=nextSlot->MeshGeom->getNumTriangles (distance);
				Lod1=Lod0+1;
				if (!getBypassLODOpacityFlag()) setTransparency(true);
			}
		}
		else
		{
			if (slot.MeshGeom)
			{
				// Render without blend with goodPolyCount
				if (!getBypassLODOpacityFlag())
				{
					setTransparency (slot.isTransparent());
					setOpacity (slot.isOpaque());
				}
				Flags&=~CMeshMultiLodInstance::Lod0Blend;
			}
			else
				Lod0=0xffffffff;
		}


	}
}

// ***************************************************************************
void		CMeshMultiLodInstance::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	if(Shape)
	{
		// Get a pointer on the shape.
		CMeshMultiLod *pMesh =safe_cast<CMeshMultiLod*> ((IShape*)Shape);
		// Affect the mesh directly.
		pMesh->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
	}
}


// ***************************************************************************
float	   CMeshMultiLodInstance::getNumTriangles (float distance)
{
	CMeshMultiLod *shape = safe_cast<CMeshMultiLod*> ((IShape*)Shape);
	return shape->getNumTrianglesWithCoarsestDist(distance, _CoarseMeshDistance);
}


// ***************************************************************************
void		CMeshMultiLodInstance::initRenderFilterType()
{
	if(Shape)
	{
		CMeshMultiLod *shape = safe_cast<CMeshMultiLod*> ((IShape*)Shape);

		// Look only the First LOD to know if it has a VP or not
		bool			hasVP= false;
		bool			coarseMesh;
		if(shape->getNumSlotMesh()>0 && shape->getSlotMesh(0, coarseMesh))
		{
			IMeshGeom		*meshGeom= shape->getSlotMesh(0, coarseMesh);
			// hasVP possible only if not a coarseMesh.
			if(!coarseMesh)
				hasVP= meshGeom->hasMeshVertexProgram();
		}

		if(hasVP)
			_RenderFilterType= UScene::FilterMeshLodVP;
		else
			_RenderFilterType= UScene::FilterMeshLodNoVP;
	}
}


// ***************************************************************************
void		CMeshMultiLodInstance::setUVCoarseMesh( CMeshGeom &geom, uint vtDstSize, uint dstUvOff )
{
	// *** Copy UVs to the vertices

	// Src vertex buffer
	const CVertexBuffer &vbSrc=geom.getVertexBuffer();
	CVertexBufferRead vba;
	vbSrc.lock (vba);

	// Check the vertex format and src Vertices
	nlassert (vbSrc.getVertexFormat() & (CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag) );
	nlassert (vbSrc.getNumVertices()==_LastCoarseMeshNumVertices);

	// src Vertex size
	uint vtSrcSize=vbSrc.getVertexSize ();

	// Copy vector
	const uint8 *vSrc = (const uint8 *)vba.getTexCoordPointer(0,0);
	uint8 *vDest = &_CoarseMeshVB[0];
	vDest+= dstUvOff;

	// Transform it
	for (uint i=0; i<_LastCoarseMeshNumVertices; i++)
	{
		// Transform position
		*(CUV*)vDest = *(const CUV*)vSrc;

		// Next point
		vSrc+=vtSrcSize;
		vDest+=vtDstSize;
	}
}
// ***************************************************************************
void		CMeshMultiLodInstance::setPosCoarseMesh( CMeshGeom &geom, const CMatrix &matrix, uint vtDstSize )
{
	// *** Transform the vertices

	// Src vertex buffer
	const CVertexBuffer &vbSrc=geom.getVertexBuffer();
	CVertexBufferRead vba;
	vbSrc.lock (vba);

	// Check the vertex format and src Vertices
	nlassert (vbSrc.getVertexFormat() & (CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag) );
	nlassert (vbSrc.getNumVertices()==_LastCoarseMeshNumVertices);

	// src Vertex size
	uint vtSrcSize=vbSrc.getVertexSize ();

	// Copy vector
	const uint8 *vSrc = (const uint8 *)vba.getVertexCoordPointer (0);
	uint8 *vDest = &_CoarseMeshVB[0];

	// Transform it
	for (uint i=0; i<_LastCoarseMeshNumVertices; i++)
	{
		// Transform position
		*(CVector*)vDest = matrix.mulPoint (*(const CVector*)vSrc);

		// Next point
		vSrc+=vtSrcSize;
		vDest+=vtDstSize;
	}
}
// ***************************************************************************
void		CMeshMultiLodInstance::setColorCoarseMesh( CRGBA color, uint vtDstSize, uint dstColorOff )
{
	// *** Copy color to vertices

	// Copy vector
	uint8 *vDest = &_CoarseMeshVB[0];
	vDest+= dstColorOff;

	// Transform it
	for (uint i=0; i<_LastCoarseMeshNumVertices; i++)
	{
		// Transform position
		*(CRGBA*)vDest = color;

		// Next point
		vDest+=vtDstSize;
	}
}


} // NL3D
