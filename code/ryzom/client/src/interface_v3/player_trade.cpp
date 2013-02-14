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
//
#include "player_trade.h"
#include "interface_manager.h"
#include "dbctrl_sheet.h"
#include "dbgroup_list_sheet.h"
#include "nel/gui/action_handler.h"
#include "inventory_manager.h"
//
#include "game_share/inventories.h"
//
#include "../sheet_manager.h"
#include "../client_sheets/item_sheet.h"

using namespace std;
using namespace NLMISC;

/////////////
// GLOBALS //
/////////////


CPlayerTrade PlayerTrade;

// Context help
extern void contextHelp (const std::string &help);

//////////////////////
// MEMBER FUNCTIONS //
//////////////////////


void CPlayerTrade::putItemInExchange(CDBCtrlSheet *src, CDBCtrlSheet *dest, uint quantityMove)
{
	if (dest && src)
	{
		restoreItem(dest);
		src->copyAspect(dest);

		// Partial copy?
		sint	quantitySrc= src->getQuantity();
		if((sint)quantityMove<quantitySrc)
		{
			dest->setQuantity(quantityMove);
			src->setQuantity(quantitySrc-quantityMove);
		}
		// no more item in inventaire
		else
		{
			src->setSheetId(0);
		}
	}
}

void CPlayerTrade::restoreItem(CDBCtrlSheet *exchangeSlot)
{
	if (!exchangeSlot) return;
	if (exchangeSlot->getSheetId() == 0) return; // not used
	CInventoryManager &im = getInventory();

	// *** If not an AMMO, try to 'auto-stack', as server does
	bool	canAutoStack= false;
	uint32	autoStackSheetId = 0;
	sint32	autoStackQuality = 0;
	sint32	autoStackMaxQuantity = 0;
	const CItemSheet	*itemSheet= exchangeSlot->asItemSheet();
	// NB: cannot auto stack ammo because their full props (dammage etc...) are not sent to client
	if(itemSheet)
	{
		canAutoStack= itemSheet->Stackable>1 && itemSheet->Family!=ITEMFAMILY::AMMO;
		autoStackSheetId= itemSheet->Id.asInt();
		autoStackQuality= exchangeSlot->getQuality();
		autoStackMaxQuantity= itemSheet->Stackable;
	}

	// *** find an empty place in the inventory, or if autoStack, distribute over existing slot
	sint32	emptySlot= -1;
	sint32	quantityLeft= exchangeSlot->getQuantity();
	for(uint k = 0; k < MAX_BAGINV_ENTRIES; ++k)
	{
		if (im.getBagItem(k).getSheetID() == 0)
		{
			emptySlot= k;
			// if cannot auto stack, don't need to go further
			if(!canAutoStack)
				break;
		}
		// can auto stack on this slot?
		else if(canAutoStack)
		{
			sint32	dstQuantity= im.getBagItem(k).getQuantity();

			// if same sheet/quality, and if can put some stack in this slot
			if( im.getBagItem(k).getSheetID()==autoStackSheetId &&
				im.getBagItem(k).getQuality()==autoStackQuality &&
				dstQuantity<autoStackMaxQuantity )
			{
				// if can put all left stack
				if(autoStackMaxQuantity-dstQuantity>=quantityLeft)
				{
					im.getBagItem(k).setQuantity((uint16)(dstQuantity+quantityLeft));
					quantityLeft= 0;
					break;
				}
				else
				{
					im.getBagItem(k).setQuantity((uint16)autoStackMaxQuantity);
					quantityLeft-= autoStackMaxQuantity-dstQuantity;
				}
			}
		}
	}

	// if some quantity not distributed, Move all the remaining exchange slot to the empty slot detected (if any)
	if(quantityLeft && emptySlot>=0)
	{
		im.getBagItem(emptySlot).setSheetID(exchangeSlot->getSheetId());
		im.getBagItem(emptySlot).setQuality((uint16) exchangeSlot->getQuality());
		im.getBagItem(emptySlot).setQuantity((uint16) quantityLeft);
		im.getBagItem(emptySlot).setUserColor((uint8) exchangeSlot->getItemColor());
		im.getBagItem(emptySlot).setWeight((uint32) exchangeSlot->getItemWeight());
		im.getBagItem(emptySlot).setNameId(exchangeSlot->getItemNameId());
		im.getBagItem(emptySlot).setInfoVersion(exchangeSlot->getItemInfoVersion());
		im.getBagItem(emptySlot).setResaleFlag(exchangeSlot->getItemResaleFlag());
	}


	// clear item from exchange
	exchangeSlot->setSheetId(0);
	exchangeSlot->setQuality(0);
	exchangeSlot->setQuantity(0);
}

CDBCtrlSheet *CPlayerTrade::getExchangeItem(uint index)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	return dynamic_cast<CDBCtrlSheet *>(CWidgetManager::getInstance()->getElementFromId(NLMISC::toString("ui:interface:player_trade:header_opened:give:given_items:slot%d", (int) index)));
}


void CPlayerTrade::restoreAllItems()
{
	for(uint k = 0; k < NumTradeSlot; ++k)
	{
		restoreItem(getExchangeItem(k));
	}
}


/////////////////////
// ACTION HANDLERS //
/////////////////////

class CPlayerTradeStartHandler : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:EXCHANGE:BEGUN")->getValue8() != 0)
		{
			//PlayerTrade.reset();

			// Context help
			contextHelp ("inventory");
		}
	}
};
REGISTER_ACTION_HANDLER(CPlayerTradeStartHandler, "player_trade_start");


class CPlayerTradeLeftClickOnSlotHandler : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet *sheet = dynamic_cast<CDBCtrlSheet *>(pCaller);
		if (!sheet) return;
		if (sheet->getSheetId() == 0)
		{
			CInventoryManager *pInv = CInventoryManager::getInstance();
			// is there's no item that is not worn, can't choose any item)
			bool isThereObjectNotWorn = false;
			for(uint k = 0; k < MAX_BAGINV_ENTRIES; ++k)
			{
				uint32 sheetid = pInv->getBagItem(k).getSheetID();
				if (sheetid != 0)
				{
					if (!pInv->isBagItemWeared(k))
					{
						CEntitySheet *pES = SheetMngr.get(CSheetId(sheetid));
						if (pES && pES->type()== CEntitySheet::ITEM)
						{
							CItemSheet *pIS = (CItemSheet*)pES;
							if (pIS->canExchangeOrGive(PlayerTrade.BotChatGiftContext))
							{
								isThereObjectNotWorn = true;
								break;
							}
						}
					}
				}
			}

			if (!isThereObjectNotWorn)
			{
				// every object are worn, so there's no use to display an empty list -> no-op
				return;
			}
		}
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CWidgetManager::getInstance()->pushModalWindow(pCaller, "ui:interface:exchange_choose_in_bag");
	}
};
REGISTER_ACTION_HANDLER(CPlayerTradeLeftClickOnSlotHandler, "exchange_left_click_on_slot");
