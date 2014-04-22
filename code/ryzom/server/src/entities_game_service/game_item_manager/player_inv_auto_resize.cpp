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

#include "player_inv_auto_resize.h"

// ****************************************************************************
CInventoryBase::TInventoryOpResult CAutoResizeInventory::insertItem(CGameItemPtr &item, uint32 slot, bool autoStack)
{
	if (slot == INVENTORIES::INSERT_IN_FIRST_FREE_SLOT)
	{
		if (getFreeSlotCount() == 0)
			setSlotCount(getSlotCount()+1);
	}
	else if (slot >= _Items.size())
	{
		setSlotCount(slot+1);
	}
	
	return CInventoryBase::insertItem(item,slot,autoStack);
}

// ****************************************************************************
void CAutoResizeInventory::forceLoadItem(CGameItemPtr &item, uint32 slot)
{
	if (slot == INVENTORIES::INSERT_IN_FIRST_FREE_SLOT)
	{
		if (getFreeSlotCount() == 0)
			setSlotCount(getSlotCount()+1);
	}
	else if (slot >= _Items.size())
	{
		setSlotCount(slot+1);
	}
	
	CInventoryBase::forceLoadItem(item,slot);
}
