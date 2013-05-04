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

#ifndef GUS_CHAT_H
#define GUS_CHAT_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/displayer.h"
#include "gus_client_manager.h"


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// forward class declarations
	//-----------------------------------------------------------------------------

	class IChatCallback;
	class CChatChannel;


	//-----------------------------------------------------------------------------
	// class IChatCallback
	//-----------------------------------------------------------------------------

	class IChatCallback
	{
	public:
		/// a client is ready to receive message in this chat channel
		virtual void clientReadyInChannel(CChatChannel *chatChannel, GUS::TClientId clientId) {}

		// A player has typed some text in the chat in forward mode
		//	Overloaded only when the channel is open with 'forward player input' 
		//	flag.
		virtual void receiveMessage(GUS::TClientId clientId,const ucstring& txt) {}

		// The chat channel ask if this player should be inserted in the channel. 
		// Only overloaded if the channel is open in non automatiuc player insertion.
		//
		virtual bool isClientAllowedInChatChannel(GUS::TClientId clientId, CChatChannel *chatChannel) {return true;}
	};


	//-----------------------------------------------------------------------------
	// class CChatChannel
	//-----------------------------------------------------------------------------

	class CChatChannel: public NLMISC::CRefCount
	{
	public:
		//-----------------------------------------------------------------------------
		// Public interface
		//-----------------------------------------------------------------------------
		virtual ~CChatChannel() {}

		// open the chat channel on all clients and set its name
		// If historySize is set other than 0, the the chat channel is
		// configured with the given lien of history.
		//
		virtual void openChannel(const NLMISC::CSString& channelTitle, uint32 historySize, bool noBroadcast, bool forwardInput, bool autoInsertPlayer)=0;

		// close the chat channel previously opened on all clients
		virtual void closeChannel()=0;

		// add a client to the chat channel
		virtual void addClient(GUS::TClientId clientId)=0;

		// remove a client from the chat channel
		virtual void removeClient(GUS::TClientId clientId)=0;

		// send a text to the chat channel for all clients
		virtual void broadcastMessage(const ucstring& speakerName, const ucstring& txt)=0;

		// send a text to the chat channel for all clients (UTF-8 encoded)
		virtual void broadcastMessage(const std::string& speakerNameUtf8, const std::string& txtUtf8)=0;

		// send a text to the chat channel for the given client
		virtual void sendMessage(GUS::TClientId clientId, const ucstring& speakerName, const ucstring& txt)=0;

		// send a text to the chat channel for the given client (UTF-8 encoded)
		virtual void sendMessage(GUS::TClientId clientId, const std::string& speakerNameUtf8, const std::string& txtUtf8)=0;

		// setup a callback to be called whenever a player sends a text to this chat channel
		virtual void setChatCallback(IChatCallback *callback)=0;

		// get the channel name used with the IOS / EGS
		virtual const NLMISC::CSString& getChannelName() const=0;

		// get the channel title that was set in openChannel()
		virtual const NLMISC::CSString& getChannelTitle() const=0;

		// set the channel title and inform the IOS of the title
		virtual void setChannelTitle(const NLMISC::CSString& title) =0;
	};
	typedef NLMISC::CSmartPtr<CChatChannel> TChatChannelPtr;


	//-----------------------------------------------------------------------------
	// class CChatManager
	//-----------------------------------------------------------------------------

	class CChatManager
	{
	public:
		//-----------------------------------------------------------------------------
		// Public interface
		//-----------------------------------------------------------------------------

		// get the singleton instance
		static CChatManager* getInstance();

		// Create a new named chat channel. Note, the channel is closed by default.
		// - can return NULL if a channel with that name already exist.
		virtual TChatChannelPtr	createChatChannel(const NLMISC::CSString &channelName) =0;

		// remove the given chat channel, closing it on all client screens
		virtual void removeChannel(CChatChannel *channel) =0;

		// Return a previously created chat channel or a NULL pointer.
		virtual TChatChannelPtr getChatChannel(const NLMISC::CSString& channelName) =0;

		// Set the title for a chat channel (and inform the IOS) - this operation returns
		// false if a chat channel already exists with the given title
		// the title is 'liberated' when the chat channel object is destroyed or its title changed
		// note that setChatChannelTitle(NULL,title) simply tests whether a title is allocated
		// to an existing chat channel - it is non-destructive and does not block future use of the title
		virtual bool setChatChannelTitle(CChatChannel *channel,const NLMISC::CSString& channelTitle) =0;
	};


	//-----------------------------------------------------------------------------
	// class CChatBroadcastDisplayer
	//-----------------------------------------------------------------------------
	// NLMISC::IDisplayer class for broadcasting to a gus chat channel

	class CChatBroadcastDisplayer: public NLMISC::IDisplayer
	{
	public:
		CChatBroadcastDisplayer(CChatChannel* channel);
		void doDisplay(const NLMISC::CLog::TDisplayInfo& args,const char *message);
	private:
		TChatChannelPtr _Channel;
	};


	//-----------------------------------------------------------------------------
	// class CChatDisplayer
	//-----------------------------------------------------------------------------
	// NLMISC::IDisplayer class for sending messages to a given user in a given gus chat channel

	class CChatDisplayer: public NLMISC::IDisplayer
	{
	public:
		CChatDisplayer(CChatChannel* channel,TClientId clientId);
		void doDisplay(const NLMISC::CLog::TDisplayInfo& args,const char *message);
	private:
		TChatChannelPtr _Channel;
		TClientId _ClientId;
	};

}


//-----------------------------------------------------------------------------
#endif
