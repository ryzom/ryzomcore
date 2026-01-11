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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "game_share/txt_command.h"

#include "ec_faction_channel.h"
#include "ec_party_channel.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace GUS;


//-----------------------------------------------------------------------------
// EC namespace
//-----------------------------------------------------------------------------

namespace EC
{
	//-----------------------------------------------------------------------------
	// Context object for use by faction channel TXT_COMMAND commands
	//-----------------------------------------------------------------------------

	struct CFactionChannelContext
	{
		CFactionChannelContext(
			CFactionChannel*		channel,
			TChannelRank			rank,
			const TCharacterId&		id,
			GUS::TClientId			clientId
			)
		{
			Channel		= channel;
			Rank		= rank;
			Id			= id;
			ClientId	= clientId;
		}

		TFactionChannelPtr	Channel;
		TChannelRank		Rank;
		TCharacterId		Id;
		GUS::TClientId		ClientId;
	};


	//-----------------------------------------------------------------------------
	// Command set for the faction channel
	//-----------------------------------------------------------------------------

	TXT_COMMAND_SET(ECFCCommandSet,CFactionChannelContext);


	//-----------------------------------------------------------------------------
	// methods CFactionChannel
	//-----------------------------------------------------------------------------

	void CFactionChannel::cbAddUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId)
	{
		switch (rank)
		{
			case ARCH:		getChannel().sendMessage(clientId,"system","Welcome to this faction channel - you are an arch user"); break;
			case OFFICER:	getChannel().sendMessage(clientId,"system","Welcome to this faction channel - you are an officer"); break;
			case MEMBER:	
			default:
				getChannel().sendMessage(clientId,"system","This is an event faction channel - only party leaders are allowed to post here"); break;
		}
	}

	void CFactionChannel::cbRemoveUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId)
	{
		getChannel().broadcastMessage("system",id+" - kicked from this channel");
		return;
	}

	void CFactionChannel::cbChatText(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId,const NLMISC::CSString& txt)
	{
		if (txt.leftStrip().left(1)=="/")
		{
			// execute a command
			CFactionChannelContext context(this,rank,id,clientId);
			CTxtCommandResult result= ECFCCommandSet->execute(context,txt.leftStrip().leftCrop(1));
			switch (result.getType())
			{
				case CTxtCommandResult::SUCCESS:
					getChannel().sendMessage(clientId,"*","Command executed: "+txt);
					break;

				case CTxtCommandResult::SYNTAX_ERROR:
					getChannel().sendMessage(clientId,"*","Error in command parameter syntax: "+txt);
					break;

				case CTxtCommandResult::BAD_PERMISSION:
					getChannel().sendMessage(clientId,"*","You don't have permission to execute this command: "+txt);
					break;

				case CTxtCommandResult::UNKNOWN_COMMAND:
					getChannel().sendMessage(clientId,"*","Unknown command - try '/help' to see the valid command list: "+txt.firstWordConst());
					break;

				case CTxtCommandResult::EXECUTION_ERROR:
				default:
					getChannel().sendMessage(clientId,"*","Error trying to execute command: "+txt);
					break;
			}
			if (result.getReason()!="" && !result.getReason().empty())
				getChannel().sendMessage(clientId,"*",result.getReason());
			return;
		}

		// make sure the use has the right to chat in the channel at the moment
		if (rank<_MinChatRank)
		{
			getChannel().sendMessage(clientId,"system","Players of your rank are currently not able to chat in this channel");
			return;
		}

		// broadcast the message back to the chat
		getChannel().broadcastMessage(id,txt);
	}

	CFactionChannel::CFactionChannel(CEventChatModule* theModule,const CSString& name): IChannel(name)
	{
		CSString chatChannelName= "ce_faction_"+name;
		_TheModule= theModule;
		_MinChatRank= OFFICER;
		getChannel().openChannel(name, 1000, true, true, true);
		getChannel().setChannelTitle(name);
//		CIOSMsgSetPhrase(chatChannelName,name).send();
	}

	bool CFactionChannel::setupParty(const NLMISC::CSString& ownerCharacterName,const NLMISC::CSString& title)
	{
		nlassert(_Parties.size()==_PartyIdx.size());

		// setup the channel name for this channel
		CSString chatChannelName= "ec_party_"+ownerCharacterName;

		// get hold of the channel pointer in the _Parties map
		TPartyChannelPtr& channel= _Parties[ownerCharacterName];
		if (channel!=NULL)
		{
			// in this case we are just renaming the channel and getting out....
			// send a message to the IOS to set the name of the chat window
			channel->setChannelTitle(title);
//			CIOSMsgSetPhrase(chatChannelName,title).send();
			return true;
		}

		// create a new chat channel for the party and add the the owner as an arch
		_PartyIdx.push_back(ownerCharacterName);
		channel= new CPartyChannel(chatChannelName,title);
		channel->addArch(ownerCharacterName);

		// add all archs from the faction channel as archs for the party chat too
		CSString s= getArchs().processCommand("").strip();
		while(!s.empty())
		{
			channel->addArch(s.firstWord(true));
		}
		return true;
	}

	void CFactionChannel::removeParty(const NLMISC::CSString& ownerCharacterName)
	{
		nlassert(_Parties.size()==_PartyIdx.size());

		// locate the map entry to erase and return if not found
		TParties::iterator it= _Parties.find(ownerCharacterName);
		if (it==_Parties.end())
			return;

		// erase the map entry
		_Parties.erase(it);

		// locate the index entry that corresponds to the erased map entry
		uint32 i=0;
		do
		{
			nlassert(i<_PartyIdx.size());
			if (_PartyIdx[i]!=ownerCharacterName) break;
		}
		while (true);

		// shuffle down the party entries to cover the deleted entry
		for (uint32 j=i+1;j<_PartyIdx.size();++j)
		{
			_PartyIdx[j-1]= _PartyIdx[j];
		}

		// shorten the index to remove the deleted entry
		_PartyIdx.pop_back();
	}

	uint32 CFactionChannel::getPartyCount() const
	{
		nlassert(_Parties.size()==_PartyIdx.size());
		return _PartyIdx.size();
	}

	CPartyChannel* CFactionChannel::getParty(uint32 idx)
	{
		nlassert(_Parties.size()==_PartyIdx.size());
		if (idx>=_PartyIdx.size())
			return NULL;
		TParties::iterator it= _Parties.find(_PartyIdx[idx]);
		nlassert(it!=_Parties.end());
		return it->second;
	}

	//-----------------------------------------------------------------------------
	// Command for faction channels: help
	//-----------------------------------------------------------------------------

	TXT_COMMAND(help,ECFCCommandSet,CFactionChannelContext)
	{
		context.Channel->sendMessage(context.ClientId,"help","/help     - display this help");
		context.Channel->sendMessage(context.ClientId,"help","/officers - list the channel officers");
		context.Channel->sendMessage(context.ClientId,"help","/archs    - list the channel arch users");

		if (context.Rank<OFFICER) return true;

		context.Channel->sendMessage(context.ClientId,"help","/party - kick a player from the channel");

		if (context.Rank<ARCH) return true;

		context.Channel->sendMessage(context.ClientId,"help","/officers +bob +frank -bert - add or remove officers for the channel");

		return true;
	}


	//-----------------------------------------------------------------------------
	// Command for faction channels: party
	//-----------------------------------------------------------------------------

	TXT_COMMAND(party,ECFCCommandSet,CFactionChannelContext)
	{
		if (context.Rank<OFFICER) return false;

		// if no explicit title is specified for the channel then use the character's name
		CSString title= rawArgs.strip();
		if (title.empty())
			title= context.Id;

		// setup the channel name for this channel & open the channel if it's not already open
		return context.Channel->setupParty(context.Id,title);
	}

	//-----------------------------------------------------------------------------
	// Command for faction channels: officers
	//-----------------------------------------------------------------------------

	TXT_COMMAND(officers,ECFCCommandSet,CFactionChannelContext)
	{
		// only archs have the right to modify the officers list but all users are allowed to view it
		CSString cmdTail= rawArgs;
		if (context.Rank<ARCH)
			cmdTail.clear();

		// if we're viewing the officers list then display the arch list too
		if (cmdTail.empty())
		{
			CSString archs= context.Channel->getArchs().processCommand("");
			context.Channel->sendMessage(context.ClientId,"archs",archs);
		}

		// have the _Officers object process our command (either query or modify) and display the results
		CSString officers= context.Channel->getOfficers().processCommand(cmdTail);
		context.Channel->sendMessage(context.ClientId,"officers",officers);

		// run through any officers that have just been removed...
		for(officers=officers.splitFrom('-');!officers.empty();officers=officers.splitFrom('-'))
		{
			// extract the name of the officer who's been removed...
			CSString name= officers.firstWord();
			// if the officer had started a party chat then get rid of it
			context.Channel->removeParty(name);
			// add the chap to the 'members' list to avoid unwanted closing of chat channel on his screen
			context.Channel->getMembers().processCommand("+"+name);
		}
		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for faction channels: archs
	//-----------------------------------------------------------------------------

	TXT_COMMAND(archs,ECFCCommandSet,CFactionChannelContext)
	{
		// display the arch list
		CSString archs= context.Channel->getArchs().processCommand("");
		context.Channel->sendMessage(context.ClientId,"archs",archs);
		return true;
	}
}
