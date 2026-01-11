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

#ifndef NL_VISUAL_COLLISION_MANAGER_USER_H
#define NL_VISUAL_COLLISION_MANAGER_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_visual_collision_manager.h"
#include "visual_collision_manager.h"
#include "landscape_user.h"
#include "visual_collision_entity_user.h"
#include "ptr_set.h"


namespace NL3D {


/**
 * UVisualCollisionManager implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVisualCollisionManagerUser : public UVisualCollisionManager
{
public:

	/// Constructor
	CVisualCollisionManagerUser()
	{
	}


	virtual void					setLandscape(ULandscape *landscape)
	{
		_Manager.setLandscape(&(dynamic_cast<CLandscapeUser*>(landscape)->getLandscape()->Landscape));
	}

	virtual UVisualCollisionEntity	*createEntity()
	{
		return _Entities.insert(new CVisualCollisionEntityUser(&_Manager));
	}

	virtual void					deleteEntity(UVisualCollisionEntity	*entity)
	{
		_Entities.erase(dynamic_cast<CVisualCollisionEntityUser*>(entity));
	}

	virtual void					setSunContributionPower (float power, float maxThreshold)
	{
		_Manager.setSunContributionPower (power, maxThreshold);
	}

	virtual void					setPlayerInside(bool state)
	{
		_Manager.setPlayerInside(state);
	}

	virtual float					getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone)
	{
		return _Manager.getCameraCollision (start, end, radius, cone);
	}

	virtual bool					getRayCollision(const NLMISC::CVector &start, const NLMISC::CVector &end, bool landscapeOnly)
	{
		return _Manager.getRayCollision (start, end, landscapeOnly);
	}

	virtual uint					addMeshInstanceCollision(const UVisualCollisionMesh &mesh, const NLMISC::CMatrix &instanceMatrix, bool avoidCollisionWhenInside, bool avoidCollisionWhenOutside);

	virtual void					removeMeshCollision(uint id)
	{
		_Manager.removeMeshCollision(id);
	}
	virtual	void					getMeshs(const NLMISC::CAABBox &aabbox, std::vector<CMeshInstanceColInfo> &dest);


public:
	CVisualCollisionManager		&getVCM() {return _Manager;}

protected:
	CVisualCollisionManager		_Manager;
	typedef	CPtrSet<CVisualCollisionEntityUser>		TVisualCollisionEntitySet;
	TVisualCollisionEntitySet	_Entities;

};


} // NL3D


#endif // NL_VISUAL_COLLISION_MANAGER_USER_H

/* End of visual_collision_manager_user.h */
