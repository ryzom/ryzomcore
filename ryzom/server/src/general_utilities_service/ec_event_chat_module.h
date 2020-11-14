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

#ifndef EC_EVENT_CHAT_MODULE_H
#define EC_EVENT_CHAT_MODULE_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_module_manager.h"
#include "gus_text.h"
#include "ec_types.h"


//-----------------------------------------------------------------------------
// EC namespace
//-----------------------------------------------------------------------------

namespace EC
{
	//-----------------------------------------------------------------------------
	// class CEventChatModule
	//-----------------------------------------------------------------------------

	class CEventChatModule: public GUS::IModule
	{
	public:
		// IModule specialisation implementation
		bool initialiseModule(const NLMISC::CSString& rawArgs);
		NLMISC::CSString getState() const;
		NLMISC::CSString getName() const;
		NLMISC::CSString getParameters() const;
		void displayModule() const;

		// management of the set of faction channels
		bool addFactionChannel(const NLMISC::CSString& channelName,const NLMISC::CSString& channelTitle);
		bool removeFactionChannel(const NLMISC::CSString& name);

		// a set of accessors for the faction channels
		uint32 getNumFactionChannels() const;
		CFactionChannel* getFactionChannel(uint32 idx);
		CFactionChannel* getFactionChannelByName(const NLMISC::CSString& name,bool addIfNotExist=false);

		// a set of accessors that englobes the ctrl channel, faction channels, and the factions' party channels
		uint32 getNumChannels() const;
		IChannel* getChannel(uint32 idx);
		IChannel* getChannelByName(const NLMISC::CSString& name);

		// accessors for the module's prepared texts
		typedef std::set<NLMISC::CSString> TTextFlags;
		GUS::CText& getPreparedText();
		TTextFlags& getPreparedTextFlags();

	public:
		// remaining public interface
		CEventChatModule();

	private:
		TCtrlChannelPtr		_CtrlChannel;
		typedef std::vector<TFactionChannelPtr>	TFactionChannels;
		TFactionChannels	_FactionChannels;
		GUS::CText			_PreparedText;
		TTextFlags			_PreparedTextFlags;
	};
}


//-----------------------------------------------------------------------------
#endif
