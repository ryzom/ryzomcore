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



//////////////
//	INCLUDE	//
//////////////
#include "stdpch.h"
// net
#include "nel/net/message.h"
// misc
#include "nel/misc/bit_mem_stream.h"
//
#include "server_share/msg_ai_service.h"
//
#include "special_power_taunt.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "special_power_phrase.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "world_instances.h"
#include "player_manager/player.h"
#include "creature_manager/creature_manager.h"
#include "egs_sheets/egs_sheets.h"

//////////////
//	USING	//
//////////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;

//////////////
//	EXTERN	//
//////////////
extern CRandom			RandomGenerator;
extern CPlayerManager	PlayerManager;
extern CCreatureManager	CreatureManager;


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
bool CSpecialPowerTaunt::validate(std::string &errorCode)
{
	// check the power can be activated righ now (reuse time)
	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerTaunt::validate> Cannot find actor entity or not a player");
		return false;
	}

	TGameCycle endDate;
	if (!actor->canUsePower(_PowerType, (uint16)~0, endDate))
	{
		uint16 seconds = uint16( (endDate - CTickEventHandler::getGameCycle())*CTickEventHandler::getGameTimeStep() );
		uint8 minutes = uint8(seconds/60);
		seconds = seconds%60;
		
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::power_type, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Enum = POWERS::Taunt;
		params[1].Int = minutes;
		params[2].Int = seconds;

		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_DISABLED", params);
		DEBUGLOG("<CSpecialPowerTaunt::validate> Cannot use Taunt yet, still disabled");
		return false;
	}
	
	// get targets
	const std::vector<TDataSetRow> &targets = _Phrase->getTargets();

	if (targets.empty())
	{
		PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "INVALID_TARGET" );
		nldebug("<CSpecialPowerTaunt::validate> No target");
		return false;
	}

	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(targets[0]);
	if (!entity)
	{
		// TODO : send message
		//nlwarning("<CSpecialPowerTaunt::validate> Cannot find targeted entity");
		return false;
	}

	// if target is dead, returns
	if (entity->isDead())
	{
		// TODO : send message
		nldebug("<CSpecialPowerTaunt::validate> Targeted entity is dead");
		return false;
	}

	// check distance
	const double distance = PHRASE_UTILITIES::getDistance(_ActorRowId, targets[0]);
	if ( distance > (double)_Range)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias( entity->getId(), CAIAliasTranslator::getInstance()->getAIAlias(entity->getId()) );
		
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_TAUNT_TARGET_TOO_FAR", params);

		DEBUGLOG("<CSpecialPowerTaunt::validate> Target out of range");
		return false;
	}

	// can only affect attackable bots
	if (entity->getId().getType() != RYZOMID::creature && entity->getId().getType() != RYZOMID::npc)
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_TAUNT_TARGET_NOT_ATTACKABLE");

		nldebug("<CSpecialPowerTaunt::validate> Can only target creature or Npcs");
		return false;
	}
	if (!entity->getContextualProperty().directAccessForStructMembers().attackable())
	{
		CCreature * creature = CreatureManager.getCreature( entity->getId() );
		if ( ! creature->checkFactionAttackable(actor->getId()))
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_TAUNT_TARGET_NOT_ATTACKABLE");
		
			DEBUGLOG("<CSpecialPowerTaunt::validate> Target not attackable");
			return false;
		}
	}

	return true;
} // validate //

//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CSpecialPowerTaunt::apply()
{
	if (!_Phrase)
		return;

	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerTaunt::apply> Cannot find actor entity or not a player");
		return;
	}

	// disable power
	actor->forbidPower(_PowerType, _Phrase->getConsumableFamilyId(), CTickEventHandler::getGameCycle() + _DisablePowerTime);

	// get targets
	const std::vector<TDataSetRow> &targets = _Phrase->getTargets();
	
	// only apply on main target 
	if (targets.empty())
	{
		return;
	}
	
	CEntityBase *target = CEntityBaseManager::getEntityBasePtr(targets[0]);
	if (!target)
		return;

	// test taunt success
	if (testTauntSuccessOnEntity(target))
	{
		CAITauntMsg	tauntMessage;
		tauntMessage.PlayerRowId = _ActorRowId;
		tauntMessage.TargetRowId = targets[0];
		CWorldInstances::instance().msgToAIInstance(target->getInstanceNumber(), tauntMessage);
//		tauntMessage.send("AIS");	
		
		// send messages
//		TVectorParamCheck params;
		// for actor
		if (actor->getId().getType() == RYZOMID::player)
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::power_type);
			params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()) );
			params[1].Enum = POWERS::Taunt;
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_USE_ON_TARGET", params);
		}
		
		// for target : cannot taunt  a player, so nothing to send
		
		// for spectators
//		if (actor->getId().getType() == RYZOMID::player || actor->getId().getType() == RYZOMID::npc)
//		{
//			vector<CEntityId> excluded;
//			excluded.push_back(actor->getId());
//			excluded.push_back(target->getId());
//			
//			params.resize(3);
//			params[0].Type = STRING_MANAGER::entity;
//			params[0].EId = actor->getId();
//			params[1].Type = STRING_MANAGER::entity;
//			params[1].EId = target->getId();
//			params[2].Type = STRING_MANAGER::power_type;
//			params[2].Enum = POWERS::Taunt;
//			PHRASE_UTILITIES::sendDynamicGroupSystemMessage(actor->getEntityRowId(), excluded, "POWER_USE_ON_TARGET_SPECTATORS", params);
//		}
	}
	else
	{
		// send failure message
		// for actor
		if (actor->getId().getType() == RYZOMID::player)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		//	TVectorParamCheck params;
			params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()) );
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_TAUNT_FAILED", params);
		}
	}

	
} // apply //


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
bool CSpecialPowerTaunt::testTauntSuccessOnEntity(const CEntityBase *entity)
{
	if (!entity)
	{
		return false;
	}

	// check target "level"
	const CStaticCreatures* creatureForm = entity->getForm();
	if (!creatureForm)
	{
		DEBUGLOG("<CSpecialPowerTaunt::validate> Cannot find target static creature form");
		return false;
	}
	
	if (creatureForm->getTauntLevel() <= _TauntPower)
	{
		return true;
	}
	else
	{
		sint32 deltaLevel = sint32(_TauntPower) - sint32(creatureForm->getTauntLevel());
		// use a standard success test
		//uint8 chances = PHRASE_UTILITIES::getSuccessChance( deltaLevel );
		//float result = PHRASE_UTILITIES::getSucessFactor(chances, (uint8)RandomGenerator.rand(99) );
		uint8 chances = CStaticSuccessTable::getSuccessChance( SUCCESS_TABLE_TYPE::FightPhrase, deltaLevel);
		float result = CStaticSuccessTable::getSuccessFactor( SUCCESS_TABLE_TYPE::FightPhrase, deltaLevel, (uint8)RandomGenerator.rand(99) );
#ifdef NL_DEBUG		
		nldebug("Taunt : power = %u, creature '%s', level %u, chances = %u, result = %f", 
			_TauntPower, 
			entity->_SheetId.toString().c_str(), 
			creatureForm->getTauntLevel(), 
			chances, 
			result);
#endif
		if (result>=1.0f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
} // testTauntSuccessOnEntity //

