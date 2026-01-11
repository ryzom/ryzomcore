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



#ifndef RY_ROOM_H
#define RY_ROOM_H

#include "game_share/lift_icons.h"

#include "mission_manager/ai_alias_translator.h"
#include "guild_manager/guild.h"


namespace LIFT_DESTS
{
	enum TDestType
	{
		GuildMain = 0,
		GuildAnnex,
		PlayerRoom,
		CommonRoom,
		Uninstanciated,
		Unknown,
	};

	TDestType toDestType( const std::string & str );
}

namespace LIFT_RESTRICTION
{
	// restriction to have a valid destination
	enum TRestriction
	{
		Rm_Fight,
		Rm_Magic,
		Rm_Harvest,
		Rm_Craft,
		Unknown,
	};
	
	TRestriction toRestriction( const std::string & str );
}

/// a lift destination
struct CLiftDestination
{
	std::string					Name;
	/// id of the area
	uint16						Area;
	/// type of the destination
	LIFT_DESTS::TDestType		Type;
	/// name id to display on client
	std::string					PhraseId;
	/// icon  to display on client
	LIFT_ICONS::TLiftIcon		Icon;
	/// names of the bot
	std::vector< TAIAlias >		Bots;
	/// exit destination
	CLiftDestination*			Exit;
	/// true if pets are allowed
	bool						TeleportPets;

	/// guilds using this destination
	std::vector< uint32 >					Guilds;
	/// players using this destination
	std::vector< NLMISC::CEntityId >		Players;
	/// cells of the destination instances. 0 if invalid
	std::vector< sint32 >					InstanceCells;
	/// session of the destination
	uint16									Session;
	/// restriction to go to this destination
	std::vector<LIFT_RESTRICTION::TRestriction> Restrictions;
};


/**
 * A building room
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CRoom
{
public:
	CRoom()
		:_IsValid(false),_RefCount(0),_Destination(NULL),_IndexIndestination(0),_Guild( CGuild::InvalidGuildPtr ){}
	
	/// return true if the room is valid
	bool isValid()const{ return _IsValid; }
	/// set the next free room id
	void setNextFreeRoomId( uint id ){ _NextFreeRoomId = id; }
	/// get the next free room id
	uint32 getNextFreeRoomId()const{ return _NextFreeRoomId; }
	/// add a reference to this room
	void addRef(){ _RefCount++; }
	/// remove a reference to this room
	void remRef(){ _RefCount--; }
	/// return the reference count of the room
	uint getRefCount()const{ return _RefCount; }
	/// init the room
	void init( CLiftDestination * destination,uint indexIndestination,CGuild * guild, const NLMISC::CEntityId & player );
	/// release the room
	void release();
	/// get the parent destination
	CLiftDestination * getDestination(uint & index)const
	{
		index = _IndexIndestination;
		return _Destination;
	}
	CGuild * getGuild()const
	{
		if ( _Guild != CGuild::InvalidGuildPtr )
			return _Guild;
		return NULL;
	}
	const NLMISC::CEntityId & getPlayer()const
	{
		return _Player;
	}

private:
	/// true if the room is valid
	bool							_IsValid;
	/// id of the next free room
	uint32							_NextFreeRoomId;
	/// reference count of the room ( number of people in it )
	uint							_RefCount;
	/// pointer on the parent destination
	CLiftDestination*				_Destination;
	/// index of the room in the parent
	uint32							_IndexIndestination;
	/// referenced guild
	NLMISC::CSmartPtr<CGuild>		_Guild;
	///spawned NPCS
	std::vector<NLMISC::CEntityId>	_NPCs;
	/// owner player
	NLMISC::CEntityId				_Player;
};


#endif // RY_ROOM_H

/* End of room.h */

