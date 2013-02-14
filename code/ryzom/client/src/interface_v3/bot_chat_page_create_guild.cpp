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
#include "bot_chat_page_create_guild.h"
#include "interface_manager.h"
#include "guild_manager.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/group_editbox.h"
#include "dbctrl_sheet.h"
#include "bot_chat_manager.h"
#include "bot_chat_page_all.h"
#include "../net_manager.h"

using namespace std;

static const char *WIN_BOT_CHAT_PAGE_CREATE_GUILD = "ui:interface:bot_chat_create_guild";

// ***************************************************************************
void CBotChatPageCreateGuild::begin()
{
	CBotChatPage::begin();
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// clear intro text
	NLGUI::CDBManager::getInstance()->getDbProp(BOT_CHAT_BASE_DB_PATH ":CREATE_GUILD")->setValue32(0);
	// show the dialog
	activateWindow(WIN_BOT_CHAT_PAGE_CREATE_GUILD, true);
}

// ***************************************************************************
void CBotChatPageCreateGuild::end()
{
	activateWindow(WIN_BOT_CHAT_PAGE_CREATE_GUILD, false);
}

// ***************************************************************************
class CHandlerGuildCreate : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string guildNameWin = getParam(Params, "guild");
		string IconWin = getParam(Params, "icon");
		string guildDescWin = getParam(Params, "desc");

		CGroupEditBox *pGEB = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(guildNameWin));
		if (pGEB == NULL) return;

		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(CWidgetManager::getInstance()->getElementFromId(IconWin));
		if (pCS == NULL) return;

		CGroupEditBox *pDesc = dynamic_cast<CGroupEditBox*>(CWidgetManager::getInstance()->getElementFromId(guildDescWin));

		ucstring guildName = pGEB->getInputString();

		ucstring guildDesc;
		if (pDesc != NULL) guildDesc = pDesc->getInputString();

		uint64 icon = CGuildManager::iconMake((uint8)pCS->getGuildBack(), (uint8)pCS->getGuildSymbol(),
								pCS->getInvertGuildSymbol(), pCS->getGuildColor1(), pCS->getGuildColor2());

		const string msgName = "GUILD:CREATE";
		NLMISC::CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		{
			out.serial( guildName );
			out.serial( icon );
			out.serial( guildDesc );
			NetMngr.push(out);
		}
		//CBotChatManager::getInstance()->setCurrPage(NULL);
	}
};
REGISTER_ACTION_HANDLER (CHandlerGuildCreate, "enter_guild_creation");





















