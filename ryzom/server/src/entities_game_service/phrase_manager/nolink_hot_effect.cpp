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
#include "nolink_hot_effect.h"
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
//		CNoLinkHoTEffect::update()
//--------------------------------------------------------------
bool CNoLinkHoTEffect::update(CTimerEvent * event, bool applyEffect)
{
	const TGameCycle time = CTickEventHandler::getGameCycle();

	if(_TargetEntity == NULL)
	{
		_TargetEntity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
		if (!_TargetEntity)
		{
			_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
			return true;
		}
	}

	if (applyEffect)
	{
		sint32 heal = sint32(_CycleHeal);
		_RemainingHeal += (float)fabs(_CycleHeal - float(heal));

		if (_RemainingHeal >= 1.0f)
		{
			_RemainingHeal -= 1.0f;
			++heal;
		}

		switch(_AffectedScore)
		{
		case SCORES::hit_points:
			_TargetEntity->changeCurrentHp(heal, _CreatorRowId);
			break;
		default:
			_TargetEntity->changeScore(_AffectedScore, heal);
		};
	}

	// set timer next event
	_UpdateTimer.setRemaining(_CycleLength, event);

	return false;
} // update //

//--------------------------------------------------------------
//		CNoLinkHoTEffect::removed()
//--------------------------------------------------------------
void CNoLinkHoTEffect::removed()
{
	if (!_TargetEntity) return;

	// if entity is dead, do not send messages
	if (_TargetEntity->isDead())
		return;

	DEBUGLOG("EFFECT: HoT effect ends on entity %s", _TargetEntity->getId().toString().c_str());
	
	// send messages to target
/*	if (_TargetEntity->getId().getType() == RYZOMID::player)
		PHRASE_UTILITIES::sendDynamicSystemMessage( _TargetRowId, "EFFECT_BLEED_ENDED");

	// try to inform actor
	if ( _CreatorRowId != _TargetRowId && _CreatorRowId.isValid() && TheDataset.isDataSetRowStillValid(_CreatorRowId))
	{
		CCharacter *actor = PlayerManager.getChar(_CreatorRowId);
		if (actor != NULL)
		{
			TVectorParamCheck params;
			params.resize(1);
			params[0].Type = STRING_MANAGER::entity;
			params[0].EId = _TargetEntity->getId();
			PHRASE_UTILITIES::sendDynamicSystemMessage( actor->getEntityRowId(), "EFFECT_BLEED_ENDED_ACTOR", params);
		}
	}
*/
} // removed //
