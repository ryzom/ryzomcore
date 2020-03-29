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

#ifndef EC_PARTY_CHANNEL_H
#define EC_PARTY_CHANNEL_H

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
	// class CPartyChannel
	//-----------------------------------------------------------------------------

	class CPartyChannel: public IChannel
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

		CPartyChannel(const NLMISC::CSString& chatChannelName,const NLMISC::CSString& title);


		//-----------------------------------------------------------------------------
		// interface for CPartyChannelContext to use

		TChannelRank getMinChatRank() const;
		void setMinChatRank(TChannelRank rank);


	private:
		//-----------------------------------------------------------------------------
		// private data

		TChannelRank		_MinChatRank;
	};
}


//-----------------------------------------------------------------------------
#endif
