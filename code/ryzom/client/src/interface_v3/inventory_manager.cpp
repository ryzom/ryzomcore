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
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/cdb_branch.h"
#include "inventory_manager.h"
#include "interface_manager.h"
#include "bot_chat_page_trade.h"
#include "bot_chat_page_all.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_menu.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/cdb_branch.h"
#include "list_sheet_base.h"
#include "../net_manager.h"
#include "../user_entity.h"
#include "../global.h"

#include "nel/misc/algo.h"

// TODO: remove this ugly dependence
#include "sphrase_manager.h"

// For handlers
#include "nel/gui/action_handler.h"
#include "dbctrl_sheet.h"

#include "../sheet_manager.h"
#include "game_share/slot_equipment.h"
#include "game_share/animal_status.h"
#include "game_share/bot_chat_types.h"

#include "../client_cfg.h"

#include "../misc.h"

using namespace std;
using namespace NLMISC;

extern NLMISC::CLog g_log;
// Context help
extern void contextHelp (const std::string &help);

CTempInvManager		*CTempInvManager::_Instance = NULL;
CInventoryManager	*CInventoryManager::_Instance = NULL;

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetBag, std::string, "list_sheet_bag");
NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupIconListBag, std::string, "list_icon_bag");
NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetFilterCLMSlot, std::string, "list_sheet_filter_clm_slot");
NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListSheetFilterExchangeable, std::string, "list_sheet_filter_exchangeable");

// ***************************************************************************
// db path for all the inventories (without the SERVER: prefix)
const std::string CInventoryManager::InventoryDBs[]=
{
	"INVENTORY:BAG",
	// MAX_INVENTORY_ANIMAL
	"INVENTORY:PACK_ANIMAL0",
	"INVENTORY:PACK_ANIMAL1",
	"INVENTORY:PACK_ANIMAL2",
	"INVENTORY:PACK_ANIMAL3",
	"INVENTORY:TEMP",
	"EXCHANGE:GIVE",
	"EXCHANGE:RECEIVE",
	"TRADING",
	"INVENTORY:SHARE",
	"GUILD:INVENTORY",
	"INVENTORY:ROOM",
};

static void dummyCheck()
{
	// if this raise, correct the 2 tables above and below
	nlctassert(MAX_INVENTORY_ANIMAL==4);
}

const uint CInventoryManager::InventoryIndexes[]=
{
	INVENTORIES::bag,
	// MAX_INVENTORY_ANIMAL
	INVENTORIES::pet_animal1,
	INVENTORIES::pet_animal2,
	INVENTORIES::pet_animal3,
	INVENTORIES::pet_animal4,
	INVENTORIES::temporary,
	INVENTORIES::exchange,
	INVENTORIES::exchange_proposition,
	INVENTORIES::trading,
	INVENTORIES::reward_sharing,
	INVENTORIES::guild,
	INVENTORIES::player_room,
};

const	uint	CInventoryManager::NumInventories= sizeof(CInventoryManager::InventoryDBs)/sizeof(CInventoryManager::InventoryDBs[0]);


// *************************************************************************************************
CItemImage::CItemImage()
{
	Sheet = NULL;
	Quality = NULL;
	Quantity = NULL;
	UserColor = NULL;
	Price = NULL;
	Weight= NULL;
	NameId= NULL;
	InfoVersion= NULL;
}

// *************************************************************************************************
void CItemImage::build(CCDBNodeBranch *branch)
{
	if (!branch) return;
	Sheet = dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("SHEET"), false));
	Quality = dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("QUALITY"), false));
	Quantity = dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("QUANTITY"), false));
	UserColor = dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("USER_COLOR"), false));
	Price = dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("PRICE"), false));
	Weight = dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("WEIGHT"), false));
	NameId = dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("NAMEID"), false));
	InfoVersion= dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("INFO_VERSION"), false));
	ResaleFlag = dynamic_cast<CCDBNodeLeaf *>(branch->getNode(ICDBNode::CTextId("RESALE_FLAG"), false));

	// Should always have at least those one:(ie all but Price)
	nlassert(Sheet && Quality && Quantity && UserColor && Weight && NameId && InfoVersion);
}

// *************************************************************************************************
// CInventoryManager
// *************************************************************************************************

// *************************************************************************************************
CInventoryManager::CInventoryManager()
{
	Money = NULL;
	ServerMoney = NULL;
	uint i;
	for (i = 0; i < MAX_HANDINV_ENTRIES; ++i)
	{
		Hands[i] = ServerHands[i] = 0;
		UIHands[i] = NULL;
	}

	for (i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		Equip[i] = ServerEquip[i] = 0;
		UIEquip[i] = NULL;
		UIEquip2[i] = NULL;
	}

	for (i = 0; i < MAX_BAGINV_ENTRIES; i++)
	{
		BagItemEquipped[i]= false;
	}

	nlctassert(NumInventories== sizeof(InventoryIndexes)/sizeof(InventoryIndexes[0]));
}

// ***************************************************************************
CInventoryManager::~CInventoryManager()
{
}

// *************************************************************************************************
CInventoryManager *CInventoryManager::getInstance()
{
	if( !_Instance )
		_Instance = new CInventoryManager();
	return _Instance;
}

// ***************************************************************************
void CInventoryManager::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

// *************************************************************************************************
CItemImage &CInventoryManager::getBagItem(uint index)
{
	nlassert(index < MAX_BAGINV_ENTRIES);
	return Bag[index];
}

// *************************************************************************************************
CItemImage &CInventoryManager::getTempItem(uint index)
{
	nlassert(index < MAX_TEMPINV_ENTRIES);
	return TempInv[index];
}

// *************************************************************************************************
CItemImage *CInventoryManager::getHandItem(uint index)
{
	nlassert(index < MAX_HANDINV_ENTRIES);
	if (Hands[index] != 0)
		return &Bag[Hands[index]-1];
	else
		return NULL;
}

// *************************************************************************************************
CItemImage *CInventoryManager::getEquipItem(uint index)
{
	nlassert(index < MAX_EQUIPINV_ENTRIES);
	if (Equip[index] != 0)
		return &Bag[Equip[index]];
	else
		return NULL;
}

// *************************************************************************************************
CDBCtrlSheet *CInventoryManager::getHandSheet(uint index)
{
	return UIHands[index];
}

// *************************************************************************************************
CDBCtrlSheet *CInventoryManager::getEquipSheet(uint index)
{
	return UIEquip[index];
}


// *************************************************************************************************
CItemImage &CInventoryManager::getServerBagItem(uint index)
{
	nlassert(index < MAX_BAGINV_ENTRIES);
	return ServerBag[index];
}

// *************************************************************************************************
CItemImage &CInventoryManager::getServerTempItem(uint index)
{
	nlassert(index < MAX_TEMPINV_ENTRIES);
	return ServerTempInv[index];
}

// *************************************************************************************************
CItemImage *CInventoryManager::getServerHandItem(uint index)
{
	nlassert(index < MAX_HANDINV_ENTRIES);
	if (ServerHands[index] != 0)
		return &ServerBag[ServerHands[index]];
	else
		return NULL;
}

// *************************************************************************************************
CItemImage *CInventoryManager::getServerEquipItem(uint index)
{
	nlassert(index < MAX_EQUIPINV_ENTRIES);
	if (ServerEquip[index] != 0)
		return &ServerBag[ServerEquip[index]];
	else
		return NULL;
}

// *************************************************************************************************
uint64 CInventoryManager::getMoney() const
{
	return Money ? Money->getValue64() : 0;
}

// *************************************************************************************************
void CInventoryManager::setMoney(uint64 value)
{
	if (Money) Money->setValue64(value);
}

// *************************************************************************************************
uint64 CInventoryManager::getServerMoney() const
{
	return ServerMoney ? ServerMoney->getValue64() : 0;
}

// *************************************************************************************************
void CInventoryManager::setServerMoney(uint64 value)
{
	if (ServerMoney) ServerMoney->setValue64(value);
}
// *************************************************************************************************
void CInventoryManager::init()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// LOCAL DB
	initItemArray(LOCAL_INVENTORY ":BAG", Bag, MAX_BAGINV_ENTRIES);
	initItemArray(LOCAL_INVENTORY ":TEMP", TempInv, MAX_TEMPINV_ENTRIES);
	Money = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":MONEY");
	initIndirection (LOCAL_INVENTORY ":HAND:", Hands, MAX_HANDINV_ENTRIES, true);
	initIndirection (LOCAL_INVENTORY ":EQUIP:", Equip, MAX_EQUIPINV_ENTRIES, true);
	// Init observers for auto equipment
	{
		for (uint i = 0; i < MAX_BAGINV_ENTRIES; ++i)
		{
			CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(i) + ":SHEET");
			ICDBNode::CTextId textId;
			pNL->addObserver(&_DBBagObs, textId);
		}
	}
	// Init Animals
	for(uint i=0;i<MAX_INVENTORY_ANIMAL;i++)
		initItemArray(toString(LOCAL_INVENTORY ":PACK_ANIMAL%d", i), PAInv[i], MAX_ANIMALINV_ENTRIES);


	// SERVER DB
	initItemArray(SERVER_INVENTORY ":BAG", ServerBag, MAX_BAGINV_ENTRIES);
	initItemArray(SERVER_INVENTORY ":TEMP", ServerTempInv, MAX_TEMPINV_ENTRIES);
	ServerMoney = NLGUI::CDBManager::getInstance()->getDbProp(SERVER_INVENTORY ":MONEY");
	// Init Animals
	for(uint i=0;i<MAX_INVENTORY_ANIMAL;i++)
		initItemArray(toString(SERVER_INVENTORY ":PACK_ANIMAL%d", i), ServerPAInv[i], MAX_ANIMALINV_ENTRIES);

	// Drag'n'Drop
	DNDCurrentItem = NULL;
	DNDFrom = Nowhere;
	// Initialize interface part
	UIHands[0] = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_HAND_RIGHT));
	UIHands[1] = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_HAND_LEFT));

	UIEquip[SLOT_EQUIPMENT::HEADDRESS]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_HEADDRESS));
	UIEquip[SLOT_EQUIPMENT::EARL]		= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_EARING_LEFT));
	UIEquip[SLOT_EQUIPMENT::EARR]		= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_EARING_RIGHT));
	UIEquip[SLOT_EQUIPMENT::NECKLACE]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_NECK));
	UIEquip[SLOT_EQUIPMENT::WRISTL]		= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_BRACELET_LEFT));
	UIEquip[SLOT_EQUIPMENT::WRISTR]		= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_BRACELET_RIGHT));
	UIEquip[SLOT_EQUIPMENT::FINGERL]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_RING_LEFT));
	UIEquip[SLOT_EQUIPMENT::FINGERR]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_RING_RIGHT));
	UIEquip[SLOT_EQUIPMENT::ANKLEL]		= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_ANKLET_LEFT));
	UIEquip[SLOT_EQUIPMENT::ANKLER]		= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWEL_ANKLET_RIGHT));

	UIEquip[SLOT_EQUIPMENT::HEAD]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMOR_HEAD));
	UIEquip[SLOT_EQUIPMENT::CHEST]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMOR_CHEST));
	UIEquip[SLOT_EQUIPMENT::ARMS]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMOR_ARMS));
	UIEquip[SLOT_EQUIPMENT::FEET]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMOR_FEET));
	UIEquip[SLOT_EQUIPMENT::LEGS]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMOR_LEGS));
	UIEquip[SLOT_EQUIPMENT::HANDS]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMOR_HANDS));

	UIEquip2[SLOT_EQUIPMENT::HEADDRESS]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_HEADDRESS));
	UIEquip2[SLOT_EQUIPMENT::EARL]		= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_EARING_LEFT));
	UIEquip2[SLOT_EQUIPMENT::EARR]		= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_EARING_RIGHT));
	UIEquip2[SLOT_EQUIPMENT::NECKLACE]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_NECK));
	UIEquip2[SLOT_EQUIPMENT::WRISTL]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_BRACELET_LEFT));
	UIEquip2[SLOT_EQUIPMENT::WRISTR]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_BRACELET_RIGHT));
	UIEquip2[SLOT_EQUIPMENT::FINGERL]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_RING_LEFT));
	UIEquip2[SLOT_EQUIPMENT::FINGERR]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_RING_RIGHT));
	UIEquip2[SLOT_EQUIPMENT::ANKLEL]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_ANKLET_LEFT));
	UIEquip2[SLOT_EQUIPMENT::ANKLER]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_JEWL2_ANKLET_RIGHT));

	UIEquip2[SLOT_EQUIPMENT::HEAD]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMR2_HEAD));
	UIEquip2[SLOT_EQUIPMENT::CHEST]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMR2_CHEST));
	UIEquip2[SLOT_EQUIPMENT::ARMS]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMR2_ARMS));
	UIEquip2[SLOT_EQUIPMENT::FEET]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMR2_FEET));
	UIEquip2[SLOT_EQUIPMENT::LEGS]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMR2_LEGS));
	UIEquip2[SLOT_EQUIPMENT::HANDS]	= dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_ARMR2_HANDS));


	// Init ItemInfoObservers
	{
		CCDBNodeLeaf	*nodeTS;

		nodeTS= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TRADING:SESSION", false);
		if( nodeTS )
		{
			ICDBNode::CTextId textId;
			nodeTS->addObserver(&_DBTradeInfoObs, textId);
		}

		// Init All Version obs
		for(uint i=0;i<NumInventories;i++)
		{
			// Don't do it for TRADING DB
			if(InventoryIndexes[i]!=INVENTORIES::trading)
			{
				// since different size per inventory, stop when node not found....
				for(uint j=0;;j++)
				{
					CCDBNodeLeaf	*nodeIV= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:" + InventoryDBs[i] + ":" + toString(j) + ":INFO_VERSION", false);
					CCDBNodeLeaf	*nodeSH= NLGUI::CDBManager::getInstance()->getDbProp("SERVER:" + InventoryDBs[i] + ":" + toString(j) + ":SHEET", false);
					if( nodeIV && nodeSH )
					{
						ICDBNode::CTextId textIdIV, textIdSH;
						nodeIV->addObserver(&_DBInfoSlotVersionObs, textIdIV);
						nodeSH->addObserver(&_DBItemSheetObs, textIdSH);
						// if current value!=0, simulate a receive item info obs
						if(nodeIV->getValue32())
						{
							onReceiveItemInfoSlotVersion(nodeIV);
						}
					}
					else
						// stop here for this inventory
						break;
				}
			}
		}
	}
}

// *************************************************************************************************
void CInventoryManager::initItemArray(const std::string &dbBranchName, CItemImage *dest, uint numItems)
{
	nlassert(dest);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CCDBNodeBranch *branch = NLGUI::CDBManager::getInstance()->getDbBranch(dbBranchName);
	if (!branch)
	{
		nlwarning("Can't init inventory image from branch %s.", dbBranchName.c_str());
		return;
	}
	for(uint k = 0; k < numItems; ++k)
	{
		CCDBNodeBranch *itemBranch = dynamic_cast<CCDBNodeBranch *>(branch->getNode((uint16) k));
		if (!itemBranch)
		{
			nlwarning("Can't retrieve item %d of branch %s", (int) k, dbBranchName.c_str());
		}
		else
		{
			dest[k].build(itemBranch);
		}
	}
}

// ***************************************************************************
void CInventoryManager::initIndirection(const std::string &dbbranch, sint32 *indices, sint32 nbIndex, bool putObs)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	for (uint i = 0 ; i < (uint)nbIndex; ++i)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(dbbranch + toString(i) + ":INDEX_IN_BAG");
		if (putObs)
		{
			ICDBNode::CTextId textId;
			pNL->addObserver(&_DBEquipObs, textId);
		}
		if (pNL != NULL)
			indices[i] = pNL->getValue32();
	}
}

// ***************************************************************************
void CInventoryManager::beginDrag(CDBCtrlSheet *pCS, TFrom From)
{
	DNDCurrentItem = pCS;
	if (pCS)
		CDBCtrlSheet::setCurrSelSheet(pCS);
	DNDFrom = From;
}

// ***************************************************************************
void CInventoryManager::endDrag()
{
	DNDCurrentItem = NULL;
	DNDFrom = Nowhere;
}

// ***************************************************************************
// Used for interface objects which are reference in bag (the getSheet() returns INDEX_IN_BAG)
std::string CInventoryManager::getDBIndexPath(CDBCtrlSheet *pCS)
{
	string sTmp;
	uint i;
	for (i = 0; i < MAX_HANDINV_ENTRIES; ++i)
	{
		if (UIHands[i] == pCS)
		{
			return string(LOCAL_INVENTORY) + ":HAND:" + toString(i);
		}
	}

	for (i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		if (UIEquip[i] == pCS)
		{
			return string(LOCAL_INVENTORY) + ":EQUIP:" + toString(i);
		}
		if (UIEquip2[i] == pCS)
		{
			return string(LOCAL_INVENTORY) + ":EQUIP:" + toString(i);
		}
	}
	return "";
}

// ***************************************************************************
bool CInventoryManager::is2HandItem(uint32 sheetID)
{
	bool result = false;
	CEntitySheet	*sheet= SheetMngr.get(CSheetId(sheetID));
	if(sheet && sheet->type()== CEntitySheet::ITEM)
	{
		CItemSheet	*item= (CItemSheet*)sheet;
		if( item->hasSlot(SLOTTYPE::TWO_HANDS) || item->hasSlot(SLOTTYPE::RIGHT_HAND_EXCLUSIVE) )
			result = true;
	}
	return result;
}

// ***************************************************************************
bool CInventoryManager::isMeleeWeaponItem(uint32 sheetID)
{
	bool result = false;
	CEntitySheet	*sheet= SheetMngr.get(CSheetId(sheetID));
	if(sheet && sheet->type()== CEntitySheet::ITEM)
	{
		CItemSheet	*item= (CItemSheet*)sheet;
		if( item->Family == ITEMFAMILY::MELEE_WEAPON )
			result = true;
	}
	return result;
}

// ***************************************************************************
bool CInventoryManager::isRangeWeaponItem(uint32 sheetID)
{
	bool result = false;
	CEntitySheet	*sheet= SheetMngr.get(CSheetId(sheetID));
	if(sheet && sheet->type()== CEntitySheet::ITEM)
	{
		CItemSheet	*item= (CItemSheet*)sheet;
		if( item->Family == ITEMFAMILY::RANGE_WEAPON )
			result = true;
	}
	return result;
}

// ***************************************************************************
bool CInventoryManager::isDagger(uint32 sheetID)
{
	bool result = false;
	CEntitySheet	*sheet= SheetMngr.get(CSheetId(sheetID));
	if(sheet && sheet->type()== CEntitySheet::ITEM)
	{
		CItemSheet	*item= (CItemSheet*)sheet;
		if( item->ItemType == ITEM_TYPE::DAGGER)
			result = true;
	}
	return result;
}

// ***************************************************************************
bool CInventoryManager::isSword(uint32 sheetID)
{
	bool result = false;
	CEntitySheet	*sheet= SheetMngr.get(CSheetId(sheetID));
	if(sheet && sheet->type()== CEntitySheet::ITEM)
	{
		CItemSheet	*item= (CItemSheet*)sheet;
		if( item->ItemType == ITEM_TYPE::SWORD)
			result = true;
	}
	return result;
}

// ***************************************************************************
bool CInventoryManager::isForageToolItem(uint32 sheetID)
{
	bool result = false;
	CEntitySheet	*sheet= SheetMngr.get(CSheetId(sheetID));
	if(sheet && sheet->type()== CEntitySheet::ITEM)
	{
		CItemSheet	*item= (CItemSheet*)sheet;
		if( item->Family == ITEMFAMILY::HARVEST_TOOL )
			result = true;
	}
	return result;
}

// ***************************************************************************
uint32 CInventoryManager::getHandItemSheet( bool rightHand ) const
{
	CSheetId item;
	CInterfaceManager *pIM= CInterfaceManager::getInstance();
	string dbPropPath = toString("LOCAL:INVENTORY:HAND:%d:INDEX_IN_BAG",rightHand?0:1);
	// get the RightHand bag index
	sint32	itemSlot= NLGUI::CDBManager::getInstance()->getDbProp(dbPropPath)->getValue32();
	// if something in hand
	if(itemSlot>0)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:BAG:"+toString(itemSlot-1) +":SHEET", false);
		if(node)
			item= node->getValue32();
	}
	return item.asInt();
}


// ***************************************************************************
bool CInventoryManager::isLeftHandItemCompatibleWithRightHandItem(uint32 leftHandSheet, uint32 rightHandSheet, uint32 lastRightHandSheet)
{
	CEntitySheet *pLastRight = SheetMngr.get (CSheetId(lastRightHandSheet));
	if (pLastRight != NULL)
	{
		if (pLastRight->type() != CEntitySheet::ITEM) return false;
		CItemSheet *pIsLastRight = (CItemSheet *)pLastRight;

		// Last item in right hand is a 2 hand item and the new item is nothing (unequip)
		if (pIsLastRight->hasSlot(SLOTTYPE::TWO_HANDS) || pIsLastRight->hasSlot(SLOTTYPE::RIGHT_HAND_EXCLUSIVE))
			return false;
	}

	if (leftHandSheet == 0) return true;

	CEntitySheet *pLeft = SheetMngr.get (CSheetId(leftHandSheet));
	if (pLeft == NULL) return false;
	if (pLeft->type() != CEntitySheet::ITEM) return false;
	CItemSheet *pIsLeft = (CItemSheet *)pLeft;

	if ((pIsLeft->Family == ITEMFAMILY::AMMO) && (rightHandSheet == 0))
		return false;

	if ((pIsLeft->ItemType == ITEM_TYPE::DAGGER) && (rightHandSheet == 0))
		return false;

	CEntitySheet *pRight = SheetMngr.get (CSheetId(rightHandSheet));
	if (pRight == NULL) return true;
	if (pRight->type() != CEntitySheet::ITEM) return true;
	CItemSheet *pIsRight = (CItemSheet *)pRight;

	if (pIsRight->Family == ITEMFAMILY::RANGE_WEAPON)
	{
		if (pIsLeft->Family == ITEMFAMILY::AMMO)
			if (pIsRight->RangeWeapon.Skill == pIsLeft->Ammo.Skill)
				return true;
	}

	if (pIsLeft->ItemType == ITEM_TYPE::DAGGER)
	{
		if ((pIsRight->ItemType == ITEM_TYPE::SWORD) || (pIsRight->ItemType == ITEM_TYPE::DAGGER))
			return true;
		else
			return false;
	}

	if (!pIsRight->hasSlot(SLOTTYPE::TWO_HANDS) && !pIsRight->hasSlot(SLOTTYPE::RIGHT_HAND_EXCLUSIVE))
	{
		return true;
	}

	return false;
}

// ***************************************************************************
static void grayItem (const std::string &listname, sint32 bagEntryIndex, bool gray)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	IListSheetBase *pList = dynamic_cast<IListSheetBase*>(CWidgetManager::getInstance()->getElementFromId(listname));

	if (pList != NULL)
	{
		pList->invalidateCoords();

		for(uint i = 0; i < MAX_BAGINV_ENTRIES; ++i)
		{
			CDBCtrlSheet *pCS = pList->getSheet(i);
			string sTmp = pCS->getSheet();
			sTmp = sTmp.substr(sTmp.rfind(':')+1,sTmp.size());
			sint32 nTmp;
			fromString(sTmp, nTmp);
			if (nTmp == bagEntryIndex)
				pCS->setItemWeared(gray);
		}
	}
}

// ***************************************************************************
void CInventoryManager::wearBagItem(sint32 bagEntryIndex)
{
	if(bagEntryIndex>=0 && bagEntryIndex<(sint32)MAX_BAGINV_ENTRIES)
	{
		BagItemEquipped[bagEntryIndex]= true;
		grayItem (LIST_BAG_TEXT, bagEntryIndex, true);
		grayItem (LIST_BAG_ICONS, bagEntryIndex, true);
		sortBag();
	}
}

// ***************************************************************************
void CInventoryManager::unwearBagItem(sint32 bagEntryIndex)
{
	if(bagEntryIndex>=0 && bagEntryIndex<(sint32)MAX_BAGINV_ENTRIES)
	{
		BagItemEquipped[bagEntryIndex]= false;
		grayItem (LIST_BAG_TEXT, bagEntryIndex, false);
		grayItem (LIST_BAG_ICONS, bagEntryIndex, false);
		sortBag();
	}
}

// ***************************************************************************
bool CInventoryManager::isBagItemWeared(sint32 bagEntryIndex)
{
	if(bagEntryIndex>=0 && bagEntryIndex<(sint32)MAX_BAGINV_ENTRIES)
	{
		return BagItemEquipped[bagEntryIndex];
	}

	return false;
}

// ----------------------------------------------------------------------------
static bool isSwimming()
{
	if (UserEntity != NULL)
		return (UserEntity->mode() == MBEHAV::SWIM || UserEntity->mode() == MBEHAV::MOUNT_SWIM);
	else
		return false;
}

static bool isRiding()
{
	if (UserEntity)
		return UserEntity->isRiding();
	else
		return false;
}

static bool isStunned()
{
	if (UserEntity != NULL)
		return (UserEntity->behaviour() == MBEHAV::STUNNED);
	else
		return false;
}

static bool isDead()
{
	if (UserEntity != NULL)
		return (UserEntity->mode() == MBEHAV::DEATH);
	else
		return false;
}

// ----------------------------------------------
//	equip
//
// ----------------------------------------------
void CInventoryManager::equip(const std::string &bagPath, const std::string &invPath)
{
	if (isSwimming() || isStunned() || isDead() || isRiding()) return;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (bagPath.empty() || invPath.empty())
	{
		return;
	}

	// Get inventory and slot
	string sIndexInBag = bagPath.substr(bagPath.rfind(':')+1,bagPath.size());
	uint16 indexInBag;
	fromString(sIndexInBag, indexInBag);

	uint16 inventory = INVENTORIES::UNDEFINED;
	uint16 invSlot = 0xffff;

	if (strnicmp(invPath.c_str(),"LOCAL:INVENTORY:HAND",20) == 0)
	{
		inventory = INVENTORIES::handling;
		fromString(invPath.substr(21,invPath.size()), invSlot);
	}
	else if (strnicmp(invPath.c_str(),"LOCAL:INVENTORY:EQUIP",21) == 0)
	{
		inventory = INVENTORIES::equipment;
		fromString(invPath.substr(22,invPath.size()), invSlot);
	}

	// Hands management : check if we have to unequip left hand because of incompatibility with right hand item
	sint16 oldRightIndexInBag = NLGUI::CDBManager::getInstance()->getDbProp(invPath + ":INDEX_IN_BAG")->getValue16();
	if (inventory == INVENTORIES::handling && invSlot == 0)
	{
		CDBCtrlSheet *pCSLeftHand = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_HAND_LEFT));
		if (pCSLeftHand == NULL)
		{
			return;
		}

		// get sheet of left item
		uint32 leftSheet = 0;
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":HAND:1:INDEX_IN_BAG", false);
		if (pNL == NULL)
		{
			return;
		}
		if (pNL->getValue32() > 0)
		{
			CCDBNodeLeaf *pNL2 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(pNL->getValue32()-1) + ":SHEET", false);
			if (pNL2 == NULL)
			{
				return;
			}
			leftSheet = pNL2->getValue32();
		}

		// get sheet of previous right hand item
		uint32 lastRightSheet = 0;
		if (oldRightIndexInBag > 0)
		{
			pNL = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(oldRightIndexInBag-1) + ":SHEET", false);
			if (pNL == NULL)
			{
				return;
			}
			lastRightSheet = pNL->getValue32();
		}

		// get sheet of new right hand item
		uint32 rightSheet = 0;
		if (indexInBag+1 > 0)
		{
			pNL = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(indexInBag) + ":SHEET", false);
			if (pNL == NULL)
			{
				return;
			}
			rightSheet = pNL->getValue32();
		}

		// If incompatible -> remove
		if (!getInventory().isLeftHandItemCompatibleWithRightHandItem(leftSheet, rightSheet, lastRightSheet))
		{
			getInventory().unequip(LOCAL_INVENTORY ":HAND:1");
		}
	}

	// update the equip DB pointer
	NLGUI::CDBManager::getInstance()->getDbProp(invPath + ":INDEX_IN_BAG")->setValue16(indexInBag+1);

	// Yoyo add: when the user equip an item, the action are invalid during some time
	if(indexInBag < MAX_BAGINV_ENTRIES)
	{
		CItemSheet *pIS= dynamic_cast<CItemSheet*>(SheetMngr.get(CSheetId(getBagItem(indexInBag).getSheetID())));
		if(pIS)
		{
			CSPhraseManager	*pPM= CSPhraseManager::getInstance();
			pPM->setEquipInvalidation(NetMngr.getCurrentServerTick(), pIS->EquipTime);
		}
	}

	// Update trade window if any
	if ((BotChatPageAll != NULL) && (BotChatPageAll->Trade != NULL))
		BotChatPageAll->Trade->invalidateCoords();

	// Send message to the server
	if (inventory != INVENTORIES::UNDEFINED)
	{
		CBitMemStream out;
		const string sMsg = "ITEM:EQUIP";
		if (GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			// Fill the message (equipped inventory, equipped inventory slot, bag slot)
			out.serial(inventory);
			out.serial(invSlot);
			out.serial(indexInBag);
			NetMngr.push (out);

			pIM->incLocalSyncActionCounter();

			//nlinfo("impulseCallBack : %s %d %d %d sent", sMsg.c_str(), inventory, invSlot, indexInBag);
		}
		else
		{
			nlwarning ("don't know message name %s", sMsg.c_str());
		}
	}
}



// ----------------------------------------------
//	unequip
//
// ----------------------------------------------
void CInventoryManager::unequip(const std::string &invPath)
{
	if (isSwimming() || isStunned() || isDead() ) return;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	sint16 oldIndexInBag = NLGUI::CDBManager::getInstance()->getDbProp(invPath + ":INDEX_IN_BAG")->getValue16();
	if( oldIndexInBag == 0 )
	{
		return;
	}

	// Get inventory and slot
	uint16 inventory = INVENTORIES::UNDEFINED;
	uint16 invSlot = 0xffff;

	if (strnicmp(invPath.c_str(),"LOCAL:INVENTORY:HAND",20) == 0)
	{
		inventory = INVENTORIES::handling;
		fromString(invPath.substr(21,invPath.size()), invSlot);
	}
	else if (strnicmp(invPath.c_str(),"LOCAL:INVENTORY:EQUIP",21) == 0)
	{
		inventory = INVENTORIES::equipment;
		fromString(invPath.substr(22,invPath.size()), invSlot);
	}

	// Hands management : check if we have to unequip left hand because of incompatibility with right hand item
	if (inventory == INVENTORIES::handling && invSlot == 0)
	{
		CDBCtrlSheet *pCSLeftHand = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_HAND_LEFT));
		if (pCSLeftHand == NULL)
		{
			return;
		}

		// get sheet of left item
		uint32 leftSheet = 0;
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":HAND:1:INDEX_IN_BAG", false);
		if (pNL == NULL)
		{
			return;
		}
		if (pNL->getValue32() > 0)
		{
			CCDBNodeLeaf *pNL2 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(pNL->getValue32()-1) + ":SHEET", false);
			if (pNL2 == NULL)
			{
				return;
			}
			leftSheet = pNL2->getValue32();
		}

		// get sheet of previous right hand item
		uint32 lastRightSheet = 0;
		if (oldIndexInBag > 0)
		{
			pNL = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(oldIndexInBag-1) + ":SHEET", false);
			if (pNL == NULL)
			{
				return;
			}
			lastRightSheet = pNL->getValue32();
		}

		// sheet of new right hand item
		uint32 rightSheet = 0;

		// If incompatible -> remove
		if (!getInventory().isLeftHandItemCompatibleWithRightHandItem(leftSheet, rightSheet, lastRightSheet))
		{
			getInventory().unequip(LOCAL_INVENTORY ":HAND:1");
		}
	}

	NLGUI::CDBManager::getInstance()->getDbProp(invPath + ":INDEX_IN_BAG")->setValue16(0);

	// Update trade window if any
	if ((BotChatPageAll != NULL) && (BotChatPageAll->Trade != NULL))
		BotChatPageAll->Trade->invalidateCoords();

	// Send message to the server
	if (inventory != INVENTORIES::UNDEFINED)
	{
		CBitMemStream out;
		const string sMsg = "ITEM:UNEQUIP";
		if (GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			// Fill the message (equipped inventory, equipped inventory slot)
			out.serial(inventory);
			out.serial(invSlot);
			NetMngr.push (out);

			pIM->incLocalSyncActionCounter();

			//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), inventory, invSlot);
		}
		else
		{
			nlwarning ("don't know message name %s", sMsg.c_str());
		}
	}
}


// ***************************************************************************
// Observer on DB equipment branch
// ***************************************************************************
void CInventoryManager::CDBEquipObs::update(ICDBNode* node)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	string sTmp = node->getFullName();
	string sIE, sIE2; // Interface Element
	CCDBNodeLeaf *pNL = dynamic_cast<CCDBNodeLeaf*>(node);
	if (pNL == NULL) return;
	if (strnicmp(sTmp.c_str(),"LOCAL:INVENTORY:HAND",20) == 0)
	{
		// Coming from hand
		sTmp = sTmp.substr(21,sTmp.size());
		sTmp = sTmp.substr(0,sTmp.rfind(':'));
		sint index;
		fromString(sTmp, index);
		if (index == 0)
			sIE = CTRL_HAND_RIGHT;
		else
			sIE = CTRL_HAND_LEFT;
		// update Hands.
		getInventory().Hands[index]= pNL->getValue16();
	}
	else if (strnicmp(sTmp.c_str(),"LOCAL:INVENTORY:EQUIP",21) == 0)
	{
		// Coming from equipement
		sTmp = sTmp.substr(22,sTmp.size());
		sTmp = sTmp.substr(0,sTmp.rfind(':'));
		sint32 nTmp;
		fromString(sTmp, nTmp);
		SLOT_EQUIPMENT::TSlotEquipment index = (SLOT_EQUIPMENT::TSlotEquipment)nTmp;
		switch(index)
		{
			case SLOT_EQUIPMENT::HEADDRESS:	sIE = CTRL_JEWEL_HEADDRESS;
											sIE2= CTRL_JEWL2_HEADDRESS;			break;
			case SLOT_EQUIPMENT::EARL:		sIE = CTRL_JEWEL_EARING_LEFT;
											sIE2= CTRL_JEWL2_EARING_LEFT;		break;
			case SLOT_EQUIPMENT::EARR:		sIE = CTRL_JEWEL_EARING_RIGHT;
											sIE2= CTRL_JEWL2_EARING_RIGHT;		break;
			case SLOT_EQUIPMENT::NECKLACE:	sIE = CTRL_JEWEL_NECK;
											sIE2= CTRL_JEWL2_NECK;				break;
			case SLOT_EQUIPMENT::WRISTL:	sIE = CTRL_JEWEL_BRACELET_LEFT;
											sIE2= CTRL_JEWL2_BRACELET_LEFT;		break;
			case SLOT_EQUIPMENT::WRISTR:	sIE = CTRL_JEWEL_BRACELET_RIGHT;
											sIE2= CTRL_JEWL2_BRACELET_RIGHT;	break;
			case SLOT_EQUIPMENT::FINGERL:	sIE = CTRL_JEWEL_RING_LEFT;
											sIE2= CTRL_JEWL2_RING_LEFT;			break;
			case SLOT_EQUIPMENT::FINGERR:	sIE = CTRL_JEWEL_RING_RIGHT;
											sIE2= CTRL_JEWL2_RING_RIGHT;		break;
			case SLOT_EQUIPMENT::ANKLEL:	sIE = CTRL_JEWEL_ANKLET_LEFT;
											sIE2= CTRL_JEWL2_ANKLET_LEFT;		break;
			case SLOT_EQUIPMENT::ANKLER:	sIE = CTRL_JEWEL_ANKLET_RIGHT;
											sIE2= CTRL_JEWL2_ANKLET_RIGHT;		break;

			case SLOT_EQUIPMENT::HEAD:		sIE = CTRL_ARMOR_HEAD;
											sIE2= CTRL_ARMR2_HEAD;		break;
			case SLOT_EQUIPMENT::CHEST:		sIE = CTRL_ARMOR_CHEST;
											sIE2= CTRL_ARMR2_CHEST;		break;
			case SLOT_EQUIPMENT::ARMS:		sIE = CTRL_ARMOR_ARMS;
											sIE2= CTRL_ARMR2_ARMS;		break;
			case SLOT_EQUIPMENT::FEET:		sIE = CTRL_ARMOR_FEET;
											sIE2= CTRL_ARMR2_FEET;		break;
			case SLOT_EQUIPMENT::LEGS:		sIE = CTRL_ARMOR_LEGS;
											sIE2= CTRL_ARMR2_LEGS;		break;
			case SLOT_EQUIPMENT::HANDS:		sIE = CTRL_ARMOR_HANDS;
											sIE2= CTRL_ARMR2_HANDS;		break;

			default:
				nlwarning("entry not handled");
				return;
			break;
		}
		// update Equips.
		getInventory().Equip[index]= pNL->getValue16();
	}
	else return;

	// Set database for wearing the right item
	CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(sIE));
	CDBCtrlSheet *pCS2 = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(sIE2));

	// Remove Last reference and update database
	sint16 oldVal = pNL->getOldValue16();
	sint16 newVal = pNL->getValue16();
	if (oldVal != 0)
		getInventory().unwearBagItem (oldVal-1);

	if (newVal != 0)
		getInventory().wearBagItem (newVal-1);

	// Update Display
	if (newVal == 0)
	{
		// in some case left sheet is same than right sheet so don't clear it now (ex: 2 hands item, right hand exclusive)
		if (sIE != CTRL_HAND_LEFT)
		{
			if (pCS  != NULL) pCS->setSheet("");
			if (pCS2 != NULL) pCS2->setSheet("");
		}
	}
	else
	{
		if (pCS  != NULL) pCS->setSheet(LOCAL_INVENTORY ":BAG:"+ toString(newVal-1));
		if (pCS2 != NULL) pCS2->setSheet(LOCAL_INVENTORY ":BAG:"+ toString(newVal-1));
	}

	// Hands management
	if (sIE == CTRL_HAND_RIGHT)
	{
		// if nothing in left hand -> return
		CDBCtrlSheet *pCSLeftHand = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_HAND_LEFT));
		if (pCSLeftHand == NULL)
		{
			return;
		}

		// reset display of left hand
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		pCSLeftHand->setTextureNoItem(rVR.getTextureIdFromName("hand_left.tga"));
		pCSLeftHand->setGrayed(false);
		pCSLeftHand->setItemSlot(SLOTTYPE::stringToSlotType("LEFT_HAND"));
		pCSLeftHand->setActionOnLeftClick("proc");

		// If something in left hand check if we have to remove
		{
			uint32 leftSheet = 0;
			CCDBNodeLeaf *pNL3 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":HAND:1:INDEX_IN_BAG", false);
			if (pNL3 == NULL) return;
			if (pNL3->getValue32() > 0)
			{
				CCDBNodeLeaf *pNL4 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(pNL3->getValue32()-1) + ":SHEET", false);
				if (pNL4 == NULL) return;
				leftSheet = pNL4->getValue32();
			}

			uint32 rightSheet = 0;
			if (newVal > 0)
			{
				pNL3 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(newVal-1) + ":SHEET", false);
				if (pNL3 == NULL) return;
				rightSheet = pNL3->getValue32();
			}

			uint32 lastRightSheet = 0;
			if (oldVal > 0)
			{
				pNL3 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(oldVal-1) + ":SHEET", false);
				if (pNL3 == NULL) return;
				lastRightSheet = pNL3->getValue32();
			}

			// If incompatible -> remove
			if (!getInventory().isLeftHandItemCompatibleWithRightHandItem(leftSheet, rightSheet, lastRightSheet))
			{
				pCSLeftHand->setSheet("");
			}
			// WORKAROUND: useful when an item is destroyed before it is unequipped (clean the left hand)
			if ((leftSheet == 0) && (rightSheet == 0))
			{
				pCSLeftHand->setSheet("");
			}
		}

		// update display of left hand according to new right hand item
		if (newVal > 0)
		{
			CCDBNodeLeaf *pNL2 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(newVal-1) + ":SHEET", false);
			if (pNL2 == NULL) return;

			if (getInventory().is2HandItem(pNL2->getValue32()))
			{
				if (getInventory().isRangeWeaponItem(pNL2->getValue32()))
				{
					pCSLeftHand->setItemSlot(SLOTTYPE::stringToSlotType("AMMO"));
					pCSLeftHand->setTextureNoItem(rVR.getTextureIdFromName("W_AM_logo.tga"));
				}
				else
				{
					pCSLeftHand->setSheet(LOCAL_INVENTORY ":BAG:"+ toString(newVal-1));
					pCSLeftHand->setGrayed(true);
					pCSLeftHand->setActionOnLeftClick("");
				}
			}
		}
	}

	// left hand item is changing
	if (sIE == CTRL_HAND_LEFT)
	{
		CDBCtrlSheet *pCSLeftHand = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_HAND_LEFT));
		if ( pCSLeftHand )
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			pCSLeftHand->setActionOnLeftClick("proc");
			pCSLeftHand->setGrayed(false);

			// if now there is nothing in left hand
			if (newVal == 0)
			{
				// check if we clear display (have to manage 2 hands weapons for instance)
				bool clearLeftHandDisplay = true;
				CDBCtrlSheet * pCSRightHand = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(CTRL_HAND_RIGHT));
				if ( pCSRightHand && pCSRightHand->getSheetId() )
				{
					CCDBNodeLeaf *pNL3 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":HAND:0:INDEX_IN_BAG", false);
					if (pNL3)
					{
						if (pNL3->getValue32() > 0)
						{
							CCDBNodeLeaf *pNL4 = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":BAG:" + toString(pNL3->getValue32()-1) + ":SHEET", false);
							if (pNL4)
							{
								uint32 rightSheet = pNL4->getValue32();
								if (getInventory().is2HandItem(rightSheet))
								{
									if (getInventory().isRangeWeaponItem(rightSheet))
									{
										pCSLeftHand->setItemSlot(SLOTTYPE::stringToSlotType("AMMO"));
										pCSLeftHand->setTextureNoItem(rVR.getTextureIdFromName("W_AM_logo.tga"));
									}
									else
									{
										pCSLeftHand->setItemSlot(SLOTTYPE::stringToSlotType("LEFT_HAND"));
										pCSLeftHand->setTextureNoItem(rVR.getTextureIdFromName("hand_left.tga"));
										clearLeftHandDisplay = false;
									}
								}
							}
						}
					}
				}
				if(clearLeftHandDisplay)
				{
					if (pCS  != NULL) pCS->setSheet("");
					if (pCS2 != NULL) pCS2->setSheet("");
				}
			}
		}
	}
}

// ***************************************************************************
void CInventoryManager::CDBBagObs::update(ICDBNode* /* node */)
{
	if (IngameDbMngr.initInProgress()) return;

	getInventory().checkIndexInBagIntegrity();

	// AUTO EQUIP the player with incoming item if we can put this item in an equipment slot

	// if we are not initializing the DB
/*
	CCDBNodeLeaf *pNL = dynamic_cast<CCDBNodeLeaf*>(node);
	if (pNL != NULL)
		if (pNL->getValue32() == 0)
			return;

	if (IngameDbMngr.initInProgress()) return;

	sint bagEntryIndex;

	string path = node->getFullName();
	path = path.substr(0,path.rfind(':'));
	path = path.substr(path.rfind(':')+1,path.size());

	fromString(path, bagEntryIndex);
	// equip only if slot empty
	getInventory().autoEquip(bagEntryIndex, false);
*/
}

// ***************************************************************************
bool CInventoryManager::autoEquip(sint bagEntryIndex, bool allowReplace)
{
	uint i;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	IListSheetBase *pList = dynamic_cast<IListSheetBase*>(CWidgetManager::getInstance()->getElementFromId(LIST_BAG_TEXT));
	CDBCtrlSheet *pCSSrc = NULL;

	if (pList == NULL) return false;

	for (i = 0; i < MAX_BAGINV_ENTRIES; ++i)
	{
		pCSSrc = pList->getSheet(i);
		string sTmp = pCSSrc->getSheet();
		sTmp = sTmp.substr(sTmp.rfind(':')+1,sTmp.size());
		sint nTmp;
		fromString(sTmp, nTmp);
		if (nTmp == bagEntryIndex)
			break;
	}

	if (i == MAX_BAGINV_ENTRIES) return false;
	if (pCSSrc == NULL) return false;

	for (i = 0; i < MAX_HANDINV_ENTRIES; ++i)
	{
		CDBCtrlSheet *pCSDst = getInventory().getHandSheet(i);
		if (pCSDst == NULL) continue;
		string dstPath = getInventory().getDBIndexPath(pCSDst);

		sint32 indexDstPath = NLGUI::CDBManager::getInstance()->getDbProp(dstPath+":INDEX_IN_BAG")->getValue16();

		// Already something in that slot?
		if (!allowReplace && indexDstPath > 0)
			continue;

		// Does the source and destination are items ?
		if (pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
		if (pCSDst->getType() == CCtrlSheetInfo::SheetType_Item)
		{
			// Right Slot ?
			if (pCSDst->canDropItem(pCSSrc))
			{
				// Ok let us equip with this item
				string srcPath = pCSSrc->getSheet();
				getInventory().equip (srcPath, dstPath);
				return true;
			}
		}
	}

	for (i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		CDBCtrlSheet *pCSDst = getInventory().getEquipSheet(i);
		if (pCSDst == NULL) continue;
		string dstPath = getInventory().getDBIndexPath(pCSDst);
		sint32 indexDstPath = NLGUI::CDBManager::getInstance()->getDbProp(dstPath+":INDEX_IN_BAG")->getValue16();

		// Already something in that slot?
		if (!allowReplace && indexDstPath > 0)
			continue;

		// Does the source and destination are items ?
		if (pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
		if (pCSDst->getType() == CCtrlSheetInfo::SheetType_Item)
		{
			// Right Slot ?
			if (pCSDst->canDropItem(pCSSrc))
			{
				// Ok let us equip with this item
				string srcPath = pCSSrc->getSheet();
				getInventory().equip (srcPath, dstPath);
				return true;
			}
		}
	}
	return false;
}

// ***************************************************************************
void CInventoryManager::dropOrDestroyItem(CDBCtrlSheet *item, CBitMemStream &out, uint16 quantity)
{
	if (!item) return;
	uint16 inventory = (uint16) item->getInventoryIndex();
	// retrieve inventory & slot
	uint16 slot = (uint16) item->getIndexInDB();
	out.serial(inventory);
	out.serial(slot);
	out.serial(quantity);
	NetMngr.push(out);
}

// ***************************************************************************
static void checkEquipmentIntegrity(const string &equipVal)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(equipVal+":INDEX_IN_BAG",false);
	if (pNL != NULL)
	{
		uint32 indexInBag = pNL->getValue16();
		if (indexInBag != 0)
		{
			string sTmp = string(LOCAL_INVENTORY) + ":BAG:" + toString(indexInBag-1) + ":SHEET";
			CCDBNodeLeaf *pNLBag = NLGUI::CDBManager::getInstance()->getDbProp(sTmp,false);
			if (pNLBag != NULL)
			{
				if (pNLBag->getValue32() == 0) // If no more item in this slot bag
				{
//					if (! IngameDbMngr.initInProgress())			// Commented because init is end when we received equipment
//						pNL->setValue16(0); // Reset INDEX_IN_BAG
				}
				else
				{
					// If the slot was previously empty check that there is no reference on it
					// else update the reference
					if (pNLBag->getOldValue32() == 0)
					{
						for (uint32 i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
						{
							CDBCtrlSheet *pCSDst = getInventory().getEquipSheet(i);
							if (pCSDst == NULL) continue;
							string dstPath = getInventory().getDBIndexPath(pCSDst);
							sint32 indexDstPath = NLGUI::CDBManager::getInstance()->getDbProp(dstPath+":INDEX_IN_BAG")->getValue16();

							// Update the sheet id of the control sheet
							if (indexDstPath == (sint32)indexInBag)
							{
								pCSDst->setSheetId(pNLBag->getValue32());
							}
						}
					}
				}
			}
		}
	}
}

// ***************************************************************************
void CInventoryManager::checkIndexInBagIntegrity()
{
	string sTmp;
	uint32 i;

	for (i = 0; i < MAX_HANDINV_ENTRIES; ++i)
	{
		sTmp = string(LOCAL_INVENTORY) + ":HAND:" + toString(i);
		checkEquipmentIntegrity(sTmp);
	}

	for (i = 0; i < MAX_EQUIPINV_ENTRIES; ++i)
	{
		sTmp = string(LOCAL_INVENTORY) + ":EQUIP:" + toString(i);
		checkEquipmentIntegrity(sTmp);
	}
}

// ***************************************************************************
double CInventoryManager::getBranchBulk(const string &basePath, uint16 startItemIndex, uint16 numItems)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeBranch *branch = NLGUI::CDBManager::getInstance()->getDbBranch(basePath);
	if (!branch)
	{
		nlwarning("<getBranchBulk> Branch is NULL");
		return 0;
	}

	double totalBulk = 0;
	//
	uint16 lastIndexItem = std::min((uint16) (startItemIndex + numItems), (uint16) branch->getNbNodes());
	for (uint16 currItem = startItemIndex; currItem < lastIndexItem; ++ currItem)
	{
		ICDBNode *node = branch->getNode(currItem);
		if (node)
		{
			CCDBNodeLeaf *sheetNode = dynamic_cast<CCDBNodeLeaf *>(node->getNode(ICDBNode::CTextId("SHEET")));
			CCDBNodeLeaf *quantityNode = dynamic_cast<CCDBNodeLeaf *>(node->getNode(ICDBNode::CTextId("QUANTITY")));
			if (sheetNode && quantityNode)
			{
				// get the Sheet
				CSheetId sheetId = CSheetId(sheetNode->getValue32());
				if (sheetId != CSheetId::Unknown)
				{
					CItemSheet	*itemSheet= dynamic_cast<CItemSheet*>(SheetMngr.get(sheetId));
					if(itemSheet)
					{
						totalBulk += std::max((sint32)1, quantityNode->getValue32()) * itemSheet->Bulk;
					}
				}
			}
		}
	}

	return totalBulk;
}

/*
 *Get the number of used and max slots
 */
void CInventoryManager::getBranchSlotCounts(const std::string &basePath, uint& nbUsedSlots, uint& nbMaxSlots )
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeBranch *branch = NLGUI::CDBManager::getInstance()->getDbBranch(basePath);
	if (!branch)
	{
		nlwarning("<getBranchSlotCounts> Branch is NULL");
		return;
	}

	nbMaxSlots = 0; // different from nbNodes, because there can be non-slots leaves (e.g. guild money...)
	nbUsedSlots = 0;
	uint nbNodes = branch->getNbNodes();
	for ( uint i=0; i!=nbNodes; ++i )
	{
		ICDBNode *node = branch->getNode(i);
		if (node)
		{
			CCDBNodeLeaf *sheetNode = dynamic_cast<CCDBNodeLeaf *>(node->getNode(ICDBNode::CTextId("SHEET")));
			if (sheetNode)
			{
				// Get the Sheet
				CSheetId sheetId = CSheetId(sheetNode->getValue32());
				if (sheetId != CSheetId::Unknown)
				{
					++nbUsedSlots;
				}

				++nbMaxSlots;
			}
		}
	}
}


// ***************************************************************************
double CInventoryManager::getBagBulk(uint32 inventoryIndex)
{
	nlctassert(MAX_INVENTORY_ANIMAL==4);
	if (inventoryIndex == 0)
		return getBranchBulk(LOCAL_INVENTORY ":BAG", 0, MAX_BAGINV_ENTRIES);
	else if (inventoryIndex == 1)
		return getBranchBulk(LOCAL_INVENTORY ":PACK_ANIMAL0", 0, MAX_ANIMALINV_ENTRIES);
	else if (inventoryIndex == 2)
		return getBranchBulk(LOCAL_INVENTORY ":PACK_ANIMAL1", 0, MAX_ANIMALINV_ENTRIES);
	else if (inventoryIndex == 3)
		return getBranchBulk(LOCAL_INVENTORY ":PACK_ANIMAL2", 0, MAX_ANIMALINV_ENTRIES);
	else if (inventoryIndex == 4)
		return getBranchBulk(LOCAL_INVENTORY ":PACK_ANIMAL3", 0, MAX_ANIMALINV_ENTRIES);
	else if (inventoryIndex == 5)
		return 0;
	else if (inventoryIndex == 6)
		return 0;
	else if (inventoryIndex == 7)
		return getBranchBulk(LOCAL_INVENTORY ":TEMP", 0, MAX_TEMPINV_ENTRIES);
	return 0;
}

// ***************************************************************************
double CInventoryManager::getItemBulk(uint32 sheetID)
{
	CItemSheet *itemSheet= dynamic_cast<CItemSheet*>(SheetMngr.get(CSheetId(sheetID)));
	if(itemSheet)
		return itemSheet->Bulk;
	return 0;
}

// ***************************************************************************
double CInventoryManager::getMaxBagBulk(uint32 inventoryIndex)
{
	nlctassert(MAX_INVENTORY_ANIMAL==4);
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNL=NULL;
	if (inventoryIndex == 0)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:STATIC_DATA:BAG_BULK_MAX");
	else if (inventoryIndex == 1)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:PACK_ANIMAL:BEAST0:BULK_MAX");
	else if (inventoryIndex == 2)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:PACK_ANIMAL:BEAST1:BULK_MAX");
	else if (inventoryIndex == 3)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:PACK_ANIMAL:BEAST2:BULK_MAX");
	else if (inventoryIndex == 4)
		pNL = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:PACK_ANIMAL:BEAST3:BULK_MAX");
	if (pNL != NULL)
		return pNL->getValue32();
	return 0;
}

// ***************************************************************************
bool CInventoryManager::isSpaceInAllBagsForItem(CDBCtrlSheet *item)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CDBCtrlSheet *pCSDst = item;
	if (!pCSDst->isSheetValid()) return false;
	string sTmp = pCSDst->getSheet();
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp+":SHEET",false);
	CCDBNodeLeaf *pNLquantity = NLGUI::CDBManager::getInstance()->getDbProp(sTmp+":QUANTITY",false);
	if (pNL == NULL) return false;
	if (pNLquantity == NULL) return false;

	// Check if we can find empty space for this item (or stack of item)
	// in all of the bags that the player owe.

	CInventoryManager *pInv = CInventoryManager::getInstance();
	uint32 quantity = pNLquantity->getValue32();
	double totalBulk = quantity * pInv->getItemBulk(pNL->getValue32());
//	bool bPlaceFound = false;
	for (uint32 i = 0; i < 7; ++i)
	{
		if (pInv->isInventoryAvailable((INVENTORIES::TInventory)CInventoryManager::InventoryIndexes[i]))
			if ((pInv->getBagBulk(i) + totalBulk) <= pInv->getMaxBagBulk(i))
				return true;
	}
	return false;
}

// ***************************************************************************
bool CInventoryManager::isSpaceInBagForItem(CDBCtrlSheet *item, uint32 quantity, uint32 bagId)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CDBCtrlSheet *pCSDst = item;
	if (!pCSDst->isSheetValid()) return false;
	string sTmp = pCSDst->getSheet();
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp+":SHEET",false);
	if (pNL == NULL) return false;

	// Check if we can find empty space for this item (or stack of item)
	// in a given bag that the player owe.

	CInventoryManager *pInv = CInventoryManager::getInstance();
	double totalBulk = quantity * pInv->getItemBulk(pNL->getValue32());
	if (pInv->isInventoryAvailable((INVENTORIES::TInventory)CInventoryManager::InventoryIndexes[bagId]))
		if ((pInv->getBagBulk(bagId) + totalBulk) <= pInv->getMaxBagBulk(bagId))
			return true;
	return false;
}


// ***************************************************************************
// CTempInvManager
// ***************************************************************************

// Observers on DB
// ***************************************************************************
void CTempInvManager::CDBObs::update(ICDBNode* /* node */)
{
	CTempInvManager::getInstance()->update();
}

void CTempInvManager::CDBObsType::update(ICDBNode* /* node */)
{
	CTempInvManager::getInstance()->updateType();
}

void CTempInvManager::CDBForageQQObs::update(ICDBNode* /* node */)
{
	CTempInvManager::getInstance()->updateForageQQ( WhichOne );
}

// ***************************************************************************
CTempInvManager::CTempInvManager()
{
	_Mode = TEMP_INV_MODE::Unknown;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	string sPath = string("LOCAL:INVENTORY:TEMP");
	for (uint i = 0; i < MAX_TEMPINV_ENTRIES; ++i)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sPath+":"+toString(i)+":SHEET", false);
		if (pNL != NULL)
		{
			ICDBNode::CTextId textId;
			pNL->addObserver(&_DBObs, textId);
		}
	}
	// Add Also the Mode to observe
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sPath+":TYPE", false);
	if(pNL)
	{
		ICDBNode::CTextId textId;
		pNL->addObserver(&_DBObsType, textId);
	}

	// Forage
	pNL = NLGUI::CDBManager::getInstance()->getDbProp(sPath+":ENABLE_TAKE");
	if (pNL != NULL)
	{
		ICDBNode::CTextId textId;
		pNL->addObserver(&_DBObs, textId);
	}
	pNL = NLGUI::CDBManager::getInstance()->getDbProp(sPath+":0:QUANTITY");
	_DBForageQQObs[0].WhichOne = 0;
	if (pNL != NULL)
	{
		ICDBNode::CTextId textId;
		pNL->addObserver(&_DBForageQQObs[0], textId);
	}
	pNL = NLGUI::CDBManager::getInstance()->getDbProp(sPath+":0:QUALITY");
	_DBForageQQObs[1].WhichOne = 1;
	if (pNL != NULL)
	{
		ICDBNode::CTextId textId;
		pNL->addObserver(&_DBForageQQObs[1], textId);
	}
}

// ***************************************************************************
CTempInvManager::~CTempInvManager()
{
}

// ***************************************************************************
void CTempInvManager::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

// ***************************************************************************
void CTempInvManager::update()
{
	bool bAllEmpty = true;
	// Check the database state
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	string sPath = string("LOCAL:INVENTORY:TEMP");
	for (uint i = 0; i < MAX_TEMPINV_ENTRIES; i++)
	{
		string sTmp = sPath + ":" + toString(i) + ":SHEET";
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp);
//		uint32 nOldSheet = pNL->getOldValue32();
		uint32 nSheet = pNL->getValue32();
		if (nSheet != 0)
			bAllEmpty = false;
	}

	_Mode = (TEMP_INV_MODE::TInventoryMode)NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:TEMP:TYPE")->getValue8();

	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_TEMPINV));
	if (pGC == NULL)
		return;

	// show/hide weight info depending on temp inventory mode
	bool displayWeight = (_Mode == TEMP_INV_MODE::Craft);
	CViewBase *weightText = dynamic_cast<CViewBase*>(pGC->getView("weight_txt"));	
	if (weightText != NULL)	
		weightText->setActive(displayWeight);
	CViewBase *weightImg = dynamic_cast<CViewBase*>(pGC->getView("weight"));
	if (weightImg != NULL)
		weightImg->setActive(displayWeight);	

	if (_Mode == TEMP_INV_MODE::Forage)
	{
		// Disable/enable "Take all" button
		bool disableTake = (NLGUI::CDBManager::getInstance()->getDbProp(sPath+":ENABLE_TAKE")->getValue32() == 0);
		NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP_INV:ALL_EMPTY")->setValue32(disableTake);
		if ( disableTake )
		{
			// Display begin of forage
			pGC->setTitle( WIN_TEMPINV_TITLE_WAIT_FORAGING );
			_DBForageQQObs[0].FullValue = 0.0f;
			_DBForageQQObs[1].FullValue = 0.0f;
		}
		else
		{
			// Display forage result
			pGC->setTitle( WIN_TEMPINV_TITLE_FORAGE_RESULT );
		}
	}
	else
	{
		// Write to the UI db the empty state
		NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP_INV:ALL_EMPTY")->setValue32(bAllEmpty);
	}

	if (bAllEmpty)
	{
		// If all slots are empty, close the interface
		pGC->setActive(false);
		CAHManager::getInstance()->runActionHandler("phrase_update_all_memory_ctrl_regen_tick_range", NULL);
	}
	else
	{
		pGC->setActive(true);
		CAHManager::getInstance()->runActionHandler("phrase_update_all_memory_ctrl_regen_tick_range", NULL);
		// Something arrived, change text
		switch(_Mode)
		{
			case TEMP_INV_MODE::Loot:			pGC->setTitle(WIN_TEMPINV_TITLE_LOOT); break;
			case TEMP_INV_MODE::Quarter:		pGC->setTitle(WIN_TEMPINV_TITLE_QUARTERING); break;
			case TEMP_INV_MODE::Forage:			/* see above */ break;
			case TEMP_INV_MODE::BagFull:		pGC->setTitle(WIN_TEMPINV_TITLE_BAGFULL); break;
			case TEMP_INV_MODE::Craft:			pGC->setTitle(WIN_TEMPINV_TITLE_CRAFT); break;
			case TEMP_INV_MODE::MissionReward:	pGC->setTitle(WIN_TEMPINV_TITLE_MISSIONREWARD); break;
			case TEMP_INV_MODE::Crystallize:	pGC->setTitle(WIN_TEMPINV_TITLE_CRYSTALLIZE); break;

			case TEMP_INV_MODE::Unknown:
			default:			pGC->setTitle(WIN_TEMPINV_TITLE_ERROR); break;
		}
	}
}

// ***************************************************************************
void CTempInvManager::updateType()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	_Mode = (TEMP_INV_MODE::TInventoryMode)NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:TEMP:TYPE")->getValue8();
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_TEMPINV));
	// Something arrived, change text
	switch(_Mode)
	{
		case TEMP_INV_MODE::Loot:			pGC->setTitle(WIN_TEMPINV_TITLE_LOOT); break;
		case TEMP_INV_MODE::Quarter:		pGC->setTitle(WIN_TEMPINV_TITLE_QUARTERING); break;
		case TEMP_INV_MODE::Forage:			/* see above */ break;
		case TEMP_INV_MODE::BagFull:		pGC->setTitle(WIN_TEMPINV_TITLE_BAGFULL); break;
		case TEMP_INV_MODE::Craft:			pGC->setTitle(WIN_TEMPINV_TITLE_CRAFT); break;
		case TEMP_INV_MODE::MissionReward:	pGC->setTitle(WIN_TEMPINV_TITLE_MISSIONREWARD); break;
		case TEMP_INV_MODE::Crystallize:	pGC->setTitle(WIN_TEMPINV_TITLE_CRYSTALLIZE); break;

		case TEMP_INV_MODE::Unknown:
		default:			/*pGC->setTitle(WIN_TEMPINV_TITLE_ERROR);*/ // do not overwrite a locally-set title with the default one when ServerAutoCopy syncs the local db during 'make item' (craft)
			break;
	}
}

// ***************************************************************************
// Called when INVENTORY:TEMP:0:QUANTITY or INVENTORY:TEMP:0:QUALITY is modified
// Reacts only if mode is Forage
void CTempInvManager::updateForageQQ( uint whichOne )
{
	if ( _Mode != TEMP_INV_MODE::Forage )
		return;

	// Avoid recursion, because we can't call CCDBNodeLeaf::setValue16() without calling observers!
	static bool isInUpdateForageQQ = false;
	if ( isInUpdateForageQQ )
		return;
	isInUpdateForageQQ = true;

	// Display forage progress with counters
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	bool disableTake = (NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:TEMP:ENABLE_TAKE")->getValue32() == 0);
	if ( disableTake )
	{
		float qt = 0.f, ql = 0.f;
		switch ( whichOne )
		{
		case 0:
			{
				CCDBNodeLeaf *leafQt = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:TEMP:0:QUANTITY");
				uint16 qtX10 = (uint16)(leafQt->getValue16());
				qt = _DBForageQQObs[whichOne].FullValue = (((float)(uint)qtX10) / 10.0f);
				leafQt->setValue16( (sint16)(sint)qt );
				ql = _DBForageQQObs[1-whichOne].FullValue;
			}
			break;
		case 1:
			{
				CCDBNodeLeaf *leafQl = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:TEMP:0:QUALITY");
				uint16 qlX10 = (uint16)(leafQl->getValue16());
				ql = _DBForageQQObs[whichOne].FullValue = (((float)(uint)qlX10) / 10.0f);
				leafQl->setValue16( (sint16)(sint)ql );
				qt = _DBForageQQObs[1-whichOne].FullValue;
			}
			break;
		default:;
		}
		ucstring title = CI18N::get( WIN_TEMPINV_TITLE_FORAGING );
		strFindReplace( title, "%qt", toString( "%.1f", qt ) );
		strFindReplace( title, "%ql", toString( "%.1f", ql ) );
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_TEMPINV));
		pGC->setUCTitle( title );
	}

	isInUpdateForageQQ = false;
}

// ***************************************************************************
void CTempInvManager::open(TEMP_INV_MODE::TInventoryMode m)
{
	_Mode = m;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_TEMPINV));

	// In Foraging mode, we can call open() on the inventory with the same contents (e.g. when changing Forage action)
	if ( _Mode != TEMP_INV_MODE::Forage )
	{
		string sPath = string("LOCAL:INVENTORY:TEMP");
		for (uint i = 0; i < MAX_TEMPINV_ENTRIES; i++)
		{
			string sTmp = sPath + ":" + toString(i) + ":SHEET";
			CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp);
			pNL->setValue32(0);
		}
	}
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:TEMP:TYPE")->setValue8((uint8)_Mode);

	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	if (pGC != NULL)
	{
		switch(_Mode)
		{
			case TEMP_INV_MODE::Loot:			pGC->setTitle(WIN_TEMPINV_TITLE_WAIT_LOOT); break;
			case TEMP_INV_MODE::Quarter:		pGC->setTitle(WIN_TEMPINV_TITLE_WAIT_QUARTERING); break;
			case TEMP_INV_MODE::Forage:			pGC->setTitle(WIN_TEMPINV_TITLE_WAIT_FORAGING); break;
			case TEMP_INV_MODE::BagFull:		pGC->setTitle(WIN_TEMPINV_TITLE_WAIT_BAGFULL); break;
			case TEMP_INV_MODE::Craft:			pGC->setTitle(WIN_TEMPINV_TITLE_WAIT_CRAFT); break;
			case TEMP_INV_MODE::MissionReward:	pGC->setTitle(WIN_TEMPINV_TITLE_WAIT_MISSIONREWARD); break;
			case TEMP_INV_MODE::Crystallize:	pGC->setTitle(WIN_TEMPINV_TITLE_WAIT_CRYSTALLIZE); break;

			case TEMP_INV_MODE::Unknown:
			default:			pGC->setTitle(WIN_TEMPINV_TITLE_WAIT_ERROR); break;
		};

		pGC->setActive(true);
		CAHManager::getInstance()->runActionHandler("phrase_update_all_memory_ctrl_regen_tick_range", NULL);
	}
}

// ***************************************************************************
void CTempInvManager::close()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	string sPath = string("LOCAL:INVENTORY:TEMP");

	// Clear temp inventory if needed
	for (uint i = 0; i < MAX_TEMPINV_ENTRIES; i++)
	{
		string sTmp = sPath + ":" + toString(i) + ":SHEET";
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp);
		pNL->setValue32(0);
	}

	CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(WIN_TEMPINV));
	if (pIG != NULL)
	{
		pIG->setActive(false);
		CAHManager::getInstance()->runActionHandler("phrase_update_all_memory_ctrl_regen_tick_range", NULL);
	}
}

// ***************************************************************************
bool CTempInvManager::isOpened()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_TEMPINV));
	if (pGC != NULL)
		return pGC->getActive();
	return false;
}

// ***************************************************************************
// BAG LISTS COMMON STUFF (sort, options ...)
// ***************************************************************************

// ***************************************************************************
#define BAG_ITEM_NOT_SORTED 1000000
// Used for sorting
void initStructForItemSort(vector<SSortStruct>&vTemp, sint32 sheetId, sint32 quality, sint32 indexInList, sint32 indexInDB)
{
	// Default value is the linear pos in the db (in case its not an item)
	vTemp[indexInList].Pos = toString("%08d", indexInDB);

	// if not empty
	if (sheetId != 0)
	{
		CEntitySheet *pItem = SheetMngr.get(CSheetId(sheetId));
		if ((pItem != NULL) && (pItem->Type == CEntitySheet::ITEM))
		{
			CItemSheet *pIS = safe_cast<CItemSheet*>(pItem);
			vTemp[indexInList].Pos = toString("%02d", pIS->Family);
			vTemp[indexInList].Pos += toString("%03d", pIS->ItemType);

			// add some specific sort for raw material
			if (pIS->Family == ITEMFAMILY::RAW_MATERIAL)
				vTemp[indexInList].Pos += toString("%010d", pIS->Mp.ItemPartBF);
			else
				vTemp[indexInList].Pos += toString("%010d", 0);

			// add some specific sort for teleport
			if (pIS->Family == ITEMFAMILY::TELEPORT)
				vTemp[indexInList].Pos += toString("%02d%02d", pIS->ItemOrigin, pIS->Teleport.Type);
			else
				vTemp[indexInList].Pos += toString("%02d%02d", 0, 0);


			vTemp[indexInList].Pos += toString("%03d", quality);

            // add sort by name
            vTemp[indexInList].Pos += CSheetId(sheetId).toString();


			// add at last the index in DB. to avoid resort for items that are exaclty the same
			vTemp[indexInList].Pos += toString("%03d", indexInDB);
		}
	}
}

// ***************************************************************************
// Used for common options
bool SBagOptions::parse(xmlNodePtr cur, CInterfaceGroup * /* parentGroup */)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	// read params
	CXMLAutoPtr prop;

	// value
	prop = xmlGetProp (cur, (xmlChar*)"inv_type");
	if (prop)
	{
		InvType = CInventoryManager::invTypeFromString(prop.str());
	}
	else
	{
		InvType = CInventoryManager::InvUnknown;
		nlwarning("cannot find inventory type");
	}

	prop = xmlGetProp (cur, (xmlChar*)"filter_armor");
	if (prop) DbFilterArmor = NLGUI::CDBManager::getInstance()->getDbProp(prop.str());

	prop = xmlGetProp (cur, (xmlChar*)"filter_weapon");
	if (prop) DbFilterWeapon = NLGUI::CDBManager::getInstance()->getDbProp(prop.str());

	prop = xmlGetProp (cur, (xmlChar*)"filter_tool");
	if (prop) DbFilterTool = NLGUI::CDBManager::getInstance()->getDbProp(prop.str());

	prop = xmlGetProp (cur, (xmlChar*)"filter_mp");
	if (prop) DbFilterMP = NLGUI::CDBManager::getInstance()->getDbProp(prop.str());

	prop = xmlGetProp (cur, (xmlChar*)"filter_missmp");
	if (prop) DbFilterMissMP = NLGUI::CDBManager::getInstance()->getDbProp(prop.str());

	prop = xmlGetProp (cur, (xmlChar*)"filter_tp");
	if (prop) DbFilterTP = NLGUI::CDBManager::getInstance()->getDbProp(prop.str());

	return true;
}

// ***************************************************************************
bool SBagOptions::isSomethingChanged()
{
	bool bRet = false;

	if (DbFilterArmor != NULL)
		if ((DbFilterArmor->getValue8() != 0) != LastDbFilterArmor)
		{
			bRet = true;
			LastDbFilterArmor = (DbFilterArmor->getValue8() != 0);
		}

	if (DbFilterWeapon != NULL)
		if ((DbFilterWeapon->getValue8() != 0) != LastDbFilterWeapon)
		{
			bRet = true;
			LastDbFilterWeapon = (DbFilterWeapon->getValue8() != 0);
		}

	if (DbFilterTool != NULL)
		if ((DbFilterTool->getValue8() != 0) != LastDbFilterTool)
		{
			bRet = true;
			LastDbFilterTool = (DbFilterTool->getValue8() != 0);
		}

	if (DbFilterMP != NULL)
		if ((DbFilterMP->getValue8() != 0) != LastDbFilterMP)
		{
			bRet = true;
			LastDbFilterMP = (DbFilterMP->getValue8() != 0);
		}

	if (DbFilterMissMP != NULL)
		if ((DbFilterMissMP->getValue8() != 0) != LastDbFilterMissMP)
		{
			bRet = true;
			LastDbFilterMissMP = (DbFilterMissMP->getValue8() != 0);
		}

	if (DbFilterTP != NULL)
		if ((DbFilterTP->getValue8() != 0) != LastDbFilterTP)
		{
			bRet = true;
			LastDbFilterTP = (DbFilterTP->getValue8() != 0);
		}

	return bRet;
}

// ***************************************************************************
bool SBagOptions::canDisplay(CDBCtrlSheet *pCS) const
{
	bool bDisplay = true;

	bool bFilterArmor = getFilterArmor();
	bool bFilterWeapon = getFilterWeapon();
	bool bFilterTool = getFilterTool();
	bool bFilterMP = getFilterMP();
	bool bFilterMissMP = getFilterMissMP();
	bool bFilterTP = getFilterTP();

	const CItemSheet *pIS = pCS->asItemSheet();
	if (pIS != NULL)
	{
		// Armor
		if ((pIS->Family == ITEMFAMILY::ARMOR) || 
			(pIS->Family == ITEMFAMILY::JEWELRY))
			if (!bFilterArmor) bDisplay = false;

		// Weapon
		if ((pIS->Family == ITEMFAMILY::SHIELD) || 
			(pIS->Family == ITEMFAMILY::MELEE_WEAPON) ||
			(pIS->Family == ITEMFAMILY::RANGE_WEAPON) || 
			(pIS->Family == ITEMFAMILY::AMMO) ||
			(pIS->Family == ITEMFAMILY::CRYSTALLIZED_SPELL) || 
			(pIS->Family == ITEMFAMILY::ITEM_SAP_RECHARGE) ||
			(pIS->Family == ITEMFAMILY::BRICK) )
			if (!bFilterWeapon) bDisplay = false;

		// Tool
		if ((pIS->Family == ITEMFAMILY::CRAFTING_TOOL) || 
			(pIS->Family == ITEMFAMILY::HARVEST_TOOL) ||
			(pIS->Family == ITEMFAMILY::TAMING_TOOL) || 
			(pIS->Family == ITEMFAMILY::TRAINING_TOOL) ||
			(pIS->Family == ITEMFAMILY::BAG) || 
			(pIS->Family == ITEMFAMILY::PET_ANIMAL_TICKET) )
			if (!bFilterTool) bDisplay = false;

		// MP
		if ((pIS->Family == ITEMFAMILY::RAW_MATERIAL) && pIS->canBuildSomeItemPart())
			if (!bFilterMP) bDisplay = false;

		// Mission MP
		if ((pIS->Family == ITEMFAMILY::MISSION_ITEM) ||
			(pIS->Family == ITEMFAMILY::XP_CATALYSER) ||
			(pIS->Family == ITEMFAMILY::CONSUMABLE) ||
			((pIS->Family == ITEMFAMILY::RAW_MATERIAL) && !pIS->canBuildSomeItemPart()))
			if (!bFilterMissMP) bDisplay = false;

		// Teleporter Pacts
		if ((pIS->Family == ITEMFAMILY::TELEPORT))
			if (!bFilterTP) bDisplay = false;

		// Jobs Items
		if (pIS->Id.toString().substr(0, 6) == "rpjob_")
			bDisplay = false;
	}

	return bDisplay;
}

// ***************************************************************************
// CDBGroupListSheetBag
// ***************************************************************************


// ***************************************************************************
bool CDBGroupListSheetBag::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CDBGroupListSheetText::parse(cur, parentGroup))
		return false;

	// Parse options (type, filters ...)
	if (!_BO.parse(cur,parentGroup))
		return false;

	return true;
}

// ***************************************************************************
void CDBGroupListSheetBag::checkCoords ()
{
	CDBGroupListSheetText::checkCoords();
	if (_BO.isSomethingChanged())
		invalidateCoords();
}

// ***************************************************************************
void CDBGroupListSheetBag::CSheetChildBag::updateViewText(CDBGroupListSheetText * /* pFather */)
{
	// common method to update text as item
	updateViewTextAsItem();
}

// ***************************************************************************
bool CDBGroupListSheetBag::CSheetChildBag::isSheetValid(CDBGroupListSheetText *pFather)
{
	if (CSheetChild::isSheetValid(pFather))
	{
		// Check if the control match the filters !
		CDBGroupListSheetBag *pList = dynamic_cast<CDBGroupListSheetBag*>(pFather);
		if (pList)
			return pList->canDisplay(Ctrl);
	}
	return false;
}

// ***************************************************************************
void CDBGroupListSheetBag::CSheetChildBag::init(CDBGroupListSheetText *pFather, uint index)
{
	// init my parent
	CSheetChild::init(pFather, index);

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// **** Bind the quality
	{
		// Basic quality
		string	db= Ctrl->getSheet()+":QUALITY";
		if( NLGUI::CDBManager::getInstance()->getDbProp(db, false) )
			CurrentQuality.link ( db.c_str() );
		else
		{
			// dummy link to ui:.....
			CurrentQuality.link("UI:DUMMY:QUALITY");
			CurrentQuality.setSInt32(0);
		}
	}
}

// ***************************************************************************
bool CDBGroupListSheetBag::CSheetChildBag::isInvalidated(CDBGroupListSheetText * /* pFather */)
{
	// quality change
	if( CurrentQuality.getSInt32() != LastQuality )
		return true;

	return false;
}

// ***************************************************************************
void CDBGroupListSheetBag::CSheetChildBag::update(CDBGroupListSheetText * /* pFather */)
{
	LastQuality= CurrentQuality.getSInt32();
}

// ***************************************************************************
void CDBGroupListSheetBag::onSwap (sint /* nDraggedSheet */, sint /* nDroppedSheet */)
{
	// No more used because automatic sort
}

// ***************************************************************************
void CDBGroupListSheetBag::sort()
{
	vector<SSortStruct> vTemp;

	vTemp.resize (_MaxItems);

	uint i;
	for (i = 0; i < _MaxItems; ++i)
	{
		vTemp[i].SheetText = _SheetChildren[i];

		CDBCtrlSheet	*ctrl= _SheetChildren[i]->Ctrl;
		initStructForItemSort (vTemp, ctrl->getSheetId(), ctrl->getQuality(), i, ctrl->getIndexInDB());
	}

	std::sort(vTemp.begin(), vTemp.end());

	for (i = 0; i < _MaxItems; ++i)
	{
		_SheetChildren[i] = vTemp[i].SheetText;
	}
}

// ***************************************************************************
// CDBGroupIconListBag
// ***************************************************************************

// ***************************************************************************
bool CDBGroupIconListBag::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CDBGroupListSheet::parse(cur, parentGroup))
		return false;

	// Parse options (type, filters ...)
	if (!_BO.parse(cur,parentGroup))
		return false;

	return true;
}

// ***************************************************************************
void CDBGroupIconListBag::sort()
{
	vector<SSortStruct> vTemp;

	vTemp.resize (_MaxItems);

	uint i;
	for (i = 0; i < _MaxItems; ++i)
	{
		vTemp[i].SheetIcon = _SheetChildren[i];

		CDBCtrlSheet	*ctrl= _SheetChildren[i]->Ctrl;
		initStructForItemSort (vTemp, ctrl->getSheetId(), ctrl->getQuality(), i, ctrl->getIndexInDB());
	}

	std::sort(vTemp.begin(), vTemp.end());

	for (i = 0; i < _MaxItems; ++i)
	{
		_SheetChildren[i] = vTemp[i].SheetIcon;
	}
}

// ***************************************************************************
void CDBGroupIconListBag::checkCoords ()
{
	CDBGroupListSheet::checkCoords();
	if (_BO.isSomethingChanged())
		invalidateCoords();
}

// ***************************************************************************
bool CDBGroupIconListBag::CSheetChildBag::isSheetValid(CDBGroupListSheet *pFather)
{
	if (CSheetChild::isSheetValid(pFather))
	{
		// Check if the control match the filters !
		CDBGroupIconListBag *pList = dynamic_cast<CDBGroupIconListBag*>(pFather);
		if (pList)
			return pList->canDisplay(Ctrl);
	}
	return false;
}



// ***************************************************************************
// CDBGroupListSheetFilterCLMSlot
// ***************************************************************************

// ***************************************************************************
bool CDBGroupListSheetFilterCLMSlot::CSheetChildFilter::isSheetValid(CDBGroupListSheet *pFather)
{
	if (CSheetChild::isSheetValid(pFather))
	{
		/* This filter look the ctrl who launch the modal where this list is displayed.
			If we can drop this ChildCtrl, on the CLM control, then ok, filtered
			Plus the ChildControl must not be locked
		*/
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *clmCtrl = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (!clmCtrl || !Ctrl) return false;
		if (clmCtrl->getInventoryIndex() == INVENTORIES::exchange &&
			Ctrl->getInventoryIndex() == INVENTORIES::exchange)
		{
			return false;
		}
		if ((clmCtrl->getType() == CCtrlSheetInfo::SheetType_Item) &&
			(Ctrl->getType() == CCtrlSheetInfo::SheetType_Item) )
		{
			// Ok if we can put in the slot Ctrl in clmCtrl
			if ( clmCtrl->canDropItem(Ctrl))
			{
				string sTmp = Ctrl->getSheet();
				// Look if the source is locked
				sTmp = sTmp.substr(sTmp.rfind(':')+1,sTmp.size());
				sint32 nTmp;
				fromString(sTmp, nTmp);
				if (!getInventory().isBagItemWeared(nTmp))
					return true;
			}
		}
	}
	return false;
}

// ***************************************************************************
// CDBGroupListSheetFilterExchangeable
// ***************************************************************************

// ***************************************************************************
bool CDBGroupListSheetFilterExchangeable::CSheetChildFilter::isSheetValid(CDBGroupListSheet *pFather)
{
	if (CSheetChild::isSheetValid(pFather))
	{
		if(!Ctrl)
			return false;
		extern bool checkCanExchangeItem(CDBCtrlSheet *);
		return checkCanExchangeItem(Ctrl);
	}
	return false;
}

// ***************************************************************************
void CDBGroupListSheetFilterExchangeable::sort()
{
	vector<SSortStruct> vTemp;

	vTemp.resize (_MaxItems);

	uint i;
	for (i = 0; i < _MaxItems; ++i)
	{
		vTemp[i].SheetIcon = _SheetChildren[i];

		CDBCtrlSheet	*ctrl= _SheetChildren[i]->Ctrl;
		initStructForItemSort (vTemp, ctrl->getSheetId(), ctrl->getQuality(), i, ctrl->getIndexInDB());
	}

	std::sort(vTemp.begin(), vTemp.end());

	for (i = 0; i < _MaxItems; ++i)
	{
		_SheetChildren[i] = vTemp[i].SheetIcon;
	}
}

// ***************************************************************************
bool CDBGroupListSheetFilterExchangeable::parse(xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CDBGroupListSheet::parse(cur, parentGroup))
		return false;

	// Parse options (type, filters ...)
	if (!_BO.parse(cur,parentGroup))
		return false;

	return true;
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ACTION HANDLERS
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
// COMMON INVENTORIES DRAG'N'DROP
// ***************************************************************************

// ***************************************************************************
class CHandlerInvCanDrag : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSSrc == NULL) return;

		// Cannot drag an item if it is completely locked
		if (pCSSrc->getNonLockedQuantity() == 0)
		{
			pCSSrc->setTempCanDrag(false);
			return;
		}

		// If the item comes from an animal list which is not available -> cannot move
		if (strnicmp(pCSSrc->getSheet().c_str(), "LOCAL:INVENTORY:PACK_ANIMAL", 27) == 0)
		{
			string sTmp = pCSSrc->getSheet().substr(0, pCSSrc->getSheet().rfind(':'));
			sTmp = sTmp.substr(27,sTmp.size());
			uint32 nAni;
			fromString(sTmp, nAni);
			INVENTORIES::TInventory inv = (INVENTORIES::TInventory)(INVENTORIES::pet_animal+nAni);
			pCSSrc->setTempCanDrag(getInventory().isInventoryAvailable(inv));
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerInvCanDrag, "inv_can_drag" );

// ***************************************************************************
class CHandlerInvDrag : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCS == NULL) return;
		if (Params == "from_text_list")
			getInventory().beginDrag(pCS, CInventoryManager::TextList);
		else if (Params == "from_slot")
			getInventory().beginDrag(pCS, CInventoryManager::Slot);
		else if (Params == "from_icon_list")
			getInventory().beginDrag(pCS, CInventoryManager::IconList);
		else
			nlwarning("DND not binded");
	}
};
REGISTER_ACTION_HANDLER( CHandlerInvDrag, "inv_drag" );

// ***************************************************************************
// COMMON INVENTORIES Test if we can drop an item to a slot or a list
class CHandlerInvCanDropTo : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		// pCSSrc is the current dragged item
		// pCSDst is a slot or a list

		CInventoryManager *pInv = CInventoryManager::getInstance();
		if (!pInv->isDragging()) return; // To prevent other things to happens

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string	src = getParam(Params, "src");
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		if (pCSSrc == NULL) return; // Cannot do anything if the incoming sheet is not a sheet

		// Special case if we want to DND an animal representation
		// -------------------------------------------------------
		{
			const CItemSheet *pIS = pCSSrc->asItemSheet();
			if ((pIS != NULL) && (pIS->Family == ITEMFAMILY::PET_ANIMAL_TICKET))
			{
				CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
				if (pCSDst != NULL)
				{
					// The destination must be exchange or gift
					if (strnicmp(pCSDst->getSheet().c_str(),"LOCAL:EXCHANGE:GIVE", 19) == 0)
					{
						// The animal bag must be empty before giving it
						if (pIS->Pet.Slot > 0)
						if (pInv->isInventoryEmpty((INVENTORIES::TInventory)(INVENTORIES::pet_animal+(pIS->Pet.Slot-1))))
							pCSDst->setCanDrop (true);
					}
					// Or moving in the bag (user sort)
					if (strnicmp(pCSDst->getSheet().c_str(),"LOCAL:INVENTORY:BAG", 19) == 0)
						pCSDst->setCanDrop (true);
				}
				return;
			}
		}

		// The destination is a slot or something like that ?
		// --------------------------------------------------
		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSDst != NULL)
		{
			// If we want to drop something on a reference slot (hand or equip)
			if (pInv->getDBIndexPath(pCSDst) != "")
			{
				// We must drag'n'drop an item
				if (pCSSrc && pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
				if (pCSDst && pCSDst->getType() == CCtrlSheetInfo::SheetType_Item)
				{
					if (pInv->getDBIndexPath(pCSSrc) != "")
					{
						// The item dragged comes from a slot check if this is the good type
						if (pCSDst->canDropItem(pCSSrc))
							pCSDst->setCanDrop ( true );
					}
					else
					{
						// The source must be from the bag
						if (strnicmp(pCSSrc->getSheet().c_str(),"LOCAL:INVENTORY:BAG", 19) == 0)
							pCSDst->setCanDrop ( true );
					}
				}
			}
			else
			{
				// Does the source and destination are items ?
				if (pCSSrc && pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
				if (pCSDst && pCSDst->getType() == CCtrlSheetInfo::SheetType_Item)
				{
					// Right Slot ?
					if (pCSDst->canDropItem(pCSSrc))
						pCSDst->setCanDrop ( true );
				}
			}
			return;
		}

		// The destination is a list of items ?
		// ------------------------------------
		CDBGroupListSheetBag *pListDstText = dynamic_cast<CDBGroupListSheetBag*>(pCaller);
		CDBGroupIconListBag  *pListDstIcon = dynamic_cast<CDBGroupIconListBag*>(pCaller);
		if ((pListDstText != NULL) || (pListDstIcon != NULL))
		{
			bool bCanDrop = true;
			// WE CANT DND if we want to dnd from other bag than BAG to guild bag
			if (pListDstIcon != NULL)
			{
				if (pListDstIcon->getInvType() == CInventoryManager::InvGuild)
					if (strnicmp(pCSSrc->getSheet().c_str(),"LOCAL:INVENTORY:BAG", 19) != 0)
						bCanDrop = false;
			}
			if (pListDstText != NULL)
			{
				if (pListDstText->getInvType() == CInventoryManager::InvGuild)
					if (strnicmp(pCSSrc->getSheet().c_str(),"LOCAL:INVENTORY:BAG", 19) != 0)
						bCanDrop = false;
			}
			// WE CANT DND if we want to dnd from guild bag to other bag than BAG
			if (pListDstIcon != NULL)
			{
				if (pListDstIcon->getInvType() != CInventoryManager::InvBag)
					if (strnicmp(pCSSrc->getSheet().c_str(),"SERVER:GUILD:INVENTORY", 19) == 0)
						bCanDrop = false;
			}
			if (pListDstText != NULL)
			{
				if (pListDstText->getInvType() != CInventoryManager::InvBag)
					if (strnicmp(pCSSrc->getSheet().c_str(),"SERVER:GUILD:INVENTORY", 19) == 0)
						bCanDrop = false;
			}

			// WE CANT DND when packer/mount is too far
			if (pListDstIcon != NULL)
			{
				if ((pListDstIcon->getInvType() == CInventoryManager::InvPA0) ||
					(pListDstIcon->getInvType() == CInventoryManager::InvPA1) ||
					(pListDstIcon->getInvType() == CInventoryManager::InvPA2) ||
					(pListDstIcon->getInvType() == CInventoryManager::InvPA3))
				{
					INVENTORIES::TInventory e = (INVENTORIES::TInventory)(INVENTORIES::pet_animal1 + (pListDstIcon->getInvType()-CInventoryManager::InvPA0));
					if (!pInv->isInventoryAvailable(e))
						bCanDrop = false;
				}
			}
			if (pListDstText != NULL)
			{
				if ((pListDstText->getInvType() == CInventoryManager::InvPA0) ||
					(pListDstText->getInvType() == CInventoryManager::InvPA1) ||
					(pListDstText->getInvType() == CInventoryManager::InvPA2) ||
					(pListDstText->getInvType() == CInventoryManager::InvPA3))
				{
					INVENTORIES::TInventory e = (INVENTORIES::TInventory)(INVENTORIES::pet_animal1 + (pListDstText->getInvType()-CInventoryManager::InvPA0));
					if (!pInv->isInventoryAvailable(e))
						bCanDrop = false;
				}
			}

			// If the source is the equipment

			if (pListDstText != NULL) pListDstText->setCanDrop(bCanDrop);
			if (pListDstIcon != NULL) pListDstIcon->setCanDrop(bCanDrop);
		}

	}
};
REGISTER_ACTION_HANDLER( CHandlerInvCanDropTo, "inv_can_drop" );

// ***************************************************************************
class CHandlerInvDropTo : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Check that we have drag'n'drop from inventory (list or slot)
		// Or if we have launched the choose_bag modal
		// To prevent other things to happens
		if (!getInventory().isDragging())
		{
			CInterfaceGroup *pIG = CWidgetManager::getInstance()->getModalWindow();
			if (pIG == NULL) return;
			if (pIG->getId() != "ui:interface:bag_choose") return;
			getInventory().beginDrag(NULL, CInventoryManager::TextList);

			// Special case for choose in bag dialog
			string	src = getParam(Params, "src");
			CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
			CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
			CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);

			string invPath = getInventory().getDBIndexPath(pCSSrc);
			string bagPath = pCSDst->getSheet();

			if (bagPath == "UI:EMPTY")
				getInventory().unequip (invPath);
			else
				getInventory().equip (bagPath, invPath);

			getInventory().endDrag();
			return;
		}

		string	src = getParam(Params, "src");
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSSrc == NULL) return;

		// Is the dragged sheet comes from an inventory list
		if (getInventory().isDraggingFromTextList() || getInventory().isDraggingFromIconList())
		{
			// If the destination is an equipment slot ?
			if (pCSDst != NULL)
			{
				string invPath = getInventory().getDBIndexPath(pCSDst); // Get the index in the equipment
				if (!invPath.empty())
				{
					// Drop to the slot ie write the database with the index of the slot
					string bagPath = pCSSrc->getSheet(); // Get the database branch of the dragged sheet

					if (pCSSrc && pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
					if (pCSDst && pCSDst->getType() == CCtrlSheetInfo::SheetType_Item)
					{
						// If the destination slot match with the type of incoming item
						if (pCSDst->canDropItem(pCSSrc))
						{
							// So directly equip
							getInventory().equip(bagPath, invPath);
						}
						else
						{
							// Else try to auto equip the player with the incoming item
							const string sTmp = bagPath.substr(bagPath.rfind(':')+1,bagPath.size());
							sint index;
							fromString(sTmp, index);
							if (!getInventory().autoEquip(index, false))
								getInventory().autoEquip(index, true);
						}
					}
					getInventory().endDrag();
					return;
				}
			}

			// If the destination is a list sheet
			IListSheetBase *pListDst = dynamic_cast<IListSheetBase*>(pCaller);
			if ((pListDst == NULL) && (pCSDst != NULL))
				pListDst = IListSheetBase::getListContaining(pCSDst);
			IListSheetBase *pListSrc = IListSheetBase::getListContaining(pCSSrc);
			if ((pListDst != NULL) && (pListSrc != NULL))
			{
				// If the source list and destination list are the same
				if (pListDst == pListSrc)
				{
					// no op
					getInventory().endDrag();
					return;
				}
				else // Source list and destination list are not the same
				{
					// Move the item to the destination list using the procedure move_to_xxx
					CDBGroupListSheetBag *pListDstText = dynamic_cast<CDBGroupListSheetBag*>(pListDst);
					CDBGroupIconListBag *pListDstIcon = dynamic_cast<CDBGroupIconListBag*>(pListDst);

					if (((pListDstText != NULL) && (pListDstText->getInvType() == CInventoryManager::InvBag)) ||
						((pListDstIcon != NULL) && (pListDstIcon->getInvType() == CInventoryManager::InvBag)))
					{
						CAHManager::getInstance()->runActionHandler("proc", pCSSrc, "move_to_bag");
					}
					else if (((pListDstText != NULL) && ((pListDstText->getInvType() == CInventoryManager::InvPA0) ||
														 (pListDstText->getInvType() == CInventoryManager::InvPA1) ||
														 (pListDstText->getInvType() == CInventoryManager::InvPA2) ||
														 (pListDstText->getInvType() == CInventoryManager::InvPA3)
														)) ||
							((pListDstIcon != NULL) && ((pListDstIcon->getInvType() == CInventoryManager::InvPA0) ||
														(pListDstIcon->getInvType() == CInventoryManager::InvPA1) ||
														(pListDstIcon->getInvType() == CInventoryManager::InvPA2) ||
														(pListDstIcon->getInvType() == CInventoryManager::InvPA3)
														)))
					{
						string sTmp;
						if (pListDstText != NULL) sTmp = toString("%d",pListDstText->getInvType()-CInventoryManager::InvPA0);
						if (pListDstIcon != NULL) sTmp = toString("%d",pListDstIcon->getInvType()-CInventoryManager::InvPA0);
						CAHManager::getInstance()->runActionHandler("proc", pCSSrc, "move_to_pa|"+sTmp);
					}
					else if (((pListDstText != NULL) && (pListDstText->getInvType() == CInventoryManager::InvGuild)) ||
							 ((pListDstIcon != NULL) && (pListDstIcon->getInvType() == CInventoryManager::InvGuild)))
					{
						if (strnicmp(pCSSrc->getSheet().c_str(), "LOCAL:INVENTORY:BAG", 19) == 0)
							CAHManager::getInstance()->runActionHandler("proc", pCSSrc, "move_to_guild");
					}
					else if (((pListDstText != NULL) && (pListDstText->getInvType() == CInventoryManager::InvRoom)) ||
							 ((pListDstIcon != NULL) && (pListDstIcon->getInvType() == CInventoryManager::InvRoom)))
					{
						CAHManager::getInstance()->runActionHandler("proc", pCSSrc, "move_to_room");
					}
				}
			}
		}

		// Is the dragged sheet comes from another slot
		if (pCSDst != NULL)
		if (getInventory().isDraggingFromSlot())
		{
			// Yes swap the 2 indices
			// Get the database branch of the dragged sheet
			string invPath1 = getInventory().getDBIndexPath(pCSSrc);
			// Get the index in the equipment
			string invPath2 = getInventory().getDBIndexPath(pCSDst);

			sint32 i1 = NLGUI::CDBManager::getInstance()->getDbProp(invPath1+":INDEX_IN_BAG")->getValue16();
			sint32 i2 = NLGUI::CDBManager::getInstance()->getDbProp(invPath2+":INDEX_IN_BAG")->getValue16();

			getInventory().unequip(invPath1);
			getInventory().unequip(invPath2);

			string sBag = LOCAL_INVENTORY ":BAG:";
			if (i2 != 0) getInventory().equip(sBag + toString(i2-1), invPath1);
			if (i1 != 0) getInventory().equip(sBag + toString(i1-1), invPath2);
		}

		CAHManager::getInstance()->runActionHandler("inv_cannot_drop", pCSSrc);
	}
};
REGISTER_ACTION_HANDLER( CHandlerInvDropTo, "inv_drop" );


// ***************************************************************************
// EQUIPMENT
class CHandlerInvCannotDrop : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		// Is the dragged sheet comes from a slot
		if (!getInventory().isDraggingFromTextList())
		{
			// Unequip
			CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
			string invPath = getInventory().getDBIndexPath(pCSDst);
			getInventory().unequip(invPath);
		}
		getInventory().endDrag();
	}
};
REGISTER_ACTION_HANDLER( CHandlerInvCannotDrop, "inv_cannot_drop" );


// ***************************************************************************
class CHandlerInvAutoEquip : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet *cs = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if(cs)
		{
			uint	index= cs->getIndexInDB();
			if (!getInventory().autoEquip(index, false))
				getInventory().autoEquip(index, true);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerInvAutoEquip, "inv_auto_equip" );


// **********************************************************************************************************
class CHandlerLockInvItem : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		// get the calling item
		CDBCtrlSheet *item = CDBCtrlSheet::getCurrSelSheet();
		if ( ! item)
		{
			nlwarning("<CHandlerDestroyItem::execute> no caller sheet found");
			return;
		}

		string lock = "1";
		if (item->getLockedByOwner())
		{
			lock = "0";
		}

		uint32 slot = item->getIndexInDB();
		uint32 inv = item->getInventoryIndex();
		INVENTORIES::TInventory inventory = INVENTORIES::UNDEFINED;
		inventory = (INVENTORIES::TInventory)(inv);
		if (inventory == INVENTORIES::UNDEFINED)
		{
			return;
		}
		NLMISC::ICommand::execute("a lockItem " + INVENTORIES::toString(inventory) + " " + toString(slot) + " " + lock, g_log);
	}
};
REGISTER_ACTION_HANDLER( CHandlerLockInvItem, "lock_inv_item" );

// ***************************************************************************
// Inventory Temporary
// ***************************************************************************

// ***************************************************************************
class CHandlerInvTempToBag : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Disable the direct click on item in Forage mode and Can't take all
		if ( (CTempInvManager::getInstance()->getMode() == TEMP_INV_MODE::Forage) &&
			 (NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:INVENTORY:TEMP:ENABLE_TAKE")->getValue32() == 0) )
			return;

		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (!pCSDst->isSheetValid()) return;
		string sTmp = pCSDst->getSheet();

		sTmp = sTmp.substr(sTmp.rfind(':')+1, sTmp.size());
		uint16 index;
		fromString(sTmp, index);

		// If we cant find place display a message and dont send the request to the server
		if (!getInventory().isSpaceInAllBagsForItem(pCSDst))
		{
			ucstring msg = CI18N::get("msgCantPutItemInBag");
			string cat = getStringCategory(msg, msg);
			pIM->displaySystemInfo(msg, cat);
			return;
		}

		sTmp = pCSDst->getSheet();
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp+":SHEET",false);
		pNL->setValue32(0);

		CBitMemStream out;
		if (!GenericMsgHeaderMngr.pushNameToStream("ITEM:TEMP_TO_BAG", out))
		{
			nlwarning ("don't know message name ITEM:TEMP_TO_BAG");
		}
		else
		{
			// Fill the message (temporary inventory slot)
			out.serial(index);
			NetMngr.push (out);
			//nlinfo("impulseCallBack : ITEM:TEMP_TO_BAG %d sent", index);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerInvTempToBag, "inv_temp_to_bag" );

// ***************************************************************************
class CHandlerInvTempAll : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInventoryManager *pInv = CInventoryManager::getInstance();

		// Try to put all items in the DB order in all the bags of the player : (bag, pa0-3, steed)
		vector <pair <double, double> > BagsBulk;
		BagsBulk.push_back(pair <double, double>(pInv->getBagBulk(0), pInv->getMaxBagBulk(0)));

		nlctassert(MAX_INVENTORY_ANIMAL==4);
		if (pInv->isInventoryAvailable(INVENTORIES::pet_animal1))
			BagsBulk.push_back(pair <double, double>(pInv->getBagBulk(1), pInv->getMaxBagBulk(1)));
		if (pInv->isInventoryAvailable(INVENTORIES::pet_animal2))
			BagsBulk.push_back(pair <double, double>(pInv->getBagBulk(2), pInv->getMaxBagBulk(2)));
		if (pInv->isInventoryAvailable(INVENTORIES::pet_animal3))
			BagsBulk.push_back(pair <double, double>(pInv->getBagBulk(3), pInv->getMaxBagBulk(3)));
		if (pInv->isInventoryAvailable(INVENTORIES::pet_animal4))
			BagsBulk.push_back(pair <double, double>(pInv->getBagBulk(4), pInv->getMaxBagBulk(4)));

		bool bPlaceFound = true;

		for (uint32 itemNb = 0; itemNb < MAX_TEMPINV_ENTRIES; ++itemNb)
		{
			CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":TEMP:" + toString(itemNb) + ":SHEET");
			CCDBNodeLeaf *pNLquantity = NLGUI::CDBManager::getInstance()->getDbProp(LOCAL_INVENTORY ":TEMP:" + toString(itemNb) + ":QUANTITY");
			if (pNL == NULL || pNLquantity == NULL) continue;
			if (pNL->getValue32() == 0 || pNLquantity->getValue32() == 0) continue;
			double itemBulk = pNLquantity->getValue32() * pInv->getItemBulk(pNL->getValue32());

			bool bLocalPlaceFound = false;
			for (uint32 i = 0; i < BagsBulk.size(); ++i)
			{
				if ((BagsBulk[i].first + itemBulk) <= BagsBulk[i].second)
				{
					BagsBulk[i].first += itemBulk;
					bLocalPlaceFound = true;
					break;
				}
			}
			if (!bLocalPlaceFound)
			{
				bPlaceFound = false;
				break;
			}
		}

		if (!bPlaceFound)
		{
			ucstring msg = CI18N::get("msgCantPutItemInBag");
			string cat = getStringCategory(msg, msg);
			CInterfaceManager::getInstance()->displaySystemInfo(msg, cat);
			return;
		}

		CBitMemStream out;
		if (!GenericMsgHeaderMngr.pushNameToStream("ITEM:ALL_TEMP", out))
		{
			nlwarning ("don't know message name ITEM:ALL_TEMP");
		}
		else
		{
			NetMngr.push (out);
			//nlinfo("impulseCallBack : ITEM:ALL_TEMP sent");
		}
		CTempInvManager::getInstance()->close();
	}
};
REGISTER_ACTION_HANDLER( CHandlerInvTempAll, "inv_temp_all" );

// ***************************************************************************
class CHandlerInvTempNone : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CBitMemStream out;
		if (!GenericMsgHeaderMngr.pushNameToStream("ITEM:NO_TEMP", out))
		{
			nlwarning ("don't know message name ITEM:NO_TEMP");
		}
		else
		{
			NetMngr.push (out);
			//nlinfo("impulseCallBack : ITEM:NO_TEMP sent");
		}
		CTempInvManager::getInstance()->close();
	}
};
REGISTER_ACTION_HANDLER( CHandlerInvTempNone, "inv_temp_none" );


// ***************************************************************************
// ***************************************************************************
// ItemInfo System
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void			CClientItemInfo::readFromImpulse(const CItemInfos &itemInfo)
{
	*(CItemInfos*)this= itemInfo;

	// Read now the version in the impulse
	InfoVersionFromMsg= itemInfo.versionInfo;
}

// ***************************************************************************
uint16				CInventoryManager::getItemSlotId(CDBCtrlSheet *ctrl)
{
	return getItemSlotId(ctrl->getSheet(), ctrl->getIndexInDB());
}

// ***************************************************************************
uint16				CInventoryManager::getItemSlotId(const std::string &itemDb, uint slotIndex)
{
	// then compare to all possible inventories (ugly)
	uint	inventoryIndex= 0;
	for(uint i=0;i<NumInventories;i++)
	{
		if( itemDb.find(InventoryDBs[i])!=string::npos )
		{
			inventoryIndex= InventoryIndexes[i];
			break;
		}
	}

	// and compress the slotId
	return (inventoryIndex<<CItemInfos::SlotIdIndexBitSize) + slotIndex;
}

// ***************************************************************************
uint				CInventoryManager::getItemSheetForSlotId(uint slotId) const
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// for all inventories
	for(uint i=0;i<NumInventories;i++)
	{
		if( InventoryIndexes[i] == (slotId>>CItemInfos::SlotIdIndexBitSize) )
		{
			CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp( toString( "SERVER:%s:%d:SHEET", InventoryDBs[i].c_str(), (slotId&CItemInfos::SlotIdIndexBitMask)), false);
			if(node)
				return node->getValue32();
			else
				return 0;
		}
	}

	return 0;
}

// ***************************************************************************
const	CClientItemInfo	&CInventoryManager::getItemInfo(uint slotId) const
{
	TItemInfoMap::const_iterator	it= _ItemInfoMap.find(slotId);
	static	CClientItemInfo	empty;
	if(it==_ItemInfoMap.end())
		return empty;
	else
		return it->second;
}

// ***************************************************************************
bool				CInventoryManager::isItemInfoUpToDate(uint slotId)
{
	// true if the version already matches
	return getItemInfo(slotId).InfoVersionFromMsg == getItemInfo(slotId).InfoVersionFromSlot;
}

// ***************************************************************************
void				CInventoryManager::addItemInfoWaiter(IItemInfoWaiter *waiter)
{
	if(!waiter)
		return;

	// first remove the waiter if already here
	removeItemInfoWaiter(waiter);

	// Then push_front (stack)
	_ItemInfoWaiters.push_front(waiter);

	// update server msg
	updateItemInfoQueue();
}

// ***************************************************************************
void				CInventoryManager::removeItemInfoWaiter(IItemInfoWaiter *waiter)
{
	TItemInfoWaiters::iterator	it;
	for(it= _ItemInfoWaiters.begin();it!=_ItemInfoWaiters.end();it++)
	{
		if(waiter==*it)
		{
			_ItemInfoWaiters.erase(it);
			break;
		}
	}
}

// ***************************************************************************
void			CInventoryManager::updateItemInfoWaiters(uint itemSlotId)
{
	// First verify if the versions matches. If differ, no need to update waiters since not good.
	if(getItemInfo(itemSlotId).InfoVersionFromMsg != getItemInfo(itemSlotId).InfoVersionFromSlot)
		return;

	bool	isItemFromTrading= (itemSlotId>>CItemInfos::SlotIdIndexBitSize)==INVENTORIES::trading;

	// For all waiters that wait for this item slot
	TItemInfoWaiters::iterator	it;
	for(it= _ItemInfoWaiters.begin();it!=_ItemInfoWaiters.end();)
	{
		TItemInfoWaiters::iterator	itNext=it;
		itNext++;
		IItemInfoWaiter *waiter=*it;
		// callback. NB: only ig the waiter agree the Sheet in the current slot id
		if(waiter->ItemSlotId == itemSlotId &&
			(isItemFromTrading || waiter->ItemSheet == getItemSheetForSlotId(itemSlotId)) )
		{
			waiter->infoReceived();
		}

		// in case it is erased in infoReceived()
		it= itNext;
	}
}

// ***************************************************************************
void CInventoryManager::CItemInfoSlotVersionObs::update(ICDBNode* node)
{
	getInventory().onReceiveItemInfoSlotVersion(node);
}

// ***************************************************************************
void CInventoryManager::CItemSheetObs::update(ICDBNode* node)
{
	getInventory().onReceiveItemSheet(node);
}

// ***************************************************************************
void CInventoryManager::CItemInfoTradeObs::update(ICDBNode* /* node */)
{
	getInventory().onTradeChangeSession();
}

// ***************************************************************************
void			CInventoryManager::onReceiveItemInfoSlotVersion(ICDBNode* node)
{
	uint	itemSlotId;

	// valid only if of form 0:INFO_VERSION
	if( !dynamic_cast<CCDBNodeLeaf*>(node) || !node->getParent() || !node->getParent()->getName())
		return;

	// get the slot in inventory of the item
	uint	slot;
	fromString(*node->getParent()->getName(), slot);

	// get the slotId for this DB
	itemSlotId= getItemSlotId(node->getFullName(), slot);

	// update the InfoVersionFromSlot
	_ItemInfoMap[itemSlotId].InfoVersionFromSlot= ((CCDBNodeLeaf*)node)->getValue8();

	// Look for Waiters that match this item slot => update
	updateItemInfoWaiters(itemSlotId);

	// new msg to send, because version updated?
	updateItemInfoQueue();
}

// ***************************************************************************
void			CInventoryManager::onReceiveItemSheet(ICDBNode* node)
{
	// When a SHEET change, it may unblock some ITEM_INFO:GET calls or some info waiters update.
	uint	itemSlotId;

	// valid only if of form 0:INFO_VERSION
	if( !dynamic_cast<CCDBNodeLeaf*>(node) || !node->getParent() || !node->getParent()->getName())
		return;

	// get the slot in inventory of the item
	uint	slot;
	fromString(*node->getParent()->getName(), slot);

	// get the slotId for this DB
	itemSlotId= getItemSlotId(node->getFullName(), slot);

	// Look for Waiters that match this item slot => update
	updateItemInfoWaiters(itemSlotId);

	// new msg to send, because sheet ok?
	updateItemInfoQueue();
}

// ***************************************************************************
void			CInventoryManager::onReceiveItemInfo(const CItemInfos &itemInfo)
{
	uint	itemSlotId;

	// update the Info
	itemSlotId= itemInfo.slotId;
	// write in map, from DB.
	_ItemInfoMap[itemSlotId].readFromImpulse(itemInfo);

	// Look for Waiters that match this item slot => update
	updateItemInfoWaiters(itemSlotId);

	// new msg to send?
	updateItemInfoQueue();
}

// ***************************************************************************
void			CInventoryManager::onRefreshItemInfoVersion(uint16 slotId, uint8 infoVersion)
{
	_ItemInfoMap[slotId].refreshInfoVersion( infoVersion );
}

// ***************************************************************************
void			CInventoryManager::onTradeChangeSession()
{
	// Dummy set a 255 InfoVersionMsg for all items in trade. Because Server cannot do the code.
	for(uint i=0;i<TRADE_MAX_ENTRIES;i++)
	{
		uint	itemSlotId = getItemSlotId("TRADING", i);
		// Since InfoVersionFromSlot always == 0, a msg will be sent to have new version
		_ItemInfoMap[itemSlotId ].InfoVersionFromMsg= 255;
		_ItemInfoMap[itemSlotId ].InfoVersionSlotServerWaiting= 255;
	}

	// new msg to send?
	updateItemInfoQueue();
}

// ***************************************************************************
void			CInventoryManager::updateItemInfoQueue()
{
	// For All waiters, look if one need update.
	TItemInfoWaiters::iterator	it;
	for(it= _ItemInfoWaiters.begin();it!=_ItemInfoWaiters.end();it++)
	{
		/* \todo yoyo remove: temp patch to be sure that the client does not send messages before the
			CONNECTION:READY is sent
		*/
		nlassert(ConnectionReadySent);

		IItemInfoWaiter *waiter=*it;
		uint	itemSlotId= waiter->ItemSlotId;
		TItemInfoMap::iterator	it= _ItemInfoMap.find(itemSlotId);
		bool	isItemFromTrading= (itemSlotId>>CItemInfos::SlotIdIndexBitSize)==INVENTORIES::trading;
		if(it!=_ItemInfoMap.end())
		{
			CClientItemInfo	&itemInfo= it->second;
			// If versions differ, and if a msg was not already sent to get the new version, then ask server
			// SheetId must also match
			if(	itemInfo.InfoVersionFromSlot != itemInfo.InfoVersionFromMsg &&
				itemInfo.InfoVersionFromSlot != itemInfo.InfoVersionSlotServerWaiting &&
				( isItemFromTrading || waiter->ItemSheet == getItemSheetForSlotId(itemSlotId)) )
			{
				// Send a msg to server
				if(!ClientCfg.Local)
				{
					CBitMemStream out;
					if (GenericMsgHeaderMngr.pushNameToStream("ITEM_INFO:GET", out))
					{
						uint16	slotId= itemSlotId;
						out.serial( slotId );
						NetMngr.push(out);
						//nlinfo("impulseCallBack : ITEM_INFO:GET %d sent", slotId);
					}
					else
					{
						nlwarning(" unknown message name 'ITEM_INFO:GET'");
					}
				}
				// Debug
				else
				{
					// debug:
					CInterfaceManager	*pIM= CInterfaceManager::getInstance();
					pIM->displaySystemInfo( toString("ITEM_INFO:GET, slotId: %d, slotVersion: %d",
						itemSlotId, itemInfo.InfoVersionFromSlot) );
				}

				itemInfo.InfoVersionSlotServerWaiting= itemInfo.InfoVersionFromSlot;
			}
		}
	}
}

// ***************************************************************************
void			CInventoryManager::debugItemInfoWaiters()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// For All waiters, log.
	TItemInfoWaiters::iterator	it;
	for(it= _ItemInfoWaiters.begin();it!=_ItemInfoWaiters.end();it++)
	{
		IItemInfoWaiter *waiter=*it;
		uint	itemSlotId= waiter->ItemSlotId;
		TItemInfoMap::iterator	itMap= _ItemInfoMap.find(itemSlotId);
		if(itMap!=_ItemInfoMap.end())
		{
			CClientItemInfo	&itemInfo= itMap->second;
			pIM->displaySystemInfo( toString("ItemInfoWaiter: slotId: %d, slotVersion: %d, msgVersion: %d, msgVersionWait: %d",
				itemSlotId, itemInfo.InfoVersionFromSlot, itemInfo.InfoVersionFromMsg, itemInfo.InfoVersionSlotServerWaiting) );
		}
	}
}

// ***************************************************************************
void CInventoryManager::sortBag()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CDBGroupIconListBag *pIconList;
	CDBGroupListSheetBag *pList;
	
	pIconList = dynamic_cast<CDBGroupIconListBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_BAG_ICONS));
	if (pIconList != NULL) pIconList->needToSort();
	pList = dynamic_cast<CDBGroupListSheetBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_BAG_TEXT));
	if (pList != NULL) pList->needToSort();

	pIconList = dynamic_cast<CDBGroupIconListBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_ROOM_ICONS));
	if (pIconList != NULL) pIconList->needToSort();
	pList = dynamic_cast<CDBGroupListSheetBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_ROOM_TEXT));
	if (pList != NULL) pList->needToSort();

	pIconList = dynamic_cast<CDBGroupIconListBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_GUILD_ICONS));
	if (pIconList != NULL) pIconList->needToSort();
	pList = dynamic_cast<CDBGroupListSheetBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_GUILD_TEXT));
	if (pList != NULL) pList->needToSort();

	pIconList = dynamic_cast<CDBGroupIconListBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_PA0_ICONS));
	if (pIconList != NULL) pIconList->needToSort();
	pList = dynamic_cast<CDBGroupListSheetBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_PA0_TEXT));
	if (pList != NULL) pList->needToSort();

	pIconList = dynamic_cast<CDBGroupIconListBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_PA1_ICONS));
	if (pIconList != NULL) pIconList->needToSort();
	pList = dynamic_cast<CDBGroupListSheetBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_PA1_TEXT));
	if (pList != NULL) pList->needToSort();

	pIconList = dynamic_cast<CDBGroupIconListBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_PA2_ICONS));
	if (pIconList != NULL) pIconList->needToSort();
	pList = dynamic_cast<CDBGroupListSheetBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_PA2_TEXT));
	if (pList != NULL) pList->needToSort();

	pIconList = dynamic_cast<CDBGroupIconListBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_PA3_ICONS));
	if (pIconList != NULL) pIconList->needToSort();
	pList = dynamic_cast<CDBGroupListSheetBag*>(CWidgetManager::getInstance()->getElementFromId(LIST_PA3_TEXT));
	if (pList != NULL) pList->needToSort();
}

// ***************************************************************************
bool				CInventoryManager::isInventoryPresent(INVENTORIES::TInventory invId)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// PA present?
	if(invId>=INVENTORIES::pet_animal && invId<INVENTORIES::pet_animal+MAX_INVENTORY_ANIMAL)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:STATUS", invId-INVENTORIES::pet_animal), false);
		if(!node)	return false;
		uint	status= node->getValue32();

		return ANIMAL_STATUS::isSpawned((ANIMAL_STATUS::EAnimalStatus)status);
	}
	else if (invId == INVENTORIES::guild)
	{
		return (NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:INVENTORY_GUILD_OPENED", true)->getValue8() != 0);
	}
	else if (invId == INVENTORIES::player_room)
	{
		return (NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:INVENTORY_ROOM_OPENED", true)->getValue8() != 0);
	}
	// other inventories (Bag...) are always present
	else
		return true;
}


// ***************************************************************************
bool				CInventoryManager::isInventoryAvailable(INVENTORIES::TInventory invId)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// PA available?
	if(invId>=INVENTORIES::pet_animal && invId<INVENTORIES::pet_animal+MAX_INVENTORY_ANIMAL)
	{
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:STATUS", invId-INVENTORIES::pet_animal), false);
		if(!node)	return false;
		uint	status= node->getValue32();

		return ANIMAL_STATUS::isInventoryAvailable((ANIMAL_STATUS::EAnimalStatus)status);
	}
	else if (invId == INVENTORIES::guild)
	{
		return (NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:INVENTORY_GUILD_OPENED", true)->getValue8() != 0);
	}
	else if (invId == INVENTORIES::player_room)
	{
		return (NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:INVENTORY_ROOM_OPENED", true)->getValue8() != 0);
	}
	// other inventories (Bag...) are always available
	else
		return true;
}

// ***************************************************************************
bool CInventoryManager::isInventoryEmpty(INVENTORIES::TInventory invId)
{
	if (!isInventoryPresent(invId)) return true;

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	string sPath = "SERVER:INVENTORY";
	sint32 nNbEntries = 256;

	// PA present?
	if(invId>=INVENTORIES::pet_animal && invId<INVENTORIES::pet_animal+MAX_INVENTORY_ANIMAL)
	{
		nNbEntries = MAX_ANIMALINV_ENTRIES;
		sPath += toString(":PACK_ANIMAL%d", invId-INVENTORIES::pet_animal);
	}
	else if (invId == INVENTORIES::guild)
	{
		nNbEntries = MAX_GUILDINV_ENTRIES;
		sPath += "GUILD";
	}
	else if (invId == INVENTORIES::player_room)
	{
		nNbEntries = MAX_ROOMINV_ENTRIES;
		sPath += "ROOM";
	}
	else
		return true;


	for (uint32 i = 0; (sint)i < nNbEntries; ++i)
	{
		CCDBNodeLeaf *pNL;
		pNL = NLGUI::CDBManager::getInstance()->getDbProp(sPath+toString(":%d:SHEET", i), false);
		if (pNL == NULL) return true;
		if (pNL->getValue32() != 0)
			return false;
	}
	return true;
}

// ***************************************************************************
CItemImage &CInventoryManager::getPAItem(uint beastIndex, uint index)
{
	nlassert(beastIndex < MAX_INVENTORY_ANIMAL);
	nlassert(index < MAX_ANIMALINV_ENTRIES);
	return PAInv[beastIndex][index];
}

// ***************************************************************************
CItemImage &CInventoryManager::getServerPAItem(uint beastIndex, uint index)
{
	nlassert(beastIndex < MAX_INVENTORY_ANIMAL);
	nlassert(index < MAX_ANIMALINV_ENTRIES);
	return ServerPAInv[beastIndex][index];
}

// ***************************************************************************
CItemImage &CInventoryManager::getLocalItem(uint inv, uint index)
{
	if(inv==INVENTORIES::bag)
		return getBagItem(index);
	if(inv>=INVENTORIES::pet_animal && inv<INVENTORIES::pet_animal+MAX_INVENTORY_ANIMAL)
		return getPAItem(inv-INVENTORIES::pet_animal, index);

	nlstop;
	static	CItemImage dummy;
	return dummy;
}

// ***************************************************************************
CItemImage &CInventoryManager::getServerItem(uint inv, uint index)
{
	if(inv==INVENTORIES::bag)
		return getServerBagItem(index);
	if(inv>=INVENTORIES::pet_animal && inv<INVENTORIES::pet_animal+MAX_INVENTORY_ANIMAL)
		return getServerPAItem(inv-INVENTORIES::pet_animal, index);

	nlstop;
	static	CItemImage dummy;
	return dummy;
}

// ***************************************************************************
CInventoryManager::TInvType CInventoryManager::invTypeFromString(const string &str)
{
	string sTmp = strlwr(str);
	if (sTmp == "inv_bag")		return InvBag;
	if (sTmp == "inv_pa0")		return InvPA0;
	if (sTmp == "inv_pa1")		return InvPA1;
	if (sTmp == "inv_pa2")		return InvPA2;
	if (sTmp == "inv_pa3")		return InvPA3;
	if (sTmp == "inv_guild")	return InvGuild;
	if (sTmp == "inv_room")		return InvRoom;
	return InvUnknown;
}
