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



#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H



// game share
#include "game_share/chat_group.h"
#include "game_share/base_types.h"
#include "game_share/dyn_chat.h"

// std
#include <vector>
#include <set>


typedef uint8 TFilter;

 
/**
 * CChatClient
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CChatClient 
{
public :
	
	/**
	 *	Constructor
	 */
	CChatClient( const TDataSetRow& id ); 
	/************************************************************************/
	/* Destructor                                                                */
	/************************************************************************/
	~CChatClient();

	/**
	 * Get the id of this client
	 */
	const NLMISC::CEntityId &getId() const { return _Id; }
	/**
	 * Get the dataset row of this client
	 */
	const TDataSetRow &getDataSetIndex() const { return _DataSetIndex; }

	/** 
	 * Mute or unmute this client, nobody will received chat from him
	 * the player can be muted for a fixed period of time
	 * If the player is unmuted, the delay is not taken into account
	 * \param delay is the mute delay
	 */
	/// DEPRECATED
	void mute( sint32 delay );

	/**
	 *	Get the mute state of this client
	 * 
	 * \return true if this client is muted
	 */
	bool isMuted();

	/**
	 * Add or remove a character from the ignore list of this client
	 *
	 * \param id is the id of the character to be add/remove in the ignore list
	 */
	void setIgnoreStatus( const NLMISC::CEntityId &id, bool ignored);

	// Set the ignore list
	void setIgnoreList(const std::vector<NLMISC::CEntityId> &ignoreList);

	/**
	 * Return true if the character is in the ignore list of this client
	 *
	 * \return true if the character is in the ignore list, false else
	 */
	bool isInIgnoreList( const NLMISC::CEntityId &id );
	bool isInIgnoreList( const TDataSetRow &id );

	/**
	 * Add or remove a string filter
	 * \param filterId the filter
	 */
	void filter( TFilter filterId );
	
	/**
	 * Return true if the filter is on
	 * \param filterId the filter
	 */
	bool isFilterOn( TFilter filterId ) const;

	/**
	 * Set the chat mode	 
	 * \param mode the chat mode
	 * \param dynChatChan If mode is dyn chat, then gives the channel
	 */
	void setChatMode( CChatGroup::TGroupType mode, TChanID dynChatChan = NLMISC::CEntityId::Unknown);

	/**
	 * Get the chat mode
	 * \return the chat mode
	 */
	CChatGroup::TGroupType getChatMode() const { return _ChatMode; }

	/**
     * Get dyn chat channel. Relevant only if chat mode is CChatGroup::dyn_chat
	 */
	TChanID	getDynChatChan() const { return _DynChatChan; }

	/**
	 * Set the chat group
	 * \param gId the group's id
	 */
//	void setChatGroup( TGroupId gId) { _ChatGroup = gId; }

	void setTeamChatGroup( TGroupId gId)	
	{ 
		_TeamChat = gId;
	}
	void setGuildChatGroup( TGroupId gId)	
	{ 
		_GuildChat = gId;
	}
	void setRegionChatGroup( TGroupId gId)	
	{ 
		_RegionChat = gId;
	}

	TGroupId getTeamChatGroup()	
	{ 
		return _TeamChat;
	}
	TGroupId getGuildChatGroup()	
	{ 
		return _GuildChat;
	}
	TGroupId getRegionChatGroup()	
	{ 
		return _RegionChat;
	}
	
	/**
	 * Get the chat group
	 * \return the group's id
	 */
//	TGroupId getChatGroup() const { return _ChatGroup; }

	/**
	 * Update the audience of this client
	 */
	void updateAudience();

	/**
	 * Get the dynamic chat group
	 * \return the dynamic chat group
	 */
	CChatGroup& getAudience();

	/**
	 * Get the say dynamic chat group
	 * \return the say dynamic chat group
	 */
	CChatGroup& getSayAudience( bool updateAudience = false );

	TGroupId getSayAudienceId()	{ return _SayAudienceId; }

	/**
	 * Get the shout dynamic chat group
	 * \return the shout dynamic chat group
	 */
	CChatGroup& getShoutAudience( bool updateAudience = false );

	TGroupId getShoutAudienceId()	{ return _ShoutAudienceId; }

	/**
	 * The client now know the string
	 * \param index is the index of the string
	 * \return true if the string was already known by the client
	 */
	bool knowString( uint32 index );

	// store chat group subscribe
	void subscribeInChatGroup(TGroupId groupId);
	// store chat group unsubscribe
	void unsubscribeInChatGroup(TGroupId groupId);

	// unsubscribe to all chat group
	void unsubscribeAllChatGroup();

private :
	
	/// CLient datasetrow
	TDataSetRow			_DataSetIndex;
	/// client character id
	NLMISC::CEntityId	_Id;

	/// if true : nobody will see chat message incoming from this client
	bool				_Muted;

	/// mute start time
	NLMISC::TTime		_MuteStartTime;

	/// mute delay (in min)
	sint32				_MuteDelay;

	typedef std::set<NLMISC::CEntityId>	TIgnoreListCont;
	/// this client won't see chat incoming from these characters
//	std::set<NLMISC::CEntityId> _IgnoreList;
	TIgnoreListCont		_IgnoreList;

	/// string filters
	std::set<TFilter>	_Filters;

	/// current chat mode
	CChatGroup::TGroupType _ChatMode;
	TChanID				   _DynChatChan; // if _ChatMode == dyn_chta, gives the unique ID of the channel

	/// Team chat
	TGroupId			_TeamChat;
	/// Guild chat
	TGroupId			_GuildChat;
	/// region chat
	TGroupId			_RegionChat;

	/// The subscribed chat group (including team and guild chat)
	typedef std::set<TGroupId>	TSubscribedGroupCont;
	TSubscribedGroupCont		_SubscribedGroups;


	/// current chat group
//	TGroupId _ChatGroup;

	/// say audience
//	CChatGroup _SayAudience;
	/// Say audience group id
	NLMISC::CEntityId	_SayAudienceId;

	/// shout audience
//	CChatGroup _ShoutAudience;
	/// Shout audience group id
	NLMISC::CEntityId	_ShoutAudienceId;

	/// Time of the last say audience update
	NLMISC::TTime		_SayLastAudienceUpdateTime;

	/// Time of the last shout audience update
	NLMISC::TTime		_ShoutLastAudienceUpdateTime;

	/// time period between 2 audience update
	NLMISC::TTime		_AudienceUpdatePeriod;

	/// keep infos about which string has been received by the client
	std::vector<bool>	_KnownStrings;

	/**
	 * Update the audience of this client
	 */
	void updateAudience( NLMISC::CEntityId &audienceId, sint maxDist, NLMISC::TTime& lastAudienceUpdateTime );
};


#endif // CHAT_CLIENT_H

/* End of chat_client.h */

