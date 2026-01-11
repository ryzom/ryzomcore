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
#include "bot_chat_page_all.h"
//
#include "bot_chat_page_create_guild.h"
#include "bot_chat_page_player_gift.h"
#include "bot_chat_page_trade.h"
#include "bot_chat_page_news.h"
#include "bot_chat_page_mission.h"
#include "bot_chat_page_dynamic_mission.h"
#include "bot_chat_page_mission_end.h"
#include "bot_chat_page_ring_sessions.h"
//
#include "bot_chat_manager.h"
#include "interface_manager.h"
#include "../global.h"


CBotChatPageAll *BotChatPageAll = NULL;

// *******************************************************************
CBotChatPageAll::CBotChatPageAll()
{
	Trade			= NULL;
	PlayerGift		= NULL;
	ChooseMission	= NULL;
	CreateGuild		= NULL;
	News			= NULL;
	MissionEnd		= NULL;
	DynamicMission  = NULL;
	RingSessions	= NULL;
}

// *******************************************************************
CBotChatPageAll::~CBotChatPageAll()
{
	delete Trade;
	delete PlayerGift;
	delete ChooseMission;
	delete CreateGuild;
	delete News;
	delete MissionEnd;
	delete DynamicMission;
	delete RingSessions;
}

// *******************************************************************
void CBotChatPageAll::init()
{
	Trade			= new CBotChatPageTrade;
	PlayerGift		= new CBotChatPagePlayerGift;
	ChooseMission	= new CBotChatPageMission;
	CreateGuild		= new CBotChatPageCreateGuild;
	News			= new CBotChatPageNews;
	MissionEnd		= new CBotChatPageMissionEnd;
	DynamicMission	= new CBotChatPageDynamicMission;
	RingSessions	= new CBotChatPageRingSessions;
	Trade->init();
	PlayerGift->init();
	ChooseMission->init();
	CreateGuild->init();
	News->init();
	MissionEnd->init();
	DynamicMission->init();
	RingSessions->init();
}

// ***************************************************************************
void CBotChatPageAll::initAfterConnectionReady()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// After Connection:Ready is sent, and after loadConfig is done, sent to server the botchat filters
	nlassert(ConnectionReadySent && pIM->isConfigLoaded());

	// reset item part and item type filters (yes, this should not be saved in UI:SAVE.....)
	Trade->resetItemPartAndTypeFilters();

	// don't need to reset list since currPage should be NULL
	Trade->sendCurrentBuyFilterToServer(false);
}


#if !FINAL_VERSION

////////////////////////////////////
// GO DIRECLTY TO A BOT CHAT PAGE //
////////////////////////////////////
NLMISC_COMMAND( bcPage, "Go to the given page in the bot chat", "")
{
	if (args.empty()) return false;
	uint page;
	NLMISC::fromString(args[0], page);
	CBotChatManager::getInstance()->endDialog();
	CBotChatManager::getInstance()->incrementSessionID();
	switch(page)
	{
		case 0:
			BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::Money);
			BotChatPageAll->Trade->setBuyOnly(false);
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
		return true;
		case 1: CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->PlayerGift); return true;
		case 2:
			BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::Mission);
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
			return true;
		case 3: CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->CreateGuild); return true;
		case 4: CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->News); return true;
		case 5: CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->MissionEnd); return true;
		case 6: CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->DynamicMission); return true;
		case 7:
			BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::SkillPoints);
			BotChatPageAll->Trade->setBuyOnly(true);
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
			return true;
		case 8:
			BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::ZCCharge);
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
			return true;
		case 9:
			BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::RMBuy);
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
			return true;
		case 10:
			BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::RMUpgrade);
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
			return true;
		case 11:
			BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::Building);
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
			return true;
		case 12:
			BotChatPageAll->Trade->setBuyOnly(true);
			BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::MoneyGuildXP);
			BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatGuildOptions"));
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
			return true;
		case 13:
			BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::MoneyFactionPoints);
			BotChatPageAll->Trade->setBuyOnly(true);
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
			return true;
		case 14:
			BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::GuildMoney);
			BotChatPageAll->Trade->setBuyOnly(true);
			BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatGuildOptions"));
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
			return true;
		case 15:
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->RingSessions);
			return true;
	}
	return false;
}

#endif

