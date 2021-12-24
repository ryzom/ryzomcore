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
#include "special_power.h"

#include "phrase_manager/phrase_utilities_functions.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "phrase_manager/s_effect.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CSpecialPower);

extern CPlayerManager PlayerManager;

//--------------------------------------------------------------
//	CSpecialPower::validate()
//--------------------------------------------------------------
bool CSpecialPower::validate(std::string &errorCode)
{
	// check the power can be activated righ now (reuse time)
	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPower::validate> Cannot find actor entity or not a player");
		return false;
	}

	if (_ByPassDisablePowerTimer)
		return true;

	TGameCycle endDate;
	if (!actor->canUsePower(_PowerType, (uint16)~0, endDate))
	{
		uint16 seconds = uint16((endDate - CTickEventHandler::getGameCycle())*CTickEventHandler::getGameTimeStep());
		uint8 minutes = uint8(seconds/60);
		seconds = seconds%60;
		
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::power_type, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Enum = _PowerType;
		params[1].Int = minutes;
		params[2].Int = seconds;
		
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_DISABLED", params);
		DEBUGLOG("<CSpecialPowerBalance::validate> Cannot use shielding yet, still disabled");
		return false;
	}
	
	return true;
} // validate //


//--------------------------------------------------------------
//	CSpecialPowerAuras::validate()
//--------------------------------------------------------------
bool CSpecialPowerAuras::validate(std::string &errorCode)
{
	// check the power can be activated righ now (reuse time)
	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerBasicAura::validate> Cannot find actor entity or not a player");
		return false;
	}
	
	if (actor->isDead())
		return false;
	
	if (_ByPassTargetsDisableAuraTime)
		return true;
	TGameCycle endDate = actor->getForbidAuraUseEndDate();
	if (actor->getForbidAuraUseEndDate() >= CTickEventHandler::getGameCycle())
	{
		uint16 seconds = uint16((endDate - CTickEventHandler::getGameCycle())*CTickEventHandler::getGameTimeStep());
		uint8 minutes = uint8(seconds/60);
		seconds = seconds%60;
		
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::power_type, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].Enum = _PowerType;
		params[1].Int = minutes;
		params[2].Int = seconds;
		
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_DISABLED", params);
		DEBUGLOG("<CSpecialPowerAuras::validate> Cannot use power yet, still disabled");
		return false;
	}	
	
	return true;
} // validate //
