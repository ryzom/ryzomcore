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

#include <nel/misc/command.h>

//#include "game_share/generic_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/synchronised_message.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/properties.h"
#include "game_share/backup_service_interface.h"

#include "server_share/r2_variables.h"
#include "server_share/log_chat_gen.h"

#include "chat_manager.h"
#include "input_output_service.h"
#include "chat_unifier_client.h"

//#include "ios_pd.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


void	logChatDirChanged(IVariable &var)
{
	// LogChatDirectory variable changed, reset it!
	//IOS->getChatManager().resetChatLog();
}

CVariable<bool>			VerboseChatManagement("ios","VerboseChatManagement", "Set verbosity for chat management", false, 0, true);
CVariable<std::string>	LogChatDirectory("ios", "LogChatDirectory", "Log Chat directory (default, unset is SaveFiles service directory", "", 0, true, logChatDirChanged);
CVariable<bool>			ForceFarChat("ios","ForceFarChat", "Force the use of SU to dispatch chat", false, 0, true);




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
void	CChatManager::resetChatLog()
{
	std::string	logPath = (LogChatDirectory.get().empty() ? Bsi.getLocalPath() : LogChatDirectory.get());
	_Displayer.setParam(CPath::standardizePath(logPath) + "chat.log");
}


bool CChatManager::checkClient( const TDataSetRow& id )
{
	TClientInfoCont::iterator itCl = _Clients.find( id );

	return itCl != _Clients.end();
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
//	chat
//
//-----------------------------------------------
void CChatManager::chat( const TDataSetRow& sender, const ucstring& ucstr )
{
	TClientInfoCont::iterator itCl = _Clients.find( sender );
	if( itCl != _Clients.end() )
	{
//		if( itCl->second->isMuted() )
		CEntityId eid = TheDataset.getEntityId(sender);
		if(_MutedUniverseUsers.find( eid ) != _MutedUniverseUsers.end())
		{
			nldebug("IOSCM:  chat The player %s:%x is universe muted",
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
		{
			// ignore muted users
//			if ( _MutedUsers.find( eid ) != _MutedUsers.end() )
//				return;

			if (ci == NULL)
			{
				senderName = TheDataset.getEntityId(sender).toString();
			}
			else
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

		switch( itCl->second->getChatMode() )
		{
			// dynamic group
		case CChatGroup::shout :
		case CChatGroup::say :
			{
				CChatGroup::TMemberCont::iterator itA;
				for( itA = itCl->second->getAudience().Members.begin();
						itA != itCl->second->getAudience().Members.end();
							++itA )
				{
					string				receiverName;
					NLMISC::CEntityId	receiverId = TheDataset.getEntityId(*itA);
					CCharacterInfos*	ci = IOS->getCharInfos(receiverId);

					_DestUsers.push_back(receiverId);

					if (ci == NULL)
					{
						receiverName = receiverId.toString();
					}
					else
					{
						receiverName = ci->Name.toString();
					}

					_Log.displayNL("'%s' to '%s' (%s) : \t\"%s\"", senderName.c_str(), receiverName.c_str(), groupNames[itCl->second->getChatMode()], ucstr.toString().c_str() );

					sendChat( itCl->second->getChatMode(), *itA, ucstr, sender );
				}
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

				_Log.displayNL("'%s' (%s) : \t\"%s\"", senderName.c_str(), groupNames[itCl->second->getChatMode()], ucstr.toString().c_str() );
				chatInGroup( grpId, ucstr, sender );
				break;
			}
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

				_Log.displayNL("'%s' (%s) : \t\"%s\"", senderName.c_str(), groupNames[itCl->second->getChatMode()], ucstr.toString().c_str() );
				_DestUsers.push_back(grpId);

				chatInGroup( grpId, ucstr, sender );
			}
			break;
		case CChatGroup::team:
			{
				TGroupId grpId = itCl->second->getTeamChatGroup();
				_DestUsers.push_back(grpId);

				_Log.displayNL("'%s' (%s) : \t\"%s\"", senderName.c_str(), groupNames[itCl->second->getChatMode()], ucstr.toString().c_str() );
				chatInGroup( grpId, ucstr, sender );
			}
			break;
		case CChatGroup::guild:
			{
				TGroupId grpId = itCl->second->getGuildChatGroup();
				_DestUsers.push_back(grpId);

				_Log.displayNL("'%s' (%s) : \t\"%s\"",
					senderName.c_str(),
					groupNames[itCl->second->getChatMode()],
					ucstr.toString().c_str() );
				chatInGroup( grpId, ucstr, sender );
			}
			break;
		case CChatGroup::dyn_chat:
		{
			TChanID chanID = itCl->second->getDynChatChan();
			CDynChatSession *session = _DynChat.getSession(chanID, sender);
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
							sendChat(itCl->second->getChatMode(), dcc->getClient()->getID(), content, sender, chanID);
							dcc = dcc->getNextChannelSession(); // next session in this channel
						}
					}
					else
					{
						// only send an echo to the sender
						sendChat(itCl->second->getChatMode(), itCl->first, ucstr, sender, chanID);
					}
					if (session->getChan()->getForwardPlayerIntputToOwnerService())
					{
						// send player input to service owner
						NLNET::TServiceId serviceId(chanID.getCreatorId());

						TPlayerInputForward	pif;
						pif.ChanID = chanID;
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
		for( itM = chatGrp.Members.begin(); itM != chatGrp.Members.end(); ++itM )
		{
			CMirrorPropValueRO<uint32> instanceId( TheDataset, *itM, DSPropertyAI_INSTANCE );

			// check the ai instance for region chat
			if (chatGrp.Type != CChatGroup::region
				|| instanceId == senderInstanceId)
			{
				// check homeSessionId for universe
				if (/*IsRingShard && */chatGrp.Type == CChatGroup::universe)
				{
					CCharacterInfos *senderChar = IOS->getCharInfos(TheDataset.getEntityId(sender));
					CCharacterInfos *receiverChar = IOS->getCharInfos(TheDataset.getEntityId(*itM));

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
					sendChat( itGrp->second.Type, *itM, ucstr, sender );
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
					IChatUnifierClient::getInstance()->sendFarGuildChat(charInfos->Name, uint32(grpId.getShortId()), ucstr);
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
					IChatUnifierClient::getInstance()->sendUniverseChat(charInfos->Name, sessionId, ucstr);
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
		CChatGroup &chatGrp = itGrp->second;
		CChatGroup::TMemberCont::const_iterator itM;
		for( itM = chatGrp.Members.begin(); itM != chatGrp.Members.end(); ++itM )
		{
			if (homeSessionId != 0)
			{
				CCharacterInfos *charInfo = IOS->getCharInfos(TheDataset.getEntityId(*itM));
				if (charInfo==NULL)
					continue;

				// determine the session id as the home session id for normal players and the current session id for GMs
				uint32 sessionId= (charInfo->HavePrivilege && !IsRingShard)? IService::getInstance()->getShardId(): (uint32)charInfo->HomeSessionId;

				// check that the dest has the same home as sender
				if (sessionId != homeSessionId)
					continue;
			}
			sendFarChat( itGrp->second.Type, *itM, text, senderName );
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
		itCl->second->updateAudience();

		// get audience around the emoting player
		CChatGroup::TMemberCont::iterator itA;
		for( itA = itCl->second->getAudience().Members.begin();
		itA != itCl->second->getAudience().Members.end();
		++itA )
		{
			sendChatCustomEmote( sender, *itA, ustr );
		}
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
				string senderName = senderInfos->Name.toString();
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
				string receiverName = receiverInfos->Name.toString();
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

				_Log.displayNL("'%s' to '%s' (%s) : \t\"%s\"", senderName.c_str(), receiverName.c_str(), "tell", ucstr.toString().c_str() );


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

	// skip begin white spaces
	while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\t'))
		++pos;

	// remove ending white space
	while (text.size() > 0 && (*(text.rbegin()) == ' ' || *(text.rbegin()) == '\t'))
		text.resize(text.size()-1);

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


