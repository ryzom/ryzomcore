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

#include "pd_lib.h"

#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>

#include <nel/net/service.h>
#include <nel/net/unified_network.h>
#include <nel/net/service.h>

#include "pd_string_mapper.h"
#include "timestamp.h"
#include "db_description_parser.h"
#include "game_share/backup_service_interface.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace RY_PDS 
{


std::vector<CPDSLib*>				CPDSLib::_Libs;



/*
 * PD System Verbosity control
 */
CVariable<bool>				PDVerbose("pd", "PDVerbose", "Control of the PD system verbosity", false, 0, true);

/**
 * PD System Verbosity level
 */
CVariable<sint>				PDVerboseLevel("pd", "PDVerboseLevel", "PD system verbosity level (the greater the more the service logs info, -1=no log, default=0)", 0, 0, true);

/*
 * PDS Resolve Unmapped rows
 * If true, PDS continue loading when it finds a row not mapped in a mapped table,
 * and keeps it unmapped. 
 * Otherwise, loading fails.
 */
CVariable<bool>				ResolveUnmappedRows("pd", "ResolveUnmappedRows", "If true, PDS continue loading when it finds a row not mapped in a mapped table, and keeps it unmapped.  Otherwise, loading fails.", true, 0, true);

/*
 * PDS Resolve Double Mapped Rows
 * If true, PDS continue loading when it finds a row already mapped, and keeps it unmapped.
 * Otherwise, loading fails.
 * See also ResolveDoubleMappedKeepOlder
 */
CVariable<bool>				ResolveDoubleMappedRows("pd", "ResolveDoubleMappedRows", "If true, PDS continue loading when it finds a row already mapped, and keeps it unmapped. Otherwise, loading fails. See also ResolveDoubleMappedKeepOlder", true, 0, true);

/*
 * PDS Resolve Double Mapped Keep Older
 * If true, when finds a doubly mapped row at loading, keep only older row as mapped (lesser row index)
 * See also ResolveDoubleMappedRows.
 */
CVariable<bool>				ResolveDoubleMappedKeepOlder("pd", "ResolveDoubleMappedKeepOlder", "If true, when finds a doubly mapped row at loading, keep only older row as mapped (lesser row index). See also ResolveDoubleMappedRows.", true, 0, true);

/*
 * PDS Resolve Unallocated rows
 * If true, when finds a reference to an unallocated row or an invalid row, force reference to null.
 */
CVariable<bool>				ResolveInvalidRow("pd", "ResolveInvalidRow", "If true, when finds a reference to an unallocated row or an invalid row, force reference to null.", true, 0, true);



/*
 * Maximal Message Queue Size in bytes
 * Set to 1mb by default
 */
CVariable<uint>				PDMaxMsgQueueSize("pd", "PDMaxMsgQueueSize", "Maximum message queue size towards PDS. If queue size is greater, PDS is displayed as not available, and processing should be halted", 1048576, 0, true);




/*
 * Enables client to log updates by itself (when PDS is not used)
 */
CVariable<bool>				PDEnableLog("pd", "PDEnableLog", "Enables client to log updates by itself (when PDS is not used)", true, 0, true);

/*
 * Enables client to log string manager updates by itself (when PDS is not used)
 */
//CVariable<bool>				PDEnableStringLog("pd", "PDEnableStringLog", "Enables client to log string manager updates by itself (when PDS is not used)", true, 0, true);

/*
 * Number of seconds between 2 updates (when PDS is not used)
 * Set to 1mb by default
 */
CVariable<uint>				PDLogUpdate("pd", "PDLogUpdate", "Number of seconds between 2 updates (when PDS is not used)", 5, 0, true);

/*
 * Time between creating 2 different pd_log file (in seconds)
 */
CVariable<uint>				PDLogFileTime("pd", "PDLogFileTime", "Time between creating 2 different pd_log file (in seconds)", 3600, 0, true);

/*
 * Directory to save logs (when PDS is not used)
 */
CVariable<std::string>		PDRootDirectory("pd", "PDRootDirectory", "Directory to save PD data (if empty, 'SaveFilesDirectory/pds' will be used instead)", "", 0, true);

/*
 * Use Backup Servce when logging
 */
CVariable<bool>				PDUseBS("pd", "PDUseBS", "Use Backup Service when logging (when PDS is not used)", true, 0, true);



/*
 * PDS is ready to work
 */
void	cbPDSReady(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	uint32	databaseId;
	msgin.serial(databaseId);

	uint32	lastUpdateId;
	msgin.serial(lastUpdateId);

	CPDSLib*	lib = CPDSLib::getLib(databaseId);

	if (lib != NULL)
		lib->ready(lastUpdateId);
}

/*
 * PDS init failed, stop everything!
 */
void	cbPDSInitFailed(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	uint32	databaseId;
	msgin.serial(databaseId);
	string	error;
	msgin.serial(error);

	nlerror("Database '%d' initialisation failed: '%s'", databaseId, error.c_str());
}

/*
 * fetch data
 */
void	cbFetch(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	uint32	databaseId;
	msgin.serial(databaseId);

	CPDSLib*	lib = CPDSLib::getLib(databaseId);

	if (lib != NULL)
		lib->fetchPDSData(msgin);
}

/*
 * fetch data failed
 */
void	cbFetchFailure(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	uint32	databaseId;
	msgin.serial(databaseId);

	CPDSLib*	lib = CPDSLib::getLib(databaseId);

	if (lib != NULL)
		lib->notifyFetchFailure(msgin);
}

/*
 * receive index allocators
 */
void	cbAllocs(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	uint32	databaseId;
	msgin.serial(databaseId);

	CPDSLib*	lib = CPDSLib::getLib(databaseId);

	if (lib == NULL)
		return;

	// do not reinit allocator as it may contain newer data
	if (lib->allocsInitialised())
		return;

	lib->setupIndexAllocators(msgin);
	lib->initAllocs();
}

/*
 * init client string manager
 */
//void	cbInitStringManager(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
//{
//	uint32	databaseId;
//	msgin.serial(databaseId);
//
//	CPDSLib*	lib = CPDSLib::getLib(databaseId);
//
//	if (lib == NULL)
//		return;
//
//	// do not reinit string manager (as it may contain newer data)
//	if (lib->stringManagerInitialised())
//		return;
//
//	CPDStringManager&	sm = lib->getStringManager();
//	sm.serial(msgin);
//
//	lib->initStringManager();
//}

/*
 * Flush client messages, when they are acknowledged by the PDS
 */
void	cbFlushAckUpdates(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	uint32	databaseId;
	msgin.serial(databaseId);

	vector<uint32>	ack;
	msgin.serialCont(ack);

	CPDSLib*	lib = CPDSLib::getLib(databaseId);

	if (lib != NULL)
		lib->flushAcknowledged(ack);
}

TUnifiedCallbackItem	PDSCbArray[] =
{
	{ "PD_READY",			cbPDSReady },
	{ "PD_INIT_FAILED",		cbPDSInitFailed },
	{ "PD_FETCH",			cbFetch },
	{ "PD_FETCH_FAILURE",	cbFetchFailure },
	{ "PD_ALLOCS",			cbAllocs },
//	{ "PD_SM_INIT",			cbInitStringManager },
	{ "PD_ACK_UPD",			cbFlushAckUpdates },
};


/*
 * Constructor
 */
CPDSLib::CPDSLib()
{
	_PDSConnected = false;
	_PDSReady = false;
	_DatabaseId = 0-1;
	_UpdateId = 0-1;
	_UsePDS = false;
	_AllocsInitialised = false;
//	_StringManagerInitialised = false;

	_PreviousLogSave = 0;
}



/*
 * Internal type checking
 */
void	CPDSLib::checkInternalTypes()
{

	if (sizeof(CColumnIndex) != 8)
		nlerror("Internal type checking failed: class CColumnIndex is not 8 bytes wide");

	if (sizeof(CObjectIndex) != 8)
		nlerror("Internal type checking failed: class CObjectIndex is not 8 bytes wide");

}




void	CPDSLib::usePDS()
{
	_UsePDS = true;
}

// Init Db description
void	CPDSLib::init(const std::string &xml, uint32 overrideDbId)
{
	nlinfo("CPDSLib: %12s", "init");

	// initial checkup
	checkInternalTypes();

	_XmlDescription = xml;

	CUnifiedNetwork::getInstance()->addCallbackArray(PDSCbArray, sizeof(PDSCbArray)/sizeof(TUnifiedCallbackItem));
	CUnifiedNetwork::getInstance()->setServiceUpCallback("PDS", onPDSUp);
	CUnifiedNetwork::getInstance()->setServiceDownCallback("PDS", onPDSDown);

	CConfigFile::CVar*	dbidptr = IService::getInstance()->ConfigFile.getVarPtr("DatabaseId");
	if (overrideDbId != 0)
	{
		nlinfo("CPDSLib: variable 'DatabaseId' orverriden with Id='%d'", overrideDbId);
		_DatabaseId = overrideDbId;
	}	
	else if (dbidptr != NULL && overrideDbId == 0)
	{
		_DatabaseId = dbidptr->asInt();
	} 
	else
	{
		nlwarning("CPDSLib: variable 'DatabaseId' not found in config file, will use '0' as default value");
		_DatabaseId = 0;
	}

	if (_Libs.size() <= _DatabaseId)
		_Libs.resize(_DatabaseId+1, NULL);
	_Libs[_DatabaseId] = this;

//	_StringManager.init(this);

	if (!_UsePDS)
	{
//		_StringManager.load();

		// save description in log repertory
		std::string	logDir = getRemoteLogDirectory();
		bool		repSuccess = true;

		if (!CFile::isDirectory(logDir))
		{
			if (!CFile::createDirectoryTree(logDir))
			{
				nlwarning("Failed to create log root directory '%s'", logDir.c_str());
				repSuccess = false;
			}

			if (!CFile::setRWAccess(logDir))
			{
				nlwarning("Failed, can't set RW access to directory '%s'", logDir.c_str());
				repSuccess = false;
			}
		}

		_LogStartDate.setToCurrent();
		_PreviousTickDate = _LogStartDate;

		if (repSuccess)
		{
			CDBDescriptionParser	dbDesc;
			dbDesc.loadDescription((const uint8*)(xml.c_str()));
//			dbDesc.saveDescription(logDir + _LogStartDate.toString() + ".description");
		}
	}
}



/*
 * Init PDS log
 */
void	CPDSLib::initLog(uint logmsg)
{
	if (logmsg >= _LogDescs.size())
		_LogDescs.resize(logmsg + 1);

	nlassertex(_LogDescs[logmsg].Log == 0xffffffff, ("Internal error! Log message %d already declared!", logmsg));

	_LogDescs[logmsg].Log = logmsg;
	_LogDescs[logmsg].ByteSize = 0;
	_LogDescs[logmsg].Params.clear();
}

/*
 * Init PDS log parameter
 */
void	CPDSLib::initLogParam(uint logmsg, uint logparam, uint byteSize)
{
	nlassertex(_LogDescs.size() > logmsg, ("Internal error! Log message %d is not yet declared!", logmsg));

	CLogDesc&	desc = _LogDescs[logmsg];
	nlassertex(desc.Log == logmsg, ("Internal error! Log message %d badly or not initialised!", logmsg));
	nlassertex(desc.Params.size() == logparam, ("Internal error! Log parameter %d of message %d already initialised!", logmsg, logparam));

	uint	byteOffset = desc.ByteSize;

	CLogParam	param;
	param.ByteSize = byteSize;
	param.ByteOffset = byteOffset;
	desc.Params.push_back(param);

	desc.ByteSize += byteSize;
}


/*
 * Release Lib
 */
void	CPDSLib::release()
{
}


/*
 * PDS up callback
 */
void	CPDSLib::onPDSUp(const std::string &serviceName, TServiceId sid, void *arg)
{
	uint	i;
	for (i=0; i<_Libs.size(); ++i)
	{
		if (_Libs[i] != NULL)
		{
			_Libs[i]->_PDSConnected = true;
			_Libs[i]->_PDSReady = false;
			_Libs[i]->connect();
		}
	}
}

/*
 * Connect service to PDS when up
 */
void	CPDSLib::connect()
{
	nlassert(_PDSConnected);

	nlinfo("PDS is up, connecting...");
	CMessage	msgout("PD_INIT");
	msgout.serial(_DatabaseId);
	msgout.serial(_XmlDescription);
	CUnifiedNetwork::getInstance()->send("PDS", msgout);
}

/*
 * PDS is ready
 */
void	CPDSLib::ready(uint32 lastUpdateId)
{
	nlassert(_PDSConnected);

	nlinfo("PDS is ready to work");
	_PDSReady = true;

	// set up next update Id
	_UpdateId = lastUpdateId+1;

	// flush previous messages that have already been written on hd
	if ((sint)lastUpdateId >= 0)
	{
		flushAcknowledged(lastUpdateId);
	}

	// prepare PD string mapper for sheet ids
	CPDStringMapper	sheetIdStringMapper;

	std::vector<NLMISC::CSheetId>	sheetIds;
	NLMISC::CSheetId::buildIdVector(sheetIds);

	uint	i;
	for (i=0; i<sheetIds.size(); ++i)
	{
		NLMISC::CSheetId	sheetId = sheetIds[i];
		sheetIdStringMapper.setMapping(sheetId.toString(), sheetId.asInt());
	}

	CMessage	msgout("PD_SHEETID_MAPPING");
	msgout.serial(_DatabaseId);
	msgout.serial(sheetIdStringMapper);
	CUnifiedNetwork::getInstance()->send("PDS", msgout);

	// if there are still some messages in queue, resend them
	if (!_QueuedMessages.empty())
	{
		// sort them before, so they are in right order
		// this is in theory useless, since new messages are inserted at end
		std::sort(_QueuedMessages.begin(), _QueuedMessages.end(), CQueuedMessagePred());

		TQueuedMessages::iterator	it;
		for (it=_QueuedMessages.begin(); it!=_QueuedMessages.end(); ++it)
		{
			uint32	updateId = (*it).first;

			if (updateId != _UpdateId)
				nlwarning("CPDSLib::ready(): update id '%d' is not consistent with message id '%d' to be sent", _UpdateId, updateId);

			_UpdateId = updateId+1;

			NLNET::CMessage*	msg = (*it).second;
			CUnifiedNetwork::getInstance()->send("PDS", *msg);
		}
	}
}



/*
 * PDS down callback
 */
void	CPDSLib::onPDSDown(const std::string &serviceName, TServiceId sid, void *arg)
{
	uint	i;
	for (i=0; i<_Libs.size(); ++i)
	{
		if (_Libs[i] != NULL)
		{
			_Libs[i]->_PDSConnected = false;
			_Libs[i]->_PDSReady = false;
			_Libs[i]->disconnect();
		}
	}
}


/*
 * Disconnect service to PDS
 */
void	CPDSLib::disconnect()
{
	nlinfo("PDS disconnected, system should be halted");
}


/*
 * Tells if PDS is ready
 */
bool	CPDSLib::PDSReady()
{
	// check PDS connected, ready and is not too busy
	return (!_UsePDS) || (_PDSReady && _PDSConnected && getMessageQueueSize() < PDMaxMsgQueueSize);
}







/*
 * Update
 * Call regularly this method once a tick to apply changes to database
 */
void	CPDSLib::update()
{
	if (_UsePDS)
	{
		// check pds is ready to receive update
		if (!PDSReady())
			return;

		if (_DbMessageQueue.empty())
			return;

		std::list<CDbMessageQueue>::iterator	it;
		for (it=_DbMessageQueue.begin(); it!=_DbMessageQueue.end(); ++it)
		{
			CDbMessageQueue&	queue = (*it);

			// create a new message
			CMessage*	msgupd = new CMessage("PD_UPDATE");
			msgupd->serial(_DatabaseId);
			msgupd->serial(_UpdateId);
			_QueuedMessages.push_back(make_pair<uint32,CMessage*>(_UpdateId, msgupd));
			++_UpdateId;

			// serial queue
			msgupd->serial(queue);

			// send update to the pds
			CUnifiedNetwork::getInstance()->send("PDS", *msgupd);
		}

		// clean up for next update
		_DbMessageQueue.clear();
	}
	else
	{
		uint	i;

		// save log (updates + string manager)
		TTime	ctime = CTime::getLocalTime();

		//
		CTimestamp	timestamp;
		timestamp.setToCurrent();

		// setup update logs with full timestamp
		if (!_DbMessageQueue.empty())
		{
			i = (uint)_UpdateLogs.size();
			_UpdateLogs.resize(_DbMessageQueue.size());

			for (; i<_UpdateLogs.size(); ++i)
			{
				_UpdateLogs[i].StartStamp = _PreviousTickDate;
				_UpdateLogs[i].EndStamp = _PreviousTickDate;
			}

			_UpdateLogs.back().EndStamp = timestamp;
		}

		_PreviousTickDate = timestamp;

		if (ctime - _PreviousLogSave > PDLogUpdate*1000 && !_DbMessageQueue.empty())
		{
//			std::string	logDir = getRemoteLogDirectory();
//			std::string	logFile = _LogStartDate.toString()+"_0000.pd_log";
//
//			// if don't use BS, create logdir directory tree
//			if (!PDUseBS && !CFile::isDirectory(logDir))
//			{
//				if (!CFile::createDirectoryTree(logDir))
//				{
//					nlwarning("Failed to create log root directory '%s'", logDir.c_str());
//				}
//
//				if (!CFile::setRWAccess(logDir))
//				{
//					nlwarning("Failed, can't set RW access to directory '%s'", logDir.c_str());
//				}
//			}
//
//			// map update logs to msg queues
//			nlassert(_UpdateLogs.size() == _DbMessageQueue.size());
//			for (i=0; i<_UpdateLogs.size(); ++i)
//				_UpdateLogs[i].setUpdates(&(_DbMessageQueue.get(i)));
//
//			if (PDUseBS)
//			{
//				CBackupMsgSaveFile	msgBS( logDir+logFile, CBackupMsgSaveFile::AppendFileCheck, PDBsi );
//				msgBS.DataMsg.serialCont(_UpdateLogs);
//				PDBsi.append(msgBS);
//			}
//			else
//			{
//				COFile				file;
//				if (file.open(logDir + logFile))
//				{
//					file.serialCont(_UpdateLogs);
//				}
//				else
//				{
//					nlwarning("Failed to save log file '%s' for database %d", (logDir + logFile).c_str(), _DatabaseId);
//				}
//			}

//			if (PDEnableStringLog && !_StringManager.logEmpty())
//			{
//				std::string			logFile = timestamp.toString()+".string_log";
//
//				if (PDUseBS)
//				{
//					bool				success = false;
//					CBackupMsgSaveFile	msgBS( logDir+logFile, CBackupMsgSaveFile::SaveFileCheck, PDBsi );
//					{
//						COXml				xml;
//
//						if (xml.init(&msgBS.DataMsg))
//						{
//							_StringManager.storeLog(xml);
//							success = true;
//						}
//						else
//						{
//							nlwarning("Failed to save string log file '%s' for database %d", msgBS.FileName.c_str(), _DatabaseId);
//						}
//					}
//
//					if (success)
//					{
//						PDBsi.sendFile(msgBS);
//					}
//				}
//				else
//				{
//					COFile				file;
//					{
//						COXml				xml;
//						if (file.open(logDir + logFile) && xml.init(&file))
//						{
//							_StringManager.storeLog(xml);
//						}
//						else
//						{
//							nlwarning("Failed to save string log file '%s' for database %d", (logDir+logFile).c_str(), _DatabaseId);
//						}
//					}
//				}
//			}

			_PreviousLogSave = ctime;

			// clean up for next update
			_DbMessageQueue.clear();
			_UpdateLogs.clear();

		}

		if (timestamp.toTime()-_LogStartDate.toTime() > sint(PDLogFileTime.get()))
		{
			_LogStartDate = timestamp;
		}

		_DbMessageQueue.forceNextQueue();
	}
}


/*
 * Register Class Mapping
 */
void	CPDSLib::registerClassMapping(TTableIndex table, const std::string& name)
{
	nlassertex(_ClassMapName.find(name) == _ClassMapName.end(), ("Class Name '%s' already registered!", name.c_str()));
	nlassertex(table >= _ClassMapId.size() || _ClassMapId[table].empty(), ("Class Id '%d' already registered!", table));

	if (_ClassMapId.size() <= table)
		_ClassMapId.resize(table+1);

	_ClassMapName[name] = table;
	_ClassMapId[table] = name;
}

/*
 * Map Class Name
 */
TTableIndex	CPDSLib::mapClassName(const std::string& name)
{
	std::map<std::string, TTableIndex>::iterator	it = _ClassMapName.find(name);
	if (it == _ClassMapName.end())
		return INVALID_TABLE_INDEX;

	return (*it).second;
}

/*
 * Map Class Name
 */
std::string	CPDSLib::mapClassId(TTableIndex table)
{
	if (_ClassMapId.size() <= table)
		return "";

	return _ClassMapId[table];
}






// Allocate a row in the PDS (key is provided for table that are keymapped
void	CPDSLib::allocateRow(TTableIndex table, TRowIndex row, uint64 key)
{
	// send to PDS alloc(table, row, key);
	if (PDVerbose)
		nlinfo("CPDSLib: %12s index=%u:%u, key=%016"NL_I64"X", "allocrow", table, row, key);

	CDbMessage&	msg = nextMessage((uint8)table, row);
	msg.allocRow(key);
}

// Deallocate a row in the PDS
void	CPDSLib::deallocateRow(TTableIndex table, TRowIndex row)
{
	// send to PDS dealloc(table, row)
	if (PDVerbose)
		nlinfo("CPDSLib: %12s index=%u:%u", "deallocrow", table, row);

	CDbMessage&	msg = nextMessage((uint8)table, row);
	msg.deallocRow();
}

// Allocate a row in the PDS (key is provided for table that are keymapped
void	CPDSLib::allocateRow(TTableIndex table, TRowIndex row, uint64 key, const NLMISC::CEntityId& id)
{
	// send to PDS alloc(table, row, key);
	if (PDVerbose)
		nlinfo("CPDSLib: %12s index=%u:%u, key=%016"NL_I64"X", "allocrow", table, row, key);

	CDbMessage&	msg = nextMessage((uint8)table, row);
	msg.allocRow(key, id);
}

// Deallocate a row in the PDS
void	CPDSLib::deallocateRow(TTableIndex table, TRowIndex row, const NLMISC::CEntityId& id)
{
	// send to PDS dealloc(table, row)
	if (PDVerbose)
		nlinfo("CPDSLib: %12s index=%u:%u", "deallocrow", table, row);

	CDbMessage&	msg = nextMessage((uint8)table, row);
	msg.deallocRow(id);
}

// Create an object of a given type
IPDBaseData	*CPDSLib::create(TTableIndex table)
{
	if (PDVerbose)
		nlinfo("CPDSLib: %12s table=%u", "create", table);

	if (table >= _Factories.size() || _Factories[table] == NULL)
	{
		nlwarning("CPDSLib: Unable to create object %d, factory not registered.", table);
		return NULL;
	}

	IPDBaseData	*obj = _Factories[table] ();

	return obj;
}

// Fetch data
void	CPDSLib::fetchPDSData(NLMISC::IStream &f)
{
	TTableIndex		table;
	TRowIndex		row;

	f.serial(table, row);

	IPDBaseData*	obj = create(table);
	if (obj == NULL)
	{
		nlerror("PDS sent an invalid object reference!");
	}

	setRowIndex(row, obj);

	if (table >= _Fetchs.size() || _Fetchs[table] == NULL)
	{
		nlerror("PDS sent an invalid object reference! -- cannot fetch object data");
	}

	TPDFetch		fetchFunc = _Fetchs[table];

	fetchFunc(obj, f);
}

// Notify client of fetch failure
void	CPDSLib::notifyFetchFailure(NLMISC::IStream &f)
{
	TTableIndex		table;
	uint64			key;

	f.serial(table, key);

	if (table >= _FetchFailures.size() || _FetchFailures[table] == NULL)
	{
		nlwarning("CPDSLib: Unable to notify fetch failure of %d:%016"NL_I64"X, callback not set.", table, key);
		return;
	}

	// get notify function
	TPDFetchFailure	failureFunc = _FetchFailures[table];

	// notify user
	failureFunc(key);
}


// Fetch data
void	CPDSLib::setupIndexAllocators(NLMISC::IStream &f)
{
	std::vector<CIndexAllocator>	allocators;

	f.serialCont(allocators);

	uint	i;
	for (i=0; i<allocators.size() && i<_Allocators.size(); ++i)
		if (_Allocators[i] != NULL)
			*(_Allocators[i]) = allocators[i];
}


// Flush acknowledged messages
void	CPDSLib::flushAcknowledged(const std::vector<uint32>& ack)
{
	uint	i;
	for (i=0; i<ack.size(); ++i)
	{
		uint32	msg = ack[i];

		TQueuedMessages::iterator	it;
		for (it=_QueuedMessages.begin(); it!=_QueuedMessages.end() && (*it).first!=msg; )
			;

		if (it != _QueuedMessages.end())
		{
			delete (*it).second;
			it = _QueuedMessages.erase(it);
		}
		else
		{
			++it;
		}
	}
}

// Flush acknowledged messages until message
void	CPDSLib::flushAcknowledged(uint32 ack)
{
	// remove all updates before or equal to the acknowledged message
	TQueuedMessages::iterator	it;
	for (it=_QueuedMessages.begin(); it!=_QueuedMessages.end(); )
	{
		uint32	updateId = (*it).first;
		if (updateId <= ack)
		{
			delete (*it).second;
			it = _QueuedMessages.erase(it);
		}
		else
		{
			++it;
		}
	}
}


/*
 * Get Message Queue Size
 */
uint	CPDSLib::getMessageQueueSize()
{
	uint	size = 0;
	uint	i;
	for (i=0; i<_QueuedMessages.size(); ++i)
		size += _QueuedMessages[i].second->length();

	return size;
}



// Erase an object from its table and key
void	CPDSLib::erase(TTableIndex table, uint64 key)
{
	if (PDVerbose)
		nlinfo("CPDSLib: %12s table=%u, key=%016"NL_I64"X", "erase", table, key);
}

// Load a row and its dependent rows from a mapped table
void	CPDSLib::load(TTableIndex table, uint64 key)
{
	if (PDVerbose)
		nlinfo("CPDSLib: %12s table=%u, key=%016"NL_I64"X", "load", table, key);

	if (!_UsePDS)
	{
		if (table >= _FetchFailures.size() || _FetchFailures[table] == NULL)
			return;

		// get notify function
		TPDFetchFailure	failureFunc = _FetchFailures[table];

		// notify user
		failureFunc(key);
	}

	CDbMessage&	msg = nextMessage();
	msg.loadRow(table, key);
}

// Release a row
void	CPDSLib::release(TTableIndex table, TRowIndex row)
{
	if (PDVerbose)
		nlinfo("CPDSLib: %12s table=%u, row=%u", "release", table, row);

	CDbMessage&	msg = nextMessage((uint8)table, row);
	msg.releaseRow();
}



// Add a string in pds
void	CPDSLib::addString(const NLMISC::CEntityId& eid, const ucstring& str)
{
	CDbMessage&	msg = nextMessage();
	msg.addString(eid.asUint64(), str);
}

// Unmap a string in pds
void	CPDSLib::unmapString(const NLMISC::CEntityId& eid)
{
	CDbMessage&	msg = nextMessage();
	msg.unmapString(eid.asUint64());
}




// Get Database Root directory
std::string	CPDSLib::getPDSRootDirectory(const std::string& shard, bool wantRemoteDir)
{
	std::string	dir;

	if (PDRootDirectory.get() == "")
	{
		if (wantRemoteDir)
			dir = "pds/";
		else
		{
			// Because we have removed the PD Bsi, but to keep the log analizer wroking,
			// we fallback to the path set in the per shard BS because they are
			// init with the same value
			dir = Bsi.getLocalPath() + "pds/";
			//dir = PDBsi.getLocalPath() + "pds/";
		}
	}
	else
	{
		dir = CPath::standardizePath(PDRootDirectory);
	}

	std::string	findstr("$shard");
	std::string::size_type p = dir.find(findstr);

	if (p != std::string::npos)
		dir.replace(p, findstr.size(), shard);

	return dir;
}

// Get Database Root directory
std::string	CPDSLib::getRootDirectory(uint databaseId, const std::string& shard, bool wantRemoteDir)
{
	std::string	dir = getPDSRootDirectory(shard, wantRemoteDir);
	return NLMISC::toString("%s%08X/", dir.c_str(), databaseId);
}


// Get Logging directory
std::string	CPDSLib::getLogDirectory(uint databaseId, const std::string& shard)
{
	return getRootDirectory(databaseId, shard) + "logs/";
}

// Get Lib Logging directory
//std::string	CPDSLib::getLogDirectory(const std::string& shard) const
//{
//	return getLogDirectory(_DatabaseId, shard);
//}

// Get Logging directory
std::string	CPDSLib::getRemoteLogDirectory(uint databaseId, const std::string& shard)
{
	return getRootDirectory(databaseId, shard, true) + "logs/";
}

// Get Lib Logging directory
std::string	CPDSLib::getRemoteLogDirectory(const std::string& shard) const
{
	return getRemoteLogDirectory(_DatabaseId, shard);
}









NLMISC_DYNVARIABLE(uint, CurrentPDQueueSize, "Gives current size of PDS messages to be send")
{
	// read or write the variable
	if (get)
	{
		uint	i;
		uint	size = 0;
		for (i=0; i<CPDSLib::_Libs.size(); ++i)
			if (CPDSLib::_Libs[i] != NULL)
				size += CPDSLib::_Libs[i]->getMessageQueueSize();
		*pointer = size;
	}
}

NLMISC_DYNVARIABLE(uint, EnqueuedPDMessages, "Tells number of messages enqueued and not yet sent")
{
	// read or write the variable
	if (get)
	{
		uint	i;
		uint	total = 0;
		for (i=0; i<CPDSLib::_Libs.size(); ++i)
			if (CPDSLib::_Libs[i] != NULL)
				total += CPDSLib::_Libs[i]->enqueuedMessages();
		*pointer = total;
	}
}


//
//NLMISC_COMMAND(displayPDLogContent, "display pd_log file human readable content", "<databaseId> <filename>")
//{
//	if (args.size() != 2)
//		return false;
//
//	uint	databaseId;
//	NLMISC::fromString(args[0], databaseId);
//	string	filename = args[1];
//
//	CPDSLib*	lib = (databaseId < CPDSLib::_Libs.size() ? CPDSLib::_Libs[databaseId] : NULL);
//
//	if (lib == NULL)
//		return false;
//
//	const std::string&		xml = lib->getXmlDescription();
//	CDBDescriptionParser	desc;
//
//	if (!desc.loadDescription((const uint8*)(xml.c_str())))
//	{
//		nlwarning("Failed to load database %d description", databaseId);
//		return false;
//	}
//
//	if (!desc.buildColumns())
//	{
//		nlwarning("Failed to build database %d columns for description", databaseId);
//		return false;
//	}
//
//	CIFile		ifile;
//	CUpdateLog	updateLog;
//
//	if (!ifile.open(filename))
//	{
//		nlwarning("Failed to open file '%s'", filename.c_str());
//		return false;
//	}
//
//	try
//	{
//		updateLog.serial(ifile);
//	}
//	catch (const Exception& e)
//	{
//		nlwarning("Failed to load file '%s': %s", filename.c_str(), e.what());
//		return false;
//	}
//
//	updateLog.display(desc, log);
//
//	return true;
//}

} // RY_PDS
