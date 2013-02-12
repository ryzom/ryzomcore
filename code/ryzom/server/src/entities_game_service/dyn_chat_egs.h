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

#ifndef RY_DYN_CHAT_EGS_H
#define RY_DYN_CHAT_EGS_H


#include "game_share/dyn_chat.h"
#include "nel/misc/twin_map.h"



/** Dyn chat EGS
  *
  * Single access point to manage dyn chat in EGS.  
  * This class takes care of updating IOS & each character DB (which reflect the available channels)
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004  
  */
class CDynChatEGS
{
public:
	// ctor
	CDynChatEGS();

	// init : register callback
	void init();

	/** Create a new channel with localized name.
	  * If channel already exist, DYN_CHAT_INVALID_CHAN is returned
	  */
	TChanID				addChan(const std::string &name, const ucstring &title, TChanID = NLMISC::CEntityId::Unknown, bool noBroadcast = false, bool forwardPlayerInputs = false, bool unify = false);
	// Add of channel whose name is its string id
	TChanID				addLocalizedChan(const std::string &name, TChanID = NLMISC::CEntityId::Unknown, bool noBroadcast = false, bool forwardPlayerInputs = false, bool unify = false);
	// Retrieves id of a channel from its localized name, or DYN_CHAT_INVALID_CHAN if not such channel exists
	TChanID				getChanIDFromName(const std::string &name) const;	
	// Returns string name of a channel from its ID, or an empty string if not found
	const std::string   &getChanNameFromID(TChanID chan) const;
	// Count number of session in that channel (in O(n))
	uint				getSessionCount(TChanID chan);
	// Remove channel with the given ID. Return true if success.
	bool				removeChan(TChanID chan);
	// Add a client with the given ID. Return true if success.
	bool				addClient(const TDataSetRow &client);
	// Remove client from its ID. Return true if success
	bool				removeClient(const TDataSetRow &client);
	/** Create a new session in channel 'chan' for the client 'client' (both client & chan must have been added)
	  * \return true if success	
	  */
	bool				addSession(TChanID chan, const TDataSetRow &client, bool writeRight);

	/** Change sessions in channel 'chan' in order that bubble, written in that chanel are nor displayed
	  * \return true if success	
	  */
	bool				setHideBubble(TChanID chan, bool hideBubble);

	/** Change sessions in channel 'chan' so that chat is treated like universe channel
	  * \return true if success	
	  */
	bool				setUniversalChannel(TChanID chan, bool universalChannel);

	/** Stop session in channel 'chan' for the client 'client'.
	  * \return true if success
	  */
	bool				removeSession(TChanID chan, const TDataSetRow &client);
	// Set 'write right' flag for a session.
	bool				setWriteRight(TChanID chan, const TDataSetRow &client, bool writeRight);
	// Set size of historic for a given channel
	void				setHistoricSize(TChanID chan, uint32 size);
	// Get list of players in channel
	bool 				getPlayersInChan(TChanID chanID, std::vector<NLMISC::CEntityId> &players);

	// Resend all channel / sessions to the IOS
	void				iosConnection();	
	// Get all channel names (for read only)
	typedef std::map<TChanID, std::string> TChanIDToName;
	const TChanIDToName	&getChanIDToNameMap() const { return _ChanNames.getAToBMap(); }
	// Get pointer on all channels
	void				getChans(std::vector<CDynChatChan *> &channels) { _DynChat.getChans(channels); }
	const TChanID				getNextChanID() const { return _NextChanID; }

	/// Message from a service that need to create a new dynamic channel
	static void			cbServiceAddChan(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	/// Message from a service that need to hide bubbble of player/npc speaking in that channel
	static void			cbServiceSetHideBubble(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	/// Message from a service that need to set channel to be like universe channel
	static void			cbServiceSetUniversalChannel(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	/// Message from a service: remove a channel.
	static void			cbServiceRemoveChan(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	/// Message from a service : set the channel history
	static void			cbServiceSetChanHistory(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	/// Message from a service: add a char into a channel
	static void			cbServiceAddSession(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	static void			cbServiceAddSessionEntity(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	/// Message from a service: remove a char from a channel
	static void			cbServiceRemoveSession(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	static void			cbServiceRemoveSessionEntity(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	/// Message from a service: add a client
	static void			cbServiceAddClient(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
	// a service is down, remove any channel it added
	void				onServiceDown(NLNET::TServiceId serviceId);

private:
	TChanID									_NextChanID;
	CDynChat								_DynChat;
	// localized channels
	typedef NLMISC::CTwinMap<TChanID, std::string> TChanTwinMap;
	TChanTwinMap 	_ChanNames;
private:
	// ios msg
	void			 iosAddChan(TChanID chan, bool noBroadcast, bool forwardPlayerInputs, bool unify);
	void			 iosSetHideBubble(TChanID chan, bool hideBubble);
	void			 iosSetUniversalChannel(TChanID chan, bool universalChannel);
	void			 iosRemoveChan(TChanID chan);
	void			 iosAddSession(TChanID chan, const TDataSetRow &client, bool readOnly);
	void			 iosRemoveSession(TChanID chan, const TDataSetRow &client);
	void			 iosSetReadOnlyFlag(TChanID chan, const TDataSetRow &client, bool readOnly);
	void			 iosSetHistoricSize(TChanID chan, uint32 size);
	// Clear channel / sessions in IOS side.
	void			 iosResetDynChat();
	//
	TChanID			 addChan(const std::string &name, const ucstring &title, bool localized, TChanID chanID, bool noBroadcast, bool forwardPlayerInputs, bool unify);


	// message received 
};
extern CDynChatEGS DynChatEGS;





#endif
