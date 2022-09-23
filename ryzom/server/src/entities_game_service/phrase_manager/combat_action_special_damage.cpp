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
#include "game_share/mode_and_behaviour.h"
//
#include "combat_action_special_damage.h"
#include "combat_phrase.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatActionSpecialDamage::apply(CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif
	H_AUTO(CCombatActionSpecialDamage_apply);

	const std::vector<CCombatPhrase::TTargetInfos> &targets = phrase->getTargets();
	for (uint i = 0; i < targets.size() ; ++i)
	{
		if ( phrase->getTargetDodgeFactor(i) == 0.0f )
			applyOnTarget(i,phrase);
	}
} // apply //

//--------------------------------------------------------------
//					applyOnTarget()  
//--------------------------------------------------------------
void CCombatActionSpecialDamage::applyOnTarget(uint8 targetIndex, CCombatPhrase *phrase)
{
#if !FINAL_VERSION
	nlassert(phrase);
#endif

	if (_DamageType >= DMGTYPE::NBTYPES || _DamageType < 0)
	{
		nlwarning("COMBAT : <CCombatActionSpecialDamage::applyOnTarget> Bad damage type");
		return;
	}

	if (!TheDataset.isAccessible(_ActorRowId))
		return;

	const CCombatDefenderPtr &targetDefender = phrase->getTarget(targetIndex);
	if(!targetDefender)
		return;

	CEntityBase *entity = targetDefender->getEntity();
	if (!entity)
	{
		nlwarning("COMBAT : <CCombatActionSpecialDamage::applyOnTarget> Cannot find the target entity, cancel");
		return;
	}

	// if entity is already dead, return
	if (entity->isDead())
		return;

	// compute damage
	sint32 damage = sint32(phrase->damage(targetIndex) * getApplyValue(phrase->weaponSabrinaValue()));

	// get resistance associated to damage type
	// get target resist value
	// test resistance and compute  really inflicted damage	
	if (entity->getId().getType() == RYZOMID::player)
	{
		CCombatDefenderPlayer *pcDefender = dynamic_cast<CCombatDefenderPlayer *> ( &(*targetDefender));
		if (pcDefender)
		{
			const SLOT_EQUIPMENT::TSlotEquipment loc = phrase->getHitLocalisation();
			// get armor
			CCombatArmor armor;
			if (pcDefender->getArmor(loc, armor) )
			{
				sint32 protection = sint32( armor.ProtectionFactor[_DamageType] * (float)damage );
				if (protection > armor.MaxProtection[_DamageType])
					protection = armor.MaxProtection[_DamageType];
				
				damage -= protection;
			}
		}
		damage = entity->applyDamageOnArmor( _DamageType, damage );
	}
	else	
	{
		// defender is AI, resistance is like an armor
		CCombatDefenderAI *aiDefender = dynamic_cast<CCombatDefenderAI *> ( &(*targetDefender));
		if (aiDefender)
		{
			const SLOT_EQUIPMENT::TSlotEquipment loc = phrase->getHitLocalisation();
			// get armor
			CCombatArmor armor;
			if (aiDefender->getArmor(loc, armor) )
			{
				sint32 protection = sint32( armor.ProtectionFactor[_DamageType] * (float)damage );
				if (protection > armor.MaxProtection[_DamageType])
					protection = armor.MaxProtection[_DamageType];

				damage -= protection;
			}
		}
	}

	const CEntityId actorId = TheDataset.getEntityId(_ActorRowId);
	
	TVectorParamCheck params;
	if (_ActorRowId != entity->getEntityRowId())
	{
		if ( actorId.getType() == RYZOMID::player)
		{
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::damage_type);
			params[0].setEIdAIAlias( entity->getId(), CAIAliasTranslator::getInstance()->getAIAlias(entity->getId()) );
			params[1].Int = damage;
			params[2].Enum = _DamageType;
			
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_SPECIAL_DAMAGE_ACTOR", params);
		}
		
		if (entity->getId().getType() == RYZOMID::player)
		{
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::damage_type);
			params[0].setEIdAIAlias( actorId, CAIAliasTranslator::getInstance()->getAIAlias(actorId) );
			params[1].Int = damage;
			params[2].Enum = _DamageType;
			PHRASE_UTILITIES::sendDynamicSystemMessage(entity->getEntityRowId(), "MAGIC_SPECIAL_DAMAGE_TARGET", params);
		}

		CEntityId senderId;
		if ( actorId.getType() == RYZOMID::player || actorId.getType() == RYZOMID::npc )
		{
			senderId = actorId;
		}
		else if ( entity->getId().getType() == RYZOMID::player || entity->getId().getType() == RYZOMID::npc )
		{
			senderId = entity->getId();
		}
		
/*		if (senderId != CEntityId::Unknown)
		{
			params.resize(4);
			params[0].Type = STRING_MANAGER::entity;
			params[0].setEIdAIAlias( actorId, CAIAliasTranslator::getInstance()->getAIAlias(actorId) );
			params[1].Type = STRING_MANAGER::entity;
			params[1].setEIdAIAlias( entity->getId(), CAIAliasTranslator::getInstance()->getAIAlias(entity->getId()) );
			params[2].Type = STRING_MANAGER::integer;
			params[2].Int = damage;
			params[3].Type = STRING_MANAGER::damage_type;
			params[3].Enum = _DamageType;
			
			vector<CEntityId> excluded;
			excluded.push_back(actorId);
			excluded.push_back(entity->getId());
			
			PHRASE_UTILITIES::sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "MAGIC_SPECIAL_DAMAGE_SPECTATORS", params);
		}
*/
	}
	else
	{
		if ( actorId.getType() == RYZOMID::player)
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::integer, STRING_MANAGER::damage_type);
			params[0].Int = damage;
			params[1].Enum = _DamageType;
			
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "MAGIC_SPECIAL_DAMAGE_SELF", params);
		}
		
/*		if ( actorId.getType() == RYZOMID::player || actorId.getType() == RYZOMID::npc )
		{
			SM_STATIC_PARAMS_3(params, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::damage_type);
			params[0].setEIdAIAlias( actorId, CAIAliasTranslator::getInstance()->getAIAlias(actorId) );
			params[1].Int = damage;
			params[2].Enum = _DamageType;
			
			vector<CEntityId> excluded;
			excluded.push_back(actorId);

			string message;
			switch(actorId.getType()) 
			{
			case RYZOMID::player:
				message = "MAGIC_SPECIAL_DAMAGE_SELF_SPECTATORS_PLAYER";
				break;
			case RYZOMID::npc:
				message = "MAGIC_SPECIAL_DAMAGE_SELF_SPECTATORS_NPC";
				break;
			default:
				message = "MAGIC_SPECIAL_DAMAGE_SELF_SPECTATORS_CREATURE";
			};
			
			PHRASE_UTILITIES::sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(actorId), excluded, message, params);
		}
*/
	}
	
	// inflict damage on target
	if ( entity->changeCurrentHp( -damage, _ActorRowId) )
	{
		PHRASE_UTILITIES::sendDeathMessages(_ActorRowId,entity->getEntityRowId());
		if (targetIndex == 0)
			phrase->getExecutionBehaviour().Combat.KillingBlow = 1;
	}

} // applyOnTarget //

