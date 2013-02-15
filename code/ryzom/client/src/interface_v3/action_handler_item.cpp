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

#include "action_handler_item.h"
#include "nel/gui/action_handler.h"
#include "interface_manager.h"
#include "../sheet_manager.h"
#include "dbctrl_sheet.h"
#include "dbgroup_list_sheet.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/interface_expr.h"
#include "player_trade.h"
#include "../user_entity.h"
#include "../net_manager.h"
#include "nel/gui/group_menu.h"
#include "../global.h"
#include "nel/gui/group_html.h"

//
#include "game_share/inventories.h"
#include "game_share/bot_chat_types.h"

#include "macrocmd_manager.h"
#include "inventory_manager.h"

#include "nel/gui/ctrl_base_button.h"
#include "../connection.h"
#include "nel/gui/view_bitmap.h"

extern CSheetManager SheetMngr;
extern NLMISC::CLog	g_log;

using namespace std;
using namespace NLMISC;

CInterfaceItemEdition *CInterfaceItemEdition::_Instance = NULL;

// ********************************************************************************************
CInterfaceItemEdition *CInterfaceItemEdition::getInstance()
{
	if (!_Instance) _Instance = new CInterfaceItemEdition;
	return _Instance;
}

// ********************************************************************************************
void CInterfaceItemEdition::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ********************************************************************************************
void CInterfaceItemEdition::setCurrWindow(CDBCtrlSheet* ctrlSheet, const std::string &windowName, const bool &isInEditionMode)
{
	_CurrWindow.end();

	if (ctrlSheet && !windowName.empty())
	{
		_CurrWindow.ItemSheet = ctrlSheet->getSheetId();
		_CurrWindow.ItemSlotId = getInventory().getItemSlotId(ctrlSheet);
		_CurrWindow._CurrItemSheet = ctrlSheet;
		_CurrWindow.WindowName = windowName;
		_CurrWindow.IsInEditionMode = isInEditionMode;
		_CurrWindow.begin();
	}
}

// ********************************************************************************************
void CInterfaceItemEdition::update()
{
	_CurrWindow.update();
}
// ********************************************************************************************
void CInterfaceItemEdition::validate()
{
	_CurrWindow.validate();
	setCurrWindow(NULL);
}

// ********************************************************************************************
void CInterfaceItemEdition::CItemEditionWindow::infoReceived()
{
	if(_CurrItemSheet && !WindowName.empty())
	{
		const CItemSheet *pIS = _CurrItemSheet->asItemSheet();
		if ((pIS != NULL) && ITEMFAMILY::isTextCustomizable(pIS->Family) )
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			// get the dialog stack
			CInterfaceGroup* group = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(WindowName) );

			CInterfaceGroup* editShort = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_short")) );
			CGroupEditBox* editBoxShort = dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_box_short")) );
			CInterfaceGroup* editLarge = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_large")) );
			CGroupEditBox* editBoxLarge = dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_box_large")) );
			CViewText* display = dynamic_cast<CViewText*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_display")) );
			if (group && editShort && editBoxShort && editLarge && editBoxLarge && display)
			{
				CClientItemInfo const& itemInfo = getInventory().getItemInfo( ItemSlotId );
				if (IsInEditionMode)
				{
					if ( pIS->Family == ITEMFAMILY::SCROLL)
					{
						editBoxLarge->setInputString(itemInfo.CustomText);
						editLarge->setActive(true);
						editBoxLarge->setActive(true);

						// Set the Keyboard focus to the editbox (after the enableModalWindow())
						CWidgetManager::getInstance()->setCaptureKeyboard(editBoxLarge);
						// Select all the text for easier selection
						editBoxLarge->setSelectionAll();
					}
					else
					{
						ucstring customText;
						if (!itemInfo.CustomText.empty())
						{
							customText = itemInfo.CustomText;
							strFindReplace(customText, "%mfc", ucstring());
						}

						editBoxShort->setInputString(customText);
						editShort->setActive(true);
						editBoxShort->setActive(true);

						// Set the Keyboard focus to the editbox (after the enableModalWindow())
						CWidgetManager::getInstance()->setCaptureKeyboard(editBoxShort);
						// Select all the text for easier selection
						editBoxShort->setSelectionAll();
					}
				}
				else
				{
					if (itemInfo.CustomText.empty())
						display->setTextFormatTaged(ucstring(STRING_MANAGER::CStringManagerClient::getItemLocalizedDescription(pIS->Id)));
					else
					{
						ucstring text = itemInfo.CustomText;
						string::size_type delimiter = text.find(' ');
						if(text.size() > 3 && text[0]=='@' && text[1]=='W' && text[2]=='E' && text[3]=='B')
						{
							CGroupHTML *pGH = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:web_transactions:content:html"));
							if (pGH)
								pGH->browse(ucstring(text.substr(4, delimiter-4)).toString().c_str());
							if (delimiter == string::npos)
								group->setActive(false);
							else
								text = text.substr(delimiter, text.size()-delimiter);
						}
						
						display->setTextFormatTaged(text);		
					}
				}
			}
		}
	}
}



// ********************************************************************************************
void CInterfaceItemEdition::CItemEditionWindow::update()
{
	if(_CurrItemSheet && ((sint32)ItemSheet != _CurrItemSheet->getSheetId()))
	{
		end();
	}
}
// ********************************************************************************************
void CInterfaceItemEdition::CItemEditionWindow::begin()
{
	if(_CurrItemSheet && !WindowName.empty())
	{
	
		const CItemSheet *pIS = _CurrItemSheet->asItemSheet();
		if ((pIS != NULL) && ITEMFAMILY::isTextCustomizable(pIS->Family) )
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			// get the dialog stack
			CGroupContainer* group = dynamic_cast<CGroupContainer*>( CWidgetManager::getInstance()->getElementFromId(WindowName) );

			CInterfaceGroup* editShort = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_short")) );
			CGroupEditBox* editBoxShort = dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_box_short")) );
			CInterfaceGroup* editLarge = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_large")) );
			CGroupEditBox* editBoxLarge = dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_box_large")) );
			CViewText* display = dynamic_cast<CViewText*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_display")) );
			CInterfaceGroup* editButtons = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_buttons")) );
			CCtrlBaseButton* closeButton = dynamic_cast<CCtrlBaseButton*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_close_button")) );
			CViewBitmap* background = dynamic_cast<CViewBitmap*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_background")) );
			if (group && editShort && editBoxShort && editLarge && editBoxLarge && display && editButtons && closeButton && background)
			{

				const	CClientItemInfo	&itemInfo = getInventory().getItemInfo( ItemSlotId );
				if ( IsInEditionMode )
				{
					// Enable the windows
					editLarge->setActive(false);
					editBoxLarge->setActive(false);
					editShort->setActive(false);
					editBoxShort->setActive(false);
					display->setActive(false);
					editButtons->setActive(true);
					closeButton->setActive(false);
					group->setActive(true);

					editBoxShort->setInputString(ucstring());
					editBoxLarge->setInputString(ucstring());
					display->setTextFormatTaged(ucstring());


					// Finish the display or add the waiter
					if (getInventory().isItemInfoUpToDate(ItemSlotId))
					{
						// If we already have item info
						if ( pIS->Family == ITEMFAMILY::SCROLL)
						{
							editBoxLarge->setInputString(itemInfo.CustomText);
							editLarge->setActive(true);
							editBoxLarge->setActive(true);

							// Set the Keyboard focus to the editbox
							CWidgetManager::getInstance()->setCaptureKeyboard(editBoxLarge);
							// Select all the text for easier selection
							editBoxLarge->setSelectionAll();
						}
						else
						{

							ucstring customText;
							if (!itemInfo.CustomText.empty())
							{
								customText = itemInfo.CustomText;
								strFindReplace(customText, "%mfc", ucstring());
							}

							editBoxShort->setInputString(customText);
							editShort->setActive(true);
							editBoxShort->setActive(true);

							// Set the Keyboard focus to the editbox
							CWidgetManager::getInstance()->setCaptureKeyboard(editBoxShort);
							// Select all the text for easier selection
							editBoxShort->setSelectionAll();
						}
					}
					else
					{
						// Add the waiter
						getInventory().addItemInfoWaiter(this);
					}
				}
				else
				{
					// Enable the windows
					editShort->setActive(false);
					editBoxShort->setActive(false);
					editLarge->setActive(false);
					editBoxLarge->setActive(false);
					display->setActive(true);
					editButtons->setActive(false);
					closeButton->setActive(true);
					group->setActive(true);

					editBoxShort->setInputString(ucstring());
					editBoxLarge->setInputString(ucstring());
					display->setTextFormatTaged(ucstring());

					// Finish the display or add the waiter
					if (getInventory().isItemInfoUpToDate(ItemSlotId))
					{
						// If we already have item info
						if (itemInfo.CustomText.empty())
							display->setTextFormatTaged(ucstring(STRING_MANAGER::CStringManagerClient::getItemLocalizedDescription(pIS->Id)));
						else
						{
							ucstring text = itemInfo.CustomText;
							string::size_type delimiter = text.find(' ');
							if(text.size() > 3 && text[0]=='@' && text[1]=='W' && text[2]=='E' && text[3]=='B')
							{
								CGroupHTML *pGH = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:web_transactions:content:html"));
								if (pGH)
									pGH->browse(ucstring(text.substr(4, delimiter-4)).toString().c_str());
								if (delimiter == string::npos)
									group->setActive(false);
								else
									text = text.substr(delimiter, text.size()-delimiter);
							}
							display->setTextFormatTaged(text);
						}
					}
					else
					{
						// Add the waiter
						getInventory().addItemInfoWaiter(this);
					}
				}
			}
		}
	}
}
// ********************************************************************************************
void CInterfaceItemEdition::CItemEditionWindow::end()
{
	
	CDBCtrlSheet *pCSItem = _CurrItemSheet;
	std::string windowName = WindowName;
	if(pCSItem && !windowName.empty())
	{
		// remove infos waiter (if not already canceled)
		getInventory().removeItemInfoWaiter(this);
		_CurrItemSheet = NULL;
		WindowName = "";

		// hide the dialog
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		// get the dialog stack
		CInterfaceGroup* group = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(windowName) );

		CInterfaceGroup* editShort = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_short")) );
		CGroupEditBox* editBoxShort = dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_box_short")) );
		CInterfaceGroup* editLarge = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_large")) );
		CGroupEditBox* editBoxLarge = dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_box_large")) );
		CViewText* display = dynamic_cast<CViewText*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_display")) );
		CInterfaceGroup* editButtons = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_buttons")) );
		CCtrlBaseButton* closeButton = dynamic_cast<CCtrlBaseButton*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_close_button")) );
		CViewBitmap* background = dynamic_cast<CViewBitmap*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_background")) );
		if (group && editShort && editBoxShort && editLarge && editBoxLarge && display && editButtons && closeButton && background)
		{
			// disable the window
			editBoxShort->setActive(false);
			editShort->setActive(false);
			editBoxLarge->setActive(false);
			editLarge->setActive(false);				
			display->setActive(false);
			editButtons->setActive(false);
			closeButton->setActive(false);

			group->setActive(false);
		}


	}


}
// *******************************************************************************************
void CInterfaceItemEdition::CItemEditionWindow::validate()
{
	CDBCtrlSheet *pCSItem = _CurrItemSheet;
	std::string windowName = WindowName;
	if(pCSItem && !windowName.empty())
	{

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		// get the dialog stack
		CInterfaceGroup* group = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(windowName) );

		CInterfaceGroup* editShort = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_short")) );
		CGroupEditBox* editBoxShort = dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_box_short")) );
		CInterfaceGroup* editLarge = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_large")) );
		CGroupEditBox* editBoxLarge = dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_box_large")) );
		CViewText* display = dynamic_cast<CViewText*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_display")) );
		CInterfaceGroup* editButtons = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_edit_buttons")) );
		CCtrlBaseButton* closeButton = dynamic_cast<CCtrlBaseButton*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_close_button")) );
		CViewBitmap* background = dynamic_cast<CViewBitmap*>( CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("edit_custom_background")) );
		if (group && editShort && editBoxShort && editLarge && editBoxLarge && display && editButtons && closeButton && background)
		{
			bool textValid = editShort->getActive();
			ucstring text = editBoxShort->getInputString();
			if (!textValid)
			{
				textValid = editLarge->getActive();
				text = editBoxLarge->getInputString();
			}
			 
			if (textValid)
			{
				CBitMemStream out;
				const string msgName = "EVENT:SET_ITEM_CUSTOM_TEXT";
				if (!GenericMsgHeaderMngr.pushNameToStream(msgName, out))
				{
					nlwarning ("don't know message name %s", msgName.c_str());
				}
				else
				{
					uint32 uiInventory = (uint32)pCSItem->getInventoryIndex();
					out.serial(uiInventory);
					uint32 uiSlot = (uint32)pCSItem->getIndexInDB();
					out.serial(uiSlot);
					out.serial(text);
					NetMngr.push(out);
					//nlinfo("impulseCallBack : %s %s %d \"%s\" sent", msgName.c_str(), INVENTORIES::toString((INVENTORIES::TInventory)pCSItem->getInventoryIndex()).c_str(), pCSItem->getIndexInDB(), text.toUtf8().c_str());
				}
			}
		}
	}
}
// ***********************************************************************************************************/
enum	TStackMode {StackModeSwap= 0, StackModeExchange};


// Globals.
static	CDBCtrlSheet	*CurrentStackSrc= NULL;
static	CDBCtrlSheet	*CurrentStackDst= NULL;
static	TStackMode		CurrentStackMode;


static void putStackableInventoryItemToExchange(CDBCtrlSheet *src, CDBCtrlSheet *dest, uint quantity);
static void putStackableExchangedItemToInventory(CDBCtrlSheet *src, CDBCtrlSheet *dest, uint quantity);
static void validateStackItem(CDBCtrlSheet *src, CDBCtrlSheet *dest, sint32 quantity, TStackMode stackMode);


//=====================================================================================================================
/** Send a swap item msg to the server
  */
static void sendSwapItemMsg(const CDBCtrlSheet *pCSSrc, const CDBCtrlSheet *pCSDst, sint32 quantitySrc)
{
	CBitMemStream out;
	const string sMsg = "ITEM:SWAP";
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		// Swap all the Src (quantity= quantitySrc) to dest
		uint16	srcInvId=	(uint16)pCSSrc->getInventoryIndex();
		uint16	srcSlotId=	(uint16)pCSSrc->getIndexInDB();
		uint16	dstInvId=	(uint16)pCSDst->getInventoryIndex();
		uint16	dstSlotId=	(uint16)pCSDst->getIndexInDB();
		uint16	quantity=	(uint16)quantitySrc;

		out.serial(srcInvId);
		out.serial(srcSlotId);
		out.serial(dstInvId);
		out.serial(dstSlotId);
		out.serial(quantity);

		// Special case for guilds that are not on the same counter as the other inventories
		// The guild counter is global on the server so it can changes without this client
		uint16 nGuildSessionCounter = 0;
		if ((srcInvId == (uint16)INVENTORIES::guild) || (dstInvId == (uint16)INVENTORIES::guild))
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			nGuildSessionCounter = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:INVENTORY:SESSION")->getValue16();
		}
		out.serial(nGuildSessionCounter); // always serial to use the fixed-sized msg.xml scheme

		NetMngr.push(out);

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->incLocalSyncActionCounter();
		//nlinfo("impulseCallBack : %s %d %d %d %d %d sent", sMsg.c_str(), srcInvId, srcSlotId, dstInvId, dstSlotId, quantity);
	}
	else
		nlwarning(" unknown message name '%s'",sMsg.c_str());
}

/** Display the popup to ask item quantity
  */
//=====================================================================================================================
static void displayQuantityPopup(CCtrlBase *pCaller, CDBCtrlSheet *pCSSrc, CDBCtrlSheet *pCSDst, sint32 availableStack, TStackMode stackMode)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	// get the dialog stack
	CInterfaceGroup	*group= dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId("ui:interface:stack_dialog") );
	CGroupEditBox	*editBox= dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromId("ui:interface:stack_dialog:edit:eb") );
	if(group && editBox)
	{
		// write all info for the modal
		//set the selected stack variables
		CurrentStackSrc= pCSSrc;
		CurrentStackDst= pCSDst;
		CurrentStackMode= stackMode;

		// tmp sheet to copy aspect in the local db
		CViewBase::TCtorParam params;
		CDBCtrlSheet destSheet(params);
		destSheet.setSheet(string("UI:VARIABLES:STACK_SELECTED:DSPSLOT"));
		pCSSrc->copyAspect(&destSheet);

		// init the editbox ctrl. init cur default to max
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:STACK_SELECTED:CUR_QUANTITY")->setValue32(availableStack);
		editBox->setInputStringAsInt(availableStack);

		// enable the modal
		CWidgetManager::getInstance()->enableModalWindow(pCaller, group);

		// Set the Keyboard focus to the editbox (after the enableModalWindow())
		CWidgetManager::getInstance()->setCaptureKeyboard(editBox);
		// Select all the text for easier selection
		editBox->setSelectionAll();
	}
}

/** Have a last chance to directly drop stack items if CTRL or SHIFT pressed.
  */
//=====================================================================================================================
static void openStackItem(CCtrlBase *pCaller, CDBCtrlSheet *pCSSrc, CDBCtrlSheet *pCSDst, sint32 availableStack, TStackMode stackMode)
{
	// If the quantity to stack is just 1, then don't open the stack
	if(availableStack==1)
	{
		validateStackItem(pCSSrc, pCSDst, 1, stackMode);
	}
	// Ctrl+Drop means only 1 of the STACK
	else if(Driver->AsyncListener.isKeyDown(KeyCONTROL) ||
		Driver->AsyncListener.isKeyDown(KeyLCONTROL) ||
		Driver->AsyncListener.isKeyDown(KeyRCONTROL) )
	{
		validateStackItem(pCSSrc, pCSDst, 1, stackMode);
	}
	// Shift+Drop means all the STACK
	else if(Driver->AsyncListener.isKeyDown(KeySHIFT) ||
		Driver->AsyncListener.isKeyDown(KeyLSHIFT) ||
		Driver->AsyncListener.isKeyDown(KeyRSHIFT) )
	{
		validateStackItem(pCSSrc, pCSDst, availableStack, stackMode);
	}
	// Else display the Select popup!
	else
	{
		displayQuantityPopup(pCaller, pCSSrc, pCSDst, availableStack, stackMode);
	}
}


//=====================================================================================================================
	static void sendExchangeAddToServer(uint16 srcSlotIndex, uint16 destSlotIndex, uint16 quantitySrc)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		CBitMemStream out;
		const string sMsg = "EXCHANGE:ADD";
		if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			// Swap all the Src (quantity= quantitySrc) to dest
			out.serial(srcSlotIndex);
			out.serial(destSlotIndex);
			out.serial(quantitySrc);
			NetMngr.push(out);

			pIM->incLocalSyncActionCounter();
			//nlinfo("impulseCallBack : %s %d %d %d sent", sMsg.c_str(), srcSlotIndex, destSlotIndex, quantitySrc);
		}
		else
			nlwarning(" unknown message name '%s'",sMsg.c_str());
	}

//=====================================================================================================================
	/** An item from the exchange window is put back in the inventory.
	  * This put the items back in its start place, and unlock them. There may be several items
	  * This put the destination item in the exchange slot, unless it is the same item than src
	  */
	static void putInventoryItemToExchange(CDBCtrlSheet *src, CDBCtrlSheet *dest)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if (!src || !dest) return;

		// **** Decide if can split stack
		// remove any selection
		CDBCtrlSheet::setCurrSelection(NULL);
		sint32	stackCapacity= src->getStackable();
		bool	canStack= stackCapacity>1;
		sint32	quantitySrc= src->getNonLockedQuantity();

		// if the dest is empty and that current quantity is ONE, just exchange
		if(canStack && quantitySrc<=1)
			canStack= false;

		// **** exchange or display dialog for selection
		// exchange full if can't stack
		if(!canStack)
		{
			// Modify LOCAL
			PlayerTrade.putItemInExchange(src, dest, quantitySrc);

			// user changed the proposal => reset the local ACCEPTED db
			NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:EXCHANGE:ACCEPTED")->setValue32(0);

			// send msg to server
			sendExchangeAddToServer((uint16)src->getIndexInDB(), (uint8)dest->getIndexInDB(), (uint16)quantitySrc);
		}
		else
		{
			openStackItem(NULL, src, dest, min(quantitySrc, stackCapacity), StackModeExchange );
		}
	}

	//=====================================================================================================================
	// An exchange item is put back in the inventory
	static void putExchangedItemToInventory(CDBCtrlSheet *exchangeSlot)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if (!exchangeSlot) return;
		PlayerTrade.restoreItem(exchangeSlot);

		// user changed the proposal => reset the local ACCEPTED db
		NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:EXCHANGE:ACCEPTED")->setValue32(0);

		// send msg to server
		CBitMemStream out;
		const string sMsg = "EXCHANGE:REMOVE";
		if(GenericMsgHeaderMngr.pushNameToStream(sMsg.c_str(), out))
		{
			// Swap all the Src (quantity= quantitySrc) to dest
			uint16	slotIndex =	(uint16) exchangeSlot->getIndexInDB();
			out.serial(slotIndex);
			NetMngr.push(out);
			pIM->incLocalSyncActionCounter();
			//nlinfo("impulseCallBack : %s %d sent", sMsg.c_str(), slotIndex);
		}
		else
			nlwarning(" unknown message name '%s'",sMsg.c_str());
	}


// ***************************************************************************
static void validateStackItem(CDBCtrlSheet *pCSSrc, CDBCtrlSheet *pCSDst, sint32 val, TStackMode stackMode)
{
	// if ok, and if some value to drop
	if(pCSSrc && pCSDst && val>0)
	{
		// **** Swap???
		if(stackMode==StackModeSwap)
		{
			// special case : add item to exchange slot
			sint32	stackCapacity= pCSSrc->getStackable();
			sint32	quantitySrc= pCSSrc->getQuantity();
			sint32	quantityDst;
			if (pCSDst->getSheetId())
			{
				quantityDst = pCSDst->getQuantity();
				if (pCSDst->getLockValuePtr())
				{
					quantityDst -= pCSDst->getLockValuePtr()->getValue32();
				}
			}
			else
			{
				quantityDst = 0;
			}


			val= min(val, quantitySrc);
			val= min(val, stackCapacity-quantityDst);

			// write to Local database for minimum effective Lag
			if (pCSSrc->getInventoryIndex() != INVENTORIES::guild)
			{
				quantitySrc-= val;
				pCSSrc->setQuantity(quantitySrc);
				// if the src has no more quantity, disable it.
				if(quantitySrc==0)
					pCSSrc->setSheetId(0);
			}
			if (pCSDst->getInventoryIndex() != INVENTORIES::guild)
			{
				quantityDst+= val;
				pCSDst->setQuantity(quantityDst);
				// the dest get our type (case was an empty slot)
				pCSDst->setSheetId(pCSSrc->getSheetId());
				pCSDst->setQuality(pCSSrc->getQuality());
			}

			// send msg to server
			if(!ClientCfg.Local)
			{
				// \todo yoyo TODO_GAMEDEV: TEST NETWORK
				sendSwapItemMsg(pCSSrc, pCSDst, val);
			}
		}
		// **** Exchange
		else
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();

			// clamp
			sint32	stackCapacity= pCSSrc->getStackable();
			sint32	quantitySrc= pCSSrc->getQuantity();
			val= min(val, quantitySrc);
			val= min(val, stackCapacity);
			if(val>0)
			{
				// Modify LOCAL
				PlayerTrade.putItemInExchange(pCSSrc, pCSDst, val);

				// user changed the proposal => reset the local ACCEPTED db
				NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:EXCHANGE:ACCEPTED")->setValue32(0);

				// send msg to server
				sendExchangeAddToServer((uint16)pCSSrc->getIndexInDB(), (uint8)pCSDst->getIndexInDB(), (uint16)val);
			}
		}
	}
}


// ***************************************************************************
/** Swap an item with another in the inventory.
 *	eg: place in sword in the set, or place an armor in the kit, or swap 2 items in bags
 */
class CHandlerSwapItem: public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string src = getParam(Params, "src");
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if ((pCSSrc == NULL) || (pCSDst == NULL)) return;

		if (pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
		if (pCSDst->getType() == CCtrlSheetInfo::SheetType_Item)
		{

			if (pCSDst->getInventoryIndex() == INVENTORIES::exchange)
			{
				putInventoryItemToExchange(pCSSrc, pCSDst);
				return;
			}
			//
			if (pCSSrc->getInventoryIndex() == INVENTORIES::exchange)
			{
				putExchangedItemToInventory(pCSSrc);
				return;
			}

			// Ok, the 2 ctrls are item type. swap or stack them
			// Don't swap anything if one of them is grayed (LOCKED)
			if(pCSSrc->getGrayed() || pCSDst->getGrayed())
				return;


			// remove any selection
			CDBCtrlSheet::setCurrSelection(NULL);
			// \todo yoyo TODO_GAMEDEV: Locked ??? gestion

			sint32	stackCapacity= pCSSrc->getStackable();
			bool	canStack= stackCapacity>1;
			sint32	quantitySrc= pCSSrc->getQuantity();
			sint32	quantityDst= pCSDst->getQuantity();
			// if the dest is not empty
			if(canStack && pCSDst->getSheetId()!=0)
			{
				// must be same sheet, and same quality
				canStack= canStack && pCSDst->getSheetId()==pCSSrc->getSheetId();
				canStack= canStack && pCSDst->getQuality()==pCSSrc->getQuality();
				// the quantity of the dest must not be full
				if(canStack)
				{
					// can't drop: full
					if(quantityDst>=stackCapacity)
						return;
				}
			}
			// if the dest is empty and that current quantity is ONE, just swap
			if(canStack && pCSDst->getSheetId()==0 && quantitySrc<=1)
				canStack= false;

			// swap if can't stack
			if(!canStack)
			{
				if ((pCSDst->getInventoryIndex() != INVENTORIES::guild) &&
					(pCSSrc->getInventoryIndex() != INVENTORIES::guild)) // ? why not always skip it now with the new itemSwap?
				{
					// write to Local database for minimum effective Lag
					pCSDst->swapSheet(pCSSrc);
				}

				// send to server
				sendSwapItemMsg(pCSSrc, pCSDst, quantitySrc);
			}
			else
			{
				openStackItem(pCaller, pCSSrc, pCSDst, min(quantitySrc, stackCapacity-quantityDst), StackModeSwap );
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerSwapItem, "swap_item");

// ***************************************************************************
/** When an item swap open a "StackDialog" for items with quantities, this is called to validate the item drop
 */
class CHandlerStackOk: public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		// get the value to drop
		sint32	val= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:STACK_SELECTED:CUR_QUANTITY")->getValue32();
		CDBCtrlSheet *pCSSrc = CurrentStackSrc;
		CDBCtrlSheet *pCSDst = CurrentStackDst;
		CurrentStackSrc= NULL;
		CurrentStackDst= NULL;
		validateStackItem(pCSSrc, pCSDst, val, CurrentStackMode);
	}

};
REGISTER_ACTION_HANDLER( CHandlerStackOk, "stack_item");

// ***************************************************************************
// called when an item as been selected from a modal to put in exchange window
class CPlayerTradePutBagItemToExchange : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			CDBCtrlSheet *src = dynamic_cast<CDBCtrlSheet *>(pCaller);
			CDBCtrlSheet *dest = dynamic_cast<CDBCtrlSheet *>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
			if (src->getSheetId() == 0)
			{
				putExchangedItemToInventory(dest);
			}
			else
			{
				putInventoryItemToExchange(src, dest);
			}
	}
};
REGISTER_ACTION_HANDLER(CPlayerTradePutBagItemToExchange, "put_bag_item_to_exchange");

// ***************************************************************************
// called when an item is retored to the inventory
class CPlayerTradePutExchangeItemtoBag : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet *src = dynamic_cast<CDBCtrlSheet *>(pCaller);
		if (src) putExchangedItemToInventory(src);
	}
};
REGISTER_ACTION_HANDLER(CPlayerTradePutExchangeItemtoBag, "put_exchange_item_to_bag");



// **********************************************************************************************************
/** drag'n'drop: true if the ctrlSheet is an item
 */
class CIsItem : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string src = getParam(Params, "src");
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		CDBCtrlSheet *pCSDst =    dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
		if (pCSDst->getType() == CCtrlSheetInfo::SheetType_Item)
		{
			pCSDst->setCanDrop (true);
		}
	}
};
REGISTER_ACTION_HANDLER (CIsItem, "isitem");


// Action handle for drag'n'drop system
class CItemToEmptySlotTest : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string src = getParam(Params, "src");
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		CDBCtrlSheet *pCSDst =    dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
		{
			if (pCSDst->getSheetId() == 0) // the destination must be empty
			{
				pCSDst->setCanDrop (true);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CItemToEmptySlotTest, "itemtoemptyslottest");

// **********************************************************************************************************
// Action handle for drag'n'drop system
// Test if an item come from the player inventory
class CIsPlayerItem : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet *cs = dynamic_cast< CDBCtrlSheet* >( CCtrlDraggable::getDraggedSheet() );
		if (cs)
		{
			CDBCtrlSheet *pCSDst =    dynamic_cast<CDBCtrlSheet*>(pCaller);
			if (pCSDst) pCSDst->setCanDrop(cs->getSelectionGroupAsString() == "inventory_selection");
		}
	}
};
REGISTER_ACTION_HANDLER (CIsPlayerItem, "isplayeritem");


// **********************************************************************************************************
/** drag'n'drop: true if the ctrlSheet is an item of a special slot ("chest", "legs", etc...)
 *	example: Used to kwon if can drop a "chest" on the "chest" Kit.
 */
class CIsItemSlot : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string	src = getParam(Params, "src");
		string	strTestEmpty= getParam(Params, "test_empty");
		bool	testEmpty= strTestEmpty=="true";
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		CDBCtrlSheet *pCSDst =    dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (!pCSSrc || !pCSDst) return;
		if (pCSSrc->getInventoryIndex() == INVENTORIES::exchange &&
			pCSDst->getInventoryIndex() == INVENTORIES::exchange
		   )
		{
			return;
		}
		if (pCSSrc && pCSSrc->getType() == CCtrlSheetInfo::SheetType_Item)
		if (pCSDst && pCSDst->getType() == CCtrlSheetInfo::SheetType_Item)
		if ( !testEmpty || pCSDst->getSheetId() == 0 ) // the destination must be empty if testEmpty
		{
			if (pCSSrc->getInventoryIndex() == INVENTORIES::exchange)
			{
				// can always put back to inventory if it is in the right slot
				if (pCSDst->canDropItem(pCSSrc))
				{
					pCSDst->setCanDrop ( true );
				}
			}
			else
			{
				// Ok if the swap can be done on the 2 slots
				if( pCSSrc->canDropItem(pCSDst) && pCSDst->canDropItem(pCSSrc) )
					pCSDst->setCanDrop ( true );
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CIsItemSlot, "isitem_slot");


// **********************************************************************************************************
// Check if we can exchange the item with a player or a bot
bool checkCanExchangeItem(CDBCtrlSheet *pCSSrc)
{
	if(!pCSSrc)
		return false;

	// Check if item is completely locked
	if (pCSSrc->getNonLockedQuantity() == 0)
		return false;

	bool bWeared = false;
	bool bDropOrSell = true;

	// Check if item weared
	// --------------------
	if (pCSSrc->getSecondIndexInDB()==INVENTORIES::bag)
	{
		CInventoryManager *pInv = CInventoryManager::getInstance();
		if (pInv->isBagItemWeared(pCSSrc->getIndexInDB()))
			bWeared= true;
	}

	// Check if item sellable and dropable
	// -----------------------------------
	const CItemSheet *pIS = pCSSrc->asItemSheet();
	if (pIS != NULL)
	{
		bDropOrSell = pIS->canExchangeOrGive(PlayerTrade.BotChatGiftContext);
	}

	// Locked by owner; cannot trade
	if (pCSSrc->getLockedByOwner())
	{
		return false;
	}

	// Special case if this is an animal ticket
	if ((pIS != NULL) && (pIS->Family == ITEMFAMILY::PET_ANIMAL_TICKET))
	{
		// If we are not giving something to a bot (so we are exchanging with a player)
		if (PlayerTrade.BotChatGiftContext == false)
			bDropOrSell = true;
	}

	if (!bWeared && bDropOrSell)
		return true;
	return false;
}

// **********************************************************************************************************
class CCanDropToExchange : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string	src = getParam(Params, "src");
		CInterfaceElement *pElt = CWidgetManager::getInstance()->getElementFromId(src);
		CDBCtrlSheet *pCSSrc = dynamic_cast<CDBCtrlSheet*>(pElt);
		CDBCtrlSheet *pCSDst =    dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (!pCSSrc || !pCSDst) return;

		// Exchange can only be done from bag to exchange inventories

		if (pCSSrc->getSecondIndexInDB() == INVENTORIES::bag &&
			pCSDst->getSecondIndexInDB() == INVENTORIES::exchange)
		{
			if (checkCanExchangeItem(pCSSrc))
			{
				pCSDst->setCanDrop ( true );
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CCanDropToExchange, "can_drop_to_exchange");


// **********************************************************************************************************

/** Clear the selected sheet
  */
class CClearSelectedSheet : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CDBCtrlSheet::setCurrSelection(NULL);
	}
};
REGISTER_ACTION_HANDLER (CClearSelectedSheet, "clear_selected_sheet");


//accept exchange
class CHandlerAcceptExchange: public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		sint64 counter;
		if (!CInterfaceExpr::evalAsInt(getParam(sParams, "counter"), counter))
		{
			nlwarning("<CHandlerAcceptExchange::execute> Can't retrieve counter.");
			return;
		}
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("EXCHANGE:VALIDATE", out))
		{
			uint8 u8counter = (uint8) counter;
			out.serial(u8counter);
			NetMngr.push(out);
		}
		else
			nlwarning("<CHandlerAcceptExchange::execute> unknown message name 'EXCHANGE:VALIDATE");
	}
private:
	CCDBNodeLeaf* _Counter;
};
REGISTER_ACTION_HANDLER( CHandlerAcceptExchange, "accept_exchange");


//invalidate exchange
class CHandlerInvalidateExchange: public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		sint64 counter;
		if (!CInterfaceExpr::evalAsInt(getParam(sParams, "counter"), counter))
		{
			nlwarning("<CHandlerInvalidateExchange::execute> Can't retrieve counter.");
			return;
		}
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("EXCHANGE:INVALIDATE", out))
		{
			NetMngr.push(out);
		}
		else
			nlwarning("<CHandlerInvalidateExchange::execute> unknown message name 'EXCHANGE:INVALIDATE");
	}
private:
	CCDBNodeLeaf* _Counter;
};
REGISTER_ACTION_HANDLER( CHandlerInvalidateExchange, "invalidate_exchange");


//end exchange
class CHandlerEndExchange: public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		PlayerTrade.restoreAllItems();
		CInterfaceManager *im = CInterfaceManager::getInstance();
		NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:EXCHANGE:BEGUN")->setValue32(0);

		if (!ClientCfg.Local)
		{
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream("EXCHANGE:END", out))
			{
				// must increment action counter
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				pIM->incLocalSyncActionCounter();

				// send the msg
				NetMngr.push(out);
			}
			else
				nlwarning("<CHandlerEndExchange::execute> unknown message name 'EXCHANGE:END");
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerEndExchange, "end_exchange");



//validate the selected quantity of seed
class CHandlerValidateSeedSel: public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		sint64 quantity;
		if (!CInterfaceExpr::evalAsInt(getParam(sParams, "quantity"), quantity))
		{
			nlwarning("<CHandlerValidateSeedSel::execute> Can't retrieve quantity of seeds");
			return;
		}
		// user changed the proposal => reset the local ACCEPTED db
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:EXCHANGE:ACCEPTED")->setValue32(0);

		// Send the msg
		CBitMemStream out;
		if(!GenericMsgHeaderMngr.pushNameToStream("EXCHANGE:SEEDS", out))
		{
			nlwarning("CHandlerValidateSeedSel : unknown message EXCHANGE:SEEDS");
			return;
		}

		out.serial(quantity);

		NetMngr.push(out);
		// increment counter
		pIM->incLocalSyncActionCounter();
	}
};
REGISTER_ACTION_HANDLER( CHandlerValidateSeedSel, "validate_seed_sel");




//accept exchange invitation
class CHandlerAcceptExchangeInvite: public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("EXCHANGE:ACCEPT_INVITATION", out))
		{
			NetMngr.push(out);
			//nlinfo("impulseCallBack : EXCHANGE:ACCEPT_INVITATION sent");
		}
		else
			nlwarning("<CHandlerAcceptExchangeInvite::execute> unknown message name 'EXCHANGE:ACCEPT_INVITATION");
	}
};
REGISTER_ACTION_HANDLER( CHandlerAcceptExchangeInvite, "accept_exchange_invitation");

//decline exchange invitation
class CHandlerDeclineExchangeInvite: public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("EXCHANGE:DECLINE_INVITATION", out))
		{
			NetMngr.push(out);
			//nlinfo("impulseCallBack : EXCHANGE:DECLINE_INVITATION sent");
		}
		else
			nlwarning("<CHandlerDeclineExchangeInvite::execute> unknown message name 'EXCHANGE:DECLINE_INVITATION");
	}
};
REGISTER_ACTION_HANDLER( CHandlerDeclineExchangeInvite, "decline_exchange_invitation");



// tool fct to destroy or drop an item
static void dropOrDestroyItem(CDBCtrlSheet *item, CBitMemStream &out, uint16 quantity)
{
	CInventoryManager::getInstance()->dropOrDestroyItem(item, out, quantity);
}

// destroy an item
class CHandlerDestroyItem : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		sint64 quantity;
		if (!CInterfaceExpr::evalAsInt(getParam(sParams, "quantity"), quantity))
		{
			nlwarning("<CHandlerDestroyItem::execute> Can't retrieve quantity");
			return;
		}
		// get the calling item
		CDBCtrlSheet *item = CDBCtrlSheet::getCurrSelSheet();
		if (!item)
		{
			nlwarning("<CHandlerDestroyItem::execute> no caller sheet found");
			return;
		}

		// Don't destroy locked items
		if (item->getLockedByOwner())
		{
			return;
		}

		// if the item is currently selected, removes the selection
		if (item == CDBCtrlSheet::getCurrSelection())
		{
			CDBCtrlSheet::setCurrSelection(NULL);
			// old bot chat code
			/*CBotChatUI::removeItemHighlight();
			CBotChatUI::removeFocusFromItemQuantityEditBox();*/
		}
		if (!ClientCfg.Local)
		{
			//
			CBitMemStream out;
			if(!GenericMsgHeaderMngr.pushNameToStream( "ITEM:DESTROY", out))
			{
				nlwarning("<CHandlerDestroyItem::execute> unknown message name 'ITEM:DESTROY'");
				return;
			}
			dropOrDestroyItem(item, out, (uint16) quantity);
		}
		else
		{
			item->setSheetId(0);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerDestroyItem, "destroy_item");


// drop an item
class CHandlerDropItem : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		sint64 quantity = 1;
		string sQuantity = getParam(sParams, "quantity");
		if (!sQuantity.empty())
		if (!CInterfaceExpr::evalAsInt(sQuantity, quantity))
		{
			nlwarning("<CHandlerDropItem::execute> Can't retrieve quantity");
			quantity = 1;
		}
		// get the calling item
		CDBCtrlSheet *item = CDBCtrlSheet::getCurrSelSheet();
		if (!item)
		{
			nlwarning("<CHandlerDestroyItem::execute> no caller sheet found");
			return;
		}
		//
		CBitMemStream out;
		if(!GenericMsgHeaderMngr.pushNameToStream( "ITEM:DROP", out))
		{
			nlwarning("<CHandlerDropItem::execute> unknown message name 'ITEM:DROP'");
			return;
		}
		dropOrDestroyItem(item, out, (uint16)quantity);
	}
};
REGISTER_ACTION_HANDLER( CHandlerDropItem, "drop_item");

// **********************************************************************************************************
/*
class CHandlerActiveSheath : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Get the user interface value.
		uint8 activeSheath = (uint8)NLGUI::CDBManager::getInstance()->getDbProp( CWidgetManager::getInstance()->getParser()->getDefine("ui_set_active") )->getValue32();

		// Write to the Local Database.
		NLGUI::CDBManager::getInstance()->getDbProp( "LOCAL:INVENTORY:ACTIVE_SHEATH" )->setValue32(activeSheath);

		// Send to server
		if(!ClientCfg.Local)
		{
			CBitMemStream out;
			if(!GenericMsgHeaderMngr.pushNameToStream( "ITEM:ACTIVATE_SHEATH", out) )
			{
				nlwarning("<CHandlerServerActiveSheath::execute> unknown message name 'ITEM:ACTIVATE_SHEATH'");
				return;
			}
			nlinfo("<CHandlerActiveSheath> send %d",activeSheath);
			out.serial( activeSheath );
			NetMngr.push(out);
			CMacroCmdManager::getInstance()->incActionId();
			// Yoyo: must increment the InterfaceCounter.
			pIM->incLocalSyncActionCounter();
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerActiveSheath, "active_sheath" );
*/
// **********************************************************************************************************
/*
class CHandlerReceiveActiveSheath : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		// NB: the System does not recurs between active_sheath and receive_active_sheath since
		//	at a time, the 2 values 'ui_set_active' and LOCAL:INVENTORY:ACTIVE_SHEATH
		//	will be the same (an observer is called only if the value change)

		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Get the user interface value.
		uint8 activeSheath = (uint8)NLGUI::CDBManager::getInstance()->getDbProp( "LOCAL:INVENTORY:ACTIVE_SHEATH" )->getValue32();

		// Write to the Local Database.
		NLGUI::CDBManager::getInstance()->getDbProp( CWidgetManager::getInstance()->getParser()->getDefine("ui_set_active") )->setValue32(activeSheath);
	}
};
REGISTER_ACTION_HANDLER( CHandlerReceiveActiveSheath, "receive_active_sheath" );
*/
// **********************************************************************************************************
class CHandlerEndHarvest : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CBitMemStream out;
		if( sParams == string("loot") )
		{
			if (!GenericMsgHeaderMngr.pushNameToStream( "ITEM:PICK_UP_CLOSE", out ) )
			{
				nlwarning("<CHandlerEndHarvest::execute> unknown message name 'PICK_UP_CLOSE'");
				return;
			}
		}
		else
		{
			if (!GenericMsgHeaderMngr.pushNameToStream( "ITEM:HARVEST_CLOSE", out ) )
			{
				nlwarning("<CHandlerEndHarvest::execute> unknown message name 'ITEM:HARVEST_CLOSE'");
				return;
			}
		}
		NetMngr.push(out);
	}
};
REGISTER_ACTION_HANDLER( CHandlerEndHarvest, "end_harvest" );

// **********************************************************************************************************
class CHandlerHarvestItem : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CBitMemStream out;

		if (!GenericMsgHeaderMngr.pushNameToStream( "ITEM:HARVEST", out ) )
		{
				nlwarning("<CHandlerEndHarvest::execute> unknown message name 'PICK_UP_CLOSE'");
				return;
		}
		CDBCtrlSheet *sheet = dynamic_cast<CDBCtrlSheet *>(pCaller);
		if (!sheet)
		{
			nlwarning("Can't get sheet or bad type.");
			return;
		}
		IListSheetBase *sheetList  = sheet->getListSheetParent();
		if (!sheetList)
		{
			nlwarning("Can't get sheet list or bad type.");
			return;
		}
		sint sheetIndex = sheetList->getIndexOf(sheet);
		if (sheetIndex == -1)
		{
			nlwarning("Can't retrieve sheet index");
			return;
		}
		uint16 slot = (uint16) sheetIndex;
		uint16 quantity = (uint16) sheet->getQuantity();
		out.serial(slot);
		out.serial(quantity);
		NetMngr.push(out);
	}
};
REGISTER_ACTION_HANDLER( CHandlerHarvestItem, "harvest_item" );

// **********************************************************************************************************
void getCtrlSheets(CInterfaceGroup *pIG, vector<CDBCtrlSheet*> &res)
{
	uint i;
	const vector<CInterfaceGroup*> child = pIG->getGroups();
	for (i = 0; i < child.size(); ++i)
		if (child[i]->getActive())
			getCtrlSheets (child[i], res);

	const vector<CCtrlBase*> &rCBs = pIG->getControls();
	for (i = 0; i < rCBs.size(); ++i)
	{
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(rCBs[i]);
		if (pCS != NULL)
			res.push_back(pCS);
	}
}

// MOVE AN ITEM
// **********************************************************************************************************
class CHandlerMoveItem : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		// get the calling item
		CDBCtrlSheet *item = CDBCtrlSheet::getCurrSelSheet();
		if (item == NULL)
		{
			item = dynamic_cast<CDBCtrlSheet*>(pCaller);
			if (item == NULL)
			{
				nlwarning("<CHandlerMoveItem::execute> no caller sheet found");
				return;
			}
		}
		if (item->getGrayed()) return;

		// Check for a place

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string sDest = getParam(sParams, "to");
		// Test With List of Items.
		if (sDest == "lists")
		{
			uint nListIt = 0, nNbList = 1024;
			string sNbList = getParam(sParams, "nblist");
			if (!sNbList.empty())
			{
				CInterfaceExprValue res;
				if (!CInterfaceExpr::eval(sNbList,res))
					return;
				nNbList = (uint)res.getInteger();
			}

			// Test All Lists
			if (nNbList > 0)
			{
				string sListId = getParam(sParams, "listsheet"+toString(nListIt));
				while (!sListId.empty())
				{
					IListSheetBase *pLS = dynamic_cast<IListSheetBase*>(CWidgetManager::getInstance()->getElementFromId(sListId));
					if (pLS == NULL) return;
					// search an empty slot where to put
					sint32 nbelt = pLS->getNbSheet();
					for (sint32 i = 0; i < nbelt; ++i)
					{
						if (pLS->getSheet(i)->getSheetId() == 0)
						{
							// Send swap_item
							CAHManager::getInstance()->runActionHandler("swap_item", pLS->getSheet(i), "src="+toString(pCaller->getId()));
							return;
						}
					}
					// ok try next list
					nListIt++;
					if (nListIt >= nNbList)
					{
						// Display a Warning Message because no more slot in inventory
						pIM->displaySystemInfo(CI18N::get("uiMoveToNoMoreSlot"), "CHK");
						return;
					}
					sListId = getParam(sParams, "listsheet"+toString(nListIt));
				}
			}
		}
		// Test With Group of Items. YOYO: TODO: test if works
		/*else if (sDest == "groups")
		{
			uint nGrpIt = 0, nNbGrp = 1024, nGrpBeg = 0;
			string sNbGrp = getParam(sParams, "nbgroup");
			string sGrpBeg = getParam(sParams, "groupbegin");

			if (!sNbGrp.empty())
			{
				CInterfaceExprValue res;
				if (!CInterfaceExpr::eval(sNbGrp, res))
				{
					nlwarning("cannot evaluate %s",sNbGrp.c_str());
					return;
				}
				nNbGrp = (uint)res.getInteger();
			}

			if (!sGrpBeg.empty())
			{
				CInterfaceExprValue res;
				if (!CInterfaceExpr::eval(sGrpBeg, res))
				{
					nlwarning("cannot evaluate %s",sGrpBeg.c_str());
					return;
				}
				nGrpBeg = (uint)res.getInteger();
			}

			nGrpIt = nGrpBeg;
			string sGrpId = getParam(sParams, "groupsheet"+toString(nGrpIt));
			// Begin by trying to put the item in an empty slot
			if (nNbGrp > 0)
			while (!sGrpId.empty())
			{
				uint i;
				CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(sGrpId));
				if (pIG != NULL)
				{
					// Get all the sheets of the group
					vector<CDBCtrlSheet*> vDBCS;
					getCtrlSheets (pIG, vDBCS);
					for (i = 0; i < vDBCS.size(); ++i)
					{
						CDBCtrlSheet *pCS = vDBCS[i];
						if ((pCS != NULL) && (pCS->getType() == CCtrlSheetInfo::SheetType_Item) &&
							(pCS->getSheetId() == 0) && pCS->canDropItem(item) && (!pCS->getGrayed()))
						{
							// Send swap_item
							CAHManager::getInstance()->runActionHandler("swap_item", pCS, "src="+toString(pCaller->getId()));
							return;
						}
					}
				}
				// Next Group
				nGrpIt++;
				if (nGrpIt == nNbGrp) nGrpIt = 0;
				if (nGrpIt == nGrpBeg) break;
				sGrpId = getParam(sParams, "groupsheet"+toString(nGrpIt));
			}

			// If there was no empty slot try to swap item from an existing slot
			nGrpIt = nGrpBeg;
			sGrpId = getParam(sParams, "groupsheet"+toString(nGrpIt));

			if (nNbGrp > 0)
			while (!sGrpId.empty())
			{
				uint i;
				CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(sGrpId));
				if (pIG != NULL)
				{
					// Get all the sheets of the group
					vector<CDBCtrlSheet*> vDBCS;
					getCtrlSheets (pIG, vDBCS);
					for (i = 0; i < vDBCS.size(); ++i)
					{
						CDBCtrlSheet *pCS = vDBCS[i];
						if ((pCS != NULL) && (pCS->getType() == CCtrlSheetInfo::SheetType_Item))
						{
							if ((pCS->canDropItem(item)) && (!pCS->getGrayed()))
							{
								// Send swap_item
								CAHManager::getInstance()->runActionHandler("swap_item", pCS, "src="+toString(pCaller->getId()));
								return;
							}
						}
					}
				}
				// Next Group
				nGrpIt++;
				if (nGrpIt == nNbGrp) nGrpIt = 0;
				if (nGrpIt == nGrpBeg) break;
				sGrpId = getParam(sParams, "groupsheet"+toString(nGrpIt));
			}

		}*/
	}
};
REGISTER_ACTION_HANDLER( CHandlerMoveItem, "move_item" );


// DragNDrop AN ITEM
// **********************************************************************************************************
class CHandlerDragNDrop : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		string sSrc = getParam(sParams,"src");
		string sDst = getParam(sParams,"dst");
		string sAH = getParam(sParams,"ah");
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *pCSsrc = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(sSrc));
		CDBCtrlSheet *pCSdst = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(sDst));
		if ((pCSdst == NULL) || (pCSsrc == NULL) || sAH.empty()) return;
		CAHManager::getInstance()->runActionHandler(sAH, pCSdst, "src="+pCSsrc->getId());
	}
};
REGISTER_ACTION_HANDLER( CHandlerDragNDrop, "drag_n_drop" );


// **********************************************************************************************************
static void sendToServerEnchantMessage(uint8 invent, uint16 slot)
{
	CBitMemStream out;
	const string sMsg = "ITEM:ENCHANT";

	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		out.serial( invent );
		out.serial( slot );
		NetMngr.push(out);
	}
	else
		nlinfo("unknown message %s", sMsg.c_str());
}

// **********************************************************************************************************
class CHandlerItemCristalEnchant : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler("item_cristal_reload", pCaller, sParams);
	}
};
REGISTER_ACTION_HANDLER( CHandlerItemCristalEnchant, "item_cristal_enchant" );

// **********************************************************************************************************
class CHandlerItemCristalReload : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCS == NULL) return;

		sendToServerEnchantMessage((uint8)pCS->getInventoryIndex(), (uint16)pCS->getIndexInDB());
	}
};
REGISTER_ACTION_HANDLER( CHandlerItemCristalReload, "item_cristal_reload" );

// **********************************************************************************************************
class CItemMenuInBagInfoWaiter
: public IItemInfoWaiter
{
public:
	// The item used to open this window
	CDBCtrlSheet* CtrlSheet;
public:
	void infoValidated(CDBCtrlSheet* CtrlSheet);
	virtual void infoReceived();
};
static CItemMenuInBagInfoWaiter ItemMenuInBagUpdater;

void CItemMenuInBagInfoWaiter::infoReceived()
{
	getInventory().removeItemInfoWaiter(&ItemMenuInBagUpdater);
	infoValidated(CtrlSheet);
}


void CItemMenuInBagInfoWaiter::infoValidated(CDBCtrlSheet* ctrlSheet)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	
	// get the dialog stack
	CInterfaceGroup* pMenu = dynamic_cast<CInterfaceGroup*>( CWidgetManager::getInstance()->getElementFromId("ui:interface:item_menu_in_bag") );
	if(!pMenu)	return;

	const CItemSheet *pIS = ctrlSheet->asItemSheet();
	CViewTextMenu	*pItemTextEdition = dynamic_cast<CViewTextMenu*>(pMenu->getView("item_text_edition"));
	if (pIS != NULL && pItemTextEdition && ITEMFAMILY::isTextCustomizable(pIS->Family))
	{
		CClientItemInfo const& itemInfo = getInventory().getItemInfo(getInventory().getItemSlotId(ctrlSheet) );

		// get the CreatorTextID
		bool isCraftedByUserEntity = false;
		ucstring creatorNameString;
		if( STRING_MANAGER::CStringManagerClient::instance()->getString ( itemInfo.CreatorName, creatorNameString) )
		{
			if (toLower(UserEntity->getEntityName()+PlayerSelectedHomeShardNameWithParenthesis) == toLower(creatorNameString))
				isCraftedByUserEntity = true;
		}

		pItemTextEdition->setActive(isCraftedByUserEntity);

	}
}

// ***************************************************************************

class CHandlerItemMenuCheck : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		uint i;

		// Get the ctrl sheet that launched this menu
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCS == NULL) return;
		INVENTORIES::TInventory		invId= (INVENTORIES::TInventory)pCS->getInventoryIndex();

		// Get the menu launched
		CInterfaceGroup	  *pMenu= dynamic_cast<CInterfaceGroup*>(pCaller);
		if(!pMenu)	return;

		// Get all Text entries
		CViewTextMenu	*pEquip = dynamic_cast<CViewTextMenu*>(pMenu->getView("equip"));
		CViewTextMenu	*pCrisEnchant = dynamic_cast<CViewTextMenu*>(pMenu->getView("cris_enchant"));
		CViewTextMenu	*pCrisReload = dynamic_cast<CViewTextMenu*>(pMenu->getView("cris_reload"));
		CViewTextMenu	*pTeleportUse = dynamic_cast<CViewTextMenu*>(pMenu->getView("teleport_use"));
		CViewTextMenu	*pItemConsume = dynamic_cast<CViewTextMenu*>(pMenu->getView("item_consume"));
		CViewTextMenu	*pXpCatalyserUse = dynamic_cast<CViewTextMenu*>(pMenu->getView("xp_catalyser_use"));
		CViewTextMenu	*pDrop = dynamic_cast<CViewTextMenu*>(pMenu->getView("drop"));
		CViewTextMenu	*pDestroy = dynamic_cast<CViewTextMenu*>(pMenu->getView("destroy"));
		CViewTextMenu	*pLockUnlock = dynamic_cast<CViewTextMenu*>(pMenu->getView("lockunlock"));
		CViewTextMenu	*pMoveSubMenu = dynamic_cast<CViewTextMenu*>(pMenu->getView("move"));
		CViewTextMenu	*pMoveToBag = dynamic_cast<CViewTextMenu*>(pMenu->getView("bag"));
		CViewTextMenu	*pMoveToGuild = dynamic_cast<CViewTextMenu*>(pMenu->getView("guild"));
		CViewTextMenu	*pMoveToRoom = dynamic_cast<CViewTextMenu*>(pMenu->getView("room"));
		CViewTextMenu	*pMoveToPa[MAX_INVENTORY_ANIMAL];

		bool bIsLockedByOwner = pCS->getLockedByOwner();

		for(i=0;i<MAX_INVENTORY_ANIMAL;i++)
		{
			pMoveToPa[i]= dynamic_cast<CViewTextMenu*>(pMenu->getView(toString("pa%d", i)));
		}
		CViewTextMenu	*pItemInfos = dynamic_cast<CViewTextMenu*>(pMenu->getView("infos"));
		CViewTextMenu	*pItemTextDisplay = dynamic_cast<CViewTextMenu*>(pMenu->getView("item_text_display"));
		CViewTextMenu	*pItemTextEdition = dynamic_cast<CViewTextMenu*>(pMenu->getView("item_text_edition"));


		// **** Active Entries
		// Active Enchant / Reload for a subset of items
		if(pCrisEnchant)	pCrisEnchant->setActive(false);
		if(pCrisReload)	pCrisReload->setActive(false);
		if(pTeleportUse) pTeleportUse->setActive(false);
		if(pItemConsume) pItemConsume->setActive(false);
		if(pXpCatalyserUse) pXpCatalyserUse->setActive(false);
		if(pItemTextDisplay) pItemTextDisplay->setActive(false);
		if(pItemTextEdition) pItemTextEdition->setActive(false);

		if(pLockUnlock) pLockUnlock->setActive(true);

		const CItemSheet *pIS = pCS->asItemSheet();
		if (invId != INVENTORIES::guild)
		if (pIS != NULL)
		{
			if (pCrisEnchant && pIS->Family == ITEMFAMILY::CRYSTALLIZED_SPELL && !bIsLockedByOwner)
				pCrisEnchant->setActive(true);
			if (pCrisReload && pIS->Family == ITEMFAMILY::ITEM_SAP_RECHARGE && !bIsLockedByOwner)
				pCrisReload->setActive(true);
			// teleport can be used only from bag (NB: should not exist in mektoub....)
			if (pTeleportUse && pIS->Family == ITEMFAMILY::TELEPORT && pCS->getInventoryIndex()==INVENTORIES::bag && !bIsLockedByOwner)
			{
				pTeleportUse->setActive(true);
			}
			if (pItemConsume && pIS->IsConsumable == true && pCS->getInventoryIndex()==INVENTORIES::bag && !bIsLockedByOwner)
			{
				pItemConsume->setActive(true);
			}
			if (pXpCatalyserUse && pIS->Family == ITEMFAMILY::XP_CATALYSER && pCS->getInventoryIndex()==INVENTORIES::bag && !bIsLockedByOwner)
			{
				pXpCatalyserUse->setActive(true);
			}
			if (pItemTextEdition && ITEMFAMILY::isTextCustomizable(pIS->Family))
			{
				if (pCS->getInventoryIndex()==INVENTORIES::bag)
				{
					bool isTextEditionActive = false;
					static const string itemTextEditionPriv = ":DEV:SGM:GM:EM:";
					if (!UserPrivileges.empty() && itemTextEditionPriv.find(UserPrivileges)!=std::string::npos)
					{
						isTextEditionActive = true;
					}
					else
					{
						// Finish setting the isTextEditionActive boolean or add the waiter
						if (getInventory().isItemInfoUpToDate(getInventory().getItemSlotId(pCS)))
						{
							// get the CreatorTextID
							ucstring creatorNameString;
							if( STRING_MANAGER::CStringManagerClient::instance()->getString ( getInventory().getItemInfo(getInventory().getItemSlotId(pCS)).CreatorName, creatorNameString) )
							{
								if (toLower(UserEntity->getEntityName()+PlayerSelectedHomeShardNameWithParenthesis) == toLower(creatorNameString))
									isTextEditionActive = true;
							}
						}
						else
						{
							// Prepare the waiter
							ItemMenuInBagUpdater.ItemSheet= pCS->getSheetId();
							ItemMenuInBagUpdater.ItemSlotId= getInventory().getItemSlotId(pCS);
							ItemMenuInBagUpdater.CtrlSheet = pCS;
							// Add the waiter
							getInventory().addItemInfoWaiter(&ItemMenuInBagUpdater);
						}
					}
					pItemTextEdition->setActive(isTextEditionActive);
				}
				pItemInfos->setActive(true);
			}
			if (pItemTextDisplay && pIS->Family == ITEMFAMILY::SCROLL)
			{
				if (pCS->getInventoryIndex()==INVENTORIES::bag)
					pItemTextDisplay->setActive(true);
				pItemInfos->setActive(false);
			}
			else
			{
				pItemInfos->setActive(true);
			}
		}

		CInventoryManager	&invMngr= getInventory();

		// If the item is an animal representation or rpjob item
		if (pIS!=NULL && ((pIS->Family == ITEMFAMILY::PET_ANIMAL_TICKET) || (pIS->Id.toString().substr(0, 6) == "rpjob_")))
		{
			// cannot move to other animals! :)
			if(pMoveToBag)		pMoveToBag->setActive(false);
			for(i=0;i<MAX_INVENTORY_ANIMAL;i++)
				if(pMoveToPa[i]) pMoveToPa[i]->setActive(false);

			// additionnaly, cannot drop/destroy/lock an animal item.
			if(pDrop)			pDrop->setActive(false);
			if(pDestroy)		pDestroy->setActive(false);
			if(pLockUnlock)		pLockUnlock->setActive(false);
		}
		else
		{
			// Disable Text entries not supported by this menu (eg: if I am the Bag, i cannot drop to myself!)
			// Also Disable Text entries if the inventory is not present
			if(pMoveToBag)
				pMoveToBag->setActive(	invId!=INVENTORIES::bag &&
										invMngr.isInventoryPresent(INVENTORIES::bag) &&
										(invId!=INVENTORIES::guild || invMngr.isInventoryPresent(INVENTORIES::guild)) );

			for(i=0;i<MAX_INVENTORY_ANIMAL;i++)
			{
				if (pMoveToPa[i])
					pMoveToPa[i]->setActive(invId!=INVENTORIES::guild &&
											(uint)invId!=INVENTORIES::pet_animal+i &&
											 invMngr.isInventoryPresent((INVENTORIES::TInventory)(INVENTORIES::pet_animal+i)) );
			}

			if (pMoveToGuild)
				pMoveToGuild->setActive(invId==INVENTORIES::bag && invMngr.isInventoryPresent(INVENTORIES::guild));

			if (pMoveToRoom)
				pMoveToRoom->setActive(invId==INVENTORIES::bag && invMngr.isInventoryPresent(INVENTORIES::player_room));

			// std case: can drop / destroy
			if(pDrop)		pDrop->setActive(invId!=INVENTORIES::guild);
			if(pDestroy)	pDestroy->setActive(invId!=INVENTORIES::guild);
			if(pLockUnlock)	pLockUnlock->setActive(invId!=INVENTORIES::guild);
		}

		// hide the move entry completely?
		bool	someMovePossible= false;
		if(pMoveSubMenu)
		{
			if(pMoveToBag)		someMovePossible= someMovePossible || pMoveToBag->getActive();
			for(i=0;i<MAX_INVENTORY_ANIMAL;i++)
			{
				if(pMoveToPa[i]) someMovePossible= someMovePossible || pMoveToPa[i]->getActive();
			}
			if(pMoveToGuild)	someMovePossible= someMovePossible || pMoveToGuild->getActive();
			if(pMoveToRoom)		someMovePossible= someMovePossible || pMoveToRoom->getActive();
			pMoveSubMenu->setActive(someMovePossible);
		}

		// Equip
		if (pEquip != NULL)
		{
			// active for some of item categories
			bool	valid= (invId==INVENTORIES::bag) && pIS && (
				 pIS->Family==ITEMFAMILY::ARMOR ||
				 pIS->Family==ITEMFAMILY::MELEE_WEAPON ||
				 pIS->Family==ITEMFAMILY::RANGE_WEAPON ||
				 pIS->Family==ITEMFAMILY::AMMO ||
				 pIS->Family==ITEMFAMILY::SHIELD ||
				 pIS->Family==ITEMFAMILY::CRAFTING_TOOL ||
				 pIS->Family==ITEMFAMILY::HARVEST_TOOL ||
				 pIS->Family==ITEMFAMILY::TAMING_TOOL ||
				 pIS->Family==ITEMFAMILY::TRAINING_TOOL ||
				 pIS->Family==ITEMFAMILY::JEWELRY ||
				 pIS->Family==ITEMFAMILY::HANDLED_ITEM);
			pEquip->setActive(valid);

			/* gray if crafting window is opened (cannot equip another tool while crafting),
				and the item is a HAND item
			*/
			if(valid)
			{
				pEquip->setGrayed(
					NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ISACTIVE:PHRASE_FABER")->getValueBool() &&
					pIS->Family!=ITEMFAMILY::ARMOR &&
					pIS->Family!=ITEMFAMILY::JEWELRY
					);
			}
		}

		//Item Text Edition
		if (pItemTextEdition != NULL && pItemTextDisplay != NULL)
		{
			pItemTextEdition->setGrayed(NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ISACTIVE:PHRASE_EDIT_CUSTOM")->getValueBool());
			pItemTextDisplay->setGrayed(NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ISACTIVE:PHRASE_EDIT_CUSTOM")->getValueBool());
		}

		if (pCS->getGrayed())
		{
			if (pEquip) pEquip->setActive(false);
			if (pDestroy) pDestroy->setActive(false);
			if (pLockUnlock) pLockUnlock->setActive(false);
			if (pMoveSubMenu) pMoveSubMenu->setActive(false);
		}

		if (bIsLockedByOwner) 
		{
			if (pLockUnlock) pLockUnlock->setHardText("uimUnlockItem");
			// Cannot drop/destroy if locked by owner
			if (pDrop)    pDrop->setActive(false);
			if (pDestroy) pDestroy->setActive(false);
		}
		else
		{
			if (pLockUnlock) pLockUnlock->setHardText("uimLockItem");
		}

		// Only show lock menu item if inventory contains the info
		if (pLockUnlock) pLockUnlock->setActive(pCS->canOwnerLock());


		// **** Gray Entries
		// If ourselves are not available, then gray all
		if(!invMngr.isInventoryAvailable(invId))
		{
			if(pCrisEnchant)	pCrisEnchant->setGrayed(true);
			if(pCrisReload)		pCrisReload->setGrayed(true);
			if(pTeleportUse)	pTeleportUse->setGrayed(true);
			if(pItemConsume)	pItemConsume->setGrayed(true);
			if(pXpCatalyserUse)	pXpCatalyserUse->setGrayed(true);
			if(pDrop)			pDrop->setGrayed(true);
			if(pDestroy)		pDestroy->setGrayed(true);
			if(pLockUnlock)		pLockUnlock->setGrayed(true);
			if(pMoveSubMenu)	pMoveSubMenu->setGrayed(true);
			if(pMoveToBag)		pMoveToBag->setGrayed(true);
			for(i=0;i<MAX_INVENTORY_ANIMAL;i++)
			{
				if(pMoveToPa[i])	pMoveToPa[i]->setGrayed(true);
			}
		}
		// Gray Text entries according to Availables Destinations
		else
		{
			// ungray basics
			if(pCrisEnchant)	pCrisEnchant->setGrayed(false);
			if(pCrisReload)		pCrisReload->setGrayed(false);
			if(pTeleportUse)	pTeleportUse->setGrayed(false);
			if(pItemConsume)	pItemConsume->setGrayed(false);
			if(pXpCatalyserUse)	pXpCatalyserUse->setGrayed(false);
			if(pDrop)			pDrop->setGrayed(false);
			if(pDestroy)		pDestroy->setGrayed(false);
			if(pLockUnlock)		pLockUnlock->setGrayed(false);
			if(pMoveSubMenu)	pMoveSubMenu->setGrayed(false);

			// check each inventory dest if available
			if(pMoveToBag)		pMoveToBag->setGrayed(!invMngr.isInventoryAvailable(INVENTORIES::bag));
			for(i=0;i<MAX_INVENTORY_ANIMAL;i++)
			{
				if(pMoveToPa[i])	pMoveToPa[i]->setGrayed(!invMngr.isInventoryAvailable(
					(INVENTORIES::TInventory)(INVENTORIES::pet_animal+i)));
			}
		}

	}
};
REGISTER_ACTION_HANDLER( CHandlerItemMenuCheck, "item_menu_check" );

class CHandlerItemMenuDeactivate : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		// The waiter may exist here only if at window init item info was
		// incorrect, and it hasn't been updated since then, but that's only
		// place to remove it in that case
		getInventory().removeItemInfoWaiter(&ItemMenuInBagUpdater);
	}
};
REGISTER_ACTION_HANDLER( CHandlerItemMenuDeactivate, "item_menu_deactivate" );


// ***************************************************************************

class CHandlerItemMenuBaseCheck : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Get the ctrl sheet that launched this menu
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCS == NULL) return;
		INVENTORIES::TInventory		invId= (INVENTORIES::TInventory)pCS->getInventoryIndex();

		// Get the menu launched
		CInterfaceGroup	  *pMenu= dynamic_cast<CInterfaceGroup*>(pCaller);
		if(!pMenu)	return;

		// Get all needed text entries
		CViewTextMenu	*pDestroy = dynamic_cast<CViewTextMenu*>(pMenu->getView("destroy"));
		CViewTextMenu	*pLockUnlock = dynamic_cast<CViewTextMenu*>(pMenu->getView("lockunlock"));

		if (pCS->getLockedByOwner()) 
		{
			pLockUnlock->setHardText("uimUnlockItem");
			// Cannot destroy if locked by owner
			if (pDestroy) pDestroy->setActive(false);
		}
		else
		{
			pLockUnlock->setHardText("uimLockItem");
			if (pDestroy) pDestroy->setActive(true);
		}

	}
};
REGISTER_ACTION_HANDLER( CHandlerItemMenuBaseCheck, "item_menu_base_check" );


// ***************************************************************************
static void sendMsgUseItem(uint16 slot)
{
	if(!ClientCfg.Local)
	{
		CBitMemStream out;
		const string sMsg = "ITEM:USE_ITEM";

		if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			out.serial( slot );
			NetMngr.push(out);
		}
		else
			nlinfo("unknown message %s", sMsg.c_str());
	}
}

// ***************************************************************************
static void sendMsgStopUseXpCat( bool isRingCatalyser )
{
	if(!ClientCfg.Local)
	{
		CBitMemStream out;
		const string sMsg = "ITEM:STOP_USE_XP_CAT";

		if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			out.serial( isRingCatalyser );
			NetMngr.push(out);
		}
		else
			nlinfo("unknown message %s", sMsg.c_str());
	}
}


// ***************************************************************************
class CHandlerTeleportUse : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCS == NULL) return;

		// use the item
		sendMsgUseItem(uint16(pCS->getIndexInDB()));

		// Last loading is a teleport
		LoadingBackground = TeleportKamiBackground;
		const CItemSheet *pIS = pCS->asItemSheet();
		if ((pIS != NULL) && (pIS->Family == ITEMFAMILY::TELEPORT))
		{
			switch (pIS->Teleport.Type)
			{
				case TELEPORT_TYPES::NONE:
				case TELEPORT_TYPES::KAMI:
					LoadingBackground = TeleportKamiBackground;
				break;
				case TELEPORT_TYPES::KARAVAN:
					LoadingBackground = TeleportKaravanBackground;
				break;
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTeleportUse, "teleport_use" );

// ***************************************************************************
class CHandlerItemConsume : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCS == NULL) return;

		// use the item
		sendMsgUseItem(uint16(pCS->getIndexInDB()));
	}
};
REGISTER_ACTION_HANDLER( CHandlerItemConsume, "item_consume" );


// ***************************************************************************
class CHandlerValidateItemTextEdition : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string & /* sParams */)
	{
		CInterfaceItemEdition::getInstance()->validate();
	}
};
REGISTER_ACTION_HANDLER( CHandlerValidateItemTextEdition, "validate_edit_custom" );


// ***************************************************************************
class CHandlerItemTextDisplay : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		std::string const& windowName = sParams;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *pCSItem = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCSItem == NULL || windowName.empty()) 
			return;

		CInterfaceItemEdition::getInstance()->setCurrWindow(pCSItem, windowName, false);
	}
};
REGISTER_ACTION_HANDLER( CHandlerItemTextDisplay, "item_text_display" );

// ***************************************************************************

class CHandlerItemTextEdition : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		std::string const& windowName = sParams;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *pCSItem = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCSItem == NULL || windowName.empty()) 
			return;

		CInterfaceItemEdition::getInstance()->setCurrWindow(pCSItem, windowName, true);
	}
};
REGISTER_ACTION_HANDLER( CHandlerItemTextEdition, "item_text_edition" );

// ***************************************************************************
class CHandlerItemTextEditionClose : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string & /* sParams */)
	{
		CInterfaceItemEdition::getInstance()->setCurrWindow(NULL);
	}
};
REGISTER_ACTION_HANDLER( CHandlerItemTextEditionClose, "on_close_edit_custom" );

// ***************************************************************************
class CHandlerXpCatalyserUse : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (pCS == NULL) return;

		// use the item
		sendMsgUseItem(uint16(pCS->getIndexInDB()));
	}
};
REGISTER_ACTION_HANDLER( CHandlerXpCatalyserUse, "xp_catalyser_use" );


// ***************************************************************************
class CHandlerXpCatalyserStopUse : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// stop use the item
		sendMsgStopUseXpCat(false);
	}
};
REGISTER_ACTION_HANDLER( CHandlerXpCatalyserStopUse, "xp_catalyser_stop_use" );

// ***************************************************************************
class CHandlerRingXpCatalyserStopUse : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// stop use the item
		sendMsgStopUseXpCat(true);
	}
};
REGISTER_ACTION_HANDLER( CHandlerRingXpCatalyserStopUse, "ring_xp_catalyser_stop_use" );



