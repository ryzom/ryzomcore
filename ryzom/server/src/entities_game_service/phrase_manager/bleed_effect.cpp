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
#include "bleed_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "entity_manager/entity_base.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager PlayerManager;

//--------------------------------------------------------------
//		CBleedEffect::update()
//--------------------------------------------------------------
bool CBleedEffect::update(CTimerEvent * event, bool applyEffect)
{

	const TGameCycle date = CTickEventHandler::getGameCycle();
	float totalDamage = 0.0f;
	std::list<CSlaveBleedEffect>::iterator it;
	for ( it = _Effects.begin() ; it != _Effects.end() ; )
	{
		totalDamage += (*it).CycleDamage;
		if ((*it).EndDate <= date)
			it = _Effects.erase(it);
		else
			++it;
	}

	sint32 damage = sint32(totalDamage);
	_RemainingDamage += (float)fabs(totalDamage - float(damage));

//	sint32 damage = sint32(_CycleDamage);
//	_RemainingDamage += (float)fabs(_CycleDamage - float(damage));
	
	if (_RemainingDamage >= 1.0f)
	{
		_RemainingDamage -= 1.0f;
		++damage;
	}
	
	// remove hp
	if ( damage > 0 && _BleedingEntity != NULL)
	{
		// send messages
		// to target
		if (_BleedingEntity->getId().getType() == RYZOMID::player)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = damage;
			PHRASE_UTILITIES::sendDynamicSystemMessage( _BleedingEntity->getEntityRowId(), "EFFECT_BLEED_LOSE_HP", params);
		}
		// to actor
		if ( _CreatorRowId != _TargetRowId && _CreatorRowId.isValid() && TheDataset.isDataSetRowStillValid(_CreatorRowId))
		{
			CCharacter *actor = PlayerManager.getChar(_CreatorRowId);
			if (actor != NULL)
			{
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::integer);
				params[0].setEIdAIAlias( _BleedingEntity->getId(), CAIAliasTranslator::getInstance()->getAIAlias(_BleedingEntity->getId()) );
				params[1].Int = damage;
				PHRASE_UTILITIES::sendDynamicSystemMessage( actor->getEntityRowId(), "EFFECT_BLEED_LOSE_HP_ACTOR", params);
			}
		}

		// remove HP
		if (_BleedingEntity->changeCurrentHp( (-1) * damage, _CreatorRowId))
		{
			// killed entity, so this effect and all other effects have been cleared send kill message and return true
			PHRASE_UTILITIES::sendDeathMessages( _CreatorRowId, _TargetRowId);
			_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
			return true;
		}
	}

	// set timer next event
	_UpdateTimer.setRemaining(_CycleLength, event);

	return false;
} // update //

//--------------------------------------------------------------
//		CBleedEffect::removed()
//--------------------------------------------------------------
void CBleedEffect::removed()
{
	if (!_BleedingEntity) return;

	// if entity is dead, do not send messages
	if (_BleedingEntity->isDead())
		return;

	DEBUGLOG("COMBAT EFFECT: Bleed effect ends on entity %s", _BleedingEntity->getId().toString().c_str());
	
	// check entity isn't bleeding anymore
	const std::vector<CSEffectPtr>& effects = _BleedingEntity->getSEffects();
	for (uint i = 0 ; i < effects.size() ; ++i)
	{
		if (effects[i] && effects[i] != this && (effects[i]->getFamily() == EFFECT_FAMILIES::CombatBleed) )
		{
			DEBUGLOG("EFFECT : entity is still bleeding (has another bleed effect)");
			return;
		}
	}

	// send messages to target
	if (_BleedingEntity->getId().getType() == RYZOMID::player)
		PHRASE_UTILITIES::sendDynamicSystemMessage( _BleedingEntity->getEntityRowId(), "EFFECT_BLEED_ENDED");

	// try to inform actor
	if ( _CreatorRowId != _TargetRowId && TheDataset.isAccessible(_CreatorRowId))
	{
		CCharacter *actor = PlayerManager.getChar(_CreatorRowId);
		if (actor != NULL)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
			params[0].setEIdAIAlias( _BleedingEntity->getId(), CAIAliasTranslator::getInstance()->getAIAlias(_BleedingEntity->getId()) );
			PHRASE_UTILITIES::sendDynamicSystemMessage( actor->getEntityRowId(), "EFFECT_BLEED_ENDED_ACTOR", params);
		}
	}

} // removed //

