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
#include "nel/net/service.h"
#include "building_manager/room_instance.h"
#include "player_manager/character.h"
#include "creature_manager/creature_manager.h"
#include "guild_manager/guild.h"
#include "building_manager/building_physical.h"
#include "primitives_parser.h"
#include "guild_manager/guild_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CRoomInstanceCommon);
NL_INSTANCE_COUNTER_IMPL(CRoomInstanceGuild);
NL_INSTANCE_COUNTER_IMPL(CRoomInstancePlayer);

//----------------------------------------------------------------------------
void CRoomInstanceCommon::removeUser( CCharacter* user )
{
	BOMB_IF( !user, "<BUILDING> null character!", return );

	CBuildingPhysicalCommon * commonBuilding = dynamic_cast<CBuildingPhysicalCommon *>( _Building );
	BOMB_IF( !commonBuilding, "<BUILDING> building type does not match with room type", return );

	if ( !commonBuilding->removeUser(user->getEntityRowId()) )
	{
		nlwarning("<BUILDING> trying to remove player %s from building '%s' but he is not in!",
			user->getId().toString().c_str(), commonBuilding->getName().c_str()
			);
		return;
	}

	--_RefCount;
	if ( _RefCount == 0 )
	{
		commonBuilding->resetRoomCell( _RoomIdx );
		release();
	}
}

//----------------------------------------------------------------------------
void CRoomInstanceCommon::addUser( CCharacter* user, CCharacter* owner )
{
	BOMB_IF( !user, "<BUILDING> null character!", return );

	++_RefCount;
}

//----------------------------------------------------------------------------
std::string CRoomInstanceCommon::getRoomDescription() const
{
	return toString("common room [index=%hu, owner index=%hu, nb users=%hu]", _RoomIdx, _OwnerIndex, _RefCount);
}

//----------------------------------------------------------------------------
void CRoomInstanceGuild::removeUser( CCharacter* user )
{
	BOMB_IF( !user, "<BUILDING> null character!", return );

	// close guild inventory window
	PlayerManager.sendImpulseToClient(user->getId(), "GUILD:CLOSE_INVENTORY");

	CBuildingPhysicalGuild * guildBuilding = dynamic_cast<CBuildingPhysicalGuild *>( _Building );
	BOMB_IF( !guildBuilding, "<BUILDING> building type does not match with room type", return );

	if ( !guildBuilding->removeUser(user->getEntityRowId()) )
	{
		nlwarning("<BUILDING> trying to remove player %s from building '%s' but he is not in!",
			user->getId().toString().c_str(), guildBuilding->getName().c_str()
			);
		return;
	}

	user->sendUrl("app_ryzhome action=quit_guild_room&room_name="+guildBuilding->getName(), "");

	--_RefCount;
	if ( _RefCount == 0 )
	{
		guildBuilding->resetRoomCell( _RoomIdx, _GuildId );
		release();
	}
}

//----------------------------------------------------------------------------
void CRoomInstanceGuild::addUser( CCharacter* user, CCharacter* owner )
{
	BOMB_IF( !user, "<BUILDING> null character!", return );

	CBuildingPhysicalGuild * guildBuilding = dynamic_cast<CBuildingPhysicalGuild *>( _Building );
	BOMB_IF( !guildBuilding, "<BUILDING> building type does not match with room type", return );

	// open guild inventory window
	PlayerManager.sendImpulseToClient(user->getId(), "GUILD:OPEN_INVENTORY");

	user->sendUrl("app_ryzhome action=open_guild_room&owner="+ owner->getName().toString()+"&room_name="+guildBuilding->getName(), "");

	++_RefCount;
}

//----------------------------------------------------------------------------
std::string CRoomInstanceGuild::getRoomDescription() const
{
	string guildName;
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( _GuildId );
	if (guild)
		guildName = guild->getName().toUtf8();

	return toString("guild room [index=%hu, owner index=%hu, nb users=%hu, guild name='%s' id=%u]",
		_RoomIdx, _OwnerIndex, _RefCount, guildName.c_str(), _GuildId);
}

//----------------------------------------------------------------------------
void CRoomInstancePlayer::removeUser( CCharacter* user )
{
	BOMB_IF( !user, "<BUILDING> null character!", return );

	// close room inventory window
	PlayerManager.sendImpulseToClient(user->getId(), "ITEM:CLOSE_ROOM_INVENTORY");

	CBuildingPhysicalPlayer * playerBuilding = dynamic_cast<CBuildingPhysicalPlayer *>( _Building );
	BOMB_IF( !playerBuilding, "<BUILDING> building type does not match with room type", return );

	if ( !playerBuilding->removeUser(user->getEntityRowId()) )
	{
		nlwarning("<BUILDING> trying to remove player %s from building '%s' but he is not in!",
			user->getId().toString().c_str(), playerBuilding->getName().c_str()
			);
		return;
	}

	user->sendUrl("app_ryzhome action=quit_player_room&room_name="+playerBuilding->getName(), "");
	
	--_RefCount;
	if ( _RefCount == 0 )
	{
		playerBuilding->resetRoomCell( _RoomIdx , user->getInRoomOfPlayer());
		release();
	}
	user->setInRoomOfPlayer(CEntityId::Unknown);
}

//----------------------------------------------------------------------------
void CRoomInstancePlayer::addUser( CCharacter* user, CCharacter* owner )
{
	BOMB_IF( !user, "<BUILDING> null character!", return );

	CBuildingPhysicalPlayer * playerBuilding = dynamic_cast<CBuildingPhysicalPlayer *>( _Building );
	BOMB_IF( !playerBuilding, "<BUILDING> building type does not match with room type", return );

	// open room inventory window
	PlayerManager.sendImpulseToClient(user->getId(), "ITEM:OPEN_ROOM_INVENTORY");
	if (owner)
	{
		owner->removeRoomAccesToPlayer(user->getId(),false);
		user->setInRoomOfPlayer(owner->getId());
	}
	else
	{
		// Very rare case
		owner = user;
	}
	user->sendUrl("app_ryzhome action=open_player_room&owner="+ owner->getName().toString()+"&room_name="+playerBuilding->getName(), "");

	++_RefCount;
}

//----------------------------------------------------------------------------
std::string CRoomInstancePlayer::getRoomDescription() const
{
	string charName;
	CCharacter * c = PlayerManager.getChar( _Player );
	if (c)
		charName = c->getName().toUtf8();

	return toString("player room [index=%hu, owner index=%hu, nb users=%hu, player name='%s' eid=%s]",
		_RoomIdx, _OwnerIndex, _RefCount, charName.c_str(), _Player.toString().c_str());
}

//----------------------------------------------------------------------------
bool IRoomInstance::create( IBuildingPhysical * building, uint16 roomIdx,uint16 ownerIdx, sint32 cellId )
{
	nlassert(building);
	nlassert(building->getTemplate());
	nlassert(roomIdx < building->getTemplate()->Rooms.size() );
	_Building = building;
	_RoomIdx = roomIdx;
	_OwnerIndex = ownerIdx;

	// spawn the bots
	const CRoomTemplate & templ = building->getTemplate()->Rooms[roomIdx];
	const uint size = (uint)templ.Bots.size();
	_Bots.reserve( size );
	for ( uint i = 0; i < size; i++ )
	{
		const CEntityId & eid = CAIAliasTranslator::getInstance()->getEntityId( templ.Bots[i] );
		if ( eid == CEntityId::Unknown )
		{
			nlwarning("<BUILDING> Invalid bot alias %s in building '%s'", CPrimitivesParser::aliasToString(templ.Bots[i]).c_str(), templ.Name.c_str() );
			continue;
		}
		CCreature * bot = CreatureManager.getCreature( eid );
		if ( bot == NULL )
		{
			nlwarning("<BUILDING> Invalid bot id '%s'%s in destination '%s'", eid.toString().c_str(), CPrimitivesParser::aliasToString(templ.Bots[i]).c_str(), templ.Name.c_str() );
			continue;
		}
		
		//allocate a new creature
		static uint64 id64 = 0;
		NLMISC::CEntityId entityId(RYZOMID::npc, id64++, TServiceId8(NLNET::IService::getInstance()->getServiceId()).get(), NLNET::TServiceId8(NLNET::IService::getInstance()->getServiceId()).get());
		nlinfo("<BUILDING> spawning bot alias %s, eid %s in building '%s'", CPrimitivesParser::aliasToString(templ.Bots[i]).c_str(), entityId.toString().c_str() ,templ.Name.c_str() );
		CCreature * bot2 = bot->getCreatureCopy(  entityId, cellId );
		nlassert( bot2 );
		_Bots.push_back( bot2->getEntityRowId() );
	}
	return true;
}

//----------------------------------------------------------------------------
void IRoomInstance::release()
{
	for (uint i = 0; i < _Bots.size(); i++)
	{
		CEntityId id = getEntityIdFromRow( _Bots[i] );
		CreatureManager.removeCreature(id);
		Mirror.removeEntity(id);
	}
}

//----------------------------------------------------------------------------
bool CRoomInstanceGuild::create( IBuildingPhysical * building, uint16 roomIdx,uint16 ownerIdx,sint32 cellId )
{
	nlassert(building);
	CBuildingPhysicalGuild * guildBuilding = dynamic_cast<CBuildingPhysicalGuild*>(building);
	nlassert(guildBuilding);
	if ( !IRoomInstance::create(building,roomIdx,ownerIdx,cellId) )
		return false;
	_GuildId = guildBuilding->getOwnerGuildId( ownerIdx );
	if( !_GuildId )
		return false;
	return true;
}

//----------------------------------------------------------------------------
bool CRoomInstancePlayer::create( IBuildingPhysical * building, uint16 roomIdx,uint16 ownerIdx,sint32 cellId )
{
	nlassert(building);
	CBuildingPhysicalPlayer * playerBuilding = dynamic_cast<CBuildingPhysicalPlayer*>(building);
	nlassert(playerBuilding);
	if( !IRoomInstance::create(building,roomIdx,ownerIdx,cellId) )
		return false;
	_Player = playerBuilding->getPlayer( ownerIdx );
	if( _Player == CEntityId::Unknown )
		return false;
	return true;
}
