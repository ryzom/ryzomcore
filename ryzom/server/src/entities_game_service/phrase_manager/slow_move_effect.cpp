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
#include "slow_move_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "phrase_manager/effect_factory.h"
#include "entity_manager/entity_base.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//		CSlowMoveEffect::isTimeToUpdate()
//--------------------------------------------------------------
bool CSlowMoveEffect::isTimeToUpdate()
{
	return true;
} // isTimeToUpdate //

//--------------------------------------------------------------
//		CSlowMoveEffect::update()
//--------------------------------------------------------------
bool CSlowMoveEffect::update( uint32 & updateFlag )
{
	if (!_SlowedEntity)
	{
		_SlowedEntity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
		if (!_SlowedEntity)
			return true;
	}

	// for players, add the value each tick, and doesn't reset it at the end
	if (_SlowedEntity->getId().getType() == RYZOMID::player)
	{
		_SlowedEntity->getPhysScores().SpeedVariationModifier += _Value;
	}
	// if entity is a creature, only apply the effect at the beginning, and remove it at the end
	else if (_FirstUpdate)
	{
		_SlowedEntity->getPhysScores().SpeedVariationModifier += _Value;
		_FirstUpdate = false;
	}

	// set flag, only the strongest slow effect is applied
	updateFlag |= _Family;
	
	return (_EndDate <= CTickEventHandler::getGameCycle());
} // update //

//--------------------------------------------------------------
//		CSlowMoveEffect::removed()
//--------------------------------------------------------------
void CSlowMoveEffect::removed()
{
	if (!_SlowedEntity) return;

	// returns if entity already dead
	if (_SlowedEntity->isDead())
		return;

	DEBUGLOG("EFFECT: slow move effect (value %d) ends on entity %s", _Value, _SlowedEntity->getId().toString().c_str());
	
	// restore entity speed (for creatures only)
	if (_SlowedEntity->getId().getType() != RYZOMID::player)
	{
		_SlowedEntity->getPhysScores().SpeedVariationModifier -= _Value;
		// if entity still slowed, do not send chat messages
		if (_SlowedEntity->getPhysScores().SpeedVariationModifier < 0)
		{
			return;
		}
	}
	else
	{
		const std::vector<CSEffectPtr>& effects = _SlowedEntity->getSEffects();
		for (uint i = 0 ; i < effects.size() ; ++i)
		{
			if (effects[i] && effects[i] != this && (effects[i]->getFamily() == EFFECT_FAMILIES::SlowMove || effects[i]->getFamily() == EFFECT_FAMILIES::CombatMvtSlow) )
				return;
		}
	}

	// send messages
	CEntityId actorId;
	if (TheDataset.isDataSetRowStillValid(_CreatorRowId))
		actorId = TheDataset.getEntityId(_CreatorRowId);

//	TVectorParamCheck params;

	if (_SlowedEntity->getId().getType() == RYZOMID::player)
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_SlowedEntity->getEntityRowId(), "EFFECT_SLOW_MOVE_END_TARGET");
	}

	if ( actorId.getType() == RYZOMID::player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].EId = _SlowedEntity->getId();

		PHRASE_UTILITIES::sendDynamicSystemMessage(_CreatorRowId, "EFFECT_SLOW_MOVE_END_ACTOR", params);
	}
	// todo spectators and self slow (after madness for instance)

} // removed //

//CEffectTFactory<CSlowMoveEffect> *CSnareEffectFactory = new CEffectTFactory<CSlowMoveEffect>(EFFECT_FAMILIES::CombatMvtSlow);
