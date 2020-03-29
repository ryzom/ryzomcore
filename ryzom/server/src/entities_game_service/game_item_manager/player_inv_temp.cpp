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

#include "player_inv_temp.h"

#include "game_share/slot_equipment.h"
#include "player_manager/character.h"

using namespace NLMISC;
using namespace std;

/////////////////////////////////////////////////////////////
// CTempInventory
/////////////////////////////////////////////////////////////

// ****************************************************************************
CTempInventory::CTempInventory()
{
	_Mode = TEMP_INV_MODE::Unknown;
	_Char = NULL;
}

// ****************************************************************************
uint32 CTempInventory::getMaxSlot() const
{
	return INVENTORIES::NbTempInvSlots;
}

// ****************************************************************************
void CTempInventory::bindView(CInventoryViewPtr inventoryView)
{
	CInventoryBase::bindView(inventoryView);
	CTempInvView *pTIV = dynamic_cast<CTempInvView*>((IInventoryView*)inventoryView);
	if (pTIV != NULL)
	{
		pTIV->init();
		_Char = pTIV->getCharacter();
	}
}

// ****************************************************************************
bool CTempInventory::canEnterMode(TEMP_INV_MODE::TInventoryMode mode)
{
	if (mode == TEMP_INV_MODE::Unknown)
		return false;
	
	if (mode == _Mode)
		return true;
	
	return (_Mode == TEMP_INV_MODE::Unknown);
}

// ****************************************************************************
bool CTempInventory::enterMode(TEMP_INV_MODE::TInventoryMode mode)
{
	BOMB_IF( (mode == TEMP_INV_MODE::Unknown), "cannot enter in temp inventory mode 'Unknown'", return false );
	
	if (mode == _Mode)
		return true;
	
	if (_Mode != TEMP_INV_MODE::Unknown)
		return false;
	
	_Mode = mode;
	onInventoryChanged(INVENTORIES::ic_other);
	
	return true;
}

// ****************************************************************************
void CTempInventory::leaveMode()
{
	_Mode = TEMP_INV_MODE::Unknown;
	onInventoryChanged(INVENTORIES::ic_other);	
}

// ****************************************************************************
void CTempInventory::enableTakeDisp(bool b)
{
	if (b)
//		_Char->_PropertyDatabase.setProp("INVENTORY:TEMP:ENABLE_TAKE", 1);
		CBankAccessor_PLR::getINVENTORY().getTEMP().setENABLE_TAKE(_Char->_PropertyDatabase, true);
	else
//		_Char->_PropertyDatabase.setProp("INVENTORY:TEMP:ENABLE_TAKE", 0);
		CBankAccessor_PLR::getINVENTORY().getTEMP().setENABLE_TAKE(_Char->_PropertyDatabase, false);
	
}

// ****************************************************************************
void CTempInventory::clearDisp(uint32 slot)
{
	nlassert(slot < getSlotCount());
	//nlassert(getUsedSlotCount() == 0);

//	string sDBPath = toString("INVENTORY:TEMP:%d", slot);
	CBankAccessor_PLR::TINVENTORY::TTEMP::TArray &tempEntry = CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot);

	// (CSheetId::Unknown == 0)
	setDispSheetId(slot, CSheetId::Unknown);

//	_Char->_PropertyDatabase.setProp(sDBPath+":QUALITY",			0);
	tempEntry.setQUALITY(_Char->_PropertyDatabase, 0);
//	_Char->_PropertyDatabase.setProp(sDBPath+":QUANTITY",			0);
	tempEntry.setQUANTITY(_Char->_PropertyDatabase, 0);
//	_Char->_PropertyDatabase.setProp(sDBPath+":USER_COLOR",			0);
	tempEntry.setUSER_COLOR(_Char->_PropertyDatabase, 0);
//	_Char->_PropertyDatabase.setProp(sDBPath+":WEIGHT",				0);
	tempEntry.setWEIGHT(_Char->_PropertyDatabase, 0);
//	_Char->_PropertyDatabase.setProp(sDBPath+":NAMEID",				0);
	tempEntry.setNAMEID(_Char->_PropertyDatabase, 0);
//	_Char->_PropertyDatabase.setProp(sDBPath+":ENCHANT",			0);
	tempEntry.setENCHANT(_Char->_PropertyDatabase, 0);
//	_Char->_PropertyDatabase.setProp(sDBPath+":RM_CLASS_TYPE",		0);
	tempEntry.setRM_CLASS_TYPE(_Char->_PropertyDatabase, 0);
//	_Char->_PropertyDatabase.setProp(sDBPath+":RM_FABER_STAT_TYPE",	0);
	tempEntry.setRM_FABER_STAT_TYPE(_Char->_PropertyDatabase, 0);
//	_Char->_PropertyDatabase.setProp(sDBPath+":PREREQUISIT_VALID",	0);
	tempEntry.setPREREQUISIT_VALID(_Char->_PropertyDatabase, 0);
}

// ****************************************************************************
void CTempInventory::setDispSheetId(uint32 slot, const NLMISC::CSheetId &sheet)
{
	nlassert(slot < getSlotCount());
	//nlassert(getUsedSlotCount() == 0);

	// increment info version only if sheet id changed
//	if (getDispSheetId(slot) != sheet)
//	{
//		sint64 nPropValue = getCharacter()->_PropertyDatabase.getProp(sDBPath+":INFO_VERSION");
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":INFO_VERSION", nPropValue+1);
//	}

//	_Char->_PropertyDatabase.setProp(NLMISC::toString("INVENTORY:TEMP:%d:SHEET", slot), sheet.asInt());
	CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot).setSHEET(_Char->_PropertyDatabase, sheet);
	// the items displayed here don't have prerequisit
//	_Char->_PropertyDatabase.setProp(NLMISC::toString("INVENTORY:TEMP:%d:PREREQUISIT_VALID", slot),	1);
	CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot).setPREREQUISIT_VALID(_Char->_PropertyDatabase, true);
}

// ****************************************************************************
CSheetId CTempInventory::getDispSheetId(uint32 slot)
{
	nlassert(slot < getSlotCount());
	//nlassert(getUsedSlotCount() == 0);
//	sint64 s = _Char->_PropertyDatabase.getProp(NLMISC::toString("INVENTORY:TEMP:%d:SHEET", slot));
//	return CSheetId((uint32)s);
	return CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot).getSHEET(_Char->_PropertyDatabase);
}

// ****************************************************************************
void CTempInventory::setDispQuality(uint32 slot, uint16 quality)
{
	nlassert(slot < getSlotCount());
	//nlassert(getUsedSlotCount() == 0);
//	_Char->_PropertyDatabase.setProp(NLMISC::toString("INVENTORY:TEMP:%d:QUALITY", slot), quality);
	CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot).setQUALITY(_Char->_PropertyDatabase, quality);
}

// ****************************************************************************
uint16 CTempInventory::getDispQuality(uint32 slot)
{
	nlassert(slot < getSlotCount());
	//nlassert(getUsedSlotCount() == 0);
//	return (uint16)_Char->_PropertyDatabase.getProp(NLMISC::toString("INVENTORY:TEMP:%d:QUALITY", slot));
	return CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot).getQUALITY(_Char->_PropertyDatabase);
}

// ****************************************************************************
void CTempInventory::setDispQuantity(uint32 slot, uint16 quantity)
{
	nlassert(slot < getSlotCount());
	//nlassert(getUsedSlotCount() == 0);
//	_Char->_PropertyDatabase.setProp(NLMISC::toString("INVENTORY:TEMP:%d:QUANTITY", slot), quality);
	CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot).setQUANTITY(_Char->_PropertyDatabase, quantity);
}

// ****************************************************************************
uint16 CTempInventory::getDispQuantity(uint32 slot)
{
	nlassert(slot < getSlotCount());
	//nlassert(getUsedSlotCount() == 0);
//	return (uint16)_Char->_PropertyDatabase.getProp(NLMISC::toString("INVENTORY:TEMP:%d:QUANTITY", slot));
	return CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot).getQUANTITY(_Char->_PropertyDatabase);
}


/////////////////////////////////////////////////////////////
// CTempInvView
/////////////////////////////////////////////////////////////

// ****************************************************************************
void CTempInvView::init()
{
	_LastInfoVersion.resize(INVENTORIES::NbTempInvSlots, 0);
}

// ****************************************************************************
void CTempInvView::onItemChanged(uint32 slot, INVENTORIES::TItemChangeFlags changeFlags)
{
	if (changeFlags.checkEnumValue(INVENTORIES::itc_inserted))
	{
		const CGameItemPtr item = getInventory()->getItem(slot);
		nlassert(item != NULL);
		
		updateClientSlot(slot, item);
	}
	
	if (changeFlags.checkEnumValue(INVENTORIES::itc_removed))
	{
		updateClientSlot(slot, NULL);
	}

	// synchronize the info version number with info version sent to the client
	if (changeFlags.checkEnumValue(INVENTORIES::itc_info_version))
	{
//		string sDBPath = toString("INVENTORY:TEMP:%d", slot);
//		sint64 nPropValue = getCharacter()->_PropertyDatabase.getProp(sDBPath+":INFO_VERSION");
//		_LastInfoVersion[slot] = (uint8)nPropValue;		
		_LastInfoVersion[slot] = CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(slot).getINFO_VERSION(getCharacter()->_PropertyDatabase);		
	}	
}

// ****************************************************************************
void CTempInvView::onInventoryChanged(INVENTORIES::TInventoryChangeFlags changeFlags)
{
	if (changeFlags.checkEnumValue(INVENTORIES::ic_other))
	{
		CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)getInventory();
//		getCharacter()->_PropertyDatabase.setProp("INVENTORY:TEMP:TYPE", invTemp->getMode());
		CBankAccessor_PLR::getINVENTORY().getTEMP().setTYPE(getCharacter()->_PropertyDatabase, invTemp->getMode());
	}
}

// ****************************************************************************
void CTempInvView::updateClientSlot(uint32 clientSlot, const CGameItemPtr item)
{
	// do nothing if client is not ready
	if (!getCharacter()->getEnterFlag())
		return;

//	string sDBPath = NLMISC::toString("INVENTORY:TEMP:%u", clientSlot);
	CBankAccessor_PLR::TINVENTORY::TTEMP::TArray &tempItem = CBankAccessor_PLR::getINVENTORY().getTEMP().getArray(clientSlot);

	if (item != NULL)
	{
		RM_FABER_STAT_TYPE::TRMStatType itemBestStat = RM_FABER_STAT_TYPE::Unknown;
		
		if (item->getCraftParameters() != NULL)
			itemBestStat = item->getCraftParameters()->getBestItemStat();

//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":SHEET",				item->getSheetId().asInt());
		tempItem.setSHEET(getCharacter()->_PropertyDatabase, item->getSheetId());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUALITY",			item->quality());
		tempItem.setQUALITY(getCharacter()->_PropertyDatabase, item->quality());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUANTITY",			item->getStackSize());
		tempItem.setQUANTITY(getCharacter()->_PropertyDatabase, uint16(item->getStackSize()));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":USER_COLOR",		item->color());
		tempItem.setUSER_COLOR(getCharacter()->_PropertyDatabase, item->color());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":WEIGHT",			item->weight() / 10);
		tempItem.setWEIGHT(getCharacter()->_PropertyDatabase, uint16(item->weight()/10));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":NAMEID",			item->sendNameId(getCharacter()));
		tempItem.setNAMEID(getCharacter()->_PropertyDatabase, item->sendNameId(getCharacter()));
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":ENCHANT",			item->getClientEnchantValue());
		tempItem.setENCHANT(getCharacter()->_PropertyDatabase, item->getClientEnchantValue());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_CLASS_TYPE",		item->getItemClass());
		tempItem.setRM_CLASS_TYPE(getCharacter()->_PropertyDatabase, item->getItemClass());
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_FABER_STAT_TYPE",itemBestStat);
		tempItem.setRM_FABER_STAT_TYPE(getCharacter()->_PropertyDatabase, itemBestStat);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":PREREQUISIT_VALID", getCharacter()->checkPreRequired( item ) );
		tempItem.setPREREQUISIT_VALID(getCharacter()->_PropertyDatabase, getCharacter()->checkPreRequired( item ) );
	}
	else
	{
		// empty slot
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":SHEET",				0);
		tempItem.setSHEET(getCharacter()->_PropertyDatabase, CSheetId::Unknown);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUALITY",			0);
		tempItem.setQUALITY(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":QUANTITY",			0);
		tempItem.setQUANTITY(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":USER_COLOR",		0);
		tempItem.setUSER_COLOR(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":WEIGHT",			0);
		tempItem.setWEIGHT(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":NAMEID",			0);
		tempItem.setNAMEID(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":ENCHANT",			0);
		tempItem.setENCHANT(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_CLASS_TYPE",		0);
		tempItem.setRM_CLASS_TYPE(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":RM_FABER_STAT_TYPE",0);
		tempItem.setRM_FABER_STAT_TYPE(getCharacter()->_PropertyDatabase, 0);
//		getCharacter()->_PropertyDatabase.setProp(sDBPath+":PREREQUISIT_VALID", 0);
		tempItem.setPREREQUISIT_VALID(getCharacter()->_PropertyDatabase, false);
	}
	
	// increment the info version number only if equal to the last info version sent to the client
	{
//		sint64 nPropValue = getCharacter()->_PropertyDatabase.getProp(sDBPath+":INFO_VERSION");
		uint8 nPropValue = tempItem.getINFO_VERSION(getCharacter()->_PropertyDatabase);
		if (_LastInfoVersion[clientSlot] == nPropValue)
//			getCharacter()->_PropertyDatabase.setProp(sDBPath+":INFO_VERSION", nPropValue+1);
			tempItem.setINFO_VERSION(getCharacter()->_PropertyDatabase, nPropValue+1);
	}
	
}

