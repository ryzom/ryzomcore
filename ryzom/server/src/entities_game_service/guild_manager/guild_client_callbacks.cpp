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

#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"

#include "guild_char_proxy.h"
#include "guild_member_module.h"
#include "guild_invitation_module.h"
#include "guild_manager.h"
#include "guild.h"
#include "building_manager/room_instance.h"
#include "building_manager/building_manager.h"

using namespace std;
using namespace NLMISC;

// local macros
#define GET_CHAR( _id_ ) \
CCharacter * user = PlayerManager.getChar( _id_ ); \
if (!user){ \
nlwarning("<GUILD>'%s' is not a valid char. Cant process callback",_id_.toString().c_str()); \
return; }\

#define GET_GUILD_MODULE( _id_ ) \
	GET_CHAR( _id_ ) \
	CGuildMemberModule * module = NULL; \
	if ( !user->getModuleParent().getModule( module ) ) \
	{ \
		nlwarning("<GUILD>'%s' has no valid guild module Cant process callback",_id_.toString().c_str()); \
		return;\
	}\
	if (module->isGuildProxy()) \
	{ \
		nlwarning("<GUILD>'%s' is a proxy guild, no action allowed",_id_.toString().c_str()); \
		return;\
	}\


#define GET_INVITATION_MODULE( _id_ ) \
	GET_CHAR( _id_ ) \
	CGuildInvitationModule * module = NULL; \
	if ( !user->getModuleParent().getModule( module ) ){ \
		nlwarning("<GUILD>'%s' has no valid guild invitation module Cant process callback",_id_.toString().c_str()); \
	return; }\


//----------------------------------------------------------------------------
void cbClientGuildCreate( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildCreate);
	
	CEntityId eId;
	ucstring name,description;
	uint64 icon;
	msgin.serial( eId );
	msgin.serial( name );
	msgin.serial( icon );
	msgin.serial( description );

	// get the character and build a proxy from it
	GET_CHAR(eId);
	CGuildCharProxy proxy(user);
	CGuildManager::getInstance()->createGuild(proxy,name,icon,description);
}

//----------------------------------------------------------------------------
void cbClientGuildSetLeader( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildSetLeader);
	
	CEntityId eId;
	uint16 index;
	uint8 session;
	msgin.serial( eId,index,session );
	GET_GUILD_MODULE(eId);
	module->setLeader( index,session );
}

//----------------------------------------------------------------------------
void cbClientGuildSetGrade( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildSetGrade);
	
	CEntityId eId;
	uint16 index;
	uint8 grade;
	uint8 session;
	msgin.serial( eId,index,grade,session );
	GET_GUILD_MODULE(eId);
	if ( grade == EGSPD::CGuildGrade::Leader )
		module->setLeader( index,session );
	else
		module->setGrade(index,session, (EGSPD::CGuildGrade::TGuildGrade) grade);
}

//----------------------------------------------------------------------------
void cbClientGuildJoinInvitation( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildJoinInvitation);
	
	CEntityId eId;
	msgin.serial( eId );
	GET_GUILD_MODULE(eId);
	module->inviteTargetInGuild();
}

//----------------------------------------------------------------------------
void cbClientGuildAcceptJoinInvitation( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildAcceptJoinInvitation);
	
	CEntityId eId;
	msgin.serial( eId );
	GET_INVITATION_MODULE(eId);
	module->accept();
}

//----------------------------------------------------------------------------
void cbClientGuildRefuseJoinInvitation( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildRefuseJoinInvitation);
	
	CEntityId eId;
	msgin.serial( eId );
	GET_INVITATION_MODULE(eId);
	module->refuse();
}

//----------------------------------------------------------------------------
void cbClientGuildQuit( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildQuit);
	
	CEntityId eId;
	msgin.serial( eId );
	GET_GUILD_MODULE(eId);
	module->quitGuild();
}

//----------------------------------------------------------------------------
void cbClientGuildKickMember( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildKickMember);
	
	CEntityId eId;
	uint16 index;
	uint8 session;
	msgin.serial( eId,index,session );
	GET_GUILD_MODULE(eId);
	module->kickMember( index,session );
}

//----------------------------------------------------------------------------
void cbClientGuildPutMoney( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildPutMoney);

	CEntityId id;
	uint64 money;
	uint16 session;
	msgin.serial( id );
	msgin.serial( money );
	msgin.serial( session );
	CCharacter * character = PlayerManager.getChar(id);
	if ( !character )
	{
		nlwarning("<cbClientGuildPutMoney> Invalid user %s",id.toString().c_str());
		return;
	}
	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbClientGuildPutMoney> player Id %s not yet ready", id.toString().c_str() );
		return;
	}

	character->setAfkState(false);

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( character->getGuildId() );
	if ( !guild )
	{
		nlwarning("<cbClientGuildPutMoney> player %s : cannot find guild %u !",id.toString().c_str(),character->getGuildId());
		return;
	}
	guild->putMoney( character, money ,session );
}

//----------------------------------------------------------------------------
void cbClientGuildTakeMoney( NLNET::CMessage& msgin, const std::string & serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientGuildTakeMoney);

	CEntityId id;
	uint64 money;
	uint16 session;
	msgin.serial( id );
	msgin.serial( money );
	msgin.serial( session );
	CCharacter * character = PlayerManager.getChar(id);
	if ( !character )
	{
		nlwarning("<cbClientGuildTakeMoney> Invalid user %s",id.toString().c_str());
		return;
	}
	// check character is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbClientGuildTakeMoney> player Id %s not yet ready", id.toString().c_str() );
		return;
	}

	character->setAfkState(false);

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( character->getGuildId() );
	if ( !guild )
	{
		nlwarning("<cbClientGuildTakeMoney> player %s : cannot find guild %u !",id.toString().c_str(),character->getGuildId());
		return;
	}
	guild->takeMoney( character, money ,session );
}


#undef GET_CHAR
#undef GET_GUILD_MODULE
#undef GET_INVITATION_MODULE
