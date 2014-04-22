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
#include "gus_net_hub.h"
#include "gus_module_manager.h"
#include "gus_net_implementation.h"
#include "gus_net_messages.h"
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
	// callbacks for network messages
	//-----------------------------------------------------------------------------

	// message sent by connections when they connect
	static void cbRegisterModuleToHub(CMessage &msgin, TSockId tsid, CCallbackNetBase &netbase)
	{
		// identify the hub that this message is being sent to
		THubModulePtr hub= CGusNetImplementation::getInstance()->getHub(netbase);
		BOMB_IF(hub==NULL,"Hub not found in cbRegisterModuleToHub",return);

		// decode the NLNET message
		CMsgRegisterModule msg;
		msgin.serial(msg);
		nldebug("GUSHUB: Received module connection: %d: %s",msg.getModuleId(),(msg.getModuleName()+" "+msg.getParameters()).c_str());

		// instantiate and initialise a new module object
		TRemoteModuleOnHubPtr theModule= new CRemoteModuleOnHub;
		bool isOK= theModule->init(hub,tsid,msg);
		DROP_IF(!isOK,"Failed to initialise new remote module on hub",return);

		// add the remote module to the hub that it's connected to
		hub->registerRemoteModule(theModule);
	}

	// message sent by connections when their modules disconnect
	static void cbUnregisterModuleToHub(CMessage &msgin, TSockId tsid, CCallbackNetBase &netbase)
	{
		// identify the hub that this message is being sent to
		THubModulePtr hub= CGusNetImplementation::getInstance()->getHub(netbase);
		BOMB_IF(hub==NULL,"Hub not found in cbUnregisterModuleToHub",return);

		// decode the NLNET message
		CMsgUnregisterModule msg;
		msgin.serial(msg);
		nldebug("GUSHUB: Received module disconnection: %d",msg.getModuleId());

		// make sure the module exisst
		IRemoteModule* modulePtr= hub->getRemoteModuleByRemoteId(tsid,msg.getModuleId());
		if (modulePtr==NULL)
			return;

		// remove the remote module from the hub that it's connected to
		hub->unregisterRemoteModule(safe_cast<GUSNET::CRemoteModuleOnHub*>(modulePtr));
	}

	// a message to be forwarded to a specific set of desitnations
	static void cbModuleMessage(CMessage &msgin, TSockId tsid, CCallbackNetBase &netbase)
	{
		// identify the hub that this message is being sent to
		THubModulePtr hub= CGusNetImplementation::getInstance()->getHub(netbase);
		BOMB_IF(hub==NULL,"Hub not found in cbModuleMessage",return);

		hub->forwardModuleMessage(msgin,tsid);
	}

	// a broadcast message intended to go to the whole hub
	static void cbBroadcastMessage(CMessage &msgin, TSockId tsid, CCallbackNetBase &netbase)
	{
		// identify the hub that this message is being sent to
		THubModulePtr hub= CGusNetImplementation::getInstance()->getHub(netbase);
		BOMB_IF(hub==NULL,"Hub not found in cbBroadcast",return);

		hub->forwardBroadcastMessage(msgin,tsid);
	}

	static void cbHubConnectionUp(TSockId tsid,void* hubModulePtr)
	{
		nldebug("GUSHUB: Connection Up: %s",tsid->asString().c_str());
		((CHubModule*)(hubModulePtr))->treatNewConnection(tsid);
	}

	static void cbHubConnectionDown(TSockId tsid,void* hubModulePtr)
	{
		nldebug("GUSHUB: Connection Down: %s",tsid->asString().c_str());
		((CHubModule*)(hubModulePtr))->unregisterAllRemoteModules(tsid);
	}


	//-----------------------------------------------------------------------------
	// methods CHubModule
	//-----------------------------------------------------------------------------

	CHubModule::CHubModule()
	{
		_Port=0;
		_CbServer= NULL;
	}

	bool CHubModule::initialiseModule(const NLMISC::CSString& rawArgs)
	{
		static NLNET::TCallbackItem cbArray[] = 
		{
			{	REGISTER_MODULE_TO_HUB,		cbRegisterModuleToHub,	},
			{	UNREGISTER_MODULE_TO_HUB,	cbUnregisterModuleToHub,	},

			{	MODULE_MSG,					cbModuleMessage,		},
			{	BROADCAST_MSG,				cbBroadcastMessage,		},
		};

		// extract the port number
		_Port= (uint16)rawArgs.atoi();
		if (NLMISC::toString(_Port)!=rawArgs)
			return false;

		// setup the network connection
		_CbServer = new CCallbackServer;
		try
		{
			_CbServer->init(_Port);
		}
		catch(...)
		{
			nlwarning("Failed to initialise connection to port: %d",_Port);
			delete _CbServer;
			return false;
		}
		_CbServer->addCallbackArray(cbArray, sizeof(cbArray)/sizeof(cbArray[0]));
		_CbServer->setConnectionCallback(cbHubConnectionUp, this);
		_CbServer->setDisconnectionCallback(cbHubConnectionDown, this);

		// now that we are initialised - add us to the hub vector
		CGusNetImplementation::getInstance()->addHubModule(this);

		// success so return true
		return true;
	}

	void CHubModule::release()
	{
		// make sure we hang on to a smart pointer to avoid surprise delete in mid processing
		TModulePtr self=this;

		// remove the hub from the net singleton's hub list
		CGusNetImplementation::getInstance()->removeHubModule(this);

		// disconnect all clients
		if(_CbServer)
		{
			_CbServer->disconnect(InvalidSockId);
			delete _CbServer;
			_CbServer = 0;
		}
	}

	void CHubModule::serviceUpdate(NLMISC::TTime localTime)
	{
		// update all server connections
		if (_CbServer)
		{
			_CbServer->update(0);
		}
	}

	NLMISC::CSString CHubModule::getState() const
	{
		return NLMISC::toString("GUSHUB %d:  Name: %s  RemoteModules: %d",_Port,_HubName.c_str(),_RemoteModules.size());
	}

	NLMISC::CSString CHubModule::getName() const
	{
		return "GUSHUB";
	}

	NLMISC::CSString CHubModule::getParameters() const
	{
		return NLMISC::toString("%d",_Port);
	}

	void CHubModule::displayModule() const
	{
		// title
		InfoLog->displayNL("GUSHUB Connection: %s (port: %d)",_HubName.c_str(),_Port);

		// remote modules
		for (TRemoteModules::const_iterator it=_RemoteModules.begin(); it!=_RemoteModules.end(); ++it )
		{
			InfoLog->displayNL("- Remote Module: %s: %4d:  %s %s",(*it).second->getSockId()->asString().c_str(),(*it).first,(*it).second->getName().c_str(),(*it).second->getParameters().c_str());
		}

		// nel net stats
//		InfoLog->displayNL("");
//		InfoLog->displayNL("ReceiveQueueStat");
//		_CbServer->displayReceiveQueueStat();
  
//		InfoLog->displayNL("");
//		InfoLog->displayNL("SendQueueStat");
//		_CbServer->displaySendQueueStat();
		
//		InfoLog->displayNL("");
//		InfoLog->displayNL("ThreadStat");
//		_CbServer->displayThreadStat();
	}

	void CHubModule::registerRemoteModule(TRemoteModuleOnHubPtr module)
	{
		// make sure the module isn't already registered
		for (TRemoteModules::iterator it=_RemoteModules.begin();it!=_RemoteModules.end();++it)
			BOMB_IF((*it).second==module,"Attempt to add the same module to a hub more than once",return)

		// add the module to the _RemoteModules map
		_RemoteModules[module->getUniqueId()]=module;

		// register the module with the net singleton
		CGusNetImplementation::getInstance()->registerRemoteModule(module);

		// broadcast the existance of the module to our connections
		nlinfo("broadcasting the existence of the new module to our connections...");
		CMsgRegisterModule msg;
		msg.setup(module->getUniqueId(),module->getName(),module->getParameters());
		CMessage outmsg(REGISTER_MODULE_FROM_HUB);
		outmsg.serial(msg);
		_CbServer->send(outmsg,InvalidSockId);
	}

	void CHubModule::unregisterRemoteModule(TRemoteModuleOnHubPtr module)
	{
		// make sure the module exists
		BOMB_IF(_RemoteModules.find(module->getUniqueId())==_RemoteModules.end(),"Failed to unregister unrecognised module",return);

		// broadcast the inexistance of the module to our connections
		CMsgUnregisterModule msg;
		msg.setup(module->getUniqueId());
		CMessage outmsg(UNREGISTER_MODULE_FROM_HUB);
		outmsg.serial(msg);
		_CbServer->send(outmsg,InvalidSockId);

		// unregister the module with the net singleton
		CGusNetImplementation::getInstance()->unregisterRemoteModule(module);

		// remove the map entry refferencing the module
		_RemoteModules.erase(module->getUniqueId());
	}

	void CHubModule::unregisterAllRemoteModules(TSockId tsid)
	{
		// for each module in the dead service
		TRemoteModules::iterator nextIt;
		nextIt= _RemoteModules.begin();
		while (nextIt!=_RemoteModules.end())
		{
			// make sure we can delete the dead entries as we go...
			TRemoteModules::iterator it= nextIt;
			++nextIt;

			// we're only interested in the modules that belong to the dead service
			if ((*it).second->getSockId()==tsid)
			{
				// tell the connected services to remove the module
				unregisterRemoteModule((*it).second);
			}
		}
	}

	void CHubModule::treatNewConnection(NLNET::TSockId tsid)
	{
		nldebug("GUSHUB: Sending module registration message to new connection");
		for (TRemoteModules::const_iterator it=_RemoteModules.begin();it!=_RemoteModules.end();++it)
		{
			CMsgRegisterModule msg;
			(*it).second->buildDescriptionMsg(msg);
			CMessage outmsg(REGISTER_MODULE_FROM_HUB);
			outmsg.serial(msg);
			_CbServer->send(outmsg,tsid);
		}
	}

	void CHubModule::forwardBroadcastMessage(CMessage &msgin,NLNET::TSockId tsid)
	{
		// decode the NLNET message
		CModuleMessage toHubMsg;
		msgin.serial(toHubMsg);

		// remap the sender id from the local id on the emitter to the uniqueId of the remote module here
		TRemoteModuleOnHubPtr remoteModule= getRemoteModuleByRemoteId(tsid,toHubMsg.getSenderId());
		uint32 senderUniqueId= (remoteModule==NULL)? InvalidRemoteModuleId: remoteModule->getUniqueId();

		// do the broadcast...
		broadcastMessage(toHubMsg.getMessageName(),toHubMsg.getMsgBody(),senderUniqueId);
	}

	void CHubModule::forwardModuleMessage(NLNET::CMessage &msgin,NLNET::TSockId tsid)
	{
		// decode the NLNET message
		CModuleMessage toHubMsg;
		msgin.serial(toHubMsg);

		// remap the sender id from the local id on the emitter to the uniqueId of the remote module here
		TRemoteModuleOnHubPtr remoteModule= getRemoteModuleByRemoteId(tsid,toHubMsg.getSenderId());
		uint32 senderUniqueId= (remoteModule==NULL)? InvalidRemoteModuleId: remoteModule->getUniqueId();

		// sort the destination modules into buckets by destination
		typedef std::map<NLNET::TSockId,TModuleIdVector> TDestinations;
		TDestinations destinations;
		for (uint32 i=0;i<toHubMsg.getDestinationModuleIds().size();++i)
		{
			// lookup the remote module record for the supplied id
			TRemoteModules::iterator remoteModuleIt= _RemoteModules.find(toHubMsg.getDestinationModuleIds()[i]);
			if (remoteModuleIt==_RemoteModules.end())
			{
				nlwarning("GUSHUB: Ignoring request to send a message to unknown destination: %d  for message: %s",
					toHubMsg.getDestinationModuleIds()[i],toHubMsg.getMessageName().c_str());
				continue;
			}
			// add the entry corresponding to the remote module record to our buckets
			destinations[(*remoteModuleIt).second->getSockId()].push_back((*remoteModuleIt).second->getRemoteId());
		}

		// run through the destinations sending the message
		for (TDestinations::iterator it=destinations.begin();it!=destinations.end();++it)
		{
			sendMessage(toHubMsg.getMessageName(),toHubMsg.getMsgBody(),(*it).first,(*it).second,senderUniqueId);
		}
	}

	void CHubModule::sendMessage(const CSString& msgName, const TRawMsgBodyPtr& msgBody, NLNET::TSockId sockId, const TModuleIdVector& remoteIds,uint32 senderModuleId)
	{
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
		_CbServer->send(msgOut,sockId);
	}

	void CHubModule::broadcastMessage(const CSString& msgName, const TRawMsgBodyPtr& msgBody, uint32 senderModuleId)
	{
		// setup a CModuleMessage
		CModuleMessage msg;
		msg.setMessageName(msgName);
		msg.setMsgBody(msgBody);
		msg.setSenderId(senderModuleId);

		// serialise the CModuleMessage to a CMessage
		CMessage msgOut(BROADCAST_MSG);
		msgOut.serial(msg);

		// do the send
		_CbServer->send(msgOut,NLNET::InvalidSockId);
	}

	const NLMISC::CSString& CHubModule::getHubName() const
	{
		return _HubName;
	}

	uint16 CHubModule::getPort() const
	{
		return _Port;
	}

	CCallbackServer* CHubModule::getCbServer() const
	{
		return _CbServer;
	}

	CRemoteModuleOnHub* CHubModule::getRemoteModuleByRemoteId(NLNET::TSockId tsid,uint32 remoteId) const
	{
		for (TRemoteModules::const_iterator it= _RemoteModules.begin();it!=_RemoteModules.end();++it)
		{
			if ((*it).second->getRemoteId()!= remoteId)
				continue;

			if ((*it).second->getSockId()!= tsid)
				continue;

			return (*it).second;
		}
		STOP(NLMISC::toString("Failed to find remote module: %s / %d",tsid->asString().c_str(),remoteId));
		return NULL;
	}

	//-----------------------------------------------------------------------------
	// CHubModule registration
	//-----------------------------------------------------------------------------

	REGISTER_GUS_MODULE(CHubModule,"GUSHUB","<port>","A network hub module")
}

