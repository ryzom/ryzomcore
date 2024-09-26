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


#ifndef PLAYER_INV_AUTO_RESIZE_H
#define PLAYER_INV_AUTO_RESIZE_H

#include "game_item_manager/game_item.h"
#include "game_item_manager/player_inventory.h"

/** auto resize inventory */
class CAutoResizeInventory : public CInventoryBase
{
public:
	//@{
	//@name Overloads from inventory base
	/// Insert an item in the inventory, auto resize inventory if needed
	virtual TInventoryOpResult insertItem(CGameItemPtr &item, uint32 slot = INVENTORIES::INSERT_IN_FIRST_FREE_SLOT, bool autoStack = false);
	/// NEVER use this method. ONLY used when loading inventory from save files.
	/// It does ignore bulk and weight limitations.
	virtual void forceLoadItem(CGameItemPtr &item, uint32 slot);
	//@}
};

#endif // PLAYER_INV_AUTO_RESIZE_H
