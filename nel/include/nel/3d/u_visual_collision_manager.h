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

#ifndef NL_U_VISUAL_COLLISION_MANAGER_H
#define NL_U_VISUAL_COLLISION_MANAGER_H

#include "nel/misc/types_nl.h"

#include <vector>

#include "nel/misc/vector.h"
#include "u_visual_collision_mesh.h"

namespace NLMISC
{
	class CAABBox;
}

namespace NL3D
{


class	ULandscape;
class	UVisualCollisionEntity;
class	UVisualCollisionMesh;



// ***************************************************************************
/**
 * Interface to visual collision manager.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UVisualCollisionManager
{
protected:
	UVisualCollisionManager() {}
	virtual ~UVisualCollisionManager() {}


public:
	// Infos about a collision mesh instance
	class CMeshInstanceColInfo
	{
	public:
		UVisualCollisionMesh  Mesh;
		const NLMISC::CMatrix *WorldMatrix;
		const NLMISC::CAABBox *WorldBBox;
		uint ID;
	};

	/** setup the landscape used for this collision manager. ptr is kept, and manager must be cleared
	 * when the landscape is deleted.
	 */
	virtual void					setLandscape(ULandscape *landscape) =0;


	/** create an entity. NB: CVisualCollisionManager owns this ptr, and you must delete it with deleteEntity().
	 * NB: all CVisualCollisionEntity are deleted when this manager is deleted.
	 */
	virtual UVisualCollisionEntity	*createEntity() =0;

	/** delete an entity.
	 */
	virtual void					deleteEntity(UVisualCollisionEntity	*entity) =0;


	/**
	 *  Build a lighting table to remap sun contribution from landscape to sun contribution for objects.
	 *  The value remap the landscape sun contribution (0 ~ 1) to an object sun contribution (0 ~1)
	 *  using the following formula:
	 *
	 *  objectSunContribution = min ( powf ( landscapeSunContribution / maxThreshold, power ), 1 );
	 *
	 *	Default is 0.5 (=> sqrt) for power and 0.5 for maxThreshold.
	 */
	virtual void					setSunContributionPower (float power, float maxThreshold) =0;


	/** Inform the VisualCollisionManager if the player is "inside" or "outside".
	 *	set it to true if the player is not on Landscape.
	 *	This is a tricky flag used for the IBBR problem: this is an issue with clusters and
	 *	"interior building that can be bigger than reality"
	 *	It is used at getCameraCollision(), and receiveShadowMap() time
	 */
	virtual void					setPlayerInside(bool state) =0;


	/** Get Typical Camera 3rd person collision.
	 *	For landscape, it is done only against TileFaces (ie only under approx 50 m)
	 *	return a [0,1] value. 0 => collision at start. 1 => no collision.
	 *	\param radius is the radius of the 'cylinder'
	 *	\param cone if true, the object tested is a cone (radius goes to end)
	 */
	virtual float					getCameraCollision(const NLMISC::CVector &start, const NLMISC::CVector &end, float radius, bool cone) =0;

	/** Get a Ray collision.
	 *	For landscape, it is done only against TileFaces (ie only under approx 50 m)
	 *	\return true if some collision found
	 */
	virtual bool					getRayCollision(const NLMISC::CVector &start, const NLMISC::CVector &end, bool landscapeOnly= false) =0;

	/** Add a Mesh to the collision manager. For now it is used only for Camera Collision
	 *	\param mesh the collision mesh (keep a refptr on it)
	 *	\param instanceMatrix the matrix instance to apply to this mesh
	 *	\param avoidCollisionWhenInside special flag for the IBBR problem. if true this collision instance won't be tested if the player is "inside"
	 *	\param avoidCollisionWhenOutside special flag for the IBBR problem. if true this collision instance won't be tested if the player is "outside"
	 *	\return the id used for remove, 0 if not succeed
	 */
	virtual uint					addMeshInstanceCollision(const UVisualCollisionMesh &mesh, const NLMISC::CMatrix &instanceMatrix, bool avoidCollisionWhenInside, bool avoidCollisionWhenOutside) =0;
	/** Remove a Mesh from the collision manager.
	 */
	virtual void					removeMeshCollision(uint id) =0;
	// retrieve mesh that are in the given box
	virtual	void					getMeshs(const NLMISC::CAABBox &aabbox, std::vector<CMeshInstanceColInfo> &dest) = 0;
};


} // NL3D


#endif // NL_U_VISUAL_COLLISION_MANAGER_H

/* End of u_visual_collision_manager.h */
