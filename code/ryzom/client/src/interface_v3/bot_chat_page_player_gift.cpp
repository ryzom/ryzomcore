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
#include "bot_chat_page_player_gift.h"
#include "interface_manager.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/action_handler.h"
#include "../net_manager.h"
#include "bot_chat_manager.h"
#include "bot_chat_page_all.h"
#include "bot_chat_page_mission_end.h"
#include "player_trade.h"

static const char *WIN_BOT_CHAT_PAGE_PLAYER_GIFT = "ui:interface:bot_chat_player_gift";

using NLMISC::CCDBNodeLeaf;

// *************************************************************************************
void CBotChatPagePlayerGift::begin()
{
	CBotChatPage::begin();
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// clear intro text
	NLGUI::CDBManager::getInstance()->getDbProp(BOT_CHAT_BASE_DB_PATH ":PLAYER_GIFT")->setValue32(0);

	// clear money proposal value
	CCDBNodeLeaf *moneyProposal = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("money_proposal"));
	if (moneyProposal) moneyProposal->setValue64(0);

	// clear 'accept' button
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:EXCHANGE:ACCEPTED")->setValue32(0);

	// Default is not validated
	PlayerGiftValidated= false;

	// open the window
	activateWindow(WIN_BOT_CHAT_PAGE_PLAYER_GIFT, true);

	// PlayerTrade is in "BotChatGift" mode
	PlayerTrade.BotChatGiftContext= true;
}

// *************************************************************************************
void CBotChatPagePlayerGift::end()
{
	// If the player gift was not validated, restore all items. else must not!
	if(!PlayerGiftValidated)
		PlayerTrade.restoreAllItems();
	activateWindow(WIN_BOT_CHAT_PAGE_PLAYER_GIFT, false);

	// PlayerTrade is no more in "BotChatGift" mode
	PlayerTrade.BotChatGiftContext= false;
}


/////////////////////
// ACTION HANDLERS //
/////////////////////
class CHandlerValidPlayerGift : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		NLMISC::CBitMemStream out;
		if (GenericMsgHeaderMngr.pushNameToStream("BOTCHAT:VALIDATE_PLAYER_GIFT", out))
		{
			NetMngr.push(out);
		}
		else
		{
			nlwarning("bad msg");
		}

		// The player gift is validated
		CBotChatPagePlayerGift	*bpGift= dynamic_cast<CBotChatPagePlayerGift*>(CBotChatManager::getInstance()->getCurrPage());
		if(bpGift)
			bpGift->PlayerGiftValidated= true;

		// go to the end mission screen
		// CBotChatManager::getInstance().setCurrPage(BotChatPageAll->MissionEnd);
		CBotChatManager::getInstance()->setCurrPage(NULL);
	}
};
REGISTER_ACTION_HANDLER( CHandlerValidPlayerGift, "valid_player_gift");
