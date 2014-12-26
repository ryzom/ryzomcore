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



#ifndef CHAT_MANAGER_H
#define CHAT_MANAGER_H


#include "chat_client.h"

// NeL
#include "nel/misc/types_nl.h"
#include "nel/net/message.h"
#include "nel/misc/displayer.h"

// game share
#include "game_share/ryzom_entity_id.h"
//#include "game_share/chat_static_database.h"
//#include "game_share/chat_dynamic_database.h"
#include "game_share/base_types.h"
#include "game_share/string_manager_sender.h"
#include "game_share/dyn_chat.h"


// std
#include <map>
#include <string>




/**
 * CChatManager
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CChatManager
{
public :

	CChatManager ();

	/// exception thrown when client is unknown
	struct EChatClient : public NLMISC::Exception
	{
		EChatClient( const NLMISC::CEntityId& id ) : Exception ("Don't have chat infos for the char "+id.toString()) {}
	};

	/// exception thrown when group is unknown
	struct EChatGroup : public NLMISC::Exception
	{
		EChatGroup( const TGroupId& gId ) : Exception ("Can't find the group "+gId.toString()) {}
	};

	/**
	 * Init the manager. Init the static and dynamic databases
	 */
	void init( /*const std::string& staticDBFileName, const std::string& dynDBFileName*/ );

	/** A service has gone down
	 */
	void onServiceDown(const std::string &serviceShortName);

	/**
	 * Reset ChatLog management
	 */
	void	resetChatLog();

	/**
	 * Check if the client is already registered in the chat manager.
	 */
	bool checkClient( const TDataSetRow& id );
	/**
	 *	Add a client (add client to universe grp in the same time)
	 * \param id is the client character id
	 */
	void addClient( const TDataSetRow& id );

	/**
	 *	Remove a client
	 * \param id is the client character id
	 */
	void removeClient( const TDataSetRow& id );

	/**
	 *	Get the client infos
	 * \param id is the client character id
	 */
	CChatClient& getClient( const TDataSetRow& id ); //throw (EChatClient);

	/**
	 * Return a reference on the static database
	 */
//	CChatStaticDatabase& getStaticDB() { return _StaticDB; }

	/**
	 * Return a reference on the dynamic database
	 */
//	CChatDynamicDatabase& getDynamicDB() { return _DynDB; }

	/**
	 * Add a chat group
	 * \param gId is the group id
	 * \param gType is the type of the group
	 */
	void addGroup( TGroupId gId, CChatGroup::TGroupType gType, const std::string &groupName );

	/**
	 * Remove a chat group
	 * \param gId is the group's id
	 */
	void removeGroup( TGroupId gId );

	/**
	 * Add a character to a chat group
	 * \param gId is the group's id
	 * \param charId is the character's id
	 */
	void addToGroup( TGroupId gId, const TDataSetRow &charId );

	/**
	 * Remove a character from a chat group
	 * \param gId is the group's id
	 * \param charId is the character's id
	 */
	void removeFromGroup( TGroupId gId, const TDataSetRow &charId );

	/**
	 * Get a group
	 * \param gId is the group's id
	 * \return the group
	 */
	CChatGroup& getGroup( const TGroupId& gId );// throw (EChatGroup);

	/**
	 * Transmit a chat message
	 * \param sender is the id of the talking char
	 * \param str is the chat content
	 */
	void chat( const TDataSetRow& sender, const ucstring& ucstr );

	/**
	 * Transmit a chat message to a group
	 * \param gId is the group's id
	 * \param str is the chat content
	 * \param sender is the id of the talking char
	 * \param excluded is a container of player that must not receive this chat
	 */
	void chatInGroup( TGroupId& grpId, const ucstring& ucstr, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded = std::vector<TDataSetRow>());

	/**
	 * Transmit a far chat message to a group
	 */
	void farChatInGroup(TGroupId &grpId, uint32 homeSessionId, const ucstring &text, const ucstring &senderName);

	/**
	 * Transmit a chat message to the receiver
	 * \param sender is the id of the speaking char
	 * \param receiver is the id of the listening char
	 * \param str is the chat content
	 */
	void tell( const TDataSetRow& sender, const std::string& receiver, const ucstring& ucstr );
	/**
	 * Transmit a chat message to the receiver
	 */
	void farTell(  const NLMISC::CEntityId &senderCharId, const ucstring &senderName, bool havePrivilege, const ucstring& receiver, const ucstring& ucstr   );
	/**
	 * Transmit a chat message to the receiver
	 * \param sender is the id of the speaking char
	 * \param receiver is the id of the listening char
	 * \param str is the identifier of a phrase to send to client
	 */
	void tell2( const TDataSetRow& sender, const TDataSetRow& receiver, const std::string& phraseId );

	/**
	 * Transmit a chat message
	 * \param sender is the id of the talking char
	 * \param phraseId is the phrase name in phrase file
	 */
	void chat2( const TDataSetRow& sender, const std::string &phraseId );



	/**
	 * Transmit a chat message
	 * \param sender is the id of the talking char
	 * \param phraseId is the phrase name in phrase file
	 * \param params is parameter of the phrase

	 */
	void chatParam( const TDataSetRow& sender, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam>& params);

	/**
	 * Transmit a chat message
	 * \param sender is the id of the talking char
	 * \param phraseId is the phrase string Id ( as uint32)
	 * \param sendSender is true -> client will now the sender id
	 * \param ignored : targeted entities that must be ignored.
	 */
	void chat2Ex( const TDataSetRow& sender, uint32 phraseId );

	/**
	 * Transmit a chat message to a group
	 * \param gId is the group's id
	 * \param phraseId is the phrase name in phrase file
	 * \param sender is the id of the talking char
	 * \param excluded is a container of player that must not receive this chat
	 */
	void chatInGroup2( TGroupId& grpId, const std::string &phraseId, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded = std::vector<TDataSetRow>() );
	// same as chatInGroup2 but use phrase with parameters
	void chatParamInGroup( TGroupId& grpId, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam> & params, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded = std::vector<TDataSetRow>() );

	/**
	 * Transmit a chat message to a group
	 * \param gId is the group's id
	 * \param phraseId is the phrase id (uint32)
	 * \param sender is the id of the talking char
	 * \param excluded is a container of player that must not receive this chat
	 */
	void chatInGroup2Ex( TGroupId& grpId, uint32 phraseId, const TDataSetRow& sender, const std::vector<TDataSetRow> & excluded = std::vector<TDataSetRow>() );

	/**
	 * send an emote text to the target audience
	 * \param sender is the id of the talking char
	 * \param phraseId is the phrase id (string)
	 * \param params is the phrase params
	 * \param excluded is a container of player that must not receive this chat
	 */
	void sendEmoteTextToAudience(  const TDataSetRow& sender,const std::string & phraseId, const TVectorParamCheck & params , const std::vector<TDataSetRow> & excluded );

	/**
	 * send an emote text to the target player
	 * \param target is the id of the target char
	 * \param phraseId is the phrase id (uint32)
	 */
	void sendEmoteTextToPlayer(  const TDataSetRow& sender, const TDataSetRow& target, uint32 phraseId )
	{
		sendChat2Ex( CChatGroup::say, target, phraseId, sender );
	}

	/**
	 * send an emote custom text to the target and target audience
	 * \param sender is the id of the talking char
	 * \param str is the custom phrase
	 */
	void sendEmoteCustomTextToAll( const TDataSetRow& sender, const ucstring & str );

	/**
	 * Send a message to the client to add a new string in the dynamic database
	 * \param receiver is the id of the client's character
	 * \param index is the index of the string
	 * \param frontendId is the id of the frontend managing the receiver
	 */
//	void addDynStr( const NLMISC::CEntityId& receiver, uint32 index, NLNET::TServiceId frontendId );

	/// Display the list of clients
	void displayChatClients(NLMISC::CLog &log);

	/// Display the list of chat group with the registered players (do not list universe)
	void displayChatGroups(NLMISC::CLog &log, bool displayUniverse, bool displayPlayerAudience);

	// display the content of oine chat group
	void displayChatGroup(NLMISC::CLog &log, TGroupId gid, CChatGroup &chatGroup);

	void displayChatAudience(NLMISC::CLog &log, const NLMISC::CEntityId &eid, bool updateAudience);

	/// add a user that ignores tells
	void addUserIgnoringTells( const NLMISC::CEntityId &eid )
	{
		_UsersIgnoringTells.insert( eid );
	}

	/// remove a user that ignores tells
	void removeUserIgnoringTells( const NLMISC::CEntityId &eid )
	{
		_UsersIgnoringTells.erase( eid );
	}

	/// add a muted user
	void addMutedUser( const NLMISC::CEntityId &eid )
	{
		_MutedUsers.insert( eid );
	}

	/// remove a muted user
	void removeMutedUser( const NLMISC::CEntityId &eid )
	{
		_MutedUsers.erase( eid );
	}

	// add a muted universe channel user
	void addUniverseMutedUser( const NLMISC::CEntityId &eid )
	{
		_MutedUniverseUsers.insert( eid );
	}

	// remove a muted universe channel user
	void removeUniverseMutedUser( const NLMISC::CEntityId &eid )
	{
		_MutedUniverseUsers.erase( eid );
	}

	/// get the dyn chat
	CDynChat &getDynChat() { return _DynChat; }

	// send historic of a dyn chat channel to the given player
	void	  sendHistoric(const TDataSetRow &receiver, TChanID chanID);

	/// Filter text send from client for removing any color code
	ucstring filterClientInputColorCode(ucstring &text);

	/// Filter text send from client for removing any forbiden or harming content
	ucstring filterClientInput(ucstring &text);

	/// Subscribe special ring users in the ring universe chat
	void subscribeCharacterInRingUniverse(const NLMISC::CEntityId &charEId);
	/// Unsubscribe special ring users in the ring universe chat
	void unsubscribeCharacterInRingUniverse(const NLMISC::CEntityId &charEId);


private :

	typedef std::map< TDataSetRow, CChatClient*>	TClientInfoCont;
	/// client infos
	TClientInfoCont	_Clients;

	/// chat groups
	std::map< TGroupId, CChatGroup > _Groups;

	/// chat group name index.
	std::map<NLMISC::TStringId, TGroupId>	_GroupNames;

	/// static database
//	CChatStaticDatabase _StaticDB;

	/// dynamic database
//	CChatDynamicDatabase _DynDB;

	/// Logger for chat
	NLMISC::CFileDisplayer	_Displayer;
	NLMISC::CLog			_Log;

	/// users ignoring tells
	std::set<NLMISC::CEntityId> _UsersIgnoringTells;

	/// muted users
	std::set<NLMISC::CEntityId> _MutedUsers;

	/// muted universe users
	std::set<NLMISC::CEntityId> _MutedUniverseUsers;

	/// Temporary list of users (to avoid large amount of reallocs...)
	std::list<NLMISC::CEntityId>	_DestUsers;

	CDynChat _DynChat;


protected:
	friend void	cbSysChat( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void cbNpcTell( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void cbGhostTell( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void cbNpcTellEx( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void cbDynChatServiceChat( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
	friend void cbDynChatServiceTell( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

public:
	/**
	 * Send a chat message
	 * \param senderChatMode is the chat mode of the sender
	 * \param receiver is the id of the receiver
	 * \param str is the message content
	 * \param sender is the id of the sender
	 * \param chanID If the chat group is CChatGroup::dyn_chan, gives target channel .
	 * \param senderName Can be used to replace the sender name with a specific string
	 */
	void sendChat( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const ucstring& ucstr, const TDataSetRow &sender = TDataSetRow(), TChanID chanID = NLMISC::CEntityId::Unknown, const ucstring &senderName = ucstring());


	/**
	 * Send a far chat message
	 */
	void sendFarChat( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const ucstring& ucstr, const ucstring &senderName, TChanID chanID = NLMISC::CEntityId::Unknown);

	/**
	 * Send a chat message
	 * \param senderChatMode is the chat mode of the sender
	 * \param receiver is the id of the receiver
	 * \param phraseId the string manager string number of the chat phrase
	 * \param sender is the id of the sender
	 */
	void sendChat2( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const std::string &phraseId, const TDataSetRow &sender = TDataSetRow() );
	// Same as sendChat2 but we can use phrase with parameters
	void sendChatParam( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, const std::string &phraseId, const std::vector<STRING_MANAGER::TParam>& params, const TDataSetRow &sender = TDataSetRow() );

	/**
	 * Send a chat message, using the string manager. Allow parametered string to be sent
	 * \param senderChatMode is the chat mode of the sender
	 * \param receiver is the id of the receiver
	 * \param phraseId the string manager string number of the chat phrase
	 * \param sender is the id of the sender
	 * \param customTxt is a custom text which can be added immediately after the chat message, on the same line
	 */
	void sendChat2Ex( CChatGroup::TGroupType senderChatMode, const TDataSetRow &receiver, uint32 phraseId, const TDataSetRow &sender = TDataSetRow(), ucstring customTxt = ucstring(""));

	/**
	 * Send a custom emote chat message
	 * \param sender is the id of the sender
	 * \param receiver is the id of the receiver
	 * \param ucstr is the message content
	 */
	void sendChatCustomEmote( const TDataSetRow &sender, const TDataSetRow &receiver, const ucstring& ucstr );
};


#endif // CHAT_MANAGER_H

/* End of chat_manager.h */
