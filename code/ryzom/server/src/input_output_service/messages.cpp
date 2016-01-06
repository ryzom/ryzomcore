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
#include "messages.h"

#include <nel/misc/entity_id.h>
#include <nel/net/unified_network.h>

#include "game_share/ryzom_mirror_properties.h"

#include "input_output_service.h"
/*#include "game_share/tick_event_handler.h"
#include "game_share/msg_client_server.h"
#include "game_share/mode_and_behaviour.h" //TEMP!!!
#include "game_share/news_types.h"
#include "game_share/bot_chat_types.h"
#include "game_share/brick_types.h"
#include "game_share/loot_harvest_state.h"
#include "game_share/ryzom_version.h"*/
#include "game_share/generic_xml_msg_mngr.h"

/*#include <nel/misc/command.h>*/

using namespace std;
using namespace NLMISC;
using namespace NLNET;


extern CGenericXmlMsgHeaderManager GenericXmlMsgHeaderMngr;

extern CVariable<bool>	VerboseChatManagement;


//-----------------------------------------------
//	cbImpulsionReadyString :
//
//-----------------------------------------------

/*
void cbImpulsionReadyString( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	nlwarning("cbImpulsionReadyString : deprecated");
	// Read sender and bitmemstream
	CEntityId sender;
	uint8 nbBitsToSkip;
	CBitMemStream bms(true);
	msgin.serial(sender);
	msgin.serial(nbBitsToSkip);
	msgin.serialMemStream(bms);
	
	// Unpack bitmemstream
	uint32 xmlcode;
	bms.serial(xmlcode, nbBitsToSkip); // the XML code was already read by tbe sender service
	std::string languageCode;
	bms.serial(languageCode);

	nlinfo("<impulsionRdy> send ready for client %s using language %s",sender.toString().c_str(), languageCode.c_str());

	CCharacterInfos * charInfos = IOS->getCharInfos( sender );
	if( charInfos != NULL )
	{
//		charInfos->Id.getDynamicId() = serviceId;
		// read the language code used by the client for this session
		charInfos->Language = SM->checkLanguageCode(languageCode);
		nldebug ("set user %s to front end %d using language code %s", sender.toString().c_str(), serviceId, SM->getLanguageCodeString(charInfos->Language).c_str());


		// send back the cache time stamp info
		uint32 timestamp = SM->getCacheTimestamp();

		// now, build the message for the client.
		NLMISC::CBitMemStream bmsOut;
		GenericXmlMsgHeaderMngr.pushNameToStream( "STRING_MANAGER:RELOAD_CACHE", bmsOut);
		bmsOut.serial(timestamp);

		// send the message to Front End
		NLNET::CMessage msgout( "IMPULS_CH_ID" );
		NLMISC::CEntityId destId = sender;
		uint8 channel = 1;
		msgout.serial( destId );
		msgout.serial( channel );

		msgout.serialBufferWithSize((uint8*)bmsOut.buffer(), bmsOut.length());
		NLNET::CUnifiedNetwork::getInstance()->send(charInfos->EntityId.getDynamicId(), msgout);
	}
	else
	{
		nlwarning("<cbImpulsionReadyString> The character %s doesn't have infos",sender.toString().c_str());
	}
}
*/


//-----------------------------------------------
//	cbImpulsionChat :
//
//-----------------------------------------------
void cbImpulsionChat( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	CEntityId sender;
	msgin.serial(sender);

	ucstring ucstr;
	try
	{
		msgin.serial(ucstr);
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionChat> %s",e.what());
		return;
	}

	// Do a general filtering on client input
	ucstr = IOS->getChatManager().filterClientInput(ucstr);

	if( ShowChat )
	{
		nlinfo("<impulsionChat> CHAT received : %s",ucstr.toString().c_str());
	}

	try
	{
		TDataSetRow senderRow = TheDataset.getDataSetRow(sender);
		IOS->getChatManager().getClient(senderRow).updateAudience();
		IOS->getChatManager().chat( senderRow, ucstr );
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionChat> %s",e.what());
	}
} // impulsionChat //


//-----------------------------------------------
//	cbImpulsionChatTeam :
//
//-----------------------------------------------
void cbImpulsionChatTeam( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	CEntityId sender;
	msgin.serial(sender);

	ucstring ucstr;
	try
	{
		msgin.serial(ucstr);
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionChatTeam> %s",e.what());
		return;
	}

	// Do a general filtering on client input
	ucstr = IOS->getChatManager().filterClientInput(ucstr);

	if( ShowChat )
	{
		nlinfo("<impulsionChatTeam> CHAT_TEAM received : %s",ucstr.toString().c_str());
	}

	try
	{		
		TDataSetRow senderRow = TheDataset.getDataSetRow(sender);
		// get current chat mode
		const CChatGroup::TGroupType chatModeBck = IOS->getChatManager().getClient(senderRow).getChatMode();
		// set chat mode to group
		IOS->getChatManager().getClient(senderRow).setChatMode( CChatGroup::team );
		//IOS->getChatManager().getClient(sender).updateAudience(); // only for say and shout
		// send the message
		IOS->getChatManager().chat( senderRow, ucstr );
		// reset chat mode to old value
		IOS->getChatManager().getClient(senderRow).setChatMode( chatModeBck );		
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionChatTeam> %s",e.what());
	}
} // impulsionChatTeam //


//-----------------------------------------------
//	cbImpulsionTell :
//
//-----------------------------------------------
void cbImpulsionTell( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	CEntityId sender;
	msgin.serial(sender);

	string receiver;
	ucstring str;
	try
	{
		msgin.serial(receiver);
		msgin.serial(str);
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionTell> %s",e.what());
		return;
	}

	// Do a general filtering on client input
	str = IOS->getChatManager().filterClientInput(str);

	if( ShowChat )
	{
		nlinfo("<impulsionTell> TELL received for char %s: '%s'",receiver.c_str(),str.toString().c_str());
	}
	TDataSetRow senderRow = TheDataset.getDataSetRow(sender);

	IOS->getChatManager().tell( senderRow, receiver, str );

} // impulsionTell //

//-----------------------------------------------
//	setIgnoreStatus
//
//-----------------------------------------------
static void updateIgnoreStatus(CMessage& msgin, const string &serviceName, TServiceId serviceId, bool ignored)
{
	CEntityId senderId;
	CEntityId ignoredId;	
	
	try
	{
		msgin.serial(senderId);
		msgin.serial(ignoredId);
		IOS->getChatManager().getClient(TheDataset.getDataSetRow(senderId)).setIgnoreStatus(ignoredId, ignored);	
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionIgnore> %s",e.what());
		return;
	}	
}

//-----------------------------------------------
//	cbImpulsionIgnore :
//
//-----------------------------------------------
void cbImpulsionIgnore( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	updateIgnoreStatus(msgin, serviceName, serviceId, true);	
} // impulsionIgnore //

//-----------------------------------------------
//	cbImpulsionUnignore :
//
//-----------------------------------------------
void cbImpulsionUnignore( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	updateIgnoreStatus(msgin, serviceName, serviceId, false);
} // impulsionUnignore //

//-----------------------------------------------
//	cbImpulsionUnignoreAll :
//
//-----------------------------------------------
void cbImpulsionUnignoreAll( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	CEntityId senderId;
	vector<CEntityId> ignoredIds;
	
	try
	{
		msgin.serial(senderId);
		msgin.serialCont(ignoredIds);
		for ( vector<CEntityId>::const_iterator it=ignoredIds.begin(); it!=ignoredIds.end(); ++it )
		{
			IOS->getChatManager().getClient(TheDataset.getDataSetRow(senderId)).setIgnoreStatus(*it, false);
		}
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionIgnoreAll> %s",e.what());
		return;
	}

} // impulsionUnignoreAll //


//-----------------------------------------------
//	cbImpulsionFilter :
//
//-----------------------------------------------
void cbImpulsionFilter( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	CEntityId sender;
	msgin.serial(sender);

	uint8 filterId;
	try
	{
		msgin.serial( filterId );
	}
	catch(const Exception &e)
	{
		nlwarning("<impulsionFilter> %s",e.what());
		return;
	}

	CCharacterInfos * charInfos = IOS->getCharInfos( sender );
	if( charInfos != NULL )
	{
		try
		{
			IOS->getChatManager().getClient(TheDataset.getDataSetRow(sender)).filter( filterId );
		}
		catch(const Exception &e)
		{
			nlwarning("<impulsionFilter> %s",e.what());
		}
	}
	else
	{
		nlwarning("<impulsionFilter>The character %s doesn't have infos",sender.toString().c_str());
	}
} // impulsionFilter //


static const char* DebugChatModeName[] =
{
	"say",
	"shout",
	"team",
	"guild",
	"civilization",
	"territory",
	"universe",
	"tell",
	"player",
	"arround",
	"system",
	"region",
	"nbChatMode",
};

//-----------------------------------------------
//	cbImpulsionChatMode :
//
//-----------------------------------------------
void cbImpulsionChatMode( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	CEntityId sender;
	msgin.serial(sender);

	uint8 chatMode;
	TChanID chanID;	
	try
	{
		msgin.serial( chatMode );
		msgin.serial(chanID);
	}
	catch(const Exception &e)
	{
		nlwarning("<cbImpulsionChatMode> %s",e.what());
		return;
	}
	CCharacterInfos * charInfos = IOS->getCharInfos( sender );
	if( charInfos != NULL )
	{
		try
		{
			switch( chatMode )
			{
			case CChatGroup::say :
			case CChatGroup::shout :
			case CChatGroup::team :
			case CChatGroup::guild :
			case CChatGroup::region :
			case CChatGroup::universe :
				IOS->getChatManager().getClient(TheDataset.getDataSetRow(sender)).setChatMode( static_cast<CChatGroup::TGroupType>(chatMode));
				nldebug("IOS: Chat mode --> %s", DebugChatModeName[chatMode]);
				break;
			case CChatGroup::dyn_chat:											
				IOS->getChatManager().getClient(TheDataset.getDataSetRow(sender)).setChatMode( static_cast<CChatGroup::TGroupType>(chatMode), chanID);			
				break;
			default:
				if (chatMode < CChatGroup::nbChatMode)
					nlwarning("<cbImpulsionChatMode>(CHAT_MODE) The chat mode '%s' can't be set", DebugChatModeName[chatMode]);
				else
					nlwarning("<cbImpulsionChatMode>(CHAT_MODE) The chat mode %d is out of enum !", chatMode);
			}
		}
		catch(const Exception &e)
		{
			nlwarning("<cbImpulsionChatMode> %s",e.what());
		}
	}
	else
	{
		nlwarning("<cbImpulsionChatMode> The character %s doesn't have infos",sender.toString().c_str());
	}

} // cbImpulsionChatMode //


//-----------------------------------------------
//	cbImpulsionAfkTxt :
//
//-----------------------------------------------
void cbImpulsionAfkTxt( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	CEntityId sender;
	msgin.serial(sender);

	ucstring afkTxt;
	try
	{
		msgin.serial( afkTxt );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbImpulsionAfkTxt> %s",e.what());
		return;
	}

	// Do a general filtering on client input
	afkTxt = IOS->getChatManager().filterClientInputColorCode(afkTxt);

	CCharacterInfos * charInfos = IOS->getCharInfos( sender );
	if( charInfos != NULL )
	{
		charInfos->AfkCustomTxt = afkTxt;
	}
	else
	{
		nlwarning("<cbImpulsionAfkTxt> The character %s doesn't have infos",sender.toString().c_str());
	}

} // cbImpulsionAfkTxt //


//-----------------------------------------------
//	cbImpulsionStringRqId :
//
//-----------------------------------------------
//void cbImpulsionStringRqId( CMessage& msgin, const string &serviceName, TServiceId serviceId )
//{
//	CEntityId sender;
//	msgin.serial(sender);
//
//	uint32 stringId;
//	msgin.serial(stringId);
//
//	SM->requestString(sender, stringId);
//}


//-----------------------------------------------
//	cbImpulsionStringRqUid :
//
//-----------------------------------------------
void cbImpulsionStringRqUid( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	uint32 userId;
	msgin.serial(userId);

	uint32 stringId;
	msgin.serial(stringId);

	SM->requestString(userId, stringId);
}


struct CReceiver
{
//	CEntityId Id;
	TDataSetRow	DataSetIndex;
	CBitMemStream BMS;
//	uint16 FrontendId;
};


//-----------------------------------------------
//	cbStaticString :
//
//-----------------------------------------------
static void cbStaticString(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{

	// read receiver id
	CEntityId receiver;
	set<CEntityId> excluded;
	string strId;

	msgin.serial( receiver );
	msgin.serialCont( excluded );
	msgin.serial( strId );

	nlwarning("cbStaticString : service %u(%s) use deprecated phrase system. Trying to output string '%s'."
		, serviceId.get(), serviceName.c_str(),
		strId.c_str());
	return;
} // cbStaticString //



//-----------------------------------------------
//	cbChatMessage :
//
//-----------------------------------------------
static void cbChatMessage(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TDataSetRow entityId;
	string str;

	try
	{
		msgin.serial(entityId);
		msgin.serial(str);
	}
	catch(const Exception &e)
	{
		nlwarning("<cbChatMessage> %s",e.what());
		return;
	}

	try
	{
		CChatGroup::TGroupType oldMode = IOS->getChatManager().getClient(entityId).getChatMode();
		if ( oldMode != CChatGroup::say )
			IOS->getChatManager().getClient(entityId).setChatMode(CChatGroup::say);
		
		IOS->getChatManager().getClient(entityId).updateAudience();
		IOS->getChatManager().chat( entityId, str );

		if ( oldMode != CChatGroup::say )
			IOS->getChatManager().getClient(entityId).setChatMode(oldMode);
	}
	catch(const Exception &e)
	{
		nlwarning("<cbChatMessage> %s",e.what());
	}
} // cbChatMessage //


//-----------------------------------------------
//	cbRemoveEntity :
//
//-----------------------------------------------
/*static void cbRemoveEntity(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	// read entity id
	CEntityId entityId;
	try
	{
		msgin.serial( entityId );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbRemoveEntity> %s",e.what());
		return;
	}

	// remove entity
	IOS->removeEntity( entityId );

	nldebug("<cbRemoveEntity> Removing entity %s",entityId.toString().c_str());
}*/


//-----------------------------------------------
//	cbCharacterName :
//
//-----------------------------------------------
static void cbCharacterName(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TDataSetRow chId;
	ucstring name;

	try
	{
		// read character id
		msgin.serial( chId );

		// character's name
		msgin.serial( name );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbCharacterName> %s",e.what());
		return;
	}

	IOS->addCharacterName( chId, name, TSessionId(0) );

} // cbCharacterName //

//-----------------------------------------------
//	cbCharacterNameId :
//
//-----------------------------------------------
static void cbCharacterNameId(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TDataSetRow chId;
	uint32 stringId;
	try
	{
		// read character id
		msgin.serial( chId );
		
		// character's string Id
		msgin.serial( stringId );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbCharacterName> %s",e.what());
		return;
	}
	IOS->addCharacterName( chId, SM->getString( stringId ), TSessionId(0) );
	
} // cbCharacterNameId //

//-----------------------------------------------
//	cbCharacterNameAndLang :
//
//-----------------------------------------------
static void cbCharacterNameAndLang(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TDataSetRow chId;
	ucstring name;
	TSessionId sessionId;
	string language;
	std::vector<NLMISC::CEntityId> ignoreList;
	bool havePrivilege;
	try
	{
		// read character id
		msgin.serial( chId );

		// character's name
		msgin.serial( name );

		// character home mainland session
		msgin.serial( sessionId  );

		// language
		msgin.serial(language);

		// ignoreList
		msgin.serialCont(ignoreList);

		// privilege
		msgin.serial( havePrivilege );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbCharacterName> %s",e.what());
		return;
	}

	IOS->addCharacterName( chId, name, sessionId );
	CCharacterInfos *ci = IOS->getCharInfos(TheDataset.getEntityId(chId), false);
	if (ci)
	{
		ci->Language = SM->checkLanguageCode(language);
		ci->HavePrivilege = havePrivilege;
		IOS->getChatManager().getClient(chId).setIgnoreList(ignoreList);
	}

} // cbCharacterNameAndLang //

//-----------------------------------------------
//	cbCharacterEventFaction :
//
//-----------------------------------------------
static void cbCharacterEventFaction(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TDataSetRow chId;
	string eventFaction;
	try
	{
		// read character id
		msgin.serial( chId );

		// event faction
		msgin.serial( eventFaction );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbCharacterEventFaction> %s", e.what());
		return;
	}

	CCharacterInfos *ci = IOS->getCharInfos(TheDataset.getEntityId(chId), false);
	if ( ci && TheDataset.isAccessible(chId) )
	{
		if (eventFaction.empty())
			ci->UntranslatedEventFactionId = 0;
		else
			ci->UntranslatedEventFactionId = SM->storeString( eventFaction );

//		CMirrorPropValue<TYPE_EVENT_FACTION_ID> propEventFactionId( TheDataset, chId, DSPropertyEVENT_FACTION_ID );
//		propEventFactionId = SM->translateEventFaction( ci->UntranslatedEventFactionId );
	}
	else
	{
		nlwarning("cbCharacterEventFaction: character has not been registered yet in IOS!");
	}

} // cbCharacterEventFaction //


//-----------------------------------------------
//	cbAddGroup :
//
//-----------------------------------------------
static void cbAddGroup( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TGroupId gId;
	uint8 gType;

	try
	{
		msgin.serial( gId );
		msgin.serial( gType );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbAddGroup> %s",e.what());
		return;
	}

	IOS->getChatManager().addGroup( gId, (CChatGroup::TGroupType)gType, "" );

	if (VerboseChatManagement)
		nldebug("IOS: cbAddGroup Adding group %s with type '%s'",
			gId.toString().c_str(),
			CChatGroup::groupTypeToString((CChatGroup::TGroupType)gType).c_str());

} // cbAddGroup //

//-----------------------------------------------
//	cbAddNamedGroup :
//
//-----------------------------------------------
static void cbAddNamedGroup( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TGroupId gId;
	uint8 gType;
	string name;

	try
	{
		msgin.serial( gId );
		msgin.serial( gType );
		msgin.serial( name );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbAddGroup> %s",e.what());
		return;
	}

	IOS->getChatManager().addGroup( gId, (CChatGroup::TGroupType)gType, name );

	if (VerboseChatManagement)
		nldebug("IOS: cbAddGroup Adding named group %s with type '%s' as '%s'", 
					gId.toString().c_str(),
					CChatGroup::groupTypeToString((CChatGroup::TGroupType)gType).c_str(),
					name.c_str());

} // cbAddGroup //

//-----------------------------------------------
//	cbRemoveGroup :
//
//-----------------------------------------------
static void cbRemoveGroup( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TGroupId gId;

	try
	{
		msgin.serial( gId );
	}
	catch(const Exception &e)
	{
		nlwarning("<cbRemoveGroup> %s",e.what());
		return;
	}

	IOS->getChatManager().removeGroup( gId );

	nldebug("IOS: cbRemoveGroup Removing group %s",gId.toString().c_str());

} // cbRemoveGroup //


//-----------------------------------------------
//	cbAddToGroup :
//
//-----------------------------------------------
static void cbAddToGroup( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TGroupId gId;
	CEntityId charId;

	try
	{
		msgin.serial( gId );
		msgin.serial( charId );
	}
	catch(const Exception& e)
	{
		nlwarning("<cbAddToGroup> %s",e.what());
		return;
	}

	TDataSetRow charRow = TheDataset.getDataSetRow(charId);

	if(charRow.getIndex() == 0xffffff)
	{
		nlwarning("id.getIndex() == 0xffffff for entity %s, skip it", charId.toString().c_str());
		return;
	}

	// if not already register in chat manager, add it
	if (!IOS->getChatManager().checkClient(charRow))
		IOS->getChatManager().addClient( charRow );
	//IOS->getChatManager().getClient(charId).setChatMode(say);

	IOS->getChatManager().addToGroup( gId, charRow );

	nldebug("IOS: cbAddToGroup Adding %s  to group %s",charId.toString().c_str(), gId.toString().c_str());

} // cbAddToGroup //


//-----------------------------------------------
//	cbRemoveFromGroup :
//
//-----------------------------------------------
static void cbRemoveFromGroup( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TGroupId gId;
	CEntityId charId;

	try
	{
		msgin.serial( gId );
		msgin.serial( charId );
	}
	catch(const Exception& e)
	{
		nlwarning("<cbRemoveFromGroup> %s",e.what());
		return;
	}

	TDataSetRow	charRow = TheDataset.getDataSetRow(charId);

	if (charRow.isValid())
	{
		IOS->getChatManager().removeFromGroup( gId, charRow );

		nldebug("IOS: cbAddToGroup Removing %s:%x from group %s",
			charId.toString().c_str(),
			charRow.getIndex(),
			gId.toString().c_str());
	}

} // cbRemoveFromGroup //



//-----------------------------------------------
//	cbSendDynamicId :
//
//-----------------------------------------------
//static void cbSendDynamicId( CMessage& msgin, const string &serviceName, TServiceId serviceId )
//{
//	CEntityId clientId;
//	CEntityId entityId;
//
//	try
//	{
//		msgin.serial( clientId );
//		msgin.serial( entityId );
//	}
//	catch(const Exception& e)
//	{
//		nlwarning("<cbSendDynamicId> %s",e.what());
//		return;
//	}
//
//	CCharacterInfos * clientInfos = IOS->getCharInfos( clientId );
//	CCharacterInfos * entityInfos = IOS->getCharInfos( entityId );
//	if( clientInfos != NULL )
//	{
//		if( entityInfos != NULL )
//		{
//			try
//			{
//				if( IOS->getChatManager().getClient(clientInfos->DataSetIndex).knowString(entityInfos->OldNameIndex) == false ) 
//				{
//					IOS->getChatManager().addDynStr( clientId, entityInfos->OldNameIndex, serviceId );
//				}
//			}
//			catch(const CChatManager::EChatClient& e)
//			{
//				nlwarning("<cbSendDynamicId> %s",e.what());
//			}
//		}
//		else
//		{
//			nlwarning("<cbSendDynamicId> Infos about the other character %s not found",entityId.toString().c_str());
//		}
//	}
//	else
//	{
//		nlwarning("<cbSendDynamicId> Infos about the receiver %s not found",clientId.toString().c_str());
//	}
//
//} // cbSendDynamicId //


//-----------------------------------------------
//	cbSysChat :
//
//-----------------------------------------------
void cbSysChat( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	bool		talkToUser;
	CEntityId	talkTo;
	string		chatString;

	msgin.serial(talkToUser);
	msgin.serial(talkTo);
	msgin.serial(chatString);

	if (talkToUser)
	{
		IOS->getChatManager().sendChat(CChatGroup::tell, TheDataset.getDataSetRow(talkTo), ucstring("{no_bubble}<"+serviceName+":"+toString(serviceId.get())+"> "+chatString));
	}
	else
	{
		IOS->getChatManager().chatInGroup(talkTo, ucstring("{no_bubble}<"+serviceName+":"+toString(serviceId.get())+"> ")+chatString, TDataSetRow());
	}

} // cbSysChat //


//-----------------------------------------------
//	cbNpcTell :
//
//-----------------------------------------------
void cbNpcTell( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	TDataSetRow				receiver;
	string					phraseId;

	msgin.serial(sender);
	msgin.serial(receiver);
	msgin.serial(phraseId);

	CChatManager &cm = IOS->getChatManager();

	try
	{
//		TDataSetRow dsr = TheDataset.getDataSetRow(sender);
		if (sender == INVALID_DATASET_ROW || !sender.isValid())
		{
			nlwarning("cbNpcTell : ignoring chat because NPC %u not yet in mirror", sender.getIndex()/*.toString().c_str()*/);
			return;
		}
//		CChatClient &client = cm.getClient(sender);
		cm.sendChat2(CChatGroup::tell, receiver, phraseId, sender);
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("cbNpcTell : ignoring chat because NPC info not available yet");
	}
}


//-----------------------------------------------
//	cbGhostTell :
//
//-----------------------------------------------
void cbGhostTell( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				receiver;
	string					phraseId;

	msgin.serial(receiver);
	msgin.serial(phraseId);

	CChatManager &cm = IOS->getChatManager();

	cm.sendChat2(CChatGroup::tell, receiver, phraseId);
}

//-----------------------------------------------
//	cbNpcTellEx :
//
//-----------------------------------------------
void cbNpcTellEx( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	TDataSetRow				receiver;
	uint32					phraseId;
	
	msgin.serial(sender);
	msgin.serial(receiver);
	msgin.serial(phraseId);
	
	CChatManager &cm = IOS->getChatManager();
	
	try
	{
//		TDataSetRow dsr = TheDataset.getDataSetRow(sender);
		if (sender == INVALID_DATASET_ROW || !sender.isValid())
		{
			nlwarning("cbNpcTell : ignoring chat because NPC %u not valid", sender.getIndex()/*.toString().c_str()*/);
			return;
		}
		//		CChatClient &client = cm.getClient(sender);
		cm.sendChat2Ex(CChatGroup::tell, receiver, phraseId, sender);
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("cbNpcTell : ignoring chat because NPC info not available yet");
	}
}


//-----------------------------------------------
//	cbNpcChatEx :
//
//-----------------------------------------------
void cbNpcChatEx( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	CChatGroup::TGroupType	type = CChatGroup::nbChatMode;
	uint32					phraseId;
	
	msgin.serial(sender);
	msgin.serialEnum(type);
	msgin.serial(phraseId);
	
	CChatManager &cm = IOS->getChatManager();
	
	try
	{
	//	TDataSetRow dsr = TheDataset.getDataSetRow(sender);
		if (sender == INVALID_DATASET_ROW)
		{
			nlwarning("cbNpcChatEx : ignoring chat because NPC %s:%x Invalid", 
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
		CChatClient &client = cm.getClient(sender);
		client.setChatMode(type);
		client.updateAudience();
		cm.chat2Ex(sender, phraseId);
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("cbNpcChatEx : ignoring chat because NPC info not available yet");
	}
}


//-----------------------------------------------
//	cbNpcChat :
//
//-----------------------------------------------
void cbNpcChat( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	CChatGroup::TGroupType	type = CChatGroup::nbChatMode;
	string					phraseId;

	msgin.serial(sender);
	msgin.serialEnum(type);
	msgin.serial(phraseId);

	CChatManager &cm = IOS->getChatManager();

	try
	{
//		TDataSetRow dsr = TheDataset.getDataSetRow(sender);
		if ( !sender.isValid() || !TheDataset.isDataSetRowStillValid(sender) )
		{
			nlwarning("cbNpcChat : ignoring chat because NPC %x not yet/not anymore in mirror", 
				sender.getIndex());
			return;
		}
		CChatClient &client = cm.getClient(sender);
		client.setChatMode(type);
		client.updateAudience();
		cm.chat2(sender, phraseId);
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("cbNpcChat : ignoring chat because NPC info not available yet");
	}
}

//-----------------------------------------------
//	cbNpcChatDyn :
//
//-----------------------------------------------
void cbNpcChatParam( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	TDataSetRow				sender;
	CChatGroup::TGroupType	type = CChatGroup::nbChatMode;
	string					phraseId;
	std::vector<STRING_MANAGER::TParam> params;

	msgin.serial(sender);
	msgin.serialEnum(type);
	msgin.serial(phraseId);

	uint32 size;
	msgin.serial(size);
	params.resize(size);
	for ( uint i = 0; i < size; i++ )
	{
		uint8 type8;
		msgin.serial( type8 );
		params[i].serialParam( false, msgin, (STRING_MANAGER::TParamType) type8 );
	}

	CChatManager &cm = IOS->getChatManager();

	try
	{
//		TDataSetRow dsr = TheDataset.getDataSetRow(sender);
		if ( !sender.isValid() || !TheDataset.isDataSetRowStillValid(sender) )
		{
			nlwarning("cbNpcChat : ignoring chat because NPC %x not yet/not anymore in mirror", 
				sender.getIndex());
			return;
		}
		CChatClient &client = cm.getClient(sender);
		client.setChatMode(type);
		client.updateAudience();
		cm.chatParam(sender, phraseId, params);
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("cbNpcChat : ignoring chat because NPC info not available yet");
	}
}


//-----------------------------------------------
//	cbNpcChatSentence :
//
//-----------------------------------------------
void cbNpcChatSentence( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	CChatGroup::TGroupType	type = CChatGroup::nbChatMode;
	ucstring					sentence;

	msgin.serial(sender);
	msgin.serialEnum(type);
	msgin.serial(sentence);

	CChatManager &cm = IOS->getChatManager();

	try
	{
//		TDataSetRow dsr = TheDataset.getDataSetRow(sender);
		if ( !sender.isValid() || !TheDataset.isDataSetRowStillValid(sender) )
		{
			nlwarning("cbNpcChatSentence : ignoring chat because NPC %x not yet/not anymore in mirror", 
				sender.getIndex());
			return;
		}
		CChatClient &client = cm.getClient(sender);
		client.setChatMode(type);
		client.updateAudience();
		cm.chat(sender, sentence);
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("cbNpcChatSentence : ignoring chat because NPC info not available yet");
	}
}

//-----------------------------------------------
//	cbNpcChatSentenceEx :
//
//-----------------------------------------------
void cbNpcChatSentenceEx( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	ucstring name;
	ucstring sentence;
	TChanID id;
	TDataSetRow sender;
	msgin.serial(id);
	msgin.serial(name);
	msgin.serial(sentence);
	if(id.isUnknownId())
	{
		nlwarning("bad channel id : %s unable to chat! ",id.toString().c_str());
		return;
	}
	CCharacterInfos* cInfo = IOS->getCharInfos(name);
	sender = cInfo->DataSetIndex;
	if(sender.isValid())
	{
		CChatManager &cm = IOS->getChatManager();
		CChatClient &client = cm.getClient(sender);
		client.setChatMode(CChatGroup::dyn_chat,id);
		client.updateAudience();
		cm.chat(sender,sentence);
	}
	else
	{
		nlwarning("sender %s:%x invalid! unable to chat!",name.toString().c_str(),sender.getIndex());
	}
}

void cbNpcChatSentenceChannel( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	ucstring sentence;
	TChanID id;
	TDataSetRow sender;
	msgin.serial(id);
	msgin.serial(sender);
	msgin.serial(sentence);
	
	if(sender.isValid())
	{
		CChatManager &cm = IOS->getChatManager();
		CChatClient &client = cm.getClient(sender);
		client.setChatMode(CChatGroup::dyn_chat,id);
		client.updateAudience();
		cm.chat(sender,sentence);
		nlwarning("%s said %s !",sender.toString().c_str(),sentence.toString().c_str());
	}else
	{
		nlwarning("sender %x invalid! unable to chat!",sender.getIndex());
	}
}


//-----------------------------------------------
//	cbGroupDynString :
//
//-----------------------------------------------
void cbGroupDynString( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	vector<CEntityId>		excluded;
	uint32					phraseId;

	msgin.serial(sender);
	msgin.serialCont(excluded);
	msgin.serial(phraseId);

	CChatManager &cm = IOS->getChatManager();

	try
	{
//		TDataSetRow dsr = TheDataset.getDataSetRow(sender);
		if (sender == INVALID_DATASET_ROW || !sender.isValid())
		{
			nlwarning("cbGroupDynString : ignoring group message because entity %x not yet in mirror", sender.getIndex());
			return;
		}
		
		list<CReceiver> receiverList;

		CChatClient &client = cm.getClient(sender);
		client.updateAudience();
		CChatGroup group = client.getSayAudience();

		CChatGroup::TMemberCont::iterator itM;
		for( itM = group.Members.begin(); itM != group.Members.end(); ++itM )
		{
			const CEntityId &id = TheDataset.getEntityId(*itM); 

			// skip it if excluded
			if ( std::find(excluded.begin(), excluded.end(), id) != excluded.end() )
				continue;

			// send message to client front end
			CMessage msgout( "IMPULSION_ID" );
			msgout.serial( const_cast<CEntityId&> (id) );
			CBitMemStream bms;
			if ( ! GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:DYN_STRING", bms) )
			{
				nlwarning("Msg name CHAT:DYN_STRING not found");
			}
			else
			{
				bms.serial( phraseId );
				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				CUnifiedNetwork::getInstance()->send( TServiceId(id.getDynamicId()), msgout );
			}
		}	
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("cbGroupDynString : ignoring group message because entity info not available yet");
	}
} // cbGroupDynString //

void cbPhrasePrepare( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	SM->receivePhrase(msgin, false, serviceName);
}

void cbPhrasePrepareDebug( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	SM->receivePhrase(msgin, true, serviceName);
}

void cbUserPhrasePrepare( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	SM->receiveUserPhrase(msgin, false);
}

void cbUserPhrasePrepareDebug( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	SM->receiveUserPhrase(msgin, true);
}

void cbBroadcastSystemPhrase( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	SM->broadcastSystemMessage(msgin, false);
}

void cbBroadcastSystemPhraseDebug( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	SM->broadcastSystemMessage(msgin, true);
}

// message STORE_STRING
void cbStoreString( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	ucstring str;
	msgin.serial(str);
	uint32 stringId = SM->storeString(str);

	CMessage msgOut("STORE_STRING_RESULT");
	msgOut.serial(str);
	msgOut.serial(stringId);

	CUnifiedNetwork::getInstance()->send(serviceId, msgOut);
}

void cbGetString(CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	uint32 nameIndex;
	msgin.serial( nameIndex );
	SM->sendString( nameIndex, serviceId );
}

void cbRetrieveEntityNames( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	SM->retrieveEntityNames( serviceId );
}

void cbUserLanguage( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	uint32 userId;
	TServiceId frontEndId;
	string lang;
	msgin.serial( userId );
	msgin.serial( frontEndId );
	msgin.serial( lang );
	SM->updateUserLanguage( userId,frontEndId, lang );

	// Note: removeUserLanguage() is never called, currently
}

void cbRemoveUserLanguage( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	uint32 userId;
	msgin.serial( userId );
	SM->removeUserLanguage( userId );
}

void cbIgnoreTellMode( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	NLMISC::CEntityId userId;
	bool mode;
	msgin.serial( userId );
	msgin.serial( mode );
	CChatManager &cm = IOS->getChatManager();
	if ( mode )
		cm.addUserIgnoringTells( userId );
	else
		cm.removeUserIgnoringTells( userId );
}

void cbMutePlayer( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	NLMISC::CEntityId userId;
	bool mode;
	msgin.serial( userId );
	msgin.serial( mode );
	CChatManager &cm = IOS->getChatManager();
	if ( mode )
		cm.addMutedUser( userId );
	else
		cm.removeMutedUser( userId );
}

void cbUniverseMode( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	NLMISC::CEntityId userId;
	bool mode;
	msgin.serial( userId );
	msgin.serial( mode );

	CChatGroup::TGroupType type = (mode)?CChatGroup::universe : CChatGroup::say;
	CCharacterInfos * charInfos = IOS->getCharInfos( userId );
	if( charInfos != NULL )
	{
		try
		{
			IOS->getChatManager().getClient(TheDataset.getDataSetRow(userId)).setChatMode( type );
		}
		catch(const Exception& e)
		{
			nlwarning("<impulsionChatMode> %s",e.what());
		}
	}
	else
	{
		nlwarning("<impulsionChatMode> The character %s doesn't have infos",userId.toString().c_str());
	}
}

void cbMuteUniverse( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	NLMISC::CEntityId userId;
	bool mode;
	msgin.serial( userId );
	msgin.serial( mode );
	CChatManager &cm = IOS->getChatManager();
	if ( mode )
		cm.addUniverseMutedUser(userId);
	else
		cm.removeUniverseMutedUser(userId);
}

//-----------------------------------------------
//	cbEmoteCrowd : a crowd receive an emote ( e.g. : everybody except the main target )
//
//-----------------------------------------------
void cbEmoteCrowd( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	string					phraseTextId;
	TVectorParamCheck params;
	vector<TDataSetRow>		ignored;
	
	CChatManager &cm = IOS->getChatManager();
	
	try
	{
		msgin.serial(sender);
		msgin.serial(phraseTextId);
		uint32 size;
		msgin.serial(size);
		params.resize(size);
		for ( uint i = 0; i < size; i++ )
		{
			uint8 type8;
			msgin.serial( type8 );
			params[i].serialParam( false, msgin, (STRING_MANAGER::TParamType) type8 );
		}
		msgin.serialCont( ignored );

		// get the plyer
		if ( sender == TDataSetRow::createFromRawIndex(INVALID_DATASET_ROW) )
		{
			nlwarning("cbNpcChat : ignoring emote because PC %x Invalid", sender.getIndex());
			return;
		}
		// send the phrase
		cm.sendEmoteTextToAudience(sender, phraseTextId, params,ignored);
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("cbNpcChatEx : ignoring chat because NPC info not available yet");
	}
}


//-----------------------------------------------
//	cbEmoteSolePlayer : a unique receive an emote ( e.g. : the main target )
//
//-----------------------------------------------
void cbEmoteSolePlayer( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	TDataSetRow				target;
	uint32					phraseId;
	msgin.serial(sender);
	msgin.serial(target);
	msgin.serial(phraseId);
	CChatManager &cm = IOS->getChatManager();
	cm.sendEmoteTextToPlayer(sender, target, phraseId );
}


//-----------------------------------------------
//		cbCustomEmote
//-----------------------------------------------
void cbCustomEmote( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	TDataSetRow				sender;
	ucstring				emoteCustomText;
	
	CChatManager &cm = IOS->getChatManager();
	
	try
	{
		msgin.serial(sender);
		msgin.serial(emoteCustomText);

		// filter emote text
		emoteCustomText = IOS->getChatManager().filterClientInputColorCode(emoteCustomText);
		
		// get the player
		if ( sender == TDataSetRow::createFromRawIndex(INVALID_DATASET_ROW) )
		{
			nlwarning("<cbCustomEmote> ignoring emote because PC %x Invalid", sender.getIndex());
			return;
		}
		// send the phrase
		cm.sendEmoteCustomTextToAll(sender, emoteCustomText);
	}
	catch(const CChatManager::EChatClient &)
	{
		nlwarning("<cbCustomEmote> exception, ignoring emote");
	}

} // cbCustomEmote //



//-----------------------------------------------
//	cbSetPhrase : change the content of a phrase
//
//-----------------------------------------------
void cbSetPhrase( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	SM->setPhrase(msgin);
}

void cbSetPhraseLang( CMessage& msgin, const string& serviceName, TServiceId serviceId )
{
	SM->setPhraseLang(msgin);
}

//-----------------------------------------------
//	Add a new channel in the dyn chat
//
//-----------------------------------------------
void cbDynChatAddChan(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID chanID;
	bool noBroadcast;
	bool forwardInput;
	bool unify;

	msgin.serial(chanID);
	msgin.serial(noBroadcast);
	msgin.serial(forwardInput);
	msgin.serial(unify);
	nlinfo("cbDynChatAddChan: add channel");
	bool res = IOS->getChatManager().getDynChat().addChan(chanID, noBroadcast, forwardInput, unify);
	if (!res) nlwarning("Couldn't add chan %s", chanID.toString().c_str());
	else nlinfo("cbDynChatAddChan: add channel %s",chanID.toString().c_str());
}

//-----------------------------------------------
//	Remove a channel from the dyn chat
//
//-----------------------------------------------
void cbDynChatRemoveChan(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID chanID;
	msgin.serial(chanID);
	bool res = IOS->getChatManager().getDynChat().removeChan(chanID);
	if (!res) nlwarning("Couldn't remove chan %s", chanID.toString().c_str());
}


//-----------------------------------------------
//	Add a session to the dyn chat
//
//-----------------------------------------------
void addSession(TDataSetRow client,TChanID chan,bool writeRight)
{
	{
		if (!IOS->getChatManager().checkClient(client))
			IOS->getChatManager().addClient( client );
	}
	CDynChatSession *session = IOS->getChatManager().getDynChat().addSession(chan, client);
	if (!session) 
	{
		nlwarning("Couldn't create session");
		return;
	}

	session->WriteRight = writeRight;
	// send channel historic to client
	IOS->getChatManager().sendHistoric(client, chan);
	nldebug("IOS: session added %s:%x!! on channel %s",TheDataset.getEntityId(client).toString().c_str(),client.getIndex(),chan.toString().c_str());
}

void cbDynChatAddSession(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID		chan;
	TDataSetRow client;
	bool        writeRight;
	msgin.serial(chan);
	msgin.serial(client);
	msgin.serial(writeRight);
	addSession(client,chan,writeRight);
	/*
	{
		if (!IOS->getChatManager().checkClient(client))
			IOS->getChatManager().addClient( client );
	}
	CDynChatSession *session = IOS->getChatManager().getDynChat().addSession(chan, client);
	if (!session) 
	{
		nlwarning("Couldn't create session");
		return;
	}

	session->WriteRight = writeRight;
	// send channel historic to client
	IOS->getChatManager().sendHistoric(client, chan);
	nlwarning("session added %s:%x!! on channel %s",TheDataset.getEntityId(client).toString().c_str(),client.getIndex(),chan.toString().c_str());
	*/
}

//-----------------------------------------------
//	Add a session to the dyn chat, using the name of a npc
//  TODO: factorize some code with cbDynChatAddSession
//-----------------------------------------------
void cbDynChatAddSessionWithName(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID	chan;
	TDataSetRow client;
	ucstring clientName;
	bool writeRight;
	msgin.serial(chan);
	msgin.serial(clientName);
	msgin.serial(writeRight);
	
	if(chan.isUnknownId())
	{
		nlwarning("unknown channel id! unable to add session!");
		return;
	}
	CCharacterInfos* cInfo = IOS->getCharInfos(clientName);
	client = cInfo->DataSetIndex;
	if(client.isValid())
	{
		addSession(client,chan,writeRight);
		/*
		if (!IOS->getChatManager().checkClient(client))
			IOS->getChatManager().addClient( client );

		CDynChatSession *session = IOS->getChatManager().getDynChat().addSession(chan, client);
		if (!session) 
		{
			nlwarning("Couldn't create session");
			return;
		}

		session->WriteRight = writeRight;
		// send channel historic to client
		IOS->getChatManager().sendHistoric(client, chan);
		*/
	}
	else
	{
		nlwarning("sender %s:%x invalid! unable to add session!",clientName.c_str(),client.getIndex());
	}
}
//-----------------------------------------------
//	Remove a session from the dyn chat
//
//-----------------------------------------------
void cbDynChatRemoveSession(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID		chan;
	TDataSetRow client;	
	msgin.serial(chan);
	msgin.serial(client);	
	bool res = IOS->getChatManager().getDynChat().removeSession(chan, client);
	if (!res) nlwarning("Couldn't remove session");
}

//-----------------------------------------------
//	Remove a session from the dyn chat, using the name of the npc
//  TODO: factorize some code with cbDynChatRemoveSession
//-----------------------------------------------
void cbDynChatRemoveSessionWithName(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID		chan;
	ucstring	clientName;
	TDataSetRow client;	
	msgin.serial(chan);
	msgin.serial(clientName);
	CCharacterInfos* cInfo = IOS->getCharInfos(clientName);
	client = cInfo->DataSetIndex;
	if(client.isValid())
	{
		/*{
			CMessage msgout("DYN_CHAT:REMOVE_SESSION");
			msgout.serial(chan);
			msgout.serial(client);
			CUnifiedNetwork::getInstance()->send("EGS",msgout);
			return;
		}*/
		bool res = IOS->getChatManager().getDynChat().removeSession(chan, client);
		if (!res) nlwarning("Couldn't remove session");	
	}
	
}

//-----------------------------------------------
//	Set read only flag for a dyn chat session
//
//-----------------------------------------------
void cbDynChatSetWriteRight(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID		chan;
	TDataSetRow client;	
	bool		writeRight;
	msgin.serial(chan);
	msgin.serial(client);	
	msgin.serial(writeRight);
	CDynChatSession *session = IOS->getChatManager().getDynChat().getSession(chan, client);
	if (!session)
	{
		nlwarning("Unknown session");
		return;
	}
	session->WriteRight = writeRight;
}

//-----------------------------------------------
//	Set the historic size for a channel
//-----------------------------------------------
void cbDynChatSetHistoricSize(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID		chanID;
	uint32		historicSize;
	msgin.serial(chanID);
	msgin.serial(historicSize);	
	CDynChatChan *chan = IOS->getChatManager().getDynChat().getChan(chanID);
	if (!chan)
	{
		nlwarning("Unknown chan");
		return;
	}
	chan->Historic.setMaxSize(historicSize);
}
void cbDynChatSetHideBubble(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID		chanID;
	bool hideBubble;
	
	msgin.serial(chanID);
	msgin.serial(hideBubble);
	
	CChatManager &cm = IOS->getChatManager();
	CDynChatChan *chan = cm.getDynChat().getChan(chanID);
	if (!chan)
	{
		nlwarning("Unknown chan");
		return;
	}
	chan->HideBubble = hideBubble;
}



void cbDynChatServiceChat(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID		chanID;
	ucstring	text;
	ucstring	senderName;

	msgin.serial(chanID);
	msgin.serial(senderName);
	msgin.serial(text);

	CChatManager &cm = IOS->getChatManager();
	CDynChatChan *chan = cm.getDynChat().getChan(chanID);
	if (!chan)
	{
		nlwarning("Unknown chan");
		return;
	}

	// add msg to the historic
	CDynChatChan::CHistoricEntry entry;
	entry.String = text;
	entry.SenderString = senderName;
	chan->Historic.push(entry);

	// send the message to all the player in this chat
	CDynChatSession *dcc = chan->getFirstSession();
	if(!dcc) nlwarning(("nobody hears on channel "+chanID.toString()).c_str());

	while (dcc)
	{		
		cm.sendChat(CChatGroup::dyn_chat, dcc->getClient()->getID(), text, TDataSetRow(), chanID, senderName);
		dcc = dcc->getNextChannelSession(); // next session in this channel
	}						
}

void cbDynChatServiceTell(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	TChanID		chanID;
	ucstring	text;
	TDataSetRow	player;
	ucstring	senderName;

	msgin.serial(chanID);
	msgin.serial(senderName);
	msgin.serial(player);
	msgin.serial(text);

	CChatManager &cm = IOS->getChatManager();
	CDynChatChan *chan = cm.getDynChat().getChan(chanID);
	if (!chan)
	{
		nlwarning("Unknown chan");
		return;
	}

	// NB: we don't put 'tell' text in channel history

	// send the message to the designated player in this chat
	CDynChatSession *dcc = chan->getFirstSession();
	while (dcc)
	{
		if (dcc->getClient()->getID() == player)
		{
			cm.sendChat(CChatGroup::dyn_chat, dcc->getClient()->getID(), text, TDataSetRow(), chanID, senderName);
			dcc = dcc->getNextChannelSession(); // next session in this channel
			// no more needed to continue
			break;
		}
		dcc = dcc->getNextChannelSession(); // next session in this channel
	}						
}

//-----------------------------------------------
//	Reset the dyn chat (all channel & sessions are removed)
//
//-----------------------------------------------
void cbDynChatReset(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	IOS->getChatManager().getDynChat().removeAllChannels();
}

//-----------------------------------------------
//	update the alias list 
//
//-----------------------------------------------
void cbUpdateAIAlias(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{

	enum {Set=0, Add = 1, Delete=2  };

	uint32 subcommand;
	msgin.serial(subcommand);

	
	switch(subcommand)
	{	

	case Set:
		IOS->getAIAliasManager().clear();

	case Add:
		{
		
			uint32 size;
			msgin.serial(size);
			uint first = 0;
			uint last = size;
			for ( ;first != last; ++first)
			{				
				uint32 alias;
				std::string name;
				msgin.serial(alias);
				msgin.serial(name);
				IOS->getAIAliasManager().add(alias, name);
			}
		}
		break;
			
		/*
	case Add: break;
	case Delete: break;
	*/
	default:
		nlwarning("<updateAlias> invalid message");
		return;
	}

}

void cbRequestDsr(CMessage& msgin, const string &serviceName, TServiceId serviceId)
{
	//CMessage msg("DSR_VALUE");
	ucstring name;
	TDataSetRow client;
	msgin.serial(name);
	CCharacterInfos* cInfo = IOS->getCharInfos(name);
	if(!cInfo)return;
	client = cInfo->DataSetIndex;
	if(client.isValid())
	{
		CMessage msg("DSR_VALUE");
		nlwarning("dsr value sent! %s = %s",name.toString().c_str(),client.toString().c_str());
		msg.serial(name);
		msg.serial(client);
		CUnifiedNetwork::getInstance()->send(serviceName,msg);
	}
}

//void cbAddDM(CMessage& msgin, const string &serviceName, TServiceId serviceId)
//{
//	uint32 charId;
//	msgin.serial(charId);
//
//	// rebuild the entity id
//	CEntityId eid(RYZOMID::player, charId, 0, 0);
//	// retrieve the datasetrow
//	TDataSetRow rowId = TheDataset.getDataSetRow(eid);
//	if (rowId.isValid())
//	{
//		// we need to add it in the universe chat
//		IOS->getChatManager().addToGroup(TGroupId(RYZOMID::chatGroup, 0), rowId);
//	}
//}
//
//void cbRemoveDM(CMessage& msgin, const string &serviceName, TServiceId serviceId)
//{
//	uint32 charId;
//	msgin.serial(charId);
//
//	// rebuild the entity id
//	CEntityId eid(RYZOMID::player, charId, 0, 0);
//	// retrieve the datasetrow
//	TDataSetRow rowId = TheDataset.getDataSetRow(eid);
//	if (rowId.isValid())
//	{
//		// we need to remove it from the universe chat
//		IOS->getChatManager().removeFromGroup(TGroupId(RYZOMID::chatGroup, 0), rowId);
//	}
//}


//----------------------------
//	CbArray
//----------------------------
TUnifiedCallbackItem CbIOSArray[]=
{
	// Received from clients (via FS)
//	{ "READY_STRING", cbImpulsionReadyString },
	{ "CLIENT:STRING:CHAT", cbImpulsionChat },
	{ "CLIENT:STRING:CHAT_TEAM", cbImpulsionChatTeam },
	{ "CLIENT:STRING:TELL", cbImpulsionTell },	
	{ "CLIENT:STRING:FILTER", cbImpulsionFilter },
	{ "CLIENT:STRING:CHAT_MODE", cbImpulsionChatMode },
	{ "CLIENT:STRING:AFK_TXT", cbImpulsionAfkTxt },
//	{ "STRING_RQ_ID", cbImpulsionStringRqId },
	{ "STRING_RQ_UID", cbImpulsionStringRqUid },

	// Received from the back-end
	{ "STATIC_STRING", cbStaticString },
	{ "CHAT_MESSAGE", cbChatMessage	},
	{ "CHARACTER_NAME",	cbCharacterName },				// receive a character name
	{ "CHARACTER_NAME_ID",	cbCharacterNameId },		// receive a character name with a string manager string Id
	{ "CHARACTER_NAME_LANG", cbCharacterNameAndLang },	// receive a character name and a language code (for player)
	{ "CHARACTER_EVENT_FACTION", cbCharacterEventFaction },
	{ "ADD_GROUP", cbAddGroup },
	{ "ADD_NAMED_GROUP", cbAddNamedGroup },
	{ "REMOVE_GROUP", cbRemoveGroup },
	{ "ADD_TO_GROUP", cbAddToGroup },
	{ "REMOVE_FROM_GROUP", cbRemoveFromGroup },
//	{ "SEND_DYNAMIC_ID", cbSendDynamicId },
	{ "CHAT", cbSysChat },					// talk to a user or a whole group
	
	{ "NPC_CHAT", cbNpcChat },				// Npc talk to a chat group	
	{ "NPC_CHAT_PARAM", cbNpcChatParam },				// Npc talk to a chat group	 (with parameter)

	{ "NPC_TELL", cbNpcTell },				// Npc tell to a player
	{ "GHOST_TELL", cbGhostTell },			// Npc tell to a player, without Npc :)

	{ "NPC_TELL_EX", cbNpcTellEx },			// Extended Npc tell to a player. Receive a stringId (uint32), obtained through the PHRASE message for example
	{ "NPC_CHAT_EX", cbNpcChatEx },			// Extended Npc chat . Receive a stringId (uint32), obtained through the PHRASE message for example
	{ "NPC_CHAT_SENTENCE", cbNpcChatSentence}, //Npc chat. receive the sentence to chat
	{ "NPC_CHAT_SENTENCE_CHANNEL",cbNpcChatSentenceChannel},
	{ "NPC_CHAT_SENTENCE_EX",cbNpcChatSentenceEx},//Npc chat. receive the name of the npc which talks, and the sentence
	
	{ "GROUP_DYN_STRING", cbGroupDynString },	// send a system dynamic string to a group
	{ "PHRASE", cbPhrasePrepare },
	{ "PHRASE_DEBUG", cbPhrasePrepareDebug },
	{ "PHRASE_USER", cbUserPhrasePrepare },
	{ "PHRASE_USER_DEBUG", cbUserPhrasePrepareDebug },
	{ "BROADCAST_SYSTEM_PHRASE", cbBroadcastSystemPhrase },
	{ "BROADCAST_SYSTEM_PHRASE_DEBUG", cbBroadcastSystemPhraseDebug },
	{ "STORE_STRING", cbStoreString },
	{ "GET_STRING", cbGetString },
	{ "RETR_ENTITY_NAMES", cbRetrieveEntityNames },
	{ "USER_LANGUAGE", cbUserLanguage },	// receive an association between a userId and a language
	{ "REMOVE_USER_LANGUAGE", cbRemoveUserLanguage },

	{ "IGNORE_TELL_MODE", cbIgnoreTellMode }, // receive an ignore tell mode command from EGS
	{ "IGNORE", cbImpulsionIgnore },
	{ "UNIGNORE", cbImpulsionUnignore },
	{ "UNIGNORE_ALL", cbImpulsionUnignoreAll },
	{ "MUTE_PLAYER",	cbMutePlayer },		// receive a mute player command from EGS
	{ "UNIVERSE_MODE",	cbUniverseMode },		// receive a universe mode command from EGS
	{ "MUTE_UNIVERSE", cbMuteUniverse },	// received a mute universe channel from EGS

	{ "EMOTE_CROWD", cbEmoteCrowd },			// EGS wants IOS to dispatch an emote texte to all users around the "emoting" player
	{ "EMOTE_PLAYER", cbEmoteSolePlayer },	// EGS wants IOS to dispatch an emote texte to a unique player
	{ "CUSTOM_EMOTE", cbCustomEmote },	// EGS wants IOS to dispatch an emote custom text to all users around
	
	{ "SET_PHRASE", cbSetPhrase },			// AIS wants IOS to change a phrase content
	{ "SET_PHRASE_LANG", cbSetPhraseLang }, // AIS or EGS wants IOS to change a phrase content for a language

	{ "UPDATE_AIALIAS", cbUpdateAIAlias},

	// Dyn chat, received from egs
	{ "DYN_CHAT:ADD_CHAN", cbDynChatAddChan },
	{ "DYN_CHAT:REMOVE_CHAN", cbDynChatRemoveChan },
	{ "DYN_CHAT:ADD_SESSION", cbDynChatAddSession },
	{ "DYN_CHAT:ADD_SESSION_WITH_NAME", cbDynChatAddSessionWithName},//add a session, receive a npc name
	{ "DYN_CHAT:REMOVE_SESSION", cbDynChatRemoveSession },
	{ "DYN_CHAT:REMOVE_SESSION_WITH_NAME", cbDynChatRemoveSessionWithName },//remove a session, receive a npc name
	{ "DYN_CHAT:SET_WRITE_RIGHT", cbDynChatSetWriteRight },

	{ "DYN_CHAT:RESET", cbDynChatReset },
	{ "DYN_CHAT:SET_HISTORIC_SIZE", cbDynChatSetHistoricSize },
	{ "DYN_CHAT:SERVICE_CHAT", cbDynChatServiceChat },		// a service send a chat message in the channel without sender id
	{ "DYN_CHAT:SERVICE_TELL", cbDynChatServiceTell },		// a service send a chat message to a specific client in the channel without sender id
	{ "DYN_CHAT:SET_HIDE_BUBBLE", cbDynChatSetHideBubble },		// a service send a chat message to a specific client in the channel without sender id
	//received from DSS
	{ "REQUEST_DSR", cbRequestDsr},
//	{ "ADD_DM",  cbAddDM	},			// A character enter a ring session that he own
//	{ "REMOVE_DM",  cbRemoveDM	}		// A character leave a DSS

};

//-------------------------------------------------------------------------
// singleton initialisation and release

void CMessages::init()
{
	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( CbIOSArray, sizeof(CbIOSArray)/sizeof(CbIOSArray[0]) );
}

void CMessages::release()
{
}



