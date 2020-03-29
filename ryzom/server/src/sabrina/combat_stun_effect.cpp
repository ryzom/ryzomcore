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
#include "phrase_utilities_functions.h"

#include "entity_base.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//--------------------------------------------------------------
//		CEntityPhrases::cancelTopSentence()
//--------------------------------------------------------------
bool CCombatStunEffect::isTimeToUpdate()
{
	return (_StunEndDate <= CTickEventHandler::getGameCycle());
} // isTimeToUpdate //

//--------------------------------------------------------------
//		CEntityPhrases::cancelTopSentence()
//--------------------------------------------------------------
bool CCombatStunEffect::update( uint32 & updateFlag )
{
	// always return true as the test is made in isTimeToUpdate
	return true;
} // update //

//--------------------------------------------------------------
//		CEntityPhrases::cancelTopSentence()
//--------------------------------------------------------------
void CCombatStunEffect::removed()
{
	if (!_StunnedEntity) return;

	// wake entity
	_StunnedEntity->wake();

	// send message
	PHRASE_UTILITIES::sendSimpleMessage( _TargetRowId, "OPS_EFFECT_STUN_END" );
	PHRASE_UTILITIES::sendMessage( _CreatorRowId, "OPS_EFFECT_STUN_END_E", _TargetRowId );
} // removed //

