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
#include "combat_stun_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "entity_manager/entity_base.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//		CCombatStunEffect::isTimeToUpdate()
//--------------------------------------------------------------
bool CCombatStunEffect::isTimeToUpdate()
{
	return (_StunEndDate <= CTickEventHandler::getGameCycle());
} // isTimeToUpdate //

//--------------------------------------------------------------
//		CCombatStunEffect::update()
//--------------------------------------------------------------
bool CCombatStunEffect::update( uint32 & updateFlag )
{
	// always return true as the test is made in isTimeToUpdate
	return true;
} // update //

//--------------------------------------------------------------
//		CCombatStunEffect::removed()
//--------------------------------------------------------------
void CCombatStunEffect::removed()
{
	if (!_StunnedEntity) return;

	// returns if entity already dead
	if (_StunnedEntity->isDead())
		return;

	DEBUGLOG("COMBAT EFFECT: stun effect ends on entity %s", _StunnedEntity->getId().toString().c_str());

	// if this is the last STUN effect on target, then wake it
	const std::vector<CSEffect*>& effects = _StunnedEntity->getSEffects();
	for (uint i = 0 ; i < effects.size() ; ++i)
	{
		if (effects[i] && effects[i] != this && (effects[i]->getFamily() == EFFECT_FAMILIES::Stun || effects[i]->getFamily() == EFFECT_FAMILIES::CombatStun) )
		{
			DEBUGLOG("COMBAT EFFECT : entity is still stunned (has another stun effect)");
			return;
		}
	}

	// wake entity
	_StunnedEntity->wake();
	DEBUGLOG("COMBAT EFFECT : was last stun effects on this entity, wake it");

	CEntityId actorId;
	if (TheDataset.isDataSetRowStillValid(_CreatorRowId))
		actorId = TheDataset.getEntityId(_CreatorRowId);

	// send message
//	PHRASE_UTILITIES::sendSimpleMessage( _TargetRowId, "OPS_EFFECT_STUN_END" );
//	PHRASE_UTILITIES::sendMessage( _CreatorRowId, "OPS_EFFECT_STUN_END_E", _TargetRowId );

	TVectorParamCheck params;

	if (_StunnedEntity->getId().getType() == RYZOMID::player)
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_StunnedEntity->getEntityRowId(), "EFFECT_STUN_END_TARGET");
	}

	if ( actorId.getType() == RYZOMID::player)
	{
		params.resize(1);
		params[0].Type = STRING_MANAGER::entity;
		params[0].EId = _StunnedEntity->getId();

		PHRASE_UTILITIES::sendDynamicSystemMessage(_CreatorRowId, "EFFECT_STUN_END_ACTOR", params);
	}
	// todo spectators and self stun
} // removed //

