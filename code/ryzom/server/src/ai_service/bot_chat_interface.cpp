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
#if 0
#error "Deprecated"
//#include "bot_chat_interface.h"
#include "game_share/synchronised_message.h"
#include "game_share/bot_chat_types.h"

/*
// Nel Misc
#include "nel/net/unified_network.h"

// Game share
#include "game_share/news_types.h"
#include "game_share/bot_chat_types.h"

// Local includes
#include "bot_chat_interface.h"
*/

using namespace NLMISC;
using namespace NLNET;
using namespace std;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// the parent class for bot chat page type classes

class CBotChatPageType
{
public:
	// virtual interface ----------------------------------------------------
	virtual bool open(const CEntityId &player, const CEntityId &bot)=0;
	virtual bool close(const CEntityId &player, const CEntityId &bot)=0;
};


//////////////////////////////////////////////////////////////////////////////
// the structure for bot chat pages

struct SBotChatPage
{
	// ctor -----------------------------------------------------------------
	SBotChatPage(
		BOTCHATTYPE::TBotChatInterfaceId	clientInterfaceId,
		CBotChatPageType *					chatPageType,
		uint								numOptions	
		)
	{
		ClientInterfaceId=	clientInterfaceId;
		PageType=			chatPageType;
		NumOptions=			numOptions;	
	}

	// data -----------------------------------------------------------------
	BOTCHATTYPE::TBotChatInterfaceId	ClientInterfaceId;	// id of interface to display on client
	CBotChatPageType *					PageType;			// type of chat page
	uint								NumOptions;			// number of options for player to click on
};


//////////////////////////////////////////////////////////////////////////////
// the structure for a state for bot chat automatons

struct SBotChatAutomatonState
{
	// public data ----------------------------------------------------------
	SBotChatPage *Page;
	uint On[5];				// value to return on player click of slot 0..4

	// ctor -----------------------------------------------------------------
	SBotChatAutomatonState(SBotChatPage	*page,uint on0=~0u,uint on1=~0u,uint on2=~0u,uint on3=~0u,uint on4=~0u)
	{
		Page=page;
		On[0]=on0;
		On[1]=on1;
		On[2]=on2;
		On[3]=on3;
		On[4]=on4;

		// make sure the number of arguments supplied corresponds to the 
		// number of options prresent on the user interfac page
		nlassert(page->NumOptions>=0 && page->NumOptions<=4);
		nlassert(page->NumOptions==0 || On[page->NumOptions-1]!=~0u);
		nlassert(page->NumOptions==4 || On[page->NumOptions]==~0u);
	}
};


//////////////////////////////////////////////////////////////////////////////
// the structure for a bot chat automatons & a singleton for indexing
// automatons by name

struct SBotChatAutomaton
{
	// public data ----------------------------------------------------------
	string					Name;
	SBotChatAutomatonState *States;
	uint					Size;

	// ctor -----------------------------------------------------------------
	SBotChatAutomaton(string name, SBotChatAutomatonState *states,uint size)
	{
		Name=name;
		States=states;
		Size=size;

		if (NameMap.find(name)!=NameMap.end())
		{
			nlwarning("SBotChatAutomaton::SBotChatAutomaton(): More than one instance with name: %s",name.c_str());
			return;
		}
		NameMap[name]=this;
	}

	// dtor -----------------------------------------------------------------
	~SBotChatAutomaton()
	{
		map <string,SBotChatAutomaton *>::iterator it=NameMap.find(Name);
		if (it!=NameMap.end() && (*it).second==this)
			NameMap.erase(it);
		// don't try to display a warning in a dtor as the warning system is 
		// probably already down
	}


	// singleton methods ----------------------------------------------------
	static SBotChatAutomaton *getAutomatonByName(string name)
	{
		map <string,SBotChatAutomaton *>::iterator it=NameMap.find(name);
		if (it==NameMap.end())
			return NULL;
		return (*it).second;
	}

	// singleton data -------------------------------------------------------
	static map <string,SBotChatAutomaton *> NameMap;
};
map <string,SBotChatAutomaton *> SBotChatAutomaton::NameMap;


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Implementation of different code modules for handling different bot
// chat page types

//////////////////////////////////////////////////////////////////////////////
// this is a dummy page used to terminate chats

class CBotChatPageTypeDone: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		return false; // stop the bot chat!
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		return false;
	}
}
BotChatPageTypeDone;


//////////////////////////////////////////////////////////////////////////////
// definition for a chat page that contains static text and buttons for 
// player to click on/ select

class CBotChatPageTypeTextOnly: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		return true;
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		return true;
	}
}
BotChatPageTypeTextOnly;


//////////////////////////////////////////////////////////////////////////////
// definition for a chat page that displays NEWS as well as other text

class CBotChatPageTypeNews: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		return true;
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		return true;
	}
}
BotChatPageTypeNews;


//////////////////////////////////////////////////////////////////////////////
// definition for a chat page that displays a SHOP interface

class CBotChatPageTypeShop: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		nlinfo ("player %s entered trade page", player.toString().c_str());
		CMessage msgout( "TRADE_BEGIN" );
		msgout.serial( const_cast<CEntityId &>(player) );
		msgout.serial( const_cast<CEntityId &>(bot) );
		sendMessageViaMirror( "WOS", msgout );
		return true;
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		nlinfo ("end of trade with player %s", player.toString().c_str());
		CMessage msgout( "TRADE_END" );
		msgout.serial( const_cast<CEntityId &>(player) );
		msgout.serial( const_cast<CEntityId &>(bot) );
		sendMessageViaMirror( "WOS", msgout );
		sendMessageViaMirror( "EGS", msgout );
		return true;
	}
}
BotChatPageTypeShop;


//////////////////////////////////////////////////////////////////////////////
// definition for a chat page that displays a MISSION SHOP interface

class CBotChatPageTypeMissionShop: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		nlinfo ("player %s entered mission page", player.toString().c_str());
		CMessage msgout( "MISSION_LIST_BEGIN" );
		msgout.serial( const_cast<CEntityId &>(player) );
		msgout.serial( const_cast<CEntityId &>(bot) );
		sendMessageViaMirror( "WOS", msgout );
		return true;
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		nlinfo ("end of mission page with player %s", player.toString().c_str());
		CMessage msgout( "MISSION_LIST_END" );
		msgout.serial( const_cast<CEntityId &>(player) );
		msgout.serial( const_cast<CEntityId &>(bot) );
		sendMessageViaMirror( "EGS", msgout );
		return true;
	}
}
BotChatPageTypeMissionShop;



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Definitions of bot chat pages and automatons

// define the usable bot chat pages ------------------------------------------
SBotChatPage BotChatPageIntro		(BOTCHATTYPE::Intro,			&BotChatPageTypeTextOnly, 4);
SBotChatPage BotChatPageFriendly	(BOTCHATTYPE::FriendlyMainPage,	&BotChatPageTypeNews, 4);
SBotChatPage BotChatPageNeutral		(BOTCHATTYPE::NeutralMainPage,	&BotChatPageTypeNews, 3);
SBotChatPage BotChatPageHostile		(BOTCHATTYPE::NastyMainPage,	&BotChatPageTypeTextOnly, 1);
SBotChatPage BotChatPageMoreNews	(BOTCHATTYPE::MoreNewsPage,		&BotChatPageTypeNews, 2);
SBotChatPage BotChatPageShop		(BOTCHATTYPE::BuySellPage,		&BotChatPageTypeShop, 2);
SBotChatPage BotChatPageMissionShop	(BOTCHATTYPE::MissionsPage,		&BotChatPageTypeMissionShop, 2);
SBotChatPage BotChatPageDone		(BOTCHATTYPE::Done,				&BotChatPageTypeDone, 0);

// the default automaton -----------------------------------------------------
SBotChatAutomatonState BotChatStatesDefault[]=
{
	SBotChatAutomatonState(&BotChatPageIntro,2,3,4,1),		// 0 - friendly/ neutral/ hostile/ done
	SBotChatAutomatonState(&BotChatPageDone),				// 1
	SBotChatAutomatonState(&BotChatPageFriendly,5,6,7,1),	// 2 - more news/ buy sell/ mission/ done
	SBotChatAutomatonState(&BotChatPageNeutral,6,7,1),		// 3 - buy sell/ mission/ done
	SBotChatAutomatonState(&BotChatPageHostile,1),			// 4 - done
	SBotChatAutomatonState(&BotChatPageMoreNews,2,1),		// 5 - friendly/ done
	SBotChatAutomatonState(&BotChatPageShop,3,1),			// 6 - neutral/ done
	SBotChatAutomatonState(&BotChatPageMissionShop,3,1),	// 7 - neutral/ done
};
SBotChatAutomaton BotChatDefault("default",BotChatStatesDefault,sizeof(BotChatStatesDefault)/sizeof(BotChatStatesDefault[0]));

// the automaton for merchants -----------------------------------------------
SBotChatAutomatonState BotChatStatesMerchant[]=
{
	SBotChatAutomatonState(&BotChatPageMoreNews,2,1),		// 0 - shop/ done
	SBotChatAutomatonState(&BotChatPageDone),				// 1
	SBotChatAutomatonState(&BotChatPageShop,0,1),			// 2 - news/ done
};
SBotChatAutomaton BotChatMerchant("merchant",BotChatStatesMerchant,sizeof(BotChatStatesMerchant)/sizeof(BotChatStatesMerchant[0]));

// the automaton for walkers and talkers -------------------------------------
SBotChatAutomatonState BotChatStatesWalkerTalker[]=
{
	SBotChatAutomatonState(&BotChatPageHostile,1),	// 0
	SBotChatAutomatonState(&BotChatPageDone)		// 1
};
SBotChatAutomaton BotChatWalkerTalker("walker talker",BotChatStatesWalkerTalker,sizeof(BotChatStatesMerchant)/sizeof(BotChatStatesMerchant[0]));


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Represetnation of a conversation between a player and a bot
// includes conversation automaton state data

struct CBotChat
{
	CBotChat () : Player(CEntityId::Unknown), Bot(CEntityId::Unknown) { }
	CBotChat (CEntityId player, CEntityId bot, SBotChatAutomaton *automaton)
	{
		Player=player;
		Bot=bot;
		setAutomaton(automaton);
	}

	void setState(uint32 state)
	{
		if (state>=Automaton->Size && state!=~0u)
		{
			nlwarning("CBotChatEntry()::setState: Invalid state: %d",state);
			return;
		}

		// if there is already a page open close it
		if (CurrentState<Automaton->Size)
			Automaton->States[CurrentState].Page->PageType->close(Player,Bot);

		// open the new page
		CurrentState=state;
		if (state==~0u)
			Done=true;
		else
			Done=!Automaton->States[CurrentState].Page->PageType->open(Player,Bot);

		// transmit the new page id to the client
		uint32 happyness=10; 
		BOTCHATTYPE::TBotChatInterfaceId interfaceId=Done?BOTCHATTYPE::Done:Automaton->States[CurrentState].Page->ClientInterfaceId;
		NEWSTYPE::TNewsType newsType=NEWSTYPE::Unknown;

		CMessage msgout("BOT_CHAT_SELECT_INTERFACE");
		msgout.serial (Player);
		msgout.serial (happyness);
		msgout.serialEnum (interfaceId);
		msgout.serialEnum (newsType);
		sendMessageViaMirror("IOS", msgout);
	}

	void setAutomaton(SBotChatAutomaton *automaton)
	{
		Automaton=automaton;
		CurrentState=~0u;	// set this to a ~0 so that setState doesn't try to clse existing page
		setState(0);
	}

	void selectEntry (sint8 userInput)
	{
		// select the new page
		if ((unsigned)userInput >= Automaton->States[CurrentState].Page->NumOptions)
		{
			nlwarning ("CBotChatEntry::selectEntry: For player %s: input out of bounds: %d", Player.toString().c_str(), userInput);
			return;
		}

		// advance through the state table
		setState(Automaton->States[CurrentState].On[userInput]);
	}

	void endChat ()
	{
		setState(~0u);
	}

	CEntityId Player;
	CEntityId Bot;
	SBotChatAutomaton *Automaton;
	uint32 CurrentState;
	bool Done;
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Singleton manager class central to the bot chat system

class CBotChatManager
{
public:

	static void newChat(CEntityId player, CEntityId bot)
	{
		// make sure the player isn't already chatting
		map<CEntityId, CBotChat>::iterator it = BotChatMap.find (player);
		if (it != BotChatMap.end())
			return;

		// a bit of logging
		nlinfo ("new chat between player %s and bot %s", player.toString().c_str(), bot.toString().c_str());

		// call the CbBegin() callback to get the name of the automaton to use
		string automatonName;
		if (CbBegin!=NULL)
			automatonName=CbBegin(player,bot);
		else
			automatonName="default";
		SBotChatAutomaton *automaton=SBotChatAutomaton::getAutomatonByName(automatonName);
		if (automaton==NULL)
		{
			nlwarning("- ignoring bot chat request as automaton '%s' not found",automatonName.c_str());
			return;
		}

		// setup the new chat
		BotChatMap[player] = CBotChat(player, bot, automaton);
	}

	static void endChat(CEntityId player)
	{
		CEntityId bot; // for use in callback ... at end of routine

		map<CEntityId, CBotChat>::iterator it = BotChatMap.find (player);
		if (it != BotChatMap.end())
		{
			bot=(*it).second.Bot;

			nlinfo ("end of bot chat between player %s and bot %s", player.toString().c_str(),bot.toString().c_str());

			// if the chat is still active then stop it
			if ((*it).second.Done)
				(*it).second.endChat();

			// remove the map entry
			BotChatMap.erase (it);

			// **** this code may be dodgy 'cos its in a dtor
			// **** if it is dodgy then	we need to migrate from an STL map
			// **** to some kind of custom structure that we can guarantee OK
		}

		if (CbEnd!=NULL)
			CbEnd(player,bot);
	}

	static void treatInput(CEntityId player, sint8 userInput)
	{
		// locate the bot chat for the given player
		map<CEntityId, CBotChat>::iterator it = BotChatMap.find (player);
		if (it == BotChatMap.end())
		{
			nlwarning ("No bot chat with the player %s", player.toString().c_str());
			return;
		}

		// pass the player input to the bot chat handler
		(*it).second.selectEntry(userInput);

		// check whether the bot chat is finished
		if ((*it).second.Done)
			endChat(player);
	}


	// static data for the singleton -----------------------------------------
	static CBotChatInterface::TCallbackBegin CbBegin;
	static CBotChatInterface::TCallbackEnd CbEnd;
	static map<CEntityId, CBotChat> BotChatMap;
};
CBotChatInterface::TCallbackBegin CBotChatManager::CbBegin;
CBotChatInterface::TCallbackEnd CBotChatManager::CbEnd;
map<CEntityId, CBotChat> CBotChatManager::BotChatMap;


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// message callbacks and callback table

static void cbBotChatStart (CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	CEntityId player, bot;
	
	msgin.serial( player );
	msgin.serial( bot );

	CBotChatManager::newChat (player, bot);
}

static void cbBotChatSelectAnswer (CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	CEntityId player, bot;
	sint8 answer;

	msgin.serial( player );
	msgin.serial( answer );

	CBotChatManager::treatInput (player, answer);
}


static TUnifiedCallbackItem CbArray[]=
{
	{ "BOT_CHAT_START", cbBotChatStart },
	{ "BOT_CHAT_SELECT_ANSWER", cbBotChatSelectAnswer },
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Interface class providing API for bot chat system

void CBotChatInterface::init(CBotChatInterface::TCallbackBegin cbBegin,CBotChatInterface::TCallbackEnd cbEnd)
{
	CBotChatManager::CbBegin=cbBegin;
	CBotChatManager::CbEnd=cbEnd;
	CUnifiedNetwork::getInstance()->addCallbackArray(CbArray, sizeof (CbArray) / sizeof (CbArray[0]));
}

void CBotChatInterface::release()
{
}

void CBotChatInterface::getBotChatPartners(CEntityId bot,vector<CEntityId> &result)
{
	map<CEntityId, CBotChat>::iterator it;
	for (it=CBotChatManager::BotChatMap.begin();it!=CBotChatManager::BotChatMap.end();++it)
		if ((*it).second.Bot==bot)
			result.push_back((*it).first);
}

void CBotChatInterface::endChatForPlayer(CEntityId player)
{
	CBotChatManager::endChat(player);
}

void CBotChatInterface::endAllChatForBot(CEntityId bot)
{
	map<CEntityId, CBotChat>::iterator it=CBotChatManager::BotChatMap.begin();
	while (it!=CBotChatManager::BotChatMap.end())
	{
		map<CEntityId, CBotChat>::iterator next=it;
		if ((*it).second.Bot==bot)
			CBotChatManager::endChat((*it).first);
		it=next;
	}
}











































//////////////////////
/// The following is probably out of date but I'm copying it here just in case...



#if 0




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// the parent class for bot chat page type classes

class CBotChatPageType
{
public:
	// virtual interface ----------------------------------------------------
	virtual bool open(const CEntityId &player, const CEntityId &bot)=0;
	virtual bool close(const CEntityId &player, const CEntityId &bot)=0;
};


//////////////////////////////////////////////////////////////////////////////
// the structure for bot chat pages

struct SBotChatPage
{
	// ctor -----------------------------------------------------------------
	SBotChatPage(
		BOTCHATTYPE::TBotChatInterfaceId	clientInterfaceId,
		CBotChatPageType *					chatPageType,
		uint								numOptions	
		)
	{
		ClientInterfaceId=	clientInterfaceId;
		PageType=			chatPageType;
		NumOptions=			numOptions;	
	}

	// data -----------------------------------------------------------------
	BOTCHATTYPE::TBotChatInterfaceId	ClientInterfaceId;	// id of interface to display on client
	CBotChatPageType *					PageType;			// type of chat page
	uint								NumOptions;			// number of options for player to click on
};


//////////////////////////////////////////////////////////////////////////////
// the structure for a state for bot chat automatons

struct SBotChatAutomatonState
{
	// public data ----------------------------------------------------------
	SBotChatPage *Page;
	uint On[5];				// value to return on player click of slot 0..4

	// ctor -----------------------------------------------------------------
	SBotChatAutomatonState(SBotChatPage	*page,uint on0=~0u,uint on1=~0u,uint on2=~0u,uint on3=~0u,uint on4=~0u)
	{
		Page=page;
		On[0]=on0;
		On[1]=on1;
		On[2]=on2;
		On[3]=on3;
		On[4]=on4;

		// make sure the number of arguments supplied corresponds to the 
		// number of options prresent on the user interfac page
		nlassert(page->NumOptions>=0 && page->NumOptions<=4);
		nlassert(page->NumOptions==0 || On[page->NumOptions-1]!=~0u);
		nlassert(page->NumOptions==4 || On[page->NumOptions]==~0u);
	}
};


//////////////////////////////////////////////////////////////////////////////
// the structure for a bot chat automatons & a singleton for indexing
// automatons by name

struct SBotChatAutomaton
{
	// public data ----------------------------------------------------------
	string					Name;
	SBotChatAutomatonState *States;
	uint					Size;

	// ctor -----------------------------------------------------------------
	SBotChatAutomaton(string name, SBotChatAutomatonState *states,uint size)
	{
		Name=name;
		States=states;
		Size=size;

		if (NameMap.find(name)!=NameMap.end())
		{
			nlwarning("SBotChatAutomaton::SBotChatAutomaton(): More than one instance with name: %s",name.c_str());
			return;
		}
		NameMap[name]=this;
	}

	// dtor -----------------------------------------------------------------
	~SBotChatAutomaton()
	{
		map <string,SBotChatAutomaton *>::iterator it=NameMap.find(Name);
		if (it!=NameMap.end() && (*it).second==this)
			NameMap.erase(it);
		// don't try to display a warning in a dtor as the warning system is 
		// probably already down
	}


	// singleton methods ----------------------------------------------------
	static SBotChatAutomaton *getAutomatonByName(string name)
	{
		map <string,SBotChatAutomaton *>::iterator it=NameMap.find(name);
		if (it==NameMap.end())
			return NULL;
		return (*it).second;
	}

	// singleton data -------------------------------------------------------
	static map <string,SBotChatAutomaton *> NameMap;
};
map <string,SBotChatAutomaton *> SBotChatAutomaton::NameMap;


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Implementation of different code modules for handling different bot
// chat page types

//////////////////////////////////////////////////////////////////////////////
// this is a dummy page used to terminate chats

class CBotChatPageTypeDone: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		return false; // stop the bot chat!
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		return false;
	}
}
BotChatPageTypeDone;


//////////////////////////////////////////////////////////////////////////////
// definition for a chat page that contains static text and buttons for 
// player to click on/ select

class CBotChatPageTypeTextOnly: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		return true;
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		return true;
	}
}
BotChatPageTypeTextOnly;


//////////////////////////////////////////////////////////////////////////////
// definition for a chat page that displays NEWS as well as other text

class CBotChatPageTypeNews: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		return true;
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		return true;
	}
}
BotChatPageTypeNews;


//////////////////////////////////////////////////////////////////////////////
// definition for a chat page that displays a SHOP interface

class CBotChatPageTypeShop: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		nlinfo ("player %s entered trade page", player.toString().c_str());
		CMessage msgout( "TRADE_BEGIN" );
		msgout.serial( const_cast<CEntityId &>(player) );
		msgout.serial( const_cast<CEntityId &>(bot) );
		sendMessageViaMirror( "WOS", msgout );
		return true;
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		nlinfo ("end of trade with player %s", player.toString().c_str());
		CMessage msgout( "TRADE_END" );
		msgout.serial( const_cast<CEntityId &>(player) );
		msgout.serial( const_cast<CEntityId &>(bot) );
		sendMessageViaMirror( "WOS", msgout );
		sendMessageViaMirror( "EGS", msgout );
		return true;
	}
}
BotChatPageTypeShop;


//////////////////////////////////////////////////////////////////////////////
// definition for a chat page that displays a MISSION SHOP interface

class CBotChatPageTypeMissionShop: public CBotChatPageType
{
public:
	virtual bool open(const CEntityId &player, const CEntityId &bot)
	{
		nlinfo ("player %s entered mission page", player.toString().c_str());
		CMessage msgout( "MISSION_LIST_BEGIN" );
		msgout.serial( const_cast<CEntityId &>(player) );
		msgout.serial( const_cast<CEntityId &>(bot) );
		sendMessageViaMirror( "WOS", msgout );
		return true;
	}

	virtual bool close(const CEntityId &player, const CEntityId &bot)
	{
		nlinfo ("end of mission page with player %s", player.toString().c_str());
		CMessage msgout( "MISSION_LIST_END" );
		msgout.serial( const_cast<CEntityId &>(player) );
		msgout.serial( const_cast<CEntityId &>(bot) );
		sendMessageViaMirror( "EGS", msgout );
		return true;
	}
}
BotChatPageTypeMissionShop;



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Definitions of bot chat pages and automatons

// define the usable bot chat pages ------------------------------------------
SBotChatPage BotChatPageIntro		(BOTCHATTYPE::Intro,			&BotChatPageTypeTextOnly, 4);
SBotChatPage BotChatPageFriendly	(BOTCHATTYPE::FriendlyMainPage,	&BotChatPageTypeNews, 4);
SBotChatPage BotChatPageNeutral		(BOTCHATTYPE::NeutralMainPage,	&BotChatPageTypeNews, 3);
SBotChatPage BotChatPageHostile		(BOTCHATTYPE::NastyMainPage,	&BotChatPageTypeTextOnly, 1);
SBotChatPage BotChatPageMoreNews	(BOTCHATTYPE::MoreNewsPage,		&BotChatPageTypeNews, 2);
SBotChatPage BotChatPageShop		(BOTCHATTYPE::BuySellPage,		&BotChatPageTypeShop, 2);
SBotChatPage BotChatPageMissionShop	(BOTCHATTYPE::MissionsPage,		&BotChatPageTypeMissionShop, 2);
SBotChatPage BotChatPageDone		(BOTCHATTYPE::Done,				&BotChatPageTypeDone, 0);

// the default automaton -----------------------------------------------------
SBotChatAutomatonState BotChatStatesDefault[]=
{
	SBotChatAutomatonState(&BotChatPageIntro,2,3,4,1),		// 0 - friendly/ neutral/ hostile/ done
	SBotChatAutomatonState(&BotChatPageDone),				// 1
	SBotChatAutomatonState(&BotChatPageFriendly,5,6,7,1),	// 2 - more news/ buy sell/ mission/ done
	SBotChatAutomatonState(&BotChatPageNeutral,6,7,1),		// 3 - buy sell/ mission/ done
	SBotChatAutomatonState(&BotChatPageHostile,1),			// 4 - done
	SBotChatAutomatonState(&BotChatPageMoreNews,2,1),		// 5 - friendly/ done
	SBotChatAutomatonState(&BotChatPageShop,3,1),			// 6 - neutral/ done
	SBotChatAutomatonState(&BotChatPageMissionShop,3,1),	// 7 - neutral/ done
};
SBotChatAutomaton BotChatDefault("default",BotChatStatesDefault,sizeof(BotChatStatesDefault)/sizeof(BotChatStatesDefault[0]));

// the automaton for merchants -----------------------------------------------
SBotChatAutomatonState BotChatStatesMerchant[]=
{
	SBotChatAutomatonState(&BotChatPageMoreNews,2,1),		// 0 - shop/ done
	SBotChatAutomatonState(&BotChatPageDone),				// 1
	SBotChatAutomatonState(&BotChatPageShop,0,1),			// 2 - news/ done
};
SBotChatAutomaton BotChatMerchant("merchant",BotChatStatesMerchant,sizeof(BotChatStatesMerchant)/sizeof(BotChatStatesMerchant[0]));

// the automaton for walkers and talkers -------------------------------------
SBotChatAutomatonState BotChatStatesWalkerTalker[]=
{
	SBotChatAutomatonState(&BotChatPageHostile,1),	// 0
	SBotChatAutomatonState(&BotChatPageDone)		// 1
};
SBotChatAutomaton BotChatWalkerTalker("walker talker",BotChatStatesWalkerTalker,sizeof(BotChatStatesMerchant)/sizeof(BotChatStatesMerchant[0]));


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Represetnation of a conversation between a player and a bot
// includes conversation automaton state data

struct CBotChat
{
	CBotChat () : Player(CEntityId::Unknown), Bot(CEntityId::Unknown) { }
	CBotChat (CEntityId player, CEntityId bot, SBotChatAutomaton *automaton)
	{
		Player=player;
		Bot=bot;
		setAutomaton(automaton);
	}

	void setState(uint32 state)
	{
		if (state>=Automaton->Size && state!=~0u)
		{
			nlwarning("CBotChatEntry()::setState: Invalid state: %d",state);
			return;
		}

		// if there is already a page open close it
		if (CurrentState<Automaton->Size)
			Automaton->States[CurrentState].Page->PageType->close(Player,Bot);

		// open the new page
		CurrentState=state;
		if (state==~0u)
			Done=true;
		else
			Done=!Automaton->States[CurrentState].Page->PageType->open(Player,Bot);

		// transmit the new page id to the client
		uint32 happyness=10; 
		BOTCHATTYPE::TBotChatInterfaceId interfaceId=Done?BOTCHATTYPE::Done:Automaton->States[CurrentState].Page->ClientInterfaceId;
		NEWSTYPE::TNewsType newsType=NEWSTYPE::Unknown;

		CMessage msgout("BOT_CHAT_SELECT_INTERFACE");
		msgout.serial (Player);
		msgout.serial (happyness);
		msgout.serialEnum (interfaceId);
		msgout.serialEnum (newsType);
		sendMessageViaMirror("IOS", msgout);
	}

	void setAutomaton(SBotChatAutomaton *automaton)
	{
		Automaton=automaton;
		CurrentState=~0u;	// set this to a ~0 so that setState doesn't try to clse existing page
		setState(0);
	}

	void selectEntry (sint8 userInput)
	{
		// select the new page
		if ((unsigned)userInput >= Automaton->States[CurrentState].Page->NumOptions)
		{
			nlwarning ("CBotChatEntry::selectEntry: For player %s: input out of bounds: %d", Player.toString().c_str(), userInput);
			return;
		}

		// advance through the state table
		setState(Automaton->States[CurrentState].On[userInput]);
	}

	void endChat ()
	{
		setState(~0u);
	}

	CEntityId Player;
	CEntityId Bot;
	SBotChatAutomaton *Automaton;
	uint32 CurrentState;
	bool Done;
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Singleton manager class central to the bot chat system

class CBotChatManager
{
public:

	static void newChat(CEntityId player, CEntityId bot)
	{
		// make sure the player isn't already chatting
		map<CEntityId, CBotChat>::iterator it = BotChatMap.find (player);
		if (it != BotChatMap.end())
			return;

		// a bit of logging
		nlinfo ("new chat between player %s and bot %s", player.toString().c_str(), bot.toString().c_str());

		// call the CbBegin() callback to get the name of the automaton to use
		string automatonName;
		if (CbBegin!=NULL)
			automatonName=CbBegin(player,bot);
		else
			automatonName="default";
		SBotChatAutomaton *automaton=SBotChatAutomaton::getAutomatonByName(automatonName);
		if (automaton==NULL)
		{
			nlwarning("- ignoring bot chat request as automaton '%s' not found",automatonName.c_str());
			return;
		}

		// setup the new chat
		BotChatMap[player] = CBotChat(player, bot, automaton);
	}

	static void endChat(CEntityId player)
	{
		CEntityId bot; // for use in callback ... at end of routine

		map<CEntityId, CBotChat>::iterator it = BotChatMap.find (player);
		if (it != BotChatMap.end())
		{
			bot=(*it).second.Bot;

			nlinfo ("end of bot chat between player %s and bot %s", player.toString().c_str(),bot.toString().c_str());

			// if the chat is still active then stop it
			if ((*it).second.Done)
				(*it).second.endChat();

			// remove the map entry
			BotChatMap.erase (it);

			// **** this code may be dodgy 'cos its in a dtor
			// **** if it is dodgy then	we need to migrate from an STL map
			// **** to some kind of custom structure that we can guarantee OK
		}

		if (CbEnd!=NULL)
			CbEnd(player,bot);
	}

	static void treatInput(CEntityId player, sint8 userInput)
	{
		// locate the bot chat for the given player
		map<CEntityId, CBotChat>::iterator it = BotChatMap.find (player);
		if (it == BotChatMap.end())
		{
			nlwarning ("No bot chat with the player %s", player.toString().c_str());
			return;
		}

		// pass the player input to the bot chat handler
		(*it).second.selectEntry(userInput);

		// check whether the bot chat is finished
		if ((*it).second.Done)
			endChat(player);
	}


	// static data for the singleton -----------------------------------------
	static CBotChatInterface::TCallbackBegin CbBegin;
	static CBotChatInterface::TCallbackEnd CbEnd;
	static map<CEntityId, CBotChat> BotChatMap;
};
CBotChatInterface::TCallbackBegin CBotChatManager::CbBegin;
CBotChatInterface::TCallbackEnd CBotChatManager::CbEnd;
map<CEntityId, CBotChat> CBotChatManager::BotChatMap;


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// message callbacks and callback table

static void cbBotChatStart (CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	CEntityId player, bot;
	
	msgin.serial( player );
	msgin.serial( bot );

	CBotChatManager::newChat (player, bot);
}

static void cbBotChatSelectAnswer (CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	CEntityId player, bot;
	sint8 answer;

	msgin.serial( player );
	msgin.serial( answer );

	CBotChatManager::treatInput (player, answer);
}


static TUnifiedCallbackItem CbArray[]=
{
	{ "BOT_CHAT_START", cbBotChatStart },
	{ "BOT_CHAT_SELECT_ANSWER", cbBotChatSelectAnswer },
};


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Interface class providing API for bot chat system

void CBotChatInterface::init(CBotChatInterface::TCallbackBegin cbBegin,CBotChatInterface::TCallbackEnd cbEnd)
{
	CBotChatManager::CbBegin=cbBegin;
	CBotChatManager::CbEnd=cbEnd;
	CUnifiedNetwork::getInstance()->addCallbackArray(CbArray, sizeof (CbArray) / sizeof (CbArray[0]));
}

void CBotChatInterface::release()
{
}

void CBotChatInterface::getBotChatPartners(CEntityId bot,vector<CEntityId> &result)
{
	map<CEntityId, CBotChat>::iterator it;
	for (it=CBotChatManager::BotChatMap.begin();it!=CBotChatManager::BotChatMap.end();++it)
		if ((*it).second.Bot==bot)
			result.push_back((*it).first);
}

void CBotChatInterface::endChatForPlayer(CEntityId player)
{
	CBotChatManager::endChat(player);
}

void CBotChatInterface::endAllChatForBot(CEntityId bot)
{
	map<CEntityId, CBotChat>::iterator it=CBotChatManager::BotChatMap.begin();
	while (it!=CBotChatManager::BotChatMap.end())
	{
		map<CEntityId, CBotChat>::iterator next=it;
		if ((*it).second.Bot==bot)
			CBotChatManager::endChat((*it).first);
		it=next;
	}
}

#endif

#endif
