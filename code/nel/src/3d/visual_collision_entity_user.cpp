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

#include "nel/3d/visual_collision_entity_user.h"
#include "nel/3d/driver_user.h"
#include "nel/misc/hierarchical_timer.h"


namespace NL3D
{

H_AUTO_DECL( NL3D_UI_VisualCollisionEntity )
H_AUTO_DECL( NL3D_Misc_VisualCollisionEntity_Snap )
H_AUTO_DECL( NL3D_Misc_VisualCollisionEntity_GetLight )

#define	NL3D_HAUTO_UI_VCE					H_AUTO_USE( NL3D_UI_VisualCollisionEntity )
#define	NL3D_HAUTO_SNAP_VCE					H_AUTO_USE( NL3D_Misc_VisualCollisionEntity_Snap )
#define	NL3D_HAUTO_LIGHT_VCE				H_AUTO_USE( NL3D_Misc_VisualCollisionEntity_GetLight )


// ****************************************************************************
bool	CVisualCollisionEntityUser::snapToGround(CVector &pos)
{
	NL3D_HAUTO_SNAP_VCE;

	return _Entity->snapToGround(pos);
}
bool	CVisualCollisionEntityUser::snapToGround(CVector &pos, CVector &normal)
{
	NL3D_HAUTO_SNAP_VCE;

	return _Entity->snapToGround(pos, normal);
}



void	CVisualCollisionEntityUser::setGroundMode(bool groundMode)
{
	NL3D_HAUTO_UI_VCE;

	_Entity->setGroundMode(groundMode);
}
void	CVisualCollisionEntityUser::setCeilMode(bool ceilMode)
{
	NL3D_HAUTO_UI_VCE;

	_Entity->setCeilMode(ceilMode);
}
bool	CVisualCollisionEntityUser::getGroundMode() const
{
	NL3D_HAUTO_UI_VCE;

	return _Entity->getGroundMode();
}
bool	CVisualCollisionEntityUser::getCeilMode() const
{
	NL3D_HAUTO_UI_VCE;

	return _Entity->getCeilMode();
}


void	CVisualCollisionEntityUser::setSnapToRenderedTesselation(bool snapMode)
{
	NL3D_HAUTO_UI_VCE;

	_Entity->setSnapToRenderedTesselation(snapMode);
}
bool	CVisualCollisionEntityUser::getSnapToRenderedTesselation() const
{
	NL3D_HAUTO_UI_VCE;

	return _Entity->getSnapToRenderedTesselation();
}


bool	CVisualCollisionEntityUser::getStaticLightSetup(NLMISC::CRGBA sunAmbient, const CVector &pos, std::vector<CPointLightInfluence> &pointLightList,
	uint8 &sunContribution, NLMISC::CRGBA &localAmbient)
{
	NL3D_HAUTO_LIGHT_VCE;

	return _Entity->getStaticLightSetup(sunAmbient, pos, pointLightList, sunContribution, localAmbient);
}

void	CVisualCollisionEntityUser::displayDebugGrid(UDriver &drv) const
{
	CDriverUser		&drvUser= static_cast<CDriverUser&>(drv);
	_Entity->displayDebugGrid(*drvUser.getDriver());
}

bool	CVisualCollisionEntityUser::getSurfaceInfo(const CVector &pos, CSurfaceInfo &surfaceInfo)
{
	return _Entity->getSurfaceInfo (pos, surfaceInfo);
}


} // NL3D
