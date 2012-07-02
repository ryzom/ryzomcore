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

#include "nel/3d/mesh_block_manager.h"
#include "nel/misc/hierarchical_timer.h"

using namespace NLMISC;

namespace NL3D
{


#define	NL3D_MBM_MAX_VBHEAP			255
#define	NL3D_MBM_VBHEAP_MESH_SHIFT	8
#define	NL3D_MBM_VBHEAP_MESH_MASK	0xFFFFFF00
#define	NL3D_MBM_VBHEAP_HEAP_MASK	0x000000FF


// ***************************************************************************
CMeshBlockManager::CMeshBlockManager()
{
	_RenderCtx.Driver= NULL;
	_RenderCtx.Scene= NULL;
	_RenderCtx.RenderTrav= NULL;

	// Allocate at least the 0th heap
	_VBHeapBlocks.resize(1);
	_VBHeapBlocks[0]= new CVBHeapBlock;
	// some reserve, avoiding first reallocation.
	_VBHeapBlocks[0]->RdrInstances.reserve(100);
	_VBHeapBlocks[0]->RdrMeshGeoms.reserve(100);
}

// ***************************************************************************
CMeshBlockManager::~CMeshBlockManager()
{
	// must release any user heap.
	releaseVBHeaps();
	// Release the 0th one.
	delete _VBHeapBlocks[0];
	_VBHeapBlocks.clear();
}

// ***************************************************************************
void			CMeshBlockManager::addInstance(IMeshGeom *meshGeom, CMeshBaseInstance *inst, float polygonCount)
{
	// If the meshGeom has never been added to the manager, may do some precalc
	if(meshGeom->_MeshBlockManager==NULL)
	{
		// Fill
		meshGeom->_MeshBlockManager= this;
		// try to fit the meshGeom in one of our VBHeap.
		allocateMeshVBHeap(meshGeom);
	}

	// TestYoyo
	/*extern uint	TEMP_Yoyo_NInstVBHeap;
	extern uint	TEMP_Yoyo_NInstNoVBHeap;
	if( meshGeom->_MeshVBHeapId & NL3D_MBM_VBHEAP_HEAP_MASK )
		TEMP_Yoyo_NInstVBHeap++;
	else
		TEMP_Yoyo_NInstNoVBHeap++;*/
	// End TestYoyo

	// Choose the HeapBlock to fit in.
	CVBHeapBlock	*hb= _VBHeapBlocks[meshGeom->_MeshVBHeapId & NL3D_MBM_VBHEAP_HEAP_MASK];

	// If the mesh geom is not added to this manager, add it.
	if(meshGeom->_RootInstanceId==-1)
	{
		hb->RdrMeshGeoms.push_back(meshGeom);
	}

	// setup the instance.
	CInstanceInfo		instInfo;
	instInfo.MeshGeom= meshGeom;
	instInfo.MBI= inst;
	instInfo.PolygonCount= polygonCount;

	// link to the head of the list.
	instInfo.NextInstance= meshGeom->_RootInstanceId;
	meshGeom->_RootInstanceId= (sint32)hb->RdrInstances.size();

	// add this instance
	hb->RdrInstances.push_back(instInfo);

}

// ***************************************************************************
void			CMeshBlockManager::flush(IDriver *drv, CScene *scene, CRenderTrav *renderTrav)
{
	uint	i,j;

	H_AUTO( NL3D_MeshBlockManager );

	// setup the manager
	nlassert(drv && scene && renderTrav);
	_RenderCtx.Driver= drv;
	_RenderCtx.Scene= scene;
	_RenderCtx.RenderTrav= renderTrav;

	// render
	//==========

	// sort by Heap first => small setup of VBs.
	for(j=0; j<_VBHeapBlocks.size();j++)
	{
		CVBHeapBlock	*hb= _VBHeapBlocks[j];
		// if not the special 0th heap, must activate VB.
		if(j==0)
		{
			_RenderCtx.RenderThroughVBHeap= false;
		}
		else
		{
			// set to true => avoid mesh to setup their own VB.
			_RenderCtx.RenderThroughVBHeap= true;
			// activate current VB in driver
#if 0		// todo hulud remove / restore VBHeap
			hb->VBHeap.activate();
#endif		// todo hulud remove / restore VBHeap
		}


		// Always sort by MeshGeom, in this heap
		for(i=0; i<hb->RdrMeshGeoms.size();i++)
		{
			// render the meshGeom and his instances
			render(hb, hb->RdrMeshGeoms[i], hb->RdrInstances);
		}
	}

	// reset.
	//==========

	// For all vb heaps
	for(j=0; j<_VBHeapBlocks.size();j++)
	{
		CVBHeapBlock	*hb= _VBHeapBlocks[j];

		// Parse all MehsGeoms, and flag them as Not Added to me
		for(i=0; i<hb->RdrMeshGeoms.size();i++)
		{
			hb->RdrMeshGeoms[i]->_RootInstanceId= -1;
		}

		// clear rdr arrays
		hb->RdrInstances.clear();
		hb->RdrMeshGeoms.clear();
	}
}


// ***************************************************************************
void			CMeshBlockManager::render(CVBHeapBlock	*vbHeapBlock, IMeshGeom *meshGeom, std::vector<CInstanceInfo> &rdrInstances)
{
	// TestYoyo
	/*extern uint TEMP_Yoyo_NMeshVBHeap;
	extern uint TEMP_Yoyo_NMeshNoVBHeap;
	if( _RenderCtx.RenderThroughVBHeap )
		TEMP_Yoyo_NMeshVBHeap++;
	else
		TEMP_Yoyo_NMeshNoVBHeap++;*/
	// End TestYoyo

	// Start for this mesh.
	meshGeom->beginMesh(_RenderCtx);


	// sort per material first?
	if( meshGeom->sortPerMaterial() )
	{
		// number of renderPasses for this mesh.
		uint	numRdrPass= meshGeom->getNumRdrPassesForMesh();

		// for all material.
		for(uint rdrPass=0;rdrPass<numRdrPass;rdrPass++)
		{
			// for all instance.
			sint32	instId= meshGeom->_RootInstanceId;
			while( instId!=-1 )
			{
				CInstanceInfo		&instInfo= rdrInstances[instId];

				// activate this instance
				meshGeom->activeInstance(_RenderCtx, instInfo.MBI, instInfo.PolygonCount, NULL);

				// render the pass.
				meshGeom->renderPass(_RenderCtx, instInfo.MBI, instInfo.PolygonCount, rdrPass);

				// next instance
				instId= instInfo.NextInstance;
			}
		}
	}
	// else sort per instance first
	else
	{
		// for all instance.
		sint32	instId= meshGeom->_RootInstanceId;
		while( instId!=-1 )
		{
			CInstanceInfo		&instInfo= rdrInstances[instId];

			// If the meshGeom need to change Some VB (geomorphs...)
			bool	needVBHeapLock= _RenderCtx.RenderThroughVBHeap && meshGeom->isActiveInstanceNeedVBFill();
			void	*vbDst= NULL;
			if(needVBHeapLock)
			{
				// Lock the VBHeap
#if 0		// todo hulud remove / restore VBHeap
				vbDst= vbHeapBlock->VBHeap.lock(meshGeom->_MeshVBHeapIndexStart);
#endif		// todo hulud remove / restore VBHeap
			}

			// activate this instance
			meshGeom->activeInstance(_RenderCtx, instInfo.MBI, instInfo.PolygonCount, vbDst);

			if(needVBHeapLock)
			{
				// unlock only what vertices have changed (ATI problem)
#if 0		// todo hulud remove / restore VBHeap
				vbHeapBlock->VBHeap.unlock(meshGeom->_MeshVBHeapIndexStart,
					meshGeom->_MeshVBHeapIndexStart + meshGeom->_MeshVBHeapNumVertices);
#endif		// todo hulud remove / restore VBHeap
			}

			// number of renderPasses for this mesh.
			uint	numRdrPass= meshGeom->getNumRdrPassesForInstance(instInfo.MBI);

			// for all material.
			for(uint rdrPass=0;rdrPass<numRdrPass;rdrPass++)
			{
				// render the pass.
				meshGeom->renderPass(_RenderCtx, instInfo.MBI, instInfo.PolygonCount, rdrPass);
			}

			// next instance
			instId= instInfo.NextInstance;
		}
	}

	// End for this mesh.
	meshGeom->endMesh(_RenderCtx);
}


// ***************************************************************************
// ***************************************************************************
// VBHeap mgt
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshBlockManager::allocateMeshVBHeap(IMeshGeom *mesh)
{
	// Get info from mesh.
	uint vertexFormat, numVertices;
	// if the mesh do not support VBHeap, quit.
	if( !mesh->getVBHeapInfo(vertexFormat, numVertices) )
		return;

	// In case of ...
	if( numVertices==0 )
		return;

	// try to find a VBHeap with this vertexFormat.
	TVBHeapMap::iterator	it= _VBHeapMap.find(vertexFormat);
	// if not found, abort
	if(it==_VBHeapMap.end())
		return;

	// access to this VBHeap.
	uint	vbHeapId= it->second;
	CVBHeapBlock	*vbHeapBlock= _VBHeapBlocks[vbHeapId];
	// try to allocate sapce into the heap. Fail=> abort.
#if 0		// todo hulud remove / restore VBHeap
	uint	indexStart;
	if( !vbHeapBlock->VBHeap.allocate(numVertices, indexStart) )
		return;
#endif		// todo hulud remove / restore VBHeap

	// All is Ok here => setup infos.
	//==================

	// Keep track of the mesh => allocate.
	uint	meshId;
	// try to get a free id.
	if( !vbHeapBlock->FreeIds.empty() )
	{
		meshId= vbHeapBlock->FreeIds.back();
		vbHeapBlock->FreeIds.pop_back();
		vbHeapBlock->AllocatedMeshGeoms[meshId]= mesh;
	}
	// else, must add to the array
	else
	{
		meshId= (uint)vbHeapBlock->AllocatedMeshGeoms.size();
		vbHeapBlock->AllocatedMeshGeoms.push_back(mesh);
	}

	// info for delete in mesh
#if 0		// todo hulud remove / restore VBHeap
	mesh->_MeshVBHeapIndexStart= indexStart;
	mesh->_MeshVBHeapId= vbHeapId + (meshId<<NL3D_MBM_VBHEAP_MESH_SHIFT);
	mesh->_MeshVBHeapNumVertices= numVertices;
#endif		// todo hulud remove / restore VBHeap

	// Fill VB.
	//==================
#if 0		// todo hulud remove / restore VBHeap
	uint8	*dst= vbHeapBlock->VBHeap.lock(indexStart);
	mesh->computeMeshVBHeap(dst, indexStart);
	// unlock only what vertices have changed (ATI problem)
	vbHeapBlock->VBHeap.unlock(indexStart, indexStart+numVertices);
#endif		// todo hulud remove / restore VBHeap
}

// ***************************************************************************
void			CMeshBlockManager::freeMeshVBHeap(IMeshGeom *mesh)
{
	nlassert(mesh->_MeshVBHeapId);

	// unpack heap and mesh id.
	uint	vbHeapId= mesh->_MeshVBHeapId & NL3D_MBM_VBHEAP_HEAP_MASK;
	uint	meshId= (mesh->_MeshVBHeapId & NL3D_MBM_VBHEAP_MESH_MASK) >> NL3D_MBM_VBHEAP_MESH_SHIFT;

	// access to this VBHeap.
	CVBHeapBlock	*vbHeapBlock= _VBHeapBlocks[vbHeapId];

	// free VB memory.
#if 0		// todo hulud remove / restore VBHeap
	vbHeapBlock->VBHeap.free(mesh->_MeshVBHeapIndexStart);
#endif		// todo hulud remove / restore VBHeap

	// free this space
	nlassert(meshId<vbHeapBlock->AllocatedMeshGeoms.size());
	vbHeapBlock->AllocatedMeshGeoms[meshId]= NULL;
	vbHeapBlock->FreeIds.push_back(meshId);

	// reset mesh info.
	mesh->_MeshVBHeapId= 0;
	mesh->_MeshVBHeapIndexStart= 0;
}

// ***************************************************************************
void			CMeshBlockManager::releaseVBHeaps()
{
	uint	i,j;

	// For all blocks but the 0th
	for(j=1; j<_VBHeapBlocks.size();j++)
	{
		CVBHeapBlock	*hb= _VBHeapBlocks[j];

		// For all allocated mesh of this heap.
		for(i=0;i<hb->AllocatedMeshGeoms.size();i++)
		{
			IMeshGeom	*mesh= hb->AllocatedMeshGeoms[i];
			// if the mesh exist.
			if(mesh)
			{
				// free his VBHeap Data.
				freeMeshVBHeap(mesh);
				nlassert( hb->AllocatedMeshGeoms[i] == NULL );
			}
		}

		// delete the block. NB: VBHeap auto released
		delete hb;
	}

	// erase all blocks but 0th
	_VBHeapBlocks.resize(1);

	// clear the map.
	contReset(_VBHeapMap);
}

// ***************************************************************************
bool			CMeshBlockManager::addVBHeap(IDriver *drv, uint vertexFormat, uint maxVertices)
{
	return false;	// todo hulud remove / restore VBHeap

	// if find an existing vertexFormat, abort.
	TVBHeapMap::iterator	it= _VBHeapMap.find(vertexFormat);
	// if found, abort
	if( it!=_VBHeapMap.end() )
		return false;

	// create the block
	CVBHeapBlock	*hb= new CVBHeapBlock;

	// allocate vertex space
#if 0		// todo hulud remove / restore VBHeap
	hb->VBHeap.init(drv, vertexFormat, maxVertices);
#endif		// todo hulud remove / restore VBHeap

	// add an entry to the array, and the map.
	_VBHeapBlocks.push_back(hb);
	_VBHeapMap[vertexFormat]= (uint)_VBHeapBlocks.size()-1;

	return true;
}


} // NL3D
