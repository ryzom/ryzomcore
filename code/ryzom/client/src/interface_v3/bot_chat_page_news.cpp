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
#include "bot_chat_page_news.h"
#include "interface_manager.h"
#include "nel/gui/interface_group.h"

static const char *WIN_BOT_CHAT_PAGE_NEWS = "ui:interface:bot_chat_news";

// ***************************************************************************
void CBotChatPageNews::begin()
{
	CBotChatPage::begin();
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// clear intro text
	NLGUI::CDBManager::getInstance()->getDbProp(BOT_CHAT_BASE_DB_PATH ":NEWS")->setValue32(0);
	//
	activateWindow(WIN_BOT_CHAT_PAGE_NEWS, true);
}

// ***************************************************************************
void CBotChatPageNews::end()
{
	activateWindow(WIN_BOT_CHAT_PAGE_NEWS, false);
}
