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


#ifndef PLAYER_INV_EQUIP_H
#define PLAYER_INV_EQUIP_H

#include "game_item_manager/game_item.h"
#include "game_item_manager/player_inventory.h"

/** Equipment inventory (reference inventory) */
class CEquipInventory : public CRefInventory
{
public:
	//@{
	//@name Overloads from inventory base
	/// Return the max slot for equipment
	uint32 getMaxSlot() const;

	/// get total wear malus for equip inv
	float getWearMalus();
};

/** View for the equipement inventory */
class CEquipInvView : public CCharacterInvView
{
public:
	virtual void onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags);
	/// The inventory information has changed (like total bulk or weight)
	virtual void onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags);

protected:
	/** Update the given slot on the client with item infos.
	 *  If item is NULL, slot is updated as empty on the client.
	 */
	virtual void updateClientSlot(uint32 clientSlot, const CGameItemPtr item);

};

#endif
