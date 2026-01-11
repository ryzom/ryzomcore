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

#include "player_inv_hotbar.h"

#include "game_share/slot_equipment.h"
#include "player_manager/character.h"


/////////////////////////////////////////////////////////////
// CHotbarInventory
/////////////////////////////////////////////////////////////

// ****************************************************************************
uint32 CHotbarInventory::getMaxSlot() const
{
	return INVENTORIES::NbHotbarSlots;
}

// ****************************************************************************
void CHotbarInvView::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags)
{
	// Call the parent before the specific code
	CCharacterInvView::onItemChanged(slot, changeFlags);

}

// ****************************************************************************
void CHotbarInvView::onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags)
{
}

// ****************************************************************************
void CHotbarInvView::updateClientSlot(uint32 clientSlot, const CGameItemPtr item)
{
	nlassert(getCharacter());
	nlassert(getInventory());

	// do nothing else if client is not ready
	if (!getCharacter()->getEnterFlag())
		return;

	if (item != NULL)
		CBankAccessor_PLR::getINVENTORY().getHOTBAR().getArray(clientSlot).setINDEX_IN_BAG(getCharacter()->_PropertyDatabase, checkedCast<uint16>(item->getInventorySlot()+1));
	else
		CBankAccessor_PLR::getINVENTORY().getHOTBAR().getArray(clientSlot).setINDEX_IN_BAG(getCharacter()->_PropertyDatabase, 0);
}
