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
#include "backup_service_messages.h"
#include "utils.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLNET;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// NLMISC CVariables
//-------------------------------------------------------------------------------------------------

// configuration variables - to be setup in cfg files
CVariable<string>	BackupServiceIP("variables","BSHost", "Host address and port of backup service (ip:port)", "localhost", 0, true );
CVariable<uint16>	BackupServiceL3Port("variables","L3BSPort", "Port for the layer 3 backup connection", 0, 0, true );
CVariable<string>	SlaveBackupServiceIP("variables","SlaveBSHost", "Host address and port of slave backup service (ip:port)", "", 0, true );
CVariable<uint16>	SlaveBackupServiceL3Port("variables","L3SlaveBSPort", "Port for the layer 3 slave backup connection", 0, 0, true );
//CVariable<string>	PDBackupServiceIP("variables","PDBSHost", "Host address and port of backup service (ip:port)", "", 0, true );
//CVariable<string>	SlavePDBackupServiceIP("variables","SlavePDBSHost", "Host address and port of slave backup service (ip:port)", "", 0, true );
CVariable<bool>    	UseBS("variables","UseBS", "if 1, use the backup service or use local save", true, 0, true);

// a variable written by the BS interface for debugging purposes
CVariable<string>	BSPingHistory("BSIF", "BSPingHistory", "The recent BS ping time history", "", 0, true);


//-------------------------------------------------------------------------------------------------
// class CGenericCallbacksRegister
//-------------------------------------------------------------------------------------------------

class CGenericRequestIdRegister: public CSingleton<CGenericRequestIdRegister>
{
public:
	// add a request id / filename pair
	void pushRequestId(uint32 requestId,const std::string& fileName);

	// process all generic callbacks up to a given request id
	// this routine is called by the ping pong system
	void processCallbacks(uint32 lastRequestId);

	// get the id of the last generic request pushed intot he container
	uint32 getLastRequestId() const;

private:
	struct SRequestId
	{
		std::string FileName;
		uint32 RequestId;
	};
	typedef std::list<SRequestId> TRequestIds;
	TRequestIds _RequestIds;
};


//-------------------------------------------------------------------------------------------------
// methods CGenericCallbacksRegister
//-------------------------------------------------------------------------------------------------

void CGenericRequestIdRegister::pushRequestId(uint32 requestId,const std::string& fileName)
{
	// check for out of order entries in the request list
	BOMB_IF(!_RequestIds.empty() && (requestId<_RequestIds.back().RequestId),"Ignoring out of order request id in generic callback registration for file "+CSString(fileName).quote(),return);

	// setup a new record for this request id / file name pair
	SRequestId theNewRequestId;
	theNewRequestId.FileName=fileName;
	theNewRequestId.RequestId=requestId;

	// append the new request to our pending requests list
	_RequestIds.push_back(theNewRequestId);
}

void CGenericRequestIdRegister::processCallbacks(uint32 lastRequestId)
{
	while (!_RequestIds.empty() && sint32(lastRequestId-_RequestIds.front().RequestId)>=0)
	{
		// pop the request id out of the container
		uint32 theRequestId= _RequestIds.front().RequestId;
		CSString theFileName= _RequestIds.front().FileName;
		_RequestIds.pop_front();

		// get a pointer to the callback object for the message we've just received
		CBackupInterfaceSingleton* singleton= CBackupInterfaceSingleton::getInstance();
		CBackupServiceInterface *itf;
		NLMISC::CSmartPtr<IBackupGenericAckCallback> cb= singleton->popGenericCallback(theRequestId, itf);
		if (cb!=NULL)
		{
			// call the calback
			cb->callback(theFileName);
		}
	}
}

uint32 CGenericRequestIdRegister::getLastRequestId() const
{
	return _RequestIds.empty()? 0: _RequestIds.back().RequestId;

}


//-------------------------------------------------------------------------------------------------
// Tracking state of backup services
//-------------------------------------------------------------------------------------------------

// count of the number of backup services that are up (map indexed by backup service name)
static std::map<std::string,uint8> BackupServiceUp;

// a structure used to track pingpong history with a backup service
struct SPingPongStruct
{
	typedef std::vector<TTime> TTimes;
	TTimes Times;
	uint32 Count;
	bool ReadyToPing;
	uint32 LastGenericRequestId;
	TTime LastAckDelay;
	TTime LastSendTime;

	SPingPongStruct(): Count(0), ReadyToPing(true), LastGenericRequestId(0) { Times.resize(20,0); LastAckDelay=0; LastSendTime=0; }
};
// ping pong history of all connected backup services
typedef std::map<NLNET::TServiceId, SPingPongStruct> TPingPongs;
TPingPongs PingPongs;

static void cbBSConnection( const std::string &serviceName, NLNET::TServiceId serviceId, void * /* arg */ )
{
	nlinfo("Backup service connects: (name %s, Id %hu)", serviceName.c_str(), serviceId.get());

	// instantiate the PingPong record for the service that just connected and dispatch a first PING message
	PingPongs[serviceId];

	// increment the counter of active backup services
	++BackupServiceUp[serviceName];
	if (BackupServiceUp[serviceName]==1 && CBackupInterfaceSingleton::getInstance()->getBSIImplementation()==&CBSIINonModule::getInstance())
	{
		CBackupInterfaceSingleton::getInstance()->connect();
	}
}

static void cbBSDisconnection( const std::string &serviceName, NLNET::TServiceId serviceId, void * /* arg */ )
{
	nlinfo("Backup service disconnects: (name %s, Id %hu)", serviceName.c_str(), serviceId.get());
	PingPongs.erase(serviceId);

	// decrement the counter of active backup services
	--BackupServiceUp[serviceName];
	if (BackupServiceUp[serviceName]==0 && CBackupInterfaceSingleton::getInstance()->getBSIImplementation()==&CBSIINonModule::getInstance())
	{
		CBackupInterfaceSingleton::getInstance()->disconnect();
	}
}


//-------------------------------------------------------------------------------------------------
// message callbacks
//-------------------------------------------------------------------------------------------------

static void cbFile( NLNET::CMessage& msgin, const std::string &/* serviceName */, NLNET::TServiceId /* serviceId */ )
{
	// decorticate the input message
	CBackupMsgReceiveFile msg;
	msgin.serial(msg);

	// get a pointer to the callback object for the message we've just received
	CBackupInterfaceSingleton* singleton= CBackupInterfaceSingleton::getInstance();
	CBackupServiceInterface *itf;
	NLMISC::CSmartPtr<IBackupFileReceiveCallback> cb= singleton->popFileCallback(msg.RequestId, itf);
	BOMB_IF(cb==NULL,"Received a file from backup service - but can't find a matching request: "+msg.FileDescription.FileName+NLMISC::toString(" (RequestId=%d)",msg.RequestId),return);

	// make sure the file size reported by the FileDescription corresponds to the quantity of data remaining in the message
	BOMB_IF(msg.FileDescription.FileSize!=msg.Data.length()-msg.Data.getPos(),"Throwing out file because message header is corrupt or message is incomplete: "+msg.FileDescription.FileName, return);

	// Restore the original filename (without the remote path). Assumes there can't be more than one BS interface.
	// This will fail if the remote path has changed between the request and now.
	msg.FileDescription.stripFilename(itf->getRemotePath());

	// call the callback
	cb->callback(msg.FileDescription,msg.Data);
}

void CBSIINonModule::cbBsReadMode( CMessage &msgin, TSockId /* from */, CCallbackNetBase &/* netbase */ )
{
	// extract the response from BS
	bool readMode;
	msgin.serial(readMode);

	CBSIINonModule &bsi = CBSIINonModule::getInstance();
	bsi._BSHandshakeDone = true;
	bsi._BSInReadMode = readMode;
}

static void cbSyncFile( CMessage &msgin, TSockId /* from */, CCallbackNetBase &/* netbase */ )
{
	// just recall the layer 5 callback with fake service info
	cbFile(msgin, "", TServiceId(0));
}

static void cbFileClass( NLNET::CMessage& msgin, const std::string &/* serviceName */, NLNET::TServiceId /* serviceId */ )
{
	// decorticate the input message
	CBackupMsgReceiveFileClass msg;
	msgin.serial(msg);

	// get a pointer to the callback object for the message we've just received
	CBackupInterfaceSingleton* singleton= CBackupInterfaceSingleton::getInstance();
	CBackupServiceInterface *itf;
	NLMISC::CSmartPtr<IBackupFileClassReceiveCallback> cb= singleton->popFileClassCallback(msg.RequestId, itf);
	BOMB_IF(cb==NULL,"Received a file class from backup service - but can't find a matching request!"+NLMISC::toString(" RequestId=%d",msg.RequestId),return);

	// Restore the original filenames (without the remote path). Assumes there can't be more than one BS interface.
	// This will fail if the remote path has changed between the request and now.
	msg.Fdc.stripFilename(itf->getRemotePath());

	// call the callback
	cb->callback(msg.Fdc);
}

static void cbSyncFileClass( CMessage &msgin, TSockId /* from */, CCallbackNetBase &/* netbase */ )
{
	// just recall the layer 5 callback with fake service info
	cbFileClass(msgin, "", TServiceId(0));
}


void	cbAppend(NLNET::CMessage& msgin, const std::string &/* serviceName */, NLNET::TServiceId /* serviceId */)
{
	CBackupMsgAppendCallback	inMsg;
	msgin.serial(inMsg);
}


void cbBSPingPong(NLNET::CMessage& /* msgin */, const std::string &/* serviceName */, NLNET::TServiceId serviceId)
{
	// get a refference to the backup service' pingpong record
	SPingPongStruct& thePingPong= PingPongs[serviceId];

	// workout the size of the Times vector
	uint32 numTimes= (uint32)thePingPong.Times.size();

	// store away the current time in the next available slot
	TTime timeNow=NLMISC::CTime::getLocalTime();
	thePingPong.Times[thePingPong.Count%numTimes]= timeNow;
	++thePingPong.Count;

	// calculate the time since the last ack
	thePingPong.LastAckDelay= timeNow- thePingPong.LastSendTime;

	// flag ourselves ready to send a ping back at the next service update
	thePingPong.ReadyToPing= true;

	// process the generic callbacks that are implicitly acked with this pong message
	CGenericRequestIdRegister::getInstance().processCallbacks(thePingPong.LastGenericRequestId);

	// store away the last request id for the next time round the loop
	thePingPong.LastGenericRequestId= CGenericRequestIdRegister::getInstance().getLastRequestId();
}


// Callback array for layer 5 comm (normal operation)
static TUnifiedCallbackItem CbArray[] =
{
	{	"bs_file",			cbFile,			},
	{	"BS_FILE_CLASS",	cbFileClass,	},
	{	"APPEND",			cbAppend		},
	{	"BS_PONG",			cbBSPingPong	},
};

// Callback array for layer 3 comm (for synchronous file loading and initial handshake)
TCallbackItem	CBSIINonModule::CbSyncArray[] =
{
	{	"BS_READ_MODE",		CBSIINonModule::cbBsReadMode,		},
	{	"bs_file",			cbSyncFile,			},
	{	"BS_FILE_CLASS",	cbSyncFileClass,	},
//	{	"APPEND",			cbAppend		},
//	{	"BS_PONG",			cbBSPingPong	},
};


//-------------------------------------------------------------------------------------------------
// methods CBSIINonModule
//-------------------------------------------------------------------------------------------------

CBSIINonModule::CBSIINonModule()
{
	_Initialised= false;
//	_HaveSeparatePDBS= false;
}

void CBSIINonModule::serviceUpdate()
{
	// update the ping pongs...
	for (TPingPongs::iterator it= PingPongs.begin(); it!=PingPongs.end(); ++it)
	{
		// if this connected BS is ready for another ping then send it
		if (it->second.ReadyToPing)
		{
			NLNET::CMessage msgOut("BS_PING");
			NLNET::CUnifiedNetwork::getInstance()->send( it->first, msgOut );
			it->second.ReadyToPing= false;
			it->second.LastSendTime= NLMISC::CTime::getLocalTime();
		}
	}

	// build a string in an NLMISC variable for debug display
	CSString displayStr;
	// iterate over all of the currently connected backup services
	for (TPingPongs::iterator it= PingPongs.begin(); it!=PingPongs.end(); ++it)
	{
		displayStr+=NLMISC::toString("%sBS(%hu)[",displayStr.empty()?"":"; ",it->first.get());

		// workout the size of the Times vector
		uint32 numTimes= (uint32)it->second.Times.size();

		// iterate down from min(...)-1 to 1
		for (uint32 i=numTimes;--i;)
		{
			TTime prevTime= it->second.Times[(it->second.Count-i-1)%numTimes];
			TTime nextTime= it->second.Times[(it->second.Count-i)%numTimes];
			displayStr+=NLMISC::toString("%s%u",(displayStr.right(1)=="[")?"":",",(uint32)(nextTime-prevTime));
		}

		displayStr+="]";
	}
	BSPingHistory= displayStr;
}

void CBSIINonModule::release()
{
	if (_L3BSConn.connected())
		_L3BSConn.disconnect();
}


void CBSIINonModule::activate()
{
	// if we're already initialised then just drop out
	if (_Initialised)
	{
		// if there are backup services connected then we can connect right away
		if (BackupServiceUp["BS"]!=0/* && (!_HaveSeparatePDBS || BackupServiceUp["PDBS"]!=0)*/ )
		{
			CBackupInterfaceSingleton::getInstance()->connect();
		}
		return;
	}

	_Initialised= true;

	CUnifiedNetwork::getInstance()->setServiceUpCallback ("BS", cbBSConnection, NULL);
	CUnifiedNetwork::getInstance()->setServiceDownCallback ("BS", cbBSDisconnection, NULL);

//	CUnifiedNetwork::getInstance()->setServiceUpCallback ("PDBS", cbBSConnection, NULL);
//	CUnifiedNetwork::getInstance()->setServiceDownCallback ("PDBS", cbBSDisconnection, NULL);

	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( CbArray, sizeof(CbArray)/sizeof(CbArray[0]) );

	string host = BackupServiceIP;
	if(host.empty())
	{
		nlwarning("Can't use backup because BSHost variable is empty");
		return;
	}
	if (host.find (":") == string::npos)
		host+= ":49990";

	_BSMasterAddress = CInetAddress(host);
	CUnifiedNetwork::getInstance()->addService ("BS", _BSMasterAddress);

	// connect to the slave bs if any
	host = SlaveBackupServiceIP.get();
	if (!host.empty())
	{
		if (host.find (":") == string::npos)
			host+= ":49990";

		_BSSlaveAddress = CInetAddress(host);
		CUnifiedNetwork::getInstance()->addService ("BS", _BSSlaveAddress);
	}

	// connect to the global bs if any
//	host = PDBackupServiceIP.get();
//	_HaveSeparatePDBS= (!host.empty());
//	if (_HaveSeparatePDBS)
//	{
//		if (host.find (":") == string::npos)
//			host+= ":49990";
//		CUnifiedNetwork::getInstance()->addService ("PDBS", CInetAddress(host));
//
//		// connect to the global slave bs if any
//		host = SlavePDBackupServiceIP.get();
//		if (!host.empty())
//		{
//			if (host.find (":") == string::npos)
//				host+= ":49990";
//			CUnifiedNetwork::getInstance()->addService ("PDBS", CInetAddress(host));
//		}
//	}

	// init the l3 BS connector
	_L3BSConn.addCallbackArray(CbSyncArray, sizeofarray(CbSyncArray));

}

void CBSIINonModule::dispatchRequestFile(const std::string& bsiname,uint32 requestId,const std::string& fileName)
{
	nlassert(bsiname=="BS"/* || bsiname=="PDBS"*/);

	CBackupMsgRequestFile msg;
	msg.RequestId= requestId;
	msg.FileName= fileName;

	NLNET::CMessage msgOut("load_file");
	msgOut.serial(msg);
	NLNET::CUnifiedNetwork::getInstance()->send( "BS", msgOut );
}

void CBSIINonModule::deactivate()
{
}

void CBSIINonModule::dispatchSendFile(const std::string& bsiname,uint32 requestId,const CBackupMsgSaveFile& msg)
{
	nlassert(bsiname=="BS"/* || bsiname=="PDBS"*/);

	// register the request id for later use
	CGenericRequestIdRegister::getInstance().pushRequestId(requestId,msg.FileName);

	// dispatch the request to the BS
	NLNET::CUnifiedNetwork::getInstance()->send( "BS", msg.DataMsg );
}

void CBSIINonModule::dispatchAppendData(const std::string& bsiname,uint32 requestId,const CBackupMsgSaveFile& msg)
{
	nlassert(bsiname=="BS"/* || bsiname=="PDBS"*/);

	// register the request id for later use
	CGenericRequestIdRegister::getInstance().pushRequestId(requestId,msg.FileName);

	// dispatch the request to the BS
	NLNET::CUnifiedNetwork::getInstance()->send( "BS", msg.DataMsg );
}

void CBSIINonModule::dispatchAppendText(const std::string& bsiname,uint32 requestId,const std::string& filename, const std::string& line)
{
	nlassert(bsiname=="BS"/* || bsiname=="PDBS"*/);

	// register the request id for later use
	CGenericRequestIdRegister::getInstance().pushRequestId(requestId,filename);

	CBackupMsgAppend	msg;
	msg.FileName = filename;
	msg.Append = line;

	// dispatch the request to the BS
	NLNET::CMessage	msgOut("APPEND");
	msgOut.serial(msg);
	NLNET::CUnifiedNetwork::getInstance()->send( "BS", msgOut );
}

void CBSIINonModule::dispatchDeleteFile(const std::string& bsiname,uint32 requestId,const std::string& fileToDelete, bool keepBackupOfFile)
{
	nlassert(bsiname=="BS"/* || bsiname=="PDBS"*/);

	// register the request id for later use
	CGenericRequestIdRegister::getInstance().pushRequestId(requestId,fileToDelete);

	// dispatch the request to the BS
	NLNET::CMessage msgOut;
	msgOut.setType( keepBackupOfFile ? "DELETE_FILE" : "DELETE_FILE_NO_BACKUP" );
	msgOut.serial(const_cast<std::string&>(fileToDelete));
	NLNET::CUnifiedNetwork::getInstance()->send( "BS", msgOut );
}

void CBSIINonModule::dispatchRequestFileClass(const std::string& bsiname,uint32 requestId,const std::string& directory, const std::vector<CBackupFileClass>& classes)
{
	nlassert(bsiname=="BS"/* || bsiname=="PDBS"*/);

	CBackupMsgFileClass	msg;
	msg.RequestId = requestId;
	msg.Directory = directory;
	msg.Classes = classes;

	NLNET::CMessage	msgOut("GET_FILE_CLASS");
	msgOut.serial(msg);
	NLNET::CUnifiedNetwork::getInstance()->send( "BS", msgOut );
}

void CBSIINonModule::connectL3()
{
	while (!_L3BSConn.connected())
	{
		// try connecting to backup master
		breakable
		{
			std::string bsName;

			CInetAddress addr1 = _BSMasterAddress;
			addr1.setPort(BackupServiceL3Port);

			CInetAddress addr2 = _BSMasterAddress;
			addr2.setPort(addr2.port()+1);

			try
			{
				// try connecting to the BS Master on an explicit port
				_L3BSConn.connect(addr1);
				bsName= addr1.asString();
			}
			catch (...)
			{
				try
				{
					// connect to the BS Master layer 3 interface (port+1)
					_L3BSConn.connect(addr2);
					bsName= addr2.asString();
				}
				catch (...)
				{
					nlinfo("Connection to BS Master: %s or %s Failed",addr1.asString().c_str(),addr2.asString().c_str());
					break;
				}
			}

			if (isConnectedL3BSInReadWriteMode())
			{
				nlinfo("Connection to BS Master: %s in read/write mode succeeded",bsName.c_str());
				return;
			}

			nlinfo("Connection to BS Master: %s succeeded but BS is in Write Only mode",bsName.c_str());
			_L3BSConn.disconnect();
		}

		// connection to backup master failed so try connecting to backup slave
		breakable
		{
			std::string bsName;

			CInetAddress addr1 = _BSSlaveAddress;
			addr1.setPort(SlaveBackupServiceL3Port);

			CInetAddress addr2 = _BSSlaveAddress;
			addr2.setPort(addr2.port()+1);

			try
			{
				// try connecting to the BS Slave on an explicit port
				_L3BSConn.connect(addr1);
				bsName= addr1.asString();
			}
			catch (...)
			{
				try
				{
					// connect to the BS Slave layer 3 interface (port+1)
					_L3BSConn.connect(addr2);
					bsName= addr2.asString();
				}
				catch (...)
				{
					nlinfo("Connection to BS Slave: %s or %s Failed",addr1.asString().c_str(),addr2.asString().c_str());
					break;
				}
			}

			if (isConnectedL3BSInReadWriteMode())
			{
				nlinfo("Connection to BS Slave: %s in read/write mode succeeded",bsName.c_str());
				return;
			}

			nlinfo("Connection to BS Slave: %s succeeded but BS is in Write Only mode",bsName.c_str());
			_L3BSConn.disconnect();
		}

		nlinfo("Sleeping a few seconds before retrying connection to backup services...");
		nlSleep(10);
	}

}

bool CBSIINonModule::isConnectedL3BSInReadWriteMode()
{
	// do a synchronous comm with the BS to determine the read state
	// return true if the BS is in read/write mode, false if it is in write only

	// We use the L3 connection to do this (as for synchronous load method)
	if (!_L3BSConn.connected())
		return false;

	CMessage msgOut("GET_READ_MODE");
	_L3BSConn.send(msgOut);

	// preset the value before the receive loop
	_BSHandshakeDone = false;
	_BSInReadMode = false;

	// wait for the response or disconnection
	do
	{
		_L3BSConn.update2(-1,10);
	}
	while(_L3BSConn.connected() && !_BSHandshakeDone);

	return _L3BSConn.connected() && _BSInReadMode;
}

void CBSIINonModule::dispatchSyncLoadFileClass(const std::string& bsiname,uint32 requestId,const std::string& directory, const std::vector<CBackupFileClass>& classes)
{
	nlassert(bsiname=="BS"/* || bsiname=="PDBS"*/);

	do
	{
		CBackupMsgFileClass	msg;
		msg.RequestId = requestId;
		msg.Directory = directory;
		msg.Classes = classes;

		// make sure the L3 connection is done
		connectL3();

		NLNET::CMessage	msgOut("GET_FILE_CLASS");
		msgOut.serial(msg);

		// send the request
		_L3BSConn.send( msgOut );

		CBackupInterfaceSingleton* singleton= CBackupInterfaceSingleton::getInstance();

		// loop until the request is served
		do
		{
			_L3BSConn.update2(-1, 10);
		}
		while(!singleton->fileClassCallbackDone(requestId) && _L3BSConn.connected());

	}
	while (!_L3BSConn.connected());
}


void CBSIINonModule::dispatchSyncLoadFile(const std::string& bsiname,uint32 requestId,const std::string& fileName, bool notBlocking)
{
	nlassert(bsiname=="BS"/* || bsiname=="PDBS"*/);

	if (notBlocking)
	{
		bool insertResult;
		insertResult= _L3PendingResquest.insert(make_pair(requestId,fileName)).second;
		STOP_IF(!insertResult,"Duplicate request ID found - this is BAD!: id="<<requestId<<" for file: "<<fileName);
	}

	// register the request id for later use
	CGenericRequestIdRegister::getInstance().pushRequestId(requestId,fileName);

	do
	{
		CBackupMsgRequestFile msg;
		msg.RequestId= requestId;
		msg.FileName= fileName;

		NLNET::CMessage msgOut("load_file");
		msgOut.serial(msg);

		// make sure the L3 conection is done
		connectL3();

		// send the request
		_L3BSConn.send( msgOut );

		CBackupInterfaceSingleton* singleton= CBackupInterfaceSingleton::getInstance();

		if (!notBlocking)
		{
			// loop until the request is served
			do
			{
				_L3BSConn.update2(-1, 10);
			} while(!singleton->fileCallbackDone(requestId));
		}

	}
	while (!_L3BSConn.connected());
}

void CBSIINonModule::terminateSyncLoads()
{
	CBackupInterfaceSingleton* singleton= CBackupInterfaceSingleton::getInstance();

	// loop until all request are served
	while (!_L3PendingResquest.empty())
	{
		// update the connection
		_L3BSConn.update2(-1, 10);

		map<uint32, std::string>::iterator first(_L3PendingResquest.begin()), last(_L3PendingResquest.end());
		for (; first != last; ++first)
		{
			if (singleton->fileCallbackDone(first->first))
			{
				_L3PendingResquest.erase(first);
				break;
			}
		}

		if (!_L3BSConn.connected())
		{
			// connection lost during loading, reconnect and reload all remaining file
			nlinfo("Lost connection with BS during loadSyncFiles !, reconnecting and reloading %u left files", _L3PendingResquest.size());
			connectL3();

			map<uint32, std::string>::iterator first(_L3PendingResquest.begin()), last(_L3PendingResquest.end());
			for (; first != last; ++first)
			{
				dispatchSyncLoadFile("BS", first->first, first->second, true);
			}
		}
	}
}


NLMISC::TTime CBSIINonModule::getLastAckTime() const
{
	NLMISC::TTime result;

	// if there's no BS connected then give up
	if (PingPongs.empty())
	{
		result=0;
		return result;
	}

	// start by setting result to a really big positive number
	result= uint64(sint64(-1))>>1;

	// iterate over all of the currently connected backup services
	for (TPingPongs::iterator it= PingPongs.begin(); it!=PingPongs.end(); ++it)
	{
		// get a refference to the backup service' pingpong record
		SPingPongStruct& thePingPong= it->second;

		// workout the size of the Times vector
		uint32 numTimes= (uint32)thePingPong.Times.size();

		// compare the last entry in the times vector to the result so far...
		result= min(result,thePingPong.Times[(thePingPong.Count-1)%numTimes]);
	}

	return result;
}

NLMISC::TTime CBSIINonModule::getLastAckDelay() const
{
	NLMISC::TTime result= 0;

	// if there's no BS connected then give up
	if (PingPongs.empty())
	{
		return result;
	}

	// iterate over all of the currently connected backup services
	for (TPingPongs::iterator it= PingPongs.begin(); it!=PingPongs.end(); ++it)
	{
		// get a refference to the backup service' pingpong record
		SPingPongStruct& thePingPong= it->second;

		// compare this connection's ack delay to the result so far
		result= max(result,thePingPong.LastAckDelay);
	}

	// return the largest ack delay of all our connections
	return result;
}
