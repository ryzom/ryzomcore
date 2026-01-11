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

#ifndef NL_MESH_GEOM_H
#define NL_MESH_GEOM_H


#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"


namespace NLMISC
{
	class CAABBoxExt;
}

namespace NL3D
{


class IDriver;
class CTransformShape;
class CMeshBaseInstance;
class CMeshBlockManager;
class CScene;
class CRenderTrav;
using NLMISC::CPlane;
using NLMISC::CMatrix;


// ***************************************************************************
/**
 * A render Context used to render MeshGeom. Contains any useful information
 */
class CMeshGeomRenderContext
{
public:
	IDriver			*Driver;
	CScene			*Scene;
	CRenderTrav		*RenderTrav;
	// true if the mesh is rendered through a VBHeap currently activated in driver.
	bool			RenderThroughVBHeap;
};


// ***************************************************************************
/**
 * Interface for MeshGeom.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class IMeshGeom : public NLMISC::IStreamable
{
public:

	/// Render Flags, used in render
	enum	TRenderFlag
	{
		RenderOpaqueMaterial= 1,		// set when the mesh geom must render opaque material
		RenderTransparentMaterial= 2,	// set when the mesh geom must render transparent material
		RenderPassOpaque=4,				// set when the current traversal rdrPass is the opaque pass
		RenderGlobalAlpha= 8,			// set when the caller wants to draw material with global alpha
		RenderGADisableZWrite= 16,		// only when globalAlpha is used. set if must disable ZWrite
	};

public:

	/// Constructor
	IMeshGeom();
	/// dtor
	virtual ~IMeshGeom();


	/** store useful information for this meshGeom in the instance. Used for IMeshVertexProgram as example
	 */
	virtual	void	initInstance(CMeshBaseInstance *mbi) =0;


	/** clip this shape with a pyramid.
	 * the pyramid is given in world space.The world matrix of the object is given.
	 * \param pyramid the clipping polytope, planes are normalized.
	 * \param worldMatrix the world matrix of the instance.
	 * \return true if the object is visible, false otherwise. The default behavior is to return true (never clipped).
	 */
	virtual bool	clip(const std::vector<CPlane>	&/* pyramid */, const CMatrix &/* worldMatrix */) {return true;}

	/** render() this meshGeom in a driver, with the specified TransformShape instance information.
	 *	NB: the meshGeom is ensured to not be skinned to a skeleton, but CMeshGeom may still have skin information.
	 */
	virtual void	render(IDriver *drv, CTransformShape *trans, float polygonCount, uint32 rdrFlags, float globalAlpha) =0;

	/** render this meshGeom as a skin, with the specified TransformShape instance information (which gives the driver)
	 *	NB: trans->isSkinned() is ensured to be true.
	 *	All the materials must be rendered.
	 */
	virtual void	renderSkin(CTransformShape *trans, float alphaMRM) =0;

	/// True if this mesh has a vertexProgram
	virtual bool	hasMeshVertexProgram() const {return false;}

	/** Profile the render of this meshGeom.
	 */
	virtual void	profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, float polygonCount, uint32 rdrFlags) =0;


	/// \name Load balancing methods
	// @{

	/** get an approximation of the number of triangles this instance will render for a fixed distance.
	  *
	  * \param distance is the distance of the shape from the eye.
	  * \return the approximate number of triangles this instance will render at this distance. This
	  * number can be a float. The function MUST be decreasing or constant with the distance but don't
	  * have to be continus.
	  */
	virtual float	getNumTriangles (float distance) =0;

	/** get the extended axis aligned bounding box of the mesh
	  */
	virtual const NLMISC::CAABBoxExt& getBoundingBox() const =0;

	// @}


	/// \name Mesh Block Render Interface
	/**
	 *	NB: Mesh Block render cannot occurs if the Mesh is Skinned/MeshMorphed.
	 *	NB: Mesh Block render can occurs only in Opaque pass => globalAlpha is not used.
	 */
	// @{

	/** true if this meshGeom support meshBlock rendering.
	 *	eg: return false if skinned/meshMorphed.
	 */
	virtual bool	supportMeshBlockRendering () const =0;

	/** true if the sort criterion must be by material. Else, sort per instance.
	 *
	 */
	virtual bool	sortPerMaterial() const =0;

	/** return the number of renderPasses for this mesh.
	 * Used only if sortPerMaterial()) is true
	 */
	virtual uint	getNumRdrPassesForMesh() const =0;

	/** return the number of renderPasses for this instance. Called after activateInstance()
	 * Used only if sortPerMaterial()) is false
	 */
	virtual uint	getNumRdrPassesForInstance(CMeshBaseInstance *inst) const =0;

	/** The framework call this method when he will render instances of this meshGeom soon.
	 *
	 */
	virtual	void	beginMesh(CMeshGeomRenderContext &rdrCtx) =0;

	/** The framework call this method any time a change of instance occurs.
	 *
	 */
	virtual	void	activeInstance(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount, void *vbDst) =0;

	/** The framework call this method to render the current renderPass, with the current instance
	 *	NB: if the material is blended, DON'T render it!!
	 */
	virtual	void	renderPass(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount, uint rdrPass) =0;

	/** The framework call this method when it has done with this meshGeom
	 *
	 */
	virtual	void	endMesh(CMeshGeomRenderContext &rdrCtx) =0;

	/** The framework call this method to know if the mesh can fit in VBHeap.
	 *	if yes, deriver must return mesh vertexFormat and num of vertices.
	 */
	virtual	bool	getVBHeapInfo(uint &/* vertexFormat */, uint &/* numVertices */) {return false;}

	/** When the framework succes to allocate a VBHeap space, it call this method to fill this space and compute
	 *	shifted Primitive block.
	 *	\param the dest VertexBuffer. NB: start to fill at dst[0]
	 *	\param indexStart used to shift primitive block.
	 */
	virtual	void	computeMeshVBHeap(void * /* dst */, uint /* indexStart */)  {}

	/** Return true if the meshGeom has to Fill some Vertices at activeInstance() time
	 *	if VBHeap enabled at this time, then vbDst in activeInstance(,,,vbDst) will contains the vb to write to.
	 */
	virtual	bool	isActiveInstanceNeedVBFill() const {return false;}

	// Return True if the mesh fit in a VBHeap
	bool			isMeshInVBHeap() const {return _MeshVBHeapId!=0;}

	// @}


// *****************
private:

	/// \name Mesh Block Render access
	// @{
	friend class CMeshBlockManager;

	/// This is the head of the list of instances to render in the CMeshBlockManager. -1 if NULL
	sint32				_RootInstanceId;

	/// The manager which owns our VBHeap data. NULL means manager must try to setup VBHeap
	CMeshBlockManager	*_MeshBlockManager;
	/// This is the Heap Id setuped in CMeshBlockManager::allocateMeshVBHeap()
	uint				_MeshVBHeapId;
	/// Delta of index for mesh into VBHeap.
	uint				_MeshVBHeapIndexStart;
	/// Number of vertices for mesh into VBHeap.
	uint				_MeshVBHeapNumVertices;
	// @}

};


} // NL3D


#endif // NL_MESH_GEOM_H

/* End of mesh_geom.h */
