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
#include "special_power_heal.h"

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
bool CSpecialPowerHeal::validate(std::string &errorCode)
{
	if (!CSpecialPower::validate(errorCode))
		return false;

	// check affected score is valid
	if (_AffectedScore < 0 || _AffectedScore >= SCORES::NUM_SCORES)
	{
		nlwarning("Invalid score %d", (sint)_AffectedScore );
		return false;
	}

	return true;
}
	

//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CSpecialPowerHeal::apply()
{
	if (!_Phrase)
		return;

#if !FINAL_VERSION
	nlassert(_AffectedScore==SCORES::hit_points || _AffectedScore==SCORES::sap || _AffectedScore==SCORES::stamina || _AffectedScore==SCORES::focus);
#endif

	CCharacter *actor = PlayerManager.getChar(_ActorRowId);
	if (!actor)
	{
		nlwarning("<CSpecialPowerHeal::apply> Cannot find actor entity or not a player");
		return;
	}

	// disable power
	actor->forbidPower(_PowerType, _Phrase->getConsumableFamilyId(), CTickEventHandler::getGameCycle() + _DisablePowerTime + _Phrase->getAdditionalRecastTime());

	sint32 healValue = sint32( _HealValue + actor->getMaxScore(_AffectedScore) * _HealFactorValue );

	CCharacter* player = PlayerManager.getChar( _ActorRowId );
	if ( player )
	{
		const CSEffect* pEffect = player->lookForActiveEffect( EFFECT_FAMILIES::TotemCombatMagDef );
		if ( pEffect != NULL )
		{
			healValue += ( healValue * pEffect->getParamValue() / 100 );
		}
	}

	// heal actor
	actor->changeScore(_AffectedScore, healValue );

	// send messages
	PHRASE_UTILITIES::sendScoreModifierSpellMessage( actor->getId(), actor->getId(), healValue, healValue, _AffectedScore , ACTNATURE::CURATIVE_MAGIC);

} // apply //
