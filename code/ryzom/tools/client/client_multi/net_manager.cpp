/** \file net_manager.cpp
 * Manage the network connection on the client.
 *
 * $Id: net_manager.cpp,v 1.6 2007/05/22 13:42:53 boucher Exp $
 */




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Misc
#include <nel/misc/command.h>
#include <nel/misc/progress_callback.h>
// Client.
#include "group_list.h"
#include "interface_manager.h"
#include "net_manager.h"
#include "client_cfg.h"
#include "entities.h"
#include "client_chat_manager.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "user_controls.h"
#include "bot_chat_ui.h"
#include "string_manager_client.h"
#include "interface_v3/people_interraction.h"
#include "interface_v3/bot_chat.h"
#include "view_text_id.h"
#include "input_handler_manager.h"
// Game Share
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/bot_chat_types.h"
#include "game_share/news_types.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/chat_group.h"
#include "game_share/character_summary.h"
// Client sheets
#include "client_sheets/bot_chat_sheet.h"
// Std.
#include <vector>


#define OLD_STRING_SYSTEM

///////////
// USING //
///////////
using namespace NLMISC;
using namespace std;


////////////
// GLOBAL //
////////////
CGenericXmlMsgHeaderManager	GenericMsgHeaderMngr;	// Manage messages
CNetManagerMulti			NetMngr;				// Manage the connection.

bool serverReceivedReady = false;

// Hierachical timer
H_AUTO_DECL ( RZ_Client_Net_Mngr_Update )

////////////
// EXTERN //
////////////
extern bool								noUserChar;	// \todo GUIGUI : do this better.
extern bool								userChar;	// \todo GUIGUI : do this better.
extern std::vector<CCharacterSummary>	CharacterSummaries;
extern bool								enter;		// \todo GUIGUI : do this better.
extern CGenericXmlMsgHeaderManager		GenericMsgHeaderMngr;
extern CClientChatManager				ChatMngr;

extern bool CharNameValidArrived;
extern bool CharNameValid;

///////////////
// FUNCTIONS //
///////////////
void impulseDatabase(NLMISC::CBitMemStream &impulse)
{
	nldebug("impulseCallBack : msg for the database.");
	try
	{
		IngameDbMngr.readDelta(impulse);
	}
	catch (Exception &e)
	{
		nlwarning ("Problem during decoding a DATABASE message, skip it: %s", e.what());
	}
}

void impulseNoUserChar(NLMISC::CBitMemStream &impulse)
{
	// received USER_CHAR
	nldebug("impulseCallBack : Received NO_USER_CHAR");

	CharacterSummaries.clear();
	noUserChar = true;
}

void impulseUserChars(NLMISC::CBitMemStream &impulse)
{
	// received USER_CHARS
	nldebug("impulseCallBack : Received USER_CHARS");

	// read characters summary	
	CharacterSummaries.clear();
	impulse.serialCont (CharacterSummaries);
	
	userChar = true;

	// Create the message for the server to select the first character.
/*	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:SELECT_CHAR", out))
	{
		CSelectCharMsg	SelectCharMsg;
		SelectCharMsg.c = 0;	//TODO set here the character choosen by player
		out.serial( SelectCharMsg );
		NetMngr.push(out);
		NetMngr.send(NetMngr.getCurrentServerTick());
		// send CONNECTION:USER_CHARS
		nldebug("impulseCallBack : CONNECTION:SELECT_CHAR sent");
	}
	else
		nlwarning("impulseCallBack : unknown message name : 'CONNECTION:SELECT_CHAR'.");

	noUserChar = true;
	*/
}

void impulseUserChar(NLMISC::CBitMemStream &impulse)
{
	// received USER_CHAR
	nldebug("impulseCallBack : Received USER_CHAR");

	// Serialize the message.
	CUserCharMsg userCharMsg;
	impulse.serial(userCharMsg);

	UserEntity.pos(CVectorD((float)userCharMsg.X/1000.0f, (float)userCharMsg.Y/1000.0f, (float)userCharMsg.Z/1000.0f));
	UserEntity.front(CVector((float)cos(userCharMsg.Heading), (float)sin(userCharMsg.Heading), 0.f));
	UserEntity.dir(UserEntity.front());
	UserEntity.head(UserEntity.front());

	nldebug("<impulseUserChar> pos : %f %f %f  heading : %f",UserEntity.pos().x,UserEntity.pos().y,UserEntity.pos().z,userCharMsg.Heading);

	// Update the position for the vision.
	NetMngr.setReferencePosition(UserEntity.pos());

	enter = true;
}

void impulseCharNameValid(NLMISC::CBitMemStream &impulse)
{
	nldebug("impulseCallBack : Received VALID_NAME");
	uint8 nTmp;
	impulse.serial(nTmp);
	CharNameValid = ((nTmp != 0) ? true : false);
	CharNameValidArrived = true;
}

void impulseServerReady(NLMISC::CBitMemStream &impulse)
{
	// received USER_CHAR
	nldebug("impulseCallBack : Received READY");
	
	serverReceivedReady = true;
}


void impulseChat(NLMISC::CBitMemStream &impulse)
{
	ucstring ucstr;
	CChatGroup::TGroupType mode;
	if( ChatMngr.getChatString(impulse,ucstr,mode) )
	{
		CInterfaceProperty prop;
		string entry="UI:VARIABLES:CHAT:COLORS:";
			
		switch(mode)
		{
		case CChatGroup::say:
			entry+="SAY";
			break;
		case CChatGroup::shout:
			entry+="SHOUT";
			break;
		case CChatGroup::group:
			entry+="GROUP"; //#error faire une sortie dans la fenetre de chat du groupe
			break;
		case CChatGroup::clade:
			entry+="CLADE";
			break;
		case CChatGroup::civilization:
			entry+="CIVILIZATION";
			break;
		case CChatGroup::territory:
			entry+="TERRITORY";
			break;
		case CChatGroup::universe:
			entry+="UNIVERSE";
			break;
		default:
			nlwarning("unknown group type");
			return;
		}
		prop.readRGBA(entry.c_str()," ");

		// TEMP TEMP TEMP display result in the 'around me' window
		// TODO GAMEDEV : display msg in the right window ('around me' or 'party chat' or 'team chat' or 'guild chat')
		if (mode == CChatGroup::group)
		{
			if (PeopleInterraction.TeamList.getChat())
			{
				PeopleInterraction.TeamList.getChat()->displayMessage(ucstr, prop.getRGBA(), 2);
			}
		}
		else
		{		
			if (PeopleInterraction.AroundMe)
			{
				PeopleInterraction.AroundMe->displayMessage(ucstr, prop.getRGBA(), 2);
			}
		}

		// received CHAT
		nldebug("<impulseChat> Received CHAT : %s",ucstr.toString().c_str());
	}
	else
	{
		nldebug("<impulseChat> Received CHAT, put in buffer : waiting association");
	}
}


void impulseTell(NLMISC::CBitMemStream &impulse)
{
	ucstring ucstr;
	if( ChatMngr.getChatString(impulse,ucstr) )
	{		
		
		if (PeopleInterraction.AroundMe)
		{
			static CCDBNodeLeaf *tellColor = NULL;
			if (!tellColor)
			{
				tellColor = CInterfaceManager::getInstance()->getDbProp("UI:VARIABLES:CHAT:COLORS:TELL");
			}
			CRGBA col;
			col.setPacked(tellColor->getValue32());
			PeopleInterraction.AroundMe->displayMessage(ucstr, col, 2);
		}		

		// received TELL
		nldebug("<impulseTell> Received TELL : %s",ucstr.toString().c_str());
	}
	else
	{
		nldebug("<impulseTell> Received TELL, put in buffer : waiting association");
	}
}

//void impulseAddDynStr(NLMISC::CBitMemStream &impulse)
//{
//	bool huff = false;
//	impulse.serialBit(huff);
//
//	uint32 index;
//	ucstring ucstr;
//
//	impulse.serial( index );
//	impulse.serial( ucstr );
//
//	vector<bool> code;
//	if( huff )
//	{
//		impulse.serialCont( code );
//	}
//	#ifdef OLD_STRING_SYSTEM
//		ChatMngr.getDynamicDB().add( index, ucstr, code );
//	#else
//		nlwarning( "// TRAP // WE MUST NEVER CALL THIS IMPULE ANYMORE : ALL IS HANDLED BY STRING_MANAGER NOW !!!" );	
//	#endif
//	
//	// received ADD_DYN_STR
//	nldebug("impulseCallBack : Received ADD_DYN_STR : adding %s at index %d",ucstr.toString().c_str(),index);
//}
/*
string getInterfaceNameFromId (sint botType, sint interfaceId)
{
	string interfaceName = "ui:interface:bot_chat_";

	switch (botType)
	{
	case 0: interfaceName += "figurant_"; break;
	case 1: interfaceName += "figurant_presse_"; break;
	case 2: interfaceName += "chef_village_"; break;
	default: interfaceName += "figurant_"; break;
	}
	
	switch (interfaceId)
	{
	case BOTCHATTYPE::Intro: interfaceName += "intro"; break;
	case BOTCHATTYPE::FriendlyMainPage: interfaceName += "friendly_main"; break;
	case BOTCHATTYPE::NeutralMainPage: interfaceName += "neutral_main"; break;
	case BOTCHATTYPE::NastyMainPage: interfaceName += "nasty_main"; break;
	case BOTCHATTYPE::MoreNewsPage: interfaceName += "more_news"; break;
	case BOTCHATTYPE::Done: nlinfo ("end of bot chat"); interfaceName = ""; break;
	}
	return interfaceName;
}

static char *shortNews[] = {
	"The wind is sour and brings only bad tidings...", "Kitins have been sighted near the village!", "",
	"The tribe of the Black Circle has recently", "increased its activities in our region.", "",
	"The Black Circle has made an incursion", "into our territory!", "",
	"The Black Circle has been sighted near one", "of our forward posts, deep in dangerous territory.", "",
	"The tide has washed up evil news, friend.", "The Black Circle is active in our region.", "",
	"Our people suffer from a debilitating shortage.", "We are in sore need of KamiBast.", "",
	"The economy is slow and our reserve of", "Live Seed low.", "",
	"We are in sore need of Live Seed", "If there is a Goo epidemic, we shall all perish!", "",
	"Our master mages have gotten wind of", "the growing Kami discontentment", "",
};

static char *longNews[] = {
	"These powerful predators haven't come this near", "to the village since their devastating attack", "over 15 seasons ago!",
	"They are after more KamiBast", "for their occult practices.", "",
	"They have captured", "2 of our fortifications in the bush!", "",
	"They have taken over one of our richest sources", "of KamiBast, and are exploiting it", "for their own occult purposes.",
	"They now hold an important source", "of Live Seed hostage,", "close to one of our forward posts.",
	"We use the magical properties of KamiBast and", "its unusually rich fibers for all our crafts.", "",
	"If we don’t harvest new Seed soon,", "we will have no way of purchasing goods", "and resources, beyond what we produce ourselves",
	"We use the rich Sap of Live Seed to produce", "an antidote that counters the disastrous", "effects of the Goo on all Atysian life forms.",
	"The Kamis are shaken by the Black Circle's", "presence. If the Circle continues it’s occult", "practices, we will all suffer the Kamic anger.",
};
*/
/*
void setFakeNews ()
{
	char *table[] = { "figurant", "chef_village", "garde", "commercant" };

	sint rnd = rand ()%(sizeof(shortNews)/sizeof(shortNews[0])/3);
	rnd;

	for (uint i = 0; i < sizeof(table)/sizeof(table[0]); i++)
	{
		{ // set test for the friendly main
			string iname;
			iname = "ui:interface:bot_chat_";
			iname += table[i];
			iname += "_friendly_main";

			CInterfaceGroup *inter = CInterfaceManager::getInstance()->getWindowFromId(iname);
			if (inter == NULL)
			{
				nlwarning ("cant find interface 's%'", iname.c_str());
				continue;
			}

			CViewText *inter2 = (CViewText *)inter->getView("title0");
			nlassert (inter2 != NULL);
			inter2->setText(ucstring(shortNews[rnd*3]));

			CViewText *inter3 = (CViewText *)inter->getView("title1");
			nlassert (inter3 != NULL);
			inter3->setText(ucstring(shortNews[rnd*3+1]));

			CViewText *inter4 = (CViewText *)inter->getView("title2");
			nlassert (inter4 != NULL);
			inter4->setText(ucstring(shortNews[rnd*3+2]));
		}
		{ // set test for the neutral main
			string iname;
			iname = "ui:interface:bot_chat_";
			iname += table[i];
			iname += "_neutral_main";

			CInterfaceGroup *inter = CInterfaceManager::getInstance()->getWindowFromId(iname);
			if (inter == NULL)
			{
				nlwarning ("cant find interface 's%'", iname.c_str());
				continue;
			}

			CViewText *inter2 = (CViewText *)inter->getView("title0");
			nlassert (inter2 != NULL);
			inter2->setText(ucstring(shortNews[rnd*3]));

			CViewText *inter3 = (CViewText *)inter->getView("title1");
			nlassert (inter3 != NULL);
			inter3->setText(ucstring(shortNews[rnd*3+1]));
		}
		{ // set test for the more news
			string iname;
			iname = "ui:interface:bot_chat_";
			iname += table[i];
			iname += "_more_news";

			CInterfaceGroup *inter = CInterfaceManager::getInstance()->getWindowFromId(iname);
			if (inter == NULL)
			{
				nlwarning ("cant find interface 's%'", iname.c_str());
				continue;
			}

			CViewText *inter2 = (CViewText *)inter->getView("title0");
			nlassert (inter2 != NULL);
			inter2->setText(ucstring(longNews[rnd*3]));

			CViewText *inter3 = (CViewText *)inter->getView("title1");
			nlassert (inter3 != NULL);
			inter3->setText(ucstring(longNews[rnd*3+1]));

			CViewText *inter4 = (CViewText *)inter->getView("title2");
			nlassert (inter4 != NULL);
			inter4->setText(ucstring(longNews[rnd*3+2]));
		}
	}
}
*/




	
//=========================================
/** Temp setup for choice list
  */
/*
static void setupBotChatChoiceList(CInterfaceGroup *botChatGroup)
{
	// Temp for test. Should then be read from server msg
	std::vector<ucstring> choices;	
	for(uint k = 0; k < 90; ++k)
	{	
		choices.push_back("Choice " + toString(k));
	}
	CBotChat::setChoiceList(botChatGroup, choices, false);
}
*/

//=========================================
/** Temp setup for description list
  */
/*
static void setupBotChatDescription(CInterfaceGroup *botChatGroup)
{
	ucstring desc;
	for(uint k = 0; k < 90; ++k)
	{	
		desc += "This is a multi line description. ";
	}
	CBotChat::setDescription(botChatGroup, desc);
}
*/

//=========================================
/** Temp setup for bot chat gift
  */
/*
static void setupBotChatBotGift(CInterfaceGroup *botChatGroup)
{	
	// create dummy item in the db
	CInterfaceManager *im = CInterfaceManager::getInstance();
	im->getDbProp("SERVER:INVENTORY:20:0:SHEET")->setValue32(CSheetId("ai_flesh_poisson.item").asInt());
	im->getDbProp("SERVER:INVENTORY:20:0:QUALITY")->setValue32(0);
	im->getDbProp("SERVER:INVENTORY:20:1:SHEET")->setValue32(CSheetId("fyros_sword_lvl_01_05.item").asInt());
	im->getDbProp("SERVER:INVENTORY:20:1:QUALITY")->setValue32(2);
	CBotChat::setBotGift(botChatGroup, ucstring("Thanks to have succeeded the mission"), ucstring("Here's your reward"), ucstring("The bot has taken the object quest from your inventory"));
}
*/

//-----------------------------------------------
// impulseBotChatSetInterface :
//-----------------------------------------------
/*
void impulseBotChatSetInterface(NLMISC::CBitMemStream &impulse)
{
	// received ADD_DYN_STR

	CEntityId user;
	uint32 happyness;
	BOTCHATTYPE::TBotChatInterfaceId interfaceId;
	bool hasNews;

	impulse.serial (user);
	impulse.serial (happyness);

//	impulse.serialEnum (interfaceId);
	uint16	interfId;
	impulse.serial(interfId);
	interfaceId = (BOTCHATTYPE::TBotChatInterfaceId)(interfId&0xff);
	uint8 botType = (interfId>>8) & 0xff;

	impulse.serial (hasNews);

	nldebug("impulseCallBack : Received BOT_CHAT:SET_INTERFACE interface %d, have news %s, happy %d, bottype %hu", interfaceId, hasNews?"yes":"no", happyness,(uint16)botType);

	string stringId;
	vector<uint64> args;
	if (hasNews)
	{
		
/*		impulse.serial (stringId);
		impulse.serialCont (args);
		nlinfo ("receive the news '%s' with %d args", stringId.c_str(), args.size());
*/
		// TEMP FOR THE DEMO, DON'T USE THE NETWORK NEW BUT SELECT A NEWS HERE
/*
		CInterfaceGroup *inter = CInterfaceManager::getInstance()->getWindowFromId("ui:interface:bot_chat_intro");
		nlassert (inter != NULL);
		inter->setActive(true);
		
		CViewText *inter2 = (CViewText *)inter->getView("hi");
		nlassert (inter2 != NULL);
		inter2->NetworkTextId.setString("IOS_NEWS_FOOTBALL_SHORT_EEII", &ChatMngr);
		inter2->NetworkTextId.Args.push_back(10);
		inter2->NetworkTextId.Args.push_back(20);
		inter2->NetworkTextId.Args.push_back(1);
		inter2->NetworkTextId.Args.push_back(2);
*/	/*}

	// FOR THE DEMO, find and set a fake news:
/*	setFakeNews ();

	string interfaceName = getInterfaceNameFromId (botType, interfaceId);

	if(interfaceName.empty())
	{
		nlwarning ("Received an unknown bot chat interface %d", interfaceId);
	}
	else
	{
		CInterfaceGroup *inter = CInterfaceManager::getInstance()->getWindowFromId(interfaceName);
		if (inter == NULL)
		{
			nlwarning ("Can't find interface name '%s' %d", interfaceName.c_str(), interfaceId);
		}
		else
		{
			CInterfaceManager::getInstance()->setBotChatWin(inter);
			if (inter->getActive())
			{
				nlwarning ("Interface %s is already active, not normal!", interfaceName.c_str());
			}
			else
			{
				nlinfo ("server want to me display the bot chat interface %s %d", interfaceName.c_str(), interfaceId);
				inter->setActive(true);
			}
		}
	}
}*/



//-----------------------------------------------
// impulseBeginTrade :
//-----------------------------------------------
void impulseBeginTrade(NLMISC::CBitMemStream &impulse)
{
	//open trade window
	CInterfaceGroup* win = CInterfaceManager::getInstance()->getWindowFromId("ui:interface:trade");
	if (!win)
	{
		nlwarning("invalid interface ui:interface:trade");
		return;
	}
	win->setActive(true);
	CInterfaceManager::getInstance()->setBotChatWin(win);

}

//-----------------------------------------------
// impulseBuyPrice :
//-----------------------------------------------
void impulseBuyPrice(NLMISC::CBitMemStream &impulse)
{
	uint32 BuyPrice;
	impulse.serial( BuyPrice ); 
	CBotChatUI::setNewTradePrice(BuyPrice);
}

//-----------------------------------------------
// impulseBeginMissionChoice :
//-----------------------------------------------
void impulseBeginMissionChoice(NLMISC::CBitMemStream &impulse)
{
	//open mission choice window
	CInterfaceGroup* win = CInterfaceManager::getInstance()->getWindowFromId("ui:interface:mission_choice");
	if (!win)
	{
		nlwarning("ui:interface:mission_choice");
		return;
	}
	win->setActive(true);
	CInterfaceManager::getInstance()->setBotChatWin(win);
}

//-----------------------------------------------
// impulseBeginMissionChoice :
//-----------------------------------------------
void impulseBeginCast(NLMISC::CBitMemStream &impulse)
{
	//open cast window
	uint32 begin,end;
	impulse.serial(begin);
	impulse.serial(end);
	CInterfaceManager* iMngr = CInterfaceManager::getInstance();
	iMngr->getDbProp("UI:VARIABLES:SPELL_CAST")->setValue32(1);
	iMngr->getDbProp("UI:VARIABLES:CAST_BEGIN")->setValue32(begin);
	iMngr->getDbProp("UI:VARIABLES:CAST_END")->setValue32(end);
}



//-----------------------------------------------
// command 'startBotChat'
//-----------------------------------------------
NLMISC_COMMAND( startBotChat, "Starts a bot chat with the targeted bot", "" )
{
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("BOTCHAT:START", out))
	{
		NetMngr.push(out);
		nldebug("impulseCallBack : BOTCHAT:START sent");
	}
	else
		nlwarning("impulseCallBack : unknown message name : 'BOTCHAT:START'.");

	return true;
}

//-----------------------------------------------
// impulseCorrectPos :
// Message from the server to correct the user position because he is not at the same position on the server..
//-----------------------------------------------
void impulseCorrectPos(NLMISC::CBitMemStream &impulse)
{
	// TP:CORRECT
	nlinfo("impulseCorrectPos: received TP:CORRECT.");
	sint32 x, y, z;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(z);
	nlinfo("impulseCorrectPos: new user position %d %d %d", x, y, z);

	if(UserEntity.mode() != MBEHAV::COMBAT_FLOAT)
	{
		// Compute the destination.
		CVectorD dest = CVectorD((float)x/1000.0f, (float)y/1000.0f, (float)z/1000.0f);

		// Update the position for the vision.
		NetMngr.setReferencePosition(dest);

		// Change the user poisition.
		UserEntity.pacsPos(dest);
	}
}// impulseCorrectPos //

class CDummyProgress : public IProgressCallback
{
	void progress (float value) {};
};

//-----------------------------------------------
// impulseTP :
// Message from the server to teleport the user.
// \warning This function remove the current target. Do no use to correct a position.
//-----------------------------------------------
void impulseTP(NLMISC::CBitMemStream &impulse)
{
	// received ADD_DYN_STR
	nlinfo("impulseTP: received a request for a TP.");
	sint32 x, y, z;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(z);
	nlinfo("impulseTP: to %d %d %d", x, y, z);

	// Msg Received, send an acknowledge.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("TP:ACK", out))
	{
		NetMngr.push(out);
		nlinfo("impulseTP: teleport acknowledge 'TP:ACK' sent.");
	}
	else
		nlwarning("impulseTP: unknown message name : 'TP:ACK'.");

	// Remove the selection.
	UserEntity.selection(CLFECOMMON::INVALID_SLOT);

	// Compute the destination.
	CVectorD dest = CVectorD((float)x/1000.0f, (float)y/1000.0f, (float)z/1000.0f);

	// Update the position for the vision.
	NetMngr.setReferencePosition(dest);

	// Change the position of the entity and in Pacs.
	UserEntity.pos(dest);
	// Select the closest continent from the new position.
	CDummyProgress dummy;
	ContinentMngr.select(dest, dummy);
	UserEntity.pacsPos(dest);

	// Return to interface mode.
	UserControls.mode(CUserControls::InterfaceMode);
	// User well oriented.
	UserEntity.dir(UserEntity.front());
	UserEntity.head(UserEntity.front());
	// Set animation with idle.
	UserEntity.setAnim(CAnimationState::Idle);
}// impulseTP //

//-----------------------------------------------
// impulseCombatEngageFailed :
//-----------------------------------------------
void impulseCombatEngageFailed(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseCombatEngageFailed: Combat Engage Failed.");

	// Unlock the motion.
	UserControls.locked(false);
}// impulseCombatEngageFailed //

//-----------------------------------------------
// impulseTeamInvitation :
//-----------------------------------------------
void impulseTeamInvitation(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseTeamInvitation: received an invitation");

	uint32 textID;
	impulse.serial(textID);

	//activate the pop up window 
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceGroup *group = dynamic_cast<CInterfaceGroup *>( im->getElementFromId("ui:interface:join_team_proposal"));
	if (!group) return;
	CViewTextID *vti = dynamic_cast<CViewTextID *>(group->getView("invitor_name"));
	if (!vti) return;
	vti->setTextId(textID);
	group->setActive(true);
}// impulseTeamInvitation //



// TEMP TEMP TEMP
NLMISC_COMMAND(testTeamInvite, "","")
{
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	uint32 index = 10;
	bm.serial(index);
	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseTeamInvitation(bm);
	return true;
}

//-----------------------------------------------
// impulseExchangeInvitation :
//-----------------------------------------------
void impulseExchangeInvitation(NLMISC::CBitMemStream &impulse)
{
	nlinfo("receiving exchange invitation");
	uint32 nameIndex;
	impulse.serial(nameIndex);

	// set the player name id in the database
	static const char *otherPlayerNameDefineStr = "other_player_name_id";
	CInterfaceManager* iMngr = CInterfaceManager::getInstance();

	std::string otherPlayerNameIdDbPath = iMngr->getDefine(otherPlayerNameDefineStr);
	if (otherPlayerNameIdDbPath.empty())
	{
		nlwarning("<impulseExchangeInvitation> Can't retrieve define %s", otherPlayerNameDefineStr);
		return;
	}
	iMngr->getDbProp(otherPlayerNameIdDbPath)->setValue64((sint64) nameIndex);

	// show the modal window that allow the player to accept / decline the invitation	
	iMngr->enableModalWindow(NULL, "ui:interface:accept_trade_invitation");	

}// impulseExchangeInvitation //


//-----------------------------------------------
// impulseMountAbort :
//-----------------------------------------------
void impulseMountAbort(NLMISC::CBitMemStream &impulse)
{
	nlwarning("impulseMountAbort: Received ANIMALS:MOUNT_ABORT => no more used");
}// impulseMountAbort //

//-----------------------------------------------
// impulseRyzomTime :
// Synchronize the ryzom time with the server.
//-----------------------------------------------
void impulseRyzomTime(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseRyzomTime: Ryzom Time Received");
	uint32 serverTick;
	float ryzomTime;
	uint32 ryzomDay;
	impulse.serial(serverTick);
	impulse.serial(ryzomTime);
	impulse.serial(ryzomDay);
	nlinfo("impulseRyzomTime: Day '%d' Time '%f'.", ryzomDay, ryzomTime);

	// Initialize
	LastGameTimeUpdate = serverTick;
	RT.init(ryzomDay, ryzomTime);
	TimeAtTick = T1;
}// impulseRyzomTime //

//-----------------------------------------------
// impulseWhere :
// Display server position
//-----------------------------------------------
void impulseWhere(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseWhere Received");
	sint32 x,y,z;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(z);
	char buf[128];

	double xf = ((double)x)/1000.0f;
	double yf = ((double)y)/1000.0f;
	double zf = ((double)z)/1000.0f;

	sprintf(buf,"Your server position is : X= %g   Y= %g   Z= %g",xf,yf,zf);
	nlinfo(buf);
	CInterfaceManager::getInstance()->displaySystemInfo(ucstring(buf));
}// impulseWhere //

//-----------------------------------------------
// impulseWho :
// Display server position
//-----------------------------------------------
void impulseWho(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseWhere Received");
	CInterfaceManager::getInstance()->displaySystemInfo(ucstring("Players currently in the game :"));
	
	ucstring name;
	uint32 loginId;
	char buf [128];
	while( impulse.getPos() < (sint32)impulse.length() )
	{
		impulse.serial(name);
		impulse.serial(loginId);
		sprintf(buf," - ID: %d", loginId);
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring(name + ucstring(buf)));
	}
}// impulseWho //

//-----------------------------------------------
// impulseCounter :
// check UDP validity
//-----------------------------------------------
void impulseCounter(NLMISC::CBitMemStream &impulse)
{
	nldebug("impulseCallBack : counter check.");
	try
	{
		uint32	counter;
		impulse.serial(counter);

		static uint			queueTop = 0;
		static deque<bool>	queue;

		if (counter > queueTop)
		{
			queue.resize(queue.size()+counter-queueTop, false);
			queueTop = counter;
		}

		if (queueTop-counter+1 > queue.size())
		{
			nlinfo("COUNTER: counter %d arrived too late...", counter);
		}
		else
		{
			if (queue[queue.size()-1-(queueTop-counter)])
			{
				nlwarning("COUNTER: Received counter %d more than once !", counter);
			}
			else
			{
				nldebug("COUNTER: set counter %d", counter);
				queue[queue.size()-1-(queueTop-counter)] = true;
			}

			while (queue.size() > 128)
			{
				if (!queue.front())
				{
					nlwarning("COUNTER: counter %d not received !", queueTop-queue.size()-1);
				}

				queue.pop_front();
			}
		}
	}
	catch (Exception &e)
	{
		nlwarning ("Problem during decoding a COUTNER message, skip it: %s", e.what());
	}
}

//-----------------------------------------------
// impulsePhraseSend :
// A dyn string (or phrase) is send (so, we receive it)
//-----------------------------------------------
void impulsePhraseSend(NLMISC::CBitMemStream &impulse)
{
	STRING_MANAGER::CStringManagerClient::instance()->receiveDynString(impulse);
}

//-----------------------------------------------
// impulseStringResp :
// Update the local string set
//-----------------------------------------------
void impulseStringResp(NLMISC::CBitMemStream &impulse)
{
	uint32 stringId;
	string	strUtf8;
	impulse.serial(stringId);
	impulse.serial(strUtf8);
	ucstring str;
	str.fromUtf8(strUtf8);
	
	STRING_MANAGER::CStringManagerClient::instance()->receiveString(stringId, str);
}

//-----------------------------------------------
// impulseReloadCache :
// reload the string cache
//-----------------------------------------------
void impulseReloadCache(NLMISC::CBitMemStream &impulse)
{
	uint32 timestamp;;
	impulse.serial(timestamp);

	STRING_MANAGER::CStringManagerClient::instance()->loadCache(timestamp);
}

//-----------------------------------------------
// impulseBotChatReceiveMission :
// receive the mission list
//-----------------------------------------------
void impulseBotChatReceiveMission(NLMISC::CBitMemStream &impulse)
{
	uint8  missionListType;
	uint8  totalNumMissions;    // the total number of mission
	uint8  numReceivedMissions; // the number of missions received in this call.
	//
	impulse.serial(missionListType);
	impulse.serial(totalNumMissions);
	impulse.serial(numReceivedMissions);
	//
	std::vector<CChoiceTextIDs> textIDs;
	textIDs.resize(numReceivedMissions);
	for(uint k = 0; k < numReceivedMissions; ++k)
	{
		impulse.serial(textIDs[k].DescTextID); // choice description
		impulse.serial(textIDs[k].DetailsTextID); // choice details
		if (missionListType == BOTCHATTYPE::AvailableMissionList)
		{
			impulse.serial(textIDs[k].LogicTextID); // choice details
		}
		else
		{
			textIDs[k].LogicTextID = 0;
		}
		if (missionListType == BOTCHATTYPE::MissionContinuationList)
		{
			uint8 flags;
			impulse.serial(flags);
			textIDs[k].NeedPlayerGift = (flags & 2)!=0;
			textIDs[k].LeadToMissionCompletion = (flags & 1)!=0;
		}
		else
		{
			textIDs[k].NeedPlayerGift = false;
			textIDs[k].LeadToMissionCompletion = false;
		}
	}
	CBotChat::updateMissionList((BOTCHATTYPE::TChoiceListType) missionListType, (uint) totalNumMissions, textIDs);
}

//-----------------------------------------------
// impulseBotChatEnd
// ForceThe end of the bot chat
//-----------------------------------------------
void impulseBotChatForceEnd(NLMISC::CBitMemStream &impulse)
{
	CBotChat::endDialog();
}


//-----------------------------------------------
// temp for test  : command 'testExchangeInvitation'
//-----------------------------------------------
NLMISC_COMMAND( testExchangeInvitation, "Test the modal window for invitation exchange", "" )
{
	CBitMemStream impulse;
	uint32 nameIndex = 0;
	impulse.serial(nameIndex);
	impulse.invert();
	impulseExchangeInvitation(impulse);
	return true;
}


//-----------------------------------------------
// MISSION COMPLETED JOURNAL
//-----------------------------------------------

#define MC_M_CONTAINER "ui:interface:info_player_journal"
#define MC_S_CONTAINER "ui:interface:ipj_com_missions"
#define MC_TEMPLATE "tipj_mission_complete"
//-----------------------------------------------
CGroupContainer *getMissionCompletedContainer()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterfaceElement *pIE = pIM->getElementFromId(MC_M_CONTAINER);
	CGroupContainer *pGCM = dynamic_cast<CGroupContainer*>(pIE);
	if (pGCM == NULL) return NULL;

	CGroupList *pList = pGCM->getList();
	CGroupContainer *pGCS = dynamic_cast<CGroupContainer*>(pList->getGroup(MC_S_CONTAINER));
	return pGCS;
}

//-----------------------------------------------
void clearMissions()
{
	CGroupContainer *pGCMC = getMissionCompletedContainer();
	CInterfaceGroup *pContent = pGCMC->getGroup("content");
	pContent->clearGroups();
}
//-----------------------------------------------
void addMission(uint32 titleID)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGCMC = getMissionCompletedContainer();
	if (pGCMC == NULL)
	{
		nlwarning("cannot get container for missions completed");
		return;
	}
	CInterfaceGroup *pContent = pGCMC->getGroup("content");

	uint32 nNbMission = pContent->getGroups().size();
	vector<pair<string, string> > vArgs;

	vArgs.push_back(pair<string,string>("id", "mc"+NLMISC::toString(nNbMission)));
	vArgs.push_back(pair<string,string>("mcid", NLMISC::toString(titleID)));

 	if (nNbMission == 0)
	{
		vArgs.push_back(pair<string,string>("posref", "TL TL"));
		vArgs.push_back(pair<string,string>("posparent", "parent"));
		vArgs.push_back(pair<string,string>("y", "0"));
	}
	else
	{
		vArgs.push_back(pair<string,string>("posref", "BL TL"));
	}

	CInterfaceGroup *pIG = pIM->createGroupInstance(MC_TEMPLATE, pContent->getId(), vArgs);
	if (pIG == NULL)
	{
		nlwarning("cannot create a mission completed");
		return;
	}
	pIG->setParent(pContent);
 	if (nNbMission == 0)
		pIG->setParentPos(pContent);
	else
		pIG->setParentPos(pContent->getGroups()[nNbMission-1]);
	pContent->addGroup(pIG);
}

//-----------------------------------------------
// impulseJournalInitCompletedMissions :
// initialize the player journal missions for completed missions
//-----------------------------------------------
void impulseJournalInitCompletedMissions (NLMISC::CBitMemStream &impulse)
{
	vector<uint32> vMissionCompleted;
	impulse.serialCont(vMissionCompleted);
	
	clearMissions();
	
	for (uint32 i = 0; i < vMissionCompleted.size(); ++i)
		addMission (vMissionCompleted[i]);
}

//-----------------------------------------------
// impulseJournalInitCompletedMissions :
// initialize the player journal missions for completed missions
//-----------------------------------------------
void impulseJournalUpdateCompletedMissions (NLMISC::CBitMemStream &impulse)
{
	uint32 nNewCompletedMission;
	impulse.serial(nNewCompletedMission);
	
	addMission (nNewCompletedMission);
}


//-----------------------------------------------
// initializeNetwork :
//-----------------------------------------------
void initializeNetwork()
{
	GenericMsgHeaderMngr.setCallback("DATABASE",						impulseDatabase);
	GenericMsgHeaderMngr.setCallback("CONNECTION:NO_USER_CHAR",			impulseNoUserChar);
	GenericMsgHeaderMngr.setCallback("CONNECTION:USER_CHARS",			impulseUserChars);
	GenericMsgHeaderMngr.setCallback("CONNECTION:USER_CHAR",			impulseUserChar);
	GenericMsgHeaderMngr.setCallback("CONNECTION:READY",				impulseServerReady);
	GenericMsgHeaderMngr.setCallback("CONNECTION:TIME_DATE_SYNCHRO",	impulseRyzomTime);
	GenericMsgHeaderMngr.setCallback("CONNECTION:VALID_NAME",			impulseCharNameValid);

	GenericMsgHeaderMngr.setCallback("STRING:CHAT",				impulseChat);
	GenericMsgHeaderMngr.setCallback("STRING:TELL",				impulseTell);
//	GenericMsgHeaderMngr.setCallback("STRING:ADD_DYN_STR",		impulseAddDynStr);
	GenericMsgHeaderMngr.setCallback("TP:DEST",					impulseTP);
	GenericMsgHeaderMngr.setCallback("TP:CORRECT",				impulseCorrectPos);
	GenericMsgHeaderMngr.setCallback("COMBAT:ENGAGE_FAILED",	impulseCombatEngageFailed);
	GenericMsgHeaderMngr.setCallback("TRADE:BEGIN",				impulseBeginTrade);	
	GenericMsgHeaderMngr.setCallback("TRADE:BUY_PRICE",			impulseBuyPrice);	
	GenericMsgHeaderMngr.setCallback("MISSIONS:BEGIN_CHOICE",	impulseBeginMissionChoice);
	GenericMsgHeaderMngr.setCallback("CASTING:BEGIN",			impulseBeginCast);
	GenericMsgHeaderMngr.setCallback("TEAM:INVITATION",			impulseTeamInvitation);	
	GenericMsgHeaderMngr.setCallback("EXCHANGE:INVITATION",		impulseExchangeInvitation);		
	GenericMsgHeaderMngr.setCallback("ANIMALS:MOUNT_ABORT",		impulseMountAbort);		



	GenericMsgHeaderMngr.setCallback("DEBUG:REPLY_WHERE",		impulseWhere);
	GenericMsgHeaderMngr.setCallback("DEBUG:REPLY_WHO",			impulseWho);
	GenericMsgHeaderMngr.setCallback("DEBUG:COUNTER",			impulseCounter);

	//
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:PHRASE_SEND",		impulsePhraseSend);
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:STRING_RESP",		impulseStringResp);
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:RELOAD_CACHE",		impulseReloadCache);
	//
	GenericMsgHeaderMngr.setCallback("BOTCHAT:RECEIVE_MISSIONS",		impulseBotChatReceiveMission);
	GenericMsgHeaderMngr.setCallback("BOTCHAT:FORCE_END",		impulseBotChatForceEnd);

	GenericMsgHeaderMngr.setCallback("JOURNAL:INIT_COMPLETED_MISSIONS",		impulseJournalInitCompletedMissions);
	GenericMsgHeaderMngr.setCallback("JOURNAL:UPDATE_COMPLETED_MISSIONS",	impulseJournalUpdateCompletedMissions);

}// initializeNetwork //


//-----------------------------------------------
// impulseCallBack :
// The impulse callback to receive all msg from the frontend.
//-----------------------------------------------
void impulseCallBack(NLMISC::CBitMemStream &impulse, sint32 packet, void *arg)
{
	GenericMsgHeaderMngr.execute(impulse);
}// impulseCallBack //


////////////
// METHOD //
////////////
//-----------------------------------------------
// CNetManager :
// Constructor.
//-----------------------------------------------
CNetManager::CNetManager()
{
	_CurrentClientTick = 0;
	_CurrentServerTick = 0;
	_MsPerTick = 100;
	_LCT = 1000;
#ifndef ENABLE_INCOMING_MSG_RECORDER
	_IsReplayStarting = false;
#endif
}// CNetManager //

//-----------------------------------------------
// update :
// Updates the whole connection with the frontend.
// Call this method evently.
// \return bool : 'true' if data were sent/received.
//-----------------------------------------------
bool CNetManager::update()
{
	H_AUTO_USE ( RZ_Client_Net_Mngr_Update )

#ifndef ENABLE_INCOMING_MSG_RECORDER
	if(_IsReplayStarting)
		return;
#endif
		
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
	{
		// Init
		if(_CurrentServerTick == 0)
		{
			if(T1 >= _LCT)
			{
				_MachineTimeAtTick = T1;
				_CurrentClientTime = _MachineTimeAtTick - _LCT;
				_CurrentClientTick = 0;
				_CurrentServerTick = 10;
			}

			return false;
		}
		
		if((T1 - _MachineTimeAtTick) >= _MsPerTick)
		{
			NLMISC::TGameCycle nbTick = (NLMISC::TGameCycle)((T1 - _MachineTimeAtTick)/_MsPerTick);
			_CurrentClientTick += nbTick;
			_CurrentServerTick += nbTick;
			_MachineTimeAtTick += nbTick*_MsPerTick;
		}

		// update the smooth server tick for debug
		CNetworkConnection::updateSmoothServerTick();

		// emulation done
#ifndef ENABLE_INCOMING_MSG_RECORDER
		return false;
#endif
	}

	// Update the base class.
	bool result = CNetworkConnection::update();

	// Get changes with the update.
	const vector<CChange> &changes = NetMngr.getChanges();

	// Manage changes
	vector<CChange>::const_iterator it;
	for(it = changes.begin(); it < changes.end(); ++it)
	{
		const CChange &change = *it;
		// Update a property.
		if(change.Property < AddNewEntity)
		{
			// Update the visual property for the slot.
			EntitiesMngr.updateVisualProperty(change.GameCycle, change.ShortId, change.Property, change.PredictedInterval);
		}
		// Add New Entity (and remove the old one in the slot).
		else if(change.Property == AddNewEntity)
		{
			// Remove the old entity.
			EntitiesMngr.remove(change.ShortId, false);
			// Create the new entity.
			EntitiesMngr.create(change.ShortId, get(change.ShortId));
		}
		// Delete an entity
		else if(change.Property == RemoveOldEntity)
		{
			// Remove the old entity.
			EntitiesMngr.remove(change.ShortId, true);
		}
		// Lag detected.
		else if(change.Property == LagDetected)
		{
			nldebug("CNetManager::update : Lag detected.");
		}
		// Lag detected.
		else if(change.Property == ProbeReceived)
		{
			nldebug("CNetManager::update : Probe Received.");
		}
		// Lag detected.
		else if(change.Property == ConnectionReady)
		{
			nldebug("CNetManager::update : Connection Ready.");
		}
		// Property unknown.
		else
			nlwarning("CNetManager::update : The property '%d' is unknown.", change.Property);
	}
	ChatMngr.flushChatBuffer();
	// Clear all changes.
	clearChanges();

	// Update data base server state
	if (IngameDbMngr.getNodePtr())
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (im)
		{
			CCDBNodeLeaf *node = im->getDbProp("UI:VARIABLES:PING", false);
			if (node)
				node->setValue32(getPing());
			node = im->getDbProp("UI:VARIABLES:UPLOAD", false);
			if (node)
				node->setValue32((sint32)(getMeanUpload()*1024.f/8.f));
			node = im->getDbProp("UI:VARIABLES:DOWNLOAD", false);
			if (node)
				node->setValue32((sint32)(getMeanDownload()*1024.f/8.f));
			node = im->getDbProp("UI:VARIABLES:PACKETLOST", false);
			if (node)
				node->setValue32((sint32)getMeanPacketLoss());
			node = im->getDbProp("UI:VARIABLES:SERVERSTATE", false);
			if (node)
				node->setValue32((sint32)getConnectionState());
			node = im->getDbProp("UI:VARIABLES:CONNECTION_QUALITY", false);
			if (node)
				node->setValue32((sint32)getConnectionQuality());			
		}
	}

	// Return 'true' if data were sent/received.
	return result;

	
}// update //

/**
 * Buffers a bitmemstream, that will be converted into a generic action, to be sent later to the server (at next update).
 */
void CNetManager::push(NLMISC::CBitMemStream &msg)
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
		return;

	CNetworkConnection::push(msg);
}

/**
 * Buffers a target action
 */
void CNetManager::pushTarget(CLFECOMMON::TCLEntityId slot)
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
	{
		if(UserEntity.mode() != MBEHAV::COMBAT
		&& UserEntity.mode() != MBEHAV::COMBAT_FLOAT)
			UserEntity.targetSlot(slot);
		return;
	}

	CNetworkConnection::pushTarget(slot, LHSTATE::NONE);
}


/**
 * Buffers a pick-up action
 */
void CNetManager::pushPickup(CLFECOMMON::TCLEntityId slot, LHSTATE::TLHState lootOrHarvest)
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
	{
		return;
	}

	CNetworkConnection::pushTarget(slot, lootOrHarvest);
}


/**
 * Send
 */
void CNetManager::send(NLMISC::TGameCycle gameCycle)
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
		return;

	// wait till next server is received
	if (_LastSentCycle >= gameCycle)
	{
		nlinfo ("Try to CNetManager::send(%d) _LastSentCycle=%d more than one time with the same game cycle, so we wait new game cycle to send", gameCycle, _LastSentCycle);
		while (_LastSentCycle >= gameCycle)
		{
			// Update network.
			update();
			// Send dummy info
			send();
			// Do not take all the CPU.
			nlSleep(100);

			gameCycle = getCurrentServerTick();
		}
	}

	CNetworkConnection::send(gameCycle);
}

/**
 * Send
 */
void CNetManager::send()
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
		return;

	CNetworkConnection::send();
}

/**
 * Disconnects the current connection
 */
void CNetManager::disconnect()
{
	// If the client is in Local Mode -> no need to disconnect.
	if(ClientCfg.Local)
		return;

	CNetworkConnection::disconnect();
}// disconnect //


/*NLMISC_COMMAND(receiveChatBot, "","<bot type> <interface id>")
{
	if (args.size() != 2)
		return false;

	string res = getInterfaceNameFromId (atoi(args[0].c_str()), atoi(args[1].c_str()));

	if (res.empty())
		return false;

	CInterfaceGroup *inter = CInterfaceManager::getInstance()->getWindowFromId(res);
	if (inter == NULL)
		return false;

	setFakeNews ();

	inter->setActive(true);
	
	return true;
}*/



void CNetManager::waitForServer()
{
	sint	LastGameCycle = getCurrentServerTick();

	while (true)
	{
		// Event server get events
		CInputHandlerManager::getInstance()->pumpEventsNoIM();
		// Update Network.
		update();

		if (LastGameCycle != getCurrentServerTick())
			break;

		nlSleep(100);
		send();
	}

}// waitForServer //


NLMISC_COMMAND(receiveNews, "","")
{
	CInterfaceGroup *inter = CInterfaceManager::getInstance()->getWindowFromId("ui:interface:bot_chat_intro");
	nlassert (inter != NULL);
	inter->setActive(true);
	
	CViewText *inter2 = (CViewText *)inter->getView("hi");
	nlassert (inter2 != NULL);
	ucstring news = "IOS_NEWS_FOOTBALL_SHORT_EEII";
//	inter2->NetworkTextId.setString(news, &ChatMngr);
//	inter2->NetworkTextId.Args.push_back(10);
//	inter2->NetworkTextId.Args.push_back(20);
//	inter2->NetworkTextId.Args.push_back(1);
//	inter2->NetworkTextId.Args.push_back(2);

	return true;
}

NLMISC_COMMAND(receiveId, "","<num> <name>")
{
	uint32 index = atoi(args[0].c_str());
	ucstring ucstr = args[1];

	vector<bool> code;

	#ifdef OLD_STRING_SYSTEM
		ChatMngr.getDynamicDB().add( index, ucstr, code );
	#else
		// TRAP // WE MUST NEVER CALL THIS COMMAND ANYMORE : ALL IS HANDLED BY STRING_MANAGER NOW !!!
		nlstop;	
	#endif

	return true;
}


#ifdef ENABLE_INCOMING_MSG_RECORDER
//-----------------------------------------------
// setReplayingMode :
//-----------------------------------------------
void CNetManager::setReplayingMode( bool onOff, const std::string& filename )
{
	CNetworkConnection::setReplayingMode(onOff, filename);
	_IsReplayStarting = onOff;
}// setReplayingMode //

//-----------------------------------------------
// startReplay :
//-----------------------------------------------
void CNetManager::startReplay()
{
	// Init Replay
	_MachineTimeAtTick = T1;
	if(_MachineTimeAtTick >= _LCT)
		_CurrentClientTime = _MachineTimeAtTick - _LCT;
	else
		_CurrentClientTime = 0;
	// Replay now in progress.
	_IsReplayStarting = false;
}// startReplay //
#endif
