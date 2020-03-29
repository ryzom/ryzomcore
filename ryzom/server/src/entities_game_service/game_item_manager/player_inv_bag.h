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


#ifndef PLAYER_INV_BAG_H
#define PLAYER_INV_BAG_H

#include "game_item_manager/game_item.h"
#include "game_item_manager/player_inventory.h"

/** Bag inventory */
class CBagInventory : public CInventoryBase
{
public:
	
	//@{
	//@name Overloads from inventory base
	/// Return the max bulk according to player capacity
	uint32 getMaxBulk() const;

	uint32 getMaxSlot() const;
};

/** View for the bag inventory */
class CBagInvView : public CCharacterInvView
{
public:
	virtual void onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags);
	/// The inventory information has changed (like total bulk or weight)
	virtual void onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags);
};

#endif
