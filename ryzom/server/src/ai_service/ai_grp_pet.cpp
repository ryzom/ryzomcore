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
#include "ai_grp_pet.h"
#include "owners.h"
#include "ai_share/ai_types.h"

#include "ai_profile_pet.h"	// for CAIPetProfileStand

using namespace NLMISC;
using namespace AITYPES;

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroupPet                                                           //
//////////////////////////////////////////////////////////////////////////////

void CSpawnGroupPet::update	()
{
	FOREACH(it, CCont<CBot>, bots())
	{
		CBotPet* const botPet = safe_cast<CBotPet*>(*it);
		if (!botPet->isSpawned() || botPet->haveToDespawn()) // must erase this bot.
			getPersistent().bots().removeChildByIndex(botPet->getChildIndex());
	}
	
	CEntityId const& entityId = getPersistent().getPetOwner();
	CAIEntityPhysical* const petOwner = CAIS::instance().getEntityPhysical(CMirrors::DataSet->getDataSetRow(entityId));
	
	//	Quick hack to prevent of too much computing..
	if (petOwner)
	{			
		double const distContDestToRealDest = petOwner->wpos().toAIVector().quickDistTo(_PathCont.getDestination());
		
		if (distContDestToRealDest>4)	// update only each 4 meters.
			_PathCont.setDestination(petOwner->wpos());
		_IsPlayerSpawned = true;
	}
	else
	{
		if (_IsPlayerSpawned)
		{
			FOREACH(it, CCont<CBot>, bots())
			{
				CBotPet	*const	pet=safe_cast<CBotPet*>(*it);
				if (pet->isSpawned())
				{
					pet->getSpawn()->setAIProfile(new	CAIPetProfileStand(pet->getSpawn()));
				}
			}
			_IsPlayerSpawned = false;
		}
	}
	
	{
		uint32 const newTime = CTimeInterface::gameCycle();
		uint32 const dt = newTime - _LastUpdate;
		
		FOREACH(it, CCont<CBot>, bots())
		{
			(safe_cast<CBotPet*>(*it))->update(dt, petOwner);
		}
		_LastUpdate = newTime; // record the tick at which we ran this update (for future refference)
	}
}

//////////////////////////////////////////////////////////////////////////////
// CGrpPet                                                                  //
//////////////////////////////////////////////////////////////////////////////

CAIS::CCounter& CGrpPet::getSpawnCounter()
{
	return CAIS::instance()._PetBotCounter;
}
