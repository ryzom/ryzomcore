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

#ifndef GUS_NET_IMPLEMENTATION_H
#define GUS_NET_IMPLEMENTATION_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nel
#include "nel/net/callback_net_base.h"

// local
#include "gus_net.h"


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// class CGusNetImplementation
	//-----------------------------------------------------------------------------

	class CGusNetImplementation: public CGusNet
	{
	private:
		// singleton so ctor is private
		CGusNetImplementation();

	public:
		// get hold of the singleton instance
		static CGusNetImplementation* getInstance();

	public:
		// CGusNet interface
		virtual void registerModule(GUS::TModulePtr module);
		virtual void unregisterModule(GUS::TModulePtr module);
		virtual void sendMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,uint32 destinationModuleId,uint32 senderModuleId);
		virtual void sendMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,const TModuleIdVector& destinationModuleIds,uint32 senderModuleId);
		virtual void broadcastMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,uint32 senderModuleId);
		virtual void display() const;

	public:
		// method used to register / unregister CModule objects with this singleton
		void addHubModule(THubModulePtr hub);
		void addConnectionModule(TConnectionModulePtr connection);

		void removeHubModule(CHubModule* theHub);
		void removeConnectionModule(CConnectionModule* theConnection);

		// hub container accessors
		THubModulePtr getHub(NLNET::CCallbackNetBase &netbase);
		THubModulePtr getHub(NLMISC::CSString& name);
		THubModulePtr getHub(uint32 idx);
		uint32 getNumHubs();

		// connection container accessors
		TConnectionModulePtr getConnection(NLNET::CCallbackNetBase &netbase);
		TConnectionModulePtr getConnection(uint32 idx);
		uint32 getNumConnections();

		// remote modules accessors
		void registerRemoteModule(IRemoteModule* module);
		void unregisterRemoteModule(IRemoteModule* module);
		TRemoteModulePtr lookupRemoteModule(uint32 uniqueId);


	private:
		// private data

		// the set of all hub modules
		typedef std::vector<THubModulePtr> THubs;
		THubs _Hubs;

		// the set of all connection modules
		typedef std::vector<TConnectionModulePtr> TConnections;
		TConnections _Connections;

		// the set of remote modules (ie modules visible via the different hubs) - indexed by unique id
		typedef std::map<uint32,TRemoteModulePtr> TRemoteModules;
		TRemoteModules _RemoteModules;
	};
}


//-----------------------------------------------------------------------------
#endif
