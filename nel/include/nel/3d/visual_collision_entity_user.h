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

#ifndef NL_VISUAL_COLLISION_ENTITY_USER_H
#define NL_VISUAL_COLLISION_ENTITY_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_visual_collision_entity.h"
#include "nel/3d/visual_collision_entity.h"
#include "nel/3d/visual_collision_manager.h"


namespace NL3D
{


/**
 * UVisualCollisionEntity implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVisualCollisionEntityUser : public UVisualCollisionEntity
{
public:

	/// Constructor. create entity.
	CVisualCollisionEntityUser(CVisualCollisionManager *manager)
	{
		_Manager= manager;
		_Entity= _Manager->createEntity();
	}
	/// dtor, delete the entity.
	~CVisualCollisionEntityUser() NL_OVERRIDE
	{
		_Manager->deleteEntity(_Entity);
	}


	virtual bool	snapToGround(CVector &pos) NL_OVERRIDE;
	virtual bool	snapToGround(CVector &pos, CVector &normal) NL_OVERRIDE;



	virtual void	setGroundMode(bool groundMode) NL_OVERRIDE;
	virtual void	setCeilMode(bool ceilMode) NL_OVERRIDE;
	virtual bool	getGroundMode() const NL_OVERRIDE;
	virtual bool	getCeilMode() const NL_OVERRIDE;


	virtual void	setSnapToRenderedTesselation(bool snapMode) NL_OVERRIDE;
	virtual bool	getSnapToRenderedTesselation() const NL_OVERRIDE;

	virtual bool	getSurfaceInfo(const CVector &pos, CSurfaceInfo &surfaceInfo) NL_OVERRIDE;

	virtual bool	getStaticLightSetup(NLMISC::CRGBA sunAmbient, const CVector &pos, std::vector<CPointLightInfluence> &pointLightList,
		uint8 &sunContribution, NLMISC::CRGBA &localAmbient) NL_OVERRIDE;


	virtual void	displayDebugGrid(UDriver &drv) const NL_OVERRIDE;


private:
	CVisualCollisionManager		*_Manager;
	CVisualCollisionEntity		*_Entity;

};


} // NL3D


#endif // NL_VISUAL_COLLISION_ENTITY_USER_H

/* End of visual_collision_entity_user.h */
