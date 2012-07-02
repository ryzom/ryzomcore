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
#include "special_power_mod_forage_success.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "special_power_phrase.h"
#include "player_manager/player_manager.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "mod_forage_success_effect.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager PlayerManager;



//--------------------------------------------------------------
//					ctor()  
//--------------------------------------------------------------
CSpecialPowerModForageSuccess::CSpecialPowerModForageSuccess(TDataSetRow actorRowId, CSpecialPowerPhrase *phrase, float durationInSeconds,  
							  const std::string& ecosystem, float modifier1, float modifier2 )
{
	if(TheDataset.isAccessible(actorRowId))
		_ActorRowId = actorRowId;
	else
	{
		nlwarning("<CSpecialPowerModForageSuccess> invalid data set row passed as actor");
	}
	
	_Phrase = phrase;
	
	_DisablePowerTime = 0;
	_Duration = NLMISC::TGameCycle(durationInSeconds / CTickEventHandler::getGameTimeStep());		
	
	_Ecosystem = ecosystem;
	_Modifier1 = modifier1;
	_Modifier2 = modifier2;

	ECOSYSTEM::EECosystem eco = ECOSYSTEM::stringToEcosystem(_Ecosystem);
	switch( eco )
	{
	case ECOSYSTEM::common_ecosystem:
		_PowerType = POWERS::ModForageSkill;
		break;
	case ECOSYSTEM::desert:
		_PowerType = POWERS::ModDesertForageSkill;
		break;
	case ECOSYSTEM::forest:
		_PowerType = POWERS::ModForestForageSkill;
		break;
	case ECOSYSTEM::lacustre:
		_PowerType = POWERS::ModLacustreForageSkill;
		break;
	case ECOSYSTEM::jungle:
		_PowerType = POWERS::ModJungleForageSkill;
		break;
	case ECOSYSTEM::primary_root:
		_PowerType = POWERS::ModPrimaryRootForageSkill;
		break;
	default:
		_PowerType = POWERS::UnknownType;
		nldebug("<CSpecialPowerModForageSuccess::CSpecialPowerModForageSuccess> The ecosystem %s is not managed by this kind of power",_Ecosystem.c_str());
	}
}



//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CSpecialPowerModForageSuccess::apply()
{
	if (!_Phrase)
		return;

	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerModForageSuccess::apply> Cannot find actor entity or not a player");
		return;
	}

	// disable power
	actor->forbidPower(_PowerType, _Phrase->getConsumableFamilyId(), CTickEventHandler::getGameCycle() + _DisablePowerTime);

	const TGameCycle endDate = _Duration + CTickEventHandler::getGameCycle();

	// create effect and apply it on target
	EFFECT_FAMILIES::TEffectFamily effectFamily;
	ECOSYSTEM::EECosystem eco = ECOSYSTEM::stringToEcosystem(_Ecosystem);
	switch( eco )
	{
		case ECOSYSTEM::common_ecosystem:
			effectFamily = EFFECT_FAMILIES::PowerModForageSkill;
			break;
		case ECOSYSTEM::desert:
			effectFamily = EFFECT_FAMILIES::PowerModDesertForageSkill;
			break;
		case ECOSYSTEM::forest:
			effectFamily = EFFECT_FAMILIES::PowerModForestForageSkill;
			break;
		case ECOSYSTEM::lacustre:
			effectFamily = EFFECT_FAMILIES::PowerModLacustreForageSkill;
			break;
		case ECOSYSTEM::jungle:
			effectFamily = EFFECT_FAMILIES::PowerModJungleForageSkill;
			break;
		case ECOSYSTEM::primary_root:
			effectFamily = EFFECT_FAMILIES::PowerModPrimaryRootForageSkill;
			break;
		default:
			nldebug("<CSpecialPowerModForageSuccess::apply> The ecosystem '%s' is not managed by this special power",_Ecosystem.c_str());
			return;
	}

	CModForageSuccessEffect *effect = new CModForageSuccessEffect(_ActorRowId, 
													endDate, 
													effectFamily,
													_Ecosystem,
													_Modifier1,
													_Modifier2);
	if (!effect)
	{
		nlwarning("<CSpecialPowerModForageSuccess::apply> Failed to allocate new CModForageSuccessEffect");
		return;
	}

	actor->addSabrinaEffect(effect);

	// send messages
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::power_type);
//	TVectorParamCheck params;
	// for actor
	if (actor->getId().getType() == RYZOMID::player)
	{
		params[0].Enum = _PowerType;
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_USE", params);
	}
} // apply //
