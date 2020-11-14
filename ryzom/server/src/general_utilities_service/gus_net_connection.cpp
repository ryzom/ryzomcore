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
#include "gus_module_manager.h"
#include "gus_net_connection.h"
#include "gus_net_implementation.h"
#include "gus_net_remote_module.h"
#include "gus_net_messages.h"


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
	// callbacks
	//-----------------------------------------------------------------------------

	static void cbRegisterModuleFromHub(CMessage &msgin, TSockId tsid, CCallbackNetBase &netbase)
	{
		// identify the connection that this message is being sent to
		TConnectionModulePtr connection= CGusNetImplementation::getInstance()->getConnection(netbase);
		BOMB_IF(connection==NULL,"Connection not found in cbRegisterModuleFromHub",return);

		// decode the NLNET message
		CMsgRegisterModule msg;
		msgin.serial(msg);
		nldebug("GUSNET: Received module connection: %d: %s",msg.getModuleId(),(msg.getModuleName()+" "+msg.getParameters()).c_str());

		// instantiate and initialise a new module object
		TRemoteModuleViaConnectionPtr theModule= new CRemoteModuleViaConnection;
		bool isOK= theModule->init(connection,msg);
		if (!isOK)
			return;

		// add the remote module to the hub that it's connected to
		connection->registerRemoteModule(theModule);
	}

	static void cbUnregisterModuleFromHub(CMessage &msgin, TSockId tsid, CCallbackNetBase &netbase)
	{
		// identify the connection that this message is being sent to
		TConnectionModulePtr connection= CGusNetImplementation::getInstance()->getConnection(netbase);
		BOMB_IF(connection==NULL,"Connection not found in cbUnregisterModuleFromHub",return);

		// decode the NLNET message
		CMsgUnregisterModule msg;
		msgin.serial(msg);

		// make sure the module exists
		IRemoteModule* modulePtr=connection->getRemoteModuleByRemoteId(msg.getModuleId());
		if (modulePtr==NULL)
		{
			nldebug("GUSNET: Received unknown module disconnection: remoteId %d",msg.getModuleId());
			return;
		}

		// remove the remote module from the connection that it's connected to
		nldebug("GUSNET: Received module disconnection: %d (remoteId %d): %s %s",modulePtr->getUniqueId(),msg.getModuleId(),
			modulePtr->getName().c_str(),modulePtr->getParameters().c_str());
		connection->unregisterRemoteModule(safe_cast<GUSNET::CRemoteModuleViaConnection*>(modulePtr));
	}

	static void cbModuleMessage(CMessage &msgin, TSockId tsid, CCallbackNetBase &netbase)
	{
		// identify the connection that this message is being sent to
		TConnectionModulePtr connection= CGusNetImplementation::getInstance()->getConnection(netbase);
		BOMB_IF(connection==NULL,"Connection not found in cbModuleMessage",return);

		connection->treatModuleMessage(msgin);
	}

	static void cbBroadcastMessage(CMessage &msgin, TSockId tsid, CCallbackNetBase &netbase)
	{
		// identify the connection that this message is being sent to
		TConnectionModulePtr connection= CGusNetImplementation::getInstance()->getConnection(netbase);
		BOMB_IF(connection==NULL,"Connection not found in cbBroadcastMessage",return);

		connection->treatBroadcastMessage(msgin);
	}


	//-----------------------------------------------------------------------------
	// methods CConnectionModule
	//-----------------------------------------------------------------------------

	CConnectionModule::CConnectionModule()
	{
		_CbClient= NULL;
		_IsConnected= false;
		_ReconnectDelayCounter= 0;
	}

	bool CConnectionModule::initialiseModule(const NLMISC::CSString& rawArgs)
	{
		static NLNET::TCallbackItem cbArray[] = 
		{
			{	REGISTER_MODULE_FROM_HUB,		cbRegisterModuleFromHub,	},
			{	UNREGISTER_MODULE_FROM_HUB,		cbUnregisterModuleFromHub,	},

			{	MODULE_MSG,						cbModuleMessage,			},
			{	BROADCAST_MSG,					cbBroadcastMessage,			},
		};

		// setup the network connection
		_CbClient = new CCallbackClient;
		_CbClient->addCallbackArray(cbArray, sizeof(cbArray)/sizeof(cbArray[0]));

		// establish the connection
		_ConnectionAddress= rawArgs.strip().unquoteIfQuoted().c_str();
		nldebug("GUSNET: Requesting connecting to hub: %s",_ConnectionAddress.c_str());

		try
		{
			_CbClient->connect(_ConnectionAddress);
		}
		catch(...)
		{
		}

		// now that we're initialised add us to the connection modules vector
		CGusNetImplementation::getInstance()->addConnectionModule(this);

		// success so return true
		return true;
	}

	void CConnectionModule::release()
	{
		nlassert(_CbClient!=NULL);

		// make sure we hang on to a smart pointer to avoid surprise delete in mid processing
		TModulePtr self=this;

		// remove the hub from the net singleton's hub list
		CGusNetImplementation::getInstance()->removeConnectionModule(this);

		// disconnect all clients
		if (_CbClient->connected())
			_CbClient->disconnect(InvalidSockId);
		delete _CbClient;
		_CbClient = 0;
	}

	void CConnectionModule::serviceUpdate(NLMISC::TTime localTime)
	{
		nlassert(_CbClient!=NULL);

		// update the connection
		_CbClient->flush();
		_CbClient->update(0);

		// check whether the connection has changed state from on line to off line
		if (!_IsConnected && _CbClient->connected())
		{
			nldebug("GUSNET: Hub is UP: %s",_ConnectionAddress.c_str());
			_IsConnected= true;

			// tell the hub all about the set of local modules
			CModuleManager::TModuleVector localModules;
			CModuleManager::getInstance()->getModules(localModules);
			for (uint32 i=0;i<localModules.size();++i)
			{
				nldebug("GUSNET: Sending module registration message to: %s  For module: %s %s",_ConnectionAddress.c_str(),localModules[i]->getName().c_str(),localModules[i]->getParameters().c_str());
				CMsgRegisterModule msg;
				msg.setup(CModuleManager::getInstance()->getModuleId(localModules[i]),localModules[i]->getName(),localModules[i]->getParameters());
				CMessage outmsg(REGISTER_MODULE_TO_HUB);
				outmsg.serial(msg);
				_CbClient->send(outmsg);
			}

		}

		// check whether the connection has changed state from off line to on line
		if (_IsConnected && !_CbClient->connected())
		{
			nldebug("GUSNET: Hub is DOWN: %s",_ConnectionAddress.c_str());
			_IsConnected= false;
			_ReconnectDelayCounter=0;
		}

		// think about trying to reconnect
		if (!_IsConnected)
		{
			++_ReconnectDelayCounter;
			if (_ReconnectDelayCounter>20)
			{
				try
				{
					_CbClient->connect(_ConnectionAddress);
				}
				catch(...)
				{
				}
				_ReconnectDelayCounter=0;
			}
		}
	}

	NLMISC::CSString CConnectionModule::getState() const
	{
		return NLMISC::toString("GUSNET %s:  RemoteModules: %d  Status: %s",_ConnectionAddress.c_str(),_RemoteModules.size(),_IsConnected?"CONNECTED":"NOT CONNECTED");
	}

	NLMISC::CSString CConnectionModule::getName() const
	{
		return "GUSNET";
	}

	NLMISC::CSString CConnectionModule::getParameters() const
	{
		return _ConnectionAddress;
	}

	void CConnectionModule::displayModule() const
	{
		// title
		InfoLog->displayNL("GUSNET Connection: %s [%s]",_ConnectionAddress.c_str(),_IsConnected?"CONNECTED":"NOT CONNECTED");

		// remote modules
		for (TRemoteModules::const_iterator it=_RemoteModules.begin(); it!=_RemoteModules.end(); ++it )
		{
			InfoLog->displayNL("- Remote Module: %4d (%d):  %s %s",(*it).first,(*it).second->getRemoteId(),(*it).second->getName().c_str(),(*it).second->getParameters().c_str());
		}

		// nel net stats
//		InfoLog->displayNL("");
//		InfoLog->displayNL("ReceiveQueueStat");
//		_CbClient->displayReceiveQueueStat();

//		InfoLog->displayNL("");
//		InfoLog->displayNL("SendQueueStat");
//		_CbClient->displaySendQueueStat();
		
//		InfoLog->displayNL("");
//		InfoLog->displayNL("ThreadStat");
//		_CbClient->displayThreadStat();
	}

	void CConnectionModule::sendMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,const TModuleIdVector& remoteIds,uint32 senderModuleId)
	{
		DROP_IF(!_CbClient->connected()||!_IsConnected,"Failed to send message via disconnected GUSNET module",return);

		// setup a CModuleMessage
		CModuleMessage msg;
		msg.setMessageName(msgName);
		msg.setMsgBody(msgBody);
		msg.setSenderId(senderModuleId);
		msg.setDestinationModuleIds(remoteIds);

		// serialise the CModuleMessage to a CMessage
		CMessage msgOut(MODULE_MSG);
		msgOut.serial(msg);

		// do the send
		_CbClient->send(msgOut);
	}

	void CConnectionModule::broadcastMessage(const NLMISC::CSString& msgName, const TRawMsgBodyPtr& msgBody,uint32 senderModuleId)
	{
		DROP_IF(!_CbClient->connected()||!_IsConnected,"Failed to broadcast message via disconnected GUSNET module",return);

		// setup a CModuleMessage
		CModuleMessage msg;
		msg.setMessageName(msgName);
		msg.setMsgBody(msgBody);
		msg.setSenderId(senderModuleId);

		// serialise the CModuleMessage to a CMessage
		CMessage msgOut(BROADCAST_MSG);
		msgOut.serial(msg);

		// do the send
		_CbClient->send(msgOut);
	}

	void CConnectionModule::treatBroadcastMessage(NLNET::CMessage &msgin)
	{
		CModuleMessage msg;
		msgin.serial(msg);

		// get hold of a vector of all of the active modules
		CModuleManager::TModuleVector modules;
		CModuleManager::getInstance()->getModules(modules);

		// remap the sender id from the local id on the emitter to the uniqueId of the remote module here
		TRemoteModuleViaConnectionPtr remoteModule= getRemoteModuleByRemoteId(msg.getSenderId());
		uint32 senderUniqueId= (remoteModule==NULL)? InvalidRemoteModuleId: remoteModule->getUniqueId();
		DROP_IF(senderUniqueId==InvalidRemoteModuleId && msg.getSenderId()!=InvalidRemoteModuleId,"Ignoring message from unknown sender",return);
		msg.setSenderId(senderUniqueId);

		// display a quick debug msg
		nldebug("GUSNET: Received broadcast message: %s  For all modules",msg.getMessageName().c_str());

		for (uint32 i=0;i<modules.size();++i)
		{
			nldebug("GUSNET: Calling module callback: Name: '%s'  Sender: %d  Destination: %d", msg.getMessageName().c_str(), msg.getSenderId(), CModuleManager::getInstance()->getModuleId(modules[i]));
			modules[i]->receiveModuleMessage(msg);
		}
	}

	void CConnectionModule::treatModuleMessage(NLNET::CMessage &msgin)
	{
		TModuleMessagePtr msg= new CModuleMessage;
		msgin.serial(*msg);

		// remap the sender id from the local id on the emitter to the uniqueId of the remote module here
		TRemoteModuleViaConnectionPtr remoteModule= getRemoteModuleByRemoteId(msg->getSenderId());
		uint32 senderUniqueId= (remoteModule==NULL)? InvalidRemoteModuleId: remoteModule->getUniqueId();
		DROP_IF(senderUniqueId==InvalidRemoteModuleId && msg->getSenderId()!=InvalidRemoteModuleId,"Ignoring message from unknown sender",return);
		msg->setSenderId(senderUniqueId);

		// display a quick debug msg
		CSString s;
		for (uint32 i=0;i<msg->getDestinationModuleIds().size();++i) s+=NLMISC::toString("%d ",msg->getDestinationModuleIds()[i]);
		nldebug("GUSNET: Received module message: %s  For modules: %s",msg->getMessageName().c_str(),s.c_str());

		// run through the list of destinations for the message, forwarding on to the modules as they're identified
		for (uint32 i=0;i<msg->getDestinationModuleIds().size();++i)
		{
			IModule* module= CModuleManager::getInstance()->lookupModuleById(*(msg->getDestinationModuleIds().begin()+i));
			if (module==NULL)
			{
				nlwarning("- Failed to identify destination: Name: '%s'  SEnder: %d  Destination: %d", msg->getMessageName().c_str(), msg->getSenderId(), msg->getDestinationModuleIds()[i]);
			}
			else
			{
				nldebug("GUSNET: Calling module callback: Name: '%s'  Sender: %d  Destination: %d", msg->getMessageName().c_str(), msg->getSenderId(), msg->getDestinationModuleIds()[i]);
				module->receiveModuleMessage(*msg);
			}
		}
	}

	const NLMISC::CSString& CConnectionModule::getConnectionAddress() const
	{
		return _ConnectionAddress;
	}

	CCallbackClient* CConnectionModule::getCbClient() const
	{
		return _CbClient;
	}

	CRemoteModuleViaConnection* CConnectionModule::getRemoteModuleByRemoteId(uint32 remoteId) const
	{
		for (TRemoteModules::const_iterator it= _RemoteModules.begin();it!=_RemoteModules.end();++it)
		{
			if ((*it).second->getRemoteId()== remoteId)
				return (*it).second;
		}
		STOP(NLMISC::toString("Failed to find remote module: %d",remoteId));
		return NULL;
	}

	void CConnectionModule::registerModule(GUS::TModulePtr module)
	{
		// if there's no client connection then fallout
		if (!_CbClient->connected() || !_IsConnected)
			return;

		// dispatch the module registration message across the connection
		nldebug("GUSNET: Sending module registration message to: %s  For module: %s %s",_ConnectionAddress.c_str(),module->getName().c_str(),module->getParameters().c_str());
		CMsgRegisterModule msg;
		msg.setup(CModuleManager::getInstance()->getModuleId(module),module->getName(),module->getParameters());
		CMessage outmsg(REGISTER_MODULE_TO_HUB);
		outmsg.serial(msg);
		_CbClient->send(outmsg);

		// register remote modules with the new module
		for (TRemoteModules::iterator it=_RemoteModules.begin(); it!=_RemoteModules.end(); ++it)
		{
			module->moduleUp(it->second);
		}
	}

	void CConnectionModule::unregisterModule(GUS::TModulePtr module)
	{
		if (!_CbClient->connected() || !_IsConnected)
			return;
		nldebug("GUSNET: Sending module unregistration message to: %s  For module: %s %s",_ConnectionAddress.c_str(),module->getName().c_str(),module->getParameters().c_str());
		CMsgUnregisterModule msg;
		msg.setup(CModuleManager::getInstance()->getModuleId(module));
		CMessage outmsg(UNREGISTER_MODULE_TO_HUB);
		outmsg.serial(msg);
		_CbClient->send(outmsg);
	}

	void CConnectionModule::registerRemoteModule(TRemoteModuleViaConnectionPtr module)
	{
		// make sure the module isn't already registered
		nlassert(_RemoteModules.find(module->getUniqueId())==_RemoteModules.end());

		// add the module to the _RemoteModules map
		_RemoteModules[module->getUniqueId()]= module;

		// register the module with the net singleton
		CGusNetImplementation::getInstance()->registerRemoteModule(module);

		// inform the other modules in the service of the new connection
		CModuleManager::TModuleVector localModules;
		CModuleManager::getInstance()->getModules(localModules);
		for (CModuleManager::TModuleVector::iterator it=localModules.begin();it!=localModules.end();++it)
		{
			(*it)->moduleUp(module);
		}
	}

	void CConnectionModule::unregisterRemoteModule(TRemoteModuleViaConnectionPtr module)
	{
		// make sure the module exists
		BOMB_IF(_RemoteModules.find(module->getUniqueId())==_RemoteModules.end(),"Unable to find requested module",return);

		// inform the other modules in the service of the dead connection
		CModuleManager::TModuleVector localModules;
		CModuleManager::getInstance()->getModules(localModules);
		for (CModuleManager::TModuleVector::iterator it=localModules.begin();it!=localModules.end();++it)
		{
			(*it)->moduleDown(module);
		}

		// unregister the module with the net singleton
		CGusNetImplementation::getInstance()->unregisterRemoteModule(module);

		// remove the module from our container
		_RemoteModules.erase(module->getUniqueId());
	}

	TRemoteModuleViaConnectionPtr CConnectionModule::lookupRemoteModule(uint32 uniqueId)
	{
		BOMB_IF(_RemoteModules.find(uniqueId)==_RemoteModules.end(),"Unable to find requested module",return NULL);
		return _RemoteModules[uniqueId];
	}

	TRemoteModuleViaConnectionPtr CConnectionModule::getRemoteModuleByRemoteId(uint32 remoteId)
	{
		for (TRemoteModules::iterator it=_RemoteModules.begin(); it!=_RemoteModules.end(); ++it)
		{
			if ((*it).second->getRemoteId()==remoteId)
				return (*it).second;
		}
		nlwarning("Unable to find module with remote Id: %d",remoteId);
		return NULL;
	}

	bool CConnectionModule::isRemoteModuleConnected(const IRemoteModule* module) const
	{
		const CRemoteModuleViaConnection* remoteModule= dynamic_cast<const CRemoteModuleViaConnection*>(module);
		return (remoteModule!=NULL) && (remoteModule->getConnection()==this);
	}

	//-----------------------------------------------------------------------------
	// CConnectionModule registration
	//-----------------------------------------------------------------------------

	REGISTER_GUS_MODULE(CConnectionModule,"GUSNET","<hub address:port>","A network connection")
}

