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
//
#include "server_share/msg_ai_service.h"
//
#include "special_power_shielding.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "special_power_phrase.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "power_shielding_effect.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager PlayerManager;


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
bool CSpecialPowerShielding::validate(std::string &errorCode)
{
	// check the power can be activated righ now (reuse time)
	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerShielding::validate> Cannot find actor entity or not a player");
		return false;
	}

	TGameCycle endDate;
	if (!actor->canUsePower(_PowerType, (uint16)~0, endDate))
	{
		uint16 seconds = uint16((endDate - CTickEventHandler::getGameCycle())*CTickEventHandler::getGameTimeStep());
		uint8 minutes = uint8(seconds/60);
		seconds = seconds%60;
		
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::power_type, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Enum = POWERS::Shielding;
		params[1].Int = minutes;
		params[2].Int = seconds;
		
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_DISABLED", params);
		DEBUGLOG("<CSpecialPowerShielding::validate> Cannot use shielding yet, still disabled");
		return false;
	}
	
	// get targets
	const std::vector<TDataSetRow> &targets = _Phrase->getTargets();

	if (targets.empty())
	{
		PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "INVALID_TARGET" );
		return false;
	}

	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(targets[0]);
	if (!entity)
	{
		PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "INVALID_TARGET" );
		return false;
	}

	// if target is dead, returns
	if (entity->isDead())
	{
		PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "INVALID_TARGET" );
		return false;
	}
	

/*	// check distance ?
	const double distance = PHRASE_UTILITIES::getDistance(_ActorRowId, targets[0]);
	if ( distance > (double)_Range)
	{
		TVectorParamCheck params;
		params.resize(1);
		params[0].Type = STRING_MANAGER::entity;
		params[0].EId = entity->getId();
		
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_TAUNT_TARGET_TOO_FAR", params);

		DEBUGLOG("<CSpecialPowerShielding::validate> Target out of range");
		return false;
	}
*/

	// TODO check protected entity is valid (not an creature, an attackable npc, ??)
	if (entity->getId().getType() != RYZOMID::player)
	{
		PHRASE_UTILITIES::sendSimpleMessage( _ActorRowId, "INVALID_TARGET" );
		return false;
	}

	return true;
} // validate //

//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CSpecialPowerShielding::apply()
{
	if (!_Phrase)
		return;

	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerShielding::apply> Cannot find actor entity or not a player");
		return;
	}

	// disable power
	actor->forbidPower(_PowerType, _Phrase->getConsumableFamilyId(), CTickEventHandler::getGameCycle() + _DisablePowerTime + _Duration);

	// get targets
	const std::vector<TDataSetRow> &targets = _Phrase->getTargets();

	const TGameCycle endDate = _Duration + CTickEventHandler::getGameCycle();
	
	// only apply on main target 
	if (!targets.empty())
	{
		// get target entity
		CEntityBase *target = CEntityBaseManager::getEntityBasePtr(targets[0]);
		if (!target)
		{
			return;
		}

		// create effect and apply it on target
		CShieldingEffect *effect = new CShieldingEffect(_ActorRowId, targets[0], endDate, 0/*power*/);
		if (!effect)
		{
			nlwarning("<CSpecialPowerShielding::apply> Failed to allocate new CShieldingEffect");
			return;
		}

		effect->setNoShieldProtection( _NoShieldFactor, _NoShieldMaxProtection );
		effect->setBucklerProtection( _BucklerFactor, _BucklerMaxProtection );
		effect->setShieldProtection( _ShieldFactor, _ShieldMaxProtection );

		target->addSabrinaEffect(effect);

		// send messages
//		TVectorParamCheck params;
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::entity, STRING_MANAGER::power_type);

		// for actor
		if (actor->getId().getType() == RYZOMID::player)
		{
			params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias( target->getId() ) );
			params[1].Enum = POWERS::Shielding;
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_USE_ON_TARGET", params);
		}

		// for target
		if (target->getId().getType() == RYZOMID::player)
		{
			params[0].setEIdAIAlias( actor->getId(), CAIAliasTranslator::getInstance()->getAIAlias( actor->getId() ) );
			params[1].Enum = POWERS::Shielding;
			PHRASE_UTILITIES::sendDynamicSystemMessage(target->getEntityRowId(), "POWER_USE_ON_TARGET_TARGET", params);
		}

		// for spectators
//		CEntityId senderId;
//		if (actor->getId().getType() == RYZOMID::player || actor->getId().getType() == RYZOMID::npc)
//		{
//			senderId = actor->getId();
//		}
//		else if (target->getId().getType() == RYZOMID::player || target->getId().getType() == RYZOMID::npc)
//		{
//			senderId = target->getId();
//		}
//
//		if (senderId != CEntityId::Unknown)
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
//			params[2].Enum = POWERS::Shielding;
//			PHRASE_UTILITIES::sendDynamicGroupSystemMessage(TheDataset.getDataSetRow(senderId), excluded, "POWER_USE_ON_TARGET_SPECTATORS", params);
//		}
	}

} // apply //
