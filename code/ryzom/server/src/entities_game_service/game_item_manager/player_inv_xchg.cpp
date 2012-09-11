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

#include "player_inv_xchg.h"

#include "game_share/slot_equipment.h"
#include "player_manager/character.h"


using namespace NLMISC;
using namespace std;


/////////////////////////////////////////////////////////////
// CExchangeView
/////////////////////////////////////////////////////////////

const uint32 CExchangeView::NbExchangeSlots = 8;

// ****************************************************************************
CExchangeView::CExchangeView()
{
	_InterlocutorView = NULL;
	_ExchangeSlots.resize(NbExchangeSlots);
	_PackerTicketCount = 0;
	_MountTicketCount = 0;
}

// ****************************************************************************
CExchangeView::~CExchangeView()
{
}

// ****************************************************************************
void CExchangeView::clearExchangeView()
{
	for (uint i = 0; i < NbExchangeSlots; i++)
	{
		removeItemFromExchange(i);
	}
}

// ****************************************************************************
void CExchangeView::setInterlocutorView(CExchangeView * interlocutorView)
{
	_InterlocutorView = interlocutorView;
}

// ****************************************************************************
bool CExchangeView::putItemInExchange(uint32 bagSlot, uint32 exchangeSlot, uint32 quantity)
{
	nlassert(bagSlot < getInventory()->getSlotCount());
	nlassert(exchangeSlot < NbExchangeSlots);

	CGameItemPtr item = getInventory()->getItem(bagSlot);
	if (item == NULL)
		return false;

	quantity = std::min(quantity, item->getNonLockedStackSize());
	if (quantity == 0)
		return false;

	const CStaticItem * form = item->getStaticForm();
	if (form == NULL)
		return false;

	// if it is an exchange between 2 players
	// do not permit exchange of non dropable items, but pet animal ticket are NoDrop item but must ne exchangeable
	if (_InterlocutorView != NULL && (!form->DropOrSell && form->Family != ITEMFAMILY::PET_ANIMAL_TICKET))
		return false;

	// Can't trade items locked by owner
	if (item->getLockedByOwner())
		return false;

	// You cannot exchange genesis named items
	if (item->getPhraseId().find("genesis_") == 0)
	{
		nlwarning("Character %s tries to sell '%s'", getCharacter()->getId().toString().c_str(), item->getPhraseId().c_str() );
		return false;
	}

	if( getCharacter()->isAnActiveXpCatalyser(item) )
		return false;

	if (getExchangeItem(exchangeSlot) != NULL)
	{
		removeItemFromExchange(exchangeSlot);
	}

	// check the pet animal count of interlocutor
	if (isPetTicket(form->Type) && _InterlocutorView != NULL)
	{
		uint petIndex = getCharacter()->getAnimalByTicket(item);
		if (petIndex != ~0)
		{
			// check that the pet animal is empty
			if (!getCharacter()->checkPackAnimalEmptyInventory(petIndex))
			{
				CCharacter::sendDynamicSystemMessage(getCharacter()->getId(), "PACK_ANIMAL_INVENTORY_MUST_BE_EMPTY");
				return false;
			}
			// check that the pet animal is not mounted
			if (getCharacter()->getPlayerPets()[petIndex].IsMounted)
			{
				CCharacter::sendDynamicSystemMessage(getCharacter()->getId(), "ANIMAL_MOUNTED");
				return false;
			}
		}
		else
			nlwarning("Pet animal ticket not found for %s", getCharacter()->getId().toString().c_str());

		// update ticket count
		(*getPetTicketCount(form->Type))++;
	}

	_ExchangeSlots[exchangeSlot].BagSlot = bagSlot;
	_ExchangeSlots[exchangeSlot].Quantity = quantity;

	// lock the item in the bag while exchanging
	item->setLockCount(item->getLockCount() + quantity);

	updateExchangeSlot(exchangeSlot);
	return true;
}

// BRIANCODE - to support automatic building of mission object exchanges
// NOTE - sets the database to "accept" (validates transaction) on client!
// ****************************************************************************
bool CExchangeView::putItemInFirstEmptyExchangeSlot(uint32 bagSlot, uint32 quantity)
{
	nlassert(bagSlot < getInventory()->getSlotCount());

	bool foundSlot;
	uint32 exchangeSlot;

	foundSlot = false;
	for (exchangeSlot = 0; exchangeSlot < NbExchangeSlots; exchangeSlot++)
	{
		if (getExchangeItem(exchangeSlot) == NULL)
		{
			foundSlot = true;
			break;
		}
	}

	// only 8 slots in exchange bag, so unlikely, but possible.
	if (foundSlot == false)
		return false;

	string sDBPath = NLMISC::toString("EXCHANGE:GIVE:%u", exchangeSlot);

	CGameItemPtr item = getInventory()->getItem(bagSlot);
	if (item == NULL)
		return false;

	quantity = min(quantity, item->getNonLockedStackSize());
	if (quantity == 0)
		return false;

	if (getExchangeItem(exchangeSlot) != NULL)
	{
		removeItemFromExchange(exchangeSlot);
	}

	_ExchangeSlots[exchangeSlot].BagSlot = bagSlot;
	_ExchangeSlots[exchangeSlot].Quantity = quantity;

	// lock the item in the bag while exchanging
	item->setLockCount(item->getLockCount() + quantity);

	updateExchangeSlot(exchangeSlot);
//	getCharacter()->_PropertyDatabase.setProp("EXCHANGE:ACCEPTED", 1);
	CBankAccessor_PLR::getEXCHANGE().setACCEPTED(getCharacter()->_PropertyDatabase, true);
	return true;
}

// ****************************************************************************
bool CExchangeView::removeItemFromExchange(uint32 exchangeSlot, uint32 quantity)
{
	nlassert(exchangeSlot < NbExchangeSlots);

	if (quantity == 0)
		return false;

	uint32 exchangeQuantity;
	CGameItemPtr item = getExchangeItem(exchangeSlot, &exchangeQuantity);
	if (item == NULL)
	{
		_ExchangeSlots[exchangeSlot].reset();
		updateExchangeSlot(exchangeSlot);
		return true;
	}

	// quantity can be set to CInventoryBase::REMOVE_MAX_STACK_QUANTITY
	quantity = std::min(quantity, _ExchangeSlots[exchangeSlot].Quantity);

	const CStaticItem * form = item->getStaticForm();
	if (form == NULL)
		return false;

	// unlock item
	if (quantity > item->getLockCount())
	{
		nlwarning("<CExchangeView::removeItemFromExchange> Trying to remove %d %s(id=%s) from exchange view in slot %d but only %d are allowed to be removed", quantity, form->SheetId.toString().c_str(), item->getItemId().toString().c_str(), exchangeSlot, item->getLockCount());
		quantity = item->getLockCount();
	}
	item->setLockCount(item->getLockCount() - quantity);

	// update the exchange slot
	if (quantity == _ExchangeSlots[exchangeSlot].Quantity)
	{
		_ExchangeSlots[exchangeSlot].reset();
	}
	else
	{
		_ExchangeSlots[exchangeSlot].Quantity -= quantity;
	}

	updateExchangeSlot(exchangeSlot);
	return true;
}

// ****************************************************************************
void CExchangeView::validateExchange(std::vector<CGameItemPtr> * givenItems)
{
	for (uint i = 0; i < _ExchangeSlots.size(); i++)
	{
		uint32 bagSlot = _ExchangeSlots[i].BagSlot;
		uint32 exchangeQuantity = _ExchangeSlots[i].Quantity;

		// skip empty slots
		if (bagSlot == INVENTORIES::INVALID_INVENTORY_SLOT)
			continue;

		// remove the item from the exchange view and unlock it
		removeItemFromExchange(i);

		// remove the exchange quantity of item from the bag
		CGameItemPtr item = getInventory()->removeItem(bagSlot, exchangeQuantity);
		BOMB_IF( item == NULL, "<CExchangeView::validateExchange> Can't remove "<<exchangeQuantity<<" item(s) from slot "<<bagSlot<<" of the bag", continue );

		// put the removed item in the given items
		if (givenItems != NULL)
			givenItems->push_back(item);
	}
}

// ****************************************************************************
CGameItemPtr CExchangeView::getExchangeItem(uint32 exchangeSlot, uint32 * exchangeQuantity) const
{
	nlassert(exchangeSlot < NbExchangeSlots);

	uint32 bagSlot = _ExchangeSlots[exchangeSlot].BagSlot;
	bool emptySlot = (bagSlot == INVENTORIES::INVALID_INVENTORY_SLOT);

	if (exchangeQuantity != NULL)
	{
		if (emptySlot)
			*exchangeQuantity = 0;
		else
			*exchangeQuantity = _ExchangeSlots[exchangeSlot].Quantity;
	}

	// get the item on the bag
	CGameItemPtr item = NULL;
	if (!emptySlot)
	{
		item = getInventory()->getItem(bagSlot);
	}

	return item;
}

// ****************************************************************************
bool CExchangeView::isEmpty() const
{
	for (uint i = 0; i < NbExchangeSlots; i++)
	{
		if (_ExchangeSlots[i].BagSlot != INVENTORIES::INVALID_INVENTORY_SLOT)
			return false;
	}

	return true;
}

// ****************************************************************************
void CExchangeView::onInterlocutorSlotChanged(uint32 interlocutorGiveSlot)
{
	nlassert(interlocutorGiveSlot < NbExchangeSlots);
	nlassert(_InterlocutorView != NULL);

//	string sDBPath = NLMISC::toString("EXCHANGE:RECEIVE:%u", interlocutorGiveSlot);
	CBankAccessor_PLR::TEXCHANGE::TRECEIVE::TArray &recvItem = CBankAccessor_PLR::getEXCHANGE().getRECEIVE().getArray(interlocutorGiveSlot);

	uint32 exchangeQuantity;
	CGameItemPtr item = _InterlocutorView->getExchangeItem(interlocutorGiveSlot, &exchangeQuantity);

	if (item != NULL)
	{
		RM_FABER_STAT_TYPE::TRMStatType itemBestStat = RM_FABER_STAT_TYPE::Unknown;
		
		if (item->getCraftParameters() != NULL)
			itemBestStat = item->getCraftParameters()->getBestItemStat();

//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":SHEET",				item->getSheetId().asInt());
		recvItem.setSHEET(getCharacter()->_PropertyDatabase, item->getSheetId());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUALITY",			item->quality());
		recvItem.setQUALITY(getCharacter()->_PropertyDatabase, item->quality());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUANTITY",			exchangeQuantity);
		recvItem.setQUANTITY(getCharacter()->_PropertyDatabase, uint16(exchangeQuantity));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":USER_COLOR",		item->color());
		recvItem.setUSER_COLOR(getCharacter()->_PropertyDatabase, item->color());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":WEIGHT",			item->weight() / 10);
		recvItem.setWEIGHT(getCharacter()->_PropertyDatabase, uint16(item->weight() / 10));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":NAMEID",			item->sendNameId(getCharacter()));
		recvItem.setNAMEID(getCharacter()->_PropertyDatabase, item->sendNameId(getCharacter()));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":ENCHANT",			item->getClientEnchantValue());
		recvItem.setENCHANT(getCharacter()->_PropertyDatabase, item->getClientEnchantValue());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_CLASS_TYPE",		item->getItemClass());
		recvItem.setRM_CLASS_TYPE(getCharacter()->_PropertyDatabase, item->getItemClass());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_FABER_STAT_TYPE",itemBestStat);
		recvItem.setRM_FABER_STAT_TYPE(getCharacter()->_PropertyDatabase, itemBestStat);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":PREREQUISIT_VALID", getCharacter()->checkPreRequired( item ) );
		recvItem.setPREREQUISIT_VALID(getCharacter()->_PropertyDatabase, getCharacter()->checkPreRequired( item ));
	}
	else
	{
		// empty slot
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":SHEET",				0);
		recvItem.setSHEET(getCharacter()->_PropertyDatabase, CSheetId::Unknown);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUALITY",			0);
		recvItem.setQUALITY(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUANTITY",			0);
		recvItem.setQUANTITY(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":USER_COLOR",		0);
		recvItem.setUSER_COLOR(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":WEIGHT",			0);
		recvItem.setWEIGHT(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":NAMEID",			0);
		recvItem.setNAMEID(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":ENCHANT",			0);
		recvItem.setENCHANT(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_CLASS_TYPE",		0);
		recvItem.setRM_CLASS_TYPE(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_FABER_STAT_TYPE",0);
		recvItem.setRM_FABER_STAT_TYPE(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":PREREQUISIT_VALID", 0);
		recvItem.setPREREQUISIT_VALID(getCharacter()->_PropertyDatabase, false);
	}
	
	// increment the info version
//	sint64 nPropValue = getCharacter()->_PropertyDatabase.getProp(sDBPath+":INFO_VERSION");
	uint8 nPropValue = recvItem.getINFO_VERSION(getCharacter()->_PropertyDatabase);
//	getCharacter()->_PropertyDatabase.setProp(sDBPath+":INFO_VERSION", nPropValue+1);
	recvItem.setINFO_VERSION(getCharacter()->_PropertyDatabase, nPropValue+1);
}

// ****************************************************************************
void CExchangeView::updateExchangeSlot(uint32 exchangeSlot)
{
//	string sDBPath = NLMISC::toString("EXCHANGE:GIVE:%u", exchangeSlot);
	CBankAccessor_PLR::TEXCHANGE::TGIVE::TArray &giveItem = CBankAccessor_PLR::getEXCHANGE().getGIVE().getArray(exchangeSlot);

	// get the item on the bag
	uint32 exchangeQuantity;
	CGameItemPtr item = getExchangeItem(exchangeSlot, &exchangeQuantity);

	if (item != NULL)
	{
		RM_FABER_STAT_TYPE::TRMStatType itemBestStat = RM_FABER_STAT_TYPE::Unknown;
		
		if (item->getCraftParameters() != NULL)
			itemBestStat = item->getCraftParameters()->getBestItemStat();

//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":SHEET",				item->getSheetId().asInt());
		giveItem.setSHEET(getCharacter()->_PropertyDatabase, item->getSheetId());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUALITY",			item->quality());
		giveItem.setQUALITY(getCharacter()->_PropertyDatabase, item->quality());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUANTITY",			exchangeQuantity);
		giveItem.setQUANTITY(getCharacter()->_PropertyDatabase, uint16(exchangeQuantity));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":USER_COLOR",		item->color());
		giveItem.setUSER_COLOR(getCharacter()->_PropertyDatabase, item->color());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":WEIGHT",			item->weight() / 10);
		giveItem.setWEIGHT(getCharacter()->_PropertyDatabase, uint16(item->weight() / 10));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":NAMEID",			item->sendNameId(getCharacter()));
		giveItem.setNAMEID(getCharacter()->_PropertyDatabase, item->sendNameId(getCharacter()));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":ENCHANT",			item->getClientEnchantValue());
		giveItem.setENCHANT(getCharacter()->_PropertyDatabase, item->getClientEnchantValue());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_CLASS_TYPE",		item->getItemClass());
		giveItem.setRM_CLASS_TYPE(getCharacter()->_PropertyDatabase, item->getItemClass());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_FABER_STAT_TYPE",itemBestStat);
		giveItem.setRM_FABER_STAT_TYPE(getCharacter()->_PropertyDatabase, itemBestStat);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":PREREQUISIT_VALID", getCharacter()->checkPreRequired( item ) );
		giveItem.setPREREQUISIT_VALID(getCharacter()->_PropertyDatabase, getCharacter()->checkPreRequired( item ));
	}
	else
	{
		// empty slot
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":SHEET",				0);
		giveItem.setSHEET(getCharacter()->_PropertyDatabase, CSheetId::Unknown);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUALITY",			0);
		giveItem.setQUALITY(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUANTITY",			0);
		giveItem.setQUANTITY(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":USER_COLOR",		0);
		giveItem.setUSER_COLOR(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":WEIGHT",			0);
		giveItem.setWEIGHT(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":NAMEID",			0);
		giveItem.setNAMEID(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":ENCHANT",			0);
		giveItem.setENCHANT(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_CLASS_TYPE",		0);
		giveItem.setRM_CLASS_TYPE(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_FABER_STAT_TYPE",0);
		giveItem.setRM_FABER_STAT_TYPE(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":PREREQUISIT_VALID", 0);
		giveItem.setPREREQUISIT_VALID(getCharacter()->_PropertyDatabase, false);
	}
	
	// increment the info version
//	sint64 nPropValue = getCharacter()->_PropertyDatabase.getProp(sDBPath+":INFO_VERSION");
	uint8 nPropValue = giveItem.getINFO_VERSION(getCharacter()->_PropertyDatabase);
//	getCharacter()->_PropertyDatabase.setProp(sDBPath+":INFO_VERSION", nPropValue+1);
	giveItem.setINFO_VERSION(getCharacter()->_PropertyDatabase, nPropValue+1);

	// update interlocutor client
	if (_InterlocutorView != NULL)
	{
		_InterlocutorView->onInterlocutorSlotChanged(exchangeSlot);
	}
}

// ****************************************************************************
bool CExchangeView::isPetTicket(ITEM_TYPE::TItemType itemType) const
{
	if (itemType == ITEM_TYPE::MEKTOUB_PACKER_TICKET || itemType == ITEM_TYPE::MEKTOUB_MOUNT_TICKET)
		return true;

	return false;
}

// ****************************************************************************
uint32 * CExchangeView::getPetTicketCount(ITEM_TYPE::TItemType itemType)
{
	if (itemType == ITEM_TYPE::MEKTOUB_PACKER_TICKET)
		return &_PackerTicketCount;

	if (itemType == ITEM_TYPE::MEKTOUB_MOUNT_TICKET)
		return &_MountTicketCount;

	return NULL;
}

/////////////////////////////////////////////////////////////
// CExchangeView::CExchangeSlot
/////////////////////////////////////////////////////////////

// ****************************************************************************
CExchangeView::CExchangeSlot::CExchangeSlot()
{
	reset();
}

// ****************************************************************************
void CExchangeView::CExchangeSlot::reset()
{
	BagSlot = INVENTORIES::INVALID_INVENTORY_SLOT;
	Quantity = 0;
}
