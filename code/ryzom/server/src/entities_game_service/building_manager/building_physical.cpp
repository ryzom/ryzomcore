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
#include "building_manager/building_physical.h"
#include "game_share/string_manager_sender.h"
#include "zone_manager.h"
#include "destination.h"
#include "building_manager/room_instance.h"
#include "building_manager/building_manager.h"
#include "player_manager/character.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "zone_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "primitives_parser.h"


using namespace NLMISC;
using namespace NLNET;
using namespace std;

NL_INSTANCE_COUNTER_IMPL(CBuildingPhysicalCommon);
NL_INSTANCE_COUNTER_IMPL(CBuildingPhysicalGuild);
NL_INSTANCE_COUNTER_IMPL(CBuildingPhysicalPlayer);
/*****************************************************************************/
//					IBuildingPhysical implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
bool IBuildingPhysical::build( const NLLIGO::IPrimitive* prim, CBuildingParseData & parseData )
{
	bool ret = true;
	std::string value;
	
	nlverify( prim->getPropertyByName("default_exit_spawn",value) );
	_DefaultExitSpawn = CZoneManager::getInstance().getTpSpawnZoneIdByName( value );
	if ( _DefaultExitSpawn == InvalidSpawnZoneId )
	{
		nlwarning("<BUILDING> :invalid spawn zone '%s'", value.c_str() );
		ret = false;
	}
	
	nlverify( prim->getPropertyByName("name",_Name) );
	
	// parse the children exits
	for ( uint i = 0; i < prim->getNumChildren(); i++ )
	{
		const NLLIGO::IPrimitive* child = NULL;
		if ( prim->getChild( child,i) && child && child->getPropertyByName("class",value) && value == "teleport_destination" )
		{
			nlverify( child->getPropertyByName("name",value) );
			CTPDestination * dest = new  CTPDestination(value);
			if ( !dest->build(child,NULL,parseData) )
				delete dest;
			else
				_Exits.push_back(dest);
		}
	}
	initRooms();
	return ret;
}

//----------------------------------------------------------------------------
bool IBuildingPhysical::addUser(CCharacter * user, uint16 roomIdx, uint16 ownerIdx, sint32 & cellId)
{
	/// simply get the cell matching the parameters
	if ( roomIdx >= _Rooms.size() )
	{
		nlwarning("<BUILDING>Invalid room %u count is %u",roomIdx,_Rooms.size() );
		return false;
	}
	if ( ownerIdx >= _Rooms[roomIdx].Cells.size() )
	{
		nlwarning("<BUILDING>Invalid owner idx %u count is %u",ownerIdx,_Rooms[roomIdx].Cells.size());
		return false;
	}


	if (user->currentHp() <= 0 )
	{
		nlwarning("<BUILDING>user %s is dead",user->getId().toString().c_str());
		return false;
	}

	CCharacter *owner;

	if (ownerIdx < _Players.size())
	{
		owner = PlayerManager.getChar(_Players[ownerIdx] );
	}
	else
	{
		owner = user;
	}


	// if the room is not already instanciated, we have to do it
	if ( _Rooms[roomIdx].Cells[ownerIdx] == 0 )
	{
		// create a new room of the appropriate type
		IRoomInstance * roomInstance =  CBuildingManager::getInstance()->allocateRoom(_Rooms[roomIdx].Cells[ownerIdx],_Template->Type);
		if ( roomIdx >= _Template->Rooms.size() )
		{
			nlwarning("<BUILDING>Invalid room idx %u count is %u. Mismatch between template and instance?",ownerIdx,_Template->Rooms.size());
			return false;
		}
		// init the room
		if( !roomInstance->create(this,roomIdx,ownerIdx, _Rooms[roomIdx].Cells[ownerIdx]) )
			return false;
		roomInstance->addUser(user, owner);
	}
	else
	{
		IRoomInstance * roomInstance =  CBuildingManager::getInstance()->getRoomInstanceFromCell(_Rooms[roomIdx].Cells[ownerIdx]);
		if ( roomInstance == NULL )
		{
			nlwarning("<BUILDING>%s invalid room cell %d.",user->getId().toString().c_str(),_Rooms[roomIdx].Cells[ownerIdx]);
			return false;
		}
		roomInstance->addUser(user, owner);
	}

	user->setBuildingExitZone( _DefaultExitSpawn );
	_UsersInside.push_back( user->getEntityRowId() );
	cellId =  _Rooms[roomIdx].Cells[ownerIdx];

	return true;
}

//----------------------------------------------------------------------------
bool IBuildingPhysical::isUserInsideBuilding( const TDataSetRow & user )
{
	for (uint32 i = 0; i < _UsersInside.size(); ++i )
	{
		if( _UsersInside[i] == user )
			return true;
	}
	return false;
}

/*****************************************************************************/
//					CBuildingPhysicalCommon implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
void CBuildingPhysicalCommon::getClientDescription(uint16 roomIdx, uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	if ( roomIdx >= _Template->Rooms.size() )
	{
		nlwarning("<BUILDING>%s ask for room %u count is %u, in building '%s'",user->getId().toString().c_str(),roomIdx,_Template->Rooms.size(), _Name.c_str() );
		textId = 0;
		icon = 0;
		return;
	}
	/// in a simple physical building, icons and text are stores in the template rooms
	icon = UINT64_CONSTANT(0x8000000000000000) + _Template->Rooms[roomIdx].Icon;
	textId = CZoneManager::getInstance().sendPlaceName( user->getEntityRowId(), _Template->Rooms[roomIdx].PhraseId );
}

//----------------------------------------------------------------------------
void CBuildingPhysicalCommon::initRooms()
{
	/// each room has a unique cell, as it can only be instanciated once
	_Rooms.resize( _Template->Rooms.size() );
	for ( uint i = 0; i < _Rooms.size(); i++ )
	{
		_Rooms[i].Cells.resize(1,0);
	}
}

//----------------------------------------------------------------------------
void CBuildingPhysicalCommon::dumpBuilding(NLMISC::CLog & log) const
{
	log.displayNL("<BUILDING_DUMP> CBuildingPhysicalCommon");
	log.displayNL("Name: %s, alias: %s", _Name.c_str(), CPrimitivesParser::aliasToString( _Alias ).c_str());

	for (uint i = 0; i < _UsersInside.size(); i++)
	{
		const TDataSetRow rowId = _UsersInside[i];
		CCharacter * c = PlayerManager.getChar( rowId );
		if ( !c )
		{
			log.displayNL("\tError: cannot find character with row id: %s", rowId.toString().c_str());
			continue;
		}

		const string charName = c->getName().toUtf8();
		const string charEId = c->getId().toString();

		CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, rowId, DSPropertyCELL);
		const sint32 cell = mirrorCell;

		IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
		if ( !room )
		{
			log.displayNL("\tError: character %s %s is in cell %d but no room was found", charName.c_str(), charEId.c_str(), cell);
			continue;
		}

		CRoomInstanceCommon * commonRoom = dynamic_cast<CRoomInstanceCommon *>(room);
		if ( !commonRoom )
		{
			log.displayNL("\tError: character %s %s is in cell %d but room is not a common room but a %s",
				charName.c_str(), charEId.c_str(), cell, room->getRoomDescription().c_str()
				);
			continue;
		}

		log.displayNL("\tCharacter %s %s is in cell %d, room desc: %s",
			charName.c_str(), charEId.c_str(), cell, room->getRoomDescription().c_str()
			);
	}
}


/*****************************************************************************/
//					CBuildingPhysicalGuild implementation
/*****************************************************************************/

//----------------------------------------------------------------------------
void CBuildingPhysicalGuild::getClientDescription(uint16 roomIdx, uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	CMirrorPropValueRO<TYPE_CELL> mirrorValue( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	const sint32 cell = mirrorValue;
	// if user is inside a building, icons and texts are found the same way as standard building
	if ( CBuildingManager::getInstance()->isRoomCell(cell) )
	{
		icon = UINT64_CONSTANT(0x8000000000000000) + _Template->Rooms[roomIdx].Icon;
		textId = CZoneManager::getInstance().sendPlaceName( user->getEntityRowId(), _Template->Rooms[roomIdx].PhraseId );
	}
	// otherwise, display the guild icon and guild name
	else
	{
		if ( ownerIndex >= _Guilds.size() )
		{
			nlwarning("<BUILDING>%s ask for guild room %u count is %u, in building '%s'",ownerIndex, _Guilds.size(), user->getId().toString().c_str(),_Name.c_str());
			textId = 0;
			icon = 0;
			return;
		}
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( _Guilds[ownerIndex] );
		if ( !guild )
		{
			nlwarning("<BUILDING>%s ask for guild %u. This guild is invalid",user->getId().toString().c_str(), _Guilds[ownerIndex] );
			textId = 0;
			icon = 0;
			return;
		}
		icon = guild->getIcon();
		static TVectorParamCheck params(1);
		params[0].Type = STRING_MANAGER::string_id;
		params[0].StringId = guild->getNameId();
		textId = STRING_MANAGER::sendStringToClient( user->getEntityRowId(),"GUILD_ROOM",params );
	}
}

//----------------------------------------------------------------------------
void CBuildingPhysicalGuild::onGuildDeletion(uint32 guild)
{
	uint guildIdx;
	for (guildIdx = 0; guildIdx < _Guilds.size(); guildIdx++)
	{
		if (_Guilds[guildIdx] == guild)
			break;
	}
	// return if guild not found
	if (guildIdx == _Guilds.size())
		return;

	// keep cells that will be deleted
	std::set<sint32> deletedCells;
	for (uint i = 0; i < _Rooms.size(); i++)
	{
		BOMB_IF( (guildIdx >= _Rooms[i].Cells.size()), "<BUILDING> trying to access a cell out of bound", return );
		if (CBuildingManager::getInstance()->isRoomCell(_Rooms[i].Cells[guildIdx]))
			deletedCells.insert(_Rooms[i].Cells[guildIdx]);
	}

	// remove players from the guild building that will be removed
	vector<TDataSetRow> usersInside = _UsersInside; // make a copy because users can be removed from _UsersInside by removePlayerFromRoom()
	for (uint i = 0; i < usersInside.size(); i++)
	{
		CCharacter * user = PlayerManager.getChar(usersInside[i]);
		if (!user || !TheDataset.isAccessible(user->getEntityRowId()))
			continue;

		CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, user->getEntityRowId(), DSPropertyCELL);
		if (deletedCells.find(mirrorCell()) != deletedCells.end())
		{
			const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(_DefaultExitSpawn);

			if ( zone == NULL )
			{
				nlwarning("<BUILDING> NULL default exit spawn");
				return;
			}

			
			sint32 x,y,z;
			float heading;
			zone->getRandomPoint(x,y,z,heading);
			user->tpWanted(x,y,z,true,heading);
		}
	}

	// remove the guild building
	_Guilds[guildIdx] = _Guilds.back();
	_Guilds.pop_back();
	for (uint i = 0; i < _Rooms.size(); i++)
	{
		BOMB_IF( (guildIdx >= _Rooms[i].Cells.size()), "<BUILDING> trying to delete a cell out of bound", return );
		STOP_IF( (_Rooms[i].Cells[guildIdx] != 0), "<BUILDING> deleting a guild building with players inside!" );
		_Rooms[i].Cells[guildIdx] = _Rooms[i].Cells.back();
		_Rooms[i].Cells.pop_back();
	}

	// state changed
	_StateCounter++;
}

//----------------------------------------------------------------------------
bool CBuildingPhysicalGuild::isUserAllowed(CCharacter * user, uint16 ownerId, uint16 roomIdx)
{
	nlassert( user );
	nlassert( ownerId < _Guilds.size() );
	nlassert( roomIdx < _Template->Rooms.size() );

	if (user->isDead())
		return false;

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
	if ( !guild )
		return false;
	if ( user->getGuildId() != _Guilds[ownerId] )
		return false;
//	for ( uint i = 0; i < _Template->Rooms[roomIdx].Restrictions.size(); i ++ )
//	{
//		switch(_Template->Rooms[roomIdx].Restrictions[i])
//		{
//		case ROOM_RESTRICTION::Rm_Craft:
//			if( !guild || !guild->hasRoleMaster( EGSPD::CSPType::Craft ) )
//				return false;
//			break;
//		case ROOM_RESTRICTION::Rm_Harvest:
//			if( !guild || !guild->hasRoleMaster( EGSPD::CSPType::Harvest ) )
//				return false;
//			break;
//		case ROOM_RESTRICTION::Rm_Magic:
//			if( !guild || !guild->hasRoleMaster( EGSPD::CSPType::Magic ) )
//				return false;
//			break;
//		case ROOM_RESTRICTION::Rm_Fight:
//			if( !guild || !guild->hasRoleMaster( EGSPD::CSPType::Fight ) )
//				return false;
//			break;
//		}
//	}
	return true;
}

//----------------------------------------------------------------------------
uint16 CBuildingPhysicalGuild::getOwnerCount()
{ 
	return (uint16)_Guilds.size();
}

//----------------------------------------------------------------------------
void CBuildingPhysicalGuild::initRooms()
{
	_Rooms.resize( _Template->Rooms.size() );
}

//----------------------------------------------------------------------------
void CBuildingPhysicalGuild::dumpBuilding(NLMISC::CLog & log) const
{
	log.displayNL("<BUILDING_DUMP> CBuildingPhysicalGuild");
	log.displayNL("Name: %s, alias: %s", _Name.c_str(), CPrimitivesParser::aliasToString( _Alias ).c_str());

	for (uint i = 0; i < _UsersInside.size(); i++)
	{
		const TDataSetRow rowId = _UsersInside[i];
		CCharacter * c = PlayerManager.getChar( rowId );
		if ( !c )
		{
			log.displayNL("\tError: cannot find character with row id: %s", rowId.toString().c_str());
			continue;
		}

		const string charName = c->getName().toUtf8();
		const string charEId = c->getId().toString();

		CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, rowId, DSPropertyCELL);
		const sint32 cell = mirrorCell;

		IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
		if ( !room )
		{
			log.displayNL("\tError: character %s %s is in cell %d but no room was found", charName.c_str(), charEId.c_str(), cell);
			continue;
		}

		CRoomInstanceGuild * guildRoom = dynamic_cast<CRoomInstanceGuild *>(room);
		if ( !guildRoom )
		{
			log.displayNL("\tError: character %s %s is in cell %d but room is not a guild room but a %s",
				charName.c_str(), charEId.c_str(), cell, room->getRoomDescription().c_str()
				);
			continue;
		}

		const uint32 guildId = c->getGuildId();
		string guildName;
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( guildId );
		if (guild)
			guildName = guild->getName().toUtf8();

		log.displayNL("\tCharacter %s %s [guild name='%s' id=%u] is in cell %d, room desc: %s",
			charName.c_str(), charEId.c_str(), guildName.c_str(), guildId, cell, room->getRoomDescription().c_str()
			);
	}

	for (uint i = 0; i < _Guilds.size(); i++)
	{
		const uint32 guildId = _Guilds[i];
		string guildName;
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( guildId );
		if (guild)
			guildName = guild->getName().toUtf8();

		log.displayNL("\t> Guild registered in building at index %u: %s", i, guildName.c_str());
	}
}


/*****************************************************************************/
//					CBuildingPhysicalPlayer implementation
/*****************************************************************************/


//----------------------------------------------------------------------------
void CBuildingPhysicalPlayer::getClientDescription(uint16 roomIdx, uint16 ownerIndex, CCharacter * user, uint64 & icon, uint32 & textId )const
{
#ifdef NL_DEBUG
	nlassert(user);
#endif
	/// send the template room icon and the player name
	if ( ownerIndex >= _Players.size() )
	{
		nlwarning("<BUILDING>%s ask for player room %u count is %u, in building '%s'",ownerIndex, _Players.size(), user->getId().toString().c_str(),_Name.c_str());
		textId = 0;
		icon = 0;
		return;
	}
	icon = UINT64_CONSTANT(0x8000000000000000) + _Template->Rooms[roomIdx].Icon;
	static TVectorParamCheck params(1);
	params[0].Type = STRING_MANAGER::player;
	params[0].setEIdAIAlias( _Players[ownerIndex], CAIAliasTranslator::getInstance()->getAIAlias(_Players[ownerIndex]) );
	textId = STRING_MANAGER::sendStringToClient( user->getEntityRowId(),"PLAYER_ROOM",params );
	
}

//----------------------------------------------------------------------------
void CBuildingPhysicalPlayer::onPlayerDeletion(const NLMISC::CEntityId & userId)
{
	uint playerIdx;
	for (playerIdx = 0; playerIdx < _Players.size(); playerIdx++)
	{
		if (_Players[playerIdx] == userId)
			break;
	}
	// return if player not found
	if (playerIdx == _Players.size())
		return;

	// keep cells that will be deleted
	std::set<sint32> deletedCells;
	for (uint i = 0; i < _Rooms.size(); i++)
	{
		BOMB_IF( (playerIdx >= _Rooms[i].Cells.size()), "<BUILDING> trying to access a cell out of bound", return );
		if (CBuildingManager::getInstance()->isRoomCell(_Rooms[i].Cells[playerIdx]))
			deletedCells.insert(_Rooms[i].Cells[playerIdx]);
	}

	// remove players from the player building that will be removed
	vector<TDataSetRow> usersInside = _UsersInside; // make a copy because users can be removed from _UsersInside by removePlayerFromRoom()
	for (uint i = 0; i < usersInside.size(); i++)
	{
		CCharacter * user = PlayerManager.getChar(usersInside[i]);
		if (!user || !TheDataset.isAccessible(user->getEntityRowId()))
			continue;

		CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, user->getEntityRowId(), DSPropertyCELL);
		if (deletedCells.find(mirrorCell()) != deletedCells.end())
		{
			const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(_DefaultExitSpawn);
			nlassert(zone);
			sint32 x,y,z;
			float heading;
			zone->getRandomPoint(x,y,z,heading);
			user->tpWanted(x,y,z,true,heading);
		}
	}

	// remove the player building
	_Players[playerIdx] = _Players.back();
	_Players.pop_back();
	for (uint i = 0; i < _Rooms.size(); i++)
	{
		BOMB_IF( (playerIdx >= _Rooms[i].Cells.size()), "<BUILDING> trying to delete a cell out of bound", return );
		STOP_IF( (_Rooms[i].Cells[playerIdx] != 0), "<BUILDING> deleting a player building with players inside!" );
		_Rooms[i].Cells[playerIdx] = _Rooms[i].Cells.back();
		_Rooms[i].Cells.pop_back();
	}

	// state changed
	_StateCounter++;
}

//----------------------------------------------------------------------------
bool CBuildingPhysicalPlayer::isUserAllowed(CCharacter * user, uint16 ownerId, uint16 roomIdx)
{
	nlassert(user);
	nlassert(ownerId < _Players.size() );

	if (user->isDead())
		return false;

	CCharacter * owner = PlayerManager.getChar( _Players[ownerId] );
	if (owner)
		return ( (user->getId() == _Players[ownerId]) || owner->playerHaveRoomAccess(user->getId()) );
	else
		return false;
}

//----------------------------------------------------------------------------
uint16 CBuildingPhysicalPlayer::getOwnerCount()
{ 
	return (uint16)_Players.size();
}

//----------------------------------------------------------------------------
void CBuildingPhysicalPlayer::initRooms()
{
	_Rooms.resize( _Template->Rooms.size() );
}

//----------------------------------------------------------------------------
void CBuildingPhysicalPlayer::dumpBuilding(NLMISC::CLog & log) const
{
	log.displayNL("<BUILDING_DUMP> CBuildingPhysicalPlayer");
	log.displayNL("Name: %s, alias: %s", _Name.c_str(), CPrimitivesParser::aliasToString( _Alias ).c_str());

	for (uint i = 0; i < _UsersInside.size(); i++)
	{
		const TDataSetRow rowId = _UsersInside[i];
		CCharacter * c = PlayerManager.getChar( rowId );
		if ( !c )
		{
			log.displayNL("\tError: cannot find character with row id: %s", rowId.toString().c_str());
			continue;
		}

		const string charName = c->getName().toUtf8();
		const string charEId = c->getId().toString();

		CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, rowId, DSPropertyCELL);
		const sint32 cell = mirrorCell;

		IRoomInstance * room = CBuildingManager::getInstance()->getRoomInstanceFromCell( cell );
		if ( !room )
		{
			log.displayNL("\tError: character %s %s is in cell %d but no room was found", charName.c_str(), charEId.c_str(), cell);
			continue;
		}

		CRoomInstancePlayer * playerRoom = dynamic_cast<CRoomInstancePlayer *>(room);
		if ( !playerRoom )
		{
			log.displayNL("\tError: character %s %s is in cell %d but room is not a player room but a %s",
				charName.c_str(), charEId.c_str(), cell, room->getRoomDescription().c_str()
				);
			continue;
		}

		log.displayNL("\tCharacter %s %s is in cell %d, room desc: %s",
			charName.c_str(), charEId.c_str(), cell, room->getRoomDescription().c_str()
			);
	}

	for (uint i = 0; i < _Players.size(); i++)
	{
		CEntityId id = _Players[i];
		CCharacter * c = PlayerManager.getChar( id );
		if ( !c )
		{
			log.displayNL("\tError: cannot find character with eid: %s", id.toString().c_str());
			continue;
		}

		const string charName = c->getName().toUtf8();
		const string charEId = c->getId().toString();

		log.displayNL("\t> Player registered in building at index %u: %s %s", i, charName.c_str(), charEId.c_str());
	}
}
