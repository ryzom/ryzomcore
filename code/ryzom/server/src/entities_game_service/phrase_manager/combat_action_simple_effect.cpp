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
#include "combat_action_simple_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "combat_phrase.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatActionSimpleEffect::apply(CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif
	H_AUTO(CCombatActionSimpleEffect_apply);

	_CombatPhrase = phrase;

	if (_ApplyOnTargets)
	{
		const std::vector<CCombatPhrase::TTargetInfos> &targets = phrase->getTargets();
		for (uint i = 0; i < targets.size() ; ++i)
		{
//			if ( !phrase->hasTargetDodged(i) )
			if ( phrase->getTargetDodgeFactor(i) == 0.0f )
			{
				if (targets[i].Target != NULL)
				{
					//applyOnEntity(targets[i].Target->getEntity(), phrase->getPhraseSuccessDamageFactor());
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
		//applyOnEntity(entity, phrase->getPhraseSuccessDamageFactor());
		applyOnEntity(entity, 1.0f);
	}
} // apply //

//--------------------------------------------------------------
//					applyOnEntity()  
//--------------------------------------------------------------
void CCombatActionSimpleEffect::applyOnEntity( CEntityBase *entity, float successFactor )
{
	H_AUTO(CCombatActionSimpleEffect_applyOnEntity);

	if (!entity || !_CombatPhrase) return;

	// if entity is already dead, return
	if (entity->isDead())
		return;
	
	TGameCycle endDate;
	if ( _UsePhraseLatencyAsDuration == true && _CombatPhrase != 0)
	{
		endDate = _CombatPhrase->latencyEndDate();
	}
	else
	{
		endDate = TGameCycle(_Duration*successFactor) + CTickEventHandler::getGameCycle();
	}

	_Effect = new CSimpleEffect( _ActorRowId, entity->getEntityRowId(), _Family, _EffectValue, endDate, _EffectPower);
	if (!_Effect)
	{
		nlwarning("COMBAT : <CCombatActionSimpleEffect::apply> Failed to allocate new CSimpleEffect object !");
		return;
	}

	entity->addSabrinaEffect(_Effect);

	// check an effect name has been set
/*	if (_EffectName.empty())
	{
		nlwarning("COMBAT : Effect name not set for effect family %s", EFFECT_FAMILIES::toString(_Family).c_str() );
		return;
	}

	CEntityId actorId;
	if (TheDataset.isDataSetRowStillValid(_ActorRowId))
		actorId = TheDataset.getEntityId(_ActorRowId);

	TVectorParamCheck params;

	/// todo: send info messages to client and spectators
	if (entity->getEntityRowId() != _ActorRowId)
	{		
		string msgName = "EFFECT_"+_EffectName+"_BEGIN";

		// effect creator
		if ( actorId.getType() == RYZOMID::player)
		{
			params.resize(1);
			params[0].Type = STRING_MANAGER::entity;
			params[0].EId = entity->getId();

			const string str = NLMISC::toString("%s_CREATOR",msgName.c_str());
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, str, params);
		}
		// effect target
		if (entity->getId().getType() == RYZOMID::player)
		{
			params.resize(1);
			params[0].Type = STRING_MANAGER::entity;
			params[0].EId = actorId;
			const string str = NLMISC::toString("%s_TARGET",msgName.c_str());
			PHRASE_UTILITIES::sendDynamicSystemMessage(entity->getEntityRowId(), str, params);
		}

		// spectators
		CEntityId centerId;
		if ( entity->getId().getType() == RYZOMID::player || entity->getId().getType() == RYZOMID::npc)
			centerId = entity->getId();
		else if ( actorId.getType() == RYZOMID::player || actorId.getType() == RYZOMID::npc)
			centerId = actorId;
		else
			return;

		params.resize(2);
		params[0].Type = STRING_MANAGER::entity;
		params[0].EId = actorId;
		params[1].Type = STRING_MANAGER::entity;
		params[1].EId = entity->getId();

		vector<CEntityId> excluded;
		excluded.push_back(actorId);
		excluded.push_back(entity->getId());
		const string str = NLMISC::toString("%s_SPECTATORS",msgName.c_str());
		PHRASE_UTILITIES::sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(centerId), excluded, str, params);
	}
	else
	{
		string msgName = "EFFECT_"+_EffectName+"_SELF_BEGIN";

		if ( actorId.getType() == RYZOMID::player)
		{
			const string str = NLMISC::toString("%s_CREATOR",msgName.c_str());
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, str);
		}
		else if( actorId.getType() != RYZOMID::npc )
		{
			// cannot send spectator messages for creatures
			return;
		}

		params.resize(1);
		// send to spectators
		switch(actorId.getType())
		{
		case RYZOMID::player:
			params[0].Type = STRING_MANAGER::player;
			msgName = "EFFECT_"+_EffectName+"_SELF_BEGIN_SPECTATORS_PLAYER";
			break;
		case RYZOMID::npc:
			params[0].Type = STRING_MANAGER::bot;
			msgName = "EFFECT_"+_EffectName+"_SELF_BEGIN_SPECTATORS_NPC";
			break;
		default:
			params[0].Type = STRING_MANAGER::creature;
			msgName = "EFFECT_"+_EffectName+"_SELF_BEGIN_SPECTATORS_CREATURE";
			break;
		};
		params[0].EId = actorId;

		vector<CEntityId> excluded;
		excluded.push_back(actorId);
		PHRASE_UTILITIES::sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(actorId), excluded, msgName, params);
	}
*/
} // applyOnEntity //
