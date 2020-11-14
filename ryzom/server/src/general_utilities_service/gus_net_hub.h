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

#ifndef GUS_NET_HUB_H
#define GUS_NET_HUB_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/net/callback_server.h"

#include "gus_net_types.h"
#include "gus_module_factory.h"


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// class CHubModule
	//-----------------------------------------------------------------------------

	class CHubModule: public GUS::IModule
	{
	public:
		// IModule specialisation implementation
		bool initialiseModule(const NLMISC::CSString& rawArgs);
		void release();
		void serviceUpdate(NLMISC::TTime localTime);
		NLMISC::CSString getState() const;
		NLMISC::CSString getName() const;
		NLMISC::CSString getParameters() const;
		void displayModule() const;

	public:
		// remaining public interface
		CHubModule();

		// management of registration / unregistration of remote modules
		void registerRemoteModule(TRemoteModuleOnHubPtr module);
		void unregisterRemoteModule(TRemoteModuleOnHubPtr module);
		void unregisterAllRemoteModules(NLNET::TSockId tsid);
		void treatNewConnection(NLNET::TSockId tsid);

		// broadcasting of messages to the remote modules
		void forwardBroadcastMessage(NLNET::CMessage &msgin,NLNET::TSockId tsid);
		void forwardModuleMessage(NLNET::CMessage &msgin,NLNET::TSockId tsid);
		void sendMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody, NLNET::TSockId sockId, const TModuleIdVector& remoteIds,TRemoteModuleId senderModuleId=InvalidRemoteModuleId);
		void broadcastMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,TRemoteModuleId senderModuleId=InvalidRemoteModuleId);

		// simple read accessors
		const NLMISC::CSString&	getHubName() const;
		uint16					getPort() const;
		NLNET::CCallbackServer* getCbServer() const;
		CRemoteModuleOnHub*		getRemoteModuleByRemoteId(NLNET::TSockId tsid,uint32 remoteId) const;

	private:
		// the hub's name
		NLMISC::CSString _HubName;

		// private data
		uint16 _Port;

		// the callback server for the network connection
		NLNET::CCallbackServer* _CbServer;

		// the set of remote modules
		typedef std::map<uint32,TRemoteModuleOnHubPtr> TRemoteModules;
		TRemoteModules _RemoteModules;
	};
}

//-----------------------------------------------------------------------------
#endif
