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

#include "nel/3d/u_water.h"
#include "nel/3d/water_pool_manager.h"
#include "nel/3d/water_height_map.h"
#include "nel/3d/water_model.h"
#include "nel/3d/driver_user.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

//===========================================================================
UWaterHeightMap &UWaterHeightMapManager::getWaterHeightMapFromID(uint32 ID)
{
	nlassert(GetWaterPoolManager().hasPool(ID)); // unknown pool ID!
	return  GetWaterPoolManager().getPoolByID(ID);
}


//===========================================================================
void	UWaterHeightMapManager::setBlendFactor(UDriver *drv, float value)
{
	NLMISC::clamp(value, 0.f, 1.f);
	GetWaterPoolManager().setBlendFactor(NLMISC::safe_cast<CDriverUser *>(drv)->getDriver(), value);
}

//===========================================================================
void UWaterHeightMapManager::releaseBlendTextures()
{
	GetWaterPoolManager().releaseBlendTextures();
}

//===========================================================================
uint32	UWaterInstance::getWaterHeightMapID() const
{
	CWaterModel	*object = getObjectPtr();
	return object ->getWaterHeightMapID();
}


//===========================================================================
float	UWaterInstance::getHeightFactor() const
{
	CWaterModel	*object = getObjectPtr();
	return object->getHeightFactor();
}

//===========================================================================
float   UWaterInstance::getHeight(const NLMISC::CVector2f &pos)
{
	CWaterModel	*object = getObjectPtr();
	return object->getHeight(pos);
}

//===========================================================================
float   UWaterInstance::getAttenuatedHeight(const NLMISC::CVector2f &pos, const NLMISC::CVector &viewer)
{
	CWaterModel	*object = getObjectPtr();
	return object->getAttenuatedHeight(pos, viewer);
}

} // NL3D
