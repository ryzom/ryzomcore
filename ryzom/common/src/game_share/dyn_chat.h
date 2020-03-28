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

#ifndef DYN_CHAN_H
#define DYN_CHAN_H


#include "nel/misc/entity_id.h"
#include "nel/misc/historic.h"
#include "base_types.h"

// forward declarations
class CDynChatClient;
class CDynChatChan;


// identifier of a dynchat channel
//typedef uint32 TChanID;
typedef NLMISC::CEntityId	TChanID;

// tag for invalid channel
const TChanID DYN_CHAT_INVALID_CHAN = NLMISC::CEntityId::Unknown;
const std::string DYN_CHAT_INVALID_NAME;


/// Message payload for player input forward to channel service owner
struct TPlayerInputForward
{
	TChanID		ChanID;
	TDataSetRow	Sender;
	ucstring	Content;

	void serial(NLMISC::IStream &f)
	{
		f.serial(ChanID);
		f.serial(Sender);
		f.serial(Content);
	}
};

//===================================================================================================================
/** This class represents an interaction between a client and a channel in a dynamic chat
  * A session is initiated from a CDynChat instance.
  */
class CDynChatSession
{
	NL_INSTANCE_COUNTER_DECL(CDynChatSession);
public:
	uint32							StringID; // string ID for EGS (to fill client DB)
	bool							WriteRight; // in read-only mode client can't talk in the channel
public:
	// Get next session for client
	CDynChatSession       *getNextClientSession()   { return _NextClientSession; }
	// Get next session for channel
	CDynChatSession       *getNextChannelSession()  { return _NextChannelSession; }
	// Get the client taking part to this session
	CDynChatClient		  *getClient()				{ return _Client; }
	// Get the channel hosting that session
	CDynChatChan		  *getChan()			    { return _Channel; }
private:
	CDynChatClient	*_Client;
	CDynChatChan	*_Channel;
	CDynChatSession	*_NextClientSession;
	CDynChatSession	**_PrevClientSession;  // Pointer on 'next' field in previous client session, or start of linked list
	CDynChatSession	*_NextChannelSession;
	CDynChatSession	**_PrevChannelSession; // Pointer on 'next' field in previous channel session, or start of linked list
	// remove the session
	void unlink();
	// ctor
	CDynChatSession(CDynChatClient *client, CDynChatChan *channel);
	~CDynChatSession();
	friend class CDynChat;
	friend class CDynChatClient;
	friend class CDynChatChan;
	static uint _NumSessions;

};

//===================================================================================================================
/** A client in a dyn chat system. Should be created from a CDynChat instance.
  */
class CDynChatClient
{
public:
	// ctor
	CDynChatClient(const TDataSetRow &client = TDataSetRow());
	~CDynChatClient();
	// Head of linked list of sessions for this channel. List must be followed by calling CDynChatSession::getNextClientSession.
	CDynChatSession			 *getFirstSession() { return _FirstSession; }
	const TDataSetRow		 &getID() const { return _ID; }
	// Return session for this client in channel 'chan', or NULL if no such session exists
	CDynChatSession			 *getSession(TChanID chan) const;
private:
	CDynChatSession   *_FirstSession;
	TDataSetRow		   _ID;
	friend class CDynChatSession;
	friend class CDynChat;
};





//===================================================================================================================
/** A channel in a dyn chat system. Should be created from a CDynChat instance.
  */
class CDynChatChan
{
public:
	struct CHistoricEntry
	{
		ucstring	String;
//		TDataSetRow Sender;
		ucstring	SenderString;
	};
	NLMISC::CHistoric<CHistoricEntry>		Historic;		// historic of messages for IOS
	uint									HistoricSize;   // Historic size for EGS
	bool									Localized;      // for EGS only
	ucstring								Title;          // gives the title of the channel when it is not translated (e.g Localized == false)
	bool									HideBubble;		// hide the display of bubble
	bool									UniversalChannel;	// treat like universe channel
public:
	CDynChatChan();
//	CDynChatChan(TChanID id = NLMISC::CEntityId::Unknown, NLNET::TServiceId ownerServiceId, bool noBroadcast, bool forwadInput);
	CDynChatChan(TChanID id, bool noBroadcast, bool forwardInput, bool unified);
	~CDynChatChan();
	// Get ID for that channel
	TChanID			getID() const { return _ID; }
	// Head of linked list of sessions for this channel. List must be followed by calling CDynChatSession::getNextChannelSession.
	CDynChatSession *getFirstSession() { return _FirstSession; }
	// Count number of session in that channel (in O(n))
	uint			 getSessionCount() const;

	bool			getDontBroadcastPlayerInputs()	{ return _DontBroadcastPlayerInputs; }
	bool			getForwardPlayerIntputToOwnerService() { return _ForwardPlayerIntputToOwnerService;}
	bool			getUnifiedChannel()				{ return _UnifyChannel;}

private:
	CDynChatSession *_FirstSession;
	TChanID			 _ID;

	/// Flag to disable client broadcasting
	bool				_DontBroadcastPlayerInputs;
	/// Flag to activate player input forwarding to owner service.
	bool				_ForwardPlayerIntputToOwnerService;
	/// Flag to activate unification (domain wide) of this channel
	bool				_UnifyChannel;

	friend class CDynChatSession;
	friend class CDynChat;
};





//===================================================================================================================
/** Data structure to store clients / channels / sessions
  * To be used by both IOS & EGS
  *
  * \TODO : Would be good to generalize the pattern used as something like NLMISC::CBinaryRelation<CompoundA, CompoundB, Relation>
  *         This would avoid to have fields for one service that aren't used by another (like the historic...)
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
class CDynChat
{
private:
	typedef std::map<TChanID, CDynChatChan>			TChanMap;
	typedef std::map<TDataSetRow, CDynChatClient>	TClientMap;
public:
	// ctor
	CDynChat();
	// dtor
	~CDynChat();
	/** Create a new channel with the given ID. A warning is issued if channel already exists.
	  * \return true if creation succeeded
	  */
	bool						addChan(TChanID chan, bool noBroadcast, bool forwardInput, bool unify);
	// Remove channel with the given ID. Return true if success
	bool						removeChan(TChanID chan);
	// Add a client with the given ID. Return true if success
	bool						addClient(const TDataSetRow &client);
	// Remove client from its ID. Return true if success
	bool						removeClient(const TDataSetRow &client);
	/** Create a new session in channel 'chan' for the client 'client'. No-op if session already exists
	  * \return pointer on created session
	  */
	CDynChatSession				*addSession(TChanID chan, const TDataSetRow &client);
	// Stop session in channel 'chan' for the client 'client'.
	bool						removeSession(TChanID chan, const TDataSetRow &client);
	// Get channel object from its ID or NULL if not found.
	CDynChatChan				*getChan(TChanID chan);
	// Get client object from its ID or NULL if not found.
	CDynChatClient				*getClient(const TDataSetRow &client);
	// Get a session or NULL if not found
	CDynChatSession				*getSession(TChanID chan, const TDataSetRow &client);
	// Get channels
	typedef std::vector<CDynChatChan *> TChanPtrVector;
	void						getChans(TChanPtrVector &channels);
	// Remove all channels
	void						removeAllChannels() { _Chans.clear(); }
private:
	//
	TChanMap   _Chans;
	TClientMap _Clients;
};




#endif

