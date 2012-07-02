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

#ifndef CL_PEOPLE_INTERRACTION_H
#define CL_PEOPLE_INTERRACTION_H

#include "people_list.h"
#include "chat_filter.h"

#include "nel/misc/smart_ptr.h"


class CChatWindow;
class CFilteredChatSummary;
class CFilteredDynChatSummary;

// ***************************************************************************
#define TEAM_DB_PATH	"SERVER:GROUP"


// ***************************************************************************
// max number of people in the team
const uint MaxNumPeopleInTeam = 8;

// max number of user chats
const uint MaxNumUserChats = 5;


// ***************************************************************************
/** Infos about a party chat
  */
struct CPartyChatInfo
{
	uint32           ID;
	CChatWindow      *Window;
	NLMISC::CSmartPtr<CChatInputFilter> Filter;
};


// ***************************************************************************
// a chat filter and the associated window
class CFilteredChat
{
public:
	CChatWindow *Window;
	CChatTargetFilter	Filter;
public:
	CFilteredChat() : Window(NULL) {}
	~CFilteredChat()
	{
		if (Window)
		{
			nlwarning("window not released");
		}
	}
	void release()
	{
		if (Window)
		{
			CChatWindowManager &cwm = getChatWndMgr();
			cwm.removeChatWindow(Window);
			Window = NULL;
		}
	}
};


// ***************************************************************************
/** Standard inputs for the chat
  */
class CChatStdInput
{
public:
	CChatInputFilter AroundMe;
	CChatInputFilter Region;
	CChatInputFilter Team;
	CChatInputFilter Guild;
	CChatInputFilter SystemInfo;
	CChatInputFilter Universe;

	CChatInputFilter Tell;

	CChatInputFilter DebugInfo;
	// YuboChat (special telnet chat for Game Masters, same channel as the Yubo Klient)
	CChatInputFilter YuboChat;

	// Dynamic Chat. A fixed number of chat that can be assign
	CChatInputFilter DynamicChat[CChatGroup::MaxDynChanPerPlayer];

public:
	void registerListeningWindow(CChatWindow *cw);

	CChatStdInput()
	{
		AroundMe.FilterType = CChatGroup::arround;
		Region.FilterType = CChatGroup::region;
		Team.FilterType = CChatGroup::team;
		Guild.FilterType = CChatGroup::guild;
		SystemInfo.FilterType = CChatGroup::system;
		Tell.FilterType = CChatGroup::tell;
		YuboChat.FilterType = CChatGroup::yubo_chat;
		Universe.FilterType = CChatGroup::universe;
		for(uint i=0;i<CChatGroup::MaxDynChanPerPlayer;i++)
		{
			DynamicChat[i].FilterType= CChatGroup::dyn_chat;
			DynamicChat[i].DynamicChatDbIndex= i;
		}
	}
};




// ***************************************************************************
/** Class that manages interractions with other people.
  */
class CPeopleInterraction
{
public:
	// the various people lists
	CPeopleList					TeamList;
	CPeopleList					FriendList; // in contact_list
	CPeopleList					IgnoreList; // in contact_list
	// Shortcut to chat windows	(also available in the chat window manager
	CChatWindow					*Region;
	CChatWindow					*Universe;
	CChatWindow					*TeamChat;
	CChatWindow					*GuildChat;
	CChatWindow					*SystemInfo;
	CChatWindow					*TellWindow;
	CChatWindow					*DebugInfo;
	// (special telnet chat for Game Masters, same channel as the Yubo Klient)
	CChatWindow					*YuboChat;
	// Special dynamic chat
	CChatWindow					*DynamicChat[CChatGroup::MaxDynChanPerPlayer];

	// List of all created party chat
	std::vector<CPartyChatInfo> PartyChats;
	uint32						CurrPartyChatID;
	//
	CChatStdInput				ChatInput;
	//
	//CFilteredChat				MainChat;
	CFilteredChat				ChatGroup;
	CFilteredChat				AroundMe;
	CFilteredChat				DebugConsole;
	// additionnal user chats
	CFilteredChat			    UserChat[MaxNumUserChats];
	CFilteredChat			    TheUserChat;
	// Id of last people who talked
	ucstring					LastSenderName;

	// system message
	struct CSysMsg
	{
		ucstring Str;
		std::string Cat;
	};
	// system message buffer
	std::vector<CSysMsg>		SystemMessageBuffer;

public:
	// ctor
	CPeopleInterraction();
	//
	void				init();    // init the people lists, create basic chats
	//
	void				initAfterLoad();    // must call after load time to reset prompt color, default channel etc...
	//
	void				release(); // removes every people list & chat windows
	//
	/** from a group container ID, returns a pointer on the people list and a people index
	  * \return false is the id was invalid
	  */
	bool				getPeopleFromContainerID(const std::string &id, CPeopleList *&peopleList, uint &index);
	/** Get the people list & index that triggered the current menu.
	  */
	bool				getPeopleFromCurrentMenu(CPeopleList *&peopleList, uint &index);
	/** Get the people list that triggered the current menu
	  */
	CPeopleList			*getPeopleListFromCurrentMenu();
	/** Get a people list from its container id
	  */
	CPeopleList			*getPeopleListFromContainerID(const std::string &id);

	/** From a window, get the associated filtered chat (or NULL if none)
	  */
	CFilteredChat		*getFilteredChatFromChatWindow(CChatWindow *cw);

	bool				testValidPartyChatName(const ucstring &name);
	bool				removePartyChat(CChatWindow *window);
	void				removeAllPartyChat();
	/**
	  * create a named party chat.
	  */
	bool				createNewPartyChat(const ucstring &title);

	static void			assignPartyChatMenu(CChatWindow *partyChat);

	/// \name CONTACT LIST
	// @{
	// ask the server to add/move/remove a contact
	void askAddContact(const ucstring &contactName, CPeopleList *pl);
	void askMoveContact(uint peopleIndexInSrc, CPeopleList *plSRC, CPeopleList *plDST);
	void askRemoveContact(uint peopleIndex, CPeopleList *pl);

	// init contact list (from server typically)
	void initContactLists(	const std::vector<uint32> &vFriendListName,
							const std::vector<TCharConnectionState> &vFriendListOnline,
							const std::vector<ucstring> &vIgnoreListName	);
	// Friend list == 0 // Ignore list == 1
	void addContactInList(uint32 contactId, uint32 nameID, TCharConnectionState Online, uint8 nList);
	void addContactInList(uint32 contactId, const ucstring &name, TCharConnectionState Online, uint8 nList);
	bool isContactInList(const ucstring &name, uint8 nList) const;
	// Called each frame to receive name from IOS
	void updateWaitingContacts();
	// server decide to remove a contact (if it does not exists anymore)
	void removeContactFromList(uint32 contactId, uint8 nList);
	// server update the online status
	void updateContactInList(uint32 contactId, TCharConnectionState online, uint nList);
	// @}

	// save info about user chats
	bool				saveUserChatsInfos(NLMISC::IStream &f);
	// restore infos about user chats
	bool				loadUserChatsInfos(NLMISC::IStream &f);

	// save info about user dyn chats
	bool				saveUserDynChatsInfos(NLMISC::IStream &f);
	// restore info about user dyn chats
	bool				loadUserDynChatsInfos(NLMISC::IStream &f);

	// remove all the user chats
	void				removeAllUserChats();

	// refrech the 'active' state of user chats : useful when a virtual desktop change occurs
	void				refreshActiveUserChats();

	// Create a user chat at the given index. The target user chat must be empty
	void				createUserChat(uint index);

	// Test if the given chat is a user chat (this includes the main chat)
	bool				isUserChat(CChatWindow *cw) const;

	void				talkInDynamicChannel(uint32 channelNb,ucstring sentence);

	CChatGroupWindow	*getChatGroupWindow() const;

	void updateAllFreeTellerHeaders();
	void removeAllFreeTellers();

	static void displayTellInMainChat(const ucstring &playerName);
private:
	// create various chat & people lists
	void createTeamChat();
	void createTeamList();
	void createFriendList();
	void createIgnoreList();
	//
	void createSystemInfo();
	void createAroundMeWindow();
	void createRegionWindow();
	void createUniverseWindow();
	void createTellWindow();
	void createGuildChat();
	void createDebugInfo();
	void createChatGroup(); // Create chat group containing all other chat
	void createTheUserChat();
	void createYuboChat();	// (special telnet chat for Game Masters, same channel as the Yubo Klient)
	void createDynamicChats();
	//
	void initStdInputs();
	//
	// build summary about a filtered chat
	void buildFilteredChatSummary(const CFilteredChat &src, CFilteredChatSummary &dest);
	void buildFilteredDynChatSummary(const CFilteredChat &src, CFilteredDynChatSummary &dest);
	void saveFilteredChat(NLMISC::IStream &f, const CFilteredChat &src);
	void saveFilteredDynChat(NLMISC::IStream &f, const CFilteredChat &src);
	// setup a user chat from its summary
	void setupUserChatFromSummary(const CFilteredChatSummary &summary, CFilteredChat &dest);
	void setupUserDynChatFromSummary(const CFilteredDynChatSummary &summary, CFilteredChat &dest);
private:
	// Contact waiting their name (received by string_manager) to be added
	struct SWaitingContact
	{
		uint32	ContactId;
		uint32	NameId;
		uint8	List;
		TCharConnectionState	Online;
	};
	std::vector<SWaitingContact>	WaitingContacts;
};

// instance of class that manage people lists
extern CPeopleInterraction PeopleInterraction;

#endif
