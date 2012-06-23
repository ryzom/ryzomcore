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

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stdpch.h"
#include "_backup_service_interface_singleton.h"
#include "utils.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLNET;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// Globals for instance counters - for memory leak detection
//-------------------------------------------------------------------------------------------------

NL_INSTANCE_COUNTER_IMPL(IBackupFileReceiveCallback);
NL_INSTANCE_COUNTER_IMPL(IBackupFileClassReceiveCallback);
NL_INSTANCE_COUNTER_IMPL(IBackupGenericAckCallback);
NL_INSTANCE_COUNTER_IMPL(CBackupInterfaceSingleton);


//-------------------------------------------------------------------------------------------------
// NLMISC CVariables
//-------------------------------------------------------------------------------------------------

// method used to update stuff on config file reload / variable change
void onSaveShardRootModified( NLMISC::IVariable &var );
// configuration variables - to be setup in cfg files
CVariable<string>	SaveShardRoot("variables", "SaveShardRoot", "Root directory of all files saved by any shard", "", 0, true, onSaveShardRootModified, false);

// stats variables
CVariable<NLMISC::TTime> BSLastAckTime("BSIF", "BSLastAckTime", "The timestamp of the last ack received from backup system", 0, 0, true);
CVariable<NLMISC::TTime> BSTimeSinceLastAck("BSIF", "BSTimeSinceLastAck", "The time since the last ack from the backup system", 0, 0, true);
CVariable<NLMISC::TTime> BSLastAckDelay("BSIF", "BSLastAckDelay", "The time it took for the last received ack to be received from slowest of our connected BS services", 0, 0, true);
CVariable<uint32>		 BSResponseTime("BSIF", "BSResponseTime", "Latest/average time (in sec) between BS request and response", 0, 20, false );


//-------------------------------------------------------------------------------------------------
// public routines for accessing CBackupServiceInterface properties of singleton
//-------------------------------------------------------------------------------------------------

// Return the instances of the Back Service Interface singleton
CBackupServiceInterface& getShardDependentBsi() { return CBackupInterfaceSingleton::getInstance()->_ShardDependentBsi; }
CBackupServiceInterface& getGlobalBsi() { return CBackupInterfaceSingleton::getInstance()->_GlobalBsi; }
//CBackupServiceInterface& getPDBsi() { return CBackupInterfaceSingleton::getInstance()->_PDBsi; }


//-------------------------------------------------------------------------------------------------
// methods CBackupInterfaceSingleton - ctor / dtor / init / update
//-------------------------------------------------------------------------------------------------

CBackupInterfaceSingleton* CBackupInterfaceSingleton::getInstance()
{
	static CBackupInterfaceSingleton* instance=NULL;
	if (instance==NULL)
		instance= new CBackupInterfaceSingleton;
	return instance;
}

CBackupInterfaceSingleton::CBackupInterfaceSingleton()
{
	_Counter=0;
	_IsConnected=false;
	_BackupServiceInterfaceImplementation=NULL;
}

void CBackupInterfaceSingleton::init()
{
	bool packingSheets= IService::getInstance()->haveArg('Q');
	if (packingSheets)
	{
		nlinfo("Not initialising BS Interface because -Q flag found meaning that we are just packing sheets and exitting");
		return;
	}

	nlassert(UseBS);

	// temporary cludge - setup a connection to the backup service
	this->setBSIImplementation(&CBSIINonModule::getInstance());

	_ShardDependentBsi.init("BS");
	_ShardDependentBsi.setRemotePath( IService::getInstance()->SaveFilesDirectory.toString() );
	_ShardDependentBsi.setLocalPath( CPath::standardizePath( SaveShardRoot.get() ) + IService::getInstance()->SaveFilesDirectory.toString() );

	_GlobalBsi.init("BS");
	_GlobalBsi.setRemotePath( string() );
	_GlobalBsi.setLocalPath( SaveShardRoot.get() );

//	_PDBsi.init("PDBS");
//	_PDBsi.setRemotePath( IService::getInstance()->SaveFilesDirectory.toString() );
//	_PDBsi.setLocalPath( CPath::standardizePath( SaveShardRoot.get() ) + IService::getInstance()->SaveFilesDirectory.toString() );

	IService::getInstance()->setDirectoryChangeCallback( this );
}

void CBackupInterfaceSingleton::serviceUpdate()
{
	if (!isConnected())
	{
		return;
	}

	NLMISC::TTime timeNow= NLMISC::CTime::getLocalTime();
	NLMISC::TTime lastAckTime= getBSIImplementation()->getLastAckTime();
	NLMISC::TTime lastAckDelay= getBSIImplementation()->getLastAckDelay();

	BSLastAckTime= lastAckTime;
	BSTimeSinceLastAck= timeNow-lastAckTime;
	BSLastAckDelay= lastAckDelay;
}



//-------------------------------------------------------------------------------------------------
// methods CBackupInterfaceSingleton - backup event callback setup
//-------------------------------------------------------------------------------------------------

uint32 CBackupInterfaceSingleton::pushFileCallback(NLMISC::CSmartPtr<IBackupFileReceiveCallback>& callback, CBackupServiceInterface* itf)
{
	BOMB_IF(callback==NULL,"Illegal attempt to register a NULL callback!",return 0);
	uint32 requestId= _Counter;
	++_Counter;
	nlassert(_FileResponses.find(requestId)==_FileResponses.end());
	TBSCallbackInfo<IBackupFileReceiveCallback> cbInfo;
	cbInfo.Callback = callback;
	cbInfo.Interface = itf;
	cbInfo.RequestTime = CTime::getSecondsSince1970();
	_FileResponses.insert(make_pair(requestId, cbInfo));
	return requestId;
}

uint32 CBackupInterfaceSingleton::pushFileClassCallback(NLMISC::CSmartPtr<IBackupFileClassReceiveCallback>& callback, CBackupServiceInterface* itf)
{
	// note that it is legal the callback to be NULL
	uint32 requestId= _Counter;
	++_Counter;
	nlassert(_FileClassResponses.find(requestId)==_FileClassResponses.end());
	TBSCallbackInfo<IBackupFileClassReceiveCallback> cbInfo;
	cbInfo.Callback = callback;
	cbInfo.Interface = itf;
	cbInfo.RequestTime = CTime::getSecondsSince1970();
	_FileClassResponses.insert(make_pair(requestId, cbInfo));
	return requestId;
}

uint32 CBackupInterfaceSingleton::pushGenericAckCallback(NLMISC::CSmartPtr<IBackupGenericAckCallback>& callback, CBackupServiceInterface* itf)
{
	// note that it is legal for the callback to be NULL
	uint32 requestId= _Counter;
	++_Counter;
	if (!_GenericResponses.empty())
	{
		nlassert(_GenericResponses.front().first<requestId);
	}

	// if we have no callback then just return the generated request Id...
	if (callback==NULL)
		return requestId;

	// store away the callback for later use...
	TBSCallbackInfo<IBackupGenericAckCallback> cbInfo;
	cbInfo.Callback = callback;
	cbInfo.Interface = itf;
	cbInfo.RequestTime = CTime::getSecondsSince1970();
	_GenericResponses.push_back(make_pair(requestId, cbInfo));
	return requestId;
}


//-------------------------------------------------------------------------------------------------
// methods CBackupInterfaceSingleton - backup event callback retireval
//-------------------------------------------------------------------------------------------------

NLMISC::CSmartPtr<IBackupFileReceiveCallback> CBackupInterfaceSingleton::popFileCallback(uint32 requestId, CBackupServiceInterface*& itf)
{
	TBSCallbackInfo<IBackupFileReceiveCallback>& bsInfo = _FileResponses[requestId];
	NLMISC::CSmartPtr<IBackupFileReceiveCallback> callback = bsInfo.Callback;
	itf = bsInfo.Interface;
	BSResponseTime = CTime::getSecondsSince1970() - bsInfo.RequestTime;
	_FileResponses.erase(requestId);
	return callback;
}

NLMISC::CSmartPtr<IBackupFileClassReceiveCallback>	CBackupInterfaceSingleton::popFileClassCallback(uint32 requestId, CBackupServiceInterface*& itf)
{
	TBSCallbackInfo<IBackupFileClassReceiveCallback>& bsInfo = _FileClassResponses[requestId];
	NLMISC::CSmartPtr<IBackupFileClassReceiveCallback> callback = bsInfo.Callback;
	itf = bsInfo.Interface;
	BSResponseTime = CTime::getSecondsSince1970() - bsInfo.RequestTime;
	_FileClassResponses.erase(requestId);
	return callback;
}

NLMISC::CSmartPtr<IBackupGenericAckCallback>	CBackupInterfaceSingleton::popGenericCallback(uint32 requestId, CBackupServiceInterface*& itf)
{
	// if there are untreated callbacks in the generic responses container then yell
	while (!_GenericResponses.empty() && sint32(_GenericResponses.front().first-requestId)<0)
	{
		STOP("Skipping untreated generic callback for request: "<<_GenericResponses.front().first<<" because we are treating request id: "<<requestId);
		_GenericResponses.pop_front();
	}

	// if there's no callback for this request then just return NULL
	if (_GenericResponses.empty() || sint32(_GenericResponses.front().first-requestId)>0)
	{
		return NULL;
	}

	// if we're here it means that the front entry in the generic responses container has the same request id as requestId

	// get hold of the callback info object
	TBSCallbackInfo<IBackupGenericAckCallback>& bsInfo = _GenericResponses.front().second;
	NLMISC::CSmartPtr<IBackupGenericAckCallback> callback = bsInfo.Callback;

	// setup the itf return value before we pop the front entry off the generic responses container
	itf = bsInfo.Interface;

	// calculate the response time for the request that we just treated...
	BSResponseTime = CTime::getSecondsSince1970() - bsInfo.RequestTime;

	// pop the entry that we just treated off the front of the generic responses container
	_GenericResponses.pop_front();

	return callback;
}

//-------------------------------------------------------------------------------------------------
// methods for checking callback remaining
//-------------------------------------------------------------------------------------------------

bool CBackupInterfaceSingleton::fileCallbackDone(uint32 requestId)
{
	return _FileResponses.find(requestId) == _FileResponses.end();
}

bool CBackupInterfaceSingleton::fileClassCallbackDone(uint32 requestId)
{
	return _FileClassResponses.find(requestId) == _FileClassResponses.end();
}

bool CBackupInterfaceSingleton::genericCallbackDone(uint32 requestId)
{
	if (_GenericResponses.empty())
		return true;
	return (sint32(_GenericResponses.front().first-requestId)>0);
}


//-------------------------------------------------------------------------------------------------
// methods CBackupInterfaceSingleton - connection management
//-------------------------------------------------------------------------------------------------

void	CBackupInterfaceSingleton::pushBSConnectCallback(IBackupServiceConnection* cb)
{
	_BSConnectCallbacks.push_back(cb);

	if (_IsConnected)
		cb->cbBSconnect(true);
}

void	CBackupInterfaceSingleton::connect()
{
	// if we're already connected then nothing to do
	if (_IsConnected)
		return;

	// set the connection flag
	_IsConnected=true;

	// call all registered connection callbacks
	for (uint i=0; i<_BSConnectCallbacks.size(); ++i)
		_BSConnectCallbacks[i]->cbBSconnect(true);
}

void	CBackupInterfaceSingleton::disconnect()
{
	// if we're already disconnected then nothing to do
	if (!_IsConnected)
		return;

	// set the connection flag
	_IsConnected=false;

	// call all registered connection callbacks
	for (uint i=0; i<_BSConnectCallbacks.size(); ++i)
		_BSConnectCallbacks[i]->cbBSconnect(false);
}

bool	CBackupInterfaceSingleton::isConnected() const
{
	return _IsConnected;
}


//-------------------------------------------------------------------------------------------------
// methods CBackupInterfaceSingleton - accessors for BSI implmentation
//-------------------------------------------------------------------------------------------------

IBackupServiceInterfaceImplementation* CBackupInterfaceSingleton::getBSIImplementation()
{
	return _BackupServiceInterfaceImplementation;
}

void CBackupInterfaceSingleton::setBSIImplementation(IBackupServiceInterfaceImplementation* bsii)
{
	// if there is no change then just return...
	if (_BackupServiceInterfaceImplementation == bsii)
		return;

	// if we were previouslty connected then disconnect
	if (_BackupServiceInterfaceImplementation!=NULL)
	{
		_BackupServiceInterfaceImplementation->deactivate();
		disconnect();
	}

	// set the new implmentation
	_BackupServiceInterfaceImplementation= bsii;

	// if we're now non null then activate the new object
	if (_BackupServiceInterfaceImplementation!=NULL)
	{
		_BackupServiceInterfaceImplementation->activate();
	}
}


//-------------------------------------------------------------------------------------------------
// CBackupInterfaceSingleton routines for responding to configuration variable changes
//-------------------------------------------------------------------------------------------------

void CBackupInterfaceSingleton::onVariableChanged( NLMISC::IVariable &var )
{
	if ( var.getName() == "SaveFilesDirectory" )
	{
		_ShardDependentBsi.setRemotePath( var.toString() );
		_ShardDependentBsi.setLocalPath( CPath::standardizePath( SaveShardRoot.get() ) + var.toString() );
//		_PDBsi.setRemotePath( var.toString() );
//		_PDBsi.setLocalPath( CPath::standardizePath( SaveShardRoot.get() ) + var.toString() );
	}
	else if ( var.getName() == "SaveShardRoot" )
	{
		_ShardDependentBsi.setLocalPath( CPath::standardizePath( var.toString() ) + IService::getInstance()->SaveFilesDirectory.toString() );
		_GlobalBsi.setLocalPath( var.toString() );
//		_PDBsi.setLocalPath( CPath::standardizePath( var.toString() ) + IService::getInstance()->SaveFilesDirectory.toString() );
	}
}

void onSaveShardRootModified( NLMISC::IVariable &var )
{
	CBackupInterfaceSingleton::getInstance()->onVariableChanged( var );
}


//-------------------------------------------------------------------------------------------------
// public routine for registering bs connection callbacks with singleton
//-------------------------------------------------------------------------------------------------

void registerBSConnectionCallback(IBackupServiceConnection* cb)
{
	CBackupInterfaceSingleton::getInstance()->pushBSConnectCallback(cb);
}


//-------------------------------------------------------------------------------------------------
// methods IBackupServiceInterfaceImplementation
//-------------------------------------------------------------------------------------------------

IBackupServiceInterfaceImplementation::~IBackupServiceInterfaceImplementation()
{
	// if we are the currently active BSI implementation on the BSI singleton...
	if (CBackupInterfaceSingleton::getInstance()->getBSIImplementation()==this)
	{
		// then set the active BSI implmentation to NULL
		CBackupInterfaceSingleton::getInstance()->setBSIImplementation(NULL);
	}
}


//-------------------------------------------------------------------------------------------------
// class CBackupInterfaceSingletonInstantiator
// backupInterfaceSingletonInstantiator
//
// This object provokes instantiation of our singleton at service startup
// The instantiation of the signleton provokes registration in the singleton registry
// menaing that the init() routine gets called properly in service init, etc.
//-------------------------------------------------------------------------------------------------

class CBackupInterfaceSingletonInstantiator
{
public:
	CBackupInterfaceSingletonInstantiator()
	{
		CBackupInterfaceSingleton::getInstance();
	}
}
backupInterfaceSingletonInstantiator;


//-------------------------------------------------------------------------------------------------
