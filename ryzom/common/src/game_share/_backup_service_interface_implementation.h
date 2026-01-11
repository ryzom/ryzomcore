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

#ifndef _BACKUP_SERVICE_INTERFACE_IMPLEMENTATION_H
#define	_BACKUP_SERVICE_INTERFACE_IMPLEMENTATION_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/singleton.h"
#include "singleton_registry.h"
#include "backup_service_interface.h"


//-------------------------------------------------------------------------------------------------
// class IBackupServiceInterfaceImplementation
//-------------------------------------------------------------------------------------------------

class IBackupServiceInterfaceImplementation
{
public:
	// make dtor virtual - also ensure that we unregister from interface singleton
	// on destruction
	virtual ~IBackupServiceInterfaceImplementation();

	// activate (called by CBackupInterfaceSingleton::setBSIImplementation() )
	// if we're already ready then we should call CBackupInterfaceSingleton::connect() here
	virtual void activate()=0;

	// deactivate (called by CBackupInterfaceSingleton::setBSIImplementation() )
	virtual void deactivate()=0;

	// send a request for info on files matching one or more patterns
	virtual void dispatchRequestFileClass(const std::string& bsiname,uint32 requestId,const std::string& directory, const std::vector<CBackupFileClass>& classes)=0;

	// send a request for info on files matching one or more patterns (synchrously)
	virtual void dispatchSyncLoadFileClass(const std::string& bsiname,uint32 requestId,const std::string& directory, const std::vector<CBackupFileClass>& classes)=0;

	// request a file (this is a rad)
	virtual void dispatchRequestFile(const std::string& bsiname,uint32 requestId,const std::string& fileName)=0;

	// load a file (synchronously)
	virtual void dispatchSyncLoadFile(const std::string& bsiname,uint32 requestId,const std::string& fileName, bool notBlocking)=0;

	// terminate all synchronous file loading pending (to terminate a synchronous multi file loading)
	virtual void terminateSyncLoads() =0;

	// send a file (this is a write)
	virtual void dispatchSendFile(const std::string& bsiname,uint32 requestId,const CBackupMsgSaveFile& msg)=0;

	// append binary data blob to a file (create file if need be)
	virtual void dispatchAppendData(const std::string& bsiname,uint32 requestId,const CBackupMsgSaveFile& msg)=0;

	// append text line to a file (create file if need be)
	virtual void dispatchAppendText(const std::string& bsiname,uint32 requestId,const std::string& filename, const std::string& line)=0;

	// delete a file (and store away a copy if required)
	virtual void dispatchDeleteFile(const std::string& bsiname,uint32 requestId,const std::string& fileToDelete, bool keepBackupOfFile)=0;

	// Return the timestamp of the last packet ack (or ping pong) from the backup system
	virtual NLMISC::TTime getLastAckTime() const=0;

	// Return the time between dispatch of last acked packet (or ping pong) and the associated ack
	virtual NLMISC::TTime getLastAckDelay() const=0;
};


//-------------------------------------------------------------------------------------------------
// class CBSIINonModule
//-------------------------------------------------------------------------------------------------

class CBSIINonModule: public IBackupServiceInterfaceImplementation, public NLMISC::CSingleton<CBSIINonModule>, public IServiceSingleton
{
public:
	CBSIINonModule();
	void serviceUpdate();

	void release();

	void activate();
	void deactivate();

	void dispatchRequestFileClass(const std::string& bsiname,uint32 requestId,const std::string& directory, const std::vector<CBackupFileClass>& classes);
	void dispatchSyncLoadFileClass(const std::string& bsiname,uint32 requestId,const std::string& directory, const std::vector<CBackupFileClass>& classes);
	void dispatchRequestFile(const std::string& bsiname,uint32 requestId,const std::string& fileName);
	void dispatchSyncLoadFile(const std::string& bsiname,uint32 requestId,const std::string& fileName, bool notBlocking);
	void terminateSyncLoads();
	void dispatchSendFile(const std::string& bsiname,uint32 requestId,const CBackupMsgSaveFile& msg);
	void dispatchAppendData(const std::string& bsiname,uint32 requestId,const CBackupMsgSaveFile& msg);
	void dispatchAppendText(const std::string& bsiname,uint32 requestId,const std::string& filename, const std::string& line);
	void dispatchDeleteFile(const std::string& bsiname,uint32 requestId,const std::string& fileToDelete, bool keepBackupOfFile);

	NLMISC::TTime getLastAckTime() const;
	NLMISC::TTime getLastAckDelay() const;

private:
	// establish or reestablish the layer 3 connection to the BS
	void connectL3();

	// do the handshake with a BS to determine if it is in read/write mode
	bool isConnectedL3BSInReadWriteMode();


	bool _Initialised;

	// when connecting to a BS, indicated that the handshake is done
	bool	_BSHandshakeDone;
	// a temporary flag indicating the result of the last BS handshake
	bool	_BSInReadMode;

//	bool _HaveSeparatePDBS;
	NLNET::CInetAddress		_BSMasterAddress;
	NLNET::CInetAddress		_BSSlaveAddress;

	// Layer 3 connection to the BS
	NLNET::CCallbackClient	_L3BSConn;
	// Layer 3 pending request
	std::map<uint32, std::string>		_L3PendingResquest;

	static void cbBsReadMode( NLNET::CMessage &msgin, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase );

	static NLNET::TCallbackItem	CbSyncArray[];

};


//-------------------------------------------------------------------------------------------------
#endif
