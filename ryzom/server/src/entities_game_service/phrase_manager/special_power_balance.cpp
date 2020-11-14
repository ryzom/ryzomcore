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
#include "special_power_balance.h"

#include "phrase_manager/phrase_utilities_functions.h"
#include "special_power_phrase.h"
#include "player_manager/player_manager.h"
#include "team_manager/team_manager.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "phrase_manager/s_effect.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CPlayerManager	PlayerManager;
extern CTeamManager		TeamManager;

//--------------------------------------------------------------
//	CSpecialPower::validate()
//--------------------------------------------------------------
bool CSpecialPowerBalance::validate(std::string &errorCode)
{
	if (!CSpecialPower::validate(errorCode))
		return false;

	// check affected score is valid
	if (_AffectedScore < 0 || _AffectedScore >= SCORES::NUM_SCORES)
	{
		nlwarning("Invalid score %d", (sint)_AffectedScore );
		return false;
	}

	// check actor is in a team
	const CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("Cannot find actor entity or not a player");
		return false;
	}

	// get user team
	const CTeam * team = TeamManager.getTeam(actor->getTeamId());
	if (!team)
	{
		/************************************************************************/
		/* TODO : send message to user
		/************************************************************************/
		return false;
	}

	return true;
}
	

//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CSpecialPowerBalance::apply()
{
	if (!_Phrase)
		return;

#if !FINAL_VERSION
	nlassert(_AffectedScore==SCORES::hit_points || _AffectedScore==SCORES::sap || _AffectedScore==SCORES::stamina);
#endif

	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerBalance::apply> Cannot find actor entity or not a player");
		return;
	}

	// disable power
	actor->forbidPower(_PowerType, _Phrase->getConsumableFamilyId(), CTickEventHandler::getGameCycle() + _DisablePowerTime);

	// get user team
	const CTeam * team = TeamManager.getTeam(actor->getTeamId());
	if (!team)
	{
		/************************************************************************/
		/* TODO : send message to user
		/************************************************************************/
		return;
	}

	//get team members within range and add their <score> current value
	sint32 totalValue = 0;
	sint32 totalMaxValue = 0;
	list<CCharacter*> affectedPlayers;
	const list<CEntityId> &members = team->getTeamMembers();
	for ( list<CEntityId>::const_iterator it = members.begin() ; it != members.end() ; ++it)
	{
		const double distance = PHRASE_UTILITIES::getDistance(_ActorRowId, TheDataset.getDataSetRow(*it));
		if (distance < (double)_Range)
		{
			CCharacter *player = PlayerManager.getChar(*it);
			if (player && !player->isDead() && CPVPManager2::getInstance()->isCurativeActionValid(dynamic_cast<CCharacter *>(actor), player, true))
			{
				affectedPlayers.push_back(player);
				totalValue += player->getScores()._PhysicalScores[_AffectedScore].Current.getValue();
				totalMaxValue += player->getScores()._PhysicalScores[_AffectedScore].Max.getValue();
			}
		}
	}

#if !FINAL_VERSION
	nlassert(!affectedPlayers.empty());
#endif

	// compute value to set in % of max
	const double factor = double( totalValue * (1.0f - _LossFactor))  / (double)totalMaxValue;

	// prepare messages
	// actor
	SM_STATIC_PARAMS_3(params3, STRING_MANAGER::power_type, STRING_MANAGER::score, STRING_MANAGER::integer);
	params3[0].Enum = _PowerType;
	params3[1].Enum = _AffectedScore;
	params3[2].Int = sint(100*factor);	
	PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "POWER_BALANCE_ACTOR", params3);

	SM_STATIC_PARAMS_4(params, STRING_MANAGER::entity, STRING_MANAGER::power_type, STRING_MANAGER::score, STRING_MANAGER::integer);
	// others
	params[0].setEIdAIAlias( actor->getId(), CAIAliasTranslator::getInstance()->getAIAlias( actor->getId()) );
	params[1].Enum = _PowerType;
	params[2].Enum = _AffectedScore;
	params[3].Int = sint(100*factor);

	// set new value
	for (list<CCharacter*>::const_iterator it = affectedPlayers.begin() ; it != affectedPlayers.end() ; ++it)
	{
		// compute new score value
		const sint32 newVal = max ( (sint32)1, sint32( factor * (*it)->getScores()._PhysicalScores[_AffectedScore].Max.getValue() ) );
		(*it)->changeScore( _AffectedScore, newVal - (*it)->getScores()._PhysicalScores[_AffectedScore].Current.getValue() );
		// send message
		if ( (*it) != actor)
		{
			// send messages
			PHRASE_UTILITIES::sendDynamicSystemMessage((*it)->getEntityRowId(), "POWER_BALANCE_OTHERS", params);
		}
	}
} // apply //
