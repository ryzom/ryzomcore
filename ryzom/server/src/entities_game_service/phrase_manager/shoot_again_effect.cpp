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
// net
#include "nel/net/message.h"
//
#include "nel/misc/string_conversion.h"

#include "shoot_again_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "entity_manager/entity_base.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager PlayerManager;


//----------------------------------------------------------------------------

bool CShootAgainEffect::update(CTimerEvent* event, bool applyEffect)
{
	
	// if needed check if caster is dead
	const CEntityBase *caster = CEntityBaseManager::getEntityBasePtr(_CreatorRowId);
	if ( !caster || caster->isDead())
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}
	
	CEntityBase	*targetEntity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	if (targetEntity == NULL)
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}
	
	// set timer next event
	_UpdateTimer.setRemaining(/*_CycleLength*/20, event);
	
	return false;
}

//----------------------------------------------------------------------------

void CShootAgainEffect::removed()
{
	CEntityBase	*targetEntity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	if (targetEntity == NULL)
	{
		return;
	}

	// if entity is dead, do not send messages
	if (targetEntity->isDead())
		return;

	DEBUGLOG("EFFECT: Shoot again effect ends on entity %s", targetEntity->getId().toString().c_str());
}

NLMISC::CSheetId CShootAgainEffect::getAssociatedSheetId() const
{
	return NLMISC::CSheetId("shoot_again.sbrick");
}
