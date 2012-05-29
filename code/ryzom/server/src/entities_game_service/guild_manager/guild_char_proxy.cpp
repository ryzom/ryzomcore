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

#include "player_manager/character.h"
#include "guild_manager/guild_char_proxy.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "creature_manager/creature.h"
#include "creature_manager/creature_manager.h"
#include "zone_manager.h"

//----------------------------------------------------------------------------
CGuildCharProxy::CGuildCharProxy()
:IModuleProxy<CCharacter>()
{
}

//----------------------------------------------------------------------------
CGuildCharProxy::CGuildCharProxy( IModuleCore * user )
:IModuleProxy<CCharacter>( user )
{
}

//----------------------------------------------------------------------------
const EGSPD::TCharacterId & CGuildCharProxy::getId() 
{
	return _ModuleCore->getId(); 
}

//----------------------------------------------------------------------------
const TDataSetRow & CGuildCharProxy::getRowId()
{
	return _ModuleCore->getEntityRowId(); 
}

//----------------------------------------------------------------------------
void CGuildCharProxy::cancelAFK()
{
	_ModuleCore->setAfkState(false);
}

//----------------------------------------------------------------------------
CCreature * CGuildCharProxy::getInterlocutor()
{
	return CreatureManager.getCreature( _ModuleCore->getCurrentInterlocutor() );
}

//----------------------------------------------------------------------------
CModuleParent & CGuildCharProxy::getModuleParent()
{
	return _ModuleCore->getModuleParent();
}

//----------------------------------------------------------------------------
void CGuildCharProxy::sendSystemMessage( const std::string &  msg, const TVectorParamCheck & params)
{
	CCharacter::sendDynamicSystemMessage( _ModuleCore->getEntityRowId(),msg,params );
}

//----------------------------------------------------------------------------
void CGuildCharProxy::sendDynamicMessageToChatGroup( const std::string &  msg, CChatGroup::TGroupType type, const TVectorParamCheck & params )
{
	CCharacter::sendDynamicMessageToChatGroup( _ModuleCore->getEntityRowId(),msg,type,params );
}

//----------------------------------------------------------------------------
uint64 CGuildCharProxy::getMoney()
{
	return _ModuleCore->getMoney();
}

//----------------------------------------------------------------------------
void CGuildCharProxy::spendMoney(uint64 money)
{
	_ModuleCore->spendMoney( money );
}

//----------------------------------------------------------------------------
bool CGuildCharProxy::isTrialPlayer()
{
	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
	BOMB_IF(p == NULL, "Failed to find player record for character: " << getId().toString(), return true);
	return p->isTrialPlayer();
}

//----------------------------------------------------------------------------
void CGuildCharProxy::endBotChat()
{
	_ModuleCore->endBotChat(false);
}

//----------------------------------------------------------------------------
bool CGuildCharProxy::getTarget(CGuildCharProxy & proxy)
{
	CCharacter * user = PlayerManager.getChar( _ModuleCore->getTarget() );
	if ( !user )
		return false;
	CGuildCharProxy tmp(user);
	proxy = user;
	return true;
}

//----------------------------------------------------------------------------
void CGuildCharProxy::setGuildId(uint32 guildId )
{
	_ModuleCore->setGuildId( guildId );
}

//----------------------------------------------------------------------------
uint32 CGuildCharProxy::getGuildId()
{
	return _ModuleCore->getGuildId();
}

//----------------------------------------------------------------------------
void CGuildCharProxy::updateTarget()
{
	_ModuleCore->updateTarget();
}

//----------------------------------------------------------------------------
uint16 CGuildCharProxy::getMainPlace()
{
	uint size = (uint)_ModuleCore->getPlaces().size();
	for ( uint i = 0; i < size; i++ )
	{
		const CPlace * place = CZoneManager::getInstance().getPlaceFromId( _ModuleCore->getPlaces()[i] );
		if ( place && place->isMainPlace() )
			return _ModuleCore->getPlaces()[i];
	}
	return 0xFF;
}

//----------------------------------------------------------------------------
void CGuildCharProxy::updateTargetingChars()
{
	_ModuleCore->updateTargetingChars();
}

//----------------------------------------------------------------------------
const TDataSetRow & CGuildCharProxy::getEntityRowId()
{
	return _ModuleCore->getEntityRowId();
}

//----------------------------------------------------------------------------
void CGuildCharProxy::tpWanted( sint32 x, sint32 y, sint32 z , bool useHeading, float heading , uint8 continent, sint32 cell)
{
	_ModuleCore->tpWanted(x,y,z,useHeading,heading,continent,cell);
}

//----------------------------------------------------------------------------
void CGuildCharProxy::updateOutpostAdminFlagInDB()
{
	_ModuleCore->updateOutpostAdminFlagInDB();
}

