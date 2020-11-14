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

#ifndef RY_ROOM_INSTANCE_H
#define RY_ROOM_INSTANCE_H

#include "egs_pd.h"

class CCharacter;
class IBuildingPhysical;


/**
 * interface for ingame room.
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class IRoomInstance
{
public:
	/// ctor
	inline IRoomInstance();
	/// remove a user from the room
	virtual void removeUser( CCharacter* user ) = 0;
	/// add a user in the room
	virtual void addUser( CCharacter* user, CCharacter* owner ) = 0;
	/// create the room from a building
	virtual bool create( IBuildingPhysical * building, uint16 roomIdx, uint16 ownerIdx , sint32 cell);
	/// return true if the room is valid
	inline bool isValid()const;
	/// return the building associated to the room
	inline IBuildingPhysical * getBuilding()const;
	/// get the room instance owner index in the building
	inline uint16 getOwnerIndex()const;
	/// get the index of the room in the template
	inline uint16 getRoomIndex()const;
	/// get the bots in the room
	inline const std::vector<TDataSetRow>& getBots()const;

	/// get room description
	virtual std::string getRoomDescription() const = 0;

protected:
	/// release the room
	void release();
	/// index of the room in the building
	uint16				_RoomIdx;
	/// index of the owner in the building
	uint16				_OwnerIndex;
	/// reference count ( = number of users in the building )
	uint16				_RefCount;
	/// building
	IBuildingPhysical*	_Building;
	/// bots in the room
	std::vector<TDataSetRow> _Bots;
};

/// a guild room
class CRoomInstanceCommon : public IRoomInstance
{
	NL_INSTANCE_COUNTER_DECL(CRoomInstanceCommon);
public:

	virtual std::string getRoomDescription() const;

private:
	virtual void removeUser( CCharacter* user );
	virtual void addUser( CCharacter* user, CCharacter* owner );
};

/// a guild room
class CRoomInstanceGuild : public IRoomInstance
{
	NL_INSTANCE_COUNTER_DECL(CRoomInstanceGuild);
public:

	EGSPD::TGuildId getGuildId()const
	{
		return _GuildId;
	}
	EGSPD::TGuildId getGuildId(){ return _GuildId; }

	virtual std::string getRoomDescription() const;

private:
	virtual bool create( IBuildingPhysical * building, uint16 roomIdx, uint16 ownerIdx , sint32 cell);
	virtual void removeUser( CCharacter* user );
	virtual void addUser( CCharacter* user, CCharacter* owner );

	EGSPD::TGuildId	_GuildId;
};

/// a player room
class CRoomInstancePlayer : public IRoomInstance
{
	NL_INSTANCE_COUNTER_DECL(CRoomInstancePlayer);
public:

	inline const NLMISC::CEntityId & getPlayer()const;

	virtual std::string getRoomDescription() const;

private:
	virtual bool create( IBuildingPhysical * building, uint16 roomIdx, uint16 ownerIdx , sint32 cell);
	virtual void removeUser( CCharacter* user );
	virtual void addUser( CCharacter* user, CCharacter* owner );
	/// owner player
	NLMISC::CEntityId				_Player;
};

//----------------------------------------------------------------------------
inline IRoomInstance::IRoomInstance()
:_RoomIdx(0xFFFF),_RefCount(0)
{
}

//----------------------------------------------------------------------------
inline bool IRoomInstance::isValid()const
{
	return _RefCount != 0;
}

//----------------------------------------------------------------------------
IBuildingPhysical * IRoomInstance::getBuilding()const
{
	return _Building;
}

//----------------------------------------------------------------------------
inline uint16 IRoomInstance::getOwnerIndex()const
{
	return _OwnerIndex;
}

//----------------------------------------------------------------------------
inline uint16 IRoomInstance::getRoomIndex()const 
{
	return _RoomIdx;
}

//----------------------------------------------------------------------------
inline const NLMISC::CEntityId & CRoomInstancePlayer::getPlayer()const
{
	return _Player;
}

//----------------------------------------------------------------------------
inline const std::vector<TDataSetRow>&  IRoomInstance::getBots()const
{
	return _Bots;
}

#endif // RY_ROOM_INSTANCE_H

/* End of room_instance.h */
