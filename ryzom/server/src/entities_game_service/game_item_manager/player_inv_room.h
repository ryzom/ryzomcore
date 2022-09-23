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


#ifndef PLAYER_INV_ROOM_H
#define PLAYER_INV_ROOM_H

#include "game_item_manager/game_item.h"
#include "game_item_manager/player_inventory.h"

/** Player room inventory */
class CPlayerRoomInventory : public CInventoryBase
{
public:
	/// ctor
	CPlayerRoomInventory(CCharacter * owner);

	//@{
	//@name Overloads from inventory base
	virtual uint32 getMaxBulk() const;
	virtual uint32 getMaxSlot() const;

	virtual TInventoryOpResult insertItem(CGameItemPtr &item, uint32 slot = INVENTORIES::INSERT_IN_FIRST_FREE_SLOT, bool autoStack = false);
	virtual CGameItemPtr removeItem(uint32 slot, uint32 quantity = INVENTORIES::REMOVE_MAX_STACK_QUANTITY, TInventoryOpResult * res = NULL);
	//@}

	/// return true if the given character can use inventory
	//bool canUseInventory(CCharacter * c) const;

private:
	/// owner of the room
	CCharacter * const _Owner;
};

/** View for the player room inventory */
class CPlayerRoomInvView : public CCharacterInvView
{
public:
	/// The inventory information has changed (like total bulk or weight)
	virtual void onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags);
};

#endif
