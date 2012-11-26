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
#include "dyn_chat.h"

using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CDynChatSession);

uint CDynChatSession::_NumSessions = 0;


/////////////////////
// CDynChatSession //
/////////////////////
//================================================================
CDynChatSession::CDynChatSession(CDynChatClient *client, CDynChatChan *channel)
							    :	StringID(0),
								WriteRight(false),
								_Client(client),
								_Channel(channel)
{
	nlassert(client);
	nlassert(channel);
	// insert at head of client linked list of session
	_NextClientSession = client->_FirstSession;
	if (client->_FirstSession)
	{
		nlassert(client->_FirstSession->_PrevClientSession == &client->_FirstSession);
		client->_FirstSession->_PrevClientSession = &_NextClientSession;
	}
	_PrevClientSession = &client->_FirstSession;
	client->_FirstSession = this;
	// insert at head of channel linked list of session
	_NextChannelSession = channel->_FirstSession;
	if (channel->_FirstSession)
	{
		nlassert(channel->_FirstSession->_PrevChannelSession == &channel->_FirstSession);
		channel->_FirstSession->_PrevChannelSession = &_NextChannelSession;
	}
	_PrevChannelSession = &channel->_FirstSession;
	channel->_FirstSession = this;

	++ _NumSessions;

}

//================================================================
void CDynChatSession::unlink()
{
	nlassert(_Client);  // object already unlinked ?
	nlassert(_Channel);
	nlassert(_PrevClientSession);
	// client
	*_PrevClientSession = _NextClientSession;
	if (_NextClientSession)
	{
		nlassert(_NextClientSession->_PrevClientSession = &_NextClientSession);
		_NextClientSession->_PrevClientSession = _PrevClientSession;
	}
	_PrevClientSession = NULL;
	_NextClientSession = NULL;
	// channel
	*_PrevChannelSession = _NextChannelSession;
	if (_NextChannelSession)
	{
		nlassert(_NextChannelSession->_PrevChannelSession = &_NextChannelSession);
		_NextChannelSession->_PrevChannelSession = _PrevChannelSession;
	}
	_PrevChannelSession = NULL;
	_NextChannelSession = NULL;
	_Client = NULL;
	_Channel = NULL;
}

//================================================================
CDynChatSession::~CDynChatSession()
{
	nlassert(!_Client); // unlink() hasn't been called
	nlassert(!_Channel);
	nlassert(!_PrevClientSession);
	nlassert(!_NextClientSession);
	nlassert(_NumSessions > 0); // check for memory leaks
	--_NumSessions;
}

////////////////////
// CDynChatClient //
////////////////////
//================================================================
CDynChatClient::CDynChatClient(const TDataSetRow &client) : _FirstSession(NULL), _ID(client)
{
}

//================================================================
CDynChatClient::~CDynChatClient()
{
	// remove all sessions
	CDynChatSession *currSession = _FirstSession;
	while (currSession)
	{
		CDynChatSession *tmpSession = currSession;
		currSession = currSession->getNextClientSession();
		tmpSession->unlink();
		delete tmpSession;
	}
}

//================================================================
CDynChatSession *CDynChatClient::getSession(TChanID chan) const
{
	CDynChatSession *currSession = _FirstSession;
	while (currSession)
	{
		if (currSession->getChan()->getID() == chan) break;
		currSession = currSession->getNextClientSession();
	}
	return currSession;
}

//////////////////
// CDynChatChan //
//////////////////
CDynChatChan::CDynChatChan()
	:	HistoricSize(0),
		HideBubble(false),
		UniversalChannel(false),
		_FirstSession(NULL),
		_ID(CEntityId::Unknown),
		_DontBroadcastPlayerInputs(false),
		_ForwardPlayerIntputToOwnerService(false),
		_UnifyChannel(false)
{
}

//================================================================
//CDynChatChan::CDynChatChan(TChanID id) : _ID(id), _FirstSession(NULL), HistoricSize(0)
CDynChatChan::CDynChatChan(TChanID id, bool noBroadcast, bool forwardInput, bool unified)
	:	HistoricSize(0),
		HideBubble(false),
		UniversalChannel(false),
		_FirstSession(NULL),
		_ID(id),
		_DontBroadcastPlayerInputs(noBroadcast),
		_ForwardPlayerIntputToOwnerService(forwardInput),
		_UnifyChannel(unified)
{
}

//================================================================
CDynChatChan::~CDynChatChan()
{
	// remove all sessions
	CDynChatSession *currSession = _FirstSession;
	while (currSession)
	{
		CDynChatSession *tmpSession = currSession;
		currSession = currSession->getNextChannelSession();
		tmpSession->unlink();
		delete tmpSession;
	}
}

//================================================================
uint CDynChatChan::getSessionCount() const
{
	uint count = 0;
	// remove all sessions
	CDynChatSession *currSession = _FirstSession;
	while (currSession)
	{
		++ count;
		currSession = currSession->getNextChannelSession();
	}
	return count;
}

//////////////
// CDynChat //
//////////////


//================================================================
bool CDynChat::addChan(TChanID chan, bool noBroadcast, bool forwardInput, bool unify)
{
	if (_Chans.count(chan)) return false;
	_Chans[chan] = CDynChatChan(chan, noBroadcast, forwardInput, unify);
	return true;
}

//================================================================
bool CDynChat::removeChan(TChanID chan)
{
	TChanMap::iterator it = _Chans.find(chan);
	if (it != _Chans.end())
	{
		_Chans.erase(it);
		return true;
	}
	else
	{
		return false;
	}
}

//================================================================
bool CDynChat::addClient(const TDataSetRow &client)
{
	if (_Clients.count(client)) return false;
	_Clients[client] = CDynChatClient(client);
	return true;
}

//================================================================
bool CDynChat::removeClient(const TDataSetRow &client)
{
	TClientMap::iterator it = _Clients.find(client);
	if (it != _Clients.end())
	{
		_Clients.erase(it);
		return true;
	}
	else
	{
		return false;
	}
}

//================================================================
CDynChatSession *CDynChat::addSession(TChanID chanID, const TDataSetRow &clientID)
{
	CDynChatChan *chan = getChan(chanID);
	if (!chan)
		return NULL;
	CDynChatClient *client = getClient(clientID);
	if (!client)
		return NULL;
	// look for channel in session (faster because there are few channels used by a single player)
	CDynChatSession *session = client->getSession(chanID);
	if (session)
	{
		nlwarning("Session already created for player %s in channel %p", clientID.toString().c_str(), chan);
		return NULL;
	}
	CDynChatSession *newSession = new CDynChatSession(client, chan);
	return newSession;
}

//================================================================
bool CDynChat::removeSession(TChanID chanID, const TDataSetRow &clientID)
{
	CDynChatClient *client = getClient(clientID);
	if (!client)
	{
		nlwarning("Client %s unknown", clientID.toString().c_str());
		return false;
	}
	// look for channel in session (faster because there are few channels used by a single player)
	CDynChatSession *session = client->getSession(chanID);
	if (!session)
	{
		nlwarning("Channel %s unknown", chanID.toString().c_str());
		return false;
	}
	session->unlink();
	delete session;
	return true;
}

//================================================================
CDynChatChan *CDynChat::getChan(TChanID chan)
{
	TChanMap::iterator it = _Chans.find(chan);
	if (it != _Chans.end()) return &(it->second);
	return NULL;
}

//================================================================
CDynChatClient *CDynChat::getClient(const TDataSetRow &client)
{
	TClientMap::iterator it = _Clients.find(client);
	if (it != _Clients.end()) return &(it->second);
	return NULL;
}

//================================================================
CDynChat::CDynChat()
{
}

//================================================================
CDynChat::~CDynChat()
{
	_Chans.clear();
	_Clients.clear();
}

//================================================================
CDynChatSession *CDynChat::getSession(TChanID chan, const TDataSetRow &client)
{
	CDynChatClient *clientPtr = getClient(client);
	if (!clientPtr) return NULL;
	return clientPtr->getSession(chan);
}

//================================================================
void CDynChat::getChans(std::vector<CDynChatChan *> &channels)
{
	uint numChans = (uint)_Chans.size();
	channels.resize(numChans);
	uint k = 0;
	for(TChanMap::iterator it = _Chans.begin(); it != _Chans.end(); ++it, ++k)
	{
		channels[k] = &(it->second);
	}
}




