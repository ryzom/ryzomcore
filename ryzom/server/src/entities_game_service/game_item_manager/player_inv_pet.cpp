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

#include "player_inv_pet.h"

#include "game_share/slot_equipment.h"
#include "player_manager/character.h"



/////////////////////////////////////////////////////////////
// CMektoubInventory
/////////////////////////////////////////////////////////////

CPetInventory::CPetInventory()
{
	// the bulk and weight are known after ticket loading, so remove limitations at loading
	_PetMaxWeight = 0xFFFFFFFF;
	_PetMaxBulk = 0xFFFFFFFF;

	_PetIndex = ~0;
}


void CPetInventory::initPetInventory(uint32 petIndex, uint32 petMaxWeight, uint32 petMaxBulk)
{
	_PetMaxWeight = petMaxWeight;
	_PetMaxBulk = petMaxBulk;
	_PetIndex = petIndex;
}


uint32 CPetInventory::getMaxWeight() const
{
	return _PetMaxWeight;
}

uint32 CPetInventory::getMaxBulk() const
{
	return _PetMaxBulk;
}

uint32 CPetInventory::getMaxSlot() const
{
	return INVENTORIES::NbPackerSlots;
}

//void CPetInventory::onItemChanged(uint32 slot)
//{
//	// update mek inventory
//	nlassert(false);
//}
//
//void CPetInventory::onInventoryChanged()
//{
//	nlassert(false);
//}


//-----------------------------------------------
//	onItemChanged
//
//-----------------------------------------------
void CPetInvView::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags)
{
	// Call the parent before the specific code
	CCharacterInvView::onItemChanged(slot, changeFlags);

} // onItemChanged




void CPetInvView::onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags)
{
	
}



