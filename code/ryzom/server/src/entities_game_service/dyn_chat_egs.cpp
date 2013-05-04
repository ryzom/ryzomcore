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
#include "dyn_chat_egs.h"
#include "entity_manager/entity_manager.h"
#include "entity_manager/entity_base.h"
#include "player_manager/character.h"
//
#include "nel/net/message.h"

// dyn chat egs instance
CDynChatEGS DynChatEGS;

using namespace std;
using namespace NLMISC;
using namespace NLNET;


// helper : serialize a TDataSetRow from a const reference
static inline void serialDSR(NLMISC::IStream &f, const TDataSetRow &dsr)
{
	nlassert(!f.isReading());
	TDataSetRow dsrCopy = dsr;
	f.serial(dsrCopy);
}

//============================================================================================================
CDynChatEGS::CDynChatEGS() : _NextChanID(RYZOMID::dynChatGroup, UINT64_CONSTANT(0))
{
}

//============================================================================================================
void CDynChatEGS::init()
{
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "DYN_CHAT:ADD_SERVICE_CHAN",		CDynChatEGS::cbServiceAddChan			},
		{ "DYN_CHAT:SET_HIDE_BUBBLE",		CDynChatEGS::cbServiceSetHideBubble		},
		{ "DYN_CHAT:SET_UNIVERSAL_CHANNEL",	CDynChatEGS::cbServiceSetUniversalChannel},
		{ "DYN_CHAT:SET_CHAN_HISTORY",		CDynChatEGS::cbServiceSetChanHistory	},
		{ "DYN_CHAT:REMOVE_SERVICE_CHAN",	CDynChatEGS::cbServiceRemoveChan		},
		{ "DYN_CHAT:ADD_CLIENT",			CDynChatEGS::cbServiceAddClient			},
		{ "DYN_CHAT:ADD_SESSION",			CDynChatEGS::cbServiceAddSession		},
		{ "DYN_CHAT:ADD_SESSION_ENTITY",	CDynChatEGS::cbServiceAddSessionEntity	},
		{ "DYN_CHAT:REMOVE_SESSION",		CDynChatEGS::cbServiceRemoveSession		},
		{ "DYN_CHAT:REMOVE_SESSION_ENTITY",	CDynChatEGS::cbServiceRemoveSessionEntity}
	};

	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
	
}


//============================================================================================================
TChanID CDynChatEGS::addChan(const std::string &name, const ucstring &title, TChanID chan, bool noBroadcast, bool forwardPlayerInputs, bool unify)
{
	return addChan(name, title, false, chan, noBroadcast, forwardPlayerInputs, unify);
}


//============================================================================================================
TChanID CDynChatEGS::addLocalizedChan(const std::string &name, TChanID chan, bool noBroadcast, bool forwardPlayerInputs, bool unify)
{
	return addChan(name, ucstring(""), true, chan, noBroadcast, forwardPlayerInputs, unify);
}

//============================================================================================================
TChanID CDynChatEGS::addChan(const std::string &name, const ucstring &title, bool localized, TChanID chan, bool noBroadcast, bool forwardPlayerInputs, bool unify)
{
	if (name.empty()) return DYN_CHAT_INVALID_CHAN;
	if (getChanIDFromName(name) != DYN_CHAT_INVALID_CHAN)
	{
		nlwarning("CDynChatEGS::addChan : channel '%s' already exist, can't create another one", name.c_str());
		return DYN_CHAT_INVALID_CHAN;
	}

	if (chan != CEntityId::Unknown)
	{
		// force the channel id
		if (_DynChat.addChan(chan, noBroadcast, forwardPlayerInputs, unify))
		{
			_DynChat.getChan(chan)->Localized = localized;
			_DynChat.getChan(chan)->Title = title;
			iosAddChan(chan, noBroadcast, forwardPlayerInputs, unify);
			_ChanNames.add(chan, name);
			return chan;
		}
		else
		{
			return DYN_CHAT_INVALID_CHAN;

		}
	}
	else if (_DynChat.addChan(_NextChanID, false, false, false))
	{
		TChanID result = _NextChanID;
		iosAddChan(_NextChanID, noBroadcast, forwardPlayerInputs, unify);
		_DynChat.getChan(_NextChanID)->Localized = localized;
		_DynChat.getChan(_NextChanID)->Title = title;
		_ChanNames.add(_NextChanID, name);
		_NextChanID.setShortId(_NextChanID.getShortId()+1);
		if (_NextChanID == DYN_CHAT_INVALID_CHAN) 
			_NextChanID.setShortId(_NextChanID.getShortId()+1);
		return result;
	}
	else
	{
		return DYN_CHAT_INVALID_CHAN;
	}
}

//============================================================================================================
TChanID CDynChatEGS::getChanIDFromName(const std::string &name) const
{
	const TChanID *chan = _ChanNames.getA(name);
	return chan ? *chan : DYN_CHAT_INVALID_CHAN;
}

//============================================================================================================
const std::string &CDynChatEGS::getChanNameFromID(TChanID chan) const
{
	static std::string emptyString;
	const std::string *stringName = _ChanNames.getB(chan);
	return stringName ? *stringName : emptyString;
}

//============================================================================================================
uint CDynChatEGS::getSessionCount(TChanID chanID)
{
	CDynChatChan *chan = _DynChat.getChan(chanID);
	if( chan )
	{
		return chan->getSessionCount();
	}
	else
	{
		nlwarning("<CDynChatEGS::getSessionCount> can't get channel %s", chanID.toString().c_str());
		return 0;
	}
}


//============================================================================================================
bool CDynChatEGS::removeChan(TChanID chanID)
{		
	// remove channel from all clients database
	CDynChatChan *chan = _DynChat.getChan(chanID);
	if (!chan) return false;
	CDynChatSession *currSession = chan->getFirstSession();
	while (currSession)
	{
		CEntityBase *eb = CEntityBaseManager::getEntityBasePtr(currSession->getClient()->getID());
		CCharacter *ch = dynamic_cast<CCharacter *>(eb);
		if (ch)
		{		
			ch->removeDynChatChan(chanID);
		}
		currSession = currSession->getNextChannelSession();
	}
	_DynChat.removeChan(chanID);
	if (_ChanNames.getB(chanID)) 
		_ChanNames.removeWithA(chanID);
	// send msg to IOS
	iosRemoveChan(chanID);
	return true;
}

//============================================================================================================
bool CDynChatEGS::addClient(const TDataSetRow &client)
{
	return _DynChat.addClient(client);
}

//============================================================================================================
bool CDynChatEGS::removeClient(const TDataSetRow &client)
{
	return _DynChat.removeClient(client);
}

//============================================================================================================
bool CDynChatEGS::addSession(TChanID chan, const TDataSetRow &client, bool writeRight)
{
	CEntityBase *eb = CEntityBaseManager::getEntityBasePtr(client);
	if (!eb) 
	{
		return false;
	}
	CDynChatSession *session = _DynChat.addSession(chan, client);
	if (!session)
	{ 
		nlwarning("error adding session for %s client!",client.toString().c_str());
		return false;
	}
	iosAddSession(chan, client, writeRight);
	const std::string *chanName = _ChanNames.getB(chan);
	if (chanName)
	{
		if (!session->getChan()->Localized)
		{			
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal= session->getChan()->Title;
			session->StringID = STRING_MANAGER::sendStringToClient(client, "LITERAL", params);
		}
		else
		{
			TVectorParamCheck params;
			// send name of channel to client			
			session->StringID = STRING_MANAGER::sendStringToClient(client, chanName->c_str(), params);
		}
	}
	session->WriteRight = writeRight;
	// add session in character	
	CCharacter *ch = dynamic_cast<CCharacter *>(eb);
	if (ch)
	{		
		ch->setDynChatChan(chan, session->StringID, writeRight);	
	}
	return true;
}

//============================================================================================================
bool CDynChatEGS::removeSession(TChanID chan,const TDataSetRow &client)
{
	CEntityBase *eb = CEntityBaseManager::getEntityBasePtr(client);
	if (!eb) return false;
	if (!_DynChat.removeSession(chan, client)) return false;
	iosRemoveSession(chan, client);
	// add session in character	
	CCharacter *ch = dynamic_cast<CCharacter *>(eb);
	if (ch)
	{		
		ch->removeDynChatChan(chan);
	}
	return true;
}

//============================================================================================================
bool CDynChatEGS::getPlayersInChan(TChanID chanID, std::vector<NLMISC::CEntityId> &players)
{
	CDynChatChan *chan = _DynChat.getChan(chanID);
	if (!chan) return false;	
		
	CDynChatSession *currSession = chan->getFirstSession();
	bool havePlayers = false;
	while (currSession)
	{
		players.push_back(TheDataset.getEntityId(currSession->getClient()->getID()));
		havePlayers = true;
		currSession = currSession->getNextChannelSession();
	}
	return havePlayers;
}


//============================================================================================================
void CDynChatEGS::iosSetHistoricSize(TChanID chan, uint32 size)
{
	CMessage msg("DYN_CHAT:SET_HISTORIC_SIZE");
	msg.serial(chan);	
	msg.serial(size);
	sendMessageViaMirror( "IOS", msg);
}

//============================================================================================================
void CDynChatEGS::setHistoricSize(TChanID chanID, uint32 size)
{
	CDynChatChan *chan = _DynChat.getChan(chanID);
	if (!chan) return;	
	chan->HistoricSize = size;
	iosSetHistoricSize(chanID, size);
}

//============================================================================================================
bool CDynChatEGS::setWriteRight(TChanID chan, const TDataSetRow &client, bool writeRight)
{
	CEntityBase *eb = CEntityBaseManager::getEntityBasePtr(client);
	if (!eb) return false;
	CDynChatSession *session = _DynChat.getSession(chan, client);
	if (!session) return false;
	if (writeRight == session->WriteRight) return true; // already good value
	session->WriteRight = writeRight;
	iosSetReadOnlyFlag(chan, client, writeRight);	
	CCharacter *ch = dynamic_cast<CCharacter *>(eb);
	if (ch)
	{		
		ch->setDynChatChan(chan, session->StringID, writeRight);	
	}
	return true;
}


//============================================================================================================
bool CDynChatEGS::setHideBubble(TChanID chanID, bool hideBubble)
{
	
	CDynChatChan *chan = _DynChat.getChan(chanID);
	if (!chan) return false;
	if (hideBubble == chan->HideBubble) return true; // already good value
	chan->HideBubble = hideBubble;
	iosSetHideBubble(chanID, hideBubble);		
	return true;
}

//============================================================================================================
bool CDynChatEGS::setUniversalChannel(TChanID chanID, bool universalChannel)
{
	CDynChatChan *chan = _DynChat.getChan(chanID);
	if (!chan) return false;
	if (universalChannel == chan->UniversalChannel) return true; // already good value
	chan->UniversalChannel = universalChannel;
	iosSetUniversalChannel(chanID, universalChannel);
	return true;
}

//============================================================================================================
void CDynChatEGS::cbServiceAddChan(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;
	string		chanName;
	bool		noBroadcast;
	bool		forwardPlayerInputs;

	msgin.serial(chan);
	msgin.serial(chanName);
	msgin.serial(noBroadcast);
	msgin.serial(forwardPlayerInputs);

	DynChatEGS.addLocalizedChan(chanName, chan, noBroadcast, forwardPlayerInputs);
}

//============================================================================================================
void CDynChatEGS::cbServiceSetHideBubble(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;	
	bool		hideBubble;
	
	msgin.serial(chan);
	msgin.serial(hideBubble);		
	DynChatEGS.setHideBubble(chan, hideBubble);
}
//============================================================================================================
void CDynChatEGS::cbServiceSetUniversalChannel(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;
	bool		universalChannel;

	msgin.serial(chan);
	msgin.serial(universalChannel);
	DynChatEGS.setUniversalChannel(chan, universalChannel);
}
//============================================================================================================
void CDynChatEGS::cbServiceAddClient(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TDataSetRow dsr;
	msgin.serial(dsr);
	DynChatEGS.addClient(dsr);
}
//============================================================================================================
void CDynChatEGS::cbServiceRemoveChan(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;
	msgin.serial(chan);

	DynChatEGS.removeChan(chan);
}
//============================================================================================================
void CDynChatEGS::cbServiceSetChanHistory(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;
	uint32		history;

	msgin.serial(chan);
	msgin.serial(history);

	DynChatEGS.setHistoricSize(chan, history);
}
//============================================================================================================
void CDynChatEGS::cbServiceAddSession(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;
	TDataSetRow	client;
	bool		writeRight;

	msgin.serial(chan);
	msgin.serial(client);
	msgin.serial(writeRight);

	if(!DynChatEGS.addSession(chan, client, writeRight))
		nlwarning("unable to add %s in %s channel!",client.toString().c_str(),chan.toString().c_str());

}
void CDynChatEGS::cbServiceAddSessionEntity(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;
	TDataSetRow	client;
	CEntityId	clientEntity;
	bool		writeRight;

	msgin.serial(chan);
	msgin.serial(clientEntity);
	msgin.serial(writeRight);
	client = TheDataset.getDataSetRow(clientEntity);
	if(!DynChatEGS.addSession(chan, client, writeRight))
		nlwarning("unable to add %s in %s channel!",client.toString().c_str(),chan.toString().c_str());

}
//============================================================================================================
void CDynChatEGS::cbServiceRemoveSession(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;
	TDataSetRow	client;

	msgin.serial(chan);
	msgin.serial(client);

	DynChatEGS.removeSession(chan, client);
}

void CDynChatEGS::cbServiceRemoveSessionEntity(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	TChanID		chan;
	TDataSetRow	client;
	CEntityId	clientEntity;
	msgin.serial(chan);
	msgin.serial(clientEntity);
	client = TheDataset.getDataSetRow(clientEntity);
	DynChatEGS.removeSession(chan, client);
}
//============================================================================================================
void CDynChatEGS::onServiceDown(NLNET::TServiceId serviceId)
{
	vector<CDynChatChan *>	channels;
	vector<TChanID>	toRemove;
	_DynChat.getChans(channels);

	// look for channel created by the service that close down
	for (uint i=0; i<channels.size(); ++i)
	{
		if (channels[i]->getID().getCreatorId() == serviceId.get())
			toRemove.push_back(channels[i]->getID());
	}

	// remove the channels
	while (!toRemove.empty())
	{
		removeChan(toRemove.back());
		toRemove.pop_back();
	}
}



//============================================================================================================
void CDynChatEGS::iosAddChan(TChanID chan, bool noBroadcast, bool forwardPlayerInputs, bool unify)
{
	CMessage msg("DYN_CHAT:ADD_CHAN");
	msg.serial(chan);	
	msg.serial(noBroadcast);
	msg.serial(forwardPlayerInputs);
	msg.serial(unify);

	sendMessageViaMirror( "IOS", msg);
}

//============================================================================================================
void CDynChatEGS::iosRemoveChan(TChanID chan)
{	
	CMessage msg("DYN_CHAT:REMOVE_CHAN");
	msg.serial(chan);	
	sendMessageViaMirror( "IOS", msg);
}

//============================================================================================================
void CDynChatEGS::iosAddSession(TChanID chan, const TDataSetRow &client, bool writeRight)
{
	CMessage msg("DYN_CHAT:ADD_SESSION");
	msg.serial(chan);	
	serialDSR(msg, client);
	msg.serial(writeRight);
	sendMessageViaMirror( "IOS", msg);
}

//============================================================================================================
void CDynChatEGS::iosSetHideBubble(TChanID chan, bool hideBubble)
{
	CMessage msg("DYN_CHAT:SET_HIDE_BUBBLE");
	msg.serial(chan);	
	msg.serial(hideBubble);		
	sendMessageViaMirror( "IOS", msg);
}

//============================================================================================================
void CDynChatEGS::iosSetUniversalChannel(TChanID chan, bool universalChannel)
{
	CMessage msg("DYN_CHAT:SET_UNIVERSAL_CHANNEL");
	msg.serial(chan);	
	msg.serial(universalChannel);
	sendMessageViaMirror( "IOS", msg);
}

//============================================================================================================
void CDynChatEGS::iosRemoveSession(TChanID chan, const TDataSetRow &client)
{
	CMessage msg("DYN_CHAT:REMOVE_SESSION");
	msg.serial(chan);
	serialDSR(msg, client);
	sendMessageViaMirror( "IOS", msg);
}

//============================================================================================================
void CDynChatEGS::iosSetReadOnlyFlag(TChanID chan, const TDataSetRow &client, bool writeRight)
{
	CMessage msg("DYN_CHAT:SET_WRITE_RIGHT");
	msg.serial(chan);
	serialDSR(msg, client);
	msg.serial(writeRight);
	sendMessageViaMirror( "IOS", msg);
}

//============================================================================================================
void CDynChatEGS::iosResetDynChat()
{
	CMessage msg("DYN_CHAT:RESET");	
	sendMessageViaMirror( "IOS", msg);
}


//============================================================================================================
void CDynChatEGS::iosConnection()
{
	iosResetDynChat();
	CDynChat::TChanPtrVector chans;
	_DynChat.getChans(chans);
	for(uint k = 0; k < chans.size(); ++k)
	{			
		iosAddChan(chans[k]->getID(), chans[k]->getDontBroadcastPlayerInputs(), chans[k]->getForwardPlayerIntputToOwnerService(), chans[k]->getUnifiedChannel());
		// add each session in the channel
		CDynChatSession *currSession = chans[k]->getFirstSession();
		while (currSession)
		{
			iosAddSession(chans[k]->getID(), currSession->getClient()->getID(), currSession->WriteRight);
			iosSetHistoricSize(chans[k]->getID(), chans[k]->HistoricSize);
			currSession = currSession->getNextChannelSession();
		}
	}
}

