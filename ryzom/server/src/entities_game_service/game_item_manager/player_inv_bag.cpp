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

#include "player_inv_bag.h"

#include "game_share/slot_equipment.h"
#include "player_manager/character.h"


extern NLMISC::CVariable<uint32>	MaxPlayerBulk;


/////////////////////////////////////////////////////////////
// CBagInventory
/////////////////////////////////////////////////////////////

// ****************************************************************************
uint32 CBagInventory::getMaxBulk() const
{
	return MaxPlayerBulk;
}

// ****************************************************************************
uint32 CBagInventory::getMaxSlot() const
{
	return INVENTORIES::NbBagSlots;
}

// ****************************************************************************
void CBagInvView::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags)
{
	// Call the parent before the specific code
	CCharacterInvView::onItemChanged(slot, changeFlags);

	// itc_hp
	// Cf. CGameItem::removeHp and CGameItem::addHp
/*	if (_Form && !ITEMFAMILY::destroyedWhenWorned(_Form->Family) )
	{
		const CEntityId itemOwner = getOwnerPlayer();
		if (itemOwner != CEntityId::Unknown)
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
			params[0].SheetId = _SheetId;
			PHRASE_UTILITIES::sendDynamicSystemMessage( TheDataset.getDataSetRow(itemOwner), ITEM_WORN_STATE::getMessageForState(ITEM_WORN_STATE::Worned), params);
		}
	}
*/

}

// ****************************************************************************
void CBagInvView::onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags)
{
}
