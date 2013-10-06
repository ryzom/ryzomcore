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
// client
#include "../string_manager_client.h"
#include "people_interraction.h"
#include "nel/gui/interface_expr.h"
#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "action_handler_misc.h"
#include "chat_window.h"
#include "../entity_animation_manager.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_menu.h"
#include "../client_chat_manager.h"
#include "../string_manager_client.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/ctrl_text_button.h"
#include "filtered_chat_summary.h"
#include "input_handler_manager.h"
#include "../user_entity.h"
#include "../entities.h"
#include "../net_manager.h"
#include "../connection.h"
#include "nel/gui/group_tab.h"
#include "guild_manager.h"
// Game share
#include "game_share/entity_types.h"
// NeL
#include <nel/misc/command.h>
#include <nel/misc/rgba.h>
#include <nel/misc/i18n.h>

#include <string>

#include "../misc.h"

using namespace NLMISC;
using namespace std;



////////////
// EXTERN //
////////////

extern CEntityAnimationManager		*EAM;
extern CClientChatManager            ChatMngr;
extern NLMISC::CLog	g_log;

/////////////
// GLOBALS //
/////////////

CPeopleInterraction PeopleInterraction;


static const string MAIN_CHAT_SOURCE_MENU      = "ui:interface:main_chat_source_menu";
static const string USER_CHAT_SOURCE_MENU      = "ui:interface:user_chat_source_menu";
static const string STD_CHAT_SOURCE_MENU       = "ui:interface:std_chat_source_menu";
static const string NEW_PARTY_CHAT_WINDOW      = "ui:interface:create_new_party_chat";

NLMISC::CRefPtr<CChatWindow> ChatWindowForFilter;

static const sint PARTY_CHAT_SPAWN_DELTA = 20; // to avoid that all party chat appear at the same position, a random value is added

//////////////////////////////////
// STATIC FUNCTIONs DECLARATION //
//////////////////////////////////

/** Display an error msg in the system info window, and also in the last window that triggered the command (so that the user is sure to see it)
  */
static void displayVisibleSystemMsg(const ucstring &msg, const string &cat = "CHK");


//////////////////////////////
// HANDLER FOR CHAT WINDOWS //
//////////////////////////////

// handler to manage user entry in party chat windows

struct CPartyChatEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow)
	{
		if (ClientCfg.Local)
		{
			chatWindow->displayMessage(msg, CRGBA::White, CChatGroup::player, 0);
		}
		else
		{
			// TODO GAMEDEV : manage entry in the party chat
		}
	}
};



// handler to manage user entry in 'around me' window
struct CAroundMeEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow)
	{
		if (ClientCfg.Local)
		{
			chatWindow->displayMessage(msg, CRGBA::White, CChatGroup::arround, 0);
		}
		else
		{
			// process msg as usual
			ChatMngr.setChatMode(CChatGroup::arround);
			ChatMngr.chat(msg);
		}
	}
};

// handler to manage user entry in 'region' window
struct CRegionEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow)
	{
		if (ClientCfg.Local)
		{
			chatWindow->displayMessage(msg, CRGBA::White, CChatGroup::region, 0);
		}
		else
		{
			// process msg as usual
			ChatMngr.setChatMode(CChatGroup::region);
			ChatMngr.chat(msg);
		}
	}
};

// handler to manage user entry in 'universe' window
struct CUniverseEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow)
	{
		if (ClientCfg.Local)
		{
			chatWindow->displayMessage(msg, CRGBA::White, CChatGroup::universe, 0);
		}
		else
		{
			// process msg as usual
			ChatMngr.setChatMode(CChatGroup::universe);
			ChatMngr.chat(msg);
		}
	}
};

// handler to manage user entry in 'guild chat' window
struct CGuildChatEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow)
	{
		if (ClientCfg.Local)
		{
			chatWindow->displayMessage(msg, CRGBA::White, CChatGroup::guild, 0);
		}
		else
		{
			ChatMngr.setChatMode(CChatGroup::guild);
			ChatMngr.chat(msg);
		}
	}
};

// handler to manage user entry in 'team chat' window
struct CTeamChatEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow)
	{
		if (ClientCfg.Local)
		{
			chatWindow->displayMessage(msg, CRGBA::White, CChatGroup::team, 0);
		}
		else
		{
			ChatMngr.setChatMode(CChatGroup::team);
			ChatMngr.chat(msg, true);
		}
	}
};

// handler to manage user entry in a 'talk with friend' window
struct CFriendTalkEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow)
	{
		if (ClientCfg.Local)
		{
			chatWindow->displayMessage(msg, CRGBA::White, CChatGroup::player, 0);
		}
		else
		{

			// TODO GAMEDEV : send msg to other player
		}
	}
};

// handler to manage user entry in a debug console window
struct CDebugConsoleEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow * /* chatWindow */)
	{
		std::string str = msg.toString();
		NLMISC::ICommand::execute( str, g_log );
	}
};

// handler to manager user entry in a Yubo Chat
struct CYuboChatEntryHandler : public IChatWindowListener
{
	virtual void msgEntered(const ucstring &msg, CChatWindow * /* chatWindow */)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		pIM->sendStringToYuboChat(msg);
	}
};

// handler to manager user entry in a Dynamic Chat
struct CDynamicChatEntryHandler : public IChatWindowListener
{
public:
	uint	DbIndex;
	CDynamicChatEntryHandler()
	{
		DbIndex= 0;
	}

	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow)
	{
		if (ClientCfg.Local)
		{
			chatWindow->displayMessage(msg, CRGBA::White, CChatGroup::dyn_chat, DbIndex);
		}
		else
		{
			ChatMngr.setChatMode(CChatGroup::dyn_chat, ChatMngr.getDynamicChannelIdFromDbIndex(DbIndex));
			ChatMngr.chat(msg);
		}
	}
};


// handler instances
static CPartyChatEntryHandler			PartyChatEntryHandler;
static CAroundMeEntryHandler			AroundMeEntryHandler;
static CRegionEntryHandler				RegionEntryHandler;
static CUniverseEntryHandler			UniverseEntryHandler;
static CGuildChatEntryHandler			GuildChatEntryHandler;
static CTeamChatEntryHandler			TeamChatEntryHandler;
static CFriendTalkEntryHandler			FriendTalkEntryHandler;
static CDebugConsoleEntryHandler		DebugConsoleEntryHandler;
static CYuboChatEntryHandler			YuboChatEntryHandler;
static CDynamicChatEntryHandler			DynamicChatEntryHandler[CChatGroup::MaxDynChanPerPlayer];


//////////////////////
// MEMBER FUNCTIONS //
//////////////////////

//===========================================================================================================
void CChatStdInput::registerListeningWindow(CChatWindow *cw)
{
	if (!cw) return;
	Guild.addListeningWindow(cw);
	Team.addListeningWindow(cw);
	Tell.addListeningWindow(cw);
	AroundMe.addListeningWindow(cw);
	Region.addListeningWindow(cw);
	SystemInfo.addListeningWindow(cw);
	Universe.addListeningWindow(cw);
}


//===========================================================================================================
CPeopleInterraction::CPeopleInterraction() : Region(NULL),
											 Universe(NULL),
											 TeamChat(NULL),
											 GuildChat(NULL),
											 SystemInfo(NULL),
											 TellWindow(NULL),
											 DebugInfo(NULL),
											 YuboChat(NULL),
											 CurrPartyChatID(0)
{
	for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		DynamicChat[i]= NULL;
	}
}


//===========================================================================================================
void CPeopleInterraction::release()
{
	ChatInput.Tell.removeListeningPeopleList(&FriendList);
	ChatInput.Tell.removeListeningPeopleList(&TeamList);
	ChatInput.Team.removeListeningPeopleList(&TeamList);

	CChatWindowManager &cwm = getChatWndMgr();

	AroundMe.release();
	if (Region) cwm.removeChatWindow(Region);
	if (Universe) cwm.removeChatWindow(Universe);
	if (TeamChat) cwm.removeChatWindow(TeamChat);
	if (GuildChat) cwm.removeChatWindow(GuildChat);
	if (SystemInfo) cwm.removeChatWindow(SystemInfo);
	TheUserChat.release();
	if (DebugInfo) cwm.removeChatWindow(DebugInfo);
	if (YuboChat) cwm.removeChatWindow(YuboChat);
	//	if (TellWindow) cwm.removeChatWindow(TellWindow);
	TeamList.reset();
	FriendList.reset();
	IgnoreList.reset();

	Region = NULL;
	Universe = NULL;
	TeamChat = NULL;
	GuildChat  = NULL;
	SystemInfo = NULL;
	TellWindow = NULL;
	DebugInfo = NULL;
	YuboChat = NULL;
//	TellWindow = NULL;

	for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		if(DynamicChat[i])	cwm.removeChatWindow(DynamicChat[i]);
		DynamicChat[i]= NULL;
	}

	removeAllPartyChat();

	ChatGroup.release();

	for(uint k = 0; k < MaxNumUserChats; ++k)
	{
		UserChat[k].release();
	}
	uint numCW = cwm.getNumChatWindow();
	if (numCW != 0)
	{
		nlwarning("%d chat windows have not been deleted", (int) numCW);
		uint numCW = cwm.getNumChatWindow();
		for(uint k = 0; k < numCW; ++k)
		{
			nlwarning("Window %d : %s", (int) k, (cwm.getChatWindowByIndex(k)->getTitle().toString()).c_str());
		}
	}
}

//===========================================================================================================
void CPeopleInterraction::removeAllPartyChat()
{
	CChatWindowManager &cwm = getChatWndMgr();
	for(std::vector<CPartyChatInfo>::iterator it = PartyChats.begin(); it != PartyChats.end(); ++it)
	{
		if (it->Window) cwm.removeChatWindow(it->Window);
		it->Window = NULL;
	}
	PartyChats.clear();
	// remove filtered chats
	//cwm.removeChatWindow(MainChat.Window);
	//MainChat.Window = NULL;
	cwm.removeChatWindow(ChatGroup.Window);
	ChatGroup.Window = NULL;
	for(uint k = 0; k < MaxNumUserChats; ++k)
	{
		if (UserChat[k].Window)
		{
			cwm.removeChatWindow(UserChat[k].Window);
			UserChat[k].Window = NULL;
		}
	}

}

//===========================================================================================================
bool CPeopleInterraction::isUserChat(CChatWindow *cw) const
{
//	if (cw == MainChat.Window) return true;
	if (cw == ChatGroup.Window) return true;
	for(uint k = 0; k < MaxNumUserChats; ++k)
	{
		if (UserChat[k].Window == cw) return true;
	}
	return false;
}

//===========================================================================================================
void CPeopleInterraction::init()
{
	// create chat windows
	// todo : internationnalization
	createAroundMeWindow();
	createRegionWindow();
	createUniverseWindow();
	createTeamChat();
	createGuildChat();
	createSystemInfo();
	createTheUserChat();
	createYuboChat();
	createDynamicChats();

	createDebugInfo();
	//createTellWindow();
	//
	createTeamList();
	createFriendList();
	createIgnoreList();

	// main chat should be created after other windows, because it relies on them to receive its messages
	createChatGroup();

	// init the standard inputs
	initStdInputs();
}


//===========================================================================================================
void CPeopleInterraction::initAfterLoad()
{
	/* activate the USER chat per default.
		Important: we must do it after ChatGroup.Window var init, DB color init etc...
		because the latest are used in chat_group_filter ActionHandler
	*/
	CChatGroupWindow	*pCGW= PeopleInterraction.getChatGroupWindow();
	if(pCGW)
		pCGW->setTabIndex(5);
}


//===========================================================================================================
void CPeopleInterraction::initStdInputs()
{
	uint	i;

	ChatInput.AroundMe.addListeningWindow	(ChatGroup.Window);
	ChatInput.Region.addListeningWindow		(ChatGroup.Window);
	ChatInput.Team.addListeningWindow		(ChatGroup.Window);
	ChatInput.Guild.addListeningWindow		(ChatGroup.Window);
	ChatInput.Tell.addListeningWindow		(ChatGroup.Window);
	ChatInput.SystemInfo.addListeningWindow	(ChatGroup.Window);
	ChatInput.YuboChat.addListeningWindow	(ChatGroup.Window);
	ChatInput.Universe.addListeningWindow	(ChatGroup.Window);

	if (AroundMe.Window)
		ChatInput.AroundMe.addListeningWindow(AroundMe.Window);

	if (Region)
		ChatInput.Region.addListeningWindow(Region);

	if (Universe)
		ChatInput.Universe.addListeningWindow(Universe);

	if (TeamChat)
		ChatInput.Team.addListeningWindow(TeamChat);

	if (GuildChat)
		ChatInput.Guild.addListeningWindow(GuildChat);

	if (SystemInfo)
		ChatInput.SystemInfo.addListeningWindow(SystemInfo);

	if (DebugInfo)
		ChatInput.DebugInfo.addListeningWindow(DebugInfo);

	if (YuboChat)
		ChatInput.YuboChat.addListeningWindow(YuboChat);

	if (TheUserChat.Window)
	{
		ChatInput.AroundMe.addListeningWindow(TheUserChat.Window);
		ChatInput.Region.addListeningWindow(TheUserChat.Window);
		ChatInput.Team.addListeningWindow(TheUserChat.Window);
		ChatInput.Guild.addListeningWindow(TheUserChat.Window);
		ChatInput.Universe.addListeningWindow	(TheUserChat.Window);
		// Don't add the system info by default
		// Dynamic chats
		for(i = 0; i < CChatGroup::MaxDynChanPerPlayer; i++)
		{
			ChatInput.DynamicChat[i].addListeningWindow(TheUserChat.Window);
		}
	}

	ChatInput.Tell.addListeningPeopleList(&FriendList);
	ChatInput.Tell.addListeningPeopleList(&TeamList);
	ChatInput.Team.addListeningPeopleList(&TeamList);

	// Dynamic chats
	for(i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		ChatInput.DynamicChat[i].addListeningWindow(ChatGroup.Window);
		if(DynamicChat[i])
			ChatInput.DynamicChat[i].addListeningWindow(DynamicChat[i]);
	}
}

//===========================================================================================================
void CPeopleInterraction::createTeamList()
{
	// temp to test people list before it is connected to the server
	CPeopleListDesc peopleListDesc;
	//peopleListDesc.FatherContainer           = "ui:interface:communication";
	peopleListDesc.PeopleListTitle           = "uiTeamTitle";
	peopleListDesc.Id			             = "team_list";
	peopleListDesc.ContactType               = CPeopleListDesc::Team /* CPeopleListDesc::Contact*/;
	peopleListDesc.BaseContainerTemplateName = "people_list_container";
	peopleListDesc.Localize = true;
	peopleListDesc.Savable = true;
	peopleListDesc.AHOnActive = "proc";
	peopleListDesc.AHOnActiveParams = "team_list_proc_active";
	peopleListDesc.AHOnDeactive = "proc";
	peopleListDesc.AHOnDeactiveParams = "team_list_proc_deactive";
	peopleListDesc.HeaderColor = "UI:SAVE:WIN:COLORS:MEM";
	//
	TeamList.create(peopleListDesc, NULL); // &chatDesc // create the team list with a chat box in it
	TeamList.setMenu("ui:interface:sort_menu");
	// Special case for team : each entry is connected to the database so we create all team member at once
	for(uint k = 0; k < MaxNumPeopleInTeam; ++k)
	{
		TeamList.addPeople(NLMISC::toString(k), k);
	}

	TeamList.setPeopleMenu("ui:interface:team_member_menu");
	TeamList.setMenu("ui:interface:team_chat_member_menu");

	// Open/Close team list when at least one / no team mate
	// NB: use an intermediate temp var, to avoid show of the window each time a new team member enters
	string sExpr = "@UI:VARIABLES:IS_TEAM_PRESENT";
	string sAction = "set";
	string sCond = "";
	string sParams = "target_property=ui:interface:team_list:active|value=@UI:VARIABLES:IS_TEAM_PRESENT";

	if (TeamChat)
	{
		CInterfaceLink *il = new CInterfaceLink;
		vector<CInterfaceLink::CTargetInfo> targets;
		vector<CInterfaceLink::CCDBTargetInfo> cdbTargets;
		il->init(targets, cdbTargets, sExpr, sAction, sParams, sCond, TeamChat->getContainer());
	}
}

//===========================================================================================================
void CPeopleInterraction::createFriendList()
{
	// create friend list
	CPeopleListDesc peopleListDesc;
	peopleListDesc.FatherContainer			= "ui:interface:contact_list";
	peopleListDesc.PeopleListTitle			= "uiFriendList";
	peopleListDesc.Id						= "friend_list";
	peopleListDesc.BaseContainerTemplateName= "people_list_container_with_add_edit_box";
	peopleListDesc.ContactType				= CPeopleListDesc::Contact;
	peopleListDesc.Localize					= true;
	//
	FriendList.create(peopleListDesc);
	FriendList.setPeopleMenuEx("ui:interface:friend_list_menu_offline_unblocked",
		                       "ui:interface:friend_list_menu_online_unblocked",
							   "ui:interface:friend_list_menu_online_abroad_unblocked",
							   "ui:interface:friend_list_menu_offline_blocked",
		                       "ui:interface:friend_list_menu_online_blocked",
							   "ui:interface:friend_list_menu_online_abroad_blocked"
		                      );
	FriendList.setMenu("ui:interface:sort_menu");
}


//===========================================================================================================
void CPeopleInterraction::createIgnoreList()
{
	// create ignore list
	CPeopleListDesc peopleListDesc;
	peopleListDesc.FatherContainer           = "ui:interface:contact_list";
	peopleListDesc.PeopleListTitle           = "uiIgnoreList";
	peopleListDesc.Id			             = "ignore_list";
	peopleListDesc.BaseContainerTemplateName = "people_list_container_with_add_edit_box";
	peopleListDesc.ContactType               = CPeopleListDesc::Ignore;
	peopleListDesc.Localize = true;
	//
	IgnoreList.create(peopleListDesc);
	//
	IgnoreList.setPeopleMenu("ui:interface:ignore_list_menu");
	IgnoreList.setMenu("ui:interface:sort_menu");
}

//===========================================================================================================
void CPeopleInterraction::createSystemInfo()
{
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiSystemInfoTitle";
	chatDesc.Listener = NULL;
	chatDesc.Savable = true;
	chatDesc.Localize = true;
	chatDesc.ChatTemplate ="system_info_id";
	chatDesc.Id = "system_info";
	chatDesc.AHOnCloseButton = "proc";
	chatDesc.AHOnCloseButtonParams = "sysinfo_chat_proc_close";

	SystemInfo = getChatWndMgr().createChatWindow(chatDesc);
	if (!SystemInfo) return;
	SystemInfo->setMenu("ui:interface:base_chat_box_menu");
}

//===========================================================================================================
void CPeopleInterraction::createDebugInfo()
{
	// can only be used by devs and CSR or in local mode
#if FINAL_VERSION
	if( ClientCfg.Local || hasPrivilegeDEV() || hasPrivilegeSGM() )
#endif
	{
		CChatWindowDesc chatDesc;
		chatDesc.FatherContainer = "ui:interface";
		chatDesc.Title = "uiDebugConsole";
		chatDesc.Listener = NULL;
		chatDesc.Savable = true;
		chatDesc.Localize = true;
		chatDesc.Listener = &DebugConsoleEntryHandler;
		chatDesc.ChatTemplate ="clearable_chat_id";
		chatDesc.Id = "debug_info";

		DebugInfo = getChatWndMgr().createChatWindow(chatDesc);
		if (!DebugInfo) return;
		DebugInfo->setMenu("ui:interface:base_chat_box_menu");
	}
}

//===========================================================================================================
void CPeopleInterraction::createAroundMeWindow()
{
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiAroundMeTitle";
	chatDesc.Listener = NULL;
	chatDesc.Localize = true;
	chatDesc.Savable = true;
	//chatDesc.ChatTemplate = "around_me_id";
	chatDesc.Id = "around_me";
	chatDesc.AHOnCloseButton = "proc";
	chatDesc.AHOnCloseButtonParams = "around_me_chat_proc_close";

	AroundMe.Window = getChatWndMgr().createChatWindow(chatDesc);
	if (!AroundMe.Window) return;
	AroundMe.Window->setMenu(STD_CHAT_SOURCE_MENU);
	// Configure filter for the main chat. By default, it listen to everything (no party chats : none have been created yet)
	AroundMe.Filter.setTargetGroup(CChatGroup::say);
	// associate filter with chat window
	AroundMe.Filter.setChat(AroundMe.Window);
}


//===========================================================================================================
void CPeopleInterraction::createRegionWindow()
{
	// create region window
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiRegionTitle";
	chatDesc.Listener = &RegionEntryHandler;
	chatDesc.Localize = true;
	chatDesc.Savable = true;
	chatDesc.Id = "region_chat";
	chatDesc.AHOnCloseButton = "proc";
	chatDesc.AHOnCloseButtonParams = "region_chat_proc_close";
	chatDesc.HeaderColor = "UI:SAVE:WIN:COLORS:MEM";

	Region = getChatWndMgr().createChatWindow(chatDesc);
	if (!Region) return;
	Region->setMenu(STD_CHAT_SOURCE_MENU);
}


//===========================================================================================================
void CPeopleInterraction::createUniverseWindow()
{
	// create universe window
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiUniverseTitle";
	chatDesc.Listener = &UniverseEntryHandler;
	chatDesc.Localize = true;
	chatDesc.Savable = true;
	chatDesc.Id = "universe_chat";
	chatDesc.AHOnCloseButton = "proc";
	chatDesc.AHOnCloseButtonParams = "universe_chat_proc_close";
	chatDesc.HeaderColor = "UI:SAVE:WIN:COLORS:MEM";

	Universe = getChatWndMgr().createChatWindow(chatDesc);
	if (!Universe) return;
	Universe->setMenu(STD_CHAT_SOURCE_MENU);
}


//===========================================================================================================
void CPeopleInterraction::createTellWindow()
{
	/*CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiTellWindow";
	chatDesc.Listener = NULL;
	chatDesc.Savable = true;
	chatDesc.Localize = true;
	chatDesc.Id = "tell";
	chatDesc.ChatTemplate ="chat_no_eb_id";
	chatDesc.AHOnActive = "set";
	chatDesc.AHOnActiveParams = "dblink=UI:SAVE:ISDETACHED:TELL|value=1";
	chatDesc.AHOnDeactive = "set";
	chatDesc.AHOnDeactiveParams = "dblink=UI:SAVE:ISDETACHED:TELL|value=0";

	TellWindow = getChatWndMgr().createChatWindow(chatDesc);
	if (!TellWindow) return;
	TellWindow->setMenu("ui:interface:base_chat_box_menu");	*/
}

//===========================================================================================================
void CPeopleInterraction::createTeamChat()
{
	// create team chat (inserted in team people list)
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiTeamChatTitle";
	chatDesc.Listener = &TeamChatEntryHandler;
	chatDesc.Localize = true;
	chatDesc.Savable = true;
	chatDesc.Id = "team_chat";
	chatDesc.AHOnCloseButton = "proc";
	chatDesc.AHOnCloseButtonParams = "team_chat_proc_close";
	chatDesc.HeaderColor = "UI:SAVE:WIN:COLORS:MEM";

	TeamChat = getChatWndMgr().createChatWindow(chatDesc);
	if (!TeamChat) return;
	TeamChat->setMenu(STD_CHAT_SOURCE_MENU);
}

//===========================================================================================================
void CPeopleInterraction::createGuildChat()
{
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiGuildChat";
	chatDesc.Listener = &GuildChatEntryHandler;
	chatDesc.Localize = true;
	chatDesc.Savable = true;
	chatDesc.Id = "guild_chat";
	chatDesc.AHOnCloseButton = "proc";
	chatDesc.AHOnCloseButtonParams = "guild_chat_proc_close";
	chatDesc.HeaderColor = "UI:SAVE:WIN:COLORS:MEM";

	GuildChat = getChatWndMgr().createChatWindow(chatDesc);
	if (!GuildChat) return;
	GuildChat->setMenu(STD_CHAT_SOURCE_MENU);
}

//===========================================================================================================
void CPeopleInterraction::createYuboChat()
{
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiYuboChat";
	chatDesc.Listener = &YuboChatEntryHandler;
	chatDesc.Localize = true;
	chatDesc.Savable = true;
	chatDesc.Id = "yubo_chat";
	chatDesc.AHOnCloseButton = "proc";
	chatDesc.AHOnCloseButtonParams = "yubo_chat_proc_close";
	chatDesc.HeaderColor = "UI:SAVE:WIN:COLORS:MEM";

	YuboChat = getChatWndMgr().createChatWindow(chatDesc);
	if (!YuboChat) return;
	YuboChat->setMenu(STD_CHAT_SOURCE_MENU);
}


//=================================================================================================================
void CPeopleInterraction::createDynamicChats()
{
	for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
	{
		CChatWindowDesc chatDesc;
		chatDesc.FatherContainer = "ui:interface";
		chatDesc.Title = toString("dummyDynChatTitle%d",i);	// title not used, but still important because create() test uniqueness
		DynamicChatEntryHandler[i].DbIndex= i;
		chatDesc.Listener = &DynamicChatEntryHandler[i];
		chatDesc.Localize = false;
		chatDesc.Savable = true;
		chatDesc.ChatTemplate ="dynamic_chat_id";
		chatDesc.ChatTemplateParams.push_back(make_pair(string("dyn_chat_nb"),toString(i)));
		chatDesc.Id = string("dynamic_chat") + toString(i);
		// no active proc because active state is driven by database
		chatDesc.AHOnCloseButton = "proc";
		chatDesc.AHOnCloseButtonParams = string("dynamic_chat_proc_close|") + toString(i);
		chatDesc.HeaderColor = "UI:SAVE:WIN:COLORS:MEM";

		DynamicChat[i] = getChatWndMgr().createChatWindow(chatDesc);
		if (!DynamicChat[i]) continue;
		DynamicChat[i]->setMenu(STD_CHAT_SOURCE_MENU);
	}
}


//=================================================================================================================
void CPeopleInterraction::createTheUserChat()
{
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Title = "uiUserChat";
	chatDesc.Listener = NULL;
	chatDesc.Localize = true;
	chatDesc.Savable = true;
	chatDesc.Id = "user_chat";
	chatDesc.ChatTemplate = "filtered_chat_id";
	chatDesc.AHOnActive = "user_chat_active";
	chatDesc.AHOnActiveParams = "";
	chatDesc.AHOnCloseButton = "set";
	chatDesc.AHOnCloseButtonParams = "dblink=UI:SAVE:ISDETACHED:USER_CHAT|value=0";

	TheUserChat.Window = getChatWndMgr().createChatWindow(chatDesc);
	if (!TheUserChat.Window) return;
	TheUserChat.Window->getContainer()->setup();
	// Configure filter for the new chat (by default, listen to everything but party chats)
	TheUserChat.Filter.setTargetGroup(CChatGroup::say);
	// assoviate filter with chat window
	TheUserChat.Filter.setChat(TheUserChat.Window);
}


class CHandlerUserChatActive : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CChatWindow *pCGW = dynamic_cast<CChatWindow*>(PeopleInterraction.TheUserChat.Window);
		if (pCGW == NULL) return;
		CCtrlTextButton *pUserBut = dynamic_cast<CCtrlTextButton*>(pCGW->getContainer()->getCtrl("content:target_button"));
		CInterfaceGroup *pEditBox = dynamic_cast<CInterfaceGroup*>(pCGW->getContainer()->getGroup("content:ebw"));

		if(!pUserBut)
			return;

		CChatGroup::TGroupType m = PeopleInterraction.TheUserChat.Filter.getTargetGroup();
		switch(m)
		{
			default:
			case CChatGroup::arround:
			case CChatGroup::say:		pUserBut->setHardText("uiFilterAround");	break;
			case CChatGroup::region:	pUserBut->setHardText("uiFilterRegion");	break;
			case CChatGroup::universe:	pUserBut->setHardText("uiFilterUniverse");	break;
			case CChatGroup::team:		pUserBut->setHardText("uiFilterTeam");		break;
			case CChatGroup::guild:		pUserBut->setHardText("uiFilterGuild");		break;
		}
		pUserBut->getParent()->updateCoords();
		pUserBut->updateCoords();

		if (pEditBox != NULL) pEditBox->setW(-pUserBut->getWReal()-4);
	}
};
REGISTER_ACTION_HANDLER(CHandlerUserChatActive, "user_chat_active");

//===========================================================================================================
void CPeopleInterraction::createChatGroup()
{
	CChatWindowDesc chatDesc;
	chatDesc.FatherContainer = "ui:interface";
	chatDesc.Listener = NULL;
	chatDesc.Title= "";		// NB: the chatgroup is the only one that can be not named (because of uniqueness title test)
	chatDesc.Localize = true;
	chatDesc.Savable = true;
	chatDesc.ChatTemplate = "main_chat_group";
	chatDesc.Id = "main_chat";
	chatDesc.AHOnActive = "proc";
	chatDesc.AHOnActiveParams = "main_chat_group_active";
	chatDesc.AHOnDeactive = "proc";
	chatDesc.AHOnDeactiveParams = "main_chat_group_deactive";

	ChatGroup.Window = getChatWndMgr().createChatGroupWindow(chatDesc);
	if (!ChatGroup.Window) return;
	// Configure filter for the main chat. By default, it listen to everything (no party chats : none have been created yet)
	ChatGroup.Filter.setTargetGroup(CChatGroup::say);
	// associate filter with chat window
	ChatGroup.Filter.setChat(ChatGroup.Window);
}

class CHandlerChatGroupFilter : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		const	string dynChatId="dyn_chat";

		bool	writeRight= true;
		bool	isDynChat= false;
		uint32	dynChatDbIndex= 0;

		// Entry output
		CChatTargetFilter &rCTF = PeopleInterraction.ChatGroup.Filter;
		if (sParams == "around")		rCTF.setTargetGroup(CChatGroup::say);
		else if (sParams == "region")	rCTF.setTargetGroup(CChatGroup::region);
		else if (sParams == "universe")	rCTF.setTargetGroup(CChatGroup::universe);
		else if (sParams == "team")		rCTF.setTargetGroup(CChatGroup::team);
		else if (sParams == "guild")	rCTF.setTargetGroup(CChatGroup::guild);
		else if (sParams == "sysinfo")
		{
			rCTF.setTargetGroup(CChatGroup::system);
			writeRight= false;
		}
		else if (sParams == "yubo_chat")	rCTF.setTargetGroup(CChatGroup::yubo_chat);
		else if (sParams.compare(0, dynChatId.size(), dynChatId)==0)
		{
			// get the number of this tab
			isDynChat= true;
			fromString(sParams.substr(dynChatId.size()), dynChatDbIndex);
			dynChatDbIndex= min(dynChatDbIndex, (uint32)(CChatGroup::MaxDynChanPerPlayer-1));
			rCTF.setTargetGroup(CChatGroup::dyn_chat, dynChatDbIndex);
		}

		// inform DB for write right.
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MAIN_CHAT:WRITE_RIGHT")->setValue32(writeRight);
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MAIN_CHAT:IS_DYN_CHAT")->setValue32(isDynChat);
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MAIN_CHAT:INDEX_DYN_CHAT")->setValue32(dynChatDbIndex);


		// Update Chat Group Window from user chat button

		CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
		if (!pCGW) return;
		CCtrlTextButton *pUserBut = dynamic_cast<CCtrlTextButton*>(pCGW->getContainer()->getCtrl("content:but_user"));
		CCtrlTextButton *pEmoteBut = dynamic_cast<CCtrlTextButton*>(pCGW->getContainer()->getCtrl("content:but_emote"));
		CInterfaceGroup *pEditBox = dynamic_cast<CInterfaceGroup*>(pCGW->getContainer()->getGroup("content:ebw"));
		CInterfaceGroup *pTextList = dynamic_cast<CInterfaceGroup*>(pCGW->getContainer()->getGroup("content:cb"));

		// Target button choose the right filter


		// Special case of the user defined chat
		if (sParams == "user")
		{
			if (pUserBut != NULL)
			{
				CChatGroup::TGroupType m = PeopleInterraction.TheUserChat.Filter.getTargetGroup();
				switch(m)
				{
					default:
					case CChatGroup::arround:
					case CChatGroup::say:		pUserBut->setHardText("uiFilterAround");	break;
					case CChatGroup::region:	pUserBut->setHardText("uiFilterRegion");	break;
					case CChatGroup::team:		pUserBut->setHardText("uiFilterTeam");		break;
					case CChatGroup::guild:		pUserBut->setHardText("uiFilterGuild");		break;
					case CChatGroup::universe:	pUserBut->setHardText("uiFilterUniverse");	break;
					case CChatGroup::dyn_chat:
						uint32 index = PeopleInterraction.TheUserChat.Filter.getTargetDynamicChannelDbIndex();
						uint32 textId = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:DYN_CHAT:CHANNEL"+toString(index)+":NAME")->getValue32();
						ucstring title;
						STRING_MANAGER::CStringManagerClient::instance()->getDynString(textId, title);
						if (title.empty())
						{
							// Dyn channel not available yet, so set to around
							PeopleInterraction.TheUserChat.Filter.setTargetGroup(CChatGroup::arround);
							pUserBut->setHardText("uiFilterAround");
						}
						else
						{
							pUserBut->setHardText(title.toUtf8());
						}
						break;
					// NB: user chat cannot have yubo_chat target
				}

				pUserBut->setActive(true);
				pUserBut->getParent()->updateCoords();
				pUserBut->updateCoords();

				pEmoteBut->setActive (true);
				pEmoteBut->updateCoords ();

				if (pEditBox != NULL)
				{
					pEditBox->setW(-pUserBut->getWReal()-pEmoteBut->getWReal()-8);
					pEditBox->setX(pUserBut->getWReal()+4);
				}


			}
			rCTF.setTargetGroup(PeopleInterraction.TheUserChat.Filter.getTargetGroup(), PeopleInterraction.TheUserChat.Filter.getTargetDynamicChannelDbIndex());
		}
		else
		{
			if (pUserBut != NULL) pUserBut->setActive(false);

			if (pEmoteBut)
			{
				pEmoteBut->setActive (true);
				pEmoteBut->updateCoords ();
			}

			if (pEditBox != NULL)
			{
				if(pEmoteBut)
					pEditBox->setW(-pEmoteBut->getWReal()-4);
				else
					pEditBox->setW(0);
				pEditBox->setX(0);
			}
			if (pTextList != NULL) pTextList->setX(0);
		}

		// if called from a tab button => force the tab ctrl button to have standard color
		CCtrlTabButton	*pTabButton= dynamic_cast<CCtrlTabButton*>(pCaller);
		if(pTabButton)
		{
			CRGBA	stdColor= CRGBA::stringToRGBA(CWidgetManager::getInstance()->getParser()->getDefine("chat_group_tab_color_normal").c_str());
			pTabButton->setTextColorNormal(stdColor);
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerChatGroupFilter, "chat_group_filter");


//===========================================================================================================
class CHandlerChatGroupUpdatePrompt : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// re set the target group will automatically reset the prompt and prompt color
		CChatTargetFilter &rCTF = PeopleInterraction.ChatGroup.Filter;
		rCTF.setTargetGroup(rCTF.getTargetGroup(), rCTF.getTargetDynamicChannelDbIndex());
	}
};
REGISTER_ACTION_HANDLER(CHandlerChatGroupUpdatePrompt, "chat_group_update_prompt");


//===========================================================================================================
CPeopleList *CPeopleInterraction::getPeopleListFromContainerID(const std::string &id)
{
	if (TeamList.getContainerID() == id) return &TeamList;
	else if (FriendList.getContainerID() == id) return &FriendList;
	else if (IgnoreList.getContainerID() == id) return &IgnoreList;
	return NULL;
}


//===========================================================================================================
bool CPeopleInterraction::getPeopleFromContainerID(const std::string &id, CPeopleList *&peopleList, uint &destIndex)
{
	// get people index
	// the name has the form "ui:interface:container_name_people_index"
	typedef std::string::size_type TCharPos;
	TCharPos index = id.find_last_of("_");
	if (index == std::string::npos || index == 0) return false;
	TCharPos nextIndex = id.rfind(":", index);
	if (nextIndex == std::string::npos) return false;
	std::string containerId = id.substr(nextIndex + 1, index - nextIndex - 1);
	// search a container with the good name
	CPeopleList *pl = getPeopleListFromContainerID(containerId);
	if (!pl) return false;
	//
	sint peopleIndex = pl->getIndexFromContainerID(id);
	if (peopleIndex == -1) return false;
	//
	destIndex = (uint) peopleIndex;
	peopleList = pl;

	return true;
}

//===========================================================================================================
bool CPeopleInterraction::getPeopleFromCurrentMenu(CPeopleList *&peopleList, uint &index)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// the group that launched the modal window (the menu) must be the header of the group container that represent a people entry
	CInterfaceGroup *header = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
	if (!header) return false;
	// get the parent container
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(header->getParent());
	if (!gc) return false;
	return 	getPeopleFromContainerID(gc->getId(), peopleList, index);
}

//===========================================================================================================
CPeopleList *CPeopleInterraction::getPeopleListFromCurrentMenu()
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	// the group that launched the modal window (the menu) must be the header of the group container that represent a people entry
	CInterfaceGroup *header = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
	if (!header) return NULL;
	// get the parent container
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(header->getParent());
	if (!gc) return NULL;
	std::string::size_type pos = gc->getId().find_last_of(":");
	if (pos == std::string::npos) return NULL;
	return getPeopleListFromContainerID(gc->getId().substr(pos + 1));
}

//===========================================================================================================
CFilteredChat *CPeopleInterraction::getFilteredChatFromChatWindow(CChatWindow *cw)
{
	//if (cw == MainChat.Window) return &MainChat;
	if (cw == ChatGroup.Window) return &ChatGroup;
	if (cw == AroundMe.Window) return &AroundMe;
	if (cw == TheUserChat.Window) return &TheUserChat;
	for(uint k = 0; k < MaxNumUserChats; ++k)
	{
		if (UserChat[k].Window == cw) return &UserChat[k];
	}
	return NULL;
}

//===========================================================================================================
void CPeopleInterraction::askAddContact(const ucstring &contactName, CPeopleList *pl)
{
	if (pl == NULL)
		return;

	if ((pl != &IgnoreList) && (pl != &FriendList))
	{
		nlwarning("<askAddContact> For now, only support friend list & ignore list");
		return;
	}

	// check that name isn't already in people list
	if (pl->getIndexFromName(contactName) != -1)
	{
		// people already in list, can't add twice
		CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiContactAlreadyInList"));
		return;
	}

	// add into server (NB: will be added by the server response later)
	const std::string sMsg = "TEAM:CONTACT_ADD";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		uint8 list = 0;

		if (pl == &IgnoreList)
			list = 1;

		if (pl == &FriendList)
			list = 0;

		ucstring temp = contactName;
		out.serial(temp);
		out.serial(list);
		NetMngr.push(out);
		//nlinfo("impulseCallBack : %s %s %d sent", sMsg.c_str(), contactName.toString().c_str(), list);
	}
	else
		nlwarning("impulseCallBack : unknown message name : '%s'.", sMsg.c_str());

	// NB: no client prediction, will be added by server later

	// Fake Local simulation
	if (ClientCfg.Local)
	{
		sint index = pl->addPeople(contactName);
		pl->setOnline(index, ccs_online);
		updateAllFreeTellerHeaders();
	}
}

//=================================================================================================================
void CPeopleInterraction::askMoveContact(uint peopleIndexInSrc, CPeopleList *plSRC, CPeopleList *plDST)
{
	if ((plSRC == NULL) || (plDST == NULL)) return;
	if ((plSRC != &IgnoreList) && (plSRC != &FriendList))
	{
		nlwarning("<askMoveContact> For now, only support friend list & ignore list");
		return;
	}
	if ((plDST != &IgnoreList) && (plDST != &FriendList))
	{
		nlwarning("<askMoveContact> For now, only support friend list & ignore list");
		return;
	}
	if ( plDST == plSRC )
	{
		// move from list to same => no op
		return;
	}

	// check that index is already in people list
	if (peopleIndexInSrc >= plSRC->getNumPeople()) return;


	// Send message to server
	uint32	contactId= plSRC->getContactId(peopleIndexInSrc);
	uint8	nListSRC;

	if (plSRC == &FriendList)
		nListSRC = 0;

	if (plSRC == &IgnoreList)
		nListSRC = 1;

	const std::string sMsg = "TEAM:CONTACT_MOVE";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		out.serial(contactId);
		out.serial(nListSRC);
		NetMngr.push(out);
		//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), contactId, nListSRC);
	}
	else
		nlwarning("impulseCallBack : unknown message name : '%s'.", sMsg.c_str());

	// NB: no client prediction, will be added by server later

	// Fake Local simulation
	if (ClientCfg.Local)
	{
		ucstring peopleName= plSRC->getName(peopleIndexInSrc);
		plSRC->removePeople(peopleIndexInSrc);
		sint dstIndex = plDST->addPeople(peopleName);
		plDST->setOnline(dstIndex, ccs_online);
		if (getChatGroupWindow())
		{
			getChatGroupWindow()->updateAllFreeTellerHeaders();
		}
		updateAllFreeTellerHeaders();
	}
}

//=================================================================================================================
void CPeopleInterraction::askRemoveContact(uint peopleIndex, CPeopleList *pl)
{
	if (pl == NULL) return;
	if ((pl != &IgnoreList) && (pl != &FriendList))
	{
		nlwarning("<askRemoveContact> For now, only support friend pl & ignore pl");
		return;
	}
	if (peopleIndex >= pl->getNumPeople())
	{
		nlwarning("<askRemoveContact> bad index given");
		return;
	}

	// send server message
	const std::string sMsg = "TEAM:CONTACT_DEL";
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		uint32 contactId = pl->getContactId(peopleIndex);
		out.serial(contactId);
		uint8 nList = 0;
		if (pl == &PeopleInterraction.FriendList)
			nList = 0;
		if (pl == &PeopleInterraction.IgnoreList)
			nList = 1;
		out.serial(nList);
		NetMngr.push(out);
		//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), contactId, nList);
	}
	else
		nlwarning("impulseCallBack : unknown message name : '%s'.", sMsg.c_str());

	// NB: no client prediction, let the server delete the contact by message.

	// Fake Local simulation
	if (ClientCfg.Local)
	{
		pl->removePeople(peopleIndex);
		updateAllFreeTellerHeaders();
	}

}

//=================================================================================================================
void CPeopleInterraction::initContactLists( const std::vector<uint32> &vFriendListName,
											const std::vector<TCharConnectionState> &vFriendListOnline,
											const std::vector<ucstring> &vIgnoreListName	)

{
	// clear the current lists if any
	FriendList.removeAllPeoples();
	IgnoreList.removeAllPeoples();

	// build the contact ids, like server did
	uint32	contactIdPool= 0;
	for (uint i = 0; i < vFriendListName.size(); ++i)
		addContactInList(contactIdPool++, vFriendListName[i], vFriendListOnline[i], 0);
	for (uint i = 0; i < vIgnoreListName.size(); ++i)
		addContactInList(contactIdPool++, vIgnoreListName[i], ccs_offline, 1);
	updateAllFreeTellerHeaders();
}

//=================================================================================================================
void CPeopleInterraction::addContactInList(uint32 contactId, const ucstring &nameIn, TCharConnectionState online, uint8 nList)
{
	// select correct people list
	CPeopleList	&pl= nList==0?FriendList:IgnoreList;

	// remove the shard name if possible
	ucstring	name= CEntityCL::removeShardFromName(nameIn);

	// add the contact to this list
	sint index = pl.getIndexFromName(name);
	// try to create if not found
	if (index == -1)
		index = pl.addPeople(name);

	if (index != -1)
	{
		pl.setOnline(index, online);
		pl.setContactId(index, contactId);
	}

	CInterfaceManager* pIM= CInterfaceManager::getInstance();
	CPeopleList::TSortOrder order = (CPeopleList::TSortOrder)(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTACT_LIST:SORT_ORDER")->getValue32());
	FriendList.sortEx(order);
}

//=================================================================================================================
void CPeopleInterraction::addContactInList(uint32 contactId, uint32 nameID, TCharConnectionState online, uint8 nList)
{
	ucstring name;
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
	if (pSMC->getString(nameID, name))
	{
		addContactInList(contactId, name, online, nList);
		// update free teller header
		updateAllFreeTellerHeaders();
	}
	else
	{
		SWaitingContact w;
		w.ContactId= contactId;
		w.NameId = nameID;
		w.List = nList; // Friend list == 0 // Ignore list == 1
		w.Online = online;
		WaitingContacts.push_back(w);

		CInterfaceManager* pIM= CInterfaceManager::getInstance();
		CPeopleList::TSortOrder order = (CPeopleList::TSortOrder)(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTACT_LIST:SORT_ORDER")->getValue32());
		FriendList.sortEx(order);
	}
}

//=================================================================================================================
bool CPeopleInterraction::isContactInList(const ucstring &nameIn, uint8 nList) const
{
	// select correct people list
	const CPeopleList	&pl= nList==0?FriendList:IgnoreList;
	// remove the shard name if possible
	ucstring	name= CEntityCL::removeShardFromName(nameIn);
	return pl.getIndexFromName(name) != -1;
}

//=================================================================================================================
void CPeopleInterraction::updateAllFreeTellerHeaders()
{
	CChatGroupWindow *gcw = getChatGroupWindow();
	if (gcw)
	{
		gcw->updateAllFreeTellerHeaders();
	}
}

//=================================================================================================================
void CPeopleInterraction::removeAllFreeTellers()
{
	CChatGroupWindow *gcw = getChatGroupWindow();
	if (gcw)
	{
		gcw->removeAllFreeTellers();
	}
}

//=================================================================================================================
void CPeopleInterraction::updateWaitingContacts()
{
	bool touched = false;
	for (uint32 i = 0; i < WaitingContacts.size();)
	{
		SWaitingContact &w = WaitingContacts[i];
		ucstring name;
		STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
		if (pSMC->getString(w.NameId, name))
		{
			addContactInList(w.ContactId, name, w.Online, w.List);
			WaitingContacts.erase(WaitingContacts.begin()+i);
			touched = true;
		}
		else
		{
			++i;
		}
	}
	if (touched)
	{
		updateAllFreeTellerHeaders();
	}
}

//=================================================================================================================
void CPeopleInterraction::updateContactInList(uint32 contactId, TCharConnectionState online, uint nList)
{
	if (nList == 0)
	{
		sint index = FriendList.getIndexFromContactId(contactId);
		if (index != -1)
		{
			// Only do work if online status has changed
			if (FriendList.getOnline(index) != online)
			{
				CCDBNodeLeaf* node = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:SHOW_ONLINE_OFFLINE_NOTIFICATIONS_CB", false);
				if (node && node->getValueBool())
				{
					// Only show the message if this player is not in my guild (because then the guild manager will show a message)
					std::vector<SGuildMember> GuildMembers = CGuildManager::getInstance()->getGuildMembers();
					bool bOnlyFriend = true;
					ucstring name = toLower(FriendList.getName(index));
					for (uint i = 0; i < GuildMembers.size(); ++i)
					{
						if (toLower(GuildMembers[i].Name) == name)
						{
							bOnlyFriend = false;
							break;
						}
					}
					
					TCharConnectionState prevState = FriendList.getOnline(index);
					bool showMsg = bOnlyFriend && (prevState == ccs_offline || online == ccs_offline);

					// Player is not in my guild, and the status change is from offline to online/abroad online or vice versa. 
					if (showMsg)
					{
						ucstring msg = (online != ccs_offline) ? CI18N::get("uiPlayerOnline") : CI18N::get("uiPlayerOffline");
						strFindReplace(msg, "%s", FriendList.getName(index));
						string cat = getStringCategory(msg, msg);
						map<string, CClientConfig::SSysInfoParam>::const_iterator it;
						NLMISC::CRGBA col = CRGBA::Yellow;
						it = ClientCfg.SystemInfoParams.find(toLower(cat));
						if (it != ClientCfg.SystemInfoParams.end())
						{
							col = it->second.Color;
						}
						bool dummy;
						PeopleInterraction.ChatInput.AroundMe.displayMessage(msg, col, 2, &dummy);
					}
				}

				FriendList.setOnline(index, online);
			}
		}
	}
	else
	{
		sint index = IgnoreList.getIndexFromContactId(contactId);
		if (index != -1)
			IgnoreList.setOnline(index, online);
	}
}

//=================================================================================================================
void CPeopleInterraction::removeContactFromList(uint32 contactId, uint8 nList)
{
	if (nList == 0)
	{
		sint index = FriendList.getIndexFromContactId(contactId);
		if (index != -1)
			FriendList.removePeople(index);
	}
	else
	{
		sint index = IgnoreList.getIndexFromContactId(contactId);
		if (index != -1)
			IgnoreList.removePeople(index);
	}
	updateAllFreeTellerHeaders();
}

//=================================================================================================================
bool CPeopleInterraction::testValidPartyChatName(const ucstring &title)
{
	if (title.empty()) return false;
	// shouldn't begin like 'user chat 1-5'
	ucstring userChatStr = CI18N::get("uiUserChat");
	if (title.substr(0, userChatStr.length()) == userChatStr) return false;
	for(uint k = 0; k < PartyChats.size(); ++k) // there shouldn't be that much party chat simultaneously so a linear search is ok
	{
		if (PartyChats[k].Window->getTitle() == title) return false;
	}
	// check for other chat window names (local only ?)
	if (SystemInfo && title == SystemInfo->getTitle()) return false;
	if (AroundMe.Window && title == AroundMe.Window->getTitle()) return false;
	if (GuildChat && title == GuildChat->getTitle()) return false;
	if (TeamChat && title == TeamChat->getTitle()) return false;
	sint index;
	index = FriendList.getIndexFromName(title);
	if (index != -1) return false;
	index = IgnoreList.getIndexFromName(title);
	if (index != -1) return false;
	// TODO_GAMEDEV server test for the name (not only local), & modify callers of this function
    // The party chat should NOT have the name of a player
	// A player name is NOT valid if it is the same that a party chat name
	return true;
}

//=================================================================================================================
bool CPeopleInterraction::removePartyChat(CChatWindow *window)
{
	if (!window) return false;
	std::vector<CPartyChatInfo>::iterator it;
	for(it = PartyChats.begin(); it != PartyChats.end(); ++it)
	{
		if (it->Window == window) break;
	}
	if (it != PeopleInterraction.PartyChats.end())
	{
		PeopleInterraction.PartyChats.erase(it);
		getChatWndMgr().removeChatWindow(window->getTitle());
		// TODO GAMEDEV : send msg to server to tell that the player has deleted this party chat.
		return true;
	}
	else
	{
		return false;
	}
}

//=================================================================================================================
void CPeopleInterraction::assignPartyChatMenu(CChatWindow *partyChat)
{
	if (!partyChat) return;
	// TODO GAMEDEV : fill the 2 following boolean
	bool isTeamLeader = true;
	bool isGuildLeader = true;

	if (isTeamLeader && isGuildLeader)
	{
		partyChat->setMenu("ui:interface:team_and_guild_chief_party_chat_menu");
	}
	else if (isTeamLeader)
	{
		partyChat->setMenu("ui:interface:team_chief_party_chat_menu");
	}
	else if (isGuildLeader)
	{
		partyChat->setMenu("ui:interface:guild_chief_party_chat_menu");
	}
}

//=================================================================================================================
bool CPeopleInterraction::createNewPartyChat(const ucstring &title)
{
	// now there are no party chat windows, party chat phrases must be filtered from the main chat

	// create a new party chat and set the focus on it
	CChatWindowDesc chatDesc;
	//chatDesc.FatherContainer = "ui:interface:communication";
	chatDesc.FatherContainer = "ui:interface:contact_list";
	chatDesc.Title = title;
	chatDesc.Title = title;
	chatDesc.Listener = &PartyChatEntryHandler;
	chatDesc.Localize = false;

	// CChatWindow *newPartyChat = getChatWndMgr().createChatWindow(chatDesc);
	CChatWindow *newPartyChat = NULL;

	//if (newPartyChat)
	{
		// popup the container
		/*
		newPartyChat->getContainer()->setup();
		newPartyChat->getContainer()->setOpen(true);
		newPartyChat->getContainer()->popupCurrentPos();
		newPartyChat->getContainer()->updateCoords();
		newPartyChat->getContainer()->center();
		newPartyChat->getContainer()->setX(newPartyChat->getContainer()->getX() + (sint32) (rand() % PARTY_CHAT_SPAWN_DELTA));
		newPartyChat->getContainer()->setY(newPartyChat->getContainer()->getY() + (sint32) (rand() % PARTY_CHAT_SPAWN_DELTA));
		newPartyChat->getContainer()->enableBlink(2);
		*/

		CPartyChatInfo pci;
		pci.Window = newPartyChat;
		pci.ID = PeopleInterraction.CurrPartyChatID ++;
		pci.Filter = new CChatInputFilter;
		pci.Filter->addListeningWindow(pci.Window);
		//CPeopleInterraction::assignPartyChatMenu(newPartyChat);
		PartyChats.push_back(pci);
		//newPartyChat->setKeyboardFocus();

		return true;
	}
	return false;
}

//=================================================================================================================
void CPeopleInterraction::buildFilteredChatSummary(const CFilteredChat &src, CFilteredChatSummary &fcs)
{
	// fill src infos
	fcs.SrcGuild      = ChatInput.Guild.isListeningWindow(src.Window);
	fcs.SrcAroundMe   = ChatInput.AroundMe.isListeningWindow(src.Window);
	fcs.SrcSystemInfo = ChatInput.SystemInfo.isListeningWindow(src.Window);
	fcs.SrcTeam		  = ChatInput.Team.isListeningWindow(src.Window);
	fcs.SrcTell		  = ChatInput.Tell.isListeningWindow(src.Window);
	fcs.SrcRegion     = ChatInput.Region.isListeningWindow(src.Window);
	fcs.SrcUniverse   = ChatInput.Universe.isListeningWindow(src.Window);

	// fill target infos
	if (src.Filter.getTargetPartyChat() != NULL || !src.Filter.getTargetPlayer().empty())
	{
		fcs.Target = CChatGroup::say;
	}
	else
	{
		fcs.Target = src.Filter.getTargetGroup();
	}
}

//=================================================================================================================
void CPeopleInterraction::buildFilteredDynChatSummary(const CFilteredChat &src, CFilteredDynChatSummary &fcs)
{
	for (uint8 i = 0; i < CChatGroup::MaxDynChanPerPlayer; i++)
	{
		fcs.SrcDynChat[i] = ChatInput.DynamicChat[i].isListeningWindow(src.Window);
	}
}

//=================================================================================================================
void CPeopleInterraction::saveFilteredChat(NLMISC::IStream &f, const CFilteredChat &src)
{
	bool present;
	if (src.Window == NULL)
	{
		present = false;
		f.serial(present);
	}
	else
	{
		present = true;
		f.serial(present);
		CFilteredChatSummary fcs;
		buildFilteredChatSummary(src, fcs);
		f.serial(fcs);
	}
}

//=================================================================================================================
void CPeopleInterraction::saveFilteredDynChat(NLMISC::IStream &f, const CFilteredChat &src)
{
	bool present;
	if (src.Window == NULL)
	{
		present = false;
		f.serial(present);
	}
	else
	{
		present = true;
		f.serial(present);
		CFilteredDynChatSummary fcs;
		buildFilteredDynChatSummary(src, fcs);
		f.serial(fcs);
	}
}

//=================================================================================================================
CChatGroupWindow *CPeopleInterraction::getChatGroupWindow() const
{
	return dynamic_cast<CChatGroupWindow*>(ChatGroup.Window);
}

#define USER_CHATS_INFO_VERSION 2
#define USER_DYN_CHATS_INFO_VERSION 1

//=================================================================================================================
bool CPeopleInterraction::saveUserChatsInfos(NLMISC::IStream &f)
{
	nlassert(!f.isReading());
	try
	{
		sint ver= f.serialVersion(USER_CHATS_INFO_VERSION);
		f.serialCheck(NELID("TAHC"));
		//saveFilteredChat(f, MainChat);
		saveFilteredChat(f, ChatGroup);
		for(uint k = 0; k < MaxNumUserChats; ++k)
		{
			saveFilteredChat(f, UserChat[k]);
		}
		f.serialCheck(NELID("TAHC"));
		if (ver>=1)
		{
			CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
			sint32 index = pCGW ? pCGW->getTabIndex() : 0;
			f.serial(index);
			saveFilteredChat(f, TheUserChat);
		}
		// Save the free tellers only if they belongs to friend list to avoid the 'only growing' situation
		if (ver>=2)
		{
			CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
			pCGW->saveFreeTeller(f);
		}
	}
	catch(const NLMISC::EStream &e)
	{
		nlwarning("Error while saving user chat infos : %s", e.what());
		return false;
	}
	return true;
}

//=================================================================================================================
bool CPeopleInterraction::saveUserDynChatsInfos(NLMISC::IStream &f)
{
	nlassert(!f.isReading());
	try
	{
		sint ver = f.serialVersion(USER_DYN_CHATS_INFO_VERSION);
		f.serialCheck(NELID("OMGY"));
		if (ver >= 1)
		{
			saveFilteredDynChat(f, TheUserChat);
		}
	}
	catch(const NLMISC::EStream &e)
	{
		nlwarning("Error while saving user dyn chat infos : %s", e.what());
		return false;
	}
	return true;
}

//=================================================================================================================
bool CPeopleInterraction::loadUserChatsInfos(NLMISC::IStream &f)
{
	removeAllUserChats();
	nlassert(f.isReading());
	try
	{
		bool present;
		sint ver = f.serialVersion(USER_CHATS_INFO_VERSION);
		f.serialCheck(NELID("TAHC"));
		f.serial(present);
		if (!present)
		{
			nlwarning("Bad data in user chats infos");
			return false;
		}
		CFilteredChatSummary fcs;
		f.serial(fcs);
		//setupUserChatFromSummary(fcs, MainChat);
		setupUserChatFromSummary(fcs, ChatGroup);

		for(uint k = 0; k < MaxNumUserChats; ++k)
		{
			f.serial(present);
			if (present)
			{
				createUserChat(k);
				f.serial(fcs);
				setupUserChatFromSummary(fcs, UserChat[k]);
			}
		}
		f.serialCheck(NELID("TAHC"));
		if (ver>=1)
		{
//			CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
			sint32 index;
			f.serial(index);
			/* Yoyo: decide to always start with the default channel (user) activated
				because complex (at this time, the buttons are not all active, must wait guild loading, UI:SAVE loading etc...)
				Hence this doesn't work for anything but User and Sysinfo (if it is activated....)
				NB: must still load the index for file format reason
				//if (pCGW) pCGW->setTabIndex(index);
			*/
			f.serial(present);
			if (present)
			{
				f.serial(fcs);
				setupUserChatFromSummary(fcs, TheUserChat);
			}
		}
		// Load the free tellers
		if (ver>=2)
		{
			CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
			if (pCGW) pCGW->loadFreeTeller(f);
		}
	}
	catch(const NLMISC::EStream &e)
	{
		nlwarning("Error while loading user chat infos : %s", e.what());
		return false;
	}
	return true;
}

//=================================================================================================================
bool CPeopleInterraction::loadUserDynChatsInfos(NLMISC::IStream &f)
{
	nlassert(f.isReading());
	try
	{
		bool present;
		sint ver = f.serialVersion(USER_DYN_CHATS_INFO_VERSION);
		f.serialCheck(NELID("OMGY"));
		f.serial(present);
		if (!present)
		{
			nlwarning("Bad data in user dyn chats infos");
			return false;
		}
		CFilteredDynChatSummary fcs;
		if (ver >= 1)
		{
			f.serial(fcs);
			setupUserDynChatFromSummary(fcs, TheUserChat);
		}
	}
	catch(const NLMISC::EStream &e)
	{
		nlwarning("Error while loading user dyn chat infos : %s", e.what());
		return false;
	}
	return true;
}


//=================================================================================================================
void CPeopleInterraction::setupUserChatFromSummary(const CFilteredChatSummary &summary, CFilteredChat &dest)
{
	// User Dest. Do not allow Universe Warning, because do not want a warning open at load (moreover, the UNIVERSE tab should not be activated)
	dest.Filter.setTargetGroup(summary.Target, 0, false);
	// src
	ChatInput.AroundMe.setWindowState(dest.Window, summary.SrcAroundMe);
	ChatInput.Guild.setWindowState(dest.Window, summary.SrcGuild);
	ChatInput.SystemInfo.setWindowState(dest.Window, summary.SrcSystemInfo);
	ChatInput.Team.setWindowState(dest.Window, summary.SrcTeam);
	ChatInput.Tell.setWindowState(dest.Window, summary.SrcTell);
	ChatInput.Region.setWindowState(dest.Window, summary.SrcRegion);
	ChatInput.Universe.setWindowState(dest.Window, summary.SrcUniverse);
}

//=================================================================================================================
void CPeopleInterraction::setupUserDynChatFromSummary(const CFilteredDynChatSummary &summary, CFilteredChat &dest)
{
	// src
	for (uint8 i = 0; i < CChatGroup::MaxDynChanPerPlayer; i++)
	{
		ChatInput.DynamicChat[i].setWindowState(dest.Window, summary.SrcDynChat[i]);
	}
}

//=================================================================================================================
void CPeopleInterraction::removeAllUserChats()
{
	for(uint k = 0; k < MaxNumUserChats; ++k)
	{
		if (UserChat[k].Window)
		{
			getChatWndMgr().removeChatWindow(UserChat[k].Window);
			UserChat[k].Filter.reset();
			UserChat[k].Window = NULL;
		}
	}
}

//=================================================================================================================
void CPeopleInterraction::createUserChat(uint index)
{
	if (index >= MaxNumUserChats)
	{
		nlwarning("Bad index");
		return;
	}
	CChatWindowDesc chatDesc;
	ucstring userChatStr = CI18N::get("uiUserChat");
	userChatStr += ucchar(' ') + ucstring(toString(index + 1));
	//chatDesc.FatherContainer = "ui:interface:communication";
	chatDesc.FatherContainer = "ui:interface:contact_list";
	chatDesc.Title = userChatStr;
	chatDesc.Listener = NULL;
	chatDesc.Localize = false;
	chatDesc.Savable = true;
	chatDesc.ChatTemplate = "filtered_chat_id";
	UserChat[index].Window = getChatWndMgr().createChatWindow(chatDesc);
	if (!UserChat[index].Window) return;
	UserChat[index].Window->getContainer()->setup();
	// Configure filter for the new chat (by default, listen to everything but party chats)
	UserChat[index].Filter.setTargetGroup(CChatGroup::say);
	// assoviate filter with chat window
	UserChat[index].Filter.setChat(UserChat[index].Window);
}

//=================================================================================================================
void CPeopleInterraction::refreshActiveUserChats()
{
	for(uint k = 0; k < MaxNumUserChats; ++k)
	{
		if (UserChat[k].Window)
		{
			UserChat[k].Window->getContainer()->setActive(true);
		}
	}
}

//=================================================================================================================
void CPeopleInterraction::talkInDynamicChannel(uint32 channelNb,ucstring sentence)
{
	if(channelNb<CChatGroup::MaxDynChanPerPlayer)
	{
		DynamicChatEntryHandler[channelNb].msgEntered(sentence,DynamicChat[channelNb]);
	}
}

//=================================================================================================================
void CPeopleInterraction::displayTellInMainChat(const ucstring &playerName)
{
	//CChatWindow *chat = PeopleInterraction.MainChat.Window;
	CChatWindow *chat = PeopleInterraction.ChatGroup.Window;
	if (!chat) return;
	chat->getContainer()->setActive (true);
	// make the container blink
	chat->getContainer()->enableBlink(2);
	// TODO : center the view on the newly created container ?
	// display a new command '/name' in the chat. The player must enter a new unique name for the party chat.
	chat->setCommand("tell " + playerName + " ", false);
	chat->setKeyboardFocus();
}

/////////////////////////////////////
// ACTION HANDLERS FOR PEOPLE LIST //
/////////////////////////////////////

//=================================================================================================================
// Target a member of the team
// See also CAHTargetTeammateShortcut
class CHandlerTeamTarget : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		// retrieve the index of the people
		uint	peopleIndex= 0;
		bool	ok= false;
		// If comes from the button, get direct index
		if( !sParams.empty() )
		{
			fromString(sParams, peopleIndex);
			ok= true;
		}
		// else comes from a menu.
		else
		{
			CPeopleList *list;
			if (PeopleInterraction.getPeopleFromCurrentMenu(list, peopleIndex))
			{
				if (list == &PeopleInterraction.TeamList) // check for good list
					ok= true;
			}
		}

		// If success to get the team index
		if(ok)
		{
			// Get the team name id.
			CLFECOMMON::TClientDataSetIndex	entityId= CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;
			CCDBNodeLeaf *prop = NLGUI::CDBManager::getInstance()->getDbProp(toString(TEAM_DB_PATH ":%d:UID", peopleIndex), false);
			if (prop)
				entityId= prop->getValue32();

			if(entityId != CLFECOMMON::INVALID_CLIENT_DATASET_INDEX)
			{
				// get the entity by its received name
				CEntityCL	*entity= EntitiesMngr.getEntityByCompressedIndex(entityId);
				if(entity)
					// Select this entity.
					UserEntity->selection(entity->slot());
				else
				{
					// the entity is not in vision, can't select it
					pIM->displaySystemInfo(CI18N::get("uiTeamSelectNotInVision"), "CHK");
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTeamTarget, "team_target" );

//=================================================================================================================
// Dismiss a member from the team
class CHandlerDismissMember : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// retrieve the index of the people
		CPeopleList *list;
		uint peopleIndex;
		if (PeopleInterraction.getPeopleFromCurrentMenu(list, peopleIndex))
		{
			if (list == &PeopleInterraction.TeamList) // check for good list
			{
				const string msgName = "TEAM:KICK";
				CBitMemStream out;
				if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
				{
					uint8 teamMember = (uint8) peopleIndex;
					out.serial(teamMember);
					NetMngr.push(out);
					//nlinfo("impulseCallBack : %s %d sent", msgName.c_str(), teamMember);
				}
				else
					nlwarning("command 'dismiss_member': unknown message named '%s'.", msgName.c_str());
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerDismissMember, "dismiss_member");

//=================================================================================================================
// Set the leader of the team
class CHandlerSetTeamLeader : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// retrieve the index of the people
		CPeopleList *list;
		uint peopleIndex;
		if (PeopleInterraction.getPeopleFromCurrentMenu(list, peopleIndex))
		{
			if (list == &PeopleInterraction.TeamList) // check for good list
			{
				/*
				const string msgName = "TEAM:SET_LEADER";
				CBitMemStream out;
				if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
				{
					uint8 teamMember = (uint8)(peopleIndex);
					out.serial(teamMember);
					NetMngr.push(out);
					//nlinfo("impulseCallBack : %s %d sent", msgName.c_str(), teamMember);
				}
				else
					nlwarning("command 'set_leader': unknown message named '%s'.", msgName.c_str());
				*/
				NLMISC::ICommand::execute("a setTeamLeader " + toString(peopleIndex), g_log);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerSetTeamLeader, "set_team_leader");

//=================================================================================================================
// Set a successor for the team
class CHandlerSetSuccessor : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// retrieve the index of the people
		CPeopleList *list;
		uint peopleIndex;
		if (PeopleInterraction.getPeopleFromCurrentMenu(list, peopleIndex))
		{
			if (list == &PeopleInterraction.TeamList) // check for good list
			{
				if (ClientCfg.Local)
				{
					NLGUI::CDBManager::getInstance()->getDbProp(TEAM_DB_PATH ":SUCCESSOR_INDEX")->setValue32(peopleIndex);
				}
				else
				{
					const string msgName = "TEAM:SET_SUCCESSOR";
					CBitMemStream out;
					if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
					{
						uint8 teamMember = (uint8) peopleIndex;
						out.serial(teamMember);
						NetMngr.push(out);
						//nlinfo("impulseCallBack : %s %d sent", msgName.c_str(), teamMember);
					}
					else
						nlwarning("command 'set_successor': unknown message named '%s'.", msgName.c_str());
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerSetSuccessor, "set_successor");


//=================================================================================================================
// player or leader quit the team
class CHandlerQuitTeam : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// Create the message for the server to execute a phrase.
		const string msgName = "TEAM:LEAVE";
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		{
			NetMngr.push(out);
			//nlinfo("impulseCallBack : %s sent", msgName.c_str());
		}
		else
			nlwarning("CHandlerContextQuitTeam::execute: unknown message name : '%s'.", msgName.c_str());
	}
};
REGISTER_ACTION_HANDLER( CHandlerQuitTeam, "quit_team");

//=================================================================================================================
// The leader enable / disbale seeds sharing
class CHandlerShareSeeds : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// TODO_GAMEDEV : enable disable seeds sharing
	}
};
REGISTER_ACTION_HANDLER( CHandlerShareSeeds, "share_seeds");

//////////////////
// CONTACT LIST //
//////////////////

//=================================================================================================================
// Remove a contact from a list
class CHandlerRemoveContact : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// retrieve the index of the people
		CPeopleList *list;
		uint peopleIndex;
		if (PeopleInterraction.getPeopleFromCurrentMenu(list, peopleIndex))
		{
			PeopleInterraction.askRemoveContact(peopleIndex, list);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerRemoveContact, "remove_contact");

//=================================================================================================================
// Invoke the 'tell' command on a contact from its menu
// The tell command is displayed in the 'around me' window
class CHandlerMenuTellContact : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		// retrieve the index of the people
		CPeopleList *list;
		uint peopleIndex;
		if (PeopleInterraction.getPeopleFromCurrentMenu(list, peopleIndex))
		{
			CPeopleInterraction::displayTellInMainChat(list->getName(peopleIndex));
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerMenuTellContact, "menu_tell_contact");


//=================================================================================================================
// Invoke the 'tell' command on a contact from a left click
class CHandlerTellContact : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		if (!pCaller) return;
		CInterfaceGroup *ig = pCaller->getParent();
		if (!ig) return;
		CGroupContainer *gc = static_cast< CGroupContainer* >( ig->getEnclosingContainer() );

		if (!gc) return;
		CPeopleList *list;
		uint peopleIndex;
		if (PeopleInterraction.getPeopleFromContainerID(gc->getId(), list, peopleIndex))
		{
			CPeopleInterraction::displayTellInMainChat(list->getName(peopleIndex));
		}

	}
};
REGISTER_ACTION_HANDLER( CHandlerTellContact, "tell_contact");


//=================================================================================================================
std::string LastFatherAddContactId;
// Add a contact to the list, first step
class CHandlerAddContactBegin : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		/** This msg may have been triggered from valid button or from the edit box itself, so retrieve
		  * the edit box from the enclosing group
		  */
		// Get enclosing container to know in which people list we are
		if (pCaller)
		{
			// Get header_open
			CInterfaceGroup *group = pCaller->getParent();
			if (group)
			{
				// Get container
				group = group->getParent();
				if (group)
				{
					LastFatherAddContactId = group->getId();
					if (!LastFatherAddContactId.empty())
					{
						CInterfaceManager *pIM = CInterfaceManager::getInstance();
						string	groupName= getParam(sParams, "group");
						CInterfaceGroup *gc = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(groupName));
						if (gc)
						{
							CGroupEditBox *geb = dynamic_cast<CGroupEditBox *>(gc->getGroup("add_contact_eb:eb"));
							geb->setInputString(ucstring(""));
						}
						CAHManager::getInstance()->runActionHandler("enter_modal", pCaller, sParams);
					}
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerAddContactBegin, "add_contact_begin");


//=================================================================================================================
// Add a contact to the list
class CHandlerAddContact : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		/** This msg may have been triggered from valid button or from the edit box itself, so retrieve
		  * the edit box from the enclosing group
		  */
		// Get enclosing container to know in which people list we are
		if (!LastFatherAddContactId.empty() && pCaller)
		{
			CInterfaceGroup *fatherGC = pCaller->getParent();
			if (fatherGC)
			{
				// Look for the root parent
				for(;;)
				{
					CInterfaceGroup *parent = fatherGC->getParent();
					if (!parent || (parent->getId()=="ui:interface"))
						break;
					fatherGC = parent;
				}

				// Get the modal edit box
				CGroupEditBox *geb = dynamic_cast<CGroupEditBox *>(fatherGC->getGroup("add_contact_eb:eb"));
				if (geb && !geb->getInputString().empty())
				{
					std::string::size_type lastIndex = LastFatherAddContactId.rfind(":");
					if (lastIndex != std::string::npos)
					{
						// Get the people list with the preselected container ID
						CPeopleList *peopleList = PeopleInterraction.getPeopleListFromContainerID(LastFatherAddContactId.substr(lastIndex+1));
						if (peopleList)
						{
							// don't add if it is the player name
							if (!ClientCfg.Local && (UserEntity->getEntityName() == geb->getInputString()))
							{
								displayVisibleSystemMsg(CI18N::get("uiCantAddYourSelfInContactList"));
							}
							else
							{
								PeopleInterraction.askAddContact(geb->getInputString(), peopleList);
								geb->setInputString(ucstring(""));
							}
						}
					}
					geb->setInputString(ucstring(""));
				}
			}
		}
		CAHManager::getInstance()->runActionHandler("leave_modal", pCaller, "");
	}
};
REGISTER_ACTION_HANDLER( CHandlerAddContact, "add_contact");


//=================================================================================================================
class CHandlerMoveContact : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// retrieve the index of the people
		CPeopleList *srcList;
		uint peopleIndex;
		if (PeopleInterraction.getPeopleFromCurrentMenu(srcList, peopleIndex))
		{
			// get the destination list
			CPeopleList *destList;
			int listIndex;
			if (!fromString(getParam(sParams, "list"), listIndex))
			{
				nlwarning("Bad list index");
				return;
			}
			switch(listIndex)
			{
				case 0:
					destList = &PeopleInterraction.IgnoreList;
				break;
				case 1:
					destList = &PeopleInterraction.FriendList;
				break;
				default: nlwarning("Bad list index"); return;
			}

			PeopleInterraction.askMoveContact(peopleIndex, srcList, destList);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerMoveContact, "move_contact");


//=================================================================================================================
class CHandlerSortContacts : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager* pIM= CInterfaceManager::getInstance();
		nlinfo("Load Order : %d", NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTACT_LIST:SORT_ORDER")->getValue32());
		CPeopleList::TSortOrder order = (CPeopleList::TSortOrder)(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTACT_LIST:SORT_ORDER")->getValue32());

		order = (CPeopleList::TSortOrder)(order + 1);
		if (order == CPeopleList::END_SORT_ORDER)
		{
			order = CPeopleList::START_SORT_ORDER;
		}

		nlinfo("Save Order : %d", order);
		NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTACT_LIST:SORT_ORDER")->setValue32((sint32)order);
		CPeopleList *pl = PeopleInterraction.getPeopleListFromCurrentMenu();
		if (pl)
			pl->sortEx(order);
	}
};
REGISTER_ACTION_HANDLER( CHandlerSortContacts, "sort_contacts");


//=================================================================================================================
// Directly chat with a friend (launch a container chat)
class CHandlerContactDirectChat : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		if (pCaller == NULL)
			return;

		CInterfaceGroup *fatherGC = pCaller->getParent();
		if (fatherGC == NULL)
			return;
		fatherGC = fatherGC->getParent();
		if (fatherGC == NULL)
			return;
		string str = fatherGC->getId().substr(0,fatherGC->getId().rfind('_'));
		str = str.substr(str.rfind(':')+1, str.size());
		CPeopleList *peopleList = PeopleInterraction.getPeopleListFromContainerID(str);
		if (peopleList == NULL)
			return;

		sint index = peopleList->getIndexFromContainerID(fatherGC->getId());
		if (index == -1)
			return;

		peopleList->openCloseChat(index, true);
	}
};
REGISTER_ACTION_HANDLER( CHandlerContactDirectChat, "contact_direct_chat");


////////////////
// PARTY CHAT //
////////////////

//=================================================================================================================
/** Menu to create a new party chat
  */
class CHandlerNewPartyChat : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		nlwarning("Deactivated for now!");
		return;
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(NEW_PARTY_CHAT_WINDOW));
		if (!gc) return;
		CWidgetManager::getInstance()->setTopWindow(gc);
		// Set the keyboard focus
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(gc->getGroup("eb"));
		if (eb)
		{
			CWidgetManager::getInstance()->setCaptureKeyboard(eb);
			eb->setInputString(ucstring(""));
		}
		//
		if (gc->getActive())
		{
			gc->enableBlink(1);
			return;
		}
		gc->setActive(true);
		gc->updateCoords();
		gc->center();
	}
};
REGISTER_ACTION_HANDLER( CHandlerNewPartyChat, "new_party_chat");

//=================================================================================================================
/** The name of a party chat has been validated
  */
class CHandlerValidatePartyChatName : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CGroupContainer *gc = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(NEW_PARTY_CHAT_WINDOW));
		if (!gc) return;
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(gc->getGroup("eb"));
		if (!eb) return;
		ucstring title = eb->getInputString();

		// TODO GAMEDEV : create (or join ?) a new channel. Each channel (party chat) should have a unique name in the game
		// moreover, it should not have the name of another available chat window (for example, it shouldn't be named 'Around Me')
		// It shouldn't have the name of an existing player, either..
		// Maybe this last test can be done in local only

		if (!PeopleInterraction.testValidPartyChatName(title))
		{
			displayVisibleSystemMsg(title + ucstring(" : ") +  CI18N::get("uiInvalidPartyChatName"));
			return;
		}

		// create the party chat
		PeopleInterraction.createNewPartyChat(title);
		return;
	}
};
REGISTER_ACTION_HANDLER(CHandlerValidatePartyChatName, "validate_party_chat_name");


//=================================================================================================================
/** Menu to create a new party chat
  */


//=================================================================================================================
/** Menu to remove a currenlty created party chat
  */
class CHandlerRemovePartyChat : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CChatWindow *chat = getChatWndMgr().getChatWindowFromCaller(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		if (chat) PeopleInterraction.removePartyChat(chat);
	}
};
REGISTER_ACTION_HANDLER( CHandlerRemovePartyChat, "remove_party_chat");

//=================================================================================================================
/** TEMP : just create an 'invite' command in the 'around me' edit box
  */
class CHandlerPartyChatInvite : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CChatWindow *am = PeopleInterraction.AroundMe.Window;
		if (!am) return;
		CCtrlBase *cb = am->getContainer();
		while (cb)
		{
			cb->forceOpen();
			cb = cb->getParent();
		}
		// make the container blink
		am->getContainer()->enableBlink(2);
		// TODO : center the view on the newly created container ?
		// display a new command '/name' in the chat. The player must enter a new unique name for the party chat.
		am->setCommand("invite ", false);
		am->setKeyboardFocus();
	}
};
REGISTER_ACTION_HANDLER( CHandlerPartyChatInvite, "party_chat_invite" );


//=================================================================================================================
/** Add all members of the team to the party chat
  */
class CHandlerAddAllTeamMembersToPartyChat : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
//		CChatWindow *chat = getChatWndMgr().getChatWindowFromCaller(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		// TODO GAMEDEV : add all team members
	}
};
REGISTER_ACTION_HANDLER( CHandlerAddAllTeamMembersToPartyChat, "add_all_team_members");

//=================================================================================================================
/** Remove all members of the team to the party chat
  */
class CHandlerRemoveAllTeamMembersToPartyChat : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
//		CChatWindow *chat = getChatWndMgr().getChatWindowFromCaller(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		// TODO GAMEDEV : remove all team members
	}
};
REGISTER_ACTION_HANDLER( CHandlerRemoveAllTeamMembersToPartyChat, "remove_all_team_members");

//=================================================================================================================
/** Add all members of the guild to the party chat
  */
class CHandlerAddAllGuildMembersToPartyChat : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
//		CChatWindow *chat = getChatWndMgr().getChatWindowFromCaller(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		// TODO GAMEDEV : add all guild members
	}
};
REGISTER_ACTION_HANDLER( CHandlerAddAllGuildMembersToPartyChat, "add_all_guild_members");

//=================================================================================================================
/** Remove all members of the team to the party chat
  */
class CHandlerRemoveAllGuildMembersToPartyChat : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
//		CChatWindow *chat = getChatWndMgr().getChatWindowFromCaller(CWidgetManager::getInstance()->getCtrlLaunchingModal());
		// TODO_GAMEDEV : remove all guild members
	}
};
REGISTER_ACTION_HANDLER( CHandlerRemoveAllGuildMembersToPartyChat, "remove_all_guild_members");

/////////////////////////////////////////
// ACTION HANDLERS FOR MAIN/USER CHATS //
/////////////////////////////////////////

//=================================================================================================================
/** Select the target on a filtered chat window
  * This create a menu with the standard window (team, around me ...) + the party chat windows
  */
class CHandlerSelectChatTarget : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CChatWindow	*cw = getChatWndMgr().getChatWindowFromCaller(pCaller);
		if (!cw) return;
		ChatWindowForFilter = cw;
		CInterfaceManager *im = CInterfaceManager::getInstance();
		//
		std::string menuName = getParam(sParams, "menu");
		std::string strPartyChats = getParam(sParams, "party_chats");
		bool partyChats = true;
		if (!strPartyChats.empty())
		{
			partyChats = nlstricmp("true", strPartyChats.c_str()) == 0;
		}
		// get the menu
		CGroupMenu *menu = dynamic_cast<CGroupMenu *>(CWidgetManager::getInstance()->getElementFromId(menuName));
		if (!menu) return;
		// remove all party chat from the previous list
		uint lastTargetSelectedIndex = 0;
		for(uint k = 0; k < menu->getNumLine();)
		{
			if (nlstricmp("chat_target_selected", menu->getActionHandler(k)) == 0)
			{
				lastTargetSelectedIndex = k;
				int dummy;
				if (fromString(menu->getActionHandlerParam(k), dummy))
				{
					// this is a party chat, removes the entry
					menu->deleteLine(k);
					-- lastTargetSelectedIndex;
				}
				else
				{
					++k;
				}
			}
			else
			{
				++k;
			}
		}

		CPeopleInterraction &pl = PeopleInterraction;
		// add names of the party chats
		uint insertionIndex = lastTargetSelectedIndex + 1; // insert after standard options
		if (partyChats)
		{
			for(uint l = 0; l < pl.PartyChats.size(); ++l)
			{
				menu->addLineAtIndex(insertionIndex, pl.PartyChats[l].Window->getTitle(), "chat_target_selected", toString(pl.PartyChats[l].ID));
				++ insertionIndex;
			}
		}

		// Case of user chat in grouped chat window
		if ((cw == PeopleInterraction.ChatGroup.Window) || (cw = PeopleInterraction.TheUserChat.Window))
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			cw = PeopleInterraction.TheUserChat.Window;
//			CChatStdInput &ci = PeopleInterraction.ChatInput;
			CGroupMenu *pMenu = dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:user_chat_target_menu"));
			CViewTextMenu *pMenuAround	= dynamic_cast<CViewTextMenu*>(pMenu->getElement("ui:interface:user_chat_target_menu:around"));
			CViewTextMenu *pMenuRegion	= dynamic_cast<CViewTextMenu*>(pMenu->getElement("ui:interface:user_chat_target_menu:region"));
			CViewTextMenu *pMenuUniverse	= dynamic_cast<CViewTextMenu*>(pMenu->getElement("ui:interface:user_chat_target_menu:universe"));
			CViewTextMenu *pMenuTeam	= dynamic_cast<CViewTextMenu*>(pMenu->getElement("ui:interface:user_chat_target_menu:team"));
			CViewTextMenu *pMenuGuild	= dynamic_cast<CViewTextMenu*>(pMenu->getElement("ui:interface:user_chat_target_menu:guild"));
			const bool teamActive = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GROUP:0:PRESENT")->getValueBool();
			const bool guildActive = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:NAME")->getValueBool();
			if (pMenuAround)	pMenuAround->setGrayed	(false);
			if (pMenuRegion)	pMenuRegion->setGrayed	(false);
			if (pMenuUniverse)	pMenuUniverse->setGrayed	(false);
			if (pMenuTeam)		pMenuTeam->setGrayed	(!teamActive);
			if (pMenuGuild)		pMenuGuild->setGrayed	(!guildActive);

			// Remove existing dynamic chats
			while (pMenu->getNumLine() > 5)
			{
				pMenu->deleteLine(pMenu->getNumLine()-1);
			}

			// Add dynamic chats
			uint insertion_index = 0;
			for (uint i = 0; i < CChatGroup::MaxDynChanPerPlayer; i++)
			{
				string s = toString(i);
				uint32 textId = ChatMngr.getDynamicChannelNameFromDbIndex(i);
				bool active = (textId != 0);
				if (active)
				{
					uint32 canWrite = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:DYN_CHAT:CHANNEL"+s+":WRITE_RIGHT")->getValue32();
					if (canWrite != 0)
					{
						ucstring title;
						STRING_MANAGER::CStringManagerClient::instance()->getDynString(textId, title);
						pMenu->addLineAtIndex(5 + insertion_index, title+" @{T8}/"+s, "chat_target_selected", "dyn"+s, "dyn"+s);
						insertion_index++;
					}
				}
			}
		}

		// activate the menu
		CWidgetManager::getInstance()->enableModalWindow(pCaller, menuName);
	}
};
REGISTER_ACTION_HANDLER( CHandlerSelectChatTarget, "select_chat_target");

//=================================================================================================================
/** A target has been selected for a filtered chat
  */
class CHandlerChatTargetSelected : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		// for now, manage a single filtered chat window
		CChatWindow	*cw = ChatWindowForFilter;
		if (!cw) return;
		ChatWindowForFilter = NULL;
		CFilteredChat *fc = PeopleInterraction.getFilteredChatFromChatWindow(cw);
		if (!fc) return;
		CChatTargetFilter &cf = fc->Filter;
		// Team
		if (nlstricmp(sParams, "team") == 0)
		{
			cf.setTargetGroup(CChatGroup::team);
		}
		// Guild
		else if (nlstricmp(sParams, "guild") == 0)
		{
			cf.setTargetGroup(CChatGroup::guild);
		}
		// Say
		else if (nlstricmp(sParams, "say") == 0)
		{
			cf.setTargetGroup(CChatGroup::say);
		}
		// Shout
		else if (nlstricmp(sParams, "shout") == 0)
		{
			cf.setTargetGroup(CChatGroup::shout);
		}
		// Region
		else if (nlstricmp(sParams, "region") == 0)
		{
			cf.setTargetGroup(CChatGroup::region);
		}
		// Universe
		else if (nlstricmp(sParams, "universe") == 0)
		{
			cf.setTargetGroup(CChatGroup::universe);
		}
		else
		{
			for (uint i = 0; i < CChatGroup::MaxDynChanPerPlayer; i++)
			{
				if (nlstricmp(sParams, "dyn"+toString("%d", i)) == 0)
				{
					cf.setTargetGroup(CChatGroup::dyn_chat, i);
				}
			}
		}

		// Case of user chat in grouped chat window
		if (cw == PeopleInterraction.ChatGroup.Window)
		{
			PeopleInterraction.TheUserChat.Filter.setTargetGroup(cf.getTargetGroup(), cf.getTargetDynamicChannelDbIndex());
			CAHManager::getInstance()->runActionHandler("chat_group_filter", NULL, "user");
		}
		if (cw == PeopleInterraction.TheUserChat.Window)
		{
			PeopleInterraction.TheUserChat.Filter.setTargetGroup(cf.getTargetGroup(), cf.getTargetDynamicChannelDbIndex());
			CAHManager::getInstance()->runActionHandler("user_chat_active", NULL, "");
		}

		// The target should be a party chat
		int partyChatID;
		if (fromString(sParams, partyChatID))
		{
			// search party chat in the list
			std::vector<CPartyChatInfo> &partyChats = PeopleInterraction.PartyChats;
			for(uint k = 0; k < partyChats.size(); ++k)
			{
				if (partyChats[k].ID == (uint) partyChatID)
				{
					cf.setTargetPartyChat(partyChats[k].Window);
					return;
				}
			}
			// The party chat has been deleted while the menu was displayed it seems.. -> no-op
			return;
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerChatTargetSelected, "chat_target_selected");


//=================================================================================================================
/** If no more in team, leave team chat mode
  */
class CHandlerLeaveTeamChat : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if( PeopleInterraction.TheUserChat.Filter.getTargetGroup() == CChatGroup::team )
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			if( im )
			{
				if( !NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:IS_TEAM_PRESENT")->getValueBool() )
				{
					ChatMngr.updateChatModeAndButton(CChatGroup::say);
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerLeaveTeamChat, "leave_team_chat");





/** Create checkbox for a menu.
  */
static CInterfaceGroup *createMenuCheckBox(const std::string &onclickL, const std::string &paramsL, bool checked)
{
	pair<string, string> params [2];
	params[0].first = "onclick_l";
	params[0].second = onclickL;
	params[1].first = "params_l";
	params[1].second = paramsL;

	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceGroup *ig = CWidgetManager::getInstance()->getParser()->createGroupInstance("menu_checkbox", "", params, sizeof(params) / sizeof(params[0]));
	if (!ig) return NULL;
	CCtrlBaseButton *cb = dynamic_cast<CCtrlBaseButton *>(ig->getCtrl("b"));
	if (!cb) return NULL;
	cb->setPushed(checked);
	return ig;
}



//=================================================================================================================
/** Display a menu to select the source on a filtered chat
  */
class CHandlerSelectChatSource : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		static const char *FILTER_TOGGLE = "chat_source_selected";
		CPeopleInterraction &pi = PeopleInterraction;
		CChatWindow	*cw = getChatWndMgr().getChatWindowFromCaller(pCaller);
		if (!cw) return;
		ChatWindowForFilter = cw;
		CInterfaceManager *im = CInterfaceManager::getInstance();


		// *** get the main_chat or user_chat menu
		CGroupMenu *menu= NULL;
		bool	addUserChatEntries= false;
		// If the current window is the chat group
		if (cw == pi.ChatGroup.Window)
		{
			// select main chat menu
			menu = dynamic_cast<CGroupMenu *>(CWidgetManager::getInstance()->getElementFromId(MAIN_CHAT_SOURCE_MENU));

			// Remove all unused dynamic channels and set the names
			for (uint i = 0; i < CChatGroup::MaxDynChanPerPlayer; i++)
			{
				string s = toString(i);
				CViewTextMenu *pVTM = dynamic_cast<CViewTextMenu *>(CWidgetManager::getInstance()->getElementFromId(MAIN_CHAT_SOURCE_MENU+":tab:dyn"+s));
				if (pVTM)
				{
					uint32 textId = ChatMngr.getDynamicChannelNameFromDbIndex(i);
					bool active = (textId != 0);
					pVTM->setActive(active);
					if (active)
					{
						ucstring title;
						STRING_MANAGER::CStringManagerClient::instance()->getDynString(textId, title);
						pVTM->setText("["+s+"] " + title);
					}
				}
			}

			// Menu with Filters
			CChatGroupWindow *pWin = pi.getChatGroupWindow();
			if (pWin->getTabIndex() == 5) // (5 == user) -> complete menu
			{
				// get the real user chat setup
				cw = pi.TheUserChat.Window;
				addUserChatEntries= true;
			}
			else
			{
				// Don't add user chat since the selected TAB is not the user chat
				addUserChatEntries= false;
			}
		}
		else
		{
			// Menu with Filters
			if (cw == pi.TheUserChat.Window)
			{
				// select user chat menu
				menu = dynamic_cast<CGroupMenu *>(CWidgetManager::getInstance()->getElementFromId(USER_CHAT_SOURCE_MENU));
				addUserChatEntries= true;
			}
			// Simple menu
			else
			{
				// This is neither the ChatGroup, nor the UserChat. Should not be here.
				// Just open the STD chat menu, and quit
				NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:GC_POPUP")->setValue64(cw->getContainer()->isPopuped() || cw->getContainer()->getLayerSetup() == 0 ? 1 : 0);
				NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:GC_HAS_HELP")->setValue64(!cw->getContainer()->getHelpPage().empty());
				CWidgetManager::getInstance()->enableModalWindow(pCaller, STD_CHAT_SOURCE_MENU);
				return;
			}
		}
		if (!menu) return;


		// *** remove any previous entries
		for(uint k = 0; k < menu->getNumLine();)
		{
			if (nlstricmp(FILTER_TOGGLE, menu->getActionHandler(k)) == 0)
			{
				menu->deleteLine(k);
			}
			else
			{
				++k;
			}
		}


		// *** create new entries
		if(addUserChatEntries)
		{
			uint insertionIndex = 0;
			// AROUND ME
			menu->addLineAtIndex(insertionIndex, CI18N::get("uiAroundMe"), FILTER_TOGGLE, "am");
			// add a checkbox
			menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, "am", pi.ChatInput.AroundMe.isListeningWindow(cw)));
			++ insertionIndex;

			// REGION
			menu->addLineAtIndex(insertionIndex, CI18N::get("uiREGION"), FILTER_TOGGLE, "region");
			// add a checkbox
			menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, "region", pi.ChatInput.Region.isListeningWindow(cw)));
			++ insertionIndex;

			// UNIVERSE
			menu->addLineAtIndex(insertionIndex, CI18N::get("uiUNIVERSE"), FILTER_TOGGLE, "universe");
			// add a checkbox
			menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, "universe", pi.ChatInput.Universe.isListeningWindow(cw)));
			++ insertionIndex;

			// TEAM
			menu->addLineAtIndex(insertionIndex, CI18N::get("uiTeam"), FILTER_TOGGLE, "team");
			// add a checkbox
			menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, "team", pi.ChatInput.Team.isListeningWindow(cw)));
			++insertionIndex;

			// GUILD
			menu->addLineAtIndex(insertionIndex, CI18N::get("uimGuild"), FILTER_TOGGLE, "guild");
			// add a checkbox
			menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, "guild", pi.ChatInput.Guild.isListeningWindow(cw)));
			++ insertionIndex;


			// TELL
			//menu->addLineAtIndex(insertionIndex, CI18N::get("uiTell"), FILTER_TOGGLE, "tell");
			// add a checkbox
			//menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, "tell", pi.ChatInput.Tell.isListeningWindow(cw)));
			//++ insertionIndex;

			// SYSTEM INFOS
			menu->addLineAtIndex(insertionIndex, CI18N::get("uiSystemInfo"), FILTER_TOGGLE, "si");
			// add a checkbox
			menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, "si", pi.ChatInput.SystemInfo.isListeningWindow(cw)));
			++insertionIndex;

			// add party chats
			std::vector<CPartyChatInfo> &pc = pi.PartyChats;
			for(uint l = 0; l < pc.size(); ++l)
			{
				if (pc[l].Filter != NULL)
				{
					menu->addLineAtIndex(insertionIndex, pc[l].Window->getTitle(), FILTER_TOGGLE, toString(pc[l].ID));
					menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, toString(pc[l].ID), pc[l].Filter->isListeningWindow(cw)));
					++ insertionIndex;
				}
			}

			// Add all existing dynamic channels and set the names
			for (uint8 i = 0; i < CChatGroup::MaxDynChanPerPlayer; i++)
			{
				string s = toString(i);
				uint32 textId = ChatMngr.getDynamicChannelNameFromDbIndex(i);
				bool active = (textId != 0);
				if (active)
				{
					ucstring title;
					STRING_MANAGER::CStringManagerClient::instance()->getDynString(textId, title);
					menu->addLineAtIndex(insertionIndex, "["+s+"] " + title, FILTER_TOGGLE, "dyn"+s);
					menu->setUserGroupLeft(insertionIndex, createMenuCheckBox(FILTER_TOGGLE, "dyn"+s, pi.ChatInput.DynamicChat[i].isListeningWindow(cw)));
					++insertionIndex;
				}
			}

		}


		// *** active the menu
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:GC_POPUP")->setValue64(cw->getContainer()->isPopuped() || cw->getContainer()->getLayerSetup() == 0 ? 1 : 0);
		NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:GC_HAS_HELP")->setValue64(!cw->getContainer()->getHelpPage().empty());
		CWidgetManager::getInstance()->enableModalWindow(pCaller, menu);
	}
};
REGISTER_ACTION_HANDLER(CHandlerSelectChatSource, "select_chat_source");



//=================================================================================================================
/** A new source has been selected / unselected from a filtered chat
  */
class CHandlerChatSourceSelected : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		int partyChatID;

		CChatWindow	*cw = ChatWindowForFilter;
		if (!cw) return;

		CChatStdInput &ci = PeopleInterraction.ChatInput;


		if (cw == PeopleInterraction.ChatGroup.Window)
		{
			CChatGroupWindow *pWin = PeopleInterraction.getChatGroupWindow();
			if (pWin->getTabIndex() != 5) // (5 == user)
				return; // Nothing to select except if user chat

			cw = PeopleInterraction.TheUserChat.Window;
		}


		/*CCtrlBaseButton *button = dynamic_cast<CCtrlBaseButton *>(pCaller);
		if (button)
		{
			button->setPushed(!button->getPushed());
		}*/
		// GUILD
		if (nlstricmp(sParams, "guild") == 0)
		{
			if (ci.Guild.isListeningWindow(cw)) ci.Guild.removeListeningWindow(cw);
			else ci.Guild.addListeningWindow(cw);
		}
		else
		// TEAM
		if (nlstricmp(sParams, "team") == 0)
		{
			if (ci.Team.isListeningWindow(cw)) ci.Team.removeListeningWindow(cw);
			else ci.Team.addListeningWindow(cw);
		}
		else
		// AROUND ME
		if (nlstricmp(sParams, "am") == 0)
		{
			if (ci.AroundMe.isListeningWindow(cw)) ci.AroundMe.removeListeningWindow(cw);
			else ci.AroundMe.addListeningWindow(cw);
		}
		else
		// REGION
		if (nlstricmp(sParams, "region") == 0)
		{
			if (ci.Region.isListeningWindow(cw)) ci.Region.removeListeningWindow(cw);
			else ci.Region.addListeningWindow(cw);
		}
		else
		// UNIVERSE
		if (nlstricmp(sParams, "universe") == 0)
		{
			if (ci.Universe.isListeningWindow(cw)) ci.Universe.removeListeningWindow(cw);
			else ci.Universe.addListeningWindow(cw);
		}
		else
		// TELL
		if (nlstricmp(sParams, "tell") == 0)
		{
			if (ci.Tell.isListeningWindow(cw)) ci.Tell.removeListeningWindow(cw);
			else ci.Tell.addListeningWindow(cw);
		}
		else
		// SYSTEM INFOS
		if (nlstricmp(sParams, "si") == 0)
		{
			if (ci.SystemInfo.isListeningWindow(cw)) ci.SystemInfo.removeListeningWindow(cw);
			else ci.SystemInfo.addListeningWindow(cw);
		}
		else
		// PARTY CHAT
		if (fromString(sParams, partyChatID))
		{
			std::vector<CPartyChatInfo> &partyChats = PeopleInterraction.PartyChats;
			for(uint k = 0; k < partyChats.size(); ++k)
			{
				if (partyChats[k].ID == (uint) partyChatID)
				{
					if (partyChats[k].Filter != NULL)
					{
						if (partyChats[k].Filter->isListeningWindow(cw)) partyChats[k].Filter->removeListeningWindow(partyChats[k].Window);
						else partyChats[k].Filter->addListeningWindow(cw);
					}
				}
			}
		}
		else if (nlstricmp(sParams.substr(0, 3), "dyn") == 0)
		{
			uint8 i = 0;
			fromString(sParams.substr(3), i);
			if (ci.DynamicChat[i].isListeningWindow(cw)) ci.DynamicChat[i].removeListeningWindow(cw);
			else ci.DynamicChat[i].addListeningWindow(cw);
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerChatSourceSelected, "chat_source_selected");


// show / hide the edit/box of a chatbox
class CHandlerToggleChatEBVis : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CCtrlBase *clm = CWidgetManager::getInstance()->getCtrlLaunchingModal();
		if (!clm) return;
		CInterfaceGroup *ig = clm->getParent();
		do
		{
			if (ig->isGroupContainer()) break;
			ig = ig->getParent();
		}
		while(ig);
		if (!ig) return;
		CGroupContainer *gc = static_cast<CGroupContainer *>(ig);
		CInterfaceGroup *eb = gc->getGroup("ebw");
		if (eb)
		{
			eb->setActive(!eb->getActive());
		}
		CCtrlBase *tb = gc->getCtrl("target_button");
		if (tb)
		{
			tb->setActive(!tb->getActive());
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerToggleChatEBVis, "toggle_chat_eb_vis");

// create a new user chat
class CHandlerNewUserChat : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CPeopleInterraction &pi = PeopleInterraction;
		for(uint k = 0; k < MaxNumUserChats; ++k)
		{
			if (pi.UserChat[k].Window == NULL) // not used ?
			{
				pi.createUserChat(k);
				// add to std listeners
				pi.ChatInput.registerListeningWindow(pi.UserChat[k].Window);
				CGroupContainer *gc = pi.UserChat[k].Window->getContainer();
				gc->setOpen(true);
				gc->popupCurrentPos();
				gc->updateCoords();
				gc->center();
				// change pos by a random amount
				gc->setX(gc->getX() + rand() % 20 - 10);
				gc->setY(gc->getY() + rand() % 20 - 10);
				gc->invalidateCoords();
				gc->enableBlink(2);
				pi.UserChat[k].Window->setKeyboardFocus();
				return;
			}
		}
		nlwarning("Too much user chats created");
	}
};
REGISTER_ACTION_HANDLER(CHandlerNewUserChat, "new_user_chat");

class CHandlerRemoveUserChat : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CPeopleInterraction &pi = PeopleInterraction;
		CChatWindow *cw = getChatWndMgr().getChatWindowFromCaller(pCaller);
		if (!cw) return;
		CFilteredChat *fc = pi.getFilteredChatFromChatWindow(cw);
		if (!fc) return;
		getChatWndMgr().removeChatWindow(fc->Window);
		fc->Filter.reset();
		fc->Window = NULL;
	}
};
REGISTER_ACTION_HANDLER(CHandlerRemoveUserChat, "remove_user_chat");

////////////////////////////////////////////
// COMMAND RELATED TO PEOPLE INTERRACTION //
////////////////////////////////////////////


//-----------------------------------------------
// 'ignore'
//-----------------------------------------------
NLMISC_COMMAND(ignore, "add or remove a player from the ignore list", "<player name>")
{
	// Check parameters.
	if(args.size() < 1)
	{
		return false;
	}

	// NB: playernames cannot have special characters
	ucstring playerName = ucstring(args[0]);

	// add to the ignore list
	PeopleInterraction.askAddContact(playerName, &PeopleInterraction.IgnoreList);

	return true;
} // ignore //

/*
  ****
  Yoyo: Party chat is not ended: DON'T LET THOSE COMMANDS AVAILABLE!
  they made the client crash (cf createNewPartyChat)...
  ****

// create a new party chat with the given name
NLMISC_COMMAND(party_chat, "Create a new party chat", "<party_chat_name>")
{
	if (args.size() != 1)
	{
		displayVisibleSystemMsg(CI18N::get("uiPartyChatCmd"));
		return true;
	}
	CPeopleInterraction &pi = PeopleInterraction;
	ucstring title = args[0];

	if (!pi.testValidPartyChatName(title))
	{
		displayVisibleSystemMsg(CI18N::get("uiInvalidPartyChatName"));
		return true;
	}

	PeopleInterraction.createNewPartyChat(title);
	return true;
}

// Remove the party chat with the given name
NLMISC_COMMAND(remove_party_chat, "Remove a party chat", "<party_chat_name>")
{
	if (args.size() != 1)
	{
		displayVisibleSystemMsg(CI18N::get("uiRemovePartyChatCmd"));
		return true;
	}
	ucstring title = ucstring(args[0]);
	CChatWindow *chat = getChatWndMgr().getChatWindow(title);
	if (!chat)
	{
		displayVisibleSystemMsg(title + ucstring(" : ") + CI18N::get("uiBadPartyChatName"));
		return true;
	}
	if (!PeopleInterraction.removePartyChat(chat))
	{
		displayVisibleSystemMsg(title + ucstring(" : ") + CI18N::get("uiCantRemovePartyChat"));
		return true;
	}
	return true;
}


// Join a party chat whose name is known
NLMISC_COMMAND(add_to_party_chat, "Join the given party chat", "<party_chat_name>")
{
	if (args.size() != 1)
	{
		displayVisibleSystemMsg(CI18N::get("uiAddPartyChatCmd"));
		return true;
	}
	// TODO GAMEDEV : join the party chat
	return true;
}

// Invite someone in a party chat
NLMISC_COMMAND(invite, "Invite someone to a party chat", "<people_name> <party_chat_name>")
{
	if (args.size() != 2)
	{
		displayVisibleSystemMsg(CI18N::get("uiInviteCmd"));
		return true;
	}
	// TODO GAMEDEV : Send invite message to the server
	//                Check that the inviter has created the chat ?
	//                The people being invited should receive a popup to announce that he is being invited
	return true;
}

*/

// ***************************************************************************
// chatLog
// Arg : none
// log all current chats in the file log_playername.txt saved in save directory
// ***************************************************************************
NLMISC_COMMAND(chatLog, "", "")
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if(args.size() != 0)
		return false;

	if (pIM->getLogState())
		pIM->displaySystemInfo(CI18N::get("uiLogTurnedOff"));

	pIM->setLogState(!pIM->getLogState());

	if (pIM->getLogState())
		pIM->displaySystemInfo(CI18N::get("uiLogTurnedOn"));

	CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHATLOG_STATE", false);
	if (node)
	{
		node->setValue32(pIM->getLogState() ? 1 : 0);
	}

	return true;
};


/////////////////////////
// INTERFACE FUNCTIONS //
/////////////////////////
static DECLARE_INTERFACE_USER_FCT(getNumUserChatLeft)
{
	CPeopleInterraction &pi = PeopleInterraction;
	uint left = 0;
	for(uint k = 0; k < MaxNumUserChats; ++k)
	{
		if (pi.UserChat[k].Window == NULL) ++ left;
	}
	result.setInteger(left);
	return true;
}
REGISTER_INTERFACE_USER_FCT("getNumUserChatLeft", getNumUserChatLeft)


//////////////////////////////////////
// STATIC FUNCTIONS IMPLEMENTATIONS //
//////////////////////////////////////

static void displayVisibleSystemMsg(const ucstring &msg, const string &cat)
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->displaySystemInfo(msg, cat);
	if (CChatWindow::getChatWindowLaunchingCommand())
	{
		CChatWindow::getChatWindowLaunchingCommand()->displayMessage(msg, im->getSystemInfoColor(cat), CChatGroup::system, 0, 2);
	}
}

#if !FINAL_VERSION
NLMISC_COMMAND(testSI, "tmp", "tmp")
{
	PeopleInterraction.ChatInput.DebugInfo.displayMessage(ucstring("test"), CRGBA::Red);
	return true;
}
#endif
