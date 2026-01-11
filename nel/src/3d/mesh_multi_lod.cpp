// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/3d/mesh_multi_lod.h"
#include "nel/3d/mesh_multi_lod_instance.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/scene.h"
#include "nel/3d/coarse_mesh_manager.h"
#include "nel/3d/skeleton_model.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/mesh_blender.h"
#include "nel/3d/visual_collision_mesh.h"

#include "nel/misc/debug.h"
#include "nel/misc/hierarchical_timer.h"

using namespace NLMISC;
using namespace std;



#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{



// ***************************************************************************

void CMeshMultiLod::build(CMeshMultiLodBuild &mbuild)
{
	// Clear the mesh
	clear ();

	// Build the base mesh
	CMeshBase::buildMeshBase (mbuild.BaseMesh);

	// Static flag
	_StaticLod=mbuild.StaticLod;

	// Resize the array
	_MeshVector.resize (mbuild.LodMeshes.size());

	// For each slots
	for (uint slot=0; slot<mbuild.LodMeshes.size(); slot++)
	{
		// Dist max
		_MeshVector[slot].DistMax=mbuild.LodMeshes[slot].DistMax;

		// BlendLength
		_MeshVector[slot].BlendLength=mbuild.LodMeshes[slot].BlendLength;

		// Flags
		_MeshVector[slot].Flags=0;

		// Blend in ?
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::BlendIn)
			_MeshVector[slot].Flags|=CMeshSlot::BlendIn;

		// Blend out ?
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::BlendOut)
			_MeshVector[slot].Flags|=CMeshSlot::BlendOut;

		// Coarse mesh ?
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::CoarseMesh)
		{
			// Flag
			_MeshVector[slot].Flags|=CMeshSlot::CoarseMesh;
		}

		// Is opaque
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::IsOpaque)
			_MeshVector[slot].Flags|=CMeshSlot::IsOpaque;

		// Is transparent
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::IsTransparent)
			_MeshVector[slot].Flags|=CMeshSlot::IsTransparent;

		// MeshGeom
		nlassert (mbuild.LodMeshes[slot].MeshGeom);

		// Valid pointer ?
		if (_MeshVector[slot].Flags&CMeshSlot::CoarseMesh)
		{
			// If it is a coarse mesh, it must be a CMeshGeom.
			if (dynamic_cast<CMeshGeom*>(mbuild.LodMeshes[slot].MeshGeom)==NULL)
			{
				// If it is a coarse mesh, it must be a CMeshGeom.
				_MeshVector[slot].MeshGeom = NULL;
				delete mbuild.LodMeshes[slot].MeshGeom;
			}
			else
				// Ok, no prb
				_MeshVector[slot].MeshGeom = mbuild.LodMeshes[slot].MeshGeom;
		}
		else
			// Ok, no prb
			_MeshVector[slot].MeshGeom = mbuild.LodMeshes[slot].MeshGeom;
	}

	// Sort the slot by the distance...
	for (int i=(uint)mbuild.LodMeshes.size()-1; i>0; i--)
	for (int j=0; j<i; j++)
	{
		// Bad sort ?
		if (_MeshVector[j].DistMax>_MeshVector[j+1].DistMax)
		{
			// Exchange slots
			CMeshSlot tmp=_MeshVector[j];
			_MeshVector[j]=_MeshVector[j+1];
			_MeshVector[j+1]=tmp;
			tmp.MeshGeom=NULL;
		}
	}

	// Calc start and end polygon count
	for (uint k=0; k<mbuild.LodMeshes.size(); k++)
	{
		// Get start distance
		float startDist;
		if (k==0)
			startDist=0;
		else
			startDist=_MeshVector[k-1].DistMax;

		// Get start poly count
		float startPolyCount;
		startPolyCount=_MeshVector[k].MeshGeom->getNumTriangles (startDist);

		// Get end distance
		float endDist=_MeshVector[k].DistMax;

		// Get end poly count
		if (k==mbuild.LodMeshes.size()-1)
		{
			_MeshVector[k].EndPolygonCount=_MeshVector[k].MeshGeom->getNumTriangles (endDist);
			if (startPolyCount==_MeshVector[k].EndPolygonCount)
				_MeshVector[k].EndPolygonCount=startPolyCount/2;
		}
		else
			_MeshVector[k].EndPolygonCount=_MeshVector[k+1].MeshGeom->getNumTriangles (endDist);

		// Calc A
		if (endDist==startDist)
			_MeshVector[k].A=0;
		else
			_MeshVector[k].A=(_MeshVector[k].EndPolygonCount-startPolyCount)/(endDist-startDist);

		// Calc A
		_MeshVector[k].B=_MeshVector[k].EndPolygonCount-_MeshVector[k].A*endDist;
	}

	// End: compile some stuff
	compileRunTime();
}

// ***************************************************************************

CTransformShape	*CMeshMultiLod::createInstance(CScene &scene)
{
	// Create a CMeshInstance, an instance of a multi lod mesh.
	CMeshMultiLodInstance *mi=(CMeshMultiLodInstance*)scene.createModel(NL3D::MeshMultiLodInstanceId);
	mi->Shape= this;
	mi->_LastLodMatrixDate=0;

	// instanciate the material part of the Mesh, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi, &scene);

	// Create the necessary space for Coarse Instanciation
	instanciateCoarseMeshSpace(mi);

	// For all lods, do some instance init for MeshGeom
	for(uint i=0; i<_MeshVector.size(); i++)
	{
		if(_MeshVector[i].MeshGeom)
			_MeshVector[i].MeshGeom->initInstance(mi);
	}

	// init the Filter type
	mi->initRenderFilterType();


	return mi;
}

// ***************************************************************************

bool CMeshMultiLod::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// Look for the biggest mesh
	uint meshCount=(uint)_MeshVector.size();
	for (uint i=0; i<meshCount; i++)
	{
		// Ref on slot
		CMeshSlot &slot=_MeshVector[i];

		// Is mesh present ?
		if (slot.MeshGeom)
		{
			// Clip this mesh
			return slot.MeshGeom->clip (pyramid, worldMatrix);
		}
	}
	return true;
}

// ***************************************************************************

void CMeshMultiLod::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	// Render good meshes
	CMeshMultiLodInstance *instance=safe_cast<CMeshMultiLodInstance*>(trans);

	// Static or dynamic coarse mesh ?
	CCoarseMeshManager *manager;
	// Get the coarse mesh manager
	manager=instance->getOwnerScene()->getCoarseMeshManager();

	// *** Render Lods

	// Second lod ?
	if ( (instance->Lod1!=0xffffffff) && (passOpaque==false) )
	{
		// build rdrFlags to rdr both transparent and opaque materials,
		// use globalAlphaBlend, and disable ZWrite for Lod1
		uint32	rdrFlags= IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderTransparentMaterial |
			IMeshGeom::RenderGlobalAlpha | IMeshGeom::RenderGADisableZWrite;
		// NB: very important to render Lod1 first, because Lod0 is still rendered with ZWrite enabled.
		renderMeshGeom (instance->Lod1, drv, instance, instance->PolygonCountLod1, rdrFlags, 1.f-instance->BlendFactor, manager);
	}


	// Have an opaque pass ?
	if ( (instance->Flags&CMeshMultiLodInstance::Lod0Blend) == 0)
	{
		// Is this slot a CoarseMesh?
		if ( _MeshVector[instance->Lod0].Flags&CMeshSlot::CoarseMesh )
		{
			// render as a CoarseMesh the lod 0, only in opaque pass
			if(passOpaque)
				renderCoarseMesh (instance->Lod0, drv, instance, manager);
		}
		else
		{
			// build rdrFlags the normal way (as CMesh::render() for example)
			uint32	mask= (0-(uint32)passOpaque);
			uint32	rdrFlags;
			// select rdrFlags, without ifs.
			rdrFlags=	mask & (IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderPassOpaque);
			rdrFlags|=	~mask & (IMeshGeom::RenderTransparentMaterial);
			// Only render the normal way the first lod
			renderMeshGeom (instance->Lod0, drv, instance, instance->PolygonCountLod0, rdrFlags, 1, manager);
		}
	}
	else
	{
		// Should not be in opaque
		nlassert (passOpaque==false);

		// build rdrFlags to rdr both transparent and opaque materials,
		// use globalAlphaBlend, BUT Don't disable ZWrite for Lod0
		uint32	rdrFlags= IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderTransparentMaterial |
			IMeshGeom::RenderGlobalAlpha;

		// Render first lod in blend mode. Don't disable ZWrite for Lod0
		renderMeshGeom (instance->Lod0, drv, instance, instance->PolygonCountLod0, rdrFlags, instance->BlendFactor, manager);
	}
}

// ***************************************************************************

void CMeshMultiLod::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// Serial a version number
	(void)f.serialVersion (0);

	// serial Materials infos contained in CMeshBase.
	CMeshBase::serialMeshBase(f);

	// Static lod flag
	f.serial (_StaticLod);

	// Serial the values
	f.serialCont (_MeshVector);


	// if reading, compile some stuff
	if (f.isReading())
		compileRunTime();
}

// ***************************************************************************

float CMeshMultiLod::getNumTrianglesWithCoarsestDist(float distance, float coarsestMeshDist) const
{
	// Look in the table for good distances..
	uint meshCount=(uint)_MeshVector.size();

	// At least on mesh
	if (meshCount>0)
	{

		if (coarsestMeshDist != -1)
		{
			if (coarsestMeshDist != 0)
			{
				// rescale distance to new coarse mesh distance..
				distance *= _MeshVector[meshCount - 1].DistMax / coarsestMeshDist;
			}
		}

		uint i=0;
		// Look for good i
		while ( _MeshVector[i].DistMax < distance)
		{
			if (i==meshCount-1)
				// Abort if last one
				break;
			i++;
		}

		// Ref on slot
		const CMeshSlot &slot=_MeshVector[i];

		// Is mesh present ?
		if (slot.MeshGeom)
		{
			// Get the polygon count with the distance
			float polyCount=slot.A * distance + slot.B;

			/*// Get the perfect polygon count in this slot for the asked distance
			float goodPolyCount=slot.MeshGeom->getNumTriangles (distance);

			// Get the next slot perfect polygon count
			float realEndPolyCount;

			// Last slot ?
			if ( (i<meshCount-1) && _MeshVector[i+1].MeshGeom )
				// Take end number polygon count in the next slot
				realEndPolyCount=_MeshVector[i+1].MeshGeom->getNumTriangles (slot.DistMax);
			else
				// Take end number polygon count in the this slot
				realEndPolyCount=slot.EndPolygonCount;

			// Return blended polygon count to have a continous decreasing function
			return (goodPolyCount-slot.BeginPolygonCount) * (realEndPolyCount-slot.BeginPolygonCount) /
				(slot.EndPolygonCount-slot.BeginPolygonCount) + slot.BeginPolygonCount;*/
			return polyCount;
		}
	}

	return 0;
}

// ***************************************************************************

void CMeshMultiLod::getAABBox(NLMISC::CAABBox &bbox) const
{
	// Get count
	uint count=(uint)_MeshVector.size();
	for (uint slot=0; slot<count; slot++)
	{
		// Shape ?
		if (_MeshVector[slot].MeshGeom)
		{
			// Get the bounding box
			bbox=_MeshVector[slot].MeshGeom->getBoundingBox().getAABBox();

			// ok
			break;
		}
	}
}

// ***************************************************************************

void CMeshMultiLod::clear ()
{
	_MeshVector.clear ();
}

// ***************************************************************************

void CMeshMultiLod::CMeshSlot::serial(NLMISC::IStream &f)
{
	// Check version
	(void)f.serialVersion (0);

	f.serialPolyPtr (MeshGeom);
	f.serial (A);
	f.serial (B);
	f.serial (DistMax);
	f.serial (EndPolygonCount);
	f.serial (BlendLength);
	f.serial (Flags);

	if (f.isReading())
	{
	}
}

// ***************************************************************************

CMeshMultiLod::CMeshSlot::CMeshSlot ()
{
	MeshGeom=NULL;
	CoarseNumTris= 0;
}

// ***************************************************************************

CMeshMultiLod::CMeshSlot::~CMeshSlot ()
{
	if (MeshGeom)
		delete MeshGeom;
}

// ***************************************************************************

void CMeshMultiLod::renderMeshGeom (uint slot, IDriver *drv, CMeshMultiLodInstance *trans, float numPoylgons, uint32 rdrFlags, float alpha, CCoarseMeshManager *manager)
{
	// Ref
	CMeshSlot &slotRef=_MeshVector[slot];

	// MeshGeom exist?
	if (slotRef.MeshGeom)
	{
		// NB Here, the meshGeom may still be a coarseMesh, but rendered through CMeshGeom
		if(slotRef.Flags&CMeshSlot::CoarseMesh)
		{
			// Render only for opaque material
			if(manager && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) )
			{
				bool	gaDisableZWrite= (rdrFlags & IMeshGeom::RenderGADisableZWrite)?true:false;

				// Render the CoarseMesh with the manager material
				CMaterial	&material= manager->getMaterial();

				// modulate material for alphaBlend transition
				// ----------
				// get average sun color for this coarseMesh
				CRGBA	newCol= trans->getCoarseMeshLighting();

				// Use a CMeshBlender to modify material and driver.
				CMeshBlender	blender;
				blender.prepareRenderForGlobalAlphaCoarseMesh(material, drv, newCol, alpha, gaDisableZWrite);


				// render simple the coarseMesh
				CMeshGeom *meshGeom= safe_cast<CMeshGeom*>(slotRef.MeshGeom);

				// Force corse mesh vertex buffer in system memory
				const_cast<CVertexBuffer&>(meshGeom->getVertexBuffer ()).setPreferredMemory (CVertexBuffer::RAMPreferred, false);

				meshGeom->renderSimpleWithMaterial(drv, trans->getWorldMatrix(), material);


				// resetup standard CoarseMeshMaterial material values
				// ----------
				// blender restore
				blender.restoreRenderCoarseMesh(material, drv, gaDisableZWrite);
			}
		}
		else
		{
			// Render the geom mesh
			// Disable ZWrite only if in transition and for rendering Lod1
			slotRef.MeshGeom->render (drv, trans, numPoylgons, rdrFlags, alpha);
		}
	}
}
// ***************************************************************************

void CMeshMultiLod::renderCoarseMesh (uint slot, IDriver *drv, CMeshMultiLodInstance *trans, CCoarseMeshManager *manager)
{
	// if the manager is NULL, quit.
	if(manager==NULL)
		return;

	// get the scene
	CScene	*scene= trans->getOwnerScene();
	if(!scene)
		return;

	// If filtered...
	if( (scene->getFilterRenderFlags() & UScene::FilterCoarseMesh)==0 )
		return;

	// Ref
	CMeshSlot &slotRef=_MeshVector[slot];

	// the slot must be a Coarse mesh
	nlassert(slotRef.Flags&CMeshSlot::CoarseMesh);

	// Get a pointer on the geom mesh
	CMeshGeom *meshGeom= safe_cast<CMeshGeom*>(slotRef.MeshGeom);

	// ** If not the same as Before (or if NULL before...)
	if ( trans->_LastCoarseMesh!=meshGeom )
	{
		uint	numVerts= meshGeom->getVertexBuffer().getNumVertices();
		uint	numTris= slotRef.CoarseNumTris;
		// If empty meshGeom, erase cache (each frame, ugly but error mgt here...)
		if( numTris==0 || numVerts==0 )
			trans->_LastCoarseMesh= NULL;
		else
		{
			// Cache
			trans->_LastCoarseMesh= meshGeom;
			trans->_LastCoarseMeshNumVertices= numVerts;

			// Check setuped size.
			nlassert( trans->_CoarseMeshVB.size() >= numVerts*manager->getVertexSize() );

			// Fill only UVs here. (Pos updated in Matrix pass. Color in Lighting Pass)
			trans->setUVCoarseMesh( *meshGeom, manager->getVertexSize(), manager->getUVOff() );

			// Dirt the matrix
			trans->_LastLodMatrixDate=0;
			// Dirt the lighting. NB: period maximum is 255. Hence the -256, to ensure lighting compute now
			trans->_LastLodLightingDate= -0x100;
		}
	}

	// ** If setuped, update and render
	if( trans->_LastCoarseMesh )
	{
		// Matrix has changed ?
		if ( trans->ITransformable::compareMatrixDate (trans->_LastLodMatrixDate) )
		{
			// Get date
			trans->_LastLodMatrixDate = trans->ITransformable::getMatrixDate();

			// Set matrix
			trans->setPosCoarseMesh ( *meshGeom, trans->getMatrix(), manager->getVertexSize() );
		}

		// Lighting: test if must update lighting, according to date of HrcTrav (num of CScene::render() call).
		sint64	currentDate= scene->getHrcTrav().CurrentDate;
		if( trans->_LastLodLightingDate < currentDate - scene->getCoarseMeshLightingUpdate() )
		{
			// reset the date.
			trans->_LastLodLightingDate= currentDate;

			// get average sun color
			CRGBA	sunContrib= trans->getCoarseMeshLighting();

			// Invert BR if driver is BGRA
			if(drv->getVertexColorFormat()==CVertexBuffer::TBGRA)
				sunContrib.swapBR();

			// Set color
			trans->setColorCoarseMesh ( sunContrib, manager->getVertexSize(), manager->getColorOff());
		}

		// Add dynamic to the manager
		if( !manager->addMesh(trans->_LastCoarseMeshNumVertices, &trans->_CoarseMeshVB[0], slotRef.CoarseNumTris, &slotRef.CoarseTriangles[0] ) )
		{
			// If failure, flush the manager
			manager->flushRender(drv);
			// then try to re-add. No-op if fails this time..
			manager->addMesh(trans->_LastCoarseMeshNumVertices, &trans->_CoarseMeshVB[0], slotRef.CoarseNumTris, &slotRef.CoarseTriangles[0]  );
		}
	}

}

// ***************************************************************************
void	CMeshMultiLod::compileDistMax()
{
	// Last element
	if(_MeshVector.empty())
		IShape::_DistMax= -1;
	else
		IShape::_DistMax= _MeshVector.back().DistMax;
}

// ***************************************************************************
const IMeshGeom& CMeshMultiLod::getMeshGeom (uint slot) const
{
	// Checks
	nlassert (slot<getNumSlotMesh ());

	return *_MeshVector[slot].MeshGeom;
}


// ***************************************************************************
void			CMeshMultiLod::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	// no-op if empty.
	if(getNumSlotMesh ()==0)
		return;

	// If not NULL
	if(_MeshVector[0].MeshGeom==NULL)
		return;

	// verify it is a CMeshMRMGeom. else no-op.
	CMeshMRMGeom	*mgeom= dynamic_cast<CMeshMRMGeom*>(_MeshVector[0].MeshGeom);
	if(mgeom==NULL)
		return;

	// ok, setup.
	mgeom->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
}


// ***************************************************************************
IMeshGeom		*CMeshMultiLod::supportMeshBlockRendering (CTransformShape *trans, float &polygonCount ) const
{
	IMeshGeom	*ret= NULL;

	// get the instance
	CMeshMultiLodInstance *instance=safe_cast<CMeshMultiLodInstance*>(trans);

	// Must not be in blend transition.
	if ( (instance->Flags&CMeshMultiLodInstance::Lod0Blend) == 0)
	{
		uint	slot= instance->Lod0;
		// The slot must not be a CoarseMesh
		if ( (_MeshVector[slot].Flags&CMeshSlot::CoarseMesh)==0 )
		{
			// MeshGeom exist?
			ret= _MeshVector[slot].MeshGeom;
		}
	}

	// Ok if meshGeom is ok.
	if( ret && ret->supportMeshBlockRendering() )
	{
		polygonCount= instance->PolygonCountLod0;
		return ret;
	}
	else
		return NULL;
}


// ***************************************************************************
void	CMeshMultiLod::profileMeshGeom (uint slot, CRenderTrav *rdrTrav, CMeshMultiLodInstance *trans, float numPoylgons, uint32 rdrFlags)
{
	// Ref
	CMeshSlot &slotRef=_MeshVector[slot];

	// MeshGeom exist?
	if (slotRef.MeshGeom)
	{
		// NB Here, the meshGeom may still be a coarseMesh, but rendered through CMeshGeom
		if(slotRef.Flags&CMeshSlot::CoarseMesh)
		{
			// Render only for opaque material
			if(rdrFlags & IMeshGeom::RenderOpaqueMaterial)
			{
				slotRef.MeshGeom->profileSceneRender(rdrTrav, trans, numPoylgons, rdrFlags);
			}
		}
		else
		{
			slotRef.MeshGeom->profileSceneRender(rdrTrav, trans, numPoylgons, rdrFlags);
		}
	}
}


// ***************************************************************************
void	CMeshMultiLod::profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, bool passOpaque)
{
	// Render good meshes
	CMeshMultiLodInstance *instance=safe_cast<CMeshMultiLodInstance*>(trans);


	// Second lod ?
	if ( (instance->Lod1!=0xffffffff) && (passOpaque==false) )
	{
		// build rdrFlags to rdr both transparent and opaque materials,
		// use globalAlphaBlend, and disable ZWrite for Lod1
		uint32	rdrFlags= IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderTransparentMaterial |
			IMeshGeom::RenderGlobalAlpha | IMeshGeom::RenderGADisableZWrite;
		// NB: very important to render Lod1 first, because Lod0 is still rendered with ZWrite enabled.
		profileMeshGeom (instance->Lod1, rdrTrav, instance, instance->PolygonCountLod1, rdrFlags);
	}


	// Have an opaque pass ?
	if ( (instance->Flags&CMeshMultiLodInstance::Lod0Blend) == 0)
	{
		// Is this slot a CoarseMesh?
		if ( _MeshVector[instance->Lod0].Flags&CMeshSlot::CoarseMesh )
		{
		}
		else
		{
			// build rdrFlags the normal way (as CMesh::render() for example)
			uint32	mask= (0-(uint32)passOpaque);
			uint32	rdrFlags;
			// select rdrFlags, without ifs.
			rdrFlags=	mask & (IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderPassOpaque);
			rdrFlags|=	~mask & (IMeshGeom::RenderTransparentMaterial);
			// Only render the normal way the first lod
			profileMeshGeom (instance->Lod0, rdrTrav, instance, instance->PolygonCountLod0, rdrFlags);
		}
	}
	else
	{
		// Should not be in opaque
		nlassert (passOpaque==false);

		// build rdrFlags to rdr both transparent and opaque materials,
		// use globalAlphaBlend, BUT Don't disable ZWrite for Lod0
		uint32	rdrFlags= IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderTransparentMaterial |
			IMeshGeom::RenderGlobalAlpha;

		// Render first lod in blend mode. Don't disable ZWrite for Lod0
		profileMeshGeom (instance->Lod0, rdrTrav, instance, instance->PolygonCountLod0, rdrFlags);
	}
}

// ***************************************************************************
void	CMeshMultiLod::instanciateCoarseMeshSpace(CMeshMultiLodInstance *mi)
{
	CCoarseMeshManager	*manager= mi->getOwnerScene()->getCoarseMeshManager();

	if(manager)
	{
		// For all MeshSlots that have a CoarseMesh, count max Coarse NumVertices;
		uint	numVertices= 0;
		for(uint i=0;i<_MeshVector.size();i++)
		{
			CMeshSlot	&slotRef= _MeshVector[i];
			if( slotRef.Flags & CMeshSlot::CoarseMesh )
			{
				// Get a pointer on the geom mesh
				CMeshGeom *meshGeom= safe_cast<CMeshGeom*>(slotRef.MeshGeom);
				numVertices= max(numVertices, (uint)meshGeom->getVertexBuffer().getNumVertices() );
			}
		}

		// Then allocate vertex space for dest manager vertex size.
		mi->_CoarseMeshVB.resize( numVertices*manager->getVertexSize() );
	}
}

// ***************************************************************************
void	CMeshMultiLod::compileCoarseMeshes()
{
	// For All Slots that are CoarseMeshes.
	for(uint i=0;i<_MeshVector.size();i++)
	{
		CMeshSlot	&slotRef= _MeshVector[i];
		if( slotRef.Flags & CMeshSlot::CoarseMesh )
		{
			// reset
			slotRef.CoarseNumTris= 0;

			// Get a pointer on the geom mesh
			CMeshGeom *meshGeom= safe_cast<CMeshGeom*>(slotRef.MeshGeom);

			// For All RdrPass of the 1st matrix block
			if( meshGeom->getNbMatrixBlock()>0 )
			{
				// 1st count
				for(uint i=0;i<meshGeom->getNbRdrPass(0);i++)
				{
					slotRef.CoarseNumTris+= meshGeom->getRdrPassPrimitiveBlock(0, i).getNumIndexes()/3;
				}

				// 2nd allocate and fill
				if( slotRef.CoarseNumTris )
				{
					slotRef.CoarseTriangles.resize(slotRef.CoarseNumTris * 3);
					TCoarseMeshIndexType	*dstPtr= &slotRef.CoarseTriangles[0];
					uint totalTris = 0;
					for(uint i=0;i<meshGeom->getNbRdrPass(0);i++)
					{
						const CIndexBuffer	&pb= meshGeom->getRdrPassPrimitiveBlock(0, i);
						CIndexBufferRead ibaRead;
						pb.lock (ibaRead);
						uint	numTris= pb.getNumIndexes()/3;
						totalTris += numTris;
						if (pb.getFormat() == CIndexBuffer::Indices16)
						{
							if (sizeof(TCoarseMeshIndexType) == sizeof(uint16))
							{
								memcpy(dstPtr, (uint16 *) ibaRead.getPtr(), numTris*3*sizeof(uint16));
								dstPtr+= numTris*3;
							}
							else
							{
								// 16 -> 32
								uint16 *src = (uint16 *) ibaRead.getPtr();
								for(uint k = 0; k < numTris; ++k)
								{
									*dstPtr++ = (TCoarseMeshIndexType) *src++;
									*dstPtr++ = (TCoarseMeshIndexType) *src++;
									*dstPtr++ = (TCoarseMeshIndexType) *src++;
								}
							}
						}
						else
						{
							if (sizeof(TCoarseMeshIndexType) == sizeof(uint32))
							{
								memcpy(dstPtr, (uint32 *) ibaRead.getPtr(), numTris*3*sizeof(uint32));
								dstPtr+= numTris*3;
							}
							else
							{
								const uint32 *src = (const uint32 *) ibaRead.getPtr();
								for(uint k = 0; k < numTris; ++k)
								{
									// 32 -> 16
									nlassert(src[0] <= 0xffff);
									nlassert(src[1] <= 0xffff);
									nlassert(src[2] <= 0xffff);
									*dstPtr++ = (TCoarseMeshIndexType) *src++;
									*dstPtr++ = (TCoarseMeshIndexType) *src++;
									*dstPtr++ = (TCoarseMeshIndexType) *src++;
								}
							}
						}
					}
					nlassert(totalTris == slotRef.CoarseNumTris);
				}
			}
		}
	}
}


// ***************************************************************************
void	CMeshMultiLod::compileRunTime()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// **** MultiLod basics
	compileDistMax();
	compileCoarseMeshes();

	// **** try to build a Visual Collision Mesh
	// clear first
	if(_VisualCollisionMesh)
	{
		delete _VisualCollisionMesh;
		_VisualCollisionMesh= NULL;
	}
	// build only if wanted
	if( (_CollisionMeshGeneration==AutoCameraCol && !_LightInfos.empty()) ||
		_CollisionMeshGeneration==ForceCameraCol )
	{
		// try to retrieve the info from a CMeshGeom only
		if(getNumSlotMesh())
		{
			const CMeshGeom		*meshGeom= dynamic_cast<const CMeshGeom*>(&getMeshGeom(0));
			if(meshGeom)
			{
				vector<CVector>		vertices;
				vector<uint32>		indices;
				if(meshGeom->retrieveVertices(vertices) && meshGeom->retrieveTriangles(indices))
				{
					// ok, can build!
					_VisualCollisionMesh= new CVisualCollisionMesh;
					// if fails to build cause of too many vertices/indices for instance
					if(!_VisualCollisionMesh->build(vertices, indices,const_cast<CVertexBuffer&>(meshGeom->getVertexBuffer())))
					{
						// delete
						delete _VisualCollisionMesh;
						_VisualCollisionMesh= NULL;
					}
				}
			}
		}
	}
}


// ***************************************************************************
void	CMeshMultiLod::buildSystemGeometry()
{
	// clear any
	_SystemGeometry.clear();

	// Use the first lod, for system geometry copy
	if(getNumSlotMesh())
	{
		// the first is a meshGeom?
		const CMeshGeom		*meshGeom= dynamic_cast<const CMeshGeom*>(&getMeshGeom(0));
		if(meshGeom)
		{
			// retrieve geometry (if VB/IB not resident)
			if( !meshGeom->retrieveVertices(_SystemGeometry.Vertices) ||
				!meshGeom->retrieveTriangles(_SystemGeometry.Triangles))
			{
				_SystemGeometry.clear();
			}
		}
		// else it is a mrm geom?
		else
		{
			const CMeshMRMGeom		*meshMRMGeom= dynamic_cast<const CMeshMRMGeom*>(&getMeshGeom(0));
			if(meshMRMGeom)
			{
				// Choose the best Lod available for system geometry
				if(meshMRMGeom->getNbLodLoaded()==0)
					return;
				uint	lodId= meshMRMGeom->getNbLodLoaded()-1;

				// retrieve geometry (if VB/IB not resident)
				if( !meshMRMGeom->buildGeometryForLod(lodId, _SystemGeometry.Vertices, _SystemGeometry.Triangles) )
				{
					_SystemGeometry.clear();
				}
			}
		}
	}

	// TestYoyo
	/*static uint32	totalMem= 0;
	totalMem+= _SystemGeometry.Vertices.size()*sizeof(CVector);
	totalMem+= _SystemGeometry.Triangles.size()*sizeof(uint32);
	nlinfo("CMeshMultiLod: TotalMem: %d", totalMem);*/
}

} // NL3D

