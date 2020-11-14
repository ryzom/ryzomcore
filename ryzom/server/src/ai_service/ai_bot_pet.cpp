// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "ai_bot_pet.h"
#include "visual_properties_interface.h"

#include "nel/misc/random.h"
#include "ai_grp_pet.h"
#include "owners.h"

using namespace NLMISC;
using namespace std;
using namespace RYAI_MAP_CRUNCH;

//////////////////////////////////////////////////////////////////////////////
// CBotPet                                                                  //
//////////////////////////////////////////////////////////////////////////////

void CBotPet::getSpawnPos(CAIVector& triedPos, RYAI_MAP_CRUNCH::CWorldPosition& spawnPos, RYAI_MAP_CRUNCH::CWorldMap const& worldMap, CAngle& spawnTheta)
{
	//	return can be false.
	worldMap.setWorldPosition(_SpawnPos.h(), spawnPos, _SpawnPos);
	spawnTheta = _SpawnPos.theta();
	triedPos = _SpawnPos;
}

CGrpPet& CBotPet::getPetGroup()
{
	return *static_cast<CGrpPet*>(getOwner());
}

void CBotPet::update(uint32 ticks, CAIEntityPhysical const* petOwner)
{
	CSpawnBotPet* const spawnPet = getSpawn();
	if (spawnPet)
	{
		if (spawnPet->isAlive())
		{
			if (spawnPet->isMounted())
				spawnPet->updatePos(); // take position from GPMS
			else
				spawnPet->updateProfile(ticks);	// update pathfinding profile
		}
	}
}

void CBotPet::triggerSetSheet(AISHEETS::ICreatureCPtr const& sheet)
{
	nlwarning("Changing the sheet of a pet bot is not currently possible"); // The guy that wrote that is a lazy bastard...
}

//////////////////////////////////////////////////////////////////////////////
// CSpawnBotPet                                                             //
//////////////////////////////////////////////////////////////////////////////

/// Return true if the animal is mounted (thus controlled by the GPMS)
bool CSpawnBotPet::isMounted() const
{
	// Do not control a pet when in mount mode (controlled by the GPMS)
	CMirrorPropValueRO<MBEHAV::TMode> currentMode( TheDataset, dataSetRow(), DSPropertyMODE );
	return currentMode().isMountMode();
}

/// Take position from mirror
void CSpawnBotPet::updatePos()
{
	RYAI_MAP_CRUNCH::CWorldPosition wpos;
	if (!CWorldContainer::getWorldMap().setWorldPosition(pos().h(), wpos, CAIVector(pos())))
		return;
	setWPos(wpos);
	
	if (wpos.getFlags() & RYAI_MAP_CRUNCH::Water)
		setActionFlags(RYZOMACTIONFLAGS::InWater);
	else
		removeActionFlags(RYZOMACTIONFLAGS::InWater);
}

CSpawnGroupPet& CSpawnBotPet::spawnGrp()
{
	return static_cast<CSpawnGroupPet&>(CSpawnBot::spawnGrp());
}

void CSpawnBotPet::setVisualPropertiesName()
{
	CBotPet& botRef = CSpawnBotPet::getPersistent();
	ucstring name = botRef.getName();
	
	if (CVisualPropertiesInterface::UseIdForName)
	{
		name = NLMISC::toString("AI:%s", botRef.getIndexString().c_str());
	}
	
	if (name.empty() && CVisualPropertiesInterface::ForceNames)
	{
		name = NLMISC::CFile::getFilenameWithoutExtension(botRef.getSheet()->SheetId().toString().c_str());
	}
	
	if (!botRef.getCustomName().empty())
		name = botRef.getCustomName();

	//	no name the bot will appear without name on the client.
	if (name.empty())
		return;
		
	CVisualPropertiesInterface::setName(dataSetRow(), name);
}
