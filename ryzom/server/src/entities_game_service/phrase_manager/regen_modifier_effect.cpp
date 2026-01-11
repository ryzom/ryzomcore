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
#include "regen_modifier_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "entity_manager/entity_base.h"
#include "entity_manager/entity_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//--------------------------------------------------------------
//		CRegenModifierEffect::update()
//--------------------------------------------------------------
bool CRegenModifierEffect::update(CTimerEvent * event, bool applyEffect)
{
	if (_AffectedScore >= SCORES::NUM_SCORES)
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	if (!entity)
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	if (applyEffect)
	{
		_RegenModifier = (-1.0f) * entity->getScores()._PhysicalScores[_AffectedScore].Base * _DebuffFactor;
		
		entity->getScores()._PhysicalScores[_AffectedScore].RegenerateModifier = _RegenModifier + entity->getScores()._PhysicalScores[_AffectedScore].RegenerateModifier;
	}
	
	// set next event
	/*if (entity->getId().getType() == RYZOMID::player)
	{
		// for player, set value every ticks
		_UpdateTimer.setRemaining(1, event);
	}
	else
	{
		// creature : only modify value once
	}
	*/

	return false;	
} // update //

//--------------------------------------------------------------
//		CRegenModifierEffect::removed()
//--------------------------------------------------------------
void CRegenModifierEffect::removed()
{
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	if (!entity)
		return;

	// if target isn't a player, reset the modifier
	if (/*entity->getId().getType() != RYZOMID::player &&*/ _AffectedScore < SCORES::NUM_SCORES)
	{
		entity->getScores()._PhysicalScores[_AffectedScore].RegenerateModifier = entity->getScores()._PhysicalScores[_AffectedScore].RegenerateModifier - _RegenModifier;
	}

	// if any other effect of the same family still exists on the target then returns without sending msg
	const std::vector<CSEffectPtr>& effects = entity->getSEffects();
	for (uint i = 0 ; i < effects.size() ; ++i)
	{
		if (effects[i] && effects[i] != this && effects[i]->getFamily() == _Family)
			return;
	}

	// send messages to clients
	PHRASE_UTILITIES::sendEffectStandardEndMessages(_CreatorRowId, _TargetRowId, _EffectName);
} // removed //

