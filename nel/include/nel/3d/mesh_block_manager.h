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

#ifndef NL_MESH_BLOCK_MANAGER_H
#define NL_MESH_BLOCK_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mesh_geom.h"
#include "nel/3d/vertex_buffer_heap.h"


namespace NL3D
{


class	CMeshBaseInstance;
class	IDriver;
class	CScene;
class	CRenderTrav;

// ***************************************************************************
/**
 * A class used to render instances sorted by MeshGeom first, then per material, where possible.
 *	This allow optimisation because less renderState swapping are needed.
 *	WARNING: if you add MeshGeom to 2 different CMeshBlockManager at same times, it won't work, and will
 *	certainly crashes (not checked/assert).
 *
 *	NB VBHeap part works, even if no User interface use it. It don't give greate performance Add, but will may be used.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CMeshBlockManager
{
public:

	/// Constructor
	CMeshBlockManager();
	~CMeshBlockManager();

	/** Add an instance of a MeshGeom to render. Only CMeshBaseInstance can be added.
	 *	For now, only CMeshGeom and CMeshMRMGeom are known to work.
	 */
	void			addInstance(IMeshGeom *meshGeom, CMeshBaseInstance *inst, float polygonCount);

	/** Flush the manager and effectively render.
	 */
	void			flush(IDriver *drv, CScene *scene, CRenderTrav *renderTrav);


	/// \name VBHeap part.
	// @{

	/// release all Heaps => clear memory of meshs registered.
	void			releaseVBHeaps();

	/** Add a Heap for a given vertexFormat. Any meshGeom added with addInstance() which has this vertex Format
	 *	may fit in this heap.
	 *	return false and fail if the heap can't be allocated or if the heap with same vertexFormat still exist.
	 */
	bool			addVBHeap(IDriver *drv, uint vertexFormat, uint maxVertices);

	/// Called by ~IMeshGeom()
	void			freeMeshVBHeap(IMeshGeom *mesh);

	// @}

// ************************
private:

	// An instance information.
	struct	CInstanceInfo
	{
		IMeshGeom			*MeshGeom;
		CMeshBaseInstance	*MBI;
		float				PolygonCount;
		// Next instance to render in the list. -1 if end of list.
		sint32				NextInstance;
	};

	// A VBHeap information.
	struct	CVBHeapBlock
	{
		/// List of instances. small realloc are performed, since same vector used each frame.
		std::vector<CInstanceInfo>	RdrInstances;

		/// List of MeshGeom. small realloc are performed, since same vector used each frame.
		std::vector<IMeshGeom*>		RdrMeshGeoms;

#if 0		// todo hulud remove / restore VBHeap
		/// The actual VertexBufferHeap
		CVertexBufferHeap			VBHeap;
#endif		// todo hulud remove / restore VBHeap
		/// List of MeshGeom to clear VBHeap info.
		std::vector<IMeshGeom*>		AllocatedMeshGeoms;
		/// List of Id free in AllocatedMeshGeoms
		std::vector<uint>			FreeIds;
	};

	/// Heap Map from vertexFormat to VBHeap Id. NB: do not contains 0th.
	typedef	std::map<uint, uint>	TVBHeapMap;
	TVBHeapMap						_VBHeapMap;

	/** List of Heaps.
	 *	NB: 0th heap is special: contains all meshs which can't fit in any VBHeap.
	 */
	std::vector<CVBHeapBlock*>		_VBHeapBlocks;

	/// Try to allocate a MeshGeom into a specific Heap.
	void			allocateMeshVBHeap(IMeshGeom *mesh);

	// render all instance of this meshGeoms, sorting by material, VP etc....
	void			render(CVBHeapBlock	*hb, IMeshGeom *meshGeom, std::vector<CInstanceInfo>	&rdrInstances);

	// current flush setup
	CMeshGeomRenderContext		_RenderCtx;
};


} // NL3D


#endif // NL_MESH_BLOCK_MANAGER_H

/* End of mesh_block_manager.h */
