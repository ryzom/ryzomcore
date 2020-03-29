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

#include "player_inv_hand.h"

#include "game_share/slot_equipment.h"
#include "player_manager/character.h"
#include "egs_sheets/egs_sheets.h"

extern NLMISC::CVariable<uint32>	MaxPlayerBulk;

/////////////////////////////////////////////////////////////
// CHanlingInventory
/////////////////////////////////////////////////////////////

// ****************************************************************************
uint32 CHandlingInventory::getMaxSlot() const
{
	return INVENTORIES::NB_HAND_SLOT;
}


// ****************************************************************************
// Take care this code is the same as in CEquipInventory : find a better solution
float CHandlingInventory::getWearMalus()
{
	float fWearEquipmentMalus = 0.0f;

	for (uint i = 0; i < getSlotCount(); ++i)
	{
		CGameItemPtr pItem = getItem(i);
		if (pItem != NULL)
		{
			const CStaticItem *pForm = CSheets::getForm(pItem->getSheetId());
			if (pForm != NULL)
				fWearEquipmentMalus += pForm->WearEquipmentMalus;
			else
				deleteItem(i);
		}
	}

	return fWearEquipmentMalus;
}

/////////////////////////////////////////////////////////////
// CHandlingInvView
/////////////////////////////////////////////////////////////

// ****************************************************************************
void CHandlingInvView::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags)
{
	if (changeFlags.checkEnumValue(INVENTORIES::itc_inserted))
	{
		const CGameItemPtr item = getInventory()->getItem(slot);
		nlassert(item != NULL);
		
		updateClientSlot(slot, item);

		const CStaticItem * form = CSheets::getForm( item->getSheetId() );
		if( !form )
		{
			nlwarning("Tryng to equip with an item but can't find sheet, problem!");
			return;
		}
		getCharacter()->addWearMalus(form->WearEquipmentMalus);
		
		getCharacter()->applyItemModifiers(item);
		
		// if equip right hand item, compute parry level and disengage if player is engaged in combat
		if (slot == INVENTORIES::right )
			getCharacter()->updateParry(form->Family, form->Skill);
	}

	if (changeFlags.checkEnumValue(INVENTORIES::itc_enchant))
	{
		const CGameItemPtr item = getInventory()->getItem(slot);
		nlassert(item != NULL);
		
		updateClientSlot(slot, item);
	}

	if (changeFlags.checkEnumValue(INVENTORIES::itc_removed))
	{
		// Cleanup the item in player inventory
		updateClientSlot(slot, NULL);
	}
}

// ****************************************************************************
void CHandlingInvView::onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags)
{
}

// ****************************************************************************
void CHandlingInvView::updateClientSlot(uint32 clientSlot, const CGameItemPtr item)
{
	nlassert(getCharacter());
	nlassert(getInventory());

	if (item != NULL)
	{
		// equip
		getCharacter()->updateVisualInformation( INVENTORIES::UNDEFINED, 0, getInventory()->getInventoryId(), uint16(clientSlot), item->getSheetId(), item );
	}
	else
	{
		// unequip
		getCharacter()->updateVisualInformation( getInventory()->getInventoryId(), uint16(clientSlot), INVENTORIES::UNDEFINED, 0, NLMISC::CSheetId::Unknown, 0 );
	}
	
	// do nothing else if client is not ready
	if (!getCharacter()->getEnterFlag())
		return;
	
	if (item != NULL)
//		getCharacter()->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:HAND:%u:INDEX_IN_BAG", clientSlot ).c_str(), item->getInventorySlot() + 1 );
		CBankAccessor_PLR::getINVENTORY().getHAND().getArray(clientSlot).setINDEX_IN_BAG(getCharacter()->_PropertyDatabase, checkedCast<uint16>(item->getInventorySlot()+1));
	else
//		getCharacter()->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:HAND:%u:INDEX_IN_BAG", clientSlot ).c_str(), 0 );
		CBankAccessor_PLR::getINVENTORY().getHAND().getArray(clientSlot).setINDEX_IN_BAG(getCharacter()->_PropertyDatabase, 0);
}
