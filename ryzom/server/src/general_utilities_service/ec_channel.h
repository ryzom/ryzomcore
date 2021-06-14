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

#ifndef EC_CHANNEL_H
#define EC_CHANNEL_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_chat.h"
#include "ec_types.h"


//-----------------------------------------------------------------------------
// EC namespace
//-----------------------------------------------------------------------------

namespace EC
{
	//-----------------------------------------------------------------------------
	// Public typedefs
	//-----------------------------------------------------------------------------

	// a user id - this is a character name or account number stored as a string
	typedef NLMISC::CSString TCharacterId;

	// standard ranks for users in a chat channel
	// - NO_RANK is used when a user is not present in any of the chat channel user groups
	enum TChannelRank { NO_RANK, MEMBER, OFFICER, ARCH };


	//-----------------------------------------------------------------------------
	// class CUserGroup
	//-----------------------------------------------------------------------------

	class CUserGroup
	{
	public:
		//-----------------------------------------------------------------------------
		// public interface

		// constructor requires a channel ptr as an argument as group belongs to a channel
		CUserGroup(IChannel* parent);

		// process a command to add, remove or display users
		NLMISC::CSString processCommand(const NLMISC::CSString& command);

		// test whether a given client id is listed in this user group
		bool contains(const TCharacterId& id) const;

	private:
		//-----------------------------------------------------------------------------
		// private data

		IChannel* _Parent;
		typedef std::vector<TCharacterId> TCharacterIds;
		TCharacterIds _CharacterIds;
	};


	//-----------------------------------------------------------------------------
	// class IChannel
	//-----------------------------------------------------------------------------

	class IChannel: private GUS::IChatCallback, public NLMISC::CRefCount
	{
	public:
		//-----------------------------------------------------------------------------
		// virtual interface exposed to derived classes

		// virtual dtor
		virtual ~IChannel();

		// Callback called whenever a user who belongs to one of the channel's user groups logs in
		// - also called if the user is already online when they are added to the user group
		// - this callback is NOT called if the user rank is NO_RANK
		virtual void cbAddUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId) =0;

		// Callback called whenever a user is removed from the channel's user groups
		virtual void cbRemoveUser(TChannelRank rank,const TCharacterId& idclientId,GUS::TClientId clientId) =0;

		// Callback called whenever a text is received from a user
		virtual void cbChatText(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId,const NLMISC::CSString& txt) =0;


	public:
		//-----------------------------------------------------------------------------
		// Accessors for channel basics

		const NLMISC::CSString& getChannelName() const;
		const NLMISC::CSString& getChannelTitle() const;
		void setChannelTitle(const NLMISC::CSString& title);


		//-----------------------------------------------------------------------------
		// Accessors for user lists

		// add a user to a selected user group
		// - removes the user from any other group that they may be in
		void addMember(const TCharacterId& id);
		void addOfficer(const TCharacterId& id);
		void addArch(const TCharacterId& id);

		// get the sets of members, officers or arch users
		CUserGroup& getMembers();
		CUserGroup& getOfficers();
		CUserGroup& getArchs();

		// remove a user from all groups
		void removeUser(const TCharacterId& id);

		// lookup a user in each of the user groups and determine their rank
		// - returns NO_RANK if user not found
		const TChannelRank getRank(const TCharacterId& id);

		// set a user rank by adding them to one of the user groups
		// - if the user is already in a user group they are removed before the operation begins
		// - if the rank is 'NO-RANK' then this operation is equivalent to 'removeUser()'
		void setRank(const TCharacterId& id,const TChannelRank& rank);

		// display the lists of members & officers
		void displayMembers();
		void displayOfficers();
		void displayArchs();
		void displayAllUsers();


		//-----------------------------------------------------------------------------
		// Methods for message sending / broadcasting

		void sendMessage(GUS::TClientId clientId,const NLMISC::CSString& speaker,const NLMISC::CSString& txt);
		void broadcastMessage(const NLMISC::CSString& speaker,const NLMISC::CSString& txt);


	protected:
		//-----------------------------------------------------------------------------
		// protected methods

		// ctor
		IChannel(const NLMISC::CSString& name);

		// accessor for the chat channel
		GUS::CChatChannel& getChannel();


	private:
		//-----------------------------------------------------------------------------
		// IChatCallback Specialisation

		virtual void receiveMessage(GUS::TClientId clientId,const ucstring& txt);
		virtual void clientReadyInChannel(GUS::CChatChannel* chatChannel, GUS::TClientId clientId);
		virtual bool isClientAllowedInChatChannel(GUS::TClientId clientId, GUS::CChatChannel *chatChannel);


	private:
		//-----------------------------------------------------------------------------
		// private data

		// smart pointer to the CChatChannel object
		GUS::TChatChannelPtr _Chat;

		// the vectors of users
		CUserGroup _Members;
		CUserGroup _Officers;
		CUserGroup _Archs;

		// the sets of names that have been added to or removes from _Members, _Officers or _Archs but 
		// have not yet been added to / removed from the _Chat
		typedef std::set<TCharacterId> TUntreatedUserSet;
		TUntreatedUserSet _ChatAdds;
		TUntreatedUserSet _ChatRemoves;


		//-----------------------------------------------------------------------------
		// private methods used by CUserGroup objects

		friend class CUserGroup;
		// either add to the container's adds list or remove from its removes list
		void _chatAdd(const TCharacterId& name);
		// either add to the container's removes list or remove from its adds list
		void _chatRemove(const TCharacterId& name);
		// Update the chat channel, removing players in the _ChatRemoves set and adding players in the _ChatAdds set
		void _chatUpdate();
	};
}


//-----------------------------------------------------------------------------
#endif
