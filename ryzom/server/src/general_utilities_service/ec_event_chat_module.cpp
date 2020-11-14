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

// game share
#include "game_share/utils.h"

// local
#include "ec_event_chat_module.h"
#include "ec_party_channel.h"
#include "ec_faction_channel.h"
#include "ec_ctrl_channel.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;


//-----------------------------------------------------------------------------
// EC namespace
//-----------------------------------------------------------------------------

namespace EC
{
	//-----------------------------------------------------------------------------
	// methods CEventChatModule
	//-----------------------------------------------------------------------------

	CEventChatModule::CEventChatModule()
	{
	}

	bool CEventChatModule::initialiseModule(const NLMISC::CSString& rawArgs)
	{
		// make sure that we don't instantiate an event chat with no arch usres
		DROP_IF(rawArgs.strip().empty(),"syntax error: Should be: modulesAdd ec <arch character name> [<arch character name>...]",return false);

		// convert the rawArgs to a string that's parsable by a CUserGroup object
		CVectorSString ids;
		rawArgs.splitWordOrWords(ids);
		CSString idList="+"+CSString().join(ids," +");
		InfoLog->displayNL("EC Instantiating event ctrl channel with following arch command: %s",idList.c_str());

		// instantiate the control channel and add the arch users
		_CtrlChannel= new CCtrlChannel(this);
		CSString result;
		result= _CtrlChannel->getArchs().processCommand(idList);
		InfoLog->displayNL("EC result: %s",result.c_str());

		// success so return true
		return true;
	}

	NLMISC::CSString CEventChatModule::getState() const
	{
		// *** todo ***
		// list factions
		// say whether faction windows open or closed
		return getName()+" "+getParameters();
	}

	NLMISC::CSString CEventChatModule::getName() const
	{
		return "EC";
	}

	NLMISC::CSString CEventChatModule::getParameters() const
	{
		CVectorSString archNames;
		_CtrlChannel->getArchs().processCommand("").splitLines(archNames);

		CSString result;
		result.join(archNames,' ');

		return result.c_str();
	}

	void CEventChatModule::displayModule() const
	{
		// *** todo ***
		//	- list factions with:
		//		- faction name
		//		- arch list with:
		//			- arch name
		//			- arch's channel name (if open)
		//			- mode of arch's channel
		//			- number of officers / members in arch's channel
		//	- list loaded text files
		//	- list archs and officers for the ctrl channel
	}

	bool CEventChatModule::addFactionChannel(const NLMISC::CSString& channelName,const NLMISC::CSString& channelTitle)
	{
		// make sure there isn't already a channel of this name
		for (uint32 i=getNumFactionChannels();i--;)
		{
			CFactionChannel* chan= getFactionChannel(i);
			if (chan->getChannelTitle()==channelTitle) return false;
			if (chan->getChannelName()==channelName) return false;
		}

		CFactionChannel* newChan= new CFactionChannel(this,channelName);
		newChan->setChannelTitle(channelTitle);
		_FactionChannels.push_back(newChan);
		return true;
	}

	bool CEventChatModule::removeFactionChannel(const NLMISC::CSString& name)
	{
		bool found= false;
		for (uint32 i=getNumFactionChannels();i--;)
		{
			CFactionChannel* chan= getFactionChannel(i);
			nlassert(chan!=NULL);
			if (	(chan->getChannelTitle()==name)
				||	(chan->getChannelName().right(name.size()+1)=="_"+name)
				||	(chan->getChannelName()==name)
				)
			{
				_FactionChannels[i]=_FactionChannels.back();
				_FactionChannels.pop_back();
				found= true;
			}
		}
		return found;
	}

	uint32 CEventChatModule::getNumFactionChannels() const
	{
		return _FactionChannels.size();
	}

	CFactionChannel* CEventChatModule::getFactionChannel(uint32 idx)
	{
		nlassert(idx<_FactionChannels.size());
		return _FactionChannels[idx];
	}

	CFactionChannel* CEventChatModule::getFactionChannelByName(const NLMISC::CSString& name,bool addIfNotExist)
	{
		for (uint32 i=getNumFactionChannels();i--;)
		{
			CFactionChannel* chan= getFactionChannel(i);
			nlassert(chan!=NULL);
			if (chan->getChannelTitle()==name)
				return chan;
			if (chan->getChannelName().right(name.size()+1)=="_"+name)
				return chan;
			if (chan->getChannelName()==name)
				return chan;
		}
		return NULL;
	}

	uint32 CEventChatModule::getNumChannels() const
	{
		uint32 count=0;

		// add the control channel
		++count;

		// add the faction channels
		count+= _FactionChannels.size();

		// add the party channels
		for (uint32 i=0;i<_FactionChannels.size();i++)
		{
			count+= _FactionChannels[i]->getPartyCount();
		}

		// all done
		return count;
	}

	IChannel* CEventChatModule::getChannel(uint32 idx)
	{
		// try to match the ctrl channel
		if (idx==0)
			return _CtrlChannel;
		else
			--idx;

		// try to match a faction channel
		if (idx<_FactionChannels.size())
			return _FactionChannels[idx];
		else
			idx-= _FactionChannels.size();

		// try to match a party channel
		for (uint32 i=0;i<_FactionChannels.size();i++)
		{
			if (idx<_FactionChannels[i]->getPartyCount())
				return _FactionChannels[i]->getParty(idx);
			else
				idx-= _FactionChannels[i]->getPartyCount();
		}

		// no match found :( - idx must have been invalid
		nlerror("idx value out of range in CEventChatModule::getChannel(idx)");
		return NULL;
	}

	IChannel* CEventChatModule::getChannelByName(const NLMISC::CSString& name)
	{
		for (uint32 i=getNumChannels();i--;)
		{
			IChannel* chan= getChannel(i);
			nlassert(chan!=NULL);
			if (chan->getChannelName()==name)
				return chan;
		}
		return NULL;
	}

	GUS::CText& CEventChatModule::getPreparedText()
	{
		return _PreparedText;
	}

	CEventChatModule::TTextFlags& CEventChatModule::getPreparedTextFlags()
	{
		return _PreparedTextFlags;
	}


	//-----------------------------------------------------------------------------
	// CEventChatModule registration
	//-----------------------------------------------------------------------------

	REGISTER_GUS_MODULE(CEventChatModule,"EC","<arch character name> [<arch character name>...]","Event chat manager")

}
