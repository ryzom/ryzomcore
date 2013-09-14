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

#include <sstream>

// Interface includes
#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "action_handler_base.h"
#include "action_handler_misc.h"
#include "bot_chat_manager.h"
#include "bot_chat_page_all.h"
#include "bot_chat_page_news.h"
#include "bot_chat_page_create_guild.h"
#include "bot_chat_page_mission.h"
#include "bot_chat_page_mission_end.h"
#include "bot_chat_page_trade.h"
#include "bot_chat_page_player_gift.h"
#include "bot_chat_page_dynamic_mission.h"
#include "bot_chat_page_ring_sessions.h"
#include "dbctrl_sheet.h"
#include "nel/gui/ctrl_sheet_selection.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/group_menu.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "inventory_manager.h"
#include "guild_manager.h"
#include "../net_manager.h"
#include "interface_ddx.h"
#include "nel/gui/group_tree.h"
#include "group_map.h"
#include "nel/gui/view_bitmap.h"
#include "action_handler_tools.h"
#include "../connection.h"
#include "../client_chat_manager.h"

// Game specific includes
#include "../motion/user_controls.h"
#include "../entities.h"
#include "../misc.h"
#include "../sheet_manager.h"
#include "../actions_client.h"
#include "people_interraction.h"
#include "../game_context_menu.h"
#include "../sound_manager.h"
#include "../far_tp.h"
#include "nel/gui/interface_link.h"
#include "../npc_icon.h"

// Game Share
#include "game_share/character_summary.h"
#include "game_share/brick_types.h"
#include "game_share/seeds.h"
#include "game_share/entity_types.h"
#include "game_share/inventories.h"
//#include "game_share/sheath.h"
//#include "game_share/jobs.h"
#include "game_share/animals_orders.h"
#include "game_share/animal_status.h"
#include "game_share/animal_type.h"
#include "game_share/interface_flags.h"
#include "game_share/slot_equipment.h"
#include "game_share/bot_chat_types.h"
#include "game_share/constants.h"
#include "game_share/scores.h"

// Game Config
#include "nel/gui/dbgroup_combo_box.h"
#include "nel/gui/ctrl_button.h"
#include "../global.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;
using namespace BRICK_TYPE;

extern NL3D::UDriver	*Driver;
extern NLMISC::CLog		g_log;
extern bool				ShowInterface;
extern bool				ShowHelp;
extern uint8			PlayerSelectedSlot;
extern bool				IsInRingSession;

// Context help
extern void contextHelp (const std::string &help);

extern CClientChatManager ChatMngr;

void beastOrder (const std::string &orderStr, const std::string &beastIndexStr, bool confirmFree = true);


string convertLanguageIntToLanguageCode(sint val)
{
	switch(val)
	{
	default:
	case 0: return "en"; break;
	case 1: return "fr"; break;
	case 2: return "de"; break;
	case 3: return "ru"; break;
	case 4: return "es"; break;
	}
}

/**********************************************************************************************************
*																										  *
*										GAME CONTEXT MENU handlers										  *
*																										  *
***********************************************************************************************************/


// Does the player know this skill?
static	bool	playerKnowSkill( SKILLS::ESkills e)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	string sPath = "SERVER:CHARACTER_INFO:SKILLS:" + toStringEnum( e ) + ":SKILL";
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sPath,false);
	if ((pNL != NULL) && (pNL->getValue64() > 0))
		return true;
	else
		return false;
}


// ***************************************************************************
// GCM Activation : called when activate the game context menu
// ***************************************************************************

class CHandlerActiveGameContextMenu : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		// Invalidate all lines
		CGroupMenu *pGM = dynamic_cast<CGroupMenu*>(pCaller);
		if (pGM == NULL) return;

		// Id names of the lines in the game_context_menu.xml ...
		static const char *sIdNames[] = { "talk", "use", "lift", "look", "attack", "invit", "exchange",
										"mount", "train" };
		const uint32 numOptions = sizeof(sIdNames) / sizeof(sIdNames[0]);

		uint32 i;
		CViewTextMenu *pVTM;
		const string sMenuPath = "ui:interface:game_context_menu";

		for (i = 0; i < numOptions; ++i)
		{
			pVTM = dynamic_cast<CViewTextMenu*>(pGM->getElement(sMenuPath+":"+sIdNames[i]));
			if (pVTM != NULL) pVTM->setGrayed(true);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerActiveGameContextMenu, "active_game_context_menu");

// ***************************************************************************
// GCM Talk
// ***************************************************************************

class CHandlerContextTalk : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
		if (selection && selection->Type == CEntityCL::Player)
		{
			ucstring name = CEntityCL::removeTitleAndShardFromName(selection->getEntityName());
			if (name.empty()) return;
			CAHManager::getInstance()->runActionHandler("enter_tell", pCaller, "player=" + name.toString());
		}
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextTalk, "context_talk");




// ***************************************************************************
// sendBotChatStart
// helper increment session id and send session id to the server with specified message
// ***************************************************************************
static void sendBotChatStart(const string &msgName)
{
	CBotChatManager::getInstance()->incrementSessionID();
	CBitMemStream out;
	string msg = "BOTCHAT:" + msgName;;
	if(GenericMsgHeaderMngr.pushNameToStream(msg, out))
	{
		uint16 session = CBotChatManager::getInstance()->getSessionID();
		out.serial(session);
		NetMngr.push(out);
	}
	else
	{
		nlwarning("msg %s does not exist.", msg.c_str());
	}
}


// ***************************************************************************
// GCM Ring Sessions
// ***************************************************************************
class CHandlerContextRingSessions : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		//	do not send start message to server here (not really a bot chage page, but should dissapear
		// when player go away)

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId("ui:interface:ring_sessions");

		// check if selection is a Ring terminal
		CEntityCL * selection = EntitiesMngr.entity(UserEntity->selection());
		if (selection && selection->sheetId()==NLMISC::CSheetId("object_karavan_device_neutrl_sel.creature"))
		{
			CVectorD vect1 = selection->pos();
			CVectorD vect2 = UserEntity->pos();

			double distanceSquare = pow(vect1.x-vect2.x,2) + pow(vect1.y-vect2.y,2);

			if(distanceSquare > MaxTalkingDistSquare)
			{
				UserEntity->moveTo(UserEntity->selection(), 3.0, CUserEntity::WebPage);
				if (pIE->getActive())
				{
					CBotChatManager::getInstance()->setCurrPage(NULL);
				}
			}
			else
			{
				if (!pIE->getActive())
				{
					CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->RingSessions);
				}
			}
		}
		else
		{
			if (pIE != NULL) pIE->setActive(false);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextRingSessions, "context_ring_sessions");


// ***************************************************************************
// GCM Trade Item (with bot)
// ***************************************************************************
class CHandlerContextTradeItem : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_ITEM");
		BotChatPageAll->Trade->setBuyOnly(false);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::Money);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatTrade"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);

		// Context help
		contextHelp ("inventory");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextTradeItem, "context_trade_item");

// ***************************************************************************
// GCM Trade Teleport (with bot)
// ***************************************************************************
class CHandlerContextTradeTeleport : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_TELEPORT");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::Money);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatTeleport"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
		// Context help
		contextHelp ("inventory");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextTradeTeleport, "context_trade_teleport");

// ***************************************************************************
// GCM Trade Faction items/bricks/named items/bonuses (with bot)
// ***************************************************************************
class CHandlerContextTradeFaction : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_FACTION");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::MoneyFactionPoints);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatFaction"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
		// Context help
		contextHelp ("inventory");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextTradeFaction, "context_trade_faction");

// ***************************************************************************
// GCM Trade Cosmetic (with bot)
// ***************************************************************************
class CHandlerContextTradeCosmetic : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_ITEM");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::Money);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatCosmetic"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
		// Context help
		contextHelp ("inventory");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextTradeCosmetic, "context_trade_cosmetic");

// ***************************************************************************
// GCM Trade Guild Options (with bot) (le concierge)
// ***************************************************************************
class CHandlerContextTradeGuildOptions : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_GUILD_OPTIONS");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::MoneyGuildXP);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatGuildOptions"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
		// Context help
		contextHelp ("inventory");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextTradeGuildOptions, "context_trade_guild_options");

// ***************************************************************************
// GCM Trade Outpost Building (with bot) (the building itself)
// ***************************************************************************
class CHandlerContextTradeOutpostBuilding : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_GUILD_OPTIONS");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::GuildMoney);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatGuildOptions"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
		// Context help
		contextHelp ("inventory");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextTradeOutpostBuilding, "context_trade_outpost_building");

// ***************************************************************************
// GCM Trade phrase from the guild role master
// ***************************************************************************
class CHandlerContextTradeGuildRoleMaster : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_GUILD_RESEARCH");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::GuildXP);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiBotChatGuildRoleMaster"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
		// Context help
		contextHelp ("inventory");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextTradeGuildRoleMaster, "context_trade_guild_role_master");

// ***************************************************************************
// GCM Trade Skill (with bot)
// ***************************************************************************
class CHandlerContextTradeSkill  : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_SKILL");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::SkillPoints);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiLearn"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextTradeSkill, "context_trade_skill");

// ***************************************************************************
// GCM Trade Pacts (with bot)
// ***************************************************************************
class CHandlerContextTradePact  : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_PACT");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::SkillPoints);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiLearn"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextTradePact, "context_trade_pact");

// ***************************************************************************
// GCM Trade Phrases (with bot)
// ***************************************************************************
class CHandlerContextTradePhrase  : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_TRADE_ACTION");
		BotChatPageAll->Trade->setBuyOnly(true);
		BotChatPageAll->Trade->setBuyMean(CBotChatPageTrade::SkillPoints);
		BotChatPageAll->Trade->setTitle(NLMISC::CI18N::get("uiLearn"));
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->Trade);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextTradePhrase, "context_trade_phrase");

// ***************************************************************************
// GCM Choose mission
// ***************************************************************************
class CHandlerContextChooseMission : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_CHOOSE_MISSION");
		BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::Mission);
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextChooseMission, "context_choose_mission");

// ***************************************************************************
// GCM Create guild
// ***************************************************************************
class CHandlerContextCreateGuild : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CBotChatManager::getInstance()->incrementSessionID();

		sendMsgToServer("BOTCHAT:START_CREATE_GUILD");

		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->CreateGuild);

		// Context help
		contextHelp ("guild");
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextCreateGuild, "context_create_guild");


// ***************************************************************************
// GCM Mission option
// ***************************************************************************
class CHandlerContextMissionOption : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		std::string id = getParam(sParams, "id");
		sint intId;
		if (!fromString(id, intId)) return;
		//nlinfo("intId = %d", intId);
		CBotChatManager::getInstance()->incrementSessionID();
		//
		CInterfaceManager *im = CInterfaceManager::getInstance();
		// get flags
		std::string playerGiftNeededDbPath = toString("LOCAL:TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%d:PLAYER_GIFT_NEEDED", intId);
		CCDBNodeLeaf *playerGiftNeeded = NLGUI::CDBManager::getInstance()->getDbProp(playerGiftNeededDbPath, false);
		if (!playerGiftNeeded) return;
		//
		//CBotChatManager::getInstance()->setChosenMissionFlags((uint) missionFlags->getValue8());
		// a gift from player is required
		sendMsgToServer("BOTCHAT:CONTINUE_MISSION", (uint8)intId);

		if (playerGiftNeeded->getValue8())
		{
			CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->PlayerGift);
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextMissionOption, "mission_option");


// ***************************************************************************
// GCM Missions
// ***************************************************************************
/*
class CHandlerContextMissions : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		// if there is only a single page available for missions, go to that page directly
		CCDBNodeLeaf *progs = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TARGET:PROGRAMMES", false);
		if (!progs)
		{
			nlwarning("<CHandlerContextMissions::execute> can't retrieve programs.");
			return;
		}
		bool missionsToDo = (progs->getValue32() & (1 << BOTCHATTYPE::AvailableMissionFamily)) != 0;
		bool missionsToContinue = (progs->getValue32() & (1 << BOTCHATTYPE::CompletedMissionFamily)) != 0;
		bool missionsTofinish = (progs->getValue32() & (1 << BOTCHATTYPE::ContinuedMissionFamily)) != 0;
		if (missionsToDo && !missionsToContinue && !missionsTofinish)
		{
			startBotChatAtPage(BOTCHATTYPE::AvailableMissions);
		}
		else if (!missionsToDo && missionsToContinue && !missionsTofinish)
		{
			startBotChatAtPage(BOTCHATTYPE::MissionProgress);
		}
		else if (!missionsToDo && !missionsToContinue && missionsTofinish)
		{
			startBotChatAtPage(BOTCHATTYPE::CompletedMissions);
		}
		else
		{
			startBotChatAtPage(BOTCHATTYPE::MissionStartPage);
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextMissions, "context_missions");
*/

// ***************************************************************************
// GCM Available missions
// ***************************************************************************
/*
class CHandlerContextAvailableMissions : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		startBotChatAtPage(BOTCHATTYPE::AvailableMissions); // missions list page. If only one option, go to this page ?
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextAvailableMissions, "context_available_missions");

// ***************************************************************************
// GCM Constinue missions
// ***************************************************************************
class CHandlerContextContinueMissions : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		startBotChatAtPage(BOTCHATTYPE::MissionProgress); // missions list page. If only one option, go to this page ?
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextContinueMissions, "context_continue_missions");

// ***************************************************************************
// GCM Finish missions
// ***************************************************************************
class CHandlerContextFinishMissions : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		startBotChatAtPage(BOTCHATTYPE::CompletedMissions); // missions list page. If only one option, go to this page ?
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextFinishMissions, "context_finish_missions");
*/


// ***************************************************************************
// GCM Attack
// ***************************************************************************

class CHandlerContextAttack : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{

		if( !UserEntity->canEngageCombat() )
			return;

		// Move to the current selection and attack.
		UserEntity->moveToAttack();
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextAttack, "context_attack");

// ***************************************************************************
// GCM Duel
// ***************************************************************************

class CHandlerContextDuel : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendMsgToServer("DUEL:ASK");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextDuel, "context_duel");

// ***************************************************************************
// GCM UnDuel
// ***************************************************************************

class CHandlerContextUnDuel : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendMsgToServer("DUEL:ABANDON");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextUnDuel, "context_unduel");

// ***************************************************************************
// GCM PVP Challenge
// ***************************************************************************

class CHandlerContextPVPChallenge : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendMsgToServer("PVP_CHALLENGE:ASK");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextPVPChallenge, "context_pvp_challenge");

// ***************************************************************************
// GCM Un_PVP_Challenge
// ***************************************************************************

class CHandlerContextUnPVPChallenge : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendMsgToServer("PVP_CHALLENGE:ABANDON");
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextUnPVPChallenge, "context_unpvp_challenge");

// ***************************************************************************
// GCM Invit
// ***************************************************************************

class CHandlerContextInvit : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Game Specific Code
		sendMsgToServer("TEAM:JOIN_PROPOSAL");
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextInvit, "context_invit");

// ***************************************************************************
// GCM Guild Invit
// ***************************************************************************

class CHandlerContextGuildInvit : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Game Specific Code
		sendMsgToServer("GUILD:INVITATION");
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextGuildInvit, "context_guild_invit");

// ***************************************************************************
// GCM Add to friend list
// ***************************************************************************
class CHandlerContextAddToFriendList : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CLFECOMMON::TCLEntityId trader = UserEntity->selection();
		if (trader != CLFECOMMON::INVALID_SLOT)
		{
			CEntityCL *entity = EntitiesMngr.entity(trader);
			if (entity)
			{
				ucstring playerName = entity->getEntityName();
				if (!playerName.empty())
				{
					PeopleInterraction.askAddContact(playerName, &PeopleInterraction.FriendList);
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextAddToFriendList, "context_add_to_friend_list");


// ***************************************************************************
// chooseSheath
// ***************************************************************************
/*
static void chooseSheath (ITEMFAMILY::EItemFamily eIF, string sAllSkills)
{
	// Choose right sheath
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNLwrite = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("ui_set_active"));
	CCDBNodeLeaf *pNLread = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("set_nb"));
	sint32 nNbSheath = (sint32)pNLread->getValue64();
	if (nNbSheath == 0) return;
	pNLread = NLGUI::CDBManager::getInstance()->getDbProp(CWidgetManager::getInstance()->getParser()->getDefine("set_active"));
	sint32 nActiveSheath = (sint32)pNLread->getValue64();
	bool bFound = false;
	for (sint32 i = 0; i < ((nNbSheath/2)+1); ++i)
	{
		for (sint32 j = 0; j < 2; ++j)
		{
			sint32 nSheathToTest;
			if (j == 0)
				nSheathToTest = nActiveSheath + i;
			else
				nSheathToTest = nActiveSheath - i;
			while (nSheathToTest < INVENTORIES::sheath) nSheathToTest += nNbSheath;
			while (nSheathToTest >= (INVENTORIES::sheath+nNbSheath)) nSheathToTest -= nNbSheath;

			string sPath;
			sPath = CWidgetManager::getInstance()->getParser()->getDefine("set_base") + ":" + NLMISC::toString(nSheathToTest) + ":" + CWidgetManager::getInstance()->getParser()->getDefine("set_r") + ":SHEET";
			pNLread = NLGUI::CDBManager::getInstance()->getDbProp(sPath);
			sint32 sheetid = (sint32)pNLread->getValue64();
			CItemSheet *pIS = dynamic_cast<CItemSheet *>(SheetMngr.get(CSheetId(sheetid)));
			if (pIS != NULL)
			{
				if (pIS->Family == eIF)
				{
					// YOYO: Fast search. important because skill_list is too big for "training" buton case
					string skillList= IActionHandler::getParam (sAllSkills, "skill_list");
					if( !skillList.empty() )
					{
						std::vector<string>	strList;
						NLMISC::splitString(skillList, ",", strList);
						for(uint k=0;k<strList.size();k++)
						{
							static string sSkillToTrain;
							sSkillToTrain= strList[k];
							// remove \n,\t and spaces
							while( !sSkillToTrain.empty() &&
								(sSkillToTrain[0]==' ' || sSkillToTrain[0]=='\n' || sSkillToTrain[0]=='\t') )
								sSkillToTrain.erase(0, 1);
							// If the tool skill match the skill filter, and if the player know this skill => OK!
							if (pIS->Tool.Skill == SKILLS::toSkill(sSkillToTrain) && playerKnowSkill(pIS->Tool.Skill) )
							{
								bFound = true;
								pNLwrite->setValue64(nSheathToTest);
								break;
							}
						}
					}
					if (bFound) break;
				}
			}
		}
		if (bFound) break;
	}
}
*/


// ***************************************************************************
// GCM Exchange
// ***************************************************************************

class CHandlerContextExchange : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Game Specific Code
		sendMsgToServer("EXCHANGE:PROPOSAL");
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextExchange, "context_exchange");

// ***************************************************************************
// GCM Free look
// ***************************************************************************
class CHandlerContextFreeLook : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Free Look Mode Activated.
		UserControls.startFreeLook();
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextFreeLook, "context_free_look");

// ***************************************************************************
// GCM Move
// ***************************************************************************
class CHandlerMove : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Autowalk Mode Activated.
		UserControls.autowalkState(true);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerMove, "context_move");

// ***************************************************************************
// GCM Stop
// ***************************************************************************
class CHandlerStop : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Autowalk Mode Activated.
		UserControls.autowalkState(false);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerStop, "context_stop");

// ***************************************************************************
class CHandlerExitFreeLook : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Deactivate free look mode
		UserControls.stopFreeLook();
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerExitFreeLook, "exit_free_look");

// ***************************************************************************
// GCM Loot Action
// ***************************************************************************
class CHandlerContextLootAction : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		NetMngr.pushPickup(UserEntity->selection(), LHSTATE::LOOTABLE);

		// For loot open directly temporary inventory
		CTempInvManager::getInstance()->open(TEMP_INV_MODE::Loot);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextLootAction, "context_loot");

// ***************************************************************************
// GCM Quartering Action
// ***************************************************************************
class CHandlerContextHarvestAction : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		NetMngr.pushPickup(UserEntity->selection(), LHSTATE::HARVESTABLE);
		// For quartering and forage open directly temporary inventory
		CTempInvManager::getInstance()->open(TEMP_INV_MODE::Quarter);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextHarvestAction, "context_quartering");

// ***************************************************************************
// GCM Forage Action
// ***************************************************************************
class	CHandlerContextForageExtract : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if (UserEntity->selection())
		{
			CEntityCL *pSel = EntitiesMngr.entity(UserEntity->selection());
			if (pSel != NULL)
				if (pSel->isForageSource())
					UserEntity->moveToExtractionPhrase(UserEntity->selection(), 2.0f, ~0, ~0, true);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextForageExtract, "context_extract_rm");


// ***************************************************************************
// GCM Quit Team
// ***************************************************************************

class CHandlerContextQuitTeam : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		// directly launch the quit_team AH.
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler("quit_team",pCaller, sParams);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextQuitTeam, "context_quit_team");


// ***************************************************************************
// GCM Quit Guild
// ***************************************************************************

class CHandlerContextQuitGuild : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Ask if ok before.
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQQuitGuild"), "do_quit_guild");
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerContextQuitGuild, "context_quit_guild");


// If Ok.
class CHandlerDoQuitGuild : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Create the message for the server to execute a phrase.
		sendMsgToServer("GUILD:QUIT");
		CGuildManager::getInstance()->closeAllInterfaces();

		if (PeopleInterraction.TheUserChat.Filter.getTargetGroup() == CChatGroup::guild)
			ChatMngr.updateChatModeAndButton(CChatGroup::say);
	}
};
REGISTER_ACTION_HANDLER( CHandlerDoQuitGuild, "do_quit_guild");


// ***************************************************************************
// GCM Disengage
// ***************************************************************************

class CHandlerContextDisengage : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Disengage
		UserEntity->disengage();
	}
protected:

};
REGISTER_ACTION_HANDLER( CHandlerContextDisengage, "context_disengage");

// ***************************************************************************
// GCM Mount
// ***************************************************************************

class CHandlerContextMount : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CEntityCL *sel = EntitiesMngr.entity(UserEntity->selection());
		if (sel == NULL) return;

		// Game Specific Code
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Look through Database the index of the mount selected
		for (uint32 i = 0; i < MAX_INVENTORY_ANIMAL; i++)
		{
			CCDBNodeLeaf *uidProp = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:UID", i), false);
			CCDBNodeLeaf *typeProp = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:TYPE", i), false);
			if ((uidProp != NULL) && (uidProp->getValue32() == (sint32)sel->dataSetId()) &&
				(typeProp != NULL) && (typeProp->getValue32() == ANIMAL_TYPE::Mount))
			{
				beastOrder("mount", toString(i+1)); // why +1 ? : dixit sendAnimalCommand in EGS : index 0 = all animals, 1 = animal 0 etc
				CAHManager::getInstance()->runActionHandler("animal_target", NULL, toString(i+1));
				UserEntity->moveTo(UserEntity->selection(),2.0,CUserEntity::None);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextMount, "context_mount");

// ***************************************************************************
// GCM Unseat
// ***************************************************************************

class CHandlerContextUnseat : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Game Specific Code
		beastOrder("unmount", "0");

		// Set the database in local.
		if(ClientCfg.Local)
		{
			// Prepare the database to unseat
			IngameDbMngr.setProp("Entities:E" + toString(UserEntity->slot())      + ":P" + toString(CLFECOMMON::PROPERTY_MODE),              MBEHAV::NORMAL);
			IngameDbMngr.setProp("Entities:E" + toString(UserEntity->slot())      + ":P" + toString(CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID), CLFECOMMON::INVALID_SLOT);
			IngameDbMngr.setProp("Entities:E" + toString(UserEntity->mount()) + ":P" + toString(CLFECOMMON::PROPERTY_MODE),              MBEHAV::NORMAL);
			IngameDbMngr.setProp("Entities:E" + toString(UserEntity->mount()) + ":P" + toString(CLFECOMMON::PROPERTY_RIDER_ENTITY_ID),   CLFECOMMON::INVALID_SLOT);
			// Read the database to unseat.
			CEntityCL *mount = EntitiesMngr.entity(UserEntity->mount());
			if(mount)
			{
				mount->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_MODE);
				mount->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_RIDER_ENTITY_ID);
			}
			UserEntity->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_MODE);
			UserEntity->updateVisualProperty(NetMngr.getCurrentServerTick()+10, CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID);
		}
	}
protected:

};
REGISTER_ACTION_HANDLER( CHandlerContextUnseat, "context_unseat");


// ***************************************************************************
class CHandlerContextWebPage : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// start the npc web page
		CLuaManager::getInstance().executeLuaScript("game:startNpcWebPage()", true);
	}
};
REGISTER_ACTION_HANDLER( CHandlerContextWebPage, "context_web_page");




// ***************************************************************************
// GCM Mission option
// ***************************************************************************
class CHandlerContextMissionRing : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		std::string id = getParam(sParams, "id");
		sint idInDb;
		if (!fromString(id, idInDb)) return;

		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get ID
		CCDBNodeLeaf *dbMissionId = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:TARGET:CONTEXT_MENU:MISSION_RING:%d:ID", idInDb), false);
		if (!dbMissionId)
			return;
		uint32	missionId= dbMissionId->getValue32();

		// send msg
		sendMsgToServer("RING_MISSION:MISSION_RING_SELECT", (uint32)missionId);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextMissionRing, "mission_ring");


/**********************************************************************************************************
*																										  *
*											game handlers       										  *
*																										  *
***********************************************************************************************************/


// ***************************************************************************
//quit the game
class CAHQuitGame : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		/* todo game_exit
		game_exit = true;
		nlinfo("User Request to Quit the game"); */
	}
};
REGISTER_ACTION_HANDLER( CAHQuitGame, "quit_game");


// ***************************************************************************
//quit the game
class CAHQuitRyzom : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// If we are not connected, quit now
		if((!ConnectionReadySent) && (!FarTP.isLeavingEGS()))
		{
			game_exit = true;
			ryzom_exit = true;
			nlinfo("User Request to Quit ryzom");
		}
		else
		{
			// Don't quit but wait for server Quit
			game_exit_request = true;
			ryzom_exit_request = true;

			const string msgName = "CONNECTION:CLIENT_QUIT_REQUEST";
			CBitMemStream out;
			nlverify(GenericMsgHeaderMngr.pushNameToStream(msgName, out));
			bool bypassDisconnectionTimer = FarTP.isFastDisconnectGranted() && (!IsInRingSession); // no need on a ring shard, as it's very short
			out.serial(bypassDisconnectionTimer);
			FarTP.writeSecurityCodeForDisconnection(out); // must always be written because of msg.xml (or could have a special handler in the FS)
			NetMngr.push(out);
			//nlinfo("impulseCallBack : %s sent", msgName.c_str());
		}
	}
};
REGISTER_ACTION_HANDLER( CAHQuitRyzom, "quit_ryzom");

// ***************************************************************************
//paying account for Free Trial
class CAHPayingAccount : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
    {
		paying_account_request = FreeTrial;
		if(!FreeTrial)
		{
			 CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			 CAHManager::getInstance()->runActionHandler("quit_ryzom", NULL);
		}
	}
};
REGISTER_ACTION_HANDLER( CAHPayingAccount, "paying_account");

// ***************************************************************************
//force quit the game
class CAHQuitRyzomNow : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if(!paying_account_already_request && FreeTrial)
		{
			paying_account_request = true;
			game_exit_after_paying_account_request = true;
		}
		else
		{
			game_exit = true;
			ryzom_exit = true;
		}
		paying_account_already_request = false;

		//game_exit = true;
		//ryzom_exit = true;

		nlinfo("User Force to Quit ryzom");
	}
};
REGISTER_ACTION_HANDLER( CAHQuitRyzomNow, "quit_ryzom_now");


// ***************************************************************************
//Abort quit the game
class CAHQuitRyzomAbort : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if(ClientCfg.Local)
		{
			game_exit_request = false;
			ryzom_exit_request = false;
			nlinfo("User Abort to Quit ryzom");
		}
		else
		{
			// send a message to server, thru cancel cast
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			CAHManager::getInstance()->runActionHandler("phrase_cancel_cast", NULL);
		}
		paying_account_request = false;
		paying_account_already_request = false;
	}
};
REGISTER_ACTION_HANDLER( CAHQuitRyzomAbort, "quit_ryzom_abort");

// ***************************************************************************
//Abort quit the game
class CAHCloseFreeTrialQuitting : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		paying_account_request = false;

		if(game_exit_after_paying_account_request)
		{
			game_exit = true;
			ryzom_exit = true;
			game_exit_after_paying_account_request = false;
		}
		else
		{
			paying_account_already_request = true;
			CAHManager::getInstance()->runActionHandler("quit_ryzom", NULL);
		}

		CAHManager::getInstance()->runActionHandler("leave_modal", NULL);
	}
};
REGISTER_ACTION_HANDLER( CAHCloseFreeTrialQuitting, "close_free_trial_game_quitting");


// ***************************************************************************
// quit the ring session and return to mainland Ryzom
class CAHReturnToMainland : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		FarTP.requestReturnToPreviousSession();
	}
};
REGISTER_ACTION_HANDLER( CAHReturnToMainland, "return_to_mainland");

// ***************************************************************************
// quit the ring session and return to character selection
class CAHReselectCharacter : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		nlinfo("User request to reselect character");
		FarTP.requestReconnection();
	}
};
REGISTER_ACTION_HANDLER( CAHReselectCharacter, "reselect_character");

// ***************************************************************************
/** Select an item in a selection group
  */
class CSelectItemSheet : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet *ctrlSheet = dynamic_cast<CDBCtrlSheet *>(pCaller);
		if (!ctrlSheet) return;
		sint selectionGroup = ctrlSheet->getSelectionGroup();
		CInterfaceManager *im = CInterfaceManager::getInstance();
		const CCtrlSheetSelection &css = CWidgetManager::getInstance()->getParser()->getCtrlSheetSelection();
		const CSheetSelectionGroup *csg = css.getGroup(selectionGroup);
		if (csg && csg->isActive())
		{
			if (ctrlSheet->getSheetId() != 0)
			{
				// set the slot as the current selection
				CDBCtrlSheet::setCurrSelection(ctrlSheet);
			}
			else
			{
				CDBCtrlSheet::setCurrSelection(NULL);
			}
		}
		bool canUse = true;
		bool canBuild = true;
		bool canUseBuiltItem = true;
		// check if user has the level to use the item (applies to item & plans)
		if (ctrlSheet->getSheetCategory() == CDBCtrlSheet::Item)
		{
			if (csg->getName() == "buy_selection")
			{
				const CItemSheet *is = ctrlSheet->asItemSheet();
				if (is)
				{
					// get the needed skill to use the item
					SKILLS::ESkills rs = is->getRequiredSkill();
					if (rs < SKILLS::unknown)
					{
						#define SKILL_PATH "SERVER:CHARACTER_INFO:SKILLS:"
						string path =  toString(SKILL_PATH "%d:SKILL", (int) rs);
						CCDBNodeLeaf *nl = NLGUI::CDBManager::getInstance()->getDbProp(path,false);
						if (nl)
						{
							if (nl->getValue32() == 0)
							{
								canUse = false;
							}
						}
					}

					// display msg in the system infos
					if (!canUse)
					{
						ucstring msg = CI18N::get("msgCantUseItem");
						string cat = getStringCategory(msg, msg);
						im->displaySystemInfo(msg, cat);
					}
					if (!canBuild)
					{
						ucstring msg = CI18N::get("msgCantBuild");
						string cat = getStringCategory(msg, msg);
						im->displaySystemInfo(msg, cat);
					}
					if (!canUseBuiltItem)
					{
						ucstring msg = CI18N::get("msgCantUseBuiltItem");
						string cat = getStringCategory(msg, msg);
						im->displaySystemInfo(msg, cat);
					}
				}
			}
		}
		showItemFlags(im, canUse, canBuild, canUseBuiltItem);
	}



	/** On the trade windows, display or not : can use item, can build item, can use built item
	  */
	void showItemFlags(CInterfaceManager *im, bool canUse, bool canBuild, bool canUseBuiltItem);
};
REGISTER_ACTION_HANDLER (CSelectItemSheet, "select_item");

void CSelectItemSheet::showItemFlags(CInterfaceManager *im,bool canUse,bool canBuild,bool canUseBuiltItem)
{
	if (!im) return;
	CInterfaceGroup *gr;
	#define BOT_CHAT_TRADE_PATH "ui:interface:bot_chat_trade:header_opened:trade_content:"
	gr = dynamic_cast<CInterfaceGroup *>( CWidgetManager::getInstance()->getElementFromId(BOT_CHAT_TRADE_PATH "cant_use_item"));
	if (gr) gr->setActive(!canUse);
	gr = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(BOT_CHAT_TRADE_PATH "cant_use_built_item"));
	if (gr) gr->setActive(!canUseBuiltItem);
	gr = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(BOT_CHAT_TRADE_PATH "cant_build_item"));
	if (gr) gr->setActive(!canBuild);
}


// ***************************************************************************
/** Set a price in the given DB entries
  *
  */
class CSetPriceInDB : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		std::string ls = getParam(Params, "ls");
		std::string ms = getParam(Params, "ms");
		std::string bs = getParam(Params, "bs");
		std::string vbs = getParam(Params, "vbs");
		std::string value = getParam(Params, "value");
		if (ls.empty() || ms.empty() || bs.empty() || vbs.empty() || value.empty())
		{
			nlwarning("<CSetItemPriceInDB::execute> can't get db address for all kind of seeds, or can't get value");
			return;
		}
		CInterfaceExprValue price;
		if (!CInterfaceExpr::eval(value, price) || !price.toInteger())
		{
			nlwarning("<CSetPriceInDB::execute> : Can't evaluate price");
			return;
		}
		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (price.getInteger() >= 0)
		{
			CSeeds money;
			money.setTotal(price.getInteger());
			NLGUI::CDBManager::getInstance()->getDbProp(ls)->setValue64(money.getLS());
			NLGUI::CDBManager::getInstance()->getDbProp(ms)->setValue64(money.getMS());
			NLGUI::CDBManager::getInstance()->getDbProp(bs)->setValue64(money.getBS());
			NLGUI::CDBManager::getInstance()->getDbProp(vbs)->setValue64(money.getVBS());
		}
		else
		{
			// undefined price
			NLGUI::CDBManager::getInstance()->getDbProp(ls)->setValue64(-1);
			NLGUI::CDBManager::getInstance()->getDbProp(ms)->setValue64(-1);
			NLGUI::CDBManager::getInstance()->getDbProp(bs)->setValue64(-1);
			NLGUI::CDBManager::getInstance()->getDbProp(vbs)->setValue64(-1);
		}
	}
};
REGISTER_ACTION_HANDLER (CSetPriceInDB, "set_price_in_db");


// ***************************************************************************
/** Pack animal orders
  */
//give an order to the beast
void beastOrder (const std::string &orderStr, const std::string &beastIndexStr, bool confirmFree)
{
	uint8 order = (uint8) ANIMALS_ORDERS::stringToBeastOrder(orderStr);
	if (order == ANIMALS_ORDERS::UNKNOWN_BEAST_ORDER)
	{
		nlwarning("<beastOrder> : invalid beast order : %s", orderStr.c_str());
		return;
	}
	sint64 beastIndex;
	if (!CInterfaceExpr::evalAsInt(beastIndexStr, beastIndex))
	{
		nlwarning("<beastOrder> : can't read beast index");
		return;
	}
	// 0 is for all beasts => MAX_INVENTORY_ANIMAL is a valid selection
	if (beastIndex > MAX_INVENTORY_ANIMAL)
	{
		nlwarning("<beastOrder> : invalid animal index %d, maximum is %d", beastIndex, (int) MAX_INVENTORY_ANIMAL);
		return;
	}

	// if the order is a free, and if confirm, open a message first
	if(order == ANIMALS_ORDERS::FREE && confirmFree)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		if(beastIndex==0)
			pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQConfirmFreeAllAnimal"),
				"do_beast_free", toString(beastIndex) );
		else
			pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQConfirmFreeAnimal"),
				"do_beast_free", toString(beastIndex) );
	}
	// else launch the command
	else
	{
		// execute the order.
		const string msgName = "ANIMALS:BEAST";
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		{
			uint8 u8BeastIndex = (uint8) beastIndex;
			out.serial(u8BeastIndex); // to activate on server side
									  // 0 -> all beasts, otherwise, the index of the beast
			out.serial(order);
			NetMngr.push(out);
		}
		else
			nlwarning("<beastOrder> : unknown message name : '%s'.", msgName.c_str());
	}
}

// ***************************************************************************
class CHandlerBeastOrder : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string orderStr = getParam(Params,"order");
		if( orderStr == "mount" )
		{
			// target the beast
			CInterfaceManager * pIM= CInterfaceManager::getInstance();
			string beastIndex;
			if( CInterfaceExpr::evalAsString(getParam(Params,"beast_index"), beastIndex) )
			{
				CAHManager::getInstance()->runActionHandler("animal_target", NULL, beastIndex);
			}
			// move to the beast
			UserEntity->moveTo(UserEntity->selection(),3.0,CUserEntity::Mount);
		}
		else
			beastOrder (getParam(Params,"order"), getParam(Params,"beast_index"), true);
	}
};
REGISTER_ACTION_HANDLER( CHandlerBeastOrder, "beast_order")


// ***************************************************************************
class CHandlerDoBeastFree : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		// free with no confirm
		beastOrder ("free", Params, false);
	}
};
REGISTER_ACTION_HANDLER( CHandlerDoBeastFree, "do_beast_free")


// ***************************************************************************
// Change Possible Orders
class CHandlerAnimalMenuOption : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup	  *pMenu= dynamic_cast<CInterfaceGroup*>(pCaller);
		if(!pMenu)	return;

		CViewTextMenu *pFollow = dynamic_cast<CViewTextMenu*>(pMenu->getView("follow"));
		CViewTextMenu *pStop = dynamic_cast<CViewTextMenu*>(pMenu->getView("stop"));
		CViewTextMenu *pFree = dynamic_cast<CViewTextMenu*>(pMenu->getView("free"));
		CViewTextMenu *pEnterStable = dynamic_cast<CViewTextMenu*>(pMenu->getView("enter_stable"));
		CViewTextMenu *pLeaveStable = dynamic_cast<CViewTextMenu*>(pMenu->getView("leave_stable"));
		CViewTextMenu *pMount = dynamic_cast<CViewTextMenu*>(pMenu->getView("mount"));
		CViewTextMenu *pUnseat = dynamic_cast<CViewTextMenu*>(pMenu->getView("unseat"));

		// Get the animal Selected. 0 for Alls, 1,2,3,4,5 for each pack animal
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:BEAST_SELECTED", false);
		if(!node)	return;
		sint		selected= node->getValue32();

		// Enable menu entries according to each beast
		// All Mode
		if(selected==0)
		{
			// Test all menu option ('OR': display even if *all* animals don't comply with the commands)
			for(uint i=0;i<MAX_INVENTORY_ANIMAL;i++)
			{
				// Get the entity if it is in vision
				CEntityCL* selectedAnimalInVision = NULL;
				CCDBNodeLeaf *uidProp = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:UID", i), false);
				if ( uidProp )
				{
					CLFECOMMON::TClientDataSetIndex datasetIndex = uidProp->getValue32();
					selectedAnimalInVision = EntitiesMngr.getEntityByCompressedIndex( datasetIndex );
				}

				// Enable menu items
				testMenuOptionForPackAnimal(selectedAnimalInVision, i, (i==0),
					pFollow, pStop, pFree, pEnterStable, pLeaveStable, pMount, pUnseat);
			}
		}
		else if(selected>=1 && selected<=MAX_INVENTORY_ANIMAL)
		{
			// Get the entity if it is in vision
			CEntityCL* selectedAnimalInVision = NULL;
			CCDBNodeLeaf *uidProp = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:UID", selected-1), false);
			if ( uidProp )
			{
				CLFECOMMON::TClientDataSetIndex datasetIndex = uidProp->getValue32();
				selectedAnimalInVision = EntitiesMngr.getEntityByCompressedIndex( datasetIndex );
			}

			// Enable menu items
			testMenuOptionForPackAnimal(selectedAnimalInVision, selected-1, true,
				pFollow, pStop, pFree, pEnterStable, pLeaveStable, pMount, pUnseat);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerAnimalMenuOption, "animal_menu_option")


// ***************************************************************************
// Target an animal
class CHandlerAnimalTarget : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// retrieve the index of the animal
		sint	animalIndex = 0;
		bool	ok = false;
		// If comes from the button, get direct index
		if( !sParams.empty() )
		{
			fromString(sParams, animalIndex);
			ok = true;
		}

		// If success to get the animal id
		if(ok)
		{
			// Get the animal id.
			CLFECOMMON::TClientDataSetIndex	entityUid= CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;

			if(animalIndex>=1 && animalIndex<=MAX_INVENTORY_ANIMAL)
			{
				CCDBNodeLeaf *prop = NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:UID", animalIndex-1), false);
				if(prop)	entityUid= prop->getValue32();
			}

			if(entityUid != CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
			{
				// get the entity by its received UID
				CEntityCL	*entity= EntitiesMngr.getEntityByCompressedIndex(entityUid);
				if(entity)
					// Select this entity.
					UserEntity->selection(entity->slot());
				else
				{
					// the entity is not in vision, can't select it
					pIM->displaySystemInfo(CI18N::get("uiAnimalSelectNotInVision"), "CHK");
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerAnimalTarget, "animal_target" );


// ***************************************************************************
// Open an animal inventory
class CHandlerAnimalOpenInventory : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// retrieve the index of the animal
		sint	animalIndex= 0;
		bool	ok= false;
		// If comes from the button, get direct index
		if( !sParams.empty() )
		{
			fromString(sParams, animalIndex);
			ok= true;
		}

		// If success to get the animal id
		if(ok)
		{
			if(animalIndex>=1 && animalIndex<=MAX_INVENTORY_ANIMAL)
			{
				// show/hide the inventory
				CInterfaceElement	*group= CWidgetManager::getInstance()->getElementFromId(toString("ui:interface:inv_pa%d", animalIndex-1) );

				if(group) group->setActive(!group->getActive());
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerAnimalOpenInventory, "animal_open_inventory" );


// ***************************************************************************
// closeGroup Helper
// ***************************************************************************
static void closeGroup(const string &groupName)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(groupName));
	if (pIG == NULL) return;
	pIG->setActive(false);
}

// ***************************************************************************
// ACCEPT TEAM INVITE
// ***************************************************************************

class CHandlerAcceptTeamInvitation : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		closeGroup("ui:interface:join_team_proposal");
		sendMsgToServer("TEAM:JOIN");
	}
protected:

};
REGISTER_ACTION_HANDLER( CHandlerAcceptTeamInvitation, "accept_team_invitation");

// ***************************************************************************
// REFUSE TEAM INVITE
// ***************************************************************************

class CHandlerRefuseTeamInvitation : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		closeGroup("ui:interface:join_team_proposal");
		sendMsgToServer("TEAM:JOIN_PROPOSAL_DECLINE");
	}
protected:

};
REGISTER_ACTION_HANDLER( CHandlerRefuseTeamInvitation, "refuse_team_invitation");

// ***************************************************************************
// ACCEPT GUILD INVITE
// ***************************************************************************

class CHandlerAcceptGuildInvitation : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		sendMsgToServer("GUILD:ACCEPT_INVITATION");
		CGuildManager::getInstance()->quitJoinProposal();
	}

};
REGISTER_ACTION_HANDLER( CHandlerAcceptGuildInvitation, "accept_guild_invitation");

// ***************************************************************************
// REFUSE GUILD INVITE
// ***************************************************************************

class CHandlerRefuseGuildInvitation : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		sendMsgToServer("GUILD:REFUSE_INVITATION");
		CGuildManager::getInstance()->quitJoinProposal();
	}
};
REGISTER_ACTION_HANDLER( CHandlerRefuseGuildInvitation, "refuse_guild_invitation");

// ***************************************************************************
// ACCEPT DUEL INVITE
// ***************************************************************************

class CHandlerAcceptDuelInvitation : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		closeGroup("ui:interface:join_duel_proposal");
		sendMsgToServer("DUEL:ACCEPT");
	}

};
REGISTER_ACTION_HANDLER( CHandlerAcceptDuelInvitation, "accept_duel_invitation");

// ***************************************************************************
// REFUSE DUEL INVITE
// ***************************************************************************

class CHandlerRefuseDuelInvitation : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		closeGroup("ui:interface:join_duel_proposal");
		sendMsgToServer("DUEL:REFUSE");
	}
};
REGISTER_ACTION_HANDLER( CHandlerRefuseDuelInvitation, "refuse_duel_invitation");

// ***************************************************************************
// ACCEPT PVP CHALLENGE INVITE
// ***************************************************************************

class CHandlerAcceptPVPChallengeInvitation : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		closeGroup("ui:interface:join_pvp_challenge_proposal");
		sendMsgToServer("PVP_CHALLENGE:ACCEPT");
	}

};
REGISTER_ACTION_HANDLER( CHandlerAcceptPVPChallengeInvitation, "accept_pvp_challenge_invitation");

// ***************************************************************************
// REFUSE PVP CHALLENGE INVITE
// ***************************************************************************

class CHandlerRefusePVPChallengeInvitation : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		closeGroup("ui:interface:join_pvp_challenge_proposal");
		sendMsgToServer("PVP_CHALLENGE:REFUSE");
	}
};
REGISTER_ACTION_HANDLER( CHandlerRefusePVPChallengeInvitation, "refuse_pvp_challenge_invitation");

// ***************************************************************************
// CHOOSE PVP CLAN
// ***************************************************************************
/*
class CHandlerChoosePVPClan : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		closeGroup("ui:interface:join_pvp_clan_proposal");

		uint8 clan = 0;
		if( Params == "neutral" )
			clan = 0;
		else if( Params == "clan1" )
			clan = 1;
		else if( Params == "clan2" )
			clan = 2;

		sendMsgToServer("PVP_VERSUS:CLAN", clan);
	}
};
REGISTER_ACTION_HANDLER( CHandlerChoosePVPClan, "pvp_clan_join");
*/

// ***************************************************************************
// Launch Bug Reporting Tool
// ***************************************************************************

class CAHLaunchBugReport : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (ClientCfg.Light)
		{
			vector<string> v;
			CWidgetManager::getInstance()->runProcedure ("proc_reset_interface", NULL, v);

			//CInterfaceManager::getInstance()->launchContextMenuInGame("ui:interface:game_context_menu");
		}
		ICommand::execute("bugReport 80", g_log);
	}
};
REGISTER_ACTION_HANDLER( CAHLaunchBugReport, "launch_bug_report");

// ***************************************************************************
// Launch Help Tool
// ***************************************************************************

class CAHLaunchHelp : public IActionHandler
{
public:

    virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
    {
		CInterfaceManager	*pIM = CInterfaceManager::getInstance();

		string url = getParam (Params, "url");
		string helpContainer = getParam (Params, "help_container");
		if (helpContainer.empty ())
			helpContainer = "ui:interface:help_browser";

		// open the help browser
		CInterfaceElement	*pIG= CWidgetManager::getInstance()->getElementFromId(helpContainer);
		if(pIG)
			pIG->setActive(true);

		// browse the url
		CAHManager::getInstance()->runActionHandler("browse", NULL, "name="+helpContainer+":content:html|url="+url);
    }
};
REGISTER_ACTION_HANDLER( CAHLaunchHelp, "launch_help");

// ***************************************************************************
static bool findInterfacePath(string &sPath, CCtrlBase *pCaller)
{
	if (sPath.rfind(':') == string::npos)
	{
		if (pCaller == NULL) return false;
		sPath = pCaller->getId() + ":" + sPath;
	}
	else
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		string elt = sPath.substr(0,sPath.rfind(':'));
		CInterfaceElement *pIE;
		if (pCaller != NULL)
			pIE = CWidgetManager::getInstance()->getElementFromId(pCaller->getId(), elt);
		else
			pIE = CWidgetManager::getInstance()->getElementFromId(elt);
		if (pIE == NULL) return false;
		sPath = pIE->getId() + ":" + sPath.substr(sPath.rfind(':')+1,sPath.size());
	}
	return true;
}

// ***************************************************************************
// TARGET NAME AND TITLE MANAGEMENT
// ***************************************************************************

// Callbacks that check that while we are waiting for the string to display the
// user has not changed its target

// *** check for user selection and remove title
class CSPPRemoveTitleAndCheckSelection : public CStringPostProcessRemoveTitle
{
public:

	sint32 Slot;

	bool cbIDStringReceived(ucstring &inout)
	{
		if (UserEntity != NULL)
		{
			if (UserEntity->selection() == Slot)
			{
				ucstring copyInout = inout;
				CStringPostProcessRemoveTitle::cbIDStringReceived(inout);
				if (inout.empty())
				{
					CEntityCL *entity = EntitiesMngr.entity(Slot);
					CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(entity);
					bool womanTitle = false;
					if (pChar != NULL)
						womanTitle = pChar->getGender() == GSGENDER::female;
					
					STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(copyInout), womanTitle);
					
					// Sometimes translation contains another title
					ucstring::size_type pos = copyInout.find('$');
					if (pos != ucstring::npos)
					{
						copyInout = STRING_MANAGER::CStringManagerClient::getTitleLocalizedName(CEntityCL::getTitleFromName(copyInout), womanTitle);
					}
					CStringPostProcessRemoveTitle::cbIDStringReceived(copyInout);
					inout = copyInout;
				}

				return true;
			}
		}

		return false;
	}
};

// *** check for user selection and remove name (keep title)
class CSPPRemoveNameAndCheckSelection : public CStringPostProcessRemoveName
{
public:

	sint32 Slot;

	bool cbIDStringReceived(ucstring &inout)
	{
		if (UserEntity != NULL)
		{
			if (UserEntity->selection() == Slot)
			{
				CEntityCL *entity = EntitiesMngr.entity(Slot);
				CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(entity);
				if (pChar != NULL)
				{
					const CCharacterSheet *pSheet = pChar->getSheet();
					if (pSheet != NULL)
					{
						string sFame = pSheet->getFame();
						if (strnicmp(sFame.c_str(),"tribe_",6)==0)
						{
							inout = STRING_MANAGER::CStringManagerClient::getFactionLocalizedName(sFame);
							return true; // return tribe name
						}
					}
					Woman = pChar->getGender() == GSGENDER::female;
				}

				CStringPostProcessRemoveName::cbIDStringReceived(inout);

				return true; // return title
			}
		}
		return false;
	}
};

// *** called when target change or target name change
class CActionHandlerSetTargetName : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		string sSlot = getParam(Params,"slot");
		string sNameTarget = getParam(Params,"target");
		string sTitleTarget = getParam(Params,"title");

		if (sSlot.empty()) return;

		if (!findInterfacePath(sNameTarget, pCaller)) return;
		findInterfacePath(sTitleTarget, pCaller);

		CInterfaceExprValue evValue;
		if (CInterfaceExpr::eval(sSlot, evValue, NULL))
		{
			sint32 nSlot = (sint32)evValue.getInteger();

			ucstring TargetName;
			ucstring TargetTitle;

			// Get from nSlot
			if (nSlot > -1)
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
//				uint32 nDBid = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:Entities:E"+toString(nSlot)+":P6")->getValue32();
				uint32 nDBid = 0;
				if (nSlot < sint32(EntitiesMngr.entities().size()) && EntitiesMngr.entities()[nSlot] != NULL)
				{
					nDBid = EntitiesMngr.entities()[nSlot]->getNameId();
				}

				if (nDBid != 0)
				{
					CSPPRemoveTitleAndCheckSelection *pSPPRT = new CSPPRemoveTitleAndCheckSelection;
					pSPPRT->Slot = nSlot;
					CSPPRemoveNameAndCheckSelection *pSPPRN = new CSPPRemoveNameAndCheckSelection;
					pSPPRN->Slot = nSlot;
					pIM->addServerID(sNameTarget, nDBid, pSPPRT);
					pIM->addServerID(sTitleTarget, nDBid, pSPPRN);
				}
				else
				{
					CEntityCL *pE = EntitiesMngr.entity(nSlot);
					if (pE != NULL)
					{
						TargetName = pE->getDisplayName();
						TargetTitle = pE->getTitle();
					}
				}
			}
			// Set to target
			CInterfaceExprValue evUCStr;
			evUCStr.setUCString(TargetName);
			CInterfaceLink::setTargetProperty(sNameTarget, evUCStr);
			evUCStr.setUCString(TargetTitle);
			CInterfaceLink::setTargetProperty(sTitleTarget, evUCStr);
		}
	}
};
REGISTER_ACTION_HANDLER (CActionHandlerSetTargetName, "set_target_name");

// ***************************************************************************
class CActionHandlerSetTargetForceRegionLevel: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sSlot = getParam(Params,"slot");
		string sTargetRegion = getParam(Params,"targetRegion");
		string sTargetLevel = getParam(Params,"targetLevel");

		// Access UI elements
		if (sSlot.empty()) return;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CViewBitmap *pVBR = dynamic_cast<CViewBitmap*>(CWidgetManager::getInstance()->getElementFromId(sTargetRegion));
		if (pVBR == NULL)
			return;
		CViewBitmap *pVBL = dynamic_cast<CViewBitmap*>(CWidgetManager::getInstance()->getElementFromId(sTargetLevel));
		if (pVBL == NULL)
			return;
		CInterfaceExprValue evValue;
		if (!CInterfaceExpr::eval(sSlot, evValue, NULL))
			return;
		sint32 nSlot = (sint32)evValue.getInteger();
		CCtrlBase *pTooltip = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:target:header_opened:force"));

		// Access target entity
		CEntityCL *pE = NULL;
		if ( (nSlot == -1) || ((pE = EntitiesMngr.entity(nSlot)) == NULL) )
		{
			// If untargetted, clear
			pVBL->setTexture(string());
			pVBR->setColor(CRGBA(0,0,0,0));

			if (pTooltip)
				pTooltip->setDefaultContextHelp(ucstring(""));

			return;
		}

		sint nLevelForce;
		sint nForceRegion;
		if ( pE->isPlayer() )
		{
			// Player => deduce RegionForce & ForceLevel from the database
			CCDBNodeLeaf *pDbTargetUid = NLGUI::CDBManager::getInstance()->getDbProp( CWidgetManager::getInstance()->getParser()->getDefine("target_uid") );
			if ( ! pDbTargetUid )
				return;
			// Hide the target level if the USER is not in PVP FACTION
			// Hide the target level if the TARGET is not in PVP FACTION
			// Also, hide it If the database is not in sync with the local target slot
			if (!UserEntity || !(UserEntity->getPvpMode()&PVP_MODE::PvpZoneFaction) ||
				!(pE->getPvpMode()&PVP_MODE::PvpZoneFaction) ||
				pE->dataSetId() != (CLFECOMMON::TClientDataSetIndex)pDbTargetUid->getValue32() )
			{
				pVBL->setTexture(string());
				pVBR->setColor(CRGBA(0,0,0,0));

				if (pTooltip)
					pTooltip->setDefaultContextHelp(CI18N::get("uittTargetUnknown"));

				return;
			}
			CCDBNodeLeaf *pDbPlayerLevel = NLGUI::CDBManager::getInstance()->getDbProp( CWidgetManager::getInstance()->getParser()->getDefine("target_player_level") );
			if ( ! pDbPlayerLevel )
				return;
			sint nLevel = pDbPlayerLevel->getValue32();
			if ( nLevel < 250 )
			{
				nLevelForce = ((nLevel % 50) * 5 / 50) + 1;
				nForceRegion = (nLevel < 20) ? 1 : (nLevel / 50) + 2;
			}
			else
			{
				nLevelForce = 6; // same as named creatures
				nForceRegion = 8;
			}
		}
		else
		{
			// Creature => RegionForce & ForceLevel are in its sheet
			CCharacterSheet *pCS = dynamic_cast<CCharacterSheet*>(SheetMngr.get(pE->sheetId()));
			if (pCS == NULL || pCS->RegionForce==-1)
			{
				pVBL->setTexture(string());
				pVBR->setColor(CRGBA(0,0,0,0));

				if (pTooltip)
					pTooltip->setDefaultContextHelp(ucstring(""));

				return;
			}

			nForceRegion = pCS->RegionForce;
			nLevelForce = pCS->ForceLevel;
		}

		// Set color
		if (nForceRegion > 6) nForceRegion = 6;
		if (nForceRegion < 1) nForceRegion = 1;
		CRGBA col = CInterfaceElement::convertColor(CWidgetManager::getInstance()->getParser()->getDefine("region_force_"+toString(nForceRegion)).c_str());
		pVBR->setColor(col);

		// Set texture
		if (nLevelForce > 8) nLevelForce = 8;
		if (nLevelForce < 1) nLevelForce = 1;
		string sTexture = CWidgetManager::getInstance()->getParser()->getDefine("force_level_"+toString(nLevelForce));
		pVBL->setTexture(sTexture);

		// Set tooltip
		CCtrlBase *tooltip = dynamic_cast<CCtrlBase*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:target:header_opened:force"));
		if (tooltip)
		{
			ucstring str;

			if (nForceRegion == 1)
				nForceRegion = 2;

			// Normal
			if (nLevelForce < 6)
			{
				sint min = (nForceRegion-2) * 50 + (nLevelForce-1) * 10 + 1;
				sint max = (nForceRegion-2) * 50 + nLevelForce * 10;

				str= CI18N::get("uittTargetLevel");
				strFindReplace(str, "%min", toString(min));
				strFindReplace(str, "%max", toString(max));
			}
			// Boss
			else if (nLevelForce == 8)
			{
				sint n = (nForceRegion-1) * 50;
				if (pE->isNPC())
					str= CI18N::get("uittTargetGuardBoss");
				else
					str= CI18N::get("uittTargetBoss");
				strFindReplace(str, "%n", toString("%d", n) );
			}
			// Named
			else
			{
				sint n = (nForceRegion-1) * 50;
				str= CI18N::get("uittTargetNamed");
				strFindReplace(str, "%n", toString("%d", n) );
			}

			tooltip->setDefaultContextHelp(str);
		}
	}
};
REGISTER_ACTION_HANDLER (CActionHandlerSetTargetForceRegionLevel, "set_force_region_level");

// ***************************************************************************
class CAHUpdateCurrentMode : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sValue = getParam(Params,"value");
		string sDBLink = getParam(Params,"dblink");
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sDBLink, false);
		if (pNL == NULL) return;

		CInterfaceExprValue eVal;
		if (!CInterfaceExpr::eval(sValue, eVal, NULL)) return;

		sint32 nNewMode = (sint32)eVal.getInteger();

		sint32 nModeMinInf, nModeMaxInf, nModeMinLab, nModeMaxLab, nModeMinKey, nModeMaxKey;
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_min_info"), nModeMinInf);
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_max_info"), nModeMaxInf);
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_min_lab"), nModeMinLab);
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_max_lab"), nModeMaxLab);
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_min_keys"), nModeMinKey);
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_max_keys"), nModeMaxKey);

		sint32 nMode = 0;
		if ((nNewMode >= nModeMinInf) && (nNewMode <= nModeMaxInf))
			nMode = 1;
		if ((nNewMode >= nModeMinLab) && (nNewMode <= nModeMaxLab))
			nMode = 2;
		if ((nNewMode >= nModeMinKey) && (nNewMode <= nModeMaxKey))
			nMode = 3;


		if (nNewMode != 0)
		if (nNewMode == pNL->getValue32())
		{
			// We have pushed 2 times the same button
			nNewMode ++;
			if (nMode == 1)
			{
				if (nNewMode > nModeMaxInf) nNewMode = nModeMinInf;
			}
			else if (nMode == 2)
			{
				if (nNewMode > nModeMaxLab) nNewMode = nModeMinLab;
			}
			else if (nMode == 3)
			{
				if (nNewMode > nModeMaxKey) nNewMode = nModeMinKey;
			}
		}

		// Found the first active entry in db
		if (nMode == 2)
		{
			bool bFound = false;
			while (!bFound)
			{
				CCDBNodeLeaf *pIntFlags = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INTERFACES:FLAGS", false);
				if (pIntFlags == NULL) return;
				sint64 nIntFlags = pIntFlags->getValue64();

				sint32 tmpMode;

				// Is NewMode entry active ?
				fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_magic"), tmpMode);
				if (nNewMode == tmpMode)
					if ((nIntFlags & (1<<INTERFACE_FLAGS::Magic)) != 0)
						bFound = true;

				if (!bFound)
				{
					fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_combat"), tmpMode);
					if (nNewMode == tmpMode)
						if ((nIntFlags & (1<<INTERFACE_FLAGS::Combat)) != 0)
							bFound = true;
				}

				if (!bFound)
				{
					fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_faber_create"), tmpMode);
					if (nNewMode == tmpMode)
						if ((nIntFlags & (1<<INTERFACE_FLAGS::FaberCreate)) != 0)
							bFound = true;
				}

				if (!bFound)
				{
					fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_faber_repair"), tmpMode);
					if (nNewMode == tmpMode)
						if ((nIntFlags & (1<<INTERFACE_FLAGS::FaberRepair)) != 0)
							bFound = true;
				}

				if (!bFound)
				{
					fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_faber_refine"), tmpMode);
					if (nNewMode == tmpMode)
						if ((nIntFlags & (1<<INTERFACE_FLAGS::FaberRefine)) != 0)
							bFound = true;
				}

				if (!bFound)
				{
					fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_commerce"), tmpMode);
					if (nNewMode == tmpMode)
						if ((nIntFlags & (1<<INTERFACE_FLAGS::Commerce)) != 0)
							bFound = true;
				}

				if (!bFound)
				{
					fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_macros"), tmpMode);
					if (nNewMode == tmpMode)
						bFound = true; // Not in DB !!!
				}

				if (!bFound)
				{
					fromString(CWidgetManager::getInstance()->getParser()->getDefine("mode_special_labo"), tmpMode);
					if (nNewMode == tmpMode)
						if ((nIntFlags & (1<<INTERFACE_FLAGS::Special)) != 0)
							bFound = true;
				}

				if (!bFound)
				{
					nNewMode++;
					if (nNewMode > nModeMaxLab) nNewMode = nModeMinLab;
					if (nNewMode == pNL->getValue32()) return;
				}
			}
		}

		pNL->setValue32(nNewMode);
	}
};
REGISTER_ACTION_HANDLER (CAHUpdateCurrentMode, "update_current_mode");

// ***************************************************************************
class CAHToggleChat : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		ShowInterface = !ShowInterface;
	}
};
REGISTER_ACTION_HANDLER (CAHToggleChat, "toggle_chat");

// ***************************************************************************
class CAHToggleHelp : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
#if FINAL_VERSION
		if( ClientCfg.Local || hasPrivilegeDEV() || hasPrivilegeSGM() || hasPrivilegeGM() )
#endif
		{
			ShowHelp = !ShowHelp;
		}
	}
};
REGISTER_ACTION_HANDLER (CAHToggleHelp, "toggle_help");

// ***************************************************************************
class CAHSelfTarget : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Select the entity
		UserEntity->selection(0);
	}
};
REGISTER_ACTION_HANDLER (CAHSelfTarget, "self_target");

// ***************************************************************************
class CAHNoTarget : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Select the entity
		UserEntity->selection(CLFECOMMON::INVALID_SLOT);
	}
};
REGISTER_ACTION_HANDLER (CAHNoTarget, "no_target");

// ***************************************************************************
class CAHTarget : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		// Get the entity name to target
		ucstring entityName;
		entityName.fromUtf8 (getParam (Params, "entity"));
		bool preferCompleteMatch = (getParam (Params, "prefer_complete_match") != "0");

		if (!entityName.empty())
		{
			CEntityCL *entity = NULL;
			if (preferCompleteMatch)
			{
				// Try to get the entity with complete match first
				entity = EntitiesMngr.getEntityByName (entityName, false, true);
			}
			
			if (entity == NULL)
			{
				// Get the entity with a partial match
				entity = EntitiesMngr.getEntityByName (entityName, false, false);
			}
			
			if (entity)
			{
				CCharacterCL *character = dynamic_cast<CCharacterCL*>(entity);
				if (character != NULL)
				{
					if(character->isSelectableBySpace())
					{
						nldebug("isSelectableBySpace");
					}
					else
					{
						nldebug("is not isSelectableBySpace");
					}
				}
				if(entity->properties().selectable())
				{
					nldebug("is prop selectable");
				}
				else
				{
					// to avoid campfire selection exploit #316
					nldebug("is not prop selectable");
					CInterfaceManager	*pIM= CInterfaceManager::getInstance();
					pIM->displaySystemInfo(CI18N::get("uiTargetErrorCmd"));
					return;
				}

				// Select the entity
				UserEntity->selection(entity->slot());
			}
			else
			{
				CInterfaceManager	*pIM= CInterfaceManager::getInstance();
				pIM->displaySystemInfo(CI18N::get("uiTargetErrorCmd"));
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHTarget, "target");



class CAHAddShape : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		string sShape = getParam(Params, "shape");
		if(sShape.empty())
		{
			nlwarning("Command 'add_shape': need at least the parameter shape.");
			return;
		}
		if (!Scene)
		{
			nlwarning("No scene available");
			return;
		}

		double x = UserEntity->pos().x;
		double y = UserEntity->pos().y;
		double z = UserEntity->pos().z;
		CVector userDir = UserEntity->dir();
		float s = 1.0f;
		string skeleton = getParam(Params, "skeleton");
		string c = getParam(Params, "text");
		string u = getParam(Params, "url");
		string texture_name = getParam(Params, "texture");
		string highlight = getParam(Params, "highlight");
		string transparency = getParam(Params, "transparency");

		if (!getParam(Params, "x").empty())
			fromString(getParam(Params, "x"), x);
		if (!getParam(Params, "y").empty())
			fromString(getParam(Params, "y"), y);
		if (!getParam(Params, "z").empty())
			fromString(getParam(Params, "z"), z);
		if (!getParam(Params, "scale").empty())
			fromString(getParam(Params, "scale"), s);
		if (!getParam(Params, "angle").empty())
		{
			float a;
			fromString(getParam(Params, "angle"), a);
			userDir = CVector(sin(a), cos(a), 0.f);
		}

		bool have_shapes = true;
		bool first_shape = true;
		while(have_shapes)
		{
			string shape;
			string::size_type index = sShape.find(string(" "));
			// multiple shapes/fx
			if (index != string::npos)
			{
				shape = sShape.substr(0, index);
				sShape = sShape.substr(index+1);
			}
			else
			{
				shape = sShape;
				have_shapes = false;
			}


			CShapeInstanceReference instref = EntitiesMngr.createInstance(shape, CVector((float)x, (float)y, (float)z), c, u, first_shape);
			UInstance instance = instref.Instance;

			if(!instance.empty())
			{
				for(uint j=0;j<instance.getNumMaterials();j++)
				{
					if (highlight.empty())
					{
						instance.getMaterial(j).setAmbient(CRGBA(0,0,0,255));
						instance.getMaterial(j).setShininess( 10.0f );
						instance.getMaterial(j).setEmissive(CRGBA(255,255,255,255));
					}
					else
					{
						instance.getMaterial(j).setAmbient(CRGBA(0,0,0,255));
						instance.getMaterial(j).setEmissive(CRGBA(255,0,0,255));
						instance.getMaterial(j).setShininess( 1000.0f );
					}

					if (!texture_name.empty() && first_shape)
					{
						sint numStages = instance.getMaterial(j).getLastTextureStage() + 1;
						for(sint l = 0; l < numStages; l++)
						{
							if (instance.getMaterial(j).isTextureFile((uint) l))
							{
								instance.getMaterial(j).setTextureFileName(texture_name, (uint) l);
							}
						}
					}
				}

				first_shape = false;

				if (transparency.empty())
					::makeInstanceTransparent(instance, 255, false);
				else
					::makeInstanceTransparent(instance, 100, true);

				instance.setClusterSystem(UserEntity->getClusterSystem()); // for simplicity, assume it is in the same
																		   // cluster system than the user
				// Compute the direction Matrix
				CMatrix dir;
				dir.identity();
				CVector vi = userDir^CVector(0.f, 0.f, 1.f);
				CVector vk = vi^userDir;
				dir.setRot(vi, userDir, vk, true);
				// Set Orientation : User Direction should be normalized.
				if (!skeleton.empty())
				{
					USkeleton skel = Scene->createSkeleton(skeleton);
					if (!skel.empty())
					{
						skel.bindSkin(instance);
						skel.setClusterSystem(UserEntity->getClusterSystem());
						skel.setScale(skel.getScale()*s);
						skel.setPos(CVector((float)x, (float)y, (float)z));
						skel.setRotQuat(dir.getRot());
					}
				}
				else
				{
					instance.setScale(instance.getScale()*s);
					instance.setPos(CVector((float)x, (float)y, (float)z));
					instance.setRotQuat(dir.getRot());
				}
				// if the shape is a particle system, additionnal parameters are user params
				UParticleSystemInstance psi;
				psi.cast (instance);
				/*if (!psi.empty())
				{
					// set each user param that is present
					for(uint k = 0; k < 4; ++k)
					{
						if (args.size() >= (k + 2))
						{
							float uparam;
							if (fromString(args[k + 1], uparam))
							{
								psi.setUserParam(k, uparam);
							}
							else
							{
								nlwarning("Cant read param %d", k);
							}
						}
					}
				}*/
			}
			else
				nlwarning("Command 'add_shape': cannot find the shape %s.", sShape.c_str());
		}

		return;
	}
};
REGISTER_ACTION_HANDLER (CAHAddShape, "add_shape");

class CAHRemoveShapes : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		EntitiesMngr.removeInstances();
	}
};
REGISTER_ACTION_HANDLER (CAHRemoveShapes, "remove_shapes");

// ***************************************************************************
// See also CHandlerTeamTarget
class CAHTargetTeammateShortcut : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		// Get shortcut parameter
		uint indexInTeam;
		fromString(getParam( Params, "indexInTeam" ), indexInTeam);
		--indexInTeam;

		if ( indexInTeam >= PeopleInterraction.TeamList.getNumPeople() )
			return;

		// Index is the database index (serverIndex() not used for team list)
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp( NLMISC::toString( TEAM_DB_PATH ":%hu:NAME", indexInTeam ), false );
		if ( ! pNL )
			return;
		if ( pNL->getValueBool() )
		{
			// There is a character corresponding to this index
			pNL = NLGUI::CDBManager::getInstance()->getDbProp( NLMISC::toString( TEAM_DB_PATH ":%hu:UID", indexInTeam ), false );
			if ( ! pNL )
				return;
			CLFECOMMON::TClientDataSetIndex compressedIndex = pNL->getValue32();

			// Search entity in vision
			CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex( compressedIndex );
			if ( entity )
			{
				UserEntity->selection( entity->slot() );
			}
			else
			{
				CInterfaceManager	*pIM= CInterfaceManager::getInstance();
				pIM->displaySystemInfo(CI18N::get("uiTeamSelectNotInVision"), "CHK");
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CAHTargetTeammateShortcut, "target_teammate_shortcut");

// ***************************************************************************
class CAHAssist : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		// Get the entity name to target
		ucstring entityName;
		entityName.fromUtf8 (getParam (Params, "entity"));
		if (!entityName.empty())
		{
			// Get the entity
			CEntityCL *entity = EntitiesMngr.getEntityByName (entityName, false, false);
			if (entity)
			{
				// Select the entity
				UserEntity->assist(entity->slot());
			}
			else
			{
				CInterfaceManager	*pIM= CInterfaceManager::getInstance();
				pIM->displaySystemInfo(CI18N::get("uiTargetErrorCmd"));
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHAssist, "assist");

// ***************************************************************************
class CAHAssistTarget : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Select the entity
		UserEntity->assist();
	}
};
REGISTER_ACTION_HANDLER (CAHAssistTarget, "assist_target");

// ***************************************************************************
class CAHToggleCombat : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Toggle from Combat to Disengage
		if(UserEntity->isFighting())
			UserEntity->disengage();
		// Toggle from Normal to Combat
		else
		{
			if( UserEntity->canEngageCombat() )
			{
				UserEntity->attack();
			}
		}

	}
};
REGISTER_ACTION_HANDLER (CAHToggleCombat, "toggle_combat");

// ***************************************************************************
class CAHSetDesktop : public IActionHandler
{
public:
	CAHSetDesktop()
	{
		_FirstTime = true;
	}

	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		/* // Previous version (multiple pressed on a desktop change a central window
		uint desktop;
		fromString(Params, desktop);
		if (desktop <MAX_NUM_MODES)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			const string procNames[MAX_NUM_MODES] = { "tb_setexp", "tb_setinfo", "tb_setlab", "tb_setkeys" };
			const string dbNames[MAX_NUM_MODES] = { "", "UI:SAVE:CURRENT_INFO_MODE", "UI:SAVE:CURRENT_LAB_MODE", "UI:SAVE:CURRENT_KEY_MODE" };
			string sValue;
			CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(dbNames[desktop], false);
			if (pNL != NULL)
				sValue = NLMISC::toString((sint32)pNL->getValue64());
			vector<string> vecStr;
			vecStr.push_back(procNames[desktop]);
			vecStr.push_back(sValue);
			CWidgetManager::getInstance()->runProcedure(procNames[desktop], NULL, vecStr);
		}*/

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:gestion_windows"));
		if (pGC == NULL)
		{
			nlwarning("gestion_windows not found as a container");
			return;
		}
		CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId("ui:interface:gestion_windows:close");
		if (pIE != NULL) pIE->setActive(false);

		CActionsManager *pAM = &Actions;
		if (!pAM->valide(CAction::CName("set_desktop",Params.c_str())))
		{
			pGC->setActive(false);
			_FirstTime = true;
		}
		else // Key is down
		{
			// For the first time ?
			if (_FirstTime)
			{
				_FirstTime = false;

				vector<string> vecStr;
				vecStr.push_back("tb_setdesktop");
				vecStr.push_back(Params);
				CWidgetManager::getInstance()->runProcedure("tb_setdesktop", NULL, vecStr);
			}
			else // Not the first time
			{
				// Show the container
				pGC->setActive(true);
				// Yoyo: important to setTopWindow ONLY if needed, else save of the TopWindow doesn't work when you switch it.
				CWidgetManager::getInstance()->setTopWindow(pGC);
			}
		}
	}
private:
	bool _FirstTime;
};
REGISTER_ACTION_HANDLER (CAHSetDesktop, "set_desktop");

// ***************************************************************************
class CAHCopyToDesktop : public IActionHandler
{
public:

	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		uint8 newMode;
		fromString(Params, newMode);
		pIM->resetMode(newMode);
		uint8 nLastMode = pIM->getMode();
		pIM->setMode(newMode);
		pIM->setMode(nLastMode);
	}
};
REGISTER_ACTION_HANDLER (CAHCopyToDesktop, "copy_to_desktop");


// ***************************************************************************
class CHandlerCloseAllLabosBut : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// list of labo windows
		const char*	laboWindows[]= {
			"ui:interface:faber_create",
			"ui:interface:faber_repair",
			"ui:interface:faber_refine",
			"ui:interface:combat",
			"ui:interface:magic",
			"ui:interface:special_labo",
			"ui:interface:commerce",
			"ui:interface:tracking",
		};
		uint	numLabos= sizeof(laboWindows)/sizeof(laboWindows[0]);

		// For all labos
		for(uint i=0;i<numLabos;i++)
		{
			// if not the excluded one
			if( Params != laboWindows[i] )
			{
				CInterfaceElement	*pElt= CWidgetManager::getInstance()->getElementFromId(laboWindows[i]);
				if(pElt)
					pElt->setActive(false);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerCloseAllLabosBut, "close_all_labos_but");

/*
// ***************************************************************************
class CHandlerToggleInventory : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// list of labo windows
		const char*	inventoryWindows[]= {
				"ui:interface:gestionsets",
				"ui:interface:userbags",
				"ui:interface:usermoney",
				"ui:interface:userarmors",
				"ui:interface:userjewelry",
		};
		uint	numWins= sizeof(inventoryWindows)/sizeof(inventoryWindows[0]);

		// For all labos
		bool state = false;
		CInterfaceElement	*pElt= CWidgetManager::getInstance()->getElementFromId(inventoryWindows[0]);
		if (pElt)
			state = !pElt->getActive();
		for(uint i=0;i<numWins;i++)
		{
			pElt= CWidgetManager::getInstance()->getElementFromId(inventoryWindows[i]);
			if(pElt)
				pElt->setActive(state);
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerToggleInventory, "toggle_inventory");
*/
// ***************************************************************************
// ***************************************************************************
// GAME CONFIG
// ***************************************************************************
// ***************************************************************************

#define GAME_CONFIG_DDX					"ui:interface:game_config:content:all"

static vector<UDriver::CMode> VideoModes;
#define GAME_CONFIG_VIDEO_MODES_COMBO	"ui:interface:game_config:content:general:video_modes"
#define GAME_CONFIG_VIDEO_FREQS_COMBO	"ui:interface:game_config:content:general:video_freqs"
#define GAME_CONFIG_VIDEO_FULLSCREEN_BUTTON	"ui:interface:game_config:content:general:fullscreen:c"
#define GAME_CONFIG_VIDEO_MODE_DB		"UI:TEMP:VID_MODE"
#define GAME_CONFIG_VIDEO_FREQ_DB		"UI:TEMP:VID_FREQ"
#define GAME_CONFIG_LANGUAGE			"UI:TEMP:LANGUAGE"
// We allow only this RGB depth to be taken
#define GAME_CONFIG_VIDEO_DEPTH_REQ		32

// VR_CONFIG
#define GAME_CONFIG_VR_DEVICES_COMBO	"ui:interface:game_config:content:vr:vr_devices"
#define GAME_CONFIG_VR_DEVICE_DB		"UI:TEMP:VR_DEVICE"

// The combo for Texture Mode selected
#define GAME_CONFIG_TEXTURE_MODE_COMBO	"ui:interface:game_config:content:general:texture_mode:combo"
#define GAME_CONFIG_TEXTURE_MODE_DB		"UI:TEMP:TEXTURE_MODE"

// The 3 possible modes editable (NB: do not allow client.cfg HDEntityTexture==1 and DivideTextureSizeBy2=2
enum	TTextureMode	{LowTextureMode= 0, NormalTextureMode= 1, HighTextureMode= 2};

void cacheStereoDisplayDevices(); // from init.cpp

void updateVRDevicesComboUI()
{
	// VR_CONFIG
	nldebug("Init VR device name list from cache into UI");
	// init vr device name list from cache
	CDBGroupComboBox *pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId(GAME_CONFIG_VR_DEVICES_COMBO));
	if (pCB)
	{
		pCB->setActive(ClientCfg.VREnable);
		if (ClientCfg.VREnable)
		{
			nldebug("pCB ok");
			cacheStereoDisplayDevices();
			pCB->resetTexts();
			sint32 selectedDevice = -1;
			for (uint i = 0; i < VRDeviceCache.size(); ++i)
			{
				std::stringstream displayname;
				displayname << std::string("[") << VRDeviceCache[i].first << "] [" << VRDeviceCache[i].second << "]";
				pCB->addText(ucstring(displayname.str()));
				if (ClientCfg.VRDisplayDevice == VRDeviceCache[i].first)
				{
					if (selectedDevice == -1 || ClientCfg.VRDisplayDeviceId == VRDeviceCache[i].second)
					{
						selectedDevice = i;
					}
				}
			}
			if (selectedDevice == -1)
			{
				// configured device not found, add a dummy
				std::stringstream displayname;
				displayname << std::string("[") << ClientCfg.VRDisplayDevice << "] [" << ClientCfg.VRDisplayDeviceId<< "] [DEVICE NOT FOUND]";
				pCB->addText(ucstring(displayname.str()));
				selectedDevice = VRDeviceCache.size();
			}
			NLGUI::CDBManager::getInstance()->getDbProp(GAME_CONFIG_VR_DEVICE_DB)->setValue32(-1);
			NLGUI::CDBManager::getInstance()->getDbProp(GAME_CONFIG_VR_DEVICE_DB)->setValue32(selectedDevice);
		}
	}
}

// ***************************************************************************
class CHandlerGameConfigInit : public IActionHandler
{
public:
	// Value used to restore the screen AR in case of a cancel
	static float	BkupScreenAspectRatio;

	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (Driver == NULL) return;

		VideoModes.clear();
		vector<string> stringModeList;

		sint nFoundMode = getRyzomModes(VideoModes, stringModeList);

		// Initialize interface combo box
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBGroupComboBox *pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_MODES_COMBO ));
		if( pCB )
		{
			pCB->resetTexts();
			for (sint j = 0; j < (sint)stringModeList.size(); j++)
				pCB->addText(ucstring(stringModeList[j]));
		}
		// -1 is important to indicate we set this value in edit mode
		NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_MODE_DB )->setValue32(-1);
		NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_FREQ_DB )->setValue32(-1);
		NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_MODE_DB )->setValue32(nFoundMode);

		CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_FULLSCREEN_BUTTON ));
		if (pBut)
		{
			pBut->setPushed(!ClientCfg.Windowed);
		}
		CAHManager::getInstance()->runActionHandler("game_config_change_vid_fullscreen",NULL);

		// **** Init Texture Size Modes
		// init the combo box, according to Texture Installed or not
		pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_TEXTURE_MODE_COMBO ));
		if( pCB )
		{
			pCB->resetTexts();
			pCB->addText(CI18N::get("uigcLowTextureMode"));
			pCB->addText(CI18N::get("uigcNormalTextureMode"));
			if(ClientCfg.HDTextureInstalled)
				pCB->addText(CI18N::get("uigcHighTextureMode"));
		}

		// VR_CONFIG
		updateVRDevicesComboUI();

		// init the mode in DB
		TTextureMode	texMode;
		if(ClientCfg.DivideTextureSizeBy2)
			texMode= LowTextureMode;
		else if(ClientCfg.HDEntityTexture && ClientCfg.HDTextureInstalled)
			texMode= HighTextureMode;
		else
			texMode= NormalTextureMode;
		// -1 is important to indicate we set this value in edit mode
		NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_TEXTURE_MODE_DB )->setValue32(-1);
		NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_TEXTURE_MODE_DB )->setValue32(texMode);

		// **** Init Screen Aspect Ratio
		// Init the combo box, according to the value
		pCB= dynamic_cast<CDBGroupComboBox*>( CWidgetManager::getInstance()->getElementFromDefine( "game_config_screen_ratio_cb" ));
		if(pCB)
		{
			// Bkup for cancel
			BkupScreenAspectRatio= ClientCfg.ScreenAspectRatio;

			// -1 is here to force var change
			NLGUI::CDBManager::getInstance()->getDbProp( "UI:TEMP:SCREEN_RATIO_MODE" )->setValue32(-1);

			// detect with an epsilon to avoid float precision problems
			if(fabs(ClientCfg.ScreenAspectRatio - 1.33333f)<=0.00001f)
				NLGUI::CDBManager::getInstance()->getDbProp( "UI:TEMP:SCREEN_RATIO_MODE" )->setValue32(0);		// 4/3
			else if(fabs(ClientCfg.ScreenAspectRatio - 1.77777f)<=0.00001f)
				NLGUI::CDBManager::getInstance()->getDbProp( "UI:TEMP:SCREEN_RATIO_MODE" )->setValue32(1);		// 16/9
			else if(ClientCfg.ScreenAspectRatio == 0.f)
				NLGUI::CDBManager::getInstance()->getDbProp( "UI:TEMP:SCREEN_RATIO_MODE" )->setValue32(3);		// Auto
			else
				NLGUI::CDBManager::getInstance()->getDbProp( "UI:TEMP:SCREEN_RATIO_MODE" )->setValue32(2);		// Custom
		}

		// **** Init Language : look in game_config.lua
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigInit, "game_config_init");
float	CHandlerGameConfigInit::BkupScreenAspectRatio= 1.3333f;


// ***************************************************************************
class CHandlerGameConfigMode : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		sint oldVideoMode= NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_MODE_DB )->getOldValue32();
		sint nVideModeNb = NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_MODE_DB )->getValue32();
		if (nVideModeNb == -1 || oldVideoMode == -1) return;

		CDBGroupComboBox *pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_MODES_COMBO ));
		if( pCB == NULL ) return;

		// Get W, H
		sint w,h;
		{
			string vidModeStr = pCB->getText(nVideModeNb).toString();
			string tmp = vidModeStr.substr(0,vidModeStr.find('x')-1);
			fromString(tmp, w);
			tmp = vidModeStr.substr(vidModeStr.find('x')+2,vidModeStr.size());
			fromString(tmp, h);
		}

		// Filter VideoModes list to get freqs
		vector<string> stringFreqList;
		sint i, j, nFoundFreq = -1;
		for (i=0; i < (sint)VideoModes.size(); ++i)
		{
			if ((VideoModes[i].Width == w) && (VideoModes[i].Height == h) && (VideoModes[i].Depth == GAME_CONFIG_VIDEO_DEPTH_REQ))
			{
				bool bFound = false;
				string tmp = toString(VideoModes[i].Frequency);
				for (j = 0; j < (sint)stringFreqList.size(); ++j)
					if (stringFreqList[j] == tmp)
					{
						bFound = true;
						break;
					}
					if (!bFound)
					{
						stringFreqList.push_back(tmp);
						if (ClientCfg.Frequency == VideoModes[i].Frequency)
							nFoundFreq = j;
					}
			}
		}
		if (nFoundFreq == -1) nFoundFreq = 0;
		// Initialize interface combo box
		pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_FREQS_COMBO ));
		if( pCB )
		{
			pCB->resetTexts();
			for (j = 0; j < (sint)stringFreqList.size(); j++)
				pCB->addText(ucstring(stringFreqList[j]) + " Hz");
		}
		NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_FREQ_DB )->setValue32(nFoundFreq);

		// **** dirt the apply button of the DDX
		// don't do it at init!
		if(oldVideoMode!=-1 && nVideModeNb!=-1)
		{
			CDDXManager *pDM = CDDXManager::getInstance();
			CInterfaceDDX *pDDX = pDM->get(GAME_CONFIG_DDX);
			if(pDDX)
				pDDX->validateApplyButton();
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigMode, "game_config_change_vid_mode");

// ***************************************************************************
class CHandlerGameConfigLanguage : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		sint old = NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_LANGUAGE )->getOldValue32();
		sint newOne = NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_LANGUAGE )->getValue32();
		if ((old != -1) && (newOne != -1))
		{
			// Set the new language
			//string newVal = (newOne==2)?"de":(newOne==1)?"fr":"en";
			string newVal = convertLanguageIntToLanguageCode(newOne);
			if (ClientCfg.LanguageCode != newVal)
			{
				CDDXManager *pDM = CDDXManager::getInstance();
				CInterfaceDDX *pDDX = pDM->get(GAME_CONFIG_DDX);
				if(pDDX)
					pDDX->validateApplyButton();
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigLanguage, "game_config_change_language");

// ***************************************************************************
class CHandlerGameConfigFreq : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		sint	oldFreq= NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_FREQ_DB )->getOldValue32();
		sint	newFreq= NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_FREQ_DB )->getValue32();

		// dirt the apply button of the DDX.
		// don't do it at init!
		if(oldFreq!=-1 && newFreq!=-1)
		{
			CDDXManager *pDM = CDDXManager::getInstance();
			CInterfaceDDX *pDDX = pDM->get(GAME_CONFIG_DDX);
			if(pDDX)
				pDDX->validateApplyButton();
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigFreq, "game_config_change_vid_freq");

// ***************************************************************************
class CHandlerGameConfigTextureMode : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCalller */, const string &/* Params */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		sint	oldTMode= NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_TEXTURE_MODE_DB )->getOldValue32();
		sint	newTMode= NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_TEXTURE_MODE_DB )->getValue32();

		// dirt the apply button of the DDX
		// don't do it at init!
		if(oldTMode!=-1 && newTMode!=-1)
		{
			CDDXManager *pDM = CDDXManager::getInstance();
			CInterfaceDDX *pDDX = pDM->get(GAME_CONFIG_DDX);
			if(pDDX)
				pDDX->validateApplyButton();
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigTextureMode, "game_config_change_texture_mode");

// ***************************************************************************
class CHandlerGameConfigFullscreen : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		bool bFullscreen = false;
		{
			CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_FULLSCREEN_BUTTON ));
			if (pBut) bFullscreen = pBut->getPushed();
		}
		CDBGroupComboBox *pCB;
		if (bFullscreen)
		{
			// show modes combo
			pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_MODES_COMBO ));
			if (pCB) pCB->setActive(true);

			// show frequencies combo
			pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_FREQS_COMBO ));
			if (pCB) pCB->setActive(true);
		}
		else
		{
			// hide modes combo
			pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_MODES_COMBO ));
			if (pCB) pCB->setActive(false);

			// hide frequencies combo
			pCB= dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_FREQS_COMBO ));
			if (pCB) pCB->setActive(false);
		}

		// **** dirt the apply button of the DDX
		// if caller is NULL, come from init, so don't validate the apply button
		if(pCaller)
		{
			CDDXManager *pDM = CDDXManager::getInstance();
			CInterfaceDDX *pDDX = pDM->get(GAME_CONFIG_DDX);
			if(pDDX)
				pDDX->validateApplyButton();
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigFullscreen, "game_config_change_vid_fullscreen");

// ***************************************************************************
class CHandlerGameConfigVRDevice : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		// VR_CONFIG

		sint oldDevice = NLGUI::CDBManager::getInstance()->getDbProp(GAME_CONFIG_VR_DEVICE_DB)->getOldValue32();
		sint newDevice = NLGUI::CDBManager::getInstance()->getDbProp(GAME_CONFIG_VR_DEVICE_DB)->getValue32();

		if (oldDevice != -1 && newDevice != -1 && pCaller)
		{
			// nldebug("TODO_VR switch vr device (from combo box)");

			CDDXManager *pDM = CDDXManager::getInstance();
			CInterfaceDDX *pDDX = pDM->get(GAME_CONFIG_DDX);
			if(pDDX)
				pDDX->validateApplyButton();
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigVRDevice, "game_config_change_vr_device");

// ***************************************************************************
class CHandlerGameConfigApply : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// **** Apply the video mode
		sint nVideModeNb = NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_MODE_DB )->getValue32();
		if (nVideModeNb != -1)
		{
			sint nVideoFreqNb = NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_VIDEO_FREQ_DB )->getValue32();
			if (nVideoFreqNb != -1)
			{
				// Get W, H
				sint w = 1024, h = 768;
				{
					CDBGroupComboBox *pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_MODES_COMBO ));
					if( pCB != NULL )
					{
						string vidModeStr = pCB->getText(nVideModeNb).toString();
						string tmp = vidModeStr.substr(0,vidModeStr.find('x')-1);
						fromString(tmp, w);
						tmp = vidModeStr.substr(vidModeStr.find('x')+2,vidModeStr.size());
						fromString(tmp, h);
					}
				}

				// Get Frequency
				sint freq = 60;
				{
					CDBGroupComboBox *pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_FREQS_COMBO ));
					if( pCB != NULL )
					{
						string vidFreqStr = pCB->getText(nVideoFreqNb).toString();
						fromString(vidFreqStr, freq);
					}
				}

				// Get Fullscreen
				bool bFullscreen = false;
				{
					CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId( GAME_CONFIG_VIDEO_FULLSCREEN_BUTTON ));
					if (pBut != NULL)
						bFullscreen = pBut->getPushed();
				}

				ClientCfg.Windowed = !bFullscreen;

				UDriver::CMode screenMode;
				Driver->getCurrentScreenMode(screenMode);

				if (bFullscreen)
				{
					ClientCfg.Depth = screenMode.Depth;
					ClientCfg.Frequency = freq;
				}
				else
				{
					uint32 width, height;
					Driver->getWindowSize(width, height);

					// window is too large
					if (width >= screenMode.Width || height >= screenMode.Height)
					{
						// choose a smaller size
						w = 1024;
						h = 768;
					}
					else
					{
						// take previous mode
						w = width;
						h = height;
					}
				}

				ClientCfg.Width = w;
				ClientCfg.Height = h;

				// Write the modified client.cfg
				ClientCfg.writeBool("FullScreen", bFullscreen);
				ClientCfg.writeInt("Width", w);
				ClientCfg.writeInt("Height", h);

				if (bFullscreen)
				{
					ClientCfg.writeInt("Depth", screenMode.Depth);
					ClientCfg.writeInt("Frequency", freq);
				}
			}
		}

		if (ClientCfg.VREnable)
		{
			// store the new config variables
			sint deviceIdx = NLGUI::CDBManager::getInstance()->getDbProp(GAME_CONFIG_VR_DEVICE_DB)->getValue32();
			ClientCfg.VRDisplayDevice = VRDeviceCache[deviceIdx].first;
			ClientCfg.VRDisplayDeviceId = VRDeviceCache[deviceIdx].second;
			ClientCfg.writeString("VRDisplayDevice", VRDeviceCache[deviceIdx].first);
			ClientCfg.writeString("VRDisplayDeviceId", VRDeviceCache[deviceIdx].second);
		}

		bool requestReboot = false;

		// **** Apply the texture mode
		sint nTexMode = NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_TEXTURE_MODE_DB )->getValue32();
		if (nTexMode>=0 && nTexMode<=HighTextureMode)
		{
			if ((ClientCfg.DivideTextureSizeBy2 != (nTexMode==LowTextureMode)) ||
				(ClientCfg.HDEntityTexture != (nTexMode==HighTextureMode)))
			{
				ClientCfg.DivideTextureSizeBy2= nTexMode==LowTextureMode;
				ClientCfg.HDEntityTexture= nTexMode==HighTextureMode;
				ClientCfg.writeInt("DivideTextureSizeBy2", ClientCfg.DivideTextureSizeBy2);
				ClientCfg.writeInt("HDEntityTexture", ClientCfg.HDEntityTexture);
				requestReboot = true;
			}
		}

		// *** Apply the Screen AR
		// since already set in the config file, need only to bkup the current version
		CHandlerGameConfigInit::BkupScreenAspectRatio= ClientCfg.ScreenAspectRatio;

		// *** Apply the language code
		// only if not in "work" language mode (else strange requestReboot)
		if(ClientCfg.LanguageCode!="wk")
		{
			sint newOne = NLGUI::CDBManager::getInstance()->getDbProp( GAME_CONFIG_LANGUAGE )->getValue32();
			//string newVal = (newOne==2)?"de":(newOne==1)?"fr":"en";
			string newVal = convertLanguageIntToLanguageCode(newOne);
			if (ClientCfg.LanguageCode != newVal)
			{
				ClientCfg.LanguageCode = newVal;
				ClientCfg.writeString("LanguageCode", ClientCfg.LanguageCode);
				requestReboot = true;
			}
		}

		// Apply the NPC icon display mode
		CNPCIconCache::getInstance().setEnabled(!ClientCfg.R2EDEnabled && NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:INSCENE:FRIEND:MISSION_ICON")->getValueBool());

		// Request a reboot
		if (requestReboot)
			pIM->messageBox (CI18N::get ("uigcRequestReboot"));

		// **** Save the config
		if (ClientCfg.SaveConfig)
			ClientCfg.ConfigFile.save ();
		ClientCfg.IsInvalidated = true;
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigApply, "game_config_apply");


// ***************************************************************************
class CHandlerGameConfigCancel: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Has something special to do only with screen aspect ratio:
		//	- Video Modes changes are only really validated at apply times so cancel => noop
		//	- All other standard changes are handled upper by ddx_cancel AH called

		// restore the bkuped Screen AR and invalidate clientcfg (NB: don't need to save, since canceled)
		ClientCfg.ScreenAspectRatio= CHandlerGameConfigInit::BkupScreenAspectRatio;
		ClientCfg.writeDouble("ScreenAspectRatio", ClientCfg.ScreenAspectRatio);
		ClientCfg.IsInvalidated = true;
	}
};
REGISTER_ACTION_HANDLER(CHandlerGameConfigCancel, "game_config_cancel");


// ***************************************************************************
class CHandlerGameConfigChangeScreenRatioMode : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (CInterfaceLink::isUpdatingAllLinks()) return; // don't want to trash the value in client.cfg at init, due to 'updateAllLinks' being called

		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// get the current mode
		sint	oldMode= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:SCREEN_RATIO_MODE")->getOldValue32();
		sint	mode= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:SCREEN_RATIO_MODE")->getValue32();
		if(mode<0 || mode>3)
			return;

		// If predef value (4/3 or 16/9)
		if(mode!=2)
		{
			switch(mode)
			{
			case 0: ClientCfg.ScreenAspectRatio= 1.33333f; break;
			case 1: ClientCfg.ScreenAspectRatio= 1.77777f; break;
			case 3: ClientCfg.ScreenAspectRatio= 0.f; break;
			}
			ClientCfg.writeDouble("ScreenAspectRatio", ClientCfg.ScreenAspectRatio);

			// set content, and freeze the edit box
			CGroupEditBox	*eb= dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromDefine("game_config_screen_ratio_eb"));
			if(eb)
			{
				eb->setFrozen(true);
				eb->setInputStringAsFloat(ClientCfg.ScreenAspectRatio);
			}
		}
		// custom value
		else
		{
			// just unfreeze the edit box, and set correct value
			CGroupEditBox	*eb= dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromDefine("game_config_screen_ratio_eb"));
			if(eb)
			{
				eb->setFrozen(false);
				eb->setInputStringAsFloat(ClientCfg.ScreenAspectRatio);
			}
		}

		// dirt the apply button of the DDX.
		// don't do it at init!
		if(oldMode!=-1)
		{
			CDDXManager *pDM = CDDXManager::getInstance();
			CInterfaceDDX *pDDX = pDM->get(GAME_CONFIG_DDX);
			if(pDDX)
				pDDX->validateApplyButton();
		}

		// Invalidate the config
		ClientCfg.IsInvalidated = true;
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigChangeScreenRatioMode, "game_config_change_screen_ratio_mode");


// ***************************************************************************
class CHandlerGameConfigChangeScreenRatioCustom : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		sint	mode= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:SCREEN_RATIO_MODE")->getValue32();
		if (mode != 2) return;
		CGroupEditBox	*eb= dynamic_cast<CGroupEditBox*>( CWidgetManager::getInstance()->getElementFromDefine("game_config_screen_ratio_eb"));
		if(eb)
		{
			// validate the value
			float	val= eb->getInputStringAsFloat();
			clamp(val, 0.1f, 10.f);
			// reset (to clamp precision also), and hence reget
			eb->setInputStringAsFloat(val);
			val= eb->getInputStringAsFloat();

			// change the CFG with this value
			ClientCfg.ScreenAspectRatio= val;
			ClientCfg.writeDouble("ScreenAspectRatio", ClientCfg.ScreenAspectRatio);

			// dirt the apply button of the DDX.
			{
				CDDXManager *pDM = CDDXManager::getInstance();
				CInterfaceDDX *pDDX = pDM->get(GAME_CONFIG_DDX);
				if(pDDX)
					pDDX->validateApplyButton();
			}

			// Invalidate the config
			ClientCfg.IsInvalidated = true;
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameConfigChangeScreenRatioCustom, "game_config_change_screen_ratio_custom");


// ***************************************************************************
class CHandlerGameMissionAbandon : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		uint8 nMissionNb;
		fromString(Params, nMissionNb);

		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MISSION_ABANDON_BUTTON",false);
		if (pNL != NULL) pNL->setValue64(0);

		sendMsgToServer("JOURNAL:MISSION_ABANDON", nMissionNb);
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameMissionAbandon, "mission_abandon");


// ***************************************************************************
class CHandlerGameGroupMissionAbandon : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		uint8 nMissionNb;
		fromString(Params, nMissionNb);

		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MISSION_ABANDON_BUTTON",false);
		if (pNL != NULL) pNL->setValue64(0);

		sendMsgToServer("JOURNAL:GROUP_MISSION_ABANDON", nMissionNb);
	}
};
REGISTER_ACTION_HANDLER (CHandlerGameGroupMissionAbandon, "group_mission_abandon");


// ***************************************************************************
// ***************************************************************************
// Yoyo New BotChat
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// GCM Choose ZoneCharge
// ***************************************************************************
class CHandlerContextChooseZoneCharge : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendBotChatStart("START_CHOOSE_DUTY");
		BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::ZCCharge);
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextChooseZoneCharge, "context_choose_zc_charge");


// ***************************************************************************
// GCM Choose Building
// ***************************************************************************
class CHandlerContextChooseBuilding : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Choose a building is like choose a mission
		sendBotChatStart("START_CHOOSE_MISSION");
		BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::Building);
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextChooseBuilding, "context_choose_building");

// ***************************************************************************
// GCM Buy RM
// ***************************************************************************
class CHandlerContextBuyRM : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Buy RM is like choose a mission
		sendBotChatStart("START_CHOOSE_MISSION");
		BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::RMBuy);
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextBuyRM, "context_buy_rm");

// ***************************************************************************
// GCM Upgrade RM
// ***************************************************************************
class CHandlerContextUpgradeRM : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Upgrde RM is like choose a mission
		sendBotChatStart("START_CHOOSE_MISSION");
		BotChatPageAll->ChooseMission->setMissionClientType(MISSION_DESC::RMUpgrade);
		CBotChatManager::getInstance()->setCurrPage(BotChatPageAll->ChooseMission);
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextUpgradeRM, "context_upgrade_rm");


// ***************************************************************************
// GCM Cancel Duty
// ***************************************************************************
class CHandlerContextCancelZoneCharge : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Check if user really want.
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQCancelZoneCharge"),
			"do_cancel_zc_charge");
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextCancelZoneCharge, "context_cancel_zc_charge");

// The Msg if Ok.
class CHandlerDoCancelZoneCharge : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendMsgToServer("BOTCHAT:DUTY_CANCEL_APPLY");
	}
};
REGISTER_ACTION_HANDLER(CHandlerDoCancelZoneCharge, "do_cancel_zc_charge");


// ***************************************************************************
// GCM Destroy Building
// ***************************************************************************
class CHandlerContextDestroyBuilding : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// Check if user really want.
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQDestroyBuilding"),
			"do_destroy_building");
	}
};
REGISTER_ACTION_HANDLER(CHandlerContextDestroyBuilding, "context_destroy_building");

// The Msg if Ok.
class CHandlerDoDestroyBuilding : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendMsgToServer("BOTCHAT:DESTROY_BUILDING");
	}
};
REGISTER_ACTION_HANDLER(CHandlerDoDestroyBuilding, "do_destroy_building");


// ***************************************************************************
// Combat defense interface handlers
// ***************************************************************************
// Select Parry.
class CHandlerSelectParry : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendMsgToServer("COMBAT:PARRY");

		// display parry mode msg
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		ucstring msg = CI18N::get("msgUserModeParry");
		string cat = getStringCategory(msg, msg);
		pIM->displaySystemInfo(msg, cat);
	}
};
REGISTER_ACTION_HANDLER(CHandlerSelectParry, "parry");

// Select dodge.
class CHandlerSelectDodge : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		sendMsgToServer("COMBAT:DODGE");

		// display dodge mode msg
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		ucstring msg = CI18N::get("msgUserModeDodge");
		string cat = getStringCategory(msg, msg);
		pIM->displaySystemInfo(msg, cat);
	}
};
REGISTER_ACTION_HANDLER(CHandlerSelectDodge, "dodge");

// Select protected slot.
class CHandlerSelectProtectedSlot : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		uint8 slot = (uint8)SLOT_EQUIPMENT::stringToSlotEquipment(sParams);
		sendMsgToServer("COMBAT:PROTECTED_SLOT", slot);
	}
};
REGISTER_ACTION_HANDLER(CHandlerSelectProtectedSlot, "select_protected_slot");


// ***************************************************************************
// Tooltips for Players Stats, with values printed.
// ***************************************************************************


// ***************************************************************************
// Common code
//static	void	fillPlayerBarText(ucstring &str, const string &dbScore, const string &dbScoreMax, const string &ttFormat)
static	void	fillPlayerBarText(ucstring &str, const string &dbScore, SCORES::TScores score, const string &ttFormat)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf		*node;

	sint	val= 0;
	sint	maxVal= 0;
	// Get from local database cause written from CBarManager, from a fast impulse
	node= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:" + dbScore, false);
	if(node)	val= node->getValue32();
	// less accurate/speed Max transferred by DB
	node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SCORES%d:", score), false);
	if(node)	maxVal= node->getValue32();
	val= max(val, 0);
	maxVal= max(maxVal, 0);

	str= CI18N::get(ttFormat);
	strFindReplace(str, "%v", toString(val) );
	strFindReplace(str, "%m", toString(maxVal) );
}

// ***************************************************************************
// Life
class CHandlerPlayerTTLife : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		ucstring	str;
		fillPlayerBarText(str, "HP", SCORES::hit_points, "uittPlayerLifeFormat");

		CWidgetManager::getInstance()->setContextHelpText(str);
	}
};
REGISTER_ACTION_HANDLER(CHandlerPlayerTTLife, "player_tt_life");

// ***************************************************************************
// Stamina
class CHandlerPlayerTTStamina : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		ucstring	str;
		fillPlayerBarText(str, "STA", SCORES::stamina, "uittPlayerStaminaFormat");

		CWidgetManager::getInstance()->setContextHelpText(str);
	}
};
REGISTER_ACTION_HANDLER(CHandlerPlayerTTStamina, "player_tt_stamina");

// ***************************************************************************
// Sap
class CHandlerPlayerTTSap : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		ucstring	str;
		fillPlayerBarText(str, "SAP", SCORES::sap, "uittPlayerSapFormat");

		CWidgetManager::getInstance()->setContextHelpText(str);
	}
};
REGISTER_ACTION_HANDLER(CHandlerPlayerTTSap, "player_tt_sap");

// ***************************************************************************
// Focus
class CHandlerPlayerTTFocus : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		ucstring	str;
		fillPlayerBarText(str, "FOCUS", SCORES::focus, "uittPlayerFocusFormat");

		CWidgetManager::getInstance()->setContextHelpText(str);
	}
};
REGISTER_ACTION_HANDLER(CHandlerPlayerTTFocus, "player_tt_focus");

// ***************************************************************************
// Bulk: NB: work for player / animal
class CHandlerGetTTBulk : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		string	dbBranch= getParam(sParams, "dbbranch");
		string	dbMax= getParam(sParams, "dbmax");

		// Get the sum of the bulk for this db branch
		const double epsilon = 0.001;
		sint32 val = sint32(CInventoryManager::getBranchBulk(dbBranch, 0, 10000) + epsilon);

		// Get the Max value
		sint32	maxVal= 0;
		CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(dbMax, false);
		if(node)
			maxVal= node->getValue32();

		// Replace in the formated text
		ucstring	str= CI18N::get("uittBulkFormat");
		strFindReplace(str, "%v", toString(val) );
		strFindReplace(str, "%m", toString(maxVal) );
		CWidgetManager::getInstance()->setContextHelpText(str);
	}
};
REGISTER_ACTION_HANDLER(CHandlerGetTTBulk, "get_tt_bulk");

// ***************************************************************************
#define UI_MISSION_LIST "ui:interface:info_player_journal:content:mission_list"

uint32 getMissionTitle(sint32 nSelected)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	sint32 nNbMission, nNbGroupMission;
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("ipj_nb_mission"), nNbMission);
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("ipj_nb_group_mission"), nNbGroupMission);

	if (nSelected < 0)
		return 0;
	else if (nSelected < nNbMission)
		return NLGUI::CDBManager::getInstance()->getDbProp("SERVER:MISSIONS:"+toString(nSelected)+":TITLE")->getValue32();
	else if (nSelected < (nNbMission+nNbGroupMission))
		return NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GROUP:MISSIONS:"+toString(nSelected-nNbMission)+":TITLE")->getValue32();
	return 0;
}

void runMissionProc(sint32 nSelected)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	sint32 nNbMission, nNbGroupMission;
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("ipj_nb_mission"), nNbMission);
	fromString(CWidgetManager::getInstance()->getParser()->getDefine("ipj_nb_group_mission"), nNbGroupMission);

	if (nSelected < 0)
		return;
	else if (nSelected < nNbMission)
	{
		string sButtonPath = UI_MISSION_LIST ":b_title"+toString(nSelected);
		CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(CWidgetManager::getInstance()->getElementFromId(sButtonPath));
		CAHManager::getInstance()->runActionHandler("proc", pCB, "mission_proc_title|"+toString(nSelected));
	}
	else if (nSelected < (nNbMission+nNbGroupMission))
	{
		string sButtonPath = UI_MISSION_LIST ":b_group_title"+toString(nSelected-nNbMission);
		CCtrlButton *pCB = dynamic_cast<CCtrlButton*>(CWidgetManager::getInstance()->getElementFromId(sButtonPath));
		CAHManager::getInstance()->runActionHandler("proc", pCB, "group_mission_proc_title|"+toString(nSelected-nNbMission));
	}
	return;
}


class CHandlerMissionChooseNextValid : public IActionHandler
{
public:
	void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		sint32 nSelected = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:MISSION_SELECTED")->getValue32();

		sint32 nNbMission, nNbGroupMission;
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("ipj_nb_mission"), nNbMission);
		fromString(CWidgetManager::getInstance()->getParser()->getDefine("ipj_nb_group_mission"), nNbGroupMission);

		// If no mission selected or title selected becomes invalid -> search a new title to select
		if ((nSelected == -1) || (getMissionTitle(nSelected) == 0))
		{
			bool bFound = false;
			sint32 nSel = nSelected;

			// From the currently selected mission go backward (in the display order)
			if (nSel != -1)
			while (!bFound)
			{
				nSel--;
				if (nSel < 0)
					break;

				// Is there a mission at the position nSel ? Yes ok select !
				if (getMissionTitle(nSel) != 0)
				{
					runMissionProc(nSel);
					return;
				}
			}
			// Ok not found a mission in backward mode try now in forward mode until the end of the mission list
			while (!bFound)
			{
				nSel++;
				if (nSel == (nNbMission+nNbGroupMission))
					break;

				// Is there a mission at the position nSel ? Yes ok select !
				if (getMissionTitle(nSel) != 0)
				{
					runMissionProc(nSel);
					return;
				}
			}
			// No mission at all found so there is no mission in the journal
			NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:MISSION_SELECTED")->setValue32(-1);
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerMissionChooseNextValid, "mission_choose_next_valid");

// ***************************************************************************
// Output a text above the entity like hit points
class CHandlerEntityFlyingText : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		string	text = getParam(sParams, "text");

		uint entity;
		fromString(getParam(sParams, "entity"), entity);

		CRGBA color = CRGBA::stringToRGBA(getParam(sParams, "color").c_str());
		if (entity < 256)
			EntitiesMngr.entity (entity)->addHPOutput (CI18N::get (text), color);
	}
};
REGISTER_ACTION_HANDLER(CHandlerEntityFlyingText, "entity_flying_text");


// ***************************************************************************
// play an event music
class CHandlerPlayEventMusic : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		bool loop = getParam(sParams, "loop")=="1";

		uint xFade;
		fromString(getParam(sParams, "xfade"), xFade);

		string fileName = getParam(sParams, "music");

		// don't play if db is in init stage
		if (IngameDbMngr.initInProgress()) return;

		if(SoundMngr)
			SoundMngr->playEventMusic(fileName, xFade, loop);
	}
};
REGISTER_ACTION_HANDLER(CHandlerPlayEventMusic, "play_event_music");


// ***************************************************************************
// stop an event music
class CHandlerStopEventMusic : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		uint xFade;
		fromString(getParam(sParams, "xfade"), xFade);

		string		fileName= getParam(sParams, "music");

		// don't play if db is in init stage
		if (IngameDbMngr.initInProgress()) return;

		if(SoundMngr)
			SoundMngr->stopEventMusic(fileName, xFade);
	}
};
REGISTER_ACTION_HANDLER(CHandlerStopEventMusic, "stop_event_music");

// ***************************************************************************
// enter cr zone for queue
class CEnterCRZone: public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// hide interface
		CInterfaceManager* pIM = CInterfaceManager::getInstance();
		CInterfaceGroup *pIG = (CInterfaceGroup*)CWidgetManager::getInstance()->getElementFromId ("ui:interface:enter_crzone_proposal");
		if(pIG)
			pIG->setActive(false);

		bool accept;
		fromString(sParams, accept);
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream("MISSION:ENTER_CRITICAL", out))
		{
			//nlinfo("impulseCallBack :MISSION:ENTER_CRITICAL sent");
			out.serial(accept);
			NetMngr.push(out);
		}
		else
		{
			nlwarning("command : unknown message name : MISSION:ENTER_CRITICAL");
		}
	}
};
REGISTER_ACTION_HANDLER(CEnterCRZone, "enter_crzone");


// ***************************************************************************
// wake player
class CWakeForMission : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		uint8 nMissionNb;
		fromString(sParams, nMissionNb);

		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MISSION_WAKE_BUTTON",false);
		if (pNL != NULL) pNL->setValue64(0);

		sendMsgToServer("MISSION:WAKE", nMissionNb);
	}
};
REGISTER_ACTION_HANDLER(CWakeForMission, "mission_wake");

// ***************************************************************************
// wake player
class CWakeForGroupMission : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		uint8 nMissionNb;
		fromString(sParams, nMissionNb);

		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MISSION_WAKE_BUTTON",false);
		if (pNL != NULL) pNL->setValue64(0);

		sendMsgToServer("MISSION:GROUP_WAKE", nMissionNb);
	}
};
REGISTER_ACTION_HANDLER(CWakeForGroupMission, "group_mission_wake");


// ***************************************************************************
// ***************************************************************************
// PVP FACTION
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
class CBuildTotem : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if (UserEntity->selection())
		{
			CEntityCL *pSel = EntitiesMngr.entity(UserEntity->selection());
			if (pSel != NULL)
				UserEntity->moveToTotemBuildingPhrase( UserEntity->selection(), 2.0f, std::numeric_limits<uint>::max(), std::numeric_limits<uint>::max(), true);
		}
	}
};
REGISTER_ACTION_HANDLER(CBuildTotem, "build_totem");


// ***************************************************************************
class CHandlerFameSetNeutral : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		if( IngameDbMngr.initInProgress() ) return;

		bool	force;
		fromString(getParam(sParams, "force"), force);

		bool	isCiv= getParam(sParams, "type")=="civ" || getParam(sParams, "type")=="civ_guild";
		bool	isGuild= getParam(sParams, "type")=="civ_guild" || getParam(sParams, "type")=="cult_guild";

		if(force)
		{
			// help the server to know what type of neutral to set (civ or cult). ugly
			uint8 u8Type= isCiv?PVP_CLAN::Fyros:PVP_CLAN::Kami;
			// send msg to server.
			if(isGuild)
				sendMsgToServer("PVP:SET_NEUTRAL_ALLEGIANCE_GUILD", u8Type);
			else
				sendMsgToServer("PVP:SET_NEUTRAL_ALLEGIANCE", u8Type);
		}
		else
		{
			// confirm first
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			if(isGuild)
			{
				pIM->validMessageBox(CInterfaceManager::WarningIconMsg,
					isCiv?CI18N::get("uiFameAllegianceCivSetNeutralGuildWarning"):
						CI18N::get("uiFameAllegianceCultSetNeutralGuildWarning"),
					"fame_set_neutral",
					isCiv?"type=civ_guild|force=1":"type=cult_guild|force=1" );
			}
			else
			{
				pIM->validMessageBox(CInterfaceManager::WarningIconMsg,
					isCiv?CI18N::get("uiFameAllegianceCivSetNeutralWarning"):
						CI18N::get("uiFameAllegianceCultSetNeutralWarning"),
					"fame_set_neutral",
					isCiv?"type=civ|force=1":"type=cult|force=1" );
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerFameSetNeutral, "fame_set_neutral");


// ***************************************************************************
class CHandlerConfigureQuitDialogBox : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager::getInstance()->configureQuitDialogBox();
	}
};
REGISTER_ACTION_HANDLER(CHandlerConfigureQuitDialogBox, "configure_quit_dialog_box");

// ----------------------------------------------------------------------------
static bool isSwimming()
{
	if (UserEntity != NULL)
		return (UserEntity->mode() == MBEHAV::SWIM || UserEntity->mode() == MBEHAV::MOUNT_SWIM);
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

// ***************************************************************************

class CHandlerEmote : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// An emote is 2 things : a phrase and an animation
		// Phrase is the phrase that server returns in chat system
		// Behav is the animation played
		// CustomPhrase is an user phrase which can replace default phrase
		string sPhraseNb = getParam(sParams, "nb");
		string sBehav = getParam(sParams, "behav");
		string sCustomPhrase = getParam(sParams, "custom_phrase");

		uint32 phraseNb;
		fromString(sPhraseNb, phraseNb);
		uint8 behaviour;
		fromString(sBehav, behaviour);

		MBEHAV::EBehaviour behavToSend = (MBEHAV::EBehaviour)(MBEHAV::EMOTE_BEGIN + behaviour);
		uint16 phraseNbToSend = (uint16)phraseNb;

		if (EAM)
		{
			const uint nbBehav = EAM->getNbEmots(); // Miscalled: this is the number of behaviour for all emotes
			if ((behaviour >= nbBehav) || (behaviour == 255))
				behavToSend = MBEHAV::IDLE;
		}
		else
		{
			if (behaviour == 255)
				behavToSend = MBEHAV::IDLE;
		}

		/* Emotes forbidden when dead, emotes with behav forbidden when
		 * stunned or swimming */
		if ( ( behavToSend != MBEHAV::IDLE && (isSwimming() || isStunned() || isDead() ) ) )
		{
			return;
		}

		if( sCustomPhrase.empty() )
		{
			// Create the message and send.
			const string msgName = "COMMAND:EMOTE";
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
			{
				out.serialEnum(behavToSend);
				out.serial(phraseNbToSend);
				NetMngr.push(out);
				//nlinfo("impulseCallBack : %s %d %d sent", msgName.c_str(), (uint32)behavToSend, phraseNbToSend);
			}
			else
				nlwarning("command 'emote': unknown message named '%s'.", msgName.c_str());
		}
		else
		{
			// Create the message and send.
			const string msgName = "COMMAND:CUSTOM_EMOTE";
			CBitMemStream out;
			if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
			{
				ucstring ucstr;
				ucstr.fromUtf8(sCustomPhrase);

				if( sCustomPhrase == "none" )
				{
					if( behavToSend == MBEHAV::IDLE )
					{
						// display "no animation for emote"
						CInterfaceManager	*pIM= CInterfaceManager::getInstance();
						ucstring msg = CI18N::get("msgCustomizedEmoteNoAnim");
						string cat = getStringCategory(msg, msg);
						pIM->displaySystemInfo(msg, cat);
						return;
					}
				}
				else
				{
					ucstr = ucstring("&EMT&") + UserEntity->getDisplayName() + ucstring(" ") + ucstr;
				}

				out.serialEnum(behavToSend);
				out.serial(ucstr);
				NetMngr.push(out);
				//nlinfo("impulseCallBack : %s %d %s sent", msgName.c_str(), (uint32)behavToSend, sCustomPhrase.c_str());
			}
			else
				nlwarning("command 'emote': unknown message named '%s'.", msgName.c_str());
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerEmote, "emote");