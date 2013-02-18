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
#include "bot_chat_page_mission_end.h"
#include "interface_manager.h"
#include "nel/gui/interface_group.h"
#include "bot_chat_manager.h"
#include "interface_manager.h"
#include "../client_cfg.h"


static const char *WIN_BOT_CHAT_PAGE_MISSION_END = "ui:interface:bot_chat_mission_end";

// Context help
extern void contextHelp (const std::string &help);

// ***************************************************************************
void CBotChatPageMissionEnd::begin()
{
	return;
	/*
	CBotChatPage::begin();
	CInterfaceManager *im = CInterfaceManager::getInstance();
	uint flags = CBotChatManager::getInstance().getChosenMissionFlags();
	// reward text
	CInterfaceGroup *ig = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(WIN_BOT_CHAT_PAGE_MISSION_END));
	if (!ig) return;
	CInterfaceGroup *rewardText = ig->getGroup("reward_text");
	if (rewardText)
	{
		if ((flags & 1) || ClientCfg.Local) // is reward text needed
		{
			rewardText->setActive(true);
			NLGUI::CDBManager::getInstance()->getDbProp(BOT_CHAT_BASE_DB_PATH ":MISSION_END_REWARD")->setValue32(0);
		}
		else
		{
			rewardText->setActive(false);
		}
	}
	CInterfaceGroup *rewardSlots = ig->getGroup("reward_slots");
	if (rewardSlots)
	{
		rewardSlots->setActive(((flags & 2) != 0) || ClientCfg.Local);
	}
	//
	NLGUI::CDBManager::getInstance()->getDbProp(BOT_CHAT_BASE_DB_PATH ":MISSION_END")->setValue32(0);
	//
	ig->setActive(true);

	// Context help
	if (flags & 4)
		contextHelp ("inventory");
	*/
}

// ***************************************************************************
void CBotChatPageMissionEnd::end()
{
	activateWindow(WIN_BOT_CHAT_PAGE_MISSION_END, false);
}





















