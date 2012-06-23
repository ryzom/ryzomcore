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


#ifndef RY_PLAYER_ROOM_H
#define RY_PLAYER_ROOM_H

#include "game_item_manager/game_item_manager.h"


class CBuildingPhysicalPlayer;
/**
 * A player room interface
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CPlayerRoomInterface
{
	NL_INSTANCE_COUNTER_DECL(CPlayerRoomInterface);
public:

	/// ctor
	CPlayerRoomInterface();

	/// dtor
	~CPlayerRoomInterface();

	/// persistence methods
	void store(CPersistentDataRecord & pdr) const;
	void apply(CPersistentDataRecord & pdr, CCharacter * owner);

	/// clear player room
	void clear();

	/// return the destination where the room physically is
	const CBuildingPhysicalPlayer * getBuilding()
	{
		if (_Data == NULL)
			return NULL;
		return _Data->Building;
	}

	/// init the room. Should be called when a user buys it
	void init(CCharacter * owner, CBuildingPhysicalPlayer * building);

	/// serial of the interface
//	void serial(NLMISC::IStream & f, CCharacter * owner, uint16 playerVersion);

	/// return true if char is allowed in the building
	bool isAllowedInBuilding(const CCharacter * owner, const CCharacter * user);

	/// return true if user can use inventory
	bool canUseInventory(const CCharacter * owner, const CCharacter * user) const;

	/// return the building inventory
	CInventoryPtr getInventory() const
	{
		if (_Data == NULL)
			return NULL;
		return _Data->Inventory;
	}

	/// return true if the player has a room
	bool isValid() const { return _Data != NULL; }


private:
	/// internal data accessed through the interface
	class CPlayerRoomData
	{
	public:
		DECLARE_PERSISTENCE_METHODS

		/// ctor
		CPlayerRoomData(CCharacter * owner, CBuildingPhysicalPlayer * building);

		/// initialize the inventory
		void initInventory(CCharacter * owner);

		/// clear
		void clear();

		/// serial
//		void serial(NLMISC::IStream & f, CCharacter * owner, uint16 playerVersion);

	public:
		/// the room inventory
		CInventoryPtr Inventory;

		/// the building where the room is
		CBuildingPhysicalPlayer * Building;
	};

	CPlayerRoomData * _Data;
};



#endif // RY_PLAYER_ROOM_H

/* End of player_room.h */
