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
//
#include "combat_action_stun.h"
#include "phrase_utilities_functions.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


extern CGenericXmlMsgHeaderManager	GenericMsgManager;


//--------------------------------------------------------------
//					build()  
//--------------------------------------------------------------
bool CCombatActionStun::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint &brickIndex, CCombatPhrase * phrase )
{
	if (!phrase) return false;

	_CombatPhrase = phrase;
	_ActorRowId = actorRowId;

	return true;
} // build //


//--------------------------------------------------------------
//					apply()  
//--------------------------------------------------------------
void CCombatActionStun::apply(CCombatPhrase *phrase)
{
	CEntityBase *entity = CEntityBaseManager::getEntityBasePtr(_TargetRowId);
	if (!entity)
		return;

	TGameCycle endDate = _StunDuration + CTickEventHandler::getGameCycle();

	_StunEffect = new CCombatStunEffect( _ActorRowId, _TargetRowId, EFFECT_FAMILIES::CombatStun, _StunLevel, endDate);
	if (!_StunEffect)
	{
		nlwarning("<CCombatActionStun::apply> Failed to allocate new CCombatStunEffect object !");
		return;
	}

	_StunEffect->stunnedEntity(entity);
	entity->stun();
	entity->addSabrinaEffect(_StunEffect);

	// send stun impulsion to the IOS
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&> (entity->getId()) );
	CBitMemStream bms;
	GenericMsgManager.pushNameToStream( "STUN:STUN", bms);
	uint16 tickCount = _StunDuration;
	bms.serial( tickCount );
	msgout.serialMemStream( bms );
	CUnifiedNetwork::getInstance()->send( entity->getId().getDynamicId(), msgout );

	// send message
	PHRASE_UTILITIES::sendSimpleMessage( _TargetRowId, "OPS_EFFECT_STUN_BEGIN" );
	PHRASE_UTILITIES::sendMessage( _ActorRowId, "OPS_EFFECT_STUN_BEGIN_E", _TargetRowId );

	///  todo : send to spectators
} // apply //
