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
// misc
#include "nel/misc/bit_mem_stream.h"
// game_share
#include "game_share/generic_xml_msg_mngr.h"
//
#include "combat_action_stun.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "combat_phrase.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//					initFromAiAction()  
//--------------------------------------------------------------
bool CCombatActionStun::initFromAiAction( const CStaticAiAction *aiAction, CCombatPhrase *phrase )
{
#ifdef NL_DEBUG
	nlassert(phrase);
	nlassert(aiAction);
#endif
	
	if (aiAction->getType() != AI_ACTION::Range && aiAction->getType() != AI_ACTION::Melee)
		return false;
	
	_ActorRowId = phrase->getAttacker()->getEntityRowId();
	
	// read parameters
	const CCombatParams &data = aiAction->getData().Combat;
	_StunDuration = data.EffectTime;
	
	//DEBUG
	if (_StunDuration > 300) // 30s
	{
		nlwarning("COMBAT : found a stun duration of %u ticks for ai action, set it to 30s instead", _StunDuration);
		_StunDuration = 300;
	}
	
	return true;
} // initFromAiAction //

//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatActionStun::apply(CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif
	H_AUTO(CCombatActionStun_apply);

	_CombatPhrase = phrase;

	if (_ApplyOnTargets)
	{
		const std::vector<CCombatPhrase::TTargetInfos> &targets = phrase->getTargets();
		for (uint i = 0; i < targets.size() ; ++i)
		{
			if ( phrase->getTargetDodgeFactor(i) == 0.0f )
			{
				if (targets[i].Target != NULL)
				{
					applyOnEntity(targets[i].Target->getEntity(), 1.0f - phrase->getTargetDodgeFactor(i));
				}
			}
		}
	}
	else
	{
		CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_ActorRowId);
		if (!entity)
		{
			nlwarning("COMBAT : <CCombatActionRegenModifier::apply> Cannot find the target entity, cancel");
			return;
		}
		applyOnEntity(entity, 1.0f);
	}
} // apply //


//--------------------------------------------------------------
//					applyOnEntity()  
//--------------------------------------------------------------
void CCombatActionStun::applyOnEntity( CEntityBase *entity, float successFactor )
{
	H_AUTO(CCombatActionStun_applyOnEntity);

	if (!entity || !_CombatPhrase) return;

	// if entity is already dead, return
	if (entity->isDead())
		return;

	if (!_StunDuration)
		return;

	if (_StunDuration > 100) // 10s
	{
		nlwarning("COMBAT : found a stun duration of %u ticks for entity %s, set it to 3s instead", _StunDuration, entity->getId().toString().c_str());
		_StunDuration = 30;
	}
	
	TGameCycle endDate = TGameCycle(_StunDuration*successFactor) + CTickEventHandler::getGameCycle();
	DEBUGLOG("EFFECT : stun entity %s for %u ticks", entity->getId().toString().c_str(), _StunDuration);

	CStunEffect *effect = new CStunEffect( _ActorRowId, entity->getEntityRowId(), EFFECT_FAMILIES::CombatStun, (uint8)_StunDuration, endDate);
	if (!effect)
	{
		nlwarning("COMBAT : <CCombatActionStun::apply> Failed to allocate new CCombatStunEffect object !");
		return;
	}

	entity->addSabrinaEffect(effect);

} // applyOnEntity //

CCombatAIActionTFactory<CCombatActionStun> *CCombatAiStunFactoryInstance = new CCombatAIActionTFactory<CCombatActionStun>(AI_ACTION::Stun);
