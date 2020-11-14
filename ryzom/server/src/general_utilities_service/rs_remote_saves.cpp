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
#include "gus_net.h"
#include "gus_net_remote_module.h"

#include "rs_remote_saves.h"
#include "remote_saves_interface.h"
#include "saves_module_messages.h"
#include "rs_module_messages.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace GUS;
using namespace GUSNET;


//-----------------------------------------------------------------------------
// SAVES namespace
//-----------------------------------------------------------------------------

namespace SAVES
{
	//-----------------------------------------------------------------------------
	// class CRemoteSavesConnectionModule
	//-----------------------------------------------------------------------------

	class CRemoteSavesConnectionModule: public IRemoteSavesConnection
	{
	public:
		// IRemoteSavesConnection specialisation implementation
		const CFileDescriptionContainer& getFileList() const;
		uint32 requestFile(const NLMISC::CSString& fileName,CRemoteSavesInterface* requestor);
		uint32 uploadFile(const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody,CRemoteSavesInterface* requestor);
		uint32 deleteFile(const NLMISC::CSString& fileName,CRemoteSavesInterface* requestor);
		uint32 moveFile(const NLMISC::CSString& fileName,const NLMISC::CSString& destination,CRemoteSavesInterface* requestor);
		bool isConnected() const;

	public:
		// IModule specialisation implementation
		bool initialiseModule(const NLMISC::CSString& rawArgs);
		void release();
		void receiveModuleMessage(GUSNET::CModuleMessage& msg);
		NLMISC::CSString getState() const;
		NLMISC::CSString getName() const;
		NLMISC::CSString getParameters() const;
		void displayModule() const;
		void moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule);
		void moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule);

	public:
		// remaining public interface
		const NLMISC::CSString& getShardName() const;
		const NLMISC::CSString& getType() const;

		void connect(const TRemoteSavesInterfacePtr& si);
		void disconnect(CRemoteSavesInterface* si);

	private:
		// private methods used for module message processing
		void processMsgInit			(uint32 sender,CMsgRSInit&			msgBody);
		void processMsgUpdate		(uint32 sender,CMsgRSUpdate&		msgBody);
		void processMsgDownload		(uint32 sender,CMsgRSDownload&		msgBody);
		void processMsgGenericReply	(uint32 sender,CMsgRSGenericReply&	msgBody);

	private:
		// private data
		NLMISC::CSString			_ShardName;
		NLMISC::CSString			_Type;
		CFileDescriptionContainer	_FileList;
		uint32						_SlaveModuleId;
		uint32						_NextRequestId;

		typedef vector<TRemoteSavesInterfacePtr> TInterfaces;
		TInterfaces					_Interfaces;

		// vector of pending request records...
		struct CRequestRecord
		{
			CRequestRecord(uint32 requestId=0,const CSString& description=CSString(),const CRefPtr<CRemoteSavesInterface>& requestor=NULL)
			{
				RequestId=		requestId;
				Description=	description;
				Requestor=		requestor;
			}

			uint32 RequestId;
			CSString Description;
			CRefPtr<CRemoteSavesInterface> Requestor;
		};
		std::vector<CRequestRecord> _Requests;
	};
	typedef NLMISC::CSmartPtr<CRemoteSavesConnectionModule> TRemoteSavesConnectionModulePtr;


	//-----------------------------------------------------------------------------
	// methods CRemoteSavesConnectionModule - IRemoteSavesConnection API
	//-----------------------------------------------------------------------------

	const CFileDescriptionContainer& CRemoteSavesConnectionModule::getFileList() const
	{
		return _FileList;
	}

	uint32 CRemoteSavesConnectionModule::requestFile(const NLMISC::CSString& fileName,CRemoteSavesInterface* requestor)
	{
		DROP_IF(!isConnected(),"Failed to send request for file because SAVES module not currently connected: "+fileName,return ~0u);

		// assign a new unique id to the request
		if (_NextRequestId==~0u) ++_NextRequestId;
		uint32 requestId= _NextRequestId++;

		// create a 'request' object to represent the request that we're sending
		_Requests.push_back(CRequestRecord(requestId,"requestFile "+fileName,requestor));

		// send the request to the executor
		CMsgSavesFileRequest msg(requestId,fileName);
		sendModuleMessage(msg,_SlaveModuleId,this);

		return requestId;
	}

	uint32 CRemoteSavesConnectionModule::uploadFile(const NLMISC::CSString& fileName,const NLMISC::CSString& fileBody,CRemoteSavesInterface* requestor)
	{
		// assign a new unique id to the request
		if (_NextRequestId==~0u) ++_NextRequestId;
		uint32 requestId= _NextRequestId++;

		// create a 'request' object to represent the request that we're sending
		_Requests.push_back(CRequestRecord(requestId,"uploadFile "+fileName,requestor));

		// send the request to the executor
		CMsgSavesUpload msg(requestId,fileName,fileBody);
		sendModuleMessage(msg,_SlaveModuleId,this);

		return requestId;
	}

	uint32 CRemoteSavesConnectionModule::deleteFile(const NLMISC::CSString& fileName,CRemoteSavesInterface* requestor)
	{
		// assign a new unique id to the request
		if (_NextRequestId==~0u) ++_NextRequestId;
		uint32 requestId= _NextRequestId++;

		// create a 'request' object to represent the request that we're sending
		_Requests.push_back(CRequestRecord(requestId,"deleteFile "+fileName,requestor));

		// send the request to the executor
		CMsgSavesDelete msg(requestId,fileName);
		sendModuleMessage(msg,_SlaveModuleId,this);

		return requestId;
	}

	uint32 CRemoteSavesConnectionModule::moveFile(const NLMISC::CSString& fileName,const NLMISC::CSString& destination,CRemoteSavesInterface* requestor)
	{
		// assign a new unique id to the request
		if (_NextRequestId==~0u) ++_NextRequestId;
		uint32 requestId= _NextRequestId++;

		// create a 'request' object to represent the request that we're sending
		_Requests.push_back(CRequestRecord(requestId,"moveFile "+fileName,requestor));

		// send the request to the executor
		CMsgSavesMove msg(requestId,fileName,destination);
		sendModuleMessage(msg,_SlaveModuleId,this);

		return requestId;
	}


	//-----------------------------------------------------------------------------
	// methods CRemoteSavesConnectionModule - remaining public interface
	//-----------------------------------------------------------------------------

	const NLMISC::CSString& CRemoteSavesConnectionModule::getShardName() const
	{
		return _ShardName;
	}

	const NLMISC::CSString& CRemoteSavesConnectionModule::getType() const
	{
		return _Type;
	}

	bool CRemoteSavesConnectionModule::isConnected() const
	{
		return (_SlaveModuleId!=~0u);
	}

	void CRemoteSavesConnectionModule::connect(const TRemoteSavesInterfacePtr& si)
	{
		BOMB_IF(si==NULL,"BUG: Call to connect() with NULL pointer",return);
		BOMB_IF(getShardName()!=si->getShardName() || getType()!=si->getType(),"BUG: Interface and RS module are not for the same shard / type",return);
		for (uint32 i=0;i<_Interfaces.size();++i)
		{
			BOMB_IF(_Interfaces[i]==si,"BUG: Interface already connected to RS module",return);
		}
		_Interfaces.push_back(si);

		// if we're already connected then call any callbacks registered with the new interface
		if (isConnected())
		{
			CRemoteSavesInterface::TCallbackSet callbacks;
			si->getCallbacks(callbacks);
			for (uint32 j=0;j<callbacks.size();++j)
			{
				callbacks[j]->cbInit(_FileList);
			}
		}
	}

	void CRemoteSavesConnectionModule::disconnect(CRemoteSavesInterface* si)
	{
		BOMB_IF(si==NULL,"BUG: Call to disconnect() with NULL pointer",return);
		BOMB_IF(getShardName()!=si->getShardName() || getType()!=si->getType(),"BUG: Interface and RS module are not for the same shard / type",return);
		for (uint32 i=_Interfaces.size();i--;)
		{
			if(_Interfaces[i]==si)
			{
				if (i<_Interfaces.size()-1)
					_Interfaces[i]=_Interfaces.back();
				_Interfaces.pop_back();
			}
		}
	}

	void CRemoteSavesConnectionModule::processMsgInit(uint32 sender,CMsgRSInit& msgBody)
	{
		BOMB_IF(!_FileList.empty(),"Received a second init message for the same SAVES module - this is not legal!",return);

		_FileList= msgBody.getFileList();
		_SlaveModuleId= sender;
		nlinfo("SAVES: %s: Processing Init message (%d files) treating them now...",_ShardName.c_str(),_FileList.size());

		// a quick bit of debugging...
		_FileList.display(NLMISC::DebugLog);

		// call the callbacks...
		for (uint32 i=0;i<_Interfaces.size();++i)
		{
			CRemoteSavesInterface::TCallbackSet callbacks;
			_Interfaces[i]->getCallbacks(callbacks);
			for (uint32 j=0;j<callbacks.size();++j)
			{
				callbacks[j]->cbInit(_FileList);
			}
		}
	}

	void CRemoteSavesConnectionModule::processMsgUpdate(uint32 sender,CMsgRSUpdate& msgBody)
	{
		BOMB_IF(_SlaveModuleId!=sender,"Received RS update message from unexpected sender - ignoring it",return);

		// check whether any changes were found...
		if (msgBody.empty())
			return;

		// setup a few local shortcuts to message contents
		const CFileDescriptionContainer&	newFiles=		msgBody.getNewFiles();
		const CFileDescriptionContainer&	changedFiles=	msgBody.getChangedFiles();
		const CVectorSString&				deletedFiles=	msgBody.getDeletedFiles();
		nlinfo("Changes in remote file lists (%d added, %d changed, %d deleted) treating them now...",newFiles.size(),changedFiles.size(),deletedFiles.size());

		// build a set of deleted files
		set<CSString> deletedFileSet;
		for (uint32 i=deletedFiles.size();i--;)
		{
			deletedFileSet.insert(deletedFiles[i]);
			nldebug("SAVES: %s: deleting: %s",_ShardName.c_str(),deletedFiles[i].c_str());
		}

		// build a map of changes
		map<CSString,const CFileDescription*> changedFileMap;
		for (uint32 i=changedFiles.size();i--;)
		{
			changedFileMap[changedFiles[i].FileName]= &changedFiles[i];
		}

		// take a copy of our file list for later use
		CFileDescriptionContainer oldFileList= _FileList;

		// run through our local file list appying changes
		for (uint32 i=_FileList.size();i--;)
		{
			// see if this file has been deleted
			if (deletedFileSet.find(_FileList[i].FileName)!=deletedFileSet.end())
			{
				_FileList.removeFile(i);
				continue;
			}

			// see if there's a change record for the file
			if (changedFileMap.find(_FileList[i].FileName)!=changedFileMap.end())
			{
				_FileList[i]= *changedFileMap.find(_FileList[i].FileName)->second;
				nldebug("SAVES: %s: updating: %s",_ShardName.c_str(),_FileList[i].FileName.c_str());
			}
		}

		// add new files
		for (uint32 i=newFiles.size();i--;)
		{
			_FileList.addFile(newFiles[i].FileName,newFiles[i].FileTimeStamp,newFiles[i].FileSize);
			nldebug("SAVES: %s: adding: %s",_ShardName.c_str(),newFiles[i].FileName.c_str());
		}

		// call the callbacks...
		for (uint32 i=0;i<_Interfaces.size();++i)
		{
			CRemoteSavesInterface::TCallbackSet callbacks;
			_Interfaces[i]->getCallbacks(callbacks);
			for (uint32 j=0;j<callbacks.size();++j)
			{
				callbacks[j]->cbFileListChanged(newFiles,changedFiles,deletedFiles,oldFileList,_FileList);
			}
		}
	}

	void CRemoteSavesConnectionModule::processMsgDownload(uint32 sender,CMsgRSDownload& msgBody)
	{
		nlinfo("Saves connection receiving download for %s from %d (id %d)",msgBody.getFileName().c_str(),sender,msgBody.getRequestId());

		// lookup to see who the file was requested by...
		BOMB_IF (_Requests.empty(),"Received file download from SAVES module but can't find matching request",return);
		BOMB_IF (_Requests.front().RequestId!=msgBody.getRequestId(),"Received file download from SAVES module but can't find matching request",return);
		CRequestRecord request=_Requests.front();
		_Requests.erase(_Requests.begin());

		// call the callback...
		if (request.Requestor!=NULL)
		{
			CRemoteSavesInterface::TCallbackSet callbacks;
			request.Requestor->getCallbacks(callbacks);
			for (uint32 j=0;j<callbacks.size();++j)
			{
				callbacks[j]->cbFileReceived(msgBody.getRequestId(),msgBody.getFileName(),msgBody.getFileBody());
			}
		}
	}

	void CRemoteSavesConnectionModule::processMsgGenericReply(uint32 sender,CMsgRSGenericReply& msgBody)
	{
		// lookup to see who the file was requested by...
		BOMB_IF (_Requests.front().RequestId!=msgBody.getRequestId(),"Received a reply msg from SAVES module but can't find matching request",return);
		CRequestRecord request=_Requests[msgBody.getRequestId()];
		_Requests.erase(_Requests.begin());

		// call the callback...
		if (request.Requestor!=NULL)
		{
			CRemoteSavesInterface::TCallbackSet callbacks;
			request.Requestor->getCallbacks(callbacks);
			for (uint32 j=0;j<callbacks.size();++j)
			{
				callbacks[j]->cbGenericReply(msgBody.getRequestId(),msgBody.getSuccesFlag(),msgBody.getExplanation());
			}
		}
	}


	//-----------------------------------------------------------------------------
	// methods CRemoteSavesConnectionModule - IModule API
	//-----------------------------------------------------------------------------

	bool CRemoteSavesConnectionModule::initialiseModule(const NLMISC::CSString& rawArgs)
	{
		// make sure the command line syntax looks ok
		DROP_IF(rawArgs.countWords()!=2,"syntax: modulesAdd RS <shard_name> shard|www",return false);
	   
		// setup private properties
		_ShardName=		rawArgs.word(0);
		_Type=			rawArgs.word(1);
		_SlaveModuleId= ~0u;
		_NextRequestId= 0;

		// make sure there isn't another RS module instantiated for the same shard / type
		CModuleManager::TModuleVector modules;
		GUS::CModuleManager::getInstance()->getModules(modules);
		for (uint32 i=0;i<modules.size();++i)
		{
			if (modules[i]==this)
				continue;

			DROP_IF((modules[i]->getName()==getName()) && (modules[i]->getParameters()==getParameters()),
				"There is already an RS module instatiated for: "+getParameters(),return false);
		}

		// register ourselves with the remote saves singleton
		CRemoteSavesManager::getInstance()->registerRemoteSavesConnectionModule(this);

		return true;
	}

	void CRemoteSavesConnectionModule::release()
	{
		CRemoteSavesManager::getInstance()->unregisterRemoteSavesConnectionModule(this);
	}

	void CRemoteSavesConnectionModule::receiveModuleMessage(GUSNET::CModuleMessage& msg)
	{
		nlinfo("Saves connection (%s %s) receiving message of type: %s from sender %d",
			getParameters().word(0).c_str(),getParameters().word(1).c_str(),
			msg.getMessageName().c_str(),msg.getSenderId());

		if (msg.getMessageName()==CMsgRSInit().getName())
		{
			CMsgRSInit msgBody(msg.getMsgBody());
			processMsgInit(msg.getSenderId(),msgBody);
		}
		else if (msg.getMessageName()==CMsgRSUpdate().getName())
		{
			CMsgRSUpdate msgBody(msg.getMsgBody());
			processMsgUpdate(msg.getSenderId(),msgBody);
		}
		else if (msg.getMessageName()==CMsgRSDownload().getName())
		{
			CMsgRSDownload msgBody(msg.getMsgBody());
			processMsgDownload(msg.getSenderId(),msgBody);
		}
		else if (msg.getMessageName()==CMsgRSGenericReply().getName())
		{
			CMsgRSGenericReply msgBody(msg.getMsgBody());
			processMsgGenericReply(msg.getSenderId(),msgBody);
		}
	}

	NLMISC::CSString CRemoteSavesConnectionModule::getState() const
	{
		return getName()+" "+getParameters();
	}

	NLMISC::CSString CRemoteSavesConnectionModule::getName() const
	{
		return "RS";
	}

	NLMISC::CSString CRemoteSavesConnectionModule::getParameters() const
	{
		return _ShardName+" "+_Type;
	}

	void CRemoteSavesConnectionModule::displayModule() const
	{
		NLMISC::InfoLog->displayNL("Module: %s %s",getName().c_str(),getParameters().c_str());
		NLMISC::InfoLog->displayNL("- Shard Name: %s",_ShardName.c_str());
		NLMISC::InfoLog->displayNL("- Type:       %s",_Type.c_str());
		NLMISC::InfoLog->displayNL("- Interfaces: %d",_Interfaces.size());

		if (_SlaveModuleId==~0u)
		{
			NLMISC::InfoLog->displayNL("- NOT CONNECTED");
			return;
		}

		NLMISC::InfoLog->displayNL("- Connection: %d",_SlaveModuleId);
		NLMISC::InfoLog->displayNL("- Requests:   %d",_Requests.size());
		NLMISC::InfoLog->displayNL("- NextReqId:  %d",_NextRequestId);
		NLMISC::InfoLog->displayNL("- NumFiles:   %d",_FileList.size());

		uint32 i=0;
		for (;i<50 && i<_Requests.size();++i)
		{
			NLMISC::InfoLog->displayNL("-> Request %5d: %s",_Requests[i].RequestId,_Requests[i].Description.c_str());
		}
		if (_Requests.size()>100)
		{
			NLMISC::InfoLog->displayNL("-> ... too many requests to list ...");
			i=_Requests.size()-50;
		}
		for (;i<_Requests.size();++i)
		{
			NLMISC::InfoLog->displayNL("-> Request %5d: %s",_Requests[i].RequestId,_Requests[i].Description.c_str());
		}
	}

	void CRemoteSavesConnectionModule::moduleUp(GUSNET::CRemoteModuleViaConnection* remoteModule)
	{
		// we're only interested in saves modules
		if (remoteModule->getName()=="SAVES")
		{
			// extract the shard name and type from the remote module's args
			NLMISC::CSString shardName=	remoteModule->getParameters().word(0);
			NLMISC::CSString type=		remoteModule->getParameters().word(1);

			// we're only interested in saves modules that match the shard and type that we're setup to connect to
			if (shardName!=this->getShardName() || type!=this->getType())
				return;

			// make sure we don't already have a connection established
			if (isConnected())
			{
				nlwarning("Breaking connection to remote saves module: %s %s (%d)  because new connection received from: %s",
					getShardName().c_str(),getType().c_str(),_SlaveModuleId,remoteModule->getInfoString().c_str());

				// mark the slave module as 'unknown' until we complete initialisation with the new module
				_SlaveModuleId= ~0u;
			}

			// display a quick info
			nlinfo("Received connection for RS module %s %s => requesting registration: %s",
					getShardName().c_str(),getType().c_str(),remoteModule->getInfoString().c_str());

			// send a registration message to the newly connected module
			CMsgSavesRegister registrationMsg;
			sendModuleMessage(registrationMsg,remoteModule->getUniqueId(),this);
		}
	}

	void CRemoteSavesConnectionModule::moduleDown(GUSNET::CRemoteModuleViaConnection* remoteModule)
	{
		// we're only interested in our own slave module
		if (remoteModule->getUniqueId()==_SlaveModuleId)
		{
			nlwarning("Connection lost to remote saves module: %s",remoteModule->getInfoString().c_str());

			// mark the slave module as 'unknown'
			_SlaveModuleId= ~0u;
		}
	}

	//-----------------------------------------------------------------------------
	// CRemoteSavesConnectionModule registration
	//-----------------------------------------------------------------------------

	REGISTER_GUS_MODULE(CRemoteSavesConnectionModule,"RS","<shard_name> shard|www","Remote save files proxy (operates with SAVES modules)")


	//-----------------------------------------------------------------------------
	// class CRemoteSavesManagerImplementation
	//-----------------------------------------------------------------------------

	class CRemoteSavesManagerImplementation: public CRemoteSavesManager
	{
	public:
		// singleton instance accessor
		static CRemoteSavesManagerImplementation* getInstance();

	public:
		// CRemoteSavesManager implementation
		void registerSavesInterface(TRemoteSavesInterfacePtr si);
		void unregisterSavesInterface(CRemoteSavesInterface* si);
		void registerRemoteSavesConnectionModule(TRemoteSavesConnectionPtr connection);
		void unregisterRemoteSavesConnectionModule(TRemoteSavesConnectionPtr connection);
		IRemoteSavesConnection* getConnection(const NLMISC::CSString& shardName,const NLMISC::CSString& type) const;

	private:
		// this is a singleton so prohibit instantiation
		CRemoteSavesManagerImplementation() {}

	private:
		// private data
		typedef vector<TRemoteSavesInterfacePtr> TInterfaces;
		TInterfaces					_Interfaces;

		typedef vector<TRemoteSavesConnectionModulePtr> TConnections;
		TConnections				_Connections;
	};


	//-----------------------------------------------------------------------------
	// methods CRemoteSavesManagerImplementation
	//-----------------------------------------------------------------------------

	CRemoteSavesManagerImplementation* CRemoteSavesManagerImplementation::getInstance()
	{
		static NLMISC::CSmartPtr<CRemoteSavesManagerImplementation> ptr=NULL;
		if (ptr==NULL)
		{
			ptr= new CRemoteSavesManagerImplementation;
		}
		return ptr;
	}

	void CRemoteSavesManagerImplementation::registerSavesInterface(TRemoteSavesInterfacePtr si)
	{
		BOMB_IF(si==NULL,"BUG: call to registerSavesInterface with NULL parameter",return);

		// make sure that this interface isn't already registered
		for (uint32 i=_Interfaces.size();i--;)
		{
			BOMB_IF(_Interfaces[i]==si,"BUG: Attempt to connect interface object failed because already connected",return);
		}
		_Interfaces.push_back(si);

		// linkup this interface to any connections that should be interested
		for (uint32 i=_Connections.size();i--;)
		{
			if (_Connections[i]->getShardName()==si->getShardName() && _Connections[i]->getType()==si->getType())
			{
				_Connections[i]->connect(si);
			}
		}
	}

	void CRemoteSavesManagerImplementation::unregisterSavesInterface(CRemoteSavesInterface* si)
	{
		BOMB_IF(si==NULL,"BUG: call to unregisterSavesInterface with NULL parameter",return);

		// remove from the _Interfaces vector
		for (uint32 i=_Interfaces.size();i--;)
		{
			if (_Interfaces[i]==si)
			{
				_Interfaces[i]=_Interfaces.back();
				_Interfaces.pop_back();
			}
		}

		// unlink this interface from any connections that are likely to be attached at the moment
		for (uint32 i=_Connections.size();i--;)
		{
			if (_Connections[i]->getShardName()==si->getShardName() && _Connections[i]->getType()==si->getType())
			{
				_Connections[i]->disconnect(si);
			}
		}
	}

	void CRemoteSavesManagerImplementation::registerRemoteSavesConnectionModule(TRemoteSavesConnectionPtr connection)
	{
		BOMB_IF(connection==NULL,"BUG: call to registerRemoteSavesConnectionModule with NULL parameter",return);

		// make sure that this connection isn't already registered
		for (uint32 i=_Connections.size();i--;)
		{
			BOMB_IF(_Connections[i]==connection,"BUG: Attempt to connect RS module failed because already connected",return);
		}
		_Connections.push_back(safe_cast<CRemoteSavesConnectionModule*>(&*connection));

		// linkup this connection to any interfaces that should be interested
		for (uint32 i=_Interfaces.size();i--;)
		{
			if (_Interfaces[i]->getShardName()==_Connections.back()->getShardName() &&
				_Interfaces[i]->getType()==_Connections.back()->getType())
			{
				_Connections.back()->connect(_Interfaces[i]);
			}
		}
	}

	void CRemoteSavesManagerImplementation::unregisterRemoteSavesConnectionModule(TRemoteSavesConnectionPtr connection)
	{
		BOMB_IF(connection==NULL,"BUG: call to unregisterRemoteSavesConnectionModule with NULL parameter",return);

		// remove from the _Connections vector
		for (uint32 i=_Connections.size();i--;)
		{
			if (_Connections[i]==connection)
			{
				_Connections[i]=_Connections.back();
				_Connections.pop_back();
			}
		}
	}

	IRemoteSavesConnection* CRemoteSavesManagerImplementation::getConnection(const NLMISC::CSString& shardName,const NLMISC::CSString& type) const
	{
		// look through existing connections for one that matches our criteria
		for (uint32 i=0;i<_Connections.size();++i)
		{
			if (_Connections[i]->getShardName()==shardName && _Connections[i]->getType()==type)
				return _Connections[i];
		}

		// the connection module wasn't found so instantiate a new one
		uint32 newModuleId= CModuleManager::getInstance()->addModule("RS",shardName+" "+type);
		DROP_IF(newModuleId==~0u,"Failed to instantiate new remote shard connection module : RS "+shardName+" "+type,return NULL);

		// get a pointer to the new module and cast it across to the type that we need...
		return safe_cast<CRemoteSavesConnectionModule*>(&*CModuleManager::getInstance()->lookupModuleById(newModuleId));
	}

	//-----------------------------------------------------------------------------
	// methods CRemoteSavesManager
	//-----------------------------------------------------------------------------

	CRemoteSavesManager* CRemoteSavesManager::getInstance()
	{
		return CRemoteSavesManagerImplementation::getInstance();
	}

}
