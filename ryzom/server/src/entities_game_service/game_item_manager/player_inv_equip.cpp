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

#include "player_inv_equip.h"

#include "game_share/slot_equipment.h"
#include "player_manager/character.h"
#include "egs_sheets/egs_sheets.h"

extern NLMISC::CVariable<uint32>	MaxPlayerBulk;

/////////////////////////////////////////////////////////////
// CEquipInventory
/////////////////////////////////////////////////////////////

// ****************************************************************************
uint32 CEquipInventory::getMaxSlot() const
{
	return SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT;
}

// ****************************************************************************
// Take care this code is the same as in CHandlingInventory : find a better solution
float CEquipInventory::getWearMalus()
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
// CEquipInvView
/////////////////////////////////////////////////////////////

// ****************************************************************************
void CEquipInvView::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags)
{
	nlassert(getCharacter() != NULL);
	nlassert(getInventory() != NULL);

	// newly equipped with item
	if ( changeFlags.checkEnumValue(INVENTORIES::itc_inserted) )
	{
		const CGameItemPtr item = getInventory()->getItem(slot);
		nlassert(item != NULL);
		
		updateClientSlot(slot, item);

		const CStaticItem * form = CSheets::getForm( item->getSheetId() );
		if( form )
		{
			getCharacter()->addWearMalus(form->WearEquipmentMalus);
		}
		
		getCharacter()->applyItemModifiers(item);
		
		// if equipped item is a jewel, re-compute max protection and resistance
		if( form )
		{
			if( form->Family == ITEMFAMILY::JEWELRY )
			{
				getCharacter()->updateMagicProtectionAndResistance();
			}
		}

	}

	if ( changeFlags.checkEnumValue(INVENTORIES::itc_enchant) )
	{
		const CGameItemPtr item = getInventory()->getItem(slot);
		nlassert(item != NULL);
		
		updateClientSlot(slot, item);
	}
	
	if (changeFlags.checkEnumValue(INVENTORIES::itc_removed))
	{
		const CGameItemPtr item = getInventory()->getItem(slot);
		// Cleanup the item in player inventory
		updateClientSlot(slot, NULL);
		if( item != NULL )
		{
			const CStaticItem * form = CSheets::getForm( item->getSheetId() );
			if( form )
			{
				if( form->Family == ITEMFAMILY::JEWELRY )
				{
					getCharacter()->updateMagicProtectionAndResistance();
				}					
			}
		}
	}
}

// ****************************************************************************
void CEquipInvView::onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags)
{
}

// ****************************************************************************
void CEquipInvView::updateClientSlot(uint32 clientSlot, const CGameItemPtr item)
{
	nlassert(getCharacter());
	nlassert(getInventory());

	if (item != NULL)
	{
		//  equip
		getCharacter()->updateVisualInformation( INVENTORIES::UNDEFINED, 0, getInventory()->getInventoryId(), uint16(clientSlot), item->getSheetId(), item );
	}
	else
	{
		// unequip
		getCharacter()->updateVisualInformation( getInventory()->getInventoryId(), uint16(clientSlot), INVENTORIES::UNDEFINED, 0, NLMISC::CSheetId::Unknown, NULL );
	}

	// do nothing else if client is not ready
	if (!getCharacter()->getEnterFlag())
		return;

	if (item != NULL)
//		getCharacter()->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:EQUIP:%u:INDEX_IN_BAG", clientSlot ).c_str(), item->getInventorySlot() + 1 );
		CBankAccessor_PLR::getINVENTORY().getEQUIP().getArray(clientSlot).setINDEX_IN_BAG(getCharacter()->_PropertyDatabase, checkedCast<uint16>(item->getInventorySlot()+1));
	else
//		getCharacter()->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:EQUIP:%u:INDEX_IN_BAG", clientSlot ).c_str(), 0 );
		CBankAccessor_PLR::getINVENTORY().getEQUIP().getArray(clientSlot).setINDEX_IN_BAG(getCharacter()->_PropertyDatabase, 0);
}

