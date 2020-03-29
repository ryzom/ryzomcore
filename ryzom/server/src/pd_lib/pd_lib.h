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

#ifndef RY_PDS_LIB_H
#define RY_PDS_LIB_H

/*
 * Includes
 */
#include "pd_utils.h"
//#include "pd_string_manager.h"
#include "pd_messages.h"

#include "nel/misc/variable.h"
#include "nel/net/message.h"
#include "nel/net/unified_network.h"

namespace RY_PDS
{

extern NLMISC::CVariable<bool>			PDEnableLog;
//extern NLMISC::CVariable<bool>			PDEnableStringLog;

/**
 * Interface library to the Persistant Data Service (PDS)
 * This lib is intended to make the link between game server and data server.
 * All methods are for internal use, you should always use generated API to
 * handle Persistant Data System.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CPDSLib
{

public:


	/**
	 * Constructor
	 */
	CPDSLib();



	/**
	 * Tells lib PDS is to be used.
	 * Transitionnal method to be called at service startup, as long as PDS
	 * is not definitely used.
	 */
	void						usePDS();

	/**
	 * Tells if PDS is used
	 */
	bool						PDSUsed() const		{ return _UsePDS; }



	/// \name Internal interface
	// @{

	/**
	 * Those function should not be called directly.
	 * To create, load, delete or any object manipulation, YOU must
	 * refer to the generated API.
	 */

	/**
	 * Tells if PDS is ready
	 */
	bool						PDSReady();

	/**
	 * The String Manager embedded in CPDS lib
	 */
//	CPDStringManager&			getStringManager()	{ return _StringManager; }

	/**
	 * Update
	 * Call regularly this method once a tick to apply changes to database
	 */
	void						update();

	/**
	 * Get Message Queue Size
	 */
	uint						getMessageQueueSize();

	/**
	 * Map Class Name
	 */
	TTableIndex					mapClassName(const std::string& name);

	/**
	 * Map Class Name
	 */
	std::string					mapClassId(TTableIndex table);

	/**
	 * Get Class Name
	 */
	std::string					getClassName(const IPDBaseData* obj)
	{
		return mapClassId(obj->getTable());
	}

	/**
	 * Register Class Mapping
	 */
	void						registerClassMapping(TTableIndex table, const std::string& name);

	/**
	 * Internal type checking
	 */
	static void					checkInternalTypes();

	/// Init PDS lib, client side only
	void						init(const std::string &xml, uint32 overrideDbId = 0);

	/// Init PDS log
	void						initLog(uint logmsg);

	/// Init PDS log parameter
	void						initLogParam(uint logmsg, uint logparam, uint byteSize);

	/// Connect service to PDS when up
	void						connect();

	/// Disconnect service to PDS
	void						disconnect();

	/// Release PDSLib
	void						release();

	/// PDS is ready
	void						ready(uint32 lastUpdateId);

	/// Register a class with its factory and its fetch
	void						registerClass(TTableIndex table, TPDFactory factory, TPDFetch fetch, TPDFetchFailure fetchFailure)
	{
		if (_Factories.size() <= table)
			_Factories.resize(table+1, NULL);
		if (_Fetchs.size() <= table)
			_Fetchs.resize(table+1, NULL);
		if (_FetchFailures.size() <= table)
			_FetchFailures.resize(table+1, NULL);

		_Factories[table] = factory;
		_Fetchs[table] = fetch;
		_FetchFailures[table] = fetchFailure;
	}

	/// Set index allocator
	void						setIndexAllocator(TTableIndex table, CIndexAllocator& alloc)
	{
		if (_Allocators.size() <= table)
			_Allocators.resize(table+1, NULL);

			_Allocators[table] = &alloc;
	}


	/// Allocate a row in the PDS (key is provided for table that are keymapped
	void						allocateRow(TTableIndex table, TRowIndex row, uint64 key);

	/// Deallocate a row in the PDS
	void						deallocateRow(TTableIndex table, TRowIndex row);

	/// Allocate a row in the PDS (key is provided for table that are keymapped
	void						allocateRow(TTableIndex table, TRowIndex row, uint64 key, const NLMISC::CEntityId& id);

	/// Deallocate a row in the PDS
	void						deallocateRow(TTableIndex table, TRowIndex row, const NLMISC::CEntityId& id);

	/// Create an object of a given type
	IPDBaseData*				create(TTableIndex table);

	/// Create an object from a given class name, assuming it is mapped
	IPDBaseData*				create(const std::string& name)
	{
		TTableIndex	table = mapClassName(name);
		return (table == INVALID_TABLE_INDEX) ? NULL : create(table);
	}

	/// Release a row
	void						release(TTableIndex table, TRowIndex row);

	/// Erase an object from its table and key
	void						erase(TTableIndex table, uint64 key);

	/// Load a row and its dependent rows from a mapped table
	void						load(TTableIndex table, uint64 key);

	/// Add a string in pds
	void						addString(const NLMISC::CEntityId& eid, const ucstring& str);

	/// Unmap a string in pds
	void						unmapString(const NLMISC::CEntityId& eid);


	/// \name Set methods
	// @{

	template<typename T>
	void						set(TTableIndex table, TRowIndex row, TColumnIndex column, const T& value)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s index=%u:%u, column=%u, value='%s'", "set", table, row, column, pdsToString(value).c_str());

		CDbMessage&	msg = nextMessage((uint8)table, row);
		msg.updateValue(column, value);
	}

	template<typename T>
	void						set(TTableIndex table, TRowIndex row, TColumnIndex column, const T& value, const NLMISC::CEntityId& objectId)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s index=%u:%u, column=%u, value='%s' %s", "set", table, row, column, pdsToString(value).c_str(), objectId.toString().c_str());

		CDbMessage&	msg = nextMessage((uint8)table, row);
		msg.updateValue(column, value, objectId);
	}

	void						setParent(TTableIndex table, TRowIndex row, TColumnIndex column, const CObjectIndex& parent)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s index=%u:%u, column=%u, value='%s'", "setParent", table, row, column, parent.toString().c_str());

		CDbMessage&	msg = nextMessage((uint8)table, row);
		msg.setParent(column, parent);
	}

	void						setParent(TTableIndex table, TRowIndex row, TColumnIndex column, const CObjectIndex& parent, const NLMISC::CEntityId& objectId)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s index=%u:%u, column=%u, value='%s' %s", "setParent", table, row, column, parent.toString().c_str(), objectId.toString().c_str());

		CDbMessage&	msg = nextMessage((uint8)table, row);
		msg.setParent(column, parent, objectId);
	}

	void						setParent(TTableIndex table, TRowIndex row, TColumnIndex column, const CObjectIndex& parent, const NLMISC::CEntityId& newParentId, const NLMISC::CEntityId& previousParentId)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s index=%u:%u, column=%u, value='%s' newParent=%s previousParent=%s", "setParent", table, row, column, parent.toString().c_str(), newParentId.toString().c_str(), previousParentId.toString().c_str());

		CDbMessage&	msg = nextMessage((uint8)table, row);
		msg.setParent(column, parent, newParentId, previousParentId);
	}

	void						setParent(TTableIndex table, TRowIndex row, TColumnIndex column, const CObjectIndex& parent, const NLMISC::CEntityId& objectId, const NLMISC::CEntityId& newParentId, const NLMISC::CEntityId& previousParentId)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s index=%u:%u, column=%u, value='%s' %s newParent=%s previousParent=%s", "setParent", table, row, column, parent.toString().c_str(), objectId.toString().c_str(), newParentId.toString().c_str(), previousParentId.toString().c_str());

		CDbMessage&	msg = nextMessage((uint8)table, row);
		msg.setParent(column, parent, objectId, newParentId, previousParentId);
	}

	// @}

	/// \name LogPush methods
	// @{

	/// Log
	void						log(uint logId)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s %d", "log", logId);

		nlassert(logId < _LogDescs.size());
		CLogDesc&	desc = _LogDescs[logId];

		_CurrentLogMessage = logId;
		_CurrentLogParam = 0;
		CDbMessage&	msg = nextMessage();
		msg.log(logId, desc.ByteSize);
	}

	/// Start log context
	void						pushContext()
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s", "pushContext");

		CDbMessage&	msg = nextMessage();
		msg.pushContext();
	}

	/// Stop log context
	void						popContext()
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s", "popContext");

		CDbMessage&	msg = nextMessage();
		msg.popContext();
	}

	/// Push a parameter for the log
	template<typename T>
	void						logPush(const T& value)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s value='%s'", "push", pdsToString(value).c_str());

		nlassert(_CurrentLogMessage < _LogDescs.size());
		CLogDesc&	desc = _LogDescs[_CurrentLogMessage];
		CLogParam&	param = desc.Params[_CurrentLogParam++];

		CDbMessage&	msg = currentMessage();
		nlassert(msg.getType() == CDbMessage::Log);
		nlassert(sizeof(value) == param.ByteSize);
		msg.pushParameter(param.ByteOffset, value);

	}

	/// Push a string parameter for the log
	void						logPush(const std::string& value)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s value='%s'", "push", pdsToString(value).c_str());

		nlassert(_CurrentLogMessage < _LogDescs.size());
		CLogDesc&	desc = _LogDescs[_CurrentLogMessage];
		CLogParam&	param = desc.Params[_CurrentLogParam++];

		CDbMessage&	msg = currentMessage();
		nlassert(msg.getType() == CDbMessage::Log);
		nlassert(sizeof(uint16) == param.ByteSize);
		msg.pushParameter(param.ByteOffset, value);
	}

	// @}

	/// \name LogChat methods
	// @{

	/// Log Chat sentence
	void						logChat(const ucstring& sentence, const NLMISC::CEntityId& sender, const std::vector<NLMISC::CEntityId>& receivers)
	{
		if (PDVerbose)
			nlinfo("CPDSLib: %12s", "popContext");

		CDbMessage&	msg = nextMessage();
		msg.logChat(sentence, sender, receivers);
	}

	// @}


	/// Fetch data
	void						fetchPDSData(NLMISC::IStream &f);

	/// Notify client of fetch failure
	void						notifyFetchFailure(NLMISC::IStream &f);

	/// Setup index allocators
	void						setupIndexAllocators(NLMISC::IStream &f);

	/// Init object row index
	void						setRowIndex(TRowIndex row, IPDBaseData* obj)		{ obj->__BaseRow = row;	}

	/// Flush acknowledged messages
	void						flushAcknowledged(const std::vector<uint32>& ack);

	/// Flush acknowledged messages until message
	void						flushAcknowledged(uint32 ack);


	/// Init String Manager
//	void						initStringManager()					{ _StringManagerInitialised = true; }
	/// Init String Manager
	void						initAllocs()						{ _AllocsInitialised = true; }
	/// Init String Manager
//	bool						stringManagerInitialised() const	{ return _StringManagerInitialised; }
	/// Init String Manager
	bool						allocsInitialised() const			{ return _AllocsInitialised; }


	static CPDSLib*				getLib(uint32 libId)	{ return (_Libs.size() <= libId) ? NULL : _Libs[libId]; }

	static std::vector<CPDSLib*>		_Libs;

	/// Get Database Id
	uint32						getDatabaseId() const				{ return _DatabaseId; }

	/// Get Database Root directory. If wantRemoteDir is true, only a relative path will be returned if PDRootDirectory is empty.
	static std::string			getPDSRootDirectory(const std::string& shard = "", bool wantRemoteDir=false);

	/// Get Database Root directory. If wantRemoteDir is true, only a relative path will be returned if PDRootDirectory is empty.
	static std::string			getRootDirectory(uint databaseId, const std::string& shard = "", bool wantRemoteDir=false);

	/// Get Logging directory
	static std::string			getLogDirectory(uint databaseId, const std::string& shard = "");

	/// Get Lib Logging directory
//	std::string					getLogDirectory(const std::string& shard = "") const;

	/// Get Logging directory for remote access through BS interface
	static std::string			getRemoteLogDirectory(uint databaseId, const std::string& shard = "");

//	/// Get Lib Logging directory for remote access through BS interface
	std::string					getRemoteLogDirectory(const std::string& shard = "") const;

	/// Get Number of Enqueued Messages
	uint32						enqueuedMessages() const			{ return _DbMessageQueue.getNumMessagesEnqueued(); }



	/// Get Xml Description
	const std::string&			getXmlDescription() const			{ return _XmlDescription; }

private:

	std::vector<TPDFactory>				_Factories;
	std::vector<TPDFetch>				_Fetchs;
	std::vector<TPDFetchFailure>		_FetchFailures;
	std::vector<CIndexAllocator*>		_Allocators;
//	CPDStringManager					_StringManager;

	std::string							_XmlDescription;

	bool								_PDSConnected;
	bool								_PDSReady;

	typedef std::pair<uint32, NLNET::CMessage*>	TQueuedMessage;
	typedef std::deque<TQueuedMessage>	TQueuedMessages;

	class CQueuedMessagePred
	{
	public:

		bool	operator () (const TQueuedMessage& a, const TQueuedMessage& b) const
		{
			return a.first < b.first;
		}

	};

	TQueuedMessages						_QueuedMessages;
	CDbMessageSplitQueue				_DbMessageQueue;
	std::vector<CUpdateLog>				_UpdateLogs;

	uint32								_DatabaseId;

	uint32								_UpdateId;


	bool								_UsePDS;
	NLMISC::TTime						_PreviousLogSave;
	CTimestamp							_LogStartDate;
	CTimestamp							_PreviousTickDate;


	bool								_AllocsInitialised;
//	bool								_StringManagerInitialised;

	struct CLogParam
	{
		uint							ByteOffset;
		uint							ByteSize;
	};

	class CLogDesc
	{
	public:

		CLogDesc() : Log(0xffffffff), ByteSize(0)	{ }

		uint					Log;
		uint					ByteSize;
		std::vector<CLogParam>	Params;

	};

	uint								_CurrentLogMessage;
	uint								_CurrentLogParam;

	std::map<std::string, TTableIndex>	_ClassMapName;
	std::vector<std::string>			_ClassMapId;

	/// Logs Descriptions
	std::vector<CLogDesc>				_LogDescs;

	/// Fake message to be used if log is not enabled
	CDbMessage							_FakeMessage;

	/// Get next message
	CDbMessage&							nextMessage()
	{
		return PDEnableLog ? _DbMessageQueue.nextMessage() : _FakeMessage;
	}
	CDbMessage&							nextMessage(uint8 table, uint32 row)
	{
		return PDEnableLog ? _DbMessageQueue.nextMessage(table, row) : _FakeMessage;
	}

	/// Get Current message
	CDbMessage&							currentMessage()							{ return _DbMessageQueue.currentMessage(); }

	/// PDS up callback
	static void							onPDSUp(const std::string &serviceName, NLNET::TServiceId sid, void *arg);

	/// PDS down callback
	static void							onPDSDown(const std::string &serviceName, NLNET::TServiceId sid, void *arg);
};


}; // RY_PDS

#endif //RY_PDS_LIB_H



