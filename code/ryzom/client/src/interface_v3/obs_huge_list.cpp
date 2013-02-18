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

#include "interface_manager.h"
#include "obs_huge_list.h"
#include "bot_chat_manager.h"
#include "bot_chat_page_all.h"
#include "bot_chat_page_trade.h"
#include "dbgroup_list_sheet_trade.h"
#include "sphrase_manager.h"
#include "../sheet_manager.h"
#include "game_share/seeds.h"
#include "../client_sheets/sphrase_sheet.h"
#include "../net_manager.h"
#include "sbrick_manager.h"

extern CSheetManager SheetMngr;

using namespace std;
using namespace NLMISC;

#define	UI_GROUP_OF_ITEMS_FOR_MONEY "ui:interface:bot_chat_trade:header_opened:trade_content:buy"
#define	UI_LIST_OF_ITEMS_FOR_MONEY UI_GROUP_OF_ITEMS_FOR_MONEY ":general_buy"
//
#define	UI_GROUP_OF_ITEMS_FOR_MONEY_BUY_ONLY "ui:interface:bot_chat_buy:header_opened:trade_content:buy"
#define	UI_LIST_OF_ITEMS_FOR_MONEY_BUY_ONLY UI_GROUP_OF_ITEMS_FOR_MONEY_BUY_ONLY ":general_buy"
//
#define	UI_GROUP_OF_ITEMS_FOR_MISSION "ui:interface:bot_chat_trade:header_opened:trade_content:list_of_items_for_missions"
#define	UI_LIST_OF_ITEMS_FOR_MISSION UI_GROUP_OF_ITEMS_FOR_MISSION ":items_for_mission"

#define	UI_GROUP_OF_ITEMS_FOR_ASCENSOR "ui:interface:ascensor_teleport_list:header_opened:list_of_items"
#define	UI_LIST_OF_ITEMS_FOR_ASCENSOR UI_GROUP_OF_ITEMS_FOR_ASCENSOR ":all_items"

#define UI_GROUP_OF_MISSIONS  "ui:interface:bot_chat_missions:header_opened:missions_content"
#define UI_LIST_OF_MISSIONS  UI_GROUP_OF_MISSIONS ":missions"


// ***************************************************************************
class CPhrasePriceCB : public	IBrickLearnedCallback
{
public:
	CHugeListObs	*Owner;

public:
	CPhrasePriceCB(CHugeListObs *owner) : Owner(owner) {}

	virtual	void	onBrickLearned()
	{
		// on any brick learned, must recompute all phrases (if done by client)
		Owner->clientGenerateAllPhrases();

		// on any brick learned, must recompute price of all phrase
		Owner->recomputeAllPhrasePrices();
	}
};

// ***************************************************************************
CHugeListObs::CHugeListObs(ListType category) : _Init(false)
{
	_Category = category;
	_CurrentSessionNb = 0;
	_MType= MISSION_DESC::Mission;
	_FirstPageNotUpdated= 0;
	_PhrasePriceCB= NULL;
	_PhrasePriceUpdateAndMaybeClientFill= false;

	// Client-Side phrase filling
	_PhraseClientFill= false;
	_PhraseClientFillNumPhrase= 0;
	_PhraseClientFillFlags= 0;
	_PhraseClientFillRace= 0;
	_RoleMasterFlagDB= NULL;
	_RoleMasterRaceDB= NULL;

	_DownloadComplete = false;

	// set the list type
	if(category!=ListTypeCount)
		setListType(category);
}

// ***************************************************************************
CHugeListObs::~CHugeListObs()
{
	// if the callback has been created, remove it
	if(_PhrasePriceCB)
	{
		CSBrickManager	*pBM= CSBrickManager::getInstance();
		pBM->removeBrickLearnedCallback(_PhrasePriceCB);
		delete _PhrasePriceCB;
		_PhrasePriceCB= NULL;
	}
}

// ***************************************************************************
void CHugeListObs::setListType(ListType listType)
{
	nlassert((uint) listType < ListTypeCount); // category has already been set
	_Category = listType;

	// For phrase trading, append the callback that will update phrase prices according to brick knowns
	if(_Category==Trading && _PhrasePriceCB==NULL)
	{
		CSBrickManager	*pBM= CSBrickManager::getInstance();
		_PhrasePriceCB= new CPhrasePriceCB(this);
		pBM->appendBrickLearnedCallback(_PhrasePriceCB);
	}
}


// ****************************************************************************
bool CHugeListObs::init()
{
	nlassert((uint) _Category < ListTypeCount); // must call setListType
	if (_Init) return true;
	std::string dbPath;
	switch(_Category)
	{
		case Trading: dbPath = DB_BRANCH_FOR_ITEM_FOR_MONEY; break;
		case ItemForMissions: dbPath = DB_BRANCH_FOR_ITEM_FOR_MISSION; break;
		case Ascensor: dbPath = DB_BRANCH_FOR_ASCENSOR; break;
		case Missions: dbPath = DB_BRANCH_FOR_MISSIONS; break;
		default:
			nlassert(0); // Provide path for accessing the elements (SERVER: ... )
		break;
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	_Session =	NLGUI::CDBManager::getInstance()->getDbProp(dbPath + ":SESSION", false);
	if (!_Session) return false;
	_PageID =	NLGUI::CDBManager::getInstance()->getDbProp(dbPath + ":PAGE_ID", false);
	if (!_PageID) return false;
	_HasNext =	NLGUI::CDBManager::getInstance()->getDbProp(dbPath + ":HAS_NEXT", false);
	if (!_HasNext) return false;

	// optional (rolemaster Trading only)
	_RoleMasterFlagDB = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + ":ROLEMASTER_FLAGS", false);
	_RoleMasterRaceDB = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + ":ROLEMASTER_RACE", false);

	// get all items
	for(uint k = 0; k < TRADE_PAGE_NUM_ITEMS; ++k)
	{
		switch(_Category)
		{
			case Trading:
				_Items[k].SlotType = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:SLOT_TYPE").c_str(), (int) k), false);
				_Items[k].Quality = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:QUALITY").c_str(), (int) k), false);
				_Items[k].SheetIDOrSkill = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:SHEET").c_str(), (int) k), false);
				_Items[k].Price = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:PRICE").c_str(), (int) k), false);
				_Items[k].Weight = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:WEIGHT").c_str(), (int) k), false);
				_Items[k].NameId = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:NAMEID").c_str(), (int) k), false);
				_Items[k].UserColor = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:USER_COLOR").c_str(), (int) k), false);
				_Items[k].Enchant = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:ENCHANT").c_str(), (int) k), false);
				_Items[k].RMClassType = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:RM_CLASS_TYPE").c_str(), (int) k), false);
				_Items[k].RMFaberStatType = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:RM_FABER_STAT_TYPE").c_str(), (int) k), false);
				_Items[k].PrerequisitValid = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:PREREQUISIT_VALID").c_str(), (int) k), false);
				_Items[k].InfoVersion = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:INFO_VERSION").c_str(), (int) k), false);
				_Items[k].Quantity = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:QUANTITY").c_str(), (int) k), false);
				_Items[k].PriceRetire = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:PRICE_RETIRE").c_str(), (int) k), false);
				_Items[k].ResaleTimeLeft = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:RESALE_TIME_LEFT").c_str(), (int) k), false);
				_Items[k].VendorNameId = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:VENDOR_NAMEID").c_str(), (int) k), false);
				_Items[k].FactionType = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:FACTION_TYPE").c_str(), (int) k), false);
				_Items[k].FactionPointPrice = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:PRICE").c_str(), (int) k), false);
				_Items[k].SellerType = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:SELLER_TYPE").c_str(), (int) k), false);
				if ((_Items[k].SlotType == NULL) || (_Items[k].Quality == NULL) || (_Items[k].SheetIDOrSkill == NULL) ||
					(_Items[k].Price == NULL) || (_Items[k].Weight==NULL) || (_Items[k].InfoVersion==NULL) ||
					(_Items[k].UserColor==NULL) || (_Items[k].NameId==NULL) || (_Items[k].Quantity==NULL) ||
					(_Items[k].PriceRetire==NULL) || (_Items[k].SellerType==NULL) || (_Items[k].ResaleTimeLeft==NULL)  ||
					(_Items[k].VendorNameId==NULL)  || (_Items[k].Enchant ==NULL) || (_Items[k].RMClassType == NULL) ||
					(_Items[k].RMFaberStatType == NULL) || (_Items[k].PrerequisitValid == NULL) ||
					(_Items[k].FactionType == NULL) || (_Items[k].FactionPointPrice == NULL)
				   )
					return false;
			break;
			case ItemForMissions:
				_Items[k].SlotType = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:SLOT_TYPE").c_str(), (int) k), false);
				_Items[k].Quality = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:QUALITY").c_str(), (int) k), false);
				_Items[k].SheetIDOrSkill = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:SHEET").c_str(), (int) k), false);
				_Items[k].LogicTextID = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:LOGIC_TEXT_ID").c_str(), (int) k), false);
				_Items[k].DescTextID = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:DESC_TEXT_ID").c_str(), (int) k), false);
				_Items[k].Weight = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:WEIGHT").c_str(), (int) k), false);
				_Items[k].NameId = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:NAMEID").c_str(), (int) k), false);
				_Items[k].UserColor = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:USER_COLOR").c_str(), (int) k), false);
				_Items[k].Enchant = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:ENCHANT").c_str(), (int) k), false);
				_Items[k].RMClassType = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:RM_CLASS_TYPE").c_str(), (int) k), false);
				_Items[k].RMFaberStatType = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:RM_FABER_STAT_TYPE").c_str(), (int) k), false);
				_Items[k].InfoVersion = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:INFO_VERSION").c_str(), (int) k), false);
				if ((_Items[k].SlotType == NULL) || (_Items[k].Quality == NULL) || (_Items[k].SheetIDOrSkill == NULL) ||
					(_Items[k].LogicTextID == NULL) || (_Items[k].DescTextID == NULL) ||
					(_Items[k].Weight==NULL) || (_Items[k].InfoVersion==NULL) || (_Items[k].UserColor==NULL) ||
					(_Items[k].Enchant ==NULL) || (_Items[k].RMClassType == NULL) || (_Items[k].RMFaberStatType == NULL) ||
					(_Items[k].NameId==NULL)
				   )
					return false;
			break;
			case Ascensor:
				_Items[k].GuildIcon = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:ICON").c_str(), (int) k), false);
				_Items[k].GuildName = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:NAME").c_str(), (int) k), false);
				if ((_Items[k].GuildIcon == NULL) || (_Items[k].GuildName == NULL))
					return false;
			break;
			case  Missions:
				_Items[k].MissionDetailText = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:DETAIL_TEXT").c_str(), (int) k), false);
				_Items[k].MissionText = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:TEXT").c_str(), (int) k), false);
				_Items[k].MissionIcon = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:ICON").c_str(), (int) k), false);
				_Items[k].MissionPreReqState = NLGUI::CDBManager::getInstance()->getDbProp(toString((dbPath + ":%d:PREREQ_STATE").c_str(), (int) k), false);
				if ((_Items[k].MissionText == NULL) || (_Items[k].MissionIcon == NULL) ||
					(_Items[k].MissionDetailText == NULL) || (_Items[k].MissionPreReqState == NULL))
					return false;
			break;
			default:
				nlassert(0); // You have to provide code to link elements of the server database
			break;
		}
	}
	_Init = true;
	return true;
}

// ****************************************************************************
void CHugeListObs::start()
{
	nlassert((uint) _Category < ListTypeCount); // must call setListType
	// Clear temp structure
	_ItemsPages.clear();
	_DownloadComplete = false;

	// clear the local database
	CInterfaceManager *im = CInterfaceManager::getInstance();
	for (uint k = 0; k < TRADE_PAGE_NUM_ITEMS * TRADE_MAX_NUM_PAGES; ++k)
	{
		CCDBNodeLeaf *leaf;
		switch(_Category)
		{
			case Trading:
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_ITEM_FOR_MONEY ":%d:SHEET", (int) k), false);
				if (leaf) leaf->setValue32(0);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_ITEM_FOR_MONEY ":%d:SLOT_TYPE", (int) k), false);
				if (leaf) leaf->setValue32(0);
			break;
			case ItemForMissions:
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_ITEM_FOR_MISSION ":%d:SHEET", (int) k), false);
				if (leaf) leaf->setValue32(0);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_ITEM_FOR_MISSION ":%d:SLOT_TYPE", (int) k), false);
				if (leaf) leaf->setValue32(0);
			break;
			case Ascensor:
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_ASCENSOR":%d:ICON", (int) k), false);
				if (leaf) leaf->setValue64(0);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_ASCENSOR":%d:NAME", (int) k), false);
				if (leaf) leaf->setValue32(0);
			break;
			case Missions:
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_MISSIONS":%d:ICON", (int) k), false);
				if (leaf) leaf->setValue64(0);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_MISSIONS":%d:TEXT", (int) k), false);
				if (leaf) leaf->setValue32(0);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(toString(DB_LOCAL_BRANCH_FOR_MISSIONS":%d:DETAIL_TEXT", (int) k), false);
				if (leaf) leaf->setValue32(0);
			break;
			default:
				nlassert(0); // You have to provide code for elements in local db to be cleared
			break;
		}
	}

	// Setup session ID
	switch(_Category)
	{
		case Trading:
		case ItemForMissions:
		case Missions:
			_CurrentSessionNb = CBotChatManager::getInstance()->getSessionID();
		break;
		case Ascensor:
		break;
		default:
			nlassert(0); // You have to provide code for the choice of the session id
		break;
	}

	// reset
	_FirstPageNotUpdated= 0;
	_PhraseClientFillNumPhrase= 0;
	_PhraseClientFill= false;
}

// ****************************************************************************
void CHugeListObs::update(ICDBNode * /* node */)
{
	nlassert((uint) _Category < ListTypeCount); // must call setListType
	// if botchat is not active, dont care with update
	if ((_Category == Trading) || (_Category == ItemForMissions) || (_Category == Missions))
	{
		if (!CBotChatManager::getInstance()->getCurrPage())
			return;
	}

	if (!init())
	{
		nlwarning("Can't do init");
		return;
	}
	// check if good session
	if (_Session->getValue16() != _CurrentSessionNb)
	{
		// msg from a previous session, dont care
		return;
	}
	TItemVect *itemPages = &_ItemsPages;
	uint pageID = (uint) _PageID->getValue16();
	if (pageID >= TRADE_MAX_NUM_PAGES) return; // bad page index
	if (pageID >= itemPages->size())
	{
		// expand items list
		itemPages->resize(pageID + 1);
	}
	// if 'has_next' flag is set, then ask server for more pages (if it is not a page update)
	if (_HasNext->getValueBool())
	{
		static const char *msgItemForMoney = "BOTCHAT:NEXT_PAGE_ITEM";
		static const char *msgItemForMission = "TRADE:NEXT_PAGE_MISSION_ITEM";
		static const char *msgAscensor = "GUILD:NEXT_ASCENSOR_PAGE";
		static const char *msgMissions;
		if(_MType==MISSION_DESC::ZCCharge)
			msgMissions= "BOTCHAT:NEXT_PAGE_DUTY";
		else
			msgMissions= "BOTCHAT:NEXT_PAGE_MISSION";

		const char* msgName;
		switch(_Category)
		{
			case Trading: msgName = msgItemForMoney; break;
			case ItemForMissions: msgName = msgItemForMission; break;
			case Ascensor: msgName = msgAscensor; break;
			case Missions: msgName = msgMissions; break;
			default:
				nlassert(0); // You have to provide message for ping-pong behavior
			break;
		}

		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		{
			out.serial(_CurrentSessionNb);
			NetMngr.push(out);
			//nlinfo("impulseCallBack : %s %d sent", msgName, _CurrentSessionNb);
		}
		else
		{
			nlwarning("<CHandlerAcceptExchange::execute> unknown message name '%s'", msgName);
		}
	}
	else
	{
		_DownloadComplete = true;
	}

	CItemPage &page = (*itemPages)[pageID];
	uint k;
	// copy items page into temporary stuff
	for (k = 0; k < TRADE_PAGE_NUM_ITEMS; ++k)
	{
		switch(_Category)
		{
			case Trading:
			{
				page.Items[k].SlotType = (TRADE_SLOT_TYPE::TTradeSlotType) _Items[k].SlotType->getValue32();
				page.Items[k].Quality = (uint16) _Items[k].Quality->getValue16();
				page.Items[k].SheetIDOrSkill = (uint32) _Items[k].SheetIDOrSkill->getValue32();
				page.Items[k].Price = (uint32) _Items[k].Price->getValue32();
				page.Items[k].Weight= (uint16) _Items[k].Weight->getValue16();
				page.Items[k].NameId= (uint32) _Items[k].NameId->getValue32();
				page.Items[k].UserColor = _Items[k].UserColor->getValue32();
				page.Items[k].Enchant = _Items[k].Enchant->getValue32();
				page.Items[k].RMClassType = _Items[k].RMClassType->getValue32();
				page.Items[k].RMFaberStatType = _Items[k].RMFaberStatType->getValue32();
				page.Items[k].PrerequisitValid = _Items[k].PrerequisitValid->getValue32();
				page.Items[k].InfoVersion= (uint16) _Items[k].InfoVersion->getValue16();
				page.Items[k].Quantity= (uint16) _Items[k].Quantity->getValue16();
				page.Items[k].PriceRetire= (uint32) _Items[k].PriceRetire->getValue32();
				page.Items[k].ResaleTimeLeft= (uint32) _Items[k].ResaleTimeLeft->getValue32();
				page.Items[k].VendorNameId= (uint32) _Items[k].VendorNameId->getValue32();
				page.Items[k].FactionType= (uint32) _Items[k].FactionType->getValue32();
				page.Items[k].FactionPointPrice= (uint32) _Items[k].FactionPointPrice->getValue32();
				page.Items[k].SellerType= (uint32) _Items[k].SellerType->getValue32();
				// is the sheet is a phrase sheet, discard it if already known
				/*
				CEntitySheet *sheet = SheetMngr.get(CSheetId(page.Items[k].SheetIDOrSkill));
				if (sheet && sheet->Type == CEntitySheet::SPHRASE)
				{
					CSPhraseSheet *ps = (CSPhraseSheet *) sheet;
					CSPhraseCom pc;
					pc.Bricks.swap(ps->Bricks); // substitute brick in the object (for the time of comparison)
					if (CSPhraseManager::getInstance()->isPhraseKnown(pc))
					{
						page.Items[k].SheetIDOrSkill = 0; // already known, do not show in list
					}
					pc.Bricks.swap(ps->Bricks); // restore list of sheet
				}
				*/
			}
			break;
			case ItemForMissions:
				page.Items[k].SlotType = (TRADE_SLOT_TYPE::TTradeSlotType) _Items[k].SlotType->getValue32();
				page.Items[k].Quality = (uint16) _Items[k].Quality->getValue16();
				page.Items[k].SheetIDOrSkill = (uint32) _Items[k].SheetIDOrSkill->getValue32();
				page.Items[k].LogicTextID = (uint32) _Items[k].LogicTextID->getValue32();
				page.Items[k].DescTextID = (uint32) _Items[k].DescTextID->getValue32();
				page.Items[k].Weight= (uint16) _Items[k].Weight->getValue16();
				page.Items[k].NameId= (uint32) _Items[k].NameId->getValue32();
				page.Items[k].UserColor = _Items[k].UserColor->getValue32();
				page.Items[k].Enchant = _Items[k].Enchant->getValue32();
				page.Items[k].RMClassType = _Items[k].RMClassType->getValue32();
				page.Items[k].RMFaberStatType = _Items[k].RMFaberStatType->getValue32();
				page.Items[k].InfoVersion= (uint16) _Items[k].InfoVersion->getValue16();
			break;
			case Ascensor:
				page.Items[k].GuildIcon	= (uint64) _Items[k].GuildIcon->getValue64();
				page.Items[k].GuildName = (uint32) _Items[k].GuildName->getValue32();
			break;
			case Missions:
				page.Items[k].MissionText = (uint32) _Items[k].MissionText->getValue64();
				page.Items[k].MissionDetailText = (uint32) _Items[k].MissionDetailText->getValue64();
				page.Items[k].MissionIcon = (uint32) _Items[k].MissionIcon->getValue32();
				page.Items[k].MissionPreReqState = (uint32) _Items[k].MissionPreReqState->getValue32();
				break;
			default:
				nlassert(0); // You have to provide code for element that you want to be copied
			break;
		}
	}
	page.PageLoaded = true;
	page.PageVisible = false; // force update, even if shown before
	uint numLoadedPages;
	// In the display we want a contiguous set of items. Not For Trading, because of Resale
	if(_Category==Trading)
	{
		numLoadedPages= (uint)itemPages->size();
	}
	else
	{
		for(numLoadedPages = 0; numLoadedPages < itemPages->size(); ++numLoadedPages)
		{
			if ((*itemPages)[numLoadedPages].PageLoaded == false && numLoadedPages != pageID) break;
		}
	}
	//
	for(k = 0; k < numLoadedPages; ++k)
	{
		// still must test if page really loaded
		if ( (*itemPages)[k].PageLoaded && (*itemPages)[k].PageVisible == false)
		{
			updateUIItemPage(k);
			(*itemPages)[k].PageVisible = true;
		}
	}

	// **** Special RoleMaster Trading
	_PhraseClientFillNumPhrase= 0;
	_PhraseClientFillFlags= 0;
	_PhraseClientFillRace= 0;
	_PhraseClientFill= false;
	// clientFill must at least be possible
	if(_Category==Trading && _PhrasePriceUpdateAndMaybeClientFill && _RoleMasterFlagDB && _RoleMasterRaceDB)
	{
		_PhraseClientFillFlags= _RoleMasterFlagDB->getValue32();
		_PhraseClientFillRace= _RoleMasterRaceDB->getValue32();
		_PhraseClientFill= _PhraseClientFillFlags!=0;
		if(_PhraseClientFill)
		{
			clientGenerateAllPhrases();
			recomputeAllPhrasePrices();
		}
	}

}


// ****************************************************************************
void CHugeListObs::updateUIItemPage(uint index)
{
	nlassert((uint) _Category < ListTypeCount); // must call setListType
	CInterfaceManager *im = CInterfaceManager::getInstance();
	//
	std::string					 dbPath; // db path for the current item list
	TItemVect					 *items = &_ItemsPages;
	CDBCtrlSheet::TSheetCategory *itemCategory = &_ItemCategory;
	CDBGroupListSheetText		 *listSheet;
	CCDBNodeLeaf				 *itemListCategoryLeaf = NULL;
	switch (_Category)
	{
		case Trading:
		{
			if (BotChatPageAll->Trade->getBuyOnly())
			{
				dbPath = DB_LOCAL_BRANCH_FOR_ITEM_FOR_MONEY ":";
				listSheet = dynamic_cast<CDBGroupListSheetText *>(CWidgetManager::getInstance()->getElementFromId(UI_LIST_OF_ITEMS_FOR_MONEY_BUY_ONLY));
			}
			else
			{
				dbPath = DB_LOCAL_BRANCH_FOR_ITEM_FOR_MONEY ":";
				listSheet = dynamic_cast<CDBGroupListSheetText *>(CWidgetManager::getInstance()->getElementFromId(UI_LIST_OF_ITEMS_FOR_MONEY));
			}
			if (!listSheet) return;
			itemListCategoryLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TYPE_OF_ITEMS_TO_BUY", false);
		}
		break;
		case ItemForMissions:
		{
			dbPath = DB_LOCAL_BRANCH_FOR_ITEM_FOR_MISSION ":";
			listSheet = dynamic_cast<CDBGroupListSheetText *>(CWidgetManager::getInstance()->getElementFromId(UI_LIST_OF_ITEMS_FOR_MISSION));
			if (!listSheet) return;
			itemListCategoryLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:TYPE_OF_ITEMS_FOR_MISSION", false);
			if (!itemListCategoryLeaf) return;
			// display the group for mission items
			CInterfaceElement *elm = CWidgetManager::getInstance()->getElementFromId(UI_GROUP_OF_ITEMS_FOR_MISSION);
			if (elm) elm->setActive(true);
		}
		break;
		case Ascensor:
		{
			dbPath = DB_LOCAL_BRANCH_FOR_ASCENSOR ":";
			listSheet = dynamic_cast<CDBGroupListSheetText *>(CWidgetManager::getInstance()->getElementFromId(UI_LIST_OF_ITEMS_FOR_ASCENSOR));
			if (!listSheet) return;
			// display the group for mission items
			CInterfaceElement *elm = CWidgetManager::getInstance()->getElementFromId(UI_GROUP_OF_ITEMS_FOR_ASCENSOR);
			if (elm) elm->setActive(true);
		}
		break;
		case Missions:
		{
			dbPath = DB_LOCAL_BRANCH_FOR_MISSIONS ":";
			listSheet = dynamic_cast<CDBGroupListSheetText *>(CWidgetManager::getInstance()->getElementFromId(UI_LIST_OF_MISSIONS));
			if (!listSheet) return;
			// display the group for mission items
			CInterfaceElement *elm = CWidgetManager::getInstance()->getElementFromId(UI_GROUP_OF_MISSIONS);
			if (elm) elm->setActive(true);
		}
		break;
		default:
			nlassert(0); // Provide code for initializing
		break;
	}
	// Copy item into the big local db
	for(uint k = 0; k < TRADE_PAGE_NUM_ITEMS; ++k)
	{
		CCDBNodeLeaf *leaf;
		const CItem &currItem = (*items)[index].Items[k];
		// get branch for the item
		switch(_Category)
		{
			case Trading:
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":QUALITY", false);
				if (leaf) leaf->setValue32(currItem.Quality);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":SLOT_TYPE", false);
				if (leaf) leaf->setValue32(currItem.SlotType);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":SHEET", false);
				if (leaf) leaf->setValue32(currItem.SheetIDOrSkill);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":PRICE", false);
				if (leaf) leaf->setValue32(currItem.Price);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":WEIGHT", false);
				if (leaf) leaf->setValue32(currItem.Weight);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":NAMEID", false);
				if (leaf) leaf->setValue32(currItem.NameId);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":USER_COLOR", false);
				if (leaf) leaf->setValue32(currItem.UserColor);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":ENCHANT", false);
				if (leaf) leaf->setValue32(currItem.Enchant);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":RM_CLASS_TYPE", false);
				if (leaf) leaf->setValue32(currItem.RMClassType);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":RM_FABER_STAT_TYPE", false);
				if (leaf) leaf->setValue32(currItem.RMFaberStatType);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":PREREQUISIT_VALID", false);
				if (leaf) leaf->setValue32(currItem.PrerequisitValid);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":INFO_VERSION", false);
				if (leaf) leaf->setValue32(currItem.InfoVersion);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":QUANTITY", false);
				if (leaf) leaf->setValue32(currItem.Quantity);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":PRICE_RETIRE", false);
				if (leaf) leaf->setValue32(currItem.PriceRetire);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":RESALE_TIME_LEFT", false);
				if (leaf) leaf->setValue32(currItem.ResaleTimeLeft);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":SELLER_TYPE", false);
				if (leaf) leaf->setValue32(currItem.SellerType);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":VENDOR_NAMEID", false);
				if (leaf) leaf->setValue32(currItem.VendorNameId);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":FACTION_TYPE", false);
				if (leaf) leaf->setValue32(currItem.FactionType);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":PRICE", false);
				if (leaf) leaf->setValue32(currItem.FactionPointPrice);

			break;
			case ItemForMissions:
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":QUALITY", false);
				if (leaf) leaf->setValue32(currItem.Quality);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":SLOT_TYPE", false);
				if (leaf) leaf->setValue32(currItem.SlotType);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":SHEET", false);
				if (leaf) leaf->setValue32(currItem.SheetIDOrSkill);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":LOGIC_TEXT_ID", false);
				if (leaf) leaf->setValue32(currItem.LogicTextID);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":DESC_TEXT_ID", false);
				if (leaf) leaf->setValue32(currItem.DescTextID);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":WEIGHT", false);
				if (leaf) leaf->setValue32(currItem.Weight);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":NAMEID", false);
				if (leaf) leaf->setValue32(currItem.NameId);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":USER_COLOR", false);
				if (leaf) leaf->setValue32(currItem.UserColor);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":ENCHANT", false);
				if (leaf) leaf->setValue32(currItem.Enchant);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":RM_CLASS", false);
				if (leaf) leaf->setValue32(currItem.RMClassType);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":RM_FABER_STAT_TYPE", false);
				if (leaf) leaf->setValue32(currItem.RMFaberStatType);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":INFO_VERSION", false);
				if (leaf) leaf->setValue32(currItem.InfoVersion);
			break;
			case Ascensor:
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":ICON", false);
				if (leaf) leaf->setValue64(currItem.GuildIcon);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":NAME", false);
				if (leaf) leaf->setValue32(currItem.GuildName);
			break;
			case Missions:
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":TEXT", false);
				if (leaf) leaf->setValue64(currItem.MissionText);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":DETAIL_TEXT", false);
				if (leaf) leaf->setValue64(currItem.MissionDetailText);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":ICON", false);
				if (leaf) leaf->setValue32(currItem.MissionIcon);
				leaf = NLGUI::CDBManager::getInstance()->getDbProp(dbPath + toString(k + index * TRADE_PAGE_NUM_ITEMS) + ":PREREQ_STATE", false);
				if (leaf) leaf->setValue32(currItem.MissionPreReqState);
				break;
			default:
				nlassert(0); // Implement the temp -> local stuff
			break;
		}

		CDBCtrlSheet *sheet = listSheet->getSheet(k + index * TRADE_PAGE_NUM_ITEMS);
		if (sheet)
		{
			CDBCtrlSheet::TSheetCategory newItemCategory = sheet->getSheetCategory();
			if (k == 0 && index == 0)
			{
				*itemCategory = newItemCategory;
			}
			else
			{
				if (*itemCategory != newItemCategory)
				{
					*itemCategory = CDBCtrlSheet::DontKnow;
				}
			}
			if (_Category == Trading)
			{
				// If price of current selected item has changed, repercute it
				if (sheet == CDBCtrlSheet::getCurrSelection())
				{
					string ls = CWidgetManager::getInstance()->getParser()->getDefine("item_price_1");
					string ms = CWidgetManager::getInstance()->getParser()->getDefine("item_price_2");
					string bs = CWidgetManager::getInstance()->getParser()->getDefine("item_price_3");
					string vbs = CWidgetManager::getInstance()->getParser()->getDefine("item_price_4");
					setPriceInDB(currItem.Price, ls, ms, bs, vbs);
				}
			}
		}
	}
	// set db entry to give the type of the items
	if (itemListCategoryLeaf) itemListCategoryLeaf->setValue64((sint64) *itemCategory);

	// max page set
	_FirstPageNotUpdated= max(_FirstPageNotUpdated, uint32(index+1));

	// For Phrase BotChat, update the DB prices
	if(_Category==Trading && _PhrasePriceUpdateAndMaybeClientFill)
	{
		CSPhraseManager		*pPM= CSPhraseManager::getInstance();
		pPM->updateBotChatPhrasePrice(index * TRADE_PAGE_NUM_ITEMS, (index+1) * TRADE_PAGE_NUM_ITEMS);
	}
}

// ***************************************************************************
void CHugeListObs::recomputeAllPhrasePrices()
{
	// need only to update page setuped
	if(_FirstPageNotUpdated>0 && _Category==Trading && _PhrasePriceUpdateAndMaybeClientFill)
	{
		CSPhraseManager		*pPM= CSPhraseManager::getInstance();
		// update all Specific or Client generated phrase Price
		pPM->updateBotChatPhrasePrice(0, max(_PhraseClientFillNumPhrase, _FirstPageNotUpdated * TRADE_PAGE_NUM_ITEMS));
	}
}

// ***************************************************************************
/// Seet a price in the db
void CHugeListObs::setPriceInDB(uint32 value, const std::string &ls, const std::string &ms, const std::string &bs, const std::string &vbs)
{
	nlassert((uint) _Category < ListTypeCount); // must call setListType
	CSeeds money;
	money.setTotal(value);
	CInterfaceManager *im = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp(ls)->setValue64(money.getLS());
	NLGUI::CDBManager::getInstance()->getDbProp(ms)->setValue64(money.getMS());
	NLGUI::CDBManager::getInstance()->getDbProp(bs)->setValue64(money.getBS());
	NLGUI::CDBManager::getInstance()->getDbProp(vbs)->setValue64(money.getVBS());
}


// ***************************************************************************
void	CHugeListObs::clientGenerateAllPhrases()
{
	// if not client fill, abort
	if(!_PhraseClientFill)
		return;

	// fill the DB with all currently possible phrase, filtered by rolematster type/race
	CSPhraseManager		*pPM= CSPhraseManager::getInstance();
	_PhraseClientFillNumPhrase= pPM->fillRoleMasterGenericPhrase(_PhraseClientFillFlags, _PhraseClientFillRace);
}



















