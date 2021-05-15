// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2016  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// MongoDB

/*b#include <mongo.h>*/


#include <nel/misc/command.h>

//#include "game_share/generic_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/synchronised_message.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/properties.h"
#include "game_share/backup_service_interface.h"

#include "server_share/r2_variables.h"
#include "server_share/mongo_wrapper.h"

#include "chat_manager.h"
#include "string_manager.h"
#include "input_output_service.h"
#include "chat_unifier_client.h"

#include "server_share/log_chat_gen.h"

//#include "ios_pd.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

#ifdef LOG_INFO
#undef LOG_INFO
#endif

void	logChatDirChanged(IVariable &var)
{
	// LogChatDirectory variable changed, reset it!
	//IOS->getChatManager().resetChatLog();
}

CVariable<bool>			VerboseChatManagement("ios","VerboseChatManagement", "Set verbosity for chat management", false, 0, true);
CVariable<std::string>	LogChatDirectory("ios", "LogChatDirectory", "Log Chat directory (default, unset is SaveFiles service directory", "", 0, true, logChatDirChanged);
CVariable<bool>			ForceFarChat("ios","ForceFarChat", "Force the use of SU to dispatch chat", false, 0, true);
CVariable<bool>			EnableDeepL("ios","EnableDeepL", "Enable DeepL auto-translation system", false, 0, true);

double last_mongo_chat_date = 1000.0*(double)CTime::getSecondsSince1970();

typedef NLMISC::CTwinMap<TChanID, string> TChanTwinMap;
TChanTwinMap 	_ChanNames;


CChatManager::CChatManager () : _Log(CLog::LOG_INFO)
{
	_Log.addDisplayer(&_Displayer);
}


//-----------------------------------------------
//	init
//
//-----------------------------------------------
void CChatManager::init( /*const string& staticDBFileName, const string& dynDBFileName*/ )
{
//	if (!staticDBFileName.empty())
//		_StaticDB.load( staticDBFileName );
//
//	if (!dynDBFileName.empty())
//		_DynDB.load( dynDBFileName );

#ifdef HAVE_MONGO
	CMongo::init();

	// reset all muted players
	CMongo::update("ryzom_users", "{ 'muted': true}", toString("{ $set:{ 'muted': false } }"), false, true);
#endif

	// create a chat group 'universe'
	addGroup(CEntityId(RYZOMID::chatGroup,0), CChatGroup::universe, "");

	// reset chat log system (at least to init it once!)
	resetChatLog();
} // init //

void CChatManager::onServiceDown(const std::string &serviceShortName)
{
	// if service is EGS, remove all chat groups
	if (serviceShortName == "EGS")
	{
		vector<TGroupId>	groupToRemove;

		// parse all group, selecting the one to remove when ESG is down
		std::map< TGroupId, CChatGroup >::iterator first(_Groups.begin()), last(_Groups.end());
		for (; first != last; ++first)
		{
			const TGroupId &gid = first->first;
			const CChatGroup &cg = first->second;

			switch (cg.Type)
			{
				case CChatGroup::universe:
				case CChatGroup::say:
				case CChatGroup::shout:
				case CChatGroup::player:
				case CChatGroup::nbChatMode:
					continue;
				case CChatGroup::team:
				case CChatGroup::guild:
				case CChatGroup::civilization:
				case CChatGroup::territory:
				case CChatGroup::tell:
				case CChatGroup::arround:
				case CChatGroup::system:
				case CChatGroup::region:
				case CChatGroup::dyn_chat:
					groupToRemove.push_back(gid);
					break;
			}
		}

		// remove all chat groups that belong to EGS or players
		for (uint i=0; i<groupToRemove.size(); ++i)
		{
			removeGroup(groupToRemove[i]);

		}

		// clear muted players table
		_MutedUsers.clear();

		// clear the dyn chats
		_DynChat.removeAllChannels();
	}
}

/*
 * Reset ChatLog management
 */
void CChatManager::resetChatLog()
{
	std::string	logPath = (LogChatDirectory.get().empty() ? Bsi.getLocalPath() : LogChatDirectory.get());
	_Displayer.setParam(CPath::standardizePath(logPath) + "chat.log");
}


bool CChatManager::checkClient( const TDataSetRow& id )
{
	TClientInfoCont::iterator itCl = _Clients.find( id );

	return itCl != _Clients.end();
}



void CChatManager::addMutedUser( const NLMISC::CEntityId &eid )
{
#ifdef HAVE_MONGO
	CMongo::update("ryzom_users", toString("{ 'cid': %d}", eid.getShortId()), toString("{ $set:{ 'muted': true } }"));
#endif

	_MutedUsers.insert( eid );
}

void CChatManager::removeMutedUser( const NLMISC::CEntityId &eid )
{
#ifdef HAVE_MONGO
	CMongo::update("ryzom_users", toString("{ 'cid': %d}", eid.getShortId()), toString("{ $set:{ 'muted': false } }"));
#endif

	_MutedUsers.erase( eid );
}

void CChatManager::addUniverseMutedUser( const NLMISC::CEntityId &eid )
{
#ifdef HAVE_MONGO
	CMongo::update("ryzom_users", toString("{ 'cid': %d}", eid.getShortId()), toString("{ $set:{ 'muted': true } }"));
#endif

	_MutedUniverseUsers.insert( eid );
}

void CChatManager::removeUniverseMutedUser( const NLMISC::CEntityId &eid )
{
#ifdef HAVE_MONGO
	CMongo::update("ryzom_users", toString("{ 'cid': %d}", eid.getShortId()), toString("{ $set:{ 'muted': false } }"));
#endif

	_MutedUniverseUsers.erase( eid );
}


//-----------------------------------------------
//	addClient
//
//-----------------------------------------------
void CChatManager::addClient( const TDataSetRow& id )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: addClient : adding client %s:%x into chat manager and universe group.",
			TheDataset.getEntityId(id).toString().c_str(),
			id.getIndex());
	}

	if(id.getIndex() == 0xffffff)
	{
		nlwarning("id.getIndex() == 0xffffff");
		return;
	}

	CEntityId eid = TheDataset.getEntityId(id);

	TClientInfoCont::iterator itCl = _Clients.find( id );
	if( itCl == _Clients.end() )
	{
		CChatClient *client = new CChatClient(id);
		_Clients.insert( make_pair(id,client) );

		if (eid.getType() == RYZOMID::player/* && !IsRingShard*/)
		{

			// add player in the group universe
			TGroupId grpUniverse = CEntityId(RYZOMID::chatGroup,0);
			addToGroup(grpUniverse, id);

			client->setChatMode(CChatGroup::say);
			client->updateAudience();

		}
	}
	else
	{
		nlwarning("CChatManager::addClient :  the client %s:%x is already in the manager !",
			TheDataset.getEntityId(id).toString().c_str(),
			id.getIndex());
	}
	// add in the dyn chat
	_DynChat.addClient(id);

} // addClient //



//-----------------------------------------------
//	removeClient
//
//-----------------------------------------------
void CChatManager::removeClient( const TDataSetRow& id )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: removeClient : removing the client %s:%x from chat manager !",
			TheDataset.getEntityId(id).toString().c_str(),
			id.getIndex());
	}

	TClientInfoCont::iterator itCl = _Clients.find( id );
	if( itCl != _Clients.end() )
	{
		// remove the client from any chat group that it subscribed.
		itCl->second->unsubscribeAllChatGroup();

		delete itCl->second;
		_Clients.erase( itCl );

	}
	else
	{
		nlwarning("CChatManager::removeClient : The client %s:%x is unknown !",
			TheDataset.getEntityId(id).toString().c_str(),
			id.getIndex());
	}
	// remove from the dyn chat
	_DynChat.removeClient(id);
} // removeClient //



//-----------------------------------------------
//	getClient
//
//-----------------------------------------------
CChatClient& CChatManager::getClient( const TDataSetRow& id )
{
	TClientInfoCont::iterator itCl = _Clients.find( id );
	if( itCl != _Clients.end() )
	{
		return *(itCl->second);
	}
	else
	{
		throw CChatManager::EChatClient(TheDataset.getEntityId(id));
	}

} // getClient //



//-----------------------------------------------
//	addGroup
//
//-----------------------------------------------
void CChatManager::addGroup( TGroupId gId, CChatGroup::TGroupType gType, const std::string &groupName )
{
	if ( gId == CEntityId::Unknown )
	{
		nlwarning("<CHAT> Cannot add chat group CEntityId::Unknown. group name = '%s'",groupName.c_str());
		return;
	}
	if (VerboseChatManagement)
	{
		if (!groupName.empty())
			nldebug("IOSCM: addGroup : adding %s named chat group %s as '%s'",
				CChatGroup::groupTypeToString(gType).c_str(),
				gId.toString().c_str(),
				groupName.c_str());
		else
			nldebug("IOSCM: addGroup : adding %s anonymous chat group %s",
				CChatGroup::groupTypeToString(gType).c_str(),
				gId.toString().c_str());
	}

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp == _Groups.end() )
	{
		TStringId nameId = CStringMapper::map(groupName);
		_Groups.insert( make_pair(gId, CChatGroup(gType, nameId)) );

		if (!groupName.empty())
		{
			pair<map<TStringId, TGroupId>::iterator, bool> ret;
			ret = _GroupNames.insert(make_pair(nameId, gId));
			if (!ret.second)
			{
				nlwarning("CChatManager::addGroup : will adding group %s, a chat group with the same name '%s' already exist !",
					gId.toString().c_str(),
					groupName.c_str());
			}
		}
	}
	else
	{
		nlwarning("CChatManager::addGroup : the group %s already exists", gId.toString().c_str());
	}

} // addGroup //



//-----------------------------------------------
//	removeGroup
//
//-----------------------------------------------
void CChatManager::removeGroup( TGroupId gId )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: removeGroup : removing group %s",
			gId.toString().c_str());
	}

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp != _Groups.end() )
	{
		if (itGrp->second.GroupName != CStringMapper::emptyId())
		{
			std::map<TStringId, TGroupId>::iterator it(_GroupNames.find(itGrp->second.GroupName));
			if (it == _GroupNames.end() || it->second != gId)
			{
				nlwarning("CChatManager::removeGroup : can't remove the group %s named '%s' from named group index",
					gId.toString().c_str(),
					CStringMapper::unmap(itGrp->second.GroupName).c_str());
			}
			else
				_GroupNames.erase(itGrp->second.GroupName);
		}
		_Groups.erase( itGrp );
	}
	else
	{
		nlwarning("CChatManager::removeGroup : the group %s is unknown", gId.toString().c_str());
	}

} // removeGroup //



//-----------------------------------------------
//	addToGroup
//
//-----------------------------------------------
void CChatManager::addToGroup( TGroupId gId, const TDataSetRow &charId )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: addtoGroup : adding player %s:%x to group %s",
		TheDataset.getEntityId(charId).toString().c_str(),
		charId.getIndex(),
		gId.toString().c_str());
	}

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp != _Groups.end() )
	{
		// add player in the group
		pair<CChatGroup::TMemberCont::iterator, bool> ret;
		ret = itGrp->second.Members.insert( charId );
		if (!ret.second)
		{
			nlwarning("CChatManager::addToGroup : can't add player %s:%x into group %s, already inside !",
				TheDataset.getEntityId(charId).toString().c_str(),
				charId.getIndex(),
				gId.toString().c_str());
		}
		else
		{
			TClientInfoCont::iterator itCl = _Clients.find( charId );
			if( itCl != _Clients.end() )
			{
				itCl->second->subscribeInChatGroup(gId);

				if (itGrp->second.Type == CChatGroup::team)
					itCl->second->setTeamChatGroup( gId );
				else if (itGrp->second.Type == CChatGroup::guild)
					itCl->second->setGuildChatGroup( gId );
				else if (itGrp->second.Type == CChatGroup::region)
					itCl->second->setRegionChatGroup( gId );
			}
			else
			{
				nlwarning("CChatManager::addToGroup : client %s:%x is unknown",
					TheDataset.getEntityId(charId).toString().c_str(),
					charId.getIndex());
				// remove it from the group (don't leave bad client...)
				itGrp->second.Members.erase(charId);
			}
		}
	}
	else
	{
		nlwarning("CChatManager::addToGroup : the group %s is unknown",gId.toString().c_str());
	}

} // addToGroup //



//-----------------------------------------------
//	removeFromGroup
//
//-----------------------------------------------
void CChatManager::removeFromGroup( TGroupId gId, const TDataSetRow &charId )
{
	if (VerboseChatManagement)
	{
		nldebug("IOSCM: removeFromGroup : removing player %s:%x from group %s",
			TheDataset.getEntityId(charId).toString().c_str(),
			charId.getIndex(),
			gId.toString().c_str());
	}

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup::TMemberCont::iterator itM = itGrp->second.Members.find(charId );
		if( itM != itGrp->second.Members.end() )
		{
			itGrp->second.Members.erase( itM );
			TClientInfoCont::iterator itCl = _Clients.find( charId );
			if( itCl != _Clients.end() )
			{
				itCl->second->unsubscribeInChatGroup(gId);

				if (itGrp->second.Type == CChatGroup::team)
					itCl->second->setTeamChatGroup( CEntityId::Unknown);
				else if (itGrp->second.Type == CChatGroup::guild)
					itCl->second->setGuildChatGroup( CEntityId::Unknown );
				else if (itGrp->second.Type == CChatGroup::region)
					itCl->second->setRegionChatGroup( CEntityId::Unknown );
			}
			else
			{
				nlwarning("CChatManager::removeFromGroup : player %s:%x is unknown",
					TheDataset.getEntityId(charId).toString().c_str(),
					charId.getIndex());
			}
		}
		else
		{
			nlwarning("CChatManager::removeFromGroup : player %s:%x is not in the group %s",
				TheDataset.getEntityId(charId).toString().c_str(),
				charId.getIndex(),
				gId.toString().c_str());
		}
	}
	else
	{
		nlwarning("CChatManager::removeFromGroup : the group %s is unknown",
			gId.toString().c_str());
	}

} // removeFromGroup //


//-----------------------------------------------
//	getGroup
//
//-----------------------------------------------
CChatGroup& CChatManager::getGroup( const TGroupId& gId )
{
	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( gId );
	if( itGrp != _Groups.end() )
	{
		return (*itGrp).second;
	}
	else
	{
		throw EChatGroup(gId);
	}

} // getGroup //


//-----------------------------------------------
//	checkNeedDeeplize
//
//-----------------------------------------------
void CChatManager::checkNeedDeeplize( const TDataSetRow& sender, const ucstring& ucstr, const string& senderLang, string &langs, uint &nbrReceivers, TGroupId grpId)
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	CChatManager &cm = IOS->getChatManager();
	CChatClient &senderClient = cm.getClient(sender);

	bool have_fr = false;
	bool have_de = false;
	bool have_en = false;
	bool have_ru = false;
	bool have_es = false;
	CChatGroup::TMemberCont::iterator itA;
	CChatGroup::TMemberCont::iterator itEnd;

	nbrReceivers = 0;

	if (grpId == CEntityId::Unknown)
	{
		itA = itCl->second->getAudience().Members.begin();
		itEnd = itCl->second->getAudience().Members.end();
	}
	else
	{
		map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
		if( itGrp != _Groups.end() )
		{
			CChatGroup &chatGrp = itGrp->second;
			itA = chatGrp.Members.begin();
			itEnd = chatGrp.Members.end();
		}
		else
		{
			itA = itCl->second->getAudience().Members.end();
			itEnd = itCl->second->getAudience().Members.end();
		}
	}


	for( ; itA != itEnd; ++itA )
	{
		ucstring message;
		string receiverName;
		NLMISC::CEntityId receiverId = TheDataset.getEntityId(*itA);
		CCharacterInfos* co = IOS->getCharInfos(receiverId);

		_DestUsers.push_back(receiverId);
		string receiverLang;
		if (co == NULL)
		{
			receiverName = receiverId.toString();
		}
		else
		{
			receiverName = co->Name.toString();
			receiverLang = SM->getLanguageCodeString(co->Language);
		}

		if (EnableDeepL && !senderClient.dontSendTranslation(senderLang))
		{
			CChatClient &client = getClient(*itA);

			if (senderLang == "wk")
				receiverLang = senderLang;

			if (ucstr[0] == '>') // Sent directly when prefixed by '>', it's the anti-translation code
			{
				if (grpId == CEntityId::Unknown && ucstr.length() > 5 && ucstr[1] == ':' && ucstr[4] == ':') // check lang prefix only for chat()
				{
					string usedLang = ucstr.toString().substr(2, 2);
					if (usedLang == receiverLang && !client.dontReceiveTranslation(usedLang))
						message = ucstr.substr(5);
				}
				else
				{
					message = ucstr.substr(1);
				}
			}
			else if (senderLang == receiverLang || client.dontReceiveTranslation(senderLang)) // Sent directly if sender and receiver uses same lang
			{
				message = ucstr;
			}
			else
			{
				if (!have_fr && receiverLang == "fr")
					have_fr = true;
				if (!have_de && receiverLang == "de")
					have_de = true;
				if (!have_en && receiverLang == "en")
					have_en = true;
				if (!have_ru && receiverLang == "ru")
					have_ru = true;
				if (!have_es && receiverLang == "es")
					have_es = true;
				nbrReceivers++;
			}
		}
		else
		{
			message = ucstr;
		}

		if (!message.empty())
		{
			if (grpId == CEntityId::Unknown)
				sendChat( itCl->second->getChatMode(), *itA, message, sender);
		}
	}

	if (grpId != CEntityId::Unknown) // Chat in group must be sent only one time
	{
		if (ucstr[0] == '>') // direct, no translation
			chatInGroup( grpId, ucstr.substr(1), sender );
		else if (ucstr.length() > 5 && ucstr[1] == ':' && ucstr[4] == ':') // Already have filter
			chatInGroup( grpId, ucstr, sender );
		else
			chatInGroup( grpId, ucstring(":"+senderLang+":")+ucstr, sender ); // Need filter
	}


	langs = senderLang;
	if (have_fr)
		langs += "-fr";
	if (have_de)
		langs += "-de";
	if (have_en)
		langs += "-en";
	if (have_ru)
		langs += "-ru";
	if (have_es)
		langs += "-es";

}




//-----------------------------------------------
//	chat
//
//-----------------------------------------------
void CChatManager::chat( const TDataSetRow& sender, const ucstring& ucstr )
{

	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
		CChatManager &cm = IOS->getChatManager();
		CChatClient &senderClient = cm.getClient(sender);

//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM:  chat The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}

//		CEntityId eid = TheDataset.getEntityId(sender);
		// Get the char info
		//WARNING: can be NULL
		CCharacterInfos *ci = IOS->getCharInfos(eid);

		// info for log the chat message
		string senderName;
		string fullName;

		if (ci == NULL)
		{
			senderName = TheDataset.getEntityId(sender).toString();
			fullName = senderName;
		}
		else
		{
			fullName = IOS->getRocketName(ci->Name);
			senderName = ci->Name.toString();
		}

		static const char*	groupNames[]=
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
			"dyn_chat",
			"nbChatMode"
		};

		// clean up container
		_DestUsers.clear();

		string senderLang = SM->getLanguageCodeString(ci->Language);

		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::shout :
		case CChatGroup::say :
		case CChatGroup::arround :
			{
				string langs;
				uint nbrReceivers;
				checkNeedDeeplize(sender, ucstr, senderLang, langs, nbrReceivers);

				if (nbrReceivers > 0)
					_Log.displayNL("%s|%s|%d|%s|%s", groupNames[itCl->second->getChatMode()], fullName.c_str(), nbrReceivers, langs.c_str(), ucstr.toUtf8().c_str() );
			}
			break;
		case CChatGroup::region :
			{
				// Previously, the msgs were sent to the current audience as well, to avoid characters around but
				// in an adjoining region not receiving the region msg. But the neighbouring was not tested,
				// and the audience was not updated after the chat mode became region (see CChatClient::updateAudience())
				// so even after teleporting in a remote region the previous around people were still receiving
				// the messages.

				TGroupId grpId = itCl->second->getRegionChatGroup();
				_DestUsers.push_back(grpId);
				string langs;
				uint nbrReceivers;
				checkNeedDeeplize(sender, ucstr, senderLang, langs, nbrReceivers, grpId);
				if (nbrReceivers > 0)
					_Log.displayNL("region:%s|%s|%d|%s|%s", grpId.toString().c_str(), fullName.c_str(), nbrReceivers, langs.c_str(), ucstr.toUtf8().c_str() );

				/*if (EnableDeepL && !senderClient.dontSendTranslation(senderLang))
				{
					if (ucstr[0] == '>') // Sent directly when prefixed by '>', it's the anti-translation code
						chatInGroup( grpId, ucstr.substr(1), sender );
					else if (senderClient.dontSendTranslation(senderLang))
						chatInGroup( grpId, ucstr, sender );
					else
						_Log.displayNL("region:%s|%s|*|%s-*|%s", grpId.toString().c_str(), fullName.c_str(), senderLang.c_str(), ucstr.toUtf8().c_str() );
				}
				else
					chatInGroup( grpId, ucstr, sender );*/
			}
			break;


		case CChatGroup::universe:
			{
				CEntityId eid = TheDataset.getEntityId(sender);
				if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
				{
					nldebug("IOSCM:  chat The player %s:%x is universe muted",
						TheDataset.getEntityId(sender).toString().c_str(),
						sender.getIndex());
					return;
				}

//				// on ring shard, the universe chat is reserved to DM and editors
//				if (IsRingShard)
//				{
//					// check that this character is in the universe group
//					CChatGroup &cg = getGroup(TGroupId(RYZOMID::chatGroup, 0));
//					if (cg.Members.find(sender) == cg.Members.end())
//					{
//						// warn the player about the unavailability of the universe chat
//						static STRING_MANAGER::CVectorParamCheck params;
//						uint32 phraseId = STRING_MANAGER::sendStringToClient( sender, "UNIVERSE_NOT_AVAILABLE_ON_RING", params, &IosLocalSender );
//						sendChat2Ex( CChatGroup::system, sender, phraseId );
//						return;
//					}
//				}

				TGroupId grpId = CEntityId(RYZOMID::chatGroup, 0);
				_DestUsers.push_back(grpId);

				double date = 1000.0*(double)CTime::getSecondsSince1970();

				bool sendToMongo = true;
				bool sendToAllUni = false;
				string autoSub = "1";
				uint8 startPos = 0;
				string mongoText = ucstr.toUtf8();
				string chatId = "all";
				string chatType = "univers";


				string usedlang = senderLang;

				if (EnableDeepL)
				{
					chatType = "dynamic";
					if (ucstr[0] == '>') // Sent directly when prefixed by '>', it's the anti-translation code
					{
						startPos = 1;
						mongoText = mongoText.substr(1);
						string::size_type endOfOriginal = mongoText.find("}@{");
						if (mongoText.size() > 4 && mongoText[0] == ':' && mongoText[3] == ':')
						{
							startPos = 5;
							usedlang = mongoText.substr(1, 2);
							string source_lang = usedlang;

							if (endOfOriginal != string::npos)
							{
								if (mongoText.size() > 9)
									source_lang = mongoText.substr(6, 2);
								string sourceText = mongoText.substr(9, endOfOriginal-9);
								strFindReplace(sourceText, ")", "}");
								mongoText = "["+source_lang+"](http://chat.ryzom.com/channel/pub-uni-"+source_lang+"?%20"+encodeURIComponent(sourceText)+") "+mongoText.substr(endOfOriginal+4, mongoText.size()-endOfOriginal-4);
							}
							else
							{
								usedlang = mongoText.substr(1, 2);
								mongoText = mongoText.substr(4, mongoText.size()-4);
							}

							if (source_lang == "en") // in RC the icon are :gb:
								mongoText = ":gb:"+mongoText;
							else
								mongoText = ":"+source_lang+":"+mongoText;

							chatId = "FACTION_"+toUpper(usedlang);
							if (usedlang != SM->getLanguageCodeString(ci->Language))
								autoSub = "0";

						}
						else
							sendToAllUni = true;

						chatInGroup( grpId, ucstr.substr(1), sender );
					}
					else if (senderClient.dontSendTranslation(senderLang))
					{
						sendToAllUni = true;
						chatInGroup( grpId, ucstr, sender );
					}
					else
					{
						_Log.displayNL("%s|%s|*|%s-*|%s", "universe", fullName.c_str(), senderLang.c_str(), ucstr.toUtf8().c_str());
						sendToMongo = false;
					}
				}
				else
					chatInGroup( grpId, ucstr, sender );

#ifdef HAVE_MONGO
				if (sendToMongo) // only send to mongo if it's not a translated message
				{
					if (sendToAllUni)
					{
						CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': '%s', 'chatId': 'FACTION_EN', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), chatType.c_str(), date, senderLang == "en"?"1":"0"));
						CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': '%s', 'chatId': 'FACTION_DE', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), chatType.c_str(), date, senderLang == "de"?"1":"0"));
						CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': '%s', 'chatId': 'FACTION_ES', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), chatType.c_str(), date, senderLang == "es"?"1":"0"));
						CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': '%s', 'chatId': 'FACTION_FR', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), chatType.c_str(), date, senderLang == "fr"?"1":"0"));
						CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': '%s', 'chatId': 'FACTION_RU', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), chatType.c_str(), date, senderLang == "ru"?"1":"0"));
					}
					else
						CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': '%s', 'chatId': '%s', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), chatType.c_str(), chatId.c_str(), date, autoSub.c_str()));
				}
#endif

			}
			break;

		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				_DestUsers.push_back(grpId);

				_Log.displayNL("'%s' (%s) : %s", fullName.c_str(), groupNames[itCl->second->getChatMode()], ucstr.toString().c_str() );
				chatInGroup( grpId, ucstr, sender );
			}
			break;

		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				_DestUsers.push_back(grpId);
				string langs;
				uint nbrReceivers;
				checkNeedDeeplize(sender, ucstr, senderLang, langs, nbrReceivers, grpId);
				if (nbrReceivers > 0)
				{
					//sendToMongo = false;
					_Log.displayNL("guild:%s|%s|%d|%s|%s", grpId.toString().c_str(), fullName.c_str(), nbrReceivers, langs.c_str(), ucstr.toUtf8().c_str() );
				}

				//_Log.displayNL("%s' (%s) : %s", fullName.c_str(), groupNames[itCl->second->getChatMode()], ucstr.toString().c_str() );

				uint32 guildId = grpId.getShortId() - 0x10000000;

				ostringstream sGuildId;
				sGuildId << guildId;

				double date = 1000.0*(double)CTime::getSecondsSince1970();

#ifdef HAVE_MONGO
				string mongoText = ucstr.toUtf8();
				string translatedLang = senderLang;
				if (mongoText[0] == '>')
					mongoText = mongoText.substr(1);

				if (mongoText.size() > 4 && mongoText[0] == ':' && mongoText[3] == ':')
				{
					translatedLang = mongoText.substr(1, 2);
					mongoText = mongoText.substr(4);
				}

				if (translatedLang == "en")
					translatedLang = "gb";

				string::size_type endOfOriginal = mongoText.find("}@{");
				if (endOfOriginal != string::npos)
					mongoText = mongoText.substr(endOfOriginal+4, mongoText.size()-endOfOriginal-4);

				CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'guildId', 'chatId': '%s', 'date': %f, 'ig': true, 'autoSub': 1 }", CMongo::quote(fullName).c_str(), CMongo::quote(":"+translatedLang+":"+mongoText).c_str(), sGuildId.str().c_str(), date));
#endif
				//chatInGroup( grpId, ucstr, sender );
			}
			break;
		case CChatGroup::dyn_chat:
		{
			TChanID chanId = itCl->second->getDynChatChan();

			CDynChatSession *session = _DynChat.getSession(chanId, sender);
			if (session) // player must have a session in that channel
			{
				if (session->WriteRight) // player must have the right to speak in the channel
				{
					// If universal channel check if player muted
					if (session->getChan()->UniversalChannel)
					{
						if(_MutedUsers.find( eid ) != _MutedUsers.end())
						{
							nldebug("IOSCM:  chat The player %s:%x is muted",
								TheDataset.getEntityId(sender).toString().c_str(),
								sender.getIndex());
							return;
						}
					}

					const std::string *tmpChatId = _ChanNames.getB(chanId);
					string chatId;
					if (tmpChatId)
						chatId = *tmpChatId;

					double date = 1000.0*(double)CTime::getSecondsSince1970();


					string mongoText = ucstr.toUtf8();

					bool sendMessages = true;
					bool sendToAllForge = false;
					bool haveOriginMessage = false;
					uint8 startPos = 0;

					string originLang = senderLang;
					string translatedLang = "";
					string autoSub = "0";

					if (EnableDeepL)
					{
						if (senderClient.dontSendTranslation(senderLang))
						{
							sendToAllForge = true;
						}
						else if (ucstr[0] == '>') // Sent directly when prefixed by '>', it's the anti-translation code
						{
							startPos = 1;
							mongoText = mongoText.substr(1);
							string::size_type endOfOriginal = mongoText.find("}@{");
							if (mongoText.size() > 4 && mongoText[0] == ':' && mongoText[3] == ':')
							{
								startPos = 5;
								translatedLang = mongoText.substr(1, 2);

								if (endOfOriginal != string::npos)
								{
									haveOriginMessage = true;
									if (mongoText.size() > 9)
										originLang = mongoText.substr(6, 2);
									string sourceText = mongoText.substr(9, endOfOriginal-9);
									strFindReplace(sourceText, ")", "}");
									mongoText = "["+originLang+"](http://chat.ryzom.com/channel/pub-forge-"+originLang+"?%20"+encodeURIComponent(sourceText)+") "+mongoText.substr(endOfOriginal+4, mongoText.size()-endOfOriginal-4);
								}
								else
								{
									originLang = translatedLang;
									mongoText = mongoText.substr(4, mongoText.size()-4);
								}

								if (originLang == "en") // in RC the icon are :gb:
									mongoText = ":gb:"+mongoText;
								else
									mongoText = ":"+originLang+":"+mongoText;
							}
							else if (chatId == "FACTION_RF") // Only when it's a not translated text
								sendToAllForge = true;

						}
						// Send for translation
						else if (chatId == "FACTION_RF" || chatId == "FACTION_EN" || chatId == "FACTION_DE" || chatId == "FACTION_FR" || chatId == "FACTION_ES" || chatId == "FACTION_RU")
						{
							_Log.displayNL("%s|%s|*|%s-*|%s", string("#"+chatId).c_str(), fullName.c_str(), senderLang.c_str(), ucstr.toUtf8().c_str());
							sendMessages = false; // We need translated it before
						}
					}

					if (sendMessages) // only send to mongo if it's not a message to translate
					{
#ifdef HAVE_MONGO

						if (sendToAllForge)
						{
							CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'dynamic', 'chatId': 'FACTION_RF-EN', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), date, senderLang == "en"?"1":"0"));
							CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'dynamic', 'chatId': 'FACTION_RF-DE', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), date, senderLang == "de"?"1":"0"));
							CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'dynamic', 'chatId': 'FACTION_RF-ES', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), date, senderLang == "es"?"1":"0"));
							CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'dynamic', 'chatId': 'FACTION_RF-FR', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), date, senderLang == "fr"?"1":"0"));
							CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'dynamic', 'chatId': 'FACTION_RF-RU', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), date, senderLang == "ru"?"1":"0"));
						}
						else
						{
							if (chatId == "FACTION_RF")
							{
								if (translatedLang.empty())
									chatId = "FACTION_RF-"+toUpper(senderLang);
								else
									chatId = "FACTION_RF-"+toUpper(translatedLang);
							}
							CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'dynamic', 'chatId': '%s', 'date': %f, 'ig': true, 'autoSub': %s }", CMongo::quote(fullName).c_str(), CMongo::quote(mongoText).c_str(), chatId.c_str(), date, (translatedLang.empty() || translatedLang == senderLang)?"1":"0"));
						}

#endif

						if (!session->getChan()->getDontBroadcastPlayerInputs())
						{
							// add msg to the historic
							CDynChatChan::CHistoricEntry entry;
							entry.String = ucstr;
							if (ci != NULL)
								entry.SenderString = ci->Name;
							else
								entry.SenderString = "";

							session->getChan()->Historic.push(entry);

							ucstring content;
							if (!session->getChan()->HideBubble)
							{	//normal case
								content = ucstr;
							}
							else
							{
								// true for control channel (Ring)
								ucstring tmp("{no_bubble}");
								if (ucstr.find(tmp) == ucstring::npos)
								{
									tmp += ucstr;
									content.swap(tmp);
								}
								else
								{
									content = ucstr;
								}
							}

							// broadcast to other client in the channel
							CDynChatSession *dcc = session->getChan()->getFirstSession();
							while (dcc)
							{
								NLMISC::CEntityId receiverId = TheDataset.getEntityId(dcc->getClient()->getID());
								CCharacterInfos* co = IOS->getCharInfos(receiverId);
								CChatClient &receiverClient = getClient(dcc->getClient()->getID());

								bool canSendChat = true;

								if (EnableDeepL)
								{
									if (receiverClient.dontReceiveTranslation(originLang))
									{
										if (haveOriginMessage) // Only send untranslated message
											canSendChat = false;
									}
									else if (!translatedLang.empty() && co && translatedLang != SM->getLanguageCodeString(co->Language))
										canSendChat = false;
								}

								if (canSendChat)
									sendChat(itCl->second->getChatMode(), dcc->getClient()->getID(), content.substr(startPos), sender, chanId);
								dcc = dcc->getNextChannelSession(); // next session in this channel
							}
						}
						else
						{
							// only send an echo to the sender
							if (!EnableDeepL) // only send an echo to the sender
								sendChat(itCl->second->getChatMode(), itCl->first, ucstr, sender, chanId);
						}

						if (session->getChan()->getForwardPlayerIntputToOwnerService())
						{
							// send player input to service owner
							NLNET::TServiceId serviceId(chanId.getCreatorId());

							TPlayerInputForward	pif;
							pif.ChanID = chanId;
							pif.Sender = sender;
							pif.Content = ucstr;

							CMessage msgout( "DYN_CHAT:FORWARD");
							msgout.serial(pif);

							CUnifiedNetwork::getInstance()->send(serviceId, msgout);
						}

						if (session->getChan()->getUnifiedChannel())
						{
							// send the text to other shards
							if (IChatUnifierClient::getInstance())
								IChatUnifierClient::getInstance()->sendUnifiedDynChat(session->getChan()->getID(), senderName, ucstr);
						}
					}
				}
			}
		}
		break;
			// static group

		default :
			nlwarning("<CChatManager::chat> client %u chat in %s ! don't know how to handle it.",
				sender.getIndex(),
				groupNames[itCl->second->getChatMode()]);
/*			{
				TGroupId grpId = itCl->second.getChatGroup();

				_Log.displayNL("'%s' (%s) : \t\"%s\"", senderName.c_str(), groupNames[itCl->second.getChatMode()], ucstr.toString().c_str() );

				chatInGroup( grpId, ucstr, sender );
			}
*/		}

		// log chat to PDS system
//		IOSPD::logChat(ucstr, itCl->second->getId(), _DestUsers);
		log_Chat_Chat(CChatGroup::groupTypeToString(itCl->second->getChatMode()),
			TheDataset.getEntityId(sender),
			ucstr.toUtf8(),
			_DestUsers);

	}
	else
	{
		nlwarning("<CChatManager::chat> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}

} // chat //


//-----------------------------------------------
//	chatInGroup
//
//-----------------------------------------------
void CChatManager::chatInGroup( TGroupId& grpId, const ucstring& ucstr, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded )
{
	CMirrorPropValueRO<uint32> senderInstanceId( TheDataset, sender, DSPropertyAI_INSTANCE );

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		list<CEntityId>	logDest;

		uint8 startPos = 0;
		string usedlang = "";
		string str = ucstr.toString();
		if (EnableDeepL && str.length() > 4 && str[0] == ':' && str[3] == ':') // check lang prefix
		{
			usedlang = str.substr(1, 2);
			startPos = 4;
		}

		bool areOriginal = true;
		string originLang = usedlang;
		if (EnableDeepL && str.length() > 8 && str[4] == '{' && str[5] == ':' && str[8] == ':') // check lang origin
		{
			areOriginal = false;
			originLang = str.substr(6, 2);
		}

		for( itM = chatGrp.Members.begin(); itM != chatGrp.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			if (EnableDeepL && !usedlang.empty())
			{
				NLMISC::CEntityId receiverId = TheDataset.getEntityId(*itM);
				CCharacterInfos* co = IOS->getCharInfos(receiverId);
				CChatClient &client = getClient(*itM);

				if (client.dontReceiveTranslation(originLang))
				{
					if (!areOriginal)
						continue;
				}
				else if (co == NULL || usedlang != SM->getLanguageCodeString(co->Language))
					continue;
			}

			// check the ai instance for region chat
			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				// check homeSessionId for universe
				if (/*IsRingShard && */chatGrp.Type == CChatGroup::universe)
				{
					CCharacterInfos *senderChar = IOS->getCharInfos(TheDataset.getEntityId(sender));
					CCharacterInfos *receiverChar = IOS->getCharInfos(TheDataset.getEntityId(*itM));

					if (senderChar == NULL || receiverChar == NULL)
						continue;

					// set GM mode if either speaker of listener is a GM
					bool isGM= senderChar->HavePrivilege || receiverChar->HavePrivilege;

					// for normal players don't send chat to them if their home session id doesn't match the speaker's
					if (!isGM && senderChar->HomeSessionId != receiverChar->HomeSessionId)
					{
						continue;
					}
				}
				// check the exclude list
				if ( std::find( excluded.begin(), excluded.end(), *itM ) == excluded.end() )
				{
					sendChat( itGrp->second.Type, *itM, ucstr.substr(startPos), sender );
					_DestUsers.push_back(TheDataset.getEntityId(*itM));
				}
			}
		}

		if (chatGrp.Type == CChatGroup::guild)
		{
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					IChatUnifierClient::getInstance()->sendFarGuildChat(charInfos->Name, uint32(grpId.getShortId()), ucstr.substr(startPos));
				}
			}
		}
		else if (chatGrp.Type == CChatGroup::universe /*&& IsRingShard*/)
		{
			// forward universe chat to other shard with home session id
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					// determine the session id as the home session id for normal players and the current session id for GMs
					uint32 sessionId= (charInfos->HavePrivilege && !IsRingShard)? IService::getInstance()->getShardId(): (uint32)charInfos->HomeSessionId;
					IChatUnifierClient::getInstance()->sendUniverseChat(charInfos->Name, sessionId, ucstr.substr(startPos));
				}
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}

} // chatInGroup //

void CChatManager::farChatInGroup(TGroupId &grpId, uint32 homeSessionId, const ucstring &text, const ucstring &senderName)
{
	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{


		uint8 startPos = 0;
		string usedlang;
		if (EnableDeepL && text.length() > 4 && text[0] == ':' && text[3] == ':') // check lang prefix
		{
			usedlang = text.toString().substr(1, 2);
			startPos = 4;
		}

		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = chatGrp.Members.begin(); itM != chatGrp.Members.end(); ++itM )
		{
			CCharacterInfos *charInfo = IOS->getCharInfos(TheDataset.getEntityId(*itM));
			if (charInfo==NULL)
				continue;

			if (homeSessionId != 0)
			{
				// determine the session id as the home session id for normal players and the current session id for GMs
				uint32 sessionId= (charInfo->HavePrivilege && !IsRingShard)? IService::getInstance()->getShardId(): (uint32)charInfo->HomeSessionId;

				// check that the dest has the same home as sender
				if (sessionId != homeSessionId)
					continue;
			}

			if (EnableDeepL && !usedlang.empty() && usedlang != SM->getLanguageCodeString(charInfo->Language))
				continue;

			sendFarChat( itGrp->second.Type, *itM, text.substr(startPos), senderName );
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}
}


void CChatManager::chat2( const TDataSetRow& sender, const std::string &phraseId )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM: chat2 The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::say :
		case CChatGroup::shout :
			{
				CChatGroup::TMemberCont::iterator itA;
				for( itA = itCl->second->getAudience().Members.begin();
						itA != itCl->second->getAudience().Members.end();
							++itA )
				{
					sendChat2( itCl->second->getChatMode(), *itA, phraseId, sender );
				}
			}
			break;
		case CChatGroup::region :
			{
				// See comment in chat()
				TGroupId grpId = itCl->second->getRegionChatGroup();
				chatInGroup2( grpId, phraseId, sender );
			}
			break;


		case CChatGroup::universe:
			{
				CEntityId eid = TheDataset.getEntityId(sender);
				if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
				{
					nldebug("IOSCM:  chat The player %s:%x is universe muted",
						TheDataset.getEntityId(sender).toString().c_str(),
						sender.getIndex());
					return;
				}

				TGroupId grpId = CEntityId(RYZOMID::chatGroup,0);
				chatInGroup2( grpId, phraseId, sender );
			}
			break;
		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				chatInGroup2( grpId, phraseId, sender );
			}
			break;

		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				chatInGroup2( grpId, phraseId, sender );
			}
			break;


			// static group
		default :
			{
			nlwarning("<CChatManager::chat> client %s:%x chat in mode %u ! don't know how to handle it.",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex(),
				itCl->second->getChatMode());
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}

}


void CChatManager::chatParam( const TDataSetRow& sender, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam>& params )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM: chat2 The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::say :
		case CChatGroup::shout :
			{
				CChatGroup::TMemberCont::iterator itA;
				for( itA = itCl->second->getAudience().Members.begin();
						itA != itCl->second->getAudience().Members.end();
							++itA )
				{
					sendChatParam( itCl->second->getChatMode(), *itA, phraseId, params, sender );
				}
			}
			break;
		case CChatGroup::region :
			{
				// See comment in chat()
				TGroupId grpId = itCl->second->getRegionChatGroup();
				chatParamInGroup( grpId, phraseId, params, sender );
			}
			break;


		case CChatGroup::universe:
			{
				CEntityId eid = TheDataset.getEntityId(sender);
				if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
				{
					nldebug("IOSCM:  chat The player %s:%x is universe muted",
						TheDataset.getEntityId(sender).toString().c_str(),
						sender.getIndex());
					return;
				}

				TGroupId grpId = CEntityId(RYZOMID::chatGroup,0);
				chatParamInGroup( grpId, phraseId, params, sender );
			}
			break;
		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				chatParamInGroup( grpId, phraseId, params, sender );
			}
			break;

		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				chatParamInGroup( grpId, phraseId, params, sender );
			}
			break;


			// static group
		default :
			{
			nlwarning("<CChatManager::chat> client %s:%x chat in mode %u ! don't know how to handle it.",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex(),
				itCl->second->getChatMode());
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}

}


void CChatManager::chat2Ex( const TDataSetRow& sender, uint32 phraseId)
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM: chat2Ex The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::say :
		case CChatGroup::shout :
			{
				CChatGroup::TMemberCont::iterator itA;
				for( itA = itCl->second->getAudience().Members.begin();
				itA != itCl->second->getAudience().Members.end();
				++itA )
				{
						sendChat2Ex( itCl->second->getChatMode(), *itA, phraseId,sender );
				}
			}
			break;
		case CChatGroup::region :
			{
				// See comment in chat()
				TGroupId grpId = itCl->second->getRegionChatGroup();
				chatInGroup2Ex( grpId, phraseId, sender );
			}
			break;

		case CChatGroup::universe:
			{
				CEntityId eid = TheDataset.getEntityId(sender);
				if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
				{
					nldebug("IOSCM:  chat The player %s:%x is universe muted",
						TheDataset.getEntityId(sender).toString().c_str(),
						sender.getIndex());
					return;
				}

				TGroupId grpId = CEntityId(RYZOMID::chatGroup,0);
				chatInGroup2Ex( grpId, phraseId, sender );
			}
			break;
		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				chatInGroup2Ex( grpId, phraseId, sender );
			}
			break;

		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				chatInGroup2Ex( grpId, phraseId, sender );
			}
			break;


			// static group
		default :
			{
				nlwarning("<CChatManager::chat> client %s:%x chat in mode %u ! don't know how to handle it.",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex(),
					itCl->second->getChatMode());
				//				TGroupId grpId = (*itCl).second.getChatGroup();
				//				chatInGroup2( grpId, phraseId, sender );
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}
}


void CChatManager::chatInGroup2Ex( TGroupId& grpId, uint32 phraseId, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded )
{
	CMirrorPropValueRO<uint32> senderInstanceId( TheDataset, sender, DSPropertyAI_INSTANCE );

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = itGrp->second.Members.begin(); itM != itGrp->second.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				const CEntityId &eid = TheDataset.getEntityId(*itM);
				// check the ai instance for region chat
				if (eid.getType() == RYZOMID::player && std::find( excluded.begin(), excluded.end(), *itM ) == excluded.end() )
					sendChat2Ex( itGrp->second.Type, *itM, phraseId, sender );
			}
		}

		if (chatGrp.Type == CChatGroup::guild)
		{
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					IChatUnifierClient::getInstance()->sendFarGuildChat2Ex(charInfos->Name, uint32(grpId.getShortId()), phraseId);
				}
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}
}

void CChatManager::chatInGroup2( TGroupId& grpId, const std::string & phraseId, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded )
{
	CMirrorPropValueRO<uint32> senderInstanceId( TheDataset, sender, DSPropertyAI_INSTANCE );

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = itGrp->second.Members.begin(); itM != itGrp->second.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			// check the ai instance for region chat
			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				const CEntityId &eid = TheDataset.getEntityId(*itM);
				if (eid.getType() == RYZOMID::player && std::find( excluded.begin(), excluded.end(), *itM ) == excluded.end() )
					sendChat2( (*itGrp ).second.Type, *itM, phraseId, sender );
			}
		}
		if (chatGrp.Type == CChatGroup::guild)
		{
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					IChatUnifierClient::getInstance()->sendFarGuildChat2(charInfos->Name, uint32(grpId.getShortId()), phraseId);
				}
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}
}

void CChatManager::chatParamInGroup( TGroupId& grpId, const std::string & phraseId, const std::vector<STRING_MANAGER::TParam>& params, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded )
{
	CMirrorPropValueRO<uint32> senderInstanceId( TheDataset, sender, DSPropertyAI_INSTANCE );

	map< TGroupId, CChatGroup >::iterator itGrp = _Groups.find( grpId );
	if( itGrp != _Groups.end() )
	{
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = itGrp->second.Members.begin(); itM != itGrp->second.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			// check the ai instance for region chat
			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				const CEntityId &eid = TheDataset.getEntityId(*itM);
				if (eid.getType() == RYZOMID::player && std::find( excluded.begin(), excluded.end(), *itM ) == excluded.end() )
					sendChat2( (*itGrp ).second.Type, *itM, phraseId, sender );
			}
		}
		if (chatGrp.Type == CChatGroup::guild)
		{
			CCharacterInfos *charInfos = IOS->getCharInfos(TheDataset.getEntityId(sender));
			if (charInfos != NULL)
			{
				// forward to chat unifier to dispatch to other shards
				if (IChatUnifierClient::getInstance())
				{
					IChatUnifierClient::getInstance()->sendFarGuildChat2(charInfos->Name, uint32(grpId.getShortId()), phraseId);
					if (!params.empty())
					{
						nlerror("Guild chat with params is not implemented yet");
					}
				}
			}
		}
	}
	else
	{
		nlwarning("<CChatManager::chatInGroup> The group %s is unknown",grpId.toString().c_str());
	}
}



void CChatManager::sendEmoteTextToAudience(  const TDataSetRow& sender,const std::string & phraseId, const TVectorParamCheck & params , const std::vector<TDataSetRow> & excluded )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
		// muted players can't do emotes (text)
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM:<CChatManager::sendEmoteTextToAudience> The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}

		// set the player chat mode and update its audience
		CChatGroup::TGroupType oldMode = itCl->second->getChatMode();
		TChanID	oldChan = itCl->second->getDynChatChan();
		itCl->second->setChatMode(CChatGroup::say);
		itCl->second->updateAudience();

		// get audience around the emoting player
		CChatGroup::TMemberCont::iterator itA;
		for( itA = itCl->second->getAudience().Members.begin();
		itA != itCl->second->getAudience().Members.end();
		++itA )
		{
			// ignore users in the excluded vector
			if ( std::find( excluded.begin(),excluded.end(), (*itA) ) == excluded.end() )
			{
				// the phrase
				uint32 sentId = STRING_MANAGER::sendStringToClient( *itA,phraseId.c_str(),params,&IosLocalSender );
				// send phrase id with an invalid sender, so that client dont display "toto says : toto bows"
				sendChat2Ex( CChatGroup::say, *itA, sentId, TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW ) );
			}
		}
		// restore old chat mode
		itCl->second->setChatMode( oldMode, oldChan );
	}
	else
	{
		nlwarning("<sendEmoteText> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}
}


//-----------------------------------------------
//		sendEmoteCustomTextToAll
//-----------------------------------------------
void CChatManager::sendEmoteCustomTextToAll( const TDataSetRow& sender, const ucstring & ustr )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
		// muted players can't do custom emotes
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUsers.find( eid ) != _MutedUsers.end())
		{
			nldebug("IOSCM:<CChatManager::sendEmoteCustomTextToAll> The player %s:%x is muted",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}

		// set the player chat mode and update its audience
		CChatGroup::TGroupType oldMode = itCl->second->getChatMode();
		TChanID	oldChan = itCl->second->getDynChatChan();
		itCl->second->setChatMode(CChatGroup::say);
		itCl->second->updateAudience(); // Use the say audience to get the correct members
		itCl->second->setChatMode(CChatGroup::arround);

		// get audience around the emoting player
		CChatGroup::TMemberCont::iterator itA;
/*		for( itA = itCl->second->getAudience().Members.begin();
		itA != itCl->second->getAudience().Members.end();
		++itA )
		{
			sendChatCustomEmote( sender, *itA, ustr );
		}
		*/
		chat(sender, ustr);
		// restore old chat mode
		itCl->second->setChatMode( oldMode, oldChan );
	}
	else
	{
		nlwarning("<sendEmoteCustomTextToAll> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
	}

}


//-----------------------------------------------
//	addDynStr
//
//-----------------------------------------------
//void CChatManager::addDynStr( const CEntityId& receiver, uint32 index, TServiceId frontendId )
//{
//	CDynamicStringInfos * infos = _DynDB.getInfos( index );
//	if( infos )
//	{
//		CMessage msgout( "IMPULS_CH_ID" );
//		CEntityId destId = receiver;
//		uint8 channel = 1;
//		msgout.serial( destId );
//		msgout.serial( channel );
//		CBitMemStream bms;
//
//		GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:ADD_DYN_STR", bms);
//
//		if( infos->IsHuffman )
//		{
//			bool huff = true;
//			bms.serialBit(huff);
//			bms.serial( index );
//			bms.serial( infos->Str );
//			vector<bool> code;
//			_DynDB.getHuffCode( infos->Str, code );
//			bms.serialCont( code );
//		}
//		else
//		{
//			bool huff = false;
//			bms.serialBit(huff);
//			bms.serial( index );
//			bms.serial( infos->Str );
//		}
//
////		nldebug("<CChatManager::addDynStr> sending association [%s,%d] to %s",infos->Str.c_str(),index,receiver.toString().c_str());
//		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
//		sendMessageViaMirror(frontendId, msgout);
//	}
//	else
//	{
//		nlwarning("<CChatManager::addDynStr> Can't find infos for string %d",index);
//	}
//
//
//} // addDynStr //



//-----------------------------------------------
//	sendChat
//
//-----------------------------------------------
void CChatManager::sendChat( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const ucstring& ucstr, const TDataSetRow &sender, TChanID chanID, const ucstring &senderName)
{


	if (senderChatMode == CChatGroup::arround)
	{
		sendChatCustomEmote(sender, receiver, ucstr );
		return;
	}

	//if( receiver != sender )
	{
		CCharacterInfos * charInfos = NULL;
		if( sender.isValid() /* != CEntityId::Unknown*/ )
		{
			charInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
			if( charInfos == NULL )
			{
				nlwarning("<CChatManager::chat> The character %s:%x is unknown, no chat msg sent",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex());
				return;
			}
		}
		CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
		if( receiverInfos )
		{
			TClientInfoCont::iterator itCl = _Clients.find( receiver );
			if( itCl != _Clients.end() )
			{
				if (itCl->second->getId().getType() == RYZOMID::player)
				{
					bool havePriv = false;
					if (charInfos && charInfos->HavePrivilege)
					{
						havePriv = true;
					}
					if ( ! havePriv && itCl->second->isInIgnoreList(sender))
					{
						return;
					}

					uint32 senderNameIndex;
					// if the sender exists
					if( charInfos )
					{
						senderNameIndex = charInfos->NameIndex;
					}
					else
					{
						// if no sender, we use a special name
						ucstring senderName("<BROADCAST MESSAGE>");
						senderNameIndex = SM->storeString( senderName );
					}

					if (!senderName.empty())
					{
						// the sender overloaded the name
						senderNameIndex = SM->storeString( senderName );
					}

					// send the string to FE
					CMessage msgout( "IMPULS_CH_ID" );
//					CEntityId& destId = receiver;
					uint8 channel = 1;
					CEntityId eid = TheDataset.getEntityId(receiver);
					msgout.serial( eid );
					msgout.serial( channel );
					CBitMemStream bms;
					GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:CHAT", bms );

					CChatMsg chatMsg;
					chatMsg.CompressedIndex = sender.getCompressedIndex();
					chatMsg.SenderNameId = senderNameIndex;
					chatMsg.ChatMode = (uint8) senderChatMode;
					if (senderChatMode == CChatGroup::dyn_chat)
					{
						chatMsg.DynChatChanID = chanID;
					}
					chatMsg.Content = ucstr;
					bms.serial( chatMsg );

	/*				nldebug("<CChatManager::sendChat> Sending dynamic chat '%s' from client %d to client %s with chat mode %d",
						chatMsg.Content.toString().c_str(),
						chatMsg.Sender,
						receiver.toString().c_str(),
						chatMsg.ChatMode);
	*/
					msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
					sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
				}
			}
			else
			{
				nlwarning("<CChatManager::sendChat> client %s:%x is unknown",
					TheDataset.getEntityId(receiver).toString().c_str(),
					receiver.getIndex());
			}
		}
		else
		{
			nlwarning("<CChatManager::chat> The character %s:%x is unknown, no chat msg sent",
				TheDataset.getEntityId(receiver).toString().c_str(),
				receiver.getIndex());
		}
	}

} // sendChat //

void CChatManager::sendFarChat(const string &name, const ucstring& ucstr, const string &chan)
{
	const TChanID *chanId = _ChanNames.getA(chan);
	if (chanId || chan == "universe" || chan.substr(0, 6) == "guild:")
	{

		string usedlang = "";
		string source_lang = "";
		string mongoText = ucstr.toUtf8();

		string::size_type endOfOriginal = mongoText.find("}@{");
		uint8 startPos = 0;
		if (mongoText.length() > 4 && mongoText[0] == ':' && mongoText[3] == ':') // check lang prefix
		{
			startPos = 4;
			double date = 1000.0*(double)CTime::getSecondsSince1970();
			string chatId;
			usedlang = mongoText.substr(1, 2);
			string rc_channel = "";
			if (chan == "FACTION_RF")
			{
				chatId = "FACTION_RF-"+toUpper(usedlang);
				rc_channel = "pub-forge-";
			}
			else if (chan == "universe")
			{
				chatId = "FACTION_"+toUpper(usedlang);
				rc_channel = "pub-uni-";
			}



			if (endOfOriginal != string::npos)
			{
				if (mongoText.size() > 9)
					source_lang = mongoText.substr(6, 2);
				else
					source_lang = usedlang;
				string sourceText = mongoText.substr(9, endOfOriginal-9);
				strFindReplace(sourceText, ")", "}");
				mongoText = "["+source_lang+"](http://chat.ryzom.com/channel/"+rc_channel+source_lang+"?%20"+encodeURIComponent(sourceText)+") "+mongoText.substr(endOfOriginal+4, mongoText.size()-endOfOriginal-4);
			}
			else
			{
				usedlang = mongoText.substr(1, 2);
				mongoText = mongoText.substr(4, mongoText.size()-4);
			}


			if (source_lang == "en") // in RC the icon are :gb:
				mongoText = ":gb:"+mongoText;
			else
				mongoText = ":"+source_lang+":"+mongoText;

#ifdef HAVE_MONGO
			if (endOfOriginal != string::npos)
				CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'dynamic', 'chatId': '%s', 'date': %f, 'ig': true }", CMongo::quote(name).c_str(), CMongo::quote(mongoText).c_str(), chatId.c_str(), date));
#endif
		}

		if (chan == "universe")
		{
			TGroupId grpId = CEntityId(RYZOMID::chatGroup, 0);
			farChatInGroup(grpId, 0, ucstr, ucstring("~")+ucstring(name));
		}
		else if (chan.substr(0, 6) == "guild:")
		{
			TGroupId groupId = CEntityId::Unknown;
			groupId.fromString(chan.substr(6).c_str());
			farChatInGroup(groupId, 0, ucstr, ucstring("~")+ucstring(name));
		}
		else
		{

			CDynChatSession *dcc = _DynChat.getChan(*chanId)->getFirstSession();
			while (dcc)
			{

				NLMISC::CEntityId receiverId = TheDataset.getEntityId(dcc->getClient()->getID());
				CCharacterInfos* co = IOS->getCharInfos(receiverId);
				if (!EnableDeepL || usedlang.empty() || (co != NULL && usedlang == SM->getLanguageCodeString(co->Language)))
					sendFarChat((CChatGroup::TGroupType)12, dcc->getClient()->getID(), ucstr.substr(startPos), ucstring("~")+ucstring(name), *chanId);
				dcc = dcc->getNextChannelSession(); // next session in this channel
			}
		}
	}
}

void CChatManager::sendFarChat( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const ucstring& ucstr, const ucstring &senderName, TChanID chanID)
{
	CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
	if( receiverInfos )
	{
		TClientInfoCont::iterator itCl = _Clients.find( receiver );
		if( itCl != _Clients.end() )
		{
			if (itCl->second->getId().getType() == RYZOMID::player)
			{
				uint32 senderNameIndex = SM->storeString( senderName );

				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
//					CEntityId& destId = receiver;
				uint8 channel = 1;
				CEntityId eid = TheDataset.getEntityId(receiver);
				msgout.serial( eid );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:CHAT", bms );

				CChatMsg chatMsg;
				chatMsg.CompressedIndex = 0xFFFFF;
				chatMsg.SenderNameId = senderNameIndex;
				chatMsg.ChatMode = (uint8) senderChatMode;
				if (senderChatMode == CChatGroup::dyn_chat)
				{
					chatMsg.DynChatChanID = chanID;
				}
				chatMsg.Content = ucstr;
				bms.serial( chatMsg );

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
			}
		}
		else
		{
			nlwarning("<CChatManager::sendChat> client %s:%x is unknown",
				TheDataset.getEntityId(receiver).toString().c_str(),
				receiver.getIndex());
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> The character %s:%x is unknown, no chat msg sent",
			TheDataset.getEntityId(receiver).toString().c_str(),
			receiver.getIndex());
	}

}


//-----------------------------------------------
//	sendChat2
//
//-----------------------------------------------
void CChatManager::sendChat2( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const std::string &phraseId, const TDataSetRow &sender )
{
	// send the chat phrase to the client
	TVectorParamCheck params;
	params.resize(1);
	params.back().Type = STRING_MANAGER::bot;
	params.back().setEId(  TheDataset.getEntityId(sender) );

	uint32 id = STRING_MANAGER::sendStringToClient(receiver, phraseId, params, &IosLocalSender);
	sendChat2Ex( senderChatMode, receiver, id, sender );
}


//-----------------------------------------------
//	sendChatParam
//
//-----------------------------------------------
void CChatManager::sendChatParam( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam>& params, const TDataSetRow &sender )
{
	TVectorParamCheck params2;
	params2.resize( params.size() + 1);
	// send the chat phrase to the client
	params2[0].Type = STRING_MANAGER::bot;
	params2[0].setEId(  TheDataset.getEntityId(sender) );
	uint32 first = 0, last = (uint32)params.size();
	for ( ; first != last ; ++first)
	{
		params2[first + 1] = params[first];
	}

	uint32 id = STRING_MANAGER::sendStringToClient(receiver, phraseId, params2, &IosLocalSender);
	sendChat2Ex( senderChatMode, receiver, id, sender );
}


//-----------------------------------------------
//	sendChat2Ex
//
//-----------------------------------------------
void CChatManager::sendChat2Ex( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, uint32 phraseId, const TDataSetRow &sender, ucstring customTxt )
{
	CCharacterInfos * charInfos = NULL;
	if( sender.isValid() /* != CEntityId::Unknown*/ )
	{
		charInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
		if( charInfos == NULL )
		{
			nlwarning("<CChatManager::sendChat2Ex> The character %s:%x is unknown, no chat msg sent",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex());
			return;
		}
	}
	CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
	if( receiverInfos )
	{
		TClientInfoCont::iterator itCl = _Clients.find( receiver );
		if( itCl != _Clients.end() )
		{
			if (itCl->second->getId().getType() == RYZOMID::player)
			{
				bool havePriv = false;
				if (charInfos && charInfos->HavePrivilege)
				{
					havePriv = true;
				}
				if ( ! havePriv && itCl->second->isInIgnoreList(sender))
				{
					return;
				}

				// send the chat phrase to the client
				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
				CEntityId destId = receiverInfos->EntityId;
				uint8 channel = 1;
				msgout.serial( destId );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:CHAT2", bms );

				CChatMsg2 chatMsg;
				chatMsg.CompressedIndex = sender.getCompressedIndex();
				chatMsg.SenderNameId = charInfos ? charInfos->NameIndex : 0; // empty string if there is no sender
				chatMsg.ChatMode = (uint8) senderChatMode;
				chatMsg.PhraseId = phraseId;
				chatMsg.CustomTxt = customTxt;
				bms.serial( chatMsg );

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				CUnifiedNetwork::getInstance()->send(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
			}
		}
		else
		{
			nlwarning("<CChatManager::sendChat2Ex> client %s:%x is unknown",
				TheDataset.getEntityId(receiver).toString().c_str(),
				receiver.getIndex());
		}
	}
	else
	{
		nlwarning("<CChatManager::sendChat2Ex> The character %s:%x is unknown, no chat msg sent",
			TheDataset.getEntityId(receiver).toString().c_str(),
			receiver.getIndex());
	}
}//	sendChat2Ex


//-----------------------------------------------
//	sendChatCustomEmote
//
//-----------------------------------------------
void CChatManager::sendChatCustomEmote( const TDataSetRow &sender, const TDataSetRow &receiver, const ucstring& ucstr )
{
	TDataSetRow senderFake = TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW );

	CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
	CCharacterInfos * senderInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
	if( receiverInfos )
	{
		TClientInfoCont::iterator itCl = _Clients.find( receiver );
		if( itCl != _Clients.end() )
		{
			if (itCl->second->getId().getType() == RYZOMID::player)
			{
				bool havePriv = false;
				if (senderInfos && senderInfos->HavePrivilege)
				{
					havePriv = true;
				}
				if ( ! havePriv && itCl->second->isInIgnoreList(sender))
				{
					return;
				}

				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
				uint8 channel = 1;
				CEntityId eid = TheDataset.getEntityId(receiver);
				msgout.serial( eid );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:CHAT", bms );

				CChatMsg chatMsg;
				chatMsg.CompressedIndex = senderFake.getCompressedIndex();
				chatMsg.SenderNameId = 0;
				chatMsg.ChatMode = (uint8) CChatGroup::say;
				chatMsg.Content = ucstr;
				bms.serial( chatMsg );

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
			}
		}
		else
		{
			nlwarning("<CChatManager::sendChatCustomEmote> client %s:%x is unknown",
				TheDataset.getEntityId(receiver).toString().c_str(),
				receiver.getIndex());
		}
	}
	else
	{
		nlwarning("<CChatManager::chat> The character %s:%x is unknown, no chat msg sent",
			TheDataset.getEntityId(receiver).toString().c_str(),
			receiver.getIndex());
	}

} // sendChatCustomEmote //


//-----------------------------------------------
//	tell
//
//-----------------------------------------------
void CChatManager::tell2( const TDataSetRow& sender, const TDataSetRow& receiver, const string& phraseId )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl == _Clients.end() )
	{
		nlwarning("<CChatManager::tell> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
		return;
	}
	CCharacterInfos * senderInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
	if( senderInfos == 0 )
	{
		nlwarning("<CChatManager::tell> The sender %s:%x is unknown, no tell message sent",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
		return;
	}

//	bool senderMuted = itCl->second->isMuted();
	bool senderMuted = _MutedUsers.find(TheDataset.getEntityId(sender)) != _MutedUsers.end();
	bool receiverMuted = _MutedUsers.find(TheDataset.getEntityId(receiver)) != _MutedUsers.end();

	CCharacterInfos * receiverInfos = IOS->getCharInfos( TheDataset.getEntityId(receiver) );
	if( receiverInfos )
	{
		if(	senderMuted && receiverInfos->HavePrivilege == false )
		{
			nldebug("IOSCM: tell2 The player %s:%x is muted and %s have no privilege",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex(),
				TheDataset.getEntityId(receiver).toString().c_str() );
			return;
		}
		itCl = _Clients.find( receiverInfos->DataSetIndex );
		if( itCl != _Clients.end() )
		{
			if( receiverMuted && senderInfos->HavePrivilege == false )
			{
				nldebug("IOSCM: tell2 The player %s:%x have no privilege and %s is muted",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex(),
					TheDataset.getEntityId(receiver).toString().c_str() );
				return;
			}

			// check if the sender is CSR or is not in the ignore list of the receiver
			if(senderInfos->HavePrivilege || !itCl->second->isInIgnoreList(sender) )
			{
				// send the chat phrase to the client
				TVectorParamCheck params;
				params.resize(1);
				params.back().Type = STRING_MANAGER::bot;
				params.back().setEId( TheDataset.getEntityId(sender));
				uint32 id = STRING_MANAGER::sendStringToClient(receiver, phraseId, params, &IosLocalSender);

				CMessage msgout( "IMPULS_CH_ID" );
				uint8 channel = 1;
				msgout.serial( receiverInfos->EntityId );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:TELL2", bms);

				bms.serial( senderInfos->NameIndex );
				bms.serial( id);

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				CUnifiedNetwork::getInstance()->send(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);
			}
		}
		else
		{
			nlwarning("<CChatManager::tell> client %s:%x is unknown",
				TheDataset.getEntityId(itCl->first).toString().c_str(),
				itCl->first.getIndex());
		}
	}
	else
	{
		nlwarning("<CChatManager::tell> The receiver %s:%x is unknown, no tell message sent",
			TheDataset.getEntityId(receiver).toString().c_str(),
			receiver.getIndex());
	}
} // tell2 //


//-----------------------------------------------
//	tell
//
//-----------------------------------------------
void CChatManager::tell( const TDataSetRow& sender, const string& receiverIn, const ucstring& ucstr )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl == _Clients.end() )
	{
		nlwarning("<CChatManager::tell> client %s:%x is unknown",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
		return;
	}
	CCharacterInfos * senderInfos = IOS->getCharInfos( TheDataset.getEntityId(sender) );
	if( senderInfos == 0 )
	{
		nlwarning("<CChatManager::tell> The sender %s:%x is unknown, no tell message sent",
			TheDataset.getEntityId(sender).toString().c_str(),
			sender.getIndex());
		return;
	}
//	bool senderMuted = itCl->second->isMuted();
	bool senderMuted = _MutedUsers.find(TheDataset.getEntityId(sender)) != _MutedUsers.end();

	// manage domain wide addressing
	string receiver;
	TSessionId receiverSessionId;

	CShardNames::getInstance().parseRelativeName(senderInfos->HomeSessionId, receiverIn, receiver, receiverSessionId);

	receiver = CShardNames::getInstance().makeFullName(receiver, receiverSessionId);
	CCharacterInfos * receiverInfos = IOS->getCharInfos( receiver );

	if( receiverInfos && !ForceFarChat)
	{
		bool receiverMuted = _MutedUsers.find(TheDataset.getEntityId(receiverInfos->DataSetIndex)) != _MutedUsers.end();
		if(	senderMuted && receiverInfos->HavePrivilege == false )
		{
			nldebug("IOSCM: tell The player %s:%x is muted and %s have no privilege",
				TheDataset.getEntityId(sender).toString().c_str(),
				sender.getIndex(),
				receiver.c_str());
			return;
		}
		itCl = _Clients.find( receiverInfos->DataSetIndex );
		if( itCl != _Clients.end() )
		{
			if( receiverMuted && senderInfos->HavePrivilege == false )
			{
				nldebug("IOSCM: tell The player %s:%x have no privilege and %s is muted",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex(),
					receiver.c_str());
				return;
			}

			// check if the sender is not in the ignore list of the receiver
			if(senderInfos->HavePrivilege || !itCl->second->isInIgnoreList(sender) )
			{
				// check if user is afk
				if ( receiverInfos->DataSetIndex.isValid() && TheDataset.isDataSetRowStillValid( receiverInfos->DataSetIndex ) )
				{
					CMirrorPropValue<uint16> mirrorValue( TheDataset, receiverInfos->DataSetIndex, DSPropertyCONTEXTUAL );
					CProperties prop(mirrorValue);
					if ( prop.afk() )
					{
						// send special message to user
						SM_STATIC_PARAMS_1( vect, STRING_MANAGER::player );
						vect[0].setEId( receiverInfos->EntityId );
						uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_AFK", vect, &IosLocalSender );
						sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId, TDataSetRow(), receiverInfos->AfkCustomTxt );
					}
					if ( _UsersIgnoringTells.find( receiverInfos->EntityId ) != _UsersIgnoringTells.end() )
					{
						// send special message to user (same message as if the receiver was offline)
						SM_STATIC_PARAMS_1( vect, STRING_MANAGER::literal );
						vect[0].Literal = ucstring( receiver );
						uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_UNKNOWN", vect, &IosLocalSender );
						sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId );
						return;
					}
				}

				// info for log the chat message
				//string senderName = senderInfos->Name.toString(); // removed by ulukyn to prevent crash
/*
				{
					if (senderInfos == NULL)
					{
						senderName = TheDataset.getEntityId(sender).toString();
					}
					else
						senderName = senderInfos->Name.toString();
				}
*/

				// info for log the chat message
				//string receiverName = receiverInfos->Name.toString();  // removed by ulukyn to prevent crash
/*
				{
					CCharacterInfos *ci = IOS->getCharInfos(senderInfos->EntityId);
					if (ci == NULL)
					{
						receiverName = receiverInfos->EntityId.toString();
					}
					else
						receiverName = receiverInfos->Name.toString();
				}
*/

				//_Log.displayNL("'%s' to '%s' (%s) : \t\"%s\"", senderName.c_str(), receiverName.c_str(), "tell", ucstr.toString().c_str() );  // removed by ulukyn to prevent crash


				// if the client doesn't know this dynamic string(name of sender), we send it to him
				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
				uint8 channel = 1;
				msgout.serial( receiverInfos->EntityId );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:TELL", bms);

				TDataSetIndex dsi = senderInfos->DataSetIndex.getCompressedIndex();
				bms.serial( dsi );
				bms.serial( senderInfos->NameIndex );
				bms.serial( const_cast<ucstring&>(ucstr) );

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);

				// log tell to PDS
//				IOSPD::logTell(ucstr, senderInfos->EntityId, receiverInfos->EntityId);
				log_Chat_Tell(senderInfos->EntityId, receiverInfos->EntityId, ucstr.toUtf8());
			}
		}
		else
		{
			nlwarning("<CChatManager::tell> client %s:%x is unknown",
				TheDataset.getEntityId(itCl->first).toString().c_str(),
				itCl->first.getIndex());
		}
	}
	else
	{
		// look for named group chat
		std::map<TStringId, TGroupId>::iterator it(_GroupNames.find(CStringMapper::map(receiver)));
		if (it != _GroupNames.end())
		{
			// we found one
			if(	senderMuted )
			{
				nldebug("IOSCM: tell The player %s:%x is muted, can't tell a group chat",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex());
					return;
			}

			CChatGroup &chatGroup = _Groups[it->second];

			//chatInGroup(it->second, str, sender);	// removed, checked after

			/// check that sender is in this group
			if (chatGroup.Members.find(sender) != chatGroup.Members.end())
			{
				chatInGroup(it->second, ucstr, sender);

				// log tell to PDS
//				IOSPD::logTell(ucstr, senderInfos->EntityId, it->second);
				log_Chat_Tell(senderInfos->EntityId, it->second, ucstr.toUtf8());
			}
			else
			{
				// ERROR : not in this chat group !!!!!
				nlwarning("<CChatManager::tell> The receiver %s is unknown, no tell message sent",receiver.c_str());

				SM_STATIC_PARAMS_1( vect, STRING_MANAGER::literal );
				vect[0].Literal = ucstring( receiver );
				uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_UNKNOWN", vect, &IosLocalSender );
				sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId );
			}
		}
		else if (IChatUnifierClient::getInstance() != NULL)
		{
			if(	senderMuted && receiverInfos && receiverInfos->HavePrivilege == false )
			{
				nldebug("IOSCM: tell The player %s:%x is muted and %s have no privilege",
					TheDataset.getEntityId(sender).toString().c_str(),
					sender.getIndex(),
					receiver.c_str());
				return;
			}
			// there is no named group chat, try to send to SU
			// send the tell to the chat unifier for eventual dispatch
			ucstring destName;
			destName.fromUtf8(receiver);
			IChatUnifierClient::getInstance()->sendFarTell(senderInfos->EntityId, senderInfos->HavePrivilege, destName, ucstr);
		}
		else
		{
			SM_STATIC_PARAMS_1( vect, STRING_MANAGER::literal );
			vect[0].Literal = ucstring( receiver );
			uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_UNKNOWN", vect, &IosLocalSender );
			sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId );
		}
	}
} // tell //


void CChatManager::farTell( const NLMISC::CEntityId &senderCharId, const ucstring &senderName, bool havePrivilege, const ucstring& receiver, const ucstring& ucstr  )
{
	if (receiver[0] == '~')
	{
		double date = 1000.0*(double)CTime::getSecondsSince1970();

		string username = IOS->getRocketName(senderName);

		string chatId = receiver.toString().substr(1);
		ucstring::size_type pos = chatId.find('(');
		if (pos != string::npos)
			chatId = chatId.substr(0, pos);

#ifdef HAVE_MONGO
		CMongo::insert("ryzom_chats", toString("{ 'username': '%s', 'chat': '%s', 'chatType': 'username', 'chatId': '%s', 'date': %f, 'ig': true }", CMongo::quote(username).c_str(), CMongo::quote(ucstr.toUtf8()).c_str(), chatId.c_str(), date));
#endif

/*
		bson base;
		bson_init( &base );
			bson_append_string( &base, "username", username.c_str() );
			bson_append_string( &base, "chat", ucstr.toUtf8().c_str() );
			bson_append_string( &base, "chatType", "username" );
			bson_append_string( &base, "chatId", chatId.c_str());
			bson_append_double( &base, "date", date );
			bson_append_bool( &base, "ig", true );
		bson_finish( &base );

		if (mongo_insert( &conn, mongo_col.c_str(), &base, 0 ) != MONGO_OK)
		{
			nlwarning("MongoDB : Error inserting data: %d %d", conn.err, conn.errcode);
		}

		bson_destroy ( &base );
*/
		/////
		return;
	}

	CCharacterInfos * receiverInfos = IOS->getCharInfos( receiver );
	if( receiverInfos )
	{
		TClientInfoCont::iterator itCl = _Clients.find( receiverInfos->DataSetIndex );
		if( itCl != _Clients.end() )
		{
			bool receiverMuted = _MutedUsers.find(receiverInfos->EntityId) != _MutedUsers.end();
			if( receiverMuted && havePrivilege == false )
			{
				nldebug("IOSCM: tell The player %s have no privilege and %s is muted",
					senderName.toUtf8().c_str(),
					receiver.toUtf8().c_str());
				return;
			}

			CCharacterInfos * senderInfos = IOS->getCharInfos(senderName);
			// check if the sender is CSR is not in the ignore list of the receiver
			if((senderInfos && senderInfos->HavePrivilege) || !itCl->second->isInIgnoreList(senderCharId) )
			{
				// check if user is afk
//				if ( receiverInfos->DataSetIndex.isValid() && TheDataset.isDataSetRowStillValid( receiverInfos->DataSetIndex ) )
//				{
//					CMirrorPropValue<uint16> mirrorValue( TheDataset, receiverInfos->DataSetIndex, DSPropertyCONTEXTUAL );
//					CProperties prop(mirrorValue);
//					if ( prop.afk() )
//					{
//						// send special message to user
//						SM_STATIC_PARAMS_1( vect, STRING_MANAGER::player );
//						vect[0].setEId( receiverInfos->EntityId );
//						uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_AFK", vect, &IosLocalSender );
//						sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId );
//						return;
//					}
//					if ( _UsersIgnoringTells.find( receiverInfos->EntityId ) != _UsersIgnoringTells.end() )
//					{
//						// send special message to user (same message as if the receiver was offline)
//						SM_STATIC_PARAMS_1( vect, STRING_MANAGER::literal );
//						vect[0].Literal = ucstring( receiver );
//						uint32 phraseId = STRING_MANAGER::sendStringToClient( senderInfos->DataSetIndex, "TELL_PLAYER_UNKNOWN", vect, &IosLocalSender );
//						sendChat2Ex( CChatGroup::tell, senderInfos->DataSetIndex, phraseId );
//						return;
//					}
//				}

				// info for log the chat message
//				string senderName = senderInfos->Name.toString();

				// info for log the chat message
				string receiverName = receiverInfos->Name.toString();

				_Log.displayNL("'%s' to '%s' (%s) : \t\"%s\"", senderName.toUtf8().c_str(), receiverName.c_str(), "tell", ucstr.toString().c_str() );


				// if the client doesn't know this dynamic string(name of sender), we send it to him
				// send the string to FE
				CMessage msgout( "IMPULS_CH_ID" );
				uint8 channel = 1;
				msgout.serial( receiverInfos->EntityId );
				msgout.serial( channel );
				CBitMemStream bms;
				GenericXmlMsgHeaderMngr.pushNameToStream( "STRING:FAR_TELL", bms);

				CFarTellMsg ftm;
				ftm.SenderName = senderName;
				ftm.Text = ucstr;
				ftm.serial(bms);

				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror(TServiceId(receiverInfos->EntityId.getDynamicId()), msgout);

				// log tell to PDS
//				IOSPD::logTell(ucstr, senderCharId, receiverInfos->EntityId);
				log_Chat_Tell(senderCharId, receiverInfos->EntityId, ucstr.toUtf8());
			}
		}
		else
		{
			// no chat unifier, so we can't dispatch
			nlwarning("<CChatManager::tell> client %s:%x is unknown",
				senderCharId.toString().c_str(),
				itCl->first.getIndex());
		}
	}
} // tell //

/*
 * Display the list of clients
 */
void CChatManager::displayChatClients(NLMISC::CLog &log)
{
	TClientInfoCont::iterator im;
	for ( im=_Clients.begin(); im!=_Clients.end(); ++im )
	{
		CCharacterInfos *ci = IOS->getCharInfos(im->second->getId());
		if (ci != NULL)
		{
			if (ci->EntityId.getType() == RYZOMID::player)
				log.displayNL("'%s' %s:%x %s mode '%s'",
					ci->Name.toString().c_str(),
					ci->EntityId.toString().c_str(),
					im->first.getIndex(),
					im->second->isMuted()?"(muted)":"",
					CChatGroup::groupTypeToString(im->second->getChatMode()).c_str() );
		}
		else
		{
			log.displayNL("*no name* %s:%x %s mode '%s'",
				ci->EntityId.toString().c_str(),
				im->first.getIndex(),
				im->second->isMuted()?"(muted)":"",
				CChatGroup::groupTypeToString(im->second->getChatMode()).c_str() );
		}
	}
}


void CChatManager::displayChatGroup(NLMISC::CLog &log, TGroupId gid, CChatGroup &chatGroup)
{
	if (chatGroup.GroupName == CStringMapper::emptyId())
	{
		log.displayNL("Group : anonym (%s), %s : %u clients :",
				gid.toString().c_str(),
				CChatGroup::groupTypeToString(chatGroup.Type).c_str(),
				chatGroup.Members.size());
	}
	else
	{
		log.displayNL("Group : '%s' (%s), %s : %u clients :",
				CStringMapper::unmap(chatGroup.GroupName).c_str(),
				gid.toString().c_str(),
				CChatGroup::groupTypeToString(chatGroup.Type).c_str(),
				chatGroup.Members.size());
	}

	CChatGroup::TMemberCont::iterator first(chatGroup.Members.begin()), last(chatGroup.Members.end());
	for (; first != last; ++first)
	{
		CEntityId eid = TheDataset.getEntityId(*first);
		if (eid.getType() == RYZOMID::player)
		{
			CCharacterInfos *ci = IOS->getCharInfos(TheDataset.getEntityId(*first));
			if (ci != NULL)
				log.displayNL("  '%s' %s:%x",
					ci->Name.toString().c_str(),
					ci->EntityId.toString().c_str(),
					first->getIndex());
			else
				log.displayNL("   *unknow* %s:%x",
					eid.toString().c_str(),
					first->getIndex());
		}
	}
}

void CChatManager::displayChatGroups(NLMISC::CLog &log, bool displayUniverse, bool displayPlayerAudience)
{
	std::map< TGroupId, CChatGroup >::iterator first(_Groups.begin()), last(_Groups.end());

	for (; first != last; ++first)
	{
		CChatGroup &cg = first->second;

		if ((displayUniverse || cg.Type != CChatGroup::universe)
			&& (cg.Type != CChatGroup::say)
			&& (cg.Type != CChatGroup::shout)
			)
		{
			displayChatGroup(log, first->first, cg);
		}
	}

	if (displayPlayerAudience)
	{
		TClientInfoCont::iterator first(_Clients.begin()), last(_Clients.end());
		for (; first != last; ++first)
		{
			CChatClient *cc = first->second;
			if (cc->getId().getType() == RYZOMID::player)
			{
				log.displayNL ("Chat group for player %s :", cc->getId().toString().c_str());
				displayChatGroup(log, cc->getSayAudienceId(), cc->getSayAudience());
				displayChatGroup(log, cc->getShoutAudienceId(), cc->getShoutAudience());
			}
		}
	}
}

void CChatManager::displayChatAudience(NLMISC::CLog &log, const CEntityId &eid, bool updateAudience)
{
	CCharacterInfos *ci = IOS->getCharInfos(eid);
	if (ci == NULL)
	{
		log.displayNL("Unknown client id '%s'", eid.toString().c_str());
		return;
	}

	TClientInfoCont::iterator it(_Clients.find(ci->DataSetIndex));
	if (it != _Clients.end())
	{
		CChatClient &cc = *(it->second);

		if (updateAudience)
			cc.updateAudience();

		CChatGroup &cg = cc.getAudience();

		if (cg.Type == CChatGroup::say)
			log.displayNL("Client '%s' (%u) say audience:", eid.toString().c_str(), ci->DataSetIndex.getIndex());
		else if (cg.Type == CChatGroup::shout)
			log.displayNL("Client '%s' (%u) shout audience:", eid.toString().c_str(), ci->DataSetIndex.getIndex());
		else
		{
			log.displayNL("Client '%s' (%u) invalide local chat mode !", eid.toString().c_str(), ci->DataSetIndex.getIndex());
			return;
		}


		CChatGroup::TMemberCont::iterator first(cg.Members.begin()), last(cg.Members.end());
		for (; first != last; ++first)
		{
			CEntityId eid = TheDataset.getEntityId(*first);
			if (eid.getType() == RYZOMID::player)
			{
				CCharacterInfos *ci = IOS->getCharInfos(TheDataset.getEntityId(*first));
				if (ci != NULL)
					log.displayNL("  '%s' %s:%x",
						ci->Name.toString().c_str(),
						TheDataset.getEntityId(*first).toString().c_str(),
						first->getIndex());

				else
					log.displayNL("   *unknow* %s:%x",
						TheDataset.getEntityId(*first).toString().c_str(),
						first->getIndex());
			}
		}
	}
	else
	{
		log.displayNL("The client '%s' is not in the chat manager client list", eid.toString().c_str());
	}
}

void CChatManager::sendHistoric(const TDataSetRow &receiver, TChanID chanID)
{
	CDynChatChan *chan = _DynChat.getChan(chanID);
	if (!chan)
	{
		nlwarning("Unknown chan");
		return;
	}
	for(uint k = 0; k < chan->Historic.getSize(); ++k)
	{
//		sendChat(CChatGroup::dyn_chat, receiver, chan->Historic[k].String, chan->Historic[k].Sender, chanID);
		sendChat(CChatGroup::dyn_chat, receiver, chan->Historic[k].String, TDataSetRow(), chanID, chan->Historic[k].SenderString);
	}
}



ucstring CChatManager::filterClientInputColorCode(ucstring &text)
{
	ucstring result;
	result.reserve(text.size());

	ucstring::size_type pos = 0;

	for (; pos < text.size(); ++pos)
	{
		if (text[pos] == '@' && pos < text.size()-1 && text[pos+1] == '{')
		{
			continue;
		}
		else
		{
			// authorized char
			result += text[pos];
		}
	}

	return result;
}

ucstring CChatManager::filterClientInput(ucstring &text)
{
	ucstring result;
	result.reserve(text.size());
	// 1st, remove any beginning or ending white space
	ucstring::size_type pos = 0;

	// skip begin white spaces or : (used by deepl)
	while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\t'))
		++pos;

	// remove ending white space
	while (text.size() > 0 && (*(text.rbegin()) == ' ' || *(text.rbegin()) == '\t'))
		text.resize(text.size()-1);

	if (pos+3 < text.size() && text[pos] == ':' && text[pos+3] == ':')
		++pos;

	if (pos+1 < text.size() && text[pos] == '>' && text[pos+1] == ':')
		pos += 2;

	// copy string, removing multi white space between words
	// filter out color code
	bool lastIsWhite = false;
	for (; pos < text.size(); ++pos)
	{
		bool currentIsWhite = (text[pos] == ' ' || text[pos] == '\t');
		if (!(lastIsWhite && currentIsWhite))
		{
			// any double white skipped
			if (text[pos] == '&')
			{
				// Special case if there is <NEW> or <CHG> at the beginning
				bool hasBrackets = false;
				if (pos >= 5)
				{
					hasBrackets = (text[pos-1] == '>') &&
						(text[pos-5] == '<');
				}
				// Filter out '&' at the first non-whitespace position to remove
				// system color code (like '&SYS&' )
				bool disallowAmpersand = (result.size() == 0) || hasBrackets;
				if (disallowAmpersand)
				{
					result += '.';
				}
				else
				{
					// authorized ampersand
					result += '&';
				}
			}
			else if (text[pos] == '@' && pos < text.size()-1 && text[pos+1] == '{')
			{
				// filter out any match of '@{' to remove color tag (like '@{rgba}')
				result += '.';
			}
			else
			{
				// authorized char
				result += text[pos];
			}
		}
		lastIsWhite = currentIsWhite;
	}

	return result;
}


/// Subscribe special ring users in the ring universe chat
void CChatManager::subscribeCharacterInRingUniverse(const NLMISC::CEntityId &charEId)
{
	// create a fake eid
	TDataSetRow dsr = TheDataset.getDataSetRow(charEId);
	BOMB_IF (!dsr.isValid(), "CChatManager::subscribeCharacterInRingUniverse : the char "<<charEId.toString()<<" is not in the mirror", return);

	// add player in the group universe
	TGroupId grpUniverse = CEntityId(RYZOMID::chatGroup,0);
	addToGroup(grpUniverse, dsr);
}

/// Unsubscribe special ring users in the ring universe chat
void CChatManager::unsubscribeCharacterInRingUniverse(const NLMISC::CEntityId &charEId)
{
	// create a fake eid
	TDataSetRow dsr = TheDataset.getDataSetRow(charEId);
	if (dsr.isValid())
	{
		// remove player of the group universe
		TGroupId grpUniverse = CEntityId(RYZOMID::chatGroup,0);
		removeFromGroup(grpUniverse, dsr);
	}
}

/// Update from input_output_service.cpp to check MongoDb changes
void CChatManager::update()
{
#ifdef HAVE_MONGO
	try {
		TTime before = CTime::getLocalTime();
		CUniquePtr<DBClientCursor> cursor = CMongo::query("ryzom_chats", toString("{'date': { $gt: %f }, 'ig': false }", last_mongo_chat_date));
		TTime after = CTime::getLocalTime();

		if(!cursor.get()) return;

		while (cursor->more())
		{
			mongo::BSONObj obj = cursor->next();
			nlinfo("mongo: new entry to parse '%s'", obj.jsonString().c_str());

			string name;
			string chat;
			string chatType;
			string chatId;
			double date;
			bool ig;

			name = obj.getStringField("username");
			chat = obj.getStringField("chat");
			chatType = obj.getStringField("chatType");
			chatId = obj.getStringField("chatId");
			date = obj.getField("date").numberDouble();

			if(date > last_mongo_chat_date)
				last_mongo_chat_date = date;

			ig = obj.getBoolField("ig");
			if(ig) continue;

			ucstring text;
			text.fromUtf8(chat);

			TGroupId grpId = CEntityId(RYZOMID::chatGroup, 0);

			if (chatType == "guildId") // Chat avec la guilde
			{
				uint32 guildId;
				NLMISC::fromString(chatId, guildId);

				grpId.setShortId(guildId + 0x10000000);  // 0x10000000 is the GuildBase to chat group id
				grpId.setDynamicId(0);
				grpId.setCreatorId(0);
			}
			else if (chatType == "dynamic")
			{
				if (EnableDeepL && chatId.size() >= 10 && (chatId.substr(0, 11) == "FACTION_RF-" || chatId == "FACTION_EN" || chatId == "FACTION_DE" || chatId == "FACTION_FR" || chatId == "FACTION_ES" || chatId == "FACTION_RU"))
				{
					string usedlang;
					if (chatId.substr(0, 11) == "FACTION_RF-")
						usedlang = chatId.substr(11, 2);
					else
						usedlang = chatId.substr(8, 2);

					_Log.displayNL("%s|%s|*|%s-*|%s", chatId.c_str(), string("~"+name).c_str(), toLower(usedlang).c_str(), text.toUtf8().c_str());
				}
				else
				{

					// broadcast to other client in the channel
					const TChanID *chanId = _ChanNames.getA(chatId);
					if (chanId)
					{
						CDynChatSession *dcc = _DynChat.getChan(*chanId)->getFirstSession();
						while (dcc)
						{
							sendFarChat((CChatGroup::TGroupType)12, dcc->getClient()->getID(), text, ucstring("~")+ucstring(name), *chanId);
							dcc = dcc->getNextChannelSession(); // next session in this channel
						}
					}
				}
					// void CChatManager::sendFarChat( C const ucstring& ucstr, const ucstring &senderName, TChanID chanID)
					continue;
			}
			else if (chatType == "univers") {
				// Send to Deepl
				if (EnableDeepL)
				{
					_Log.displayNL("%s|%s|wk|wk-*|%s", "universe", name.c_str(), chat.c_str());
					continue;
				}
			}
			else if (chatType == "username")
			{
				if (IService::getInstance()->getShardId() == 501)
					chatId = chatId+"(Gingo)";
				else
					chatId = chatId+"(Atys)";

				farTell(CEntityId(uint64(0)), ucstring("~")+ucstring(name), false, ucstring(chatId), text);
				continue;
			}
			farChatInGroup(grpId, 0, text, ucstring("~")+ucstring(name));
		}
	}
	catch(const DBException& e)
	{
		cout << "caught DBException " << e.toString() << endl;
	}
#endif
}

TChanID CChatManager::getChanId(const string name) {
	return *_ChanNames.getA(name);
}
