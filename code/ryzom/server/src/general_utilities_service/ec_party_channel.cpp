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

#include "game_share/utils.h"
#include "game_share/txt_command.h"

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
	// Context object for use by party channel TXT_COMMAND commands
	//-----------------------------------------------------------------------------

	struct CPartyChannelContext
	{
		CPartyChannelContext(
			CPartyChannel*			channel,
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

		TPartyChannelPtr		Channel;
		TChannelRank			Rank;
		TCharacterId			Id;
		GUS::TClientId			ClientId;
	};

	//-----------------------------------------------------------------------------
	// Command set for the party channel
	//-----------------------------------------------------------------------------

	TXT_COMMAND_SET(ECPCCommandSet,CPartyChannelContext);


	//-----------------------------------------------------------------------------
	// methods CPartyChannel
	//-----------------------------------------------------------------------------

	void CPartyChannel::cbAddUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId)
	{
		switch (rank)
		{
			case MEMBER:	getChannel().sendMessage(clientId,"system","Welcome to this party channel - you are a normal member"); break;
			case OFFICER:	getChannel().sendMessage(clientId,"system","Welcome to this party channel - you are an officer"); break;
			case ARCH:		getChannel().sendMessage(clientId,"system","Welcome to this party channel - you are an arch user"); break;
			default: getChannel().sendMessage(clientId,"system","ERROR: Unable to identify your rank!!!");
		}
	}

	void CPartyChannel::cbRemoveUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId)
	{
		getChannel().broadcastMessage("system",id+" - kicked from this channel");
		return;
	}

	void CPartyChannel::cbChatText(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId,const NLMISC::CSString& txt)
	{
		if (txt.leftStrip().left(1)=="/")
		{
			// execute a command
			CPartyChannelContext context(this,rank,id,clientId);
			CTxtCommandResult result= ECPCCommandSet->execute(context,txt.leftStrip().leftCrop(1));
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

		// make sure the user has the right to chat in the channel at the moment
		if (rank<_MinChatRank)
		{
			getChannel().sendMessage(clientId,"system","Players of your rank are currently not able to chat in this channel");
			return;
		}

		// broadcast the message back to the chat
		getChannel().broadcastMessage(id,txt);
	}

	CPartyChannel::CPartyChannel(const CSString& chatChannelName,const NLMISC::CSString& title): IChannel(chatChannelName)
	{
		_MinChatRank= MEMBER;
		getChannel().openChannel(chatChannelName, 0, true, true, false);
		getChannel().setChannelTitle(title);
//		CIOSMsgSetPhrase(chatChannelName,title).send();
	}

	TChannelRank CPartyChannel::getMinChatRank() const
	{
		return _MinChatRank;
	}

	void CPartyChannel::setMinChatRank(TChannelRank rank)
	{
		_MinChatRank= rank;
	}


	//-----------------------------------------------------------------------------
	// Command for party channels: help
	//-----------------------------------------------------------------------------

	TXT_COMMAND(help,ECPCCommandSet,CPartyChannelContext)
	{
		context.Channel->sendMessage(context.ClientId,"help","/help     - display this help");
		context.Channel->sendMessage(context.ClientId,"help","/members  - list the normal channel members");
		context.Channel->sendMessage(context.ClientId,"help","/officers - list the channel officers");
		context.Channel->sendMessage(context.ClientId,"help","/archs    - list the channel arch users");
		context.Channel->sendMessage(context.ClientId,"help","/mode     - display the channel's mode");

		if (context.Rank<OFFICER) return true;

		context.Channel->sendMessage(context.ClientId,"help","/kick - kick a player from the channel");
		context.Channel->sendMessage(context.ClientId,"help","/members +bob +frank -bert - add or remove players from the channel");

		if (context.Rank<ARCH) return true;

		context.Channel->sendMessage(context.ClientId,"help","/officers +bob +frank -bert - add or remove officers for the channel");
		context.Channel->sendMessage(context.ClientId,"help","/mode all - allow all players to chat");
		context.Channel->sendMessage(context.ClientId,"help","/mode officers - only allow officers to chat");
		context.Channel->sendMessage(context.ClientId,"help","/mode arch - only allow arch users to chat");

		return true;
	}


	//-----------------------------------------------------------------------------
	// Command for party channels: kick
	//-----------------------------------------------------------------------------

	TXT_COMMAND(kick,ECPCCommandSet,CPartyChannelContext)
	{
		if (context.Rank<OFFICER)
		{
			context.Channel->sendMessage(context.ClientId,"system","You don't have the right to kick other players from this channel");
			return true;
		}
		for (uint32 i=0;i<args.size();++i)
		{
			TChannelRank targetRank= context.Channel->getRank(args[i]);
			if (targetRank>= context.Rank)
			{
				context.Channel->sendMessage(context.ClientId,"system",args[i]+" - player is too high rank for you to kick from this this channel");
				continue;
			}
			switch(targetRank)
			{
				case OFFICER:	context.Channel->getOfficers().processCommand("-"+args[i]); break;
				case MEMBER:	context.Channel->getMembers().processCommand("-"+args[i]); break;
				default: BOMB("ERROR: Don't know how to kick player: "+args[i],continue);
			}
		}
		return true;
	}


	//-----------------------------------------------------------------------------
	// Command for party channels: members
	//-----------------------------------------------------------------------------

	TXT_COMMAND(members,ECPCCommandSet,CPartyChannelContext)
	{
		CSString cmd= (context.Rank<OFFICER)? "": rawArgs.strip();
		if (cmd.empty())
		{
			CSString archs= context.Channel->getArchs().processCommand("");
			context.Channel->sendMessage(context.ClientId,"archs",archs);
			CSString officers= context.Channel->getOfficers().processCommand("");
			context.Channel->sendMessage(context.ClientId,"officers",officers);
		}
		CSString members= context.Channel->getMembers().processCommand(cmd);
		context.Channel->sendMessage(context.ClientId,"members",members);
		return true;
	}


	//-----------------------------------------------------------------------------
	// Command for party channels: officers
	//-----------------------------------------------------------------------------

	TXT_COMMAND(officers,ECPCCommandSet,CPartyChannelContext)
	{
		CSString cmd= (context.Rank<ARCH)? "": rawArgs.strip();
		if (cmd.empty())
		{
			CSString archs= context.Channel->getArchs().processCommand("");
			context.Channel->sendMessage(context.ClientId,"archs",archs);
		}
		CSString officers= context.Channel->getOfficers().processCommand(cmd);
		context.Channel->sendMessage(context.ClientId,"officers",officers);
		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for party channels: archs
	//-----------------------------------------------------------------------------

	TXT_COMMAND(archs,ECPCCommandSet,CPartyChannelContext)
	{
		CSString archs= context.Channel->getArchs().processCommand("");
		context.Channel->sendMessage(context.ClientId,"archs",archs);
		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for party channels: mode
	//-----------------------------------------------------------------------------

	TXT_COMMAND(mode,ECPCCommandSet,CPartyChannelContext)
	{
		if (rawArgs.strip().empty() || context.Rank!=ARCH)
		{
			switch (context.Channel->getMinChatRank())
			{
				case ARCH:		context.Channel->sendMessage(context.ClientId,"system","Chat mode: ARCH");
				case OFFICER:	context.Channel->sendMessage(context.ClientId,"system","Chat mode: OFFICERS");
				case MEMBER:	context.Channel->sendMessage(context.ClientId,"system","Chat mode: ALL");
				default:		context.Channel->sendMessage(context.ClientId,"system","Chat mode: UNKNOWN");
			}
		}
		else if (rawArgs.strip()=="all")
		{
			if (context.Channel->getMinChatRank()== MEMBER)	return true;
			context.Channel->setMinChatRank(MEMBER);
			context.Channel->broadcastMessage("system","Chat mode changed to 'ALL' by "+context.Id);
		}
		else if (rawArgs.strip()=="officers")
		{
			if (context.Channel->getMinChatRank()== OFFICER) return true;
			context.Channel->setMinChatRank(OFFICER);
			context.Channel->broadcastMessage("system","Chat mode changed to 'OFFICERS' by "+context.Id);
		}
		else if (rawArgs.strip()=="arch")
		{
			if (context.Channel->getMinChatRank()== ARCH) return true;
			context.Channel->setMinChatRank(ARCH);
			context.Channel->broadcastMessage("system","Chat mode changed to 'ARCH' by "+context.Id);
		}
		else
		{
			context.Channel->sendMessage(context.ClientId,"system","Bad command syntax: expected 'mode', 'mode all', 'mode officers' or 'mode arch' but found: "+fullCmdLine);
		}
		return true;
	}
}
