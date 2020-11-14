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

#ifndef EC_FACTION_CHANNEL_H
#define EC_FACTION_CHANNEL_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "ec_channel.h"


//-----------------------------------------------------------------------------
// EC namespace
//-----------------------------------------------------------------------------

namespace EC
{
	//-----------------------------------------------------------------------------
	// class CFactionChannel
	//-----------------------------------------------------------------------------

	class CFactionChannel: public IChannel
	{
	public:
		//-----------------------------------------------------------------------------
		// IChannel Specialisation

		virtual void cbAddUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId);
		virtual void cbRemoveUser(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId);
		virtual void cbChatText(TChannelRank rank,const TCharacterId& id,GUS::TClientId clientId,const NLMISC::CSString& txt);


	public:
		//-----------------------------------------------------------------------------
		// public interface

		CFactionChannel(CEventChatModule* theModule,const NLMISC::CSString& name);


		//-----------------------------------------------------------------------------
		// interface for CFactionChannelContext to use

		bool setupParty(const NLMISC::CSString& ownerCharacterName,const NLMISC::CSString& title);
		void removeParty(const NLMISC::CSString& ownerCharacterName);

		uint32 getPartyCount() const;
		CPartyChannel* getParty(uint32 idx);


	private:
		//-----------------------------------------------------------------------------
		// private data

		CEventChatModule*	_TheModule;
		TChannelRank		_MinChatRank;
		typedef std::map<TCharacterId,TPartyChannelPtr>	TParties;
		TParties			_Parties;
		typedef std::vector<TCharacterId> TPartyIdx;
		TPartyIdx			_PartyIdx;
	};
}


//-----------------------------------------------------------------------------
#endif
