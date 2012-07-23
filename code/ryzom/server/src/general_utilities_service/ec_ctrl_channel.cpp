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

#include "ec_event_chat_module.h"
#include "ec_faction_channel.h"
#include "ec_ctrl_channel.h"
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
	// Handy utility methods
	//-----------------------------------------------------------------------------

	static void sendLongTextToClient(TClientId clientId,IChannel* channel,const CSString& speaker,CSString txt)
	{
		BOMB_IF(channel==NULL,"Trying to send a multi line chat to a NULL channel",return);

		CSString firstWord= txt.firstWord(true).strip();
		while (!firstWord.empty())
		{
			CSString line;
			do
			{
				if (!line.empty()) line+=' ';
				line+= firstWord;
				firstWord= txt.firstWord(true).strip();
			}
			while (!firstWord.empty() && firstWord.size()+line.size()>=40);
			channel->sendMessage(clientId,speaker,line);
		}
	}

	//-----------------------------------------------------------------------------
	// Context object for use by ctrl channel TXT_COMMAND commands
	//-----------------------------------------------------------------------------

	struct CCtrlChannelContext
	{
		CCtrlChannelContext(
			CEventChatModule*		module,
			CCtrlChannel*			channel,
			TChannelRank			rank,
			const TCharacterId&		id,
			GUS::TClientId			clientId
			)
		{
			Module		= module;
			Channel		= channel;
			Rank		= rank;
			Id			= id;
			ClientId	= clientId;
		}

		TEventChatModulePtr		Module;
		TCtrlChannelPtr			Channel;
		TChannelRank			Rank;
		TCharacterId			Id;
		GUS::TClientId			ClientId;
	};

	//-----------------------------------------------------------------------------
	// Command set for the ctrl channel
	//-----------------------------------------------------------------------------

	TXT_COMMAND_SET(ECCCCommandSet,CCtrlChannelContext);


	//-----------------------------------------------------------------------------
	// methods CCtrlChannel
	//-----------------------------------------------------------------------------

	void CCtrlChannel::cbAddUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId)
	{
		switch (rank)
		{
			case MEMBER:	getChannel().sendMessage(clientId,"system","Welcome to the event control channel - you are a normal member"); break;
			case OFFICER:	getChannel().sendMessage(clientId,"system","Welcome to the event control channel - you are an officer"); break;
			case ARCH:		getChannel().sendMessage(clientId,"system","Welcome to the event control channel - you are an arch user"); break;
			default: getChannel().sendMessage(clientId,"system","ERROR: Unable to identify your rank!!!");
		}
	}

	void CCtrlChannel::cbRemoveUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId)
	{
		getChannel().broadcastMessage("*",id+" - kicked from this channel");
		return;
	}

	void CCtrlChannel::cbChatText(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId,const NLMISC::CSString& txt)
	{
		if (txt.leftStrip().left(1)=="/")
		{
			// display a little info message to the logs...
			nlinfo("Executing Event Ctrl Command: %s(%d): %s",id.c_str(),clientId.toString().c_str(),txt.c_str());

			// build a NEL displayer for sending messages to the client who's running this command
			CChatDisplayer displayer(&getChannel(),clientId);

			// setup the new NLMISC info log channel
			NLMISC::CLog* oldInfoLog= NLMISC::InfoLog;
			NLMISC::CLog infoLog= *NLMISC::InfoLog;
			infoLog.addDisplayer(&displayer);
			NLMISC::INelContext::getInstance().setInfoLog(&infoLog);

			// setup the new NLMISC warning log channel
			NLMISC::CLog* oldWarningLog= NLMISC::WarningLog;
			NLMISC::CLog warningLog= *NLMISC::WarningLog;
			warningLog.addDisplayer(&displayer);
			NLMISC::INelContext::getInstance().setWarningLog(&warningLog);

			// execute a command
			try
			{
				CCtrlChannelContext context(_TheModule,this,rank,id,clientId);
				CTxtCommandResult result= ECCCCommandSet->execute(context,txt.leftStrip().leftCrop(1));
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
			}
			catch(...)
			{
				nlwarning("Exception caught while trying to execute command: %s",txt.leftStrip().leftCrop(1).c_str());
			}

			// housekeeping	- restore NLMISC log channels
			infoLog.removeDisplayer(&displayer);
			NLMISC::INelContext::getInstance().setInfoLog(oldInfoLog);
			warningLog.removeDisplayer(&displayer);
			NLMISC::INelContext::getInstance().setWarningLog(oldWarningLog);
			return;
		}

		// make sure the use has the right to chat in the channel at the moment
		if (rank<_MinChatRank)
		{
			getChannel().sendMessage(clientId,"*","Players of your rank are currently not able to chat in this channel");
			return;
		}

		// display a little info message to the logs...
		nlinfo("Broadcasting to Event Ctrl: %s(%d): %s",id.c_str(),clientId.toString().c_str(),txt.c_str());

		// broadcast the message back to the chat
		getChannel().broadcastMessage(id,txt);
	}

	CCtrlChannel::CCtrlChannel(CEventChatModule* theModule): IChannel("EventCtrl")
	{
		CSString chatChannelName= "ce_event_control";
		_TheModule= theModule;
		_MinChatRank= MEMBER;
		getChannel().openChannel(chatChannelName, 1000, true, true, false);
		getChannel().setChannelTitle("EC");
	}


	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: help
	//-----------------------------------------------------------------------------

	TXT_COMMAND(help,ECCCCommandSet,CCtrlChannelContext)
	{
		InfoLog->displayNL("help","/help     - display this help");

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (args.size()!=0)			return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		// add 'officer' commands here
		InfoLog->displayNL("/info - show control channel memeber list & faction list with archs and associated party chats");
		InfoLog->displayNL("/info <channel> - show arch user, officer and member list for a given chat channel");
		InfoLog->displayNL("/say <channel> <pseudo> <txt> - say something in a party or faction channel using a given pseudo");

		if (context.Rank<ARCH) return true;

		// add 'arch' commands here

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: info
	//-----------------------------------------------------------------------------

	TXT_COMMAND(info,ECCCCommandSet,CCtrlChannelContext)
	{
		// info							- show control channel memeber list & faction list with archs and associated party chats
		// info <channel>				- show arch user, officer and member list for a given chat channel

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (args.size()!=1)			return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		if (args.empty())
		{
			void displayAllUsers();
			for (uint32 i=0;i<context.Module->getNumFactionChannels();++i)
			{
				CFactionChannel* faction= context.Module->getFactionChannel(i);
				InfoLog->displayNL("- Faction: %s",faction->getChannelTitle().c_str());
				CSString officers=faction->getOfficers().processCommand("").strip();
				for(CSString officerName= officers.firstWord(true);!officers.empty();officerName= officers.firstWord(true))
				{
					CSString partyName= "ec_party_"+officerName;
					IChannel* partyChannel= context.Module->getChannelByName(partyName);
					InfoLog->displayNL("  - Officer: %s: %s",officerName.c_str(),(partyChannel==NULL)?"<no chat channel>":partyChannel->getChannelTitle().c_str());
				}
			}
		}
		else
		{
			IChannel* channel= context.Module->getChannelByName(args[0]);
			DROP_IF(channel==NULL,"Channel not found: "+args[0],return true);

			InfoLog->displayNL("Member list for channel: %s",args[0].c_str());
			channel->displayAllUsers();
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: say
	//-----------------------------------------------------------------------------

	TXT_COMMAND(say,ECCCCommandSet,CCtrlChannelContext)
	{
		// say <channel> <pseudo> <txt>	- say something in a party or faction channel using a given pseudo (must be >=2 words)

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (args.size()<4)			return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		// lookup the channel
		CSString s= rawArgs;
		IChannel* channel= context.Module->getChannelByName(s.firstWord(true));
		if (channel==NULL) return false;

		// send the message
		CSString pseudo= s.firstWord(true);
		channel->broadcastMessage(pseudo,s.strip());

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: txtSay
	//-----------------------------------------------------------------------------

	TXT_COMMAND(txtSay,ECCCCommandSet,CCtrlChannelContext)
	{
		// txtSay <channel> <pseudo> <txt_id>	- add a pre-prepared event text to the event chat channel

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (args.size()!=3)			return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		// lookup the string in the localised string container
		GUS::CText& prepTxt= context.Module->getPreparedText();
		ucstring txt= prepTxt.get(args[0]);
		if (txt.empty()) return CTxtCommandResult(CTxtCommandResult::EXECUTION_ERROR,"string not found in string table: "+args[2]);

		// lookup the channel
		IChannel* channel= context.Module->getChannelByName(args[0]);
		if (channel==NULL) return CTxtCommandResult(CTxtCommandResult::EXECUTION_ERROR,"channel not found: "+args[0]);

		// if string already marked as used in used string table then abort
		CEventChatModule::TTextFlags& txtFlags= context.Module->getPreparedTextFlags();
		DROP_IF(txtFlags.find(args[0])!=txtFlags.end(),"This prepared text has already been used in this channel",return true);

		// mark string as used in used string table
		txtFlags.insert(args[0]);

		// send the message
		CSString pseudo= args[1];
		channel->broadcastMessage(pseudo,txt.toUtf8());

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: txtLoad
	//-----------------------------------------------------------------------------

	TXT_COMMAND(txtLoad,ECCCCommandSet,CCtrlChannelContext)
	{
		// txtLoad <fileSpec>			- load one or more event text files

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (args.size()!=1)			return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		GUS::CText& prepTxt= context.Module->getPreparedText();
		bool result= prepTxt.read(args[0]);

		// clear used string table
		CEventChatModule::TTextFlags& txtFlags= context.Module->getPreparedTextFlags();
		txtFlags.clear();

		return result? true: CTxtCommandResult(CTxtCommandResult::EXECUTION_ERROR,"Error reading input file: "+args[0]);
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: txtList
	//-----------------------------------------------------------------------------

	TXT_COMMAND(txtList,ECCCCommandSet,CCtrlChannelContext)
	{
		// txtList						- list the event texts that have been pre-prepared

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (args.size()!=0)			return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		// get hold of the prepared text and associated flags from the module in the context
		GUS::CText& prepTxt= context.Module->getPreparedText();
		CEventChatModule::TTextFlags& txtFlags= context.Module->getPreparedTextFlags();

		// display info about languages etc
		prepTxt.display();

		// get hold of the set of text handles in the text set (across all languages)
		typedef std::set<NLMISC::CSString> TTxtNames;
		TTxtNames txtNames;
		prepTxt.getTokenNameSet(txtNames);

		// display an 'ls' style output
		CSString line;
		for (TTxtNames::iterator it= txtNames.begin();it!=txtNames.end();++it)
		{
			CSString name= *it;
			if (txtFlags.find(*it)!=txtFlags.end())
				name="#"+name;
			if (!line.empty() && line.size()+name.size()>80)
			{
				context.Channel->sendMessage(context.ClientId,"texts",line);
				line.clear();
			}
			line+= " "+name;
		}
		context.Channel->sendMessage(context.ClientId,"texts",line);

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: txtShow
	//-----------------------------------------------------------------------------

	TXT_COMMAND(txtShow,ECCCCommandSet,CCtrlChannelContext)
	{
		// txtShow <txt_id>				- show the event text corresponding to the given text id

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (args.size()!=1)			return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		// lookup the string in the localised string container
		GUS::CText& prepTxt= context.Module->getPreparedText();
		ucstring txt= prepTxt.get(args[0]);
		if (txt.empty()) return CTxtCommandResult(CTxtCommandResult::EXECUTION_ERROR,"string not found in string table: "+args[2]);

		// send the message
		CEventChatModule::TTextFlags& txtFlags= context.Module->getPreparedTextFlags();
		CSString speakerName= (txtFlags.find(args[0])==txtFlags.end())? args[0]: "#"+args[0];
		context.Channel->sendMessage(context.ClientId,speakerName,txt.toUtf8());

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: spy
	//-----------------------------------------------------------------------------

	TXT_COMMAND(spy,ECCCCommandSet,CCtrlChannelContext)
	{
		// spy <channel>				- add self as an invisible member to a given party chat / faction chat

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (args.size()!=1)			return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		// lookup the channel
		IChannel* channel= context.Module->getChannelByName(args[0]);
		if (channel==NULL) return CTxtCommandResult(CTxtCommandResult::EXECUTION_ERROR,"channel not found: "+args[0]);

		// *** todo ***
		// *** add this user explicitly to the chat channel as a special user type (not arch, officer or member)

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: ctrlMembers
	//-----------------------------------------------------------------------------

	TXT_COMMAND(ctrlMembers,ECCCCommandSet,CCtrlChannelContext)
	{
		// ctrlMembers [...]	- Display, add or remove members for the channel 
		  // ctrlMembers				- display a list of all members
		  // ctrlMembers +ravna +jo	- add 'ravna' and 'jo' to the members list
		  // ctrlMembers -ravna -jo	- remove 'ravna' and 'jo' from the member list

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (context.Channel==NULL)	return CTxtCommandResult(CTxtCommandResult::EXECUTION_ERROR,"ctrl channel not found: "+args[0]);

		CSString result= context.Channel->getMembers().processCommand(rawArgs);
		sendLongTextToClient(context.ClientId,context.Channel,"*",result);

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: ctrlOfficers
	//-----------------------------------------------------------------------------

	TXT_COMMAND(ctrlOfficers,ECCCCommandSet,CCtrlChannelContext)
	{
		// ctrlOfficers [...]	- Display, add or remove officers for the channel
		  // ctrlOfficers				- display a list of all officers
		  // ctrlOfficers +ravna +jo	- add 'ravna' and 'jo' to the officers list (promote from mmember or add from 0)
		  // ctrlOfficers -ravna -jo	- remove 'ravna' and 'jo' from the officers list (demote to members)

		if (context.Rank<ARCH)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (context.Channel==NULL)	return CTxtCommandResult(CTxtCommandResult::EXECUTION_ERROR,"ctrl channel not found: "+args[0]);

		CSString result= context.Channel->getOfficers().processCommand(rawArgs);
		sendLongTextToClient(context.ClientId,context.Channel,"*",result);

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: ctrlArchs
	//-----------------------------------------------------------------------------

	TXT_COMMAND(ctrlArchs,ECCCCommandSet,CCtrlChannelContext)
	{
		// ctrlArchs	- display the arch list

		CSString archs= context.Channel->getArchs().processCommand("");
		context.Channel->sendMessage(context.ClientId,"archs",archs);
		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: faction
	//-----------------------------------------------------------------------------

	TXT_COMMAND(faction,ECCCCommandSet,CCtrlChannelContext)
	{
		// faction <name> [...]	- Perform an operation on a faction channel:
		  // faction +bob +frank		- add a faction called bob and another called frank
		  // faction -bob -frank		- remove a facion called bob and another called frank
		  // faction bob				- display arch user list for the 'bob' faction
		  // faction bob +dude +bert	- add 'dude' and 'bert' players to arch list for 'bob' faction
		  // faction bob -dude -bert	- remove 'dude' and 'bert' from 'bob' arch player list

		if (context.Rank<OFFICER)		return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		if (rawArgs.strip().empty())	return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);

		CSString argTxt= rawArgs;
		if (argTxt.firstWord()=="+" || argTxt.firstWord()=="-")
		{
			// faction +bob +frank		- add a faction called bob and another called frank
			// faction -bob -frank		- remove a facion called bob and another called frank
			CVectorSString words;
			argTxt.splitWords(words);

			// perform a quick syntax check
			if ((words.size()&1)!=0)	return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);
			for (uint32 i=0;i<words.size();i+=2)
			{
				if (words[i]!='+' && words[i]!='-')	return CTxtCommandResult(CTxtCommandResult::SYNTAX_ERROR);
			}

			for (uint32 i=0;i<words.size();i+=2)
			{
				if (words[i]=='+')
				{
					context.Module->addFactionChannel("ec_faction_"+words[i+1],words[i+1]);
				}
				else
				{
					context.Module->removeFactionChannel(words[i+1]);
				}
			}
		}
		else
		{
			// faction bob				- display officer list for the 'bob' faction
			// faction bob +dude +bert	- add 'dude' and 'bert' players to arch list for 'bob' faction
			// faction bob -dude -bert	- remove 'dude' and 'bert' from 'bob' arch player list
			CSString channelName= argTxt.firstWord(true);
			IChannel* channel= context.Module->getFactionChannelByName(channelName);
			if (channel==NULL) return false;

			CSString result= context.Channel->getOfficers().processCommand(rawArgs);
			sendLongTextToClient(context.ClientId,context.Channel,"*",result);
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: factionsOpen
	//-----------------------------------------------------------------------------

	TXT_COMMAND(factionsOpen,ECCCCommandSet,CCtrlChannelContext)
	{
		/// factionsOpen			- open the faction channels on all player screens

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		// *** todo ***

		return true;
	}

	//-----------------------------------------------------------------------------
	// Command for the event ctrl channel: factionsClose
	//-----------------------------------------------------------------------------

	TXT_COMMAND(factionsClose,ECCCCommandSet,CCtrlChannelContext)
	{
		// factionsClose		- close all faction and party channels associated with the module

		if (context.Rank<OFFICER)	return CTxtCommandResult(CTxtCommandResult::UNKNOWN_COMMAND);
		// *** todo ***

		return true;
	}
}
