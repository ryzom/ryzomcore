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

// game_share
#include "game_share/utils.h"

// local
#include "gus_net_implementation.h"
#include "gus_net_hub.h"
#include "gus_net_connection.h"
#include "gus_net_remote_module.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;


//-----------------------------------------------------------------------------
// GUSNET namespace
//-----------------------------------------------------------------------------

namespace GUSNET
{
	//-----------------------------------------------------------------------------
	// methods CGusNetImplementation
	//-----------------------------------------------------------------------------

	CGusNetImplementation::CGusNetImplementation()
	{
	}

	CGusNetImplementation* CGusNetImplementation::getInstance()
	{
		static CGusNetImplementation* ptr=NULL;
		if (ptr==NULL)
			ptr= new CGusNetImplementation;
		return ptr;
	}

	void CGusNetImplementation::addHubModule(THubModulePtr hub)
	{
		// make sure the hub wasn't already registered
		for (uint32 i=0;i<_Hubs.size();++i)
			BOMB_IF(_Hubs[i]==hub,"Attempt to add the same hub to the CGusNetImplementation singleton more than once",return)

		// add the new hub
		nldebug("GUSNET: Registering new hub: %d",hub->getPort());
		_Hubs.push_back(hub);
	}

	void CGusNetImplementation::addConnectionModule(TConnectionModulePtr connection)
	{
		// make sure the connection wasn't already registered
		for (uint32 i=0;i<_Connections.size();++i)
			BOMB_IF(_Connections[i]==connection,"Attempt to add the same connection to the CGusNetImplementation singleton more than once",return)

		// add the new connection
		nldebug("GUSNET: Registering new connection: %s",connection->getConnectionAddress().c_str());
		_Connections.push_back(connection);
	}

	void CGusNetImplementation::removeHubModule(CHubModule* theHub)
	{
		for (THubs::iterator it=_Hubs.begin();it!=_Hubs.end();++it)
		{
			if ((*it)==theHub)
			{
				nldebug("GUSNET: Unregistering hub: %d",theHub->getPort());
				_Hubs.erase(it);
				return;
			}
		}

		STOP("Failed to remove vub from hub vector because no match found");
	}

	void CGusNetImplementation::removeConnectionModule(CConnectionModule* theConnection)
	{
		for (TConnections::iterator it=_Connections.begin();it!=_Connections.end();++it)
		{
			if ((*it)==theConnection)
			{
				nldebug("GUSNET: Unregistering connection: %s",theConnection->getConnectionAddress().c_str());
				_Connections.erase(it);
				return;
			}
		}

		STOP("Failed to remove connection from connection vector because no match found");
	}

	THubModulePtr CGusNetImplementation::getHub(CCallbackNetBase &netbase)
	{
		// identify the hub that this message is being sent to
		THubs::iterator hubit;
		for (hubit=_Hubs.begin();hubit!=_Hubs.end();++hubit)
		{
			if (safe_cast<CCallbackNetBase*>((*hubit)->getCbServer())==&netbase)
				return (*hubit);
		}
		BOMB("Module registration message received for unknown hub!",return NULL);
	}

	THubModulePtr CGusNetImplementation::getHub(NLMISC::CSString& name)
	{
		for (uint32 i=0;i<_Hubs.size();++i)
			if (_Hubs[i]->getName()==name)
				return _Hubs[i];
		BOMB("No hub found with name: "+name,return NULL);
	}

	THubModulePtr CGusNetImplementation::getHub(uint32 idx)
	{
		BOMB_IF(idx>=_Hubs.size(),"Attempt to access passed the end of the hubs vector!!!",return NULL);
		return _Hubs[idx];
	}

	uint32 CGusNetImplementation::getNumHubs()
	{
		return _Hubs.size();
	}

	TConnectionModulePtr CGusNetImplementation::getConnection(CCallbackNetBase &netbase)
	{
		// identify the connection that this message is being sent to
		TConnections::iterator connectionit;
		for (connectionit=_Connections.begin();connectionit!=_Connections.end();++connectionit)
		{
			if (safe_cast<CCallbackNetBase*>((*connectionit)->getCbClient())==&netbase)
				return (*connectionit);
		}
		BOMB("Module registration message received for unknown hub!",return NULL);
	}

	TConnectionModulePtr CGusNetImplementation::getConnection(uint32 idx)
	{
		BOMB_IF(idx>=_Connections.size(),"Attempt to access passed the end of the connections vector!!!",return NULL);
		return _Connections[idx];
	}

	uint32 CGusNetImplementation::getNumConnections()
	{
		return _Connections.size();
	}

	void CGusNetImplementation::registerModule(TModulePtr module)
	{
		// register the new module with all of the active connections
		nldebug("GUSNET: Registering Module: %s %s",module->getName().c_str(),module->getParameters().c_str());
		for (TConnections::iterator it=_Connections.begin();it!=_Connections.end();++it)
		{
			(*it)->registerModule(module);
		}
	}

	void CGusNetImplementation::unregisterModule(TModulePtr module)
	{
		// register the new module with all of the active connections
		nldebug("GUSNET: Unrgistering Module: %s %s",module->getName().c_str(),module->getParameters().c_str());
		for (TConnections::iterator it=_Connections.begin();it!=_Connections.end();++it)
		{
			(*it)->unregisterModule(module);
		}
	}

	void CGusNetImplementation::sendMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,uint32 destinationModuleId, uint32 senderModuleId)
	{
		TRemoteModulePtr remoteModule= lookupRemoteModule(destinationModuleId);
		DROP_IF(remoteModule==NULL,"Failed to find remote module to send the message to",return);

		remoteModule->sendMessage(msgName,msgBody,senderModuleId);
	}

	void CGusNetImplementation::sendMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,const TModuleIdVector& destinationModuleIds, uint32 senderModuleId)
	{
		// display a quick debug msg
		CSString s;
		for (uint32 i=0;i<destinationModuleIds.size();++i) s+=NLMISC::toString("%d ",destinationModuleIds[i]);
		nldebug("GUSNET: Send Module Message: %s  to: %s",msgName.c_str(),s.c_str());

		// build a vector of module ptrs from the unique module ids
		std::vector<TRemoteModulePtr> modules;
		for (uint32 i=0;i<destinationModuleIds.size();++i)
		{
			TRemoteModulePtr ptr= lookupRemoteModule(destinationModuleIds[i]);
			if (ptr!=NULL)
				modules.push_back(ptr);
		}

		// for each network connection
		for (uint32 i=0;i<_Connections.size();++i)
		{
			TModuleIdVector remoteModuleIds;

			// extract remote ids of modules that are found on this connection
			for (uint32 j=0;j<modules.size();++j)
			{
				if (_Connections[i]->isRemoteModuleConnected(modules[j]))
					remoteModuleIds.push_back(modules[j]->getRemoteId());
			}

			// if we found any matching ids then send a message to this connection
			if (!remoteModuleIds.empty())
			{
				// display a quick debug msg
				CSString s;
				for (uint32 k=0;k<remoteModuleIds.size();++k) s+=NLMISC::toString("%d ",remoteModuleIds[k]);
				nldebug("GUSNET: Send Module Message: %s  to: %s On Connection: %s",msgName.c_str(),s.c_str(),_Connections[i]->getConnectionAddress().c_str());

				_Connections[i]->sendMessage(msgName,msgBody,remoteModuleIds,senderModuleId);
			}
		}
	}

	void CGusNetImplementation::broadcastMessage(const CSString& msgName, const TRawMsgBodyPtr& msgBody,uint32 senderModuleId)
	{
		// for each GUSNET network connection...
		for (uint32 i=0;i<_Connections.size();++i)
		{
			// have the message broadcast..
			_Connections[i]->broadcastMessage(msgName,msgBody,senderModuleId);
		}
	}

	void CGusNetImplementation::display() const
	{
		// display header
		InfoLog->displayNL("==== Displaying net connection state ...");

		// display connection modules with state info
		if (!_Connections.empty())
		{
			InfoLog->displayNL("==== Connections:");
			for (TConnections::const_iterator it=_Connections.begin();it!=_Connections.end();++it)
			{
				InfoLog->displayNL("- %s",(*it)->getState().c_str());
			}
		}

		// display hub modules with state info
		if (!_Hubs.empty())
		{
			InfoLog->displayNL("==== Hubs:");
			for (THubs::const_iterator it=_Hubs.begin();it!=_Hubs.end();++it)
			{
				InfoLog->displayNL("- %s",(*it)->getState().c_str());
			}
		}

		// display remote modules with state info
		if (!_RemoteModules.empty())
		{
			CVectorSString lines;
			InfoLog->displayNL("==== Remote Modules:");
			for (TRemoteModules::const_iterator it=_RemoteModules.begin();it!=_RemoteModules.end();++it)
			{
				lines.push_back((*it).second->getInfoString());
			}
			std::sort(lines.begin(),lines.end());
			for (uint32 i=0;i<lines.size();++i)
			{
				InfoLog->displayNL("- %s",lines[i].c_str());
			}

		}

		// display footer
		InfoLog->displayNL("==== ... End of display of net connection state");
	}

	void CGusNetImplementation::registerRemoteModule(IRemoteModule* module)
	{
		// make sure the module id is valid
		nlassert(module->getUniqueId()!=InvalidRemoteModuleId);
		// make sure the module doesn't already exist
		nlassert(_RemoteModules.find(module->getUniqueId())==_RemoteModules.end());
		// register the module
//		nldebug("GUSNET: Register remote module: Id: %d (Remote Id: %d): %s %s",module->getUniqueId(),module->getRemoteId(),module->getName().c_str(),module->getParameters().c_str());
		_RemoteModules[module->getUniqueId()]= module;
	}

	void CGusNetImplementation::unregisterRemoteModule(IRemoteModule* module)
	{
		// make sure the module id is valid
		nlassert(module->getUniqueId()!=InvalidRemoteModuleId);
		// make sure the module doesn't already exist
		BOMB_IF(_RemoteModules.find(module->getUniqueId())==_RemoteModules.end(),"Failed to find requested module",return);
		// unregister the module
//		nldebug("GUSNET: Unegister remote module: Id: %d (Remote Id: %d): %s %s",module->getUniqueId(),module->getRemoteId(),module->getName().c_str(),module->getParameters().c_str());
		_RemoteModules.erase(module->getUniqueId());
	}

	TRemoteModulePtr CGusNetImplementation::lookupRemoteModule(uint32 uniqueId)
	{
		BOMB_IF(_RemoteModules.find(uniqueId)==_RemoteModules.end(),"Failed to find requested module",return NULL);
		return _RemoteModules[uniqueId];
	}


	//-----------------------------------------------------------------------------
	// CGusNetImplementation instantiator
	//-----------------------------------------------------------------------------

	class CGusNetImplementationInstantiator
	{
	public:
		CGusNetImplementationInstantiator()
		{
			CGusNet::getInstance();
		}
	};
	static CGusNetImplementationInstantiator NetInstantiator;
}
