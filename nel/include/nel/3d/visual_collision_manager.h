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

#ifndef NL_VISUAL_COLLISION_MANAGER_H
#define NL_VISUAL_COLLISION_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/block_memory.h"
#include "patch.h"
#include "landscape.h"
#include "quad_grid.h"
#include "visual_collision_mesh.h"


namespace NL3D
{


class CVisualCollisionEntity;
class CLandscapeCollisionGrid;
class IDriver;
class CShadowMap;
class CShadowMapProjector;
class CMaterial;


// ***************************************************************************
/**
 * Server to Client collision manager. Snap logic position to Visual position.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CVisualTileDescNode
{
public:
	/// If od the patchblock containing this tile.
	uint16		PatchQuadBlocId;
	/// id in this patchblock of the tile.
	uint16		QuadId;
	/// for the quadgrid of chain list of CVisualTileDescNode.
	CVisualTileDescNode		*Next;
};


// ***************************************************************************
/**
 * Server to Client collision manager. It is used for multiple purpose:
 *	- Snap logic entities position to Visual position (snap on landscape for instance).
 *	- Get precise camera collision with mesh instances
 *	- Used for ShadowMap receiving
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVisualCollisionManager
{
public:
	class CMeshInstanceColInfo
	{
	public:
		CVisualCollisionMesh  *Mesh;
		const NLMISC::CMatrix *WorldMatrix;
		const NLMISC::CAABBox *WorldBBox;
		uint ID;
	};

	/// Constructor
	CVisualCollisionManager();
	~CVisualCollisionManager();


	/** setup the landscape used for this collision manager. ptr is kept, and manager and all entities must be cleared
	 * when the landscape is deleted.
	 */
	void						setLandscape(CLandscape *landscape);


	/** create an entity. NB: CVisualCollisionManager do not owns this ptr for now, and you must delete it with deleteEntity().
	 * NB: CVisualCollisionEntity are no more valid when this manager is deleted.
	 */
	CVisualCollisionEntity		*createEntity();

	/** delete an entity.
	 */
	void						deleteEntity(CVisualCollisionEntity	*entity);


	/** for CVisualCollisionEntity::getStaticLightSetup().
	 *  \see CVisualCollisionEntity::getStaticLightSetup()
	 *
	 *  Build a lighting table to remap sun contribution from landscape to sun contribution for objects.
	 *  The value remap the landscape sun contribution (0 ~ 1) to an object sun contribution (0 ~1)
	 *  using the following formula:
	 *
	 *  objectSunContribution = min ( powf ( landscapeSunContribution / maxThreshold, power ), 1 );
	 *
	 *	Default is 0.5 (=> sqrt) for power and 0.5 for maxThreshold.
	 */
	void						setSunContributionPower(float power, float maxThreshold);


	/** Inform the VisualCollisionManager if the player is "inside" or "outside".
	 *	set it to true if the player is not on Landscape.
	 *	This is a tricky flag used for the IBBR problem: this is an issue with clusters and
	 *	"interior building that can be bigger than reality"
	 *	It is used at getCameraCollision(), and receiveShadowMap() time
	 */
	void						setPlayerInside(bool state);


	/** Get Typical Camera 3rd person collision.
	 *	For landscape, it is done only against TileFaces (ie only under approx 50 m)
	 *	return a [0,1] value. 0 => collision at start. 1 => no collision.
	 *	\param radius is the radius of the 'cylinder'
	 *	\param cone if true, the object tested is a cone (radius goes to end)
	 */
	float						getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone);

	/** Get a Ray collision.
	 *	For landscape, it is done only against TileFaces (ie only under approx 50 m)
	 *	\return true if some collision found
	 */
	bool						getRayCollision(const NLMISC::CVector &start, const NLMISC::CVector &end, bool landscapeOnly= false);

	/** Add a Mesh Instance to the collision manager. For now it is used only for Camera Collision
	 *	\param mesh the collision mesh (keep a refptr on it)
	 *	\param instanceMatrix the matrix instance to apply to this mesh
	 *	\param avoidCollisionWhenInside special flag for the IBBR problem. if true this collision instance won't be tested if the player is "inside"
	 *	\param avoidCollisionWhenOutside special flag for the IBBR problem. if true this collision instance won't be tested if the player is "outside"
	 *	\return the id used for remove, 0 if not succeed
	 */
	uint						addMeshInstanceCollision(CVisualCollisionMesh *mesh, const CMatrix &instanceMatrix, bool avoidCollisionWhenInside, bool avoidCollisionWhenOutside);

	/** Remove a Mesh from the collision manager.
	 */
	void						removeMeshCollision(uint id);


	/** Use the MeshInstance Collision to rended a ShadowMap on them.
	 *	NB: only the minimum faces touched by the shadowmap are rendered
	 *	NB: the MeshInstance Collision use a fake lighting to avoid backface shadowing.
	 *		- driver light setup must be reseted
	 *		- light0 must be enabled
	 *		- light0 setup is modified internally
	 *	\see CTransform::receiveShadowMap() for more information
	 */
	void						receiveShadowMap(IDriver *drv, CShadowMap *shadowMap, const CVector &casterPos, CMaterial &shadowMat, CShadowMapProjector &smp);

	// Get collision meshs instances that are inside the given box
	void						getMeshs(const NLMISC::CAABBox &aabbox, std::vector<CMeshInstanceColInfo> &dest);

// ***************************
private:
	/// The landscape used to generate tiles, and to snap position to tesselated ground.
	NLMISC::CRefPtr<CLandscape>			_Landscape;

	/// Allocators.
	CBlockMemory<CVisualTileDescNode>	_TileDescNodeAllocator;
	CBlockMemory<CPatchQuadBlock>		_PatchQuadBlockAllocator;

	// An instance of a Mesh collision
	class CMeshInstanceCol
	{
	public:
		// A ref to the visual col mesh
		CRefPtr<CVisualCollisionMesh>		Mesh;
		// The Matrix to apply to the mesh to get instance
		CMatrix								WorldMatrix;
		// The World BBox
		NLMISC::CAABBox						WorldBBox;
		// The pos in Mesh Instance quadgrid
		CQuadGrid<CMeshInstanceCol*>::CIterator	QuadGridIt;
		// see addMeshInstanceCollision for those special flags
		bool								AvoidCollisionWhenPlayerInside;
		bool								AvoidCollisionWhenPlayerOutside;
		uint								ID;
	public:
		/// get collision with camera. [0,1] value
		float		getCameraCollision(class CCameraCol &camCol);

		/// receive a shadow map
		void		receiveShadowMap(const CVisualCollisionMesh::CShadowContext &shadowContext);
	};

	// The map of Meshes Instance
	typedef	std::map<uint32, CMeshInstanceCol>	TMeshColMap;
	TMeshColMap						_Meshs;
	// The QuadGrid of Collision meshs
	CQuadGrid<CMeshInstanceCol*>	_MeshQuadGrid;
	// The pool of id.
	uint32							_MeshIdPool;


private:
	friend class CVisualCollisionEntity;
	friend class CLandscapeCollisionGrid;

	// Allocators.
	CVisualTileDescNode			*newVisualTileDescNode();
	void						deleteVisualTileDescNode(CVisualTileDescNode *ptr);

	CPatchQuadBlock				*newPatchQuadBlock();
	void						deletePatchQuadBlock(CPatchQuadBlock *ptr);


	// setSunContributionPower Table.
	uint8						_SunContributionLUT[256];


	// PlayerInside state
	bool						_PlayerInside;


	// Same IndexBuffer filled at each CVisalCollisionMesh::receiveShadowMap()
	CIndexBuffer				_ShadowIndexBuffer;
};


} // NL3D


#endif // NL_VISUAL_COLLISION_MANAGER_H

/* End of visual_collision_manager.h */
