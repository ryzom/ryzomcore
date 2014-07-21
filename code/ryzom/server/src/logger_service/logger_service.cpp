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

#include "nel/misc/types_nl.h"
#include <list>
#include <time.h>
#include "nel/misc/time_nl.h"
#include "nel/misc/thread.h"
#include "nel/misc/random.h"
#include "nel/misc/singleton.h"
#include "nel/net/service.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "server_share/logger_service_itf.h"
#include "game_share/inventories.h"
#include "game_share/backup_service_interface.h"
#include "game_share/singleton_registry.h"
#include "game_share/shard_names.h"

#include "logger_service.h"
#include "log_query.h"
#include "log_storage.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace INVENTORIES;
using namespace LGS;


CVariable<string> LQLState("lgs", "LQLState", "The current Query state", "");
CVariable<uint32> LastFinishedQuery("lgs", "LastFinishedQuery", "The number of the last finished request", 0);
CVariable<string> LogQueryResultFile("lgs", "LogQueryResultFile", "The file used to output the query result", "log_query_result.txt");


extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}



uint32 randu32(CRandom &rand, uint32 mod)
{
	uint32 ret = (((uint32)rand.rand(0xffff))<<16)+rand.rand(0xffff);

	return ret % mod;
}

sint32 rands32(CRandom &rand, sint32 mod)
{
	sint32 ret = (((sint32)rand.rand(0xffff))<<16)+rand.rand(0xffff);

	return ret % mod;
}


uint64 randu64(CRandom &rand, uint64 mod)
{
	uint64 ret = (((uint64)randu32(rand, 0xffffffff))<<32)+randu32(rand, 0xffffffff);

	return ret % mod;
}


/** An safe inter-thread communication channel.
 *  Allow safe transmission of object of type T 
 *	from one thread to another (need 2 channel
 *	for a bi-di operation).
 */
template <class T>
class CThreadChannel
{
	/// the object ready to be transfered to the receiver thread
	list<T>		_Datas;

	/// The mutex that protect the object.
	NLMISC::CMutex	_Mutex;

	/// Wait until a data is available or channel empty, 
	/// NB : The is Aquired on exit to be sure that the wait will lock the
	/// available data.
	/// NB : the mutex MUST be aquired before calling wait
	void wait(bool empty)
	{
		// NB : this is a very dirty implementation using a waiting loop.
		// A correct one should use win32 event or unix semaphore for
		// efficiency.

		/// Loop until there is some data to read or no more date
		do 
		{
			_Mutex.leave();
			// begin by a sleep
			nlSleep(100);

			_Mutex.enter();
		} while(_Datas.empty() != empty);

		// exit with mutex acquired
	}

public:

	/// Write data to the channel
	void write(const T &t)
	{
		CAutoMutex<CMutex> mutex(_Mutex);

		_Datas.push_back(t);
	}

	/// Write data to the channel and wait until the value
	/// is read
	void syncWrite(const T &t)
	{
		_Mutex.enter();

		_Datas.push_back(t);

		// loop and wait until the lost is empty, i.e the data just
		// pushed back has been read by another thread.
		wait(true);

		_Mutex.leave();
	}

	/// Read a data. Block if no data available
	T read()
	{
		_Mutex.enter();
		if (_Datas.empty())
		{
			// wait for an object to be available
			wait(false);

			// NB : the mutex is entered by the wait method

		}

		T t = _Datas.front();
		_Datas.pop_front();

		_Mutex.leave();

		return t;
	}

	/// Return true if the channel is empty, i.e there is no data to read
	bool empty()
	{
		CAutoMutex<CMutex> lock(_Mutex);

		return _Datas.empty();
	}

	/// Peek a reference on a data. Block if no data available
	const T &peek()
	{
		_Mutex.enter();
		if (_Datas.empty())
		{
			// wait for an object to be available
			wait(false);

			// NB : the mutex is entered by the wait method

		}

		const T &t = _Datas.front();

		_Mutex.leave();

		return t;
	}

	/// Try to read a data. 
	/// Return true if a data have been read, false otherwise
	bool read(T &result)
	{
		CAutoMutex<CMutex> lock(_Mutex);
		if (_Datas.empty())
		{
			// no data available
			return false;
		}

		result = _Datas.front();
		_Datas.pop_front();

		return true;
	}

	/// Try to read a data if a data is available AND if the predicate is valid.
	/// Return true if a data have been read, false otherwise
	template <class predicate>
	bool read(T &result, const predicate &pred)
	{
		CAutoMutex<CMutex> lock(_Mutex);
		if (_Datas.empty())
		{
			// no data available
			return false;
		}

		if (!pred(_Datas.front()))
		{
			// the predicate is not ok
			return false;
		}

		result = _Datas.front();
		_Datas.pop_front();

		return true;
	}

};



// the logger module
class CLoggerServiceMod 
	:	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
		public LGS::CLoggerServiceSkel
{

	typedef map<TModuleProxyPtr, TShardId>	TLogClients;
	// A list of logger client
	TLogClients		_Clients;

	// The log definitions
	TLogDefinitions				_LogDefs;

	typedef map<std::string, size_t>	TLogDefLindex;
	// the log def index
	TLogDefLindex				_LogDefIndex;

	/// the log serial number
	uint32						_LogId;

	/// storage for the log entries
	TLogInfos					_LogInfos;
	/// 'manual' count of log entry (because list<>.size() id not constant time)
	size_t						_LogInfosSize;

	/// A mutex to protect the access to the log info when the query thread read
	/// the log in memory
	CMutex						_LogMutex;

	/// date of last 'minute' log output
	uint32						_LastMinuteOutput;
	/// date of last 'hourly' consolidation
	uint32						_LastHourlyProcess;

	/// Line counter of current request being written to backup service
	uint32						_WriteLineCounter;
	/// Query number, used to number each query sent to the worker thread.
	uint32						_LastQueryNumber;


	enum
	{
		MinuteDelay = 60,
		HourlyDelay = 60*60,
	};

	enum TQueryCommand
	{
		/// execute a query
		qc_query,
		/// interrupt the current query
		qc_interupt,
		/// Terminate the query thread
		qc_terminate
	};
	struct TThreadCommand
	{
		TQueryCommand	QueryCommand;
		uint32			QueryNumber;
		string			QueryString;
	};

	struct TQueryInterupPred
	{
		bool operator () (const TThreadCommand &command) const
		{
			return command.QueryCommand == qc_interupt;
		}
	};


	friend struct TQueryInterupPred;

	enum TQueryStatus
	{
		/// Normal status report, put in the log output
		qs_log,
		/// push a service state string
		qs_push_state,
		/// pop a service state string
		qs_pop_state,
		/// Set the LQL state variable
		qs_lql_state,

		invalid
	};

	struct TThreadStatus
	{
		volatile TQueryStatus	ThreadStatus;
		string			StatusString;

		TThreadStatus(TQueryStatus	threadStatus, const string &statusString)
			:	ThreadStatus(threadStatus),
				StatusString(statusString)
		{}
	};

	struct TThreadResult
	{
		uint32				QueryNumber;
		string				OutputFilename;
		std::list<string>	*Lines;
	};

	/// A thread channel used by the service to control the query thread
	CThreadChannel<TThreadCommand>	_QueryCommands;
	/// A thread channel used by the query thread to report status string
	CThreadChannel<TThreadStatus>	_QueryStatus;
	/// A thread channel used by the query thread to report query result to be 
	/// stored on disk by the main thread using the backup service interface.
	CThreadChannel<TThreadResult>	_QueryResult;


	/// the thread used to execute the query
	class CQueryThread : public NLMISC::IRunnable
	{
	public:
		CLoggerServiceMod *_LoggerService;

		CQueryThread(CLoggerServiceMod *loggerService)
			:	_LoggerService(loggerService)
		{
		}

		virtual void getName (std::string &result) const
		{
			result = "QueryThread";
		}

		virtual void run()
		{
			while (true)
			{
				TThreadCommand tc = _LoggerService->_QueryCommands.read();

				if (tc.QueryCommand == qc_terminate)
					// terminate the thread
					return;

				if (tc.QueryCommand == qc_query)
				{
					// execute a query
					_LoggerService->executeQuery(tc.QueryString, tc.QueryNumber);
				}
			}
		}
	};

	friend class CLoggerServiceMod::CQueryThread;
	/// the query thread
	IThread				*_QueryThread;



public:
	CLoggerServiceMod()
		:	_LogInfosSize(0),
			_LogId(0),
			_LastMinuteOutput(0),
			_LastHourlyProcess(0),
			_LastQueryNumber(0),
			_WriteLineCounter(0),
			_QueryThread(NULL)
	{
		CLoggerServiceSkel::init(this);
	}

	~CLoggerServiceMod()
	{
		// stop the query thread
		TThreadCommand tc;
		tc.QueryCommand = qc_interupt;
		_QueryCommands.write(tc);

		tc.QueryCommand = qc_terminate;
		_QueryCommands.write(tc);

		// wait thread termination
		_QueryThread->wait();
		// consolidate the last logs
		doHourlyProcess();
	}

	bool initModule(const TParsedCommandLine &initInfo)
	{
		CModuleBase::initModule(initInfo);
		// Check and eventually process the hourly output of the preceding minute run
		consolidatePreviousFiles();

		// start the query thread
		_QueryThread = IThread::create(new CQueryThread(this));
		_QueryThread->start();

		return true;
	}
	
	void onModuleDown(IModuleProxy *moduleProxy)
	{
		// check if this is one of our client
		TLogClients::iterator it(_Clients.find(moduleProxy));

		if (it != _Clients.end())
		{
			nlinfo("Logger client '%s' disconnected'", moduleProxy->getModuleName().c_str());

			_Clients.erase(it);
		}
	}

	void onModuleUpdate()
	{
		uint32 now = CTime::getSecondsSince1970();

		if (now > _LastHourlyProcess+HourlyDelay)
		{
			// do the hourly process

			_LogMutex.enter();
			doHourlyProcess();
			_LogMutex.leave();
			
			// update working time
			_LastHourlyProcess = now;
			_LastMinuteOutput = now;
		}
		if (now > _LastMinuteOutput+MinuteDelay)
		{
			// do the minute output

			_LogMutex.enter();
			CLogStorage ls(_LogDefs);
			ls.saveMinutely(_LogInfos);
			_LogMutex.leave();

			_LastMinuteOutput = now;
		}

		// check status from query thread
		TThreadStatus status(invalid, string());
		while (_QueryStatus.read(status))
		{
			switch (status.ThreadStatus)
			{
			case qs_log:
				nlinfo("Request Status : %s", status.StatusString.c_str());
				break;
			case qs_push_state:
				IService::getInstance()->setCurrentStatus(status.StatusString);
				break;
			case qs_pop_state:
				IService::getInstance()->clearCurrentStatus(status.StatusString);
				break;
			case qs_lql_state:
				LQLState = status.StatusString;
				break;
			}
		}

		// check the thread result
		if (!_QueryResult.empty())
		{

			// NB : the query result is stored in chunk of 10K lines at a time
			// to reduce packet size to the backup service.
			// counter is used to track the state of the current result writing
			// between module update.

			// the query thread has posted a result, send it to the BS
			const TThreadResult &threadResult = _QueryResult.peek();

			uint32 nbLog = (uint32)threadResult.Lines->size();

			if( _WriteLineCounter != nbLog)
			{
				bool firstBlock = _WriteLineCounter == 0;

				CBackupMsgSaveFile saveFile(BsiGlobal.getRemotePath()+"/"+threadResult.OutputFilename, 
											firstBlock ? CBackupMsgSaveFile::SaveFile : CBackupMsgSaveFile::AppendFile ,
											BsiGlobal);
				saveFile.FileName = threadResult.OutputFilename;

				const char *newLine="\n";

				list<string>::const_iterator first(threadResult.Lines->begin()), last(threadResult.Lines->end());
				for (uint32 localCounter = 0; first != last; ++first, ++localCounter)
				{
					if (localCounter < _WriteLineCounter)
					{
						// skip the first line until we reach the last already saved line
						continue;
					}

					saveFile.DataMsg.serialBuffer((uint8*)&(*first->begin()), (uint)first->size());
					saveFile.DataMsg.serialBuffer((uint8*)newLine, 1);

					++_WriteLineCounter;

					if (saveFile.DataMsg.length() > CBufNetBase::DefaultMaxSentBlockSize/2)
					{
						// segment the output soo that it never exceed the maximum message size allowed
						break;
					}
				}

				LQLState = toString("Writing result logs %u/%u...", _WriteLineCounter+1, nbLog);


				// send the file to the backup service
				if (firstBlock)
				{
					// this is the first part of the file
					BsiGlobal.sendFile(saveFile);
				}
				else
				{
					BsiGlobal.append(saveFile);
				}
			}
			else
			{
				// The last write has been completed during the previous update,
				/// mark the request as terminated.
				// we have finished to write the result
				_WriteLineCounter = 0;

				LastFinishedQuery = threadResult.QueryNumber;

				// consume the data in the channel to unlock the query thread
				_QueryResult.read();
			}
		}
	}

	void doHourlyProcess()
	{
		// 1- generate the hourly output
		CLogStorage ls(_LogDefs);
		ls.saveHourly(_LogInfos);
		// 2- remove minute outputs
		std::string logRoot = ls.getLogRoot();
		vector<string> files;
		CPath::getPathContent(logRoot, false, false, true, files, NULL, true);
		
		for (uint i=0; i<files.size(); ++i)
		{
			if (files[i].find("minutely") != string::npos)
			{
				// ok, this is a minute output, remove it
				CFile::deleteFile(files[i]);
			}
		}
		// 3- cleanup logs from memory
		_LogInfos.clear();
	}


	/// Rebuild the last 'hourly' from residual minutely files found on disk
	void consolidatePreviousFiles()
	{
		CLogStorage ls;

		std::string logRoot = ls.getLogRoot();
		vector<string> files;
		CPath::getPathContent(logRoot, false, false, true, files, NULL, true);
		
		for (uint i=0; i<files.size(); ++i)
		{
			if (files[i].find("minutely") != string::npos && files[i].find(".binlog") == files[i].size()-7)
			{
				ls.loadLogs(files[i]);
			}
		}

		// save all logs in one file
		ls.saveLogfile("hourly_");

		// delete minute files (even those .tmp file left by failed copy)
		for (uint i=0; i<files.size(); ++i)
		{
			if (files[i].find("minutely") != string::npos)
			{
				CFile::deleteFile(files[i]);
			}
		}
	}


	///////////////////////////////////////////////////////////////////////////
	/////    CLoggetServiceSkel implementation    /////////////////////////////
	///////////////////////////////////////////////////////////////////////////


	// A logger client register itself wy providing it's definition of 
	// the log content. It is mandatory that ALL client share
	// Exactly the same definition of log.
	virtual void registerClient(NLNET::IModuleProxy *sender, uint32 shardId, const std::vector < TLogDefinition > &logDef)
	{
		CAutoMutex<CMutex> lock(_LogMutex);
		// check that the client use the correct log format
		if (_LogDefs.empty() || _Clients.empty())
		{
			// clear any log still in memory
			// TODO : finish close the current log file
			_LogInfos.clear();
			_LogInfosSize = 0;


			// this is the first client, or no other client connected, it define the format
			_LogDefs = logDef;

			// init the index
			_LogDefIndex.clear();
			for (uint i=0; i<_LogDefs.size(); ++i)
			{
				_LogDefIndex.insert(make_pair(_LogDefs[i].getLogName(), i));
			}
		}
		else
		{
			if (_LogDefs != logDef)
			{

				// this client use a different format, reject him
				nlwarning("Client %s from shard %u connect with a different log format!",
					sender->getModuleName().c_str(),
					shardId);

				// make the information visible to admins
				IService::getInstance()->addStatusTag("INVALID_CLIENT");

				return;
			}
		}

		// ok, store it in the client base
		_Clients.insert(make_pair(sender, shardId));
	}

	// A client send a log
	virtual void reportLog(NLNET::IModuleProxy *sender, const std::vector < TLogInfo > &logInfos)
	{
		CAutoMutex<CMutex> lock(_LogMutex);
		// 1st check that the client is allowed
		TLogClients::iterator it = _Clients.find(sender);
		if (it == _Clients.end())
		{
			// ignoring log of unidentified or rejected client
			nlinfo("Ignoring log from 's'", sender->getModuleName().c_str());
			return;
		}

		// for each log received 
		for (uint l=0; l<logInfos.size(); ++l)
		{
			const TLogInfo &logInfo = logInfos[l];
			// 2nd find the log desc
			TLogDefLindex::iterator logDefIt = _LogDefIndex.find(logInfo.getLogName());
			if (logDefIt == _LogDefIndex.end())
			{
				nlwarning("Service '%s' send unknown log named '%s'", 
					sender ? sender->getModuleName().c_str() : "NULL", 
					logInfo.getLogName().c_str());
				return;
			}
			
			// 3rd validate the parameter list
			nlassert(logDefIt->second < _LogDefs.size());
			const TLogDefinition &ld = _LogDefs[logDefIt->second];
			
			if (logInfo.getParams().size() != ld.getParams().size())
			{
				nlwarning("Service '%s' send log '%s' with %u param instead of %u", 
					sender ? sender->getModuleName().c_str() : "NULL", 
					logInfo.getLogName().c_str(),
					logInfo.getParams().size(),
					ld.getParams().size());
				return;
			}
			if (logInfo.getListParams().size() != ld.getListParams().size())
			{
				nlwarning("Service '%s' send log '%s' with %u listParam instead of %u", 
					sender ? sender->getModuleName().c_str() : "NULL", 
					logInfo.getLogName().c_str(),
					logInfo.getListParams().size(),
					ld.getListParams().size());
				return;
			}
			for (uint i=0; i<ld.getParams().size(); ++i)
			{
				if (ld.getParams()[i].getType() != logInfo.getParams()[i].getType())
				{
					nlwarning("Service '%s' send log '%s' with %uth param (%s) of type %s instead of %s", 
						sender ? sender->getModuleName().c_str() : "NULL", 
						logInfo.getLogName().c_str(),
						i+1,
						ld.getParams()[i].getName().c_str(),
						logInfo.getParams()[i].getType().toString().c_str(),
						ld.getParams()[i].getType().toString().c_str());
					return;
				}
			}
			for (uint i=0; i<ld.getListParams().size(); ++i)
			{
				const LGS::TListParamValues &values = logInfo.getListParams()[i];
				std::list < LGS::TParamValue >::const_iterator first(values.getParams().begin()), last(values.getParams().end());
				for (uint j=0; first != last; ++first, ++j)
				{
					if (ld.getListParams()[i].getType() != first->getType())
					{
						nlwarning("Service '%s' send log '%s' with %uth listParam (%s) of type %s instead of %s", 
							sender ? sender->getModuleName().c_str() : "NULL", 
							logInfo.getLogName().c_str(),
							i+1,
							ld.getParams()[i].getName().c_str(),
							first->getType().toString().c_str(),
							ld.getParams()[i].getType().toString().c_str());
						return;
					}
				}
			}
			
			
			// 4st store the log
			_LogInfos.push_back(TLogEntry());
			++_LogInfosSize;
			_LogInfos.back().LogId = _LogId++;
			_LogInfos.back().ShardId = it->second;
			_LogInfos.back().LogInfo = logInfo;
		}
	}

	void queryLogs(const std::string queryString, const CLogStorage &ls, list<string> &result, uint32 &totalLogParsed, uint32 &totalLogSelected, uint32 &totalLogOutput)
	{
		uint32 now= CTime::getSecondsSince1970();
		CQueryParser qp(ls._LogDefs);
		CQueryParser::TParserResult pr = qp.parseQuery(queryString, false);

		if (pr.QueryTree.get() != NULL)
		{
			totalLogParsed += (uint32)ls._DiskLogEntries.size();
			TLogEntries logs = pr.QueryTree->evalNode(ls);
			totalLogSelected += (uint32)logs.size();

			// add the context log to the selection
			vector<uint32> contextLogs;
			TLogEntries::iterator first(logs.begin()), last(logs.end());
			for (; first != last; ++first)
			{
				uint32 id = *first;
				const CLogStorage::TDiskLogEntry &dle = ls._DiskLogEntries[id];

				uint32 stackSize = dle.ContextStack;
				uint32 lowerStackSize = dle.ContextStack;
				// select opening logs
				while (stackSize > 0 && id > 0)
				{
					--id;
					if (ls._DiskLogEntries[id].LogDate == 0)
					{
						if (stackSize == lowerStackSize || pr.FullContext)
							contextLogs.push_back(id);
						--stackSize;
						lowerStackSize = min(lowerStackSize, stackSize);
					}
					else if (ls._DiskLogEntries[id].LogDate == ~0)
					{
						++stackSize;

						if (pr.FullContext)
							contextLogs.push_back(id);
					}
					else if (pr.FullContext)
					{
						contextLogs.push_back(id);
					}
				}
				// select closing logs
				id = *first;
				stackSize = dle.ContextStack;
				lowerStackSize = dle.ContextStack;
				while (stackSize > 0 && id < ls._DiskLogEntries.size()-1)
				{
					++id;
					if (ls._DiskLogEntries[id].LogDate == 0)
					{
						++stackSize;
						if (pr.FullContext)
							contextLogs.push_back(id);
					}
					else if (ls._DiskLogEntries[id].LogDate == ~0)
					{
						if (stackSize == lowerStackSize || pr.FullContext)
							contextLogs.push_back(id);
						--stackSize;
						lowerStackSize = min(lowerStackSize, stackSize);
					}
					else if (pr.FullContext)
					{
						contextLogs.push_back(id);
					}
				}
			}

			// insert the selected context
			logs.insert(contextLogs.begin(), contextLogs.end());

			totalLogOutput += (uint32)logs.size();

			// output the final log selection
			first = logs.begin();
			last = logs.end();
			for (; first != last; ++first)
			{
				uint32 id = *first;
				const CLogStorage::TDiskLogEntry &dle = ls._DiskLogEntries[id];
				const LGS::TLogDefinition &logDef = ls._LogDefs[dle.LogType];

				CSString logLine;
				logLine.reserve(256);

				string shardName = CShardNames::getInstance().getShardName(dle.ShardId);
				if (shardName.empty())
					logLine << "ShardId="<<dle.ShardId<<":";
				else
					logLine << "ShardId="<<shardName<<":";

				for (uint i=0; i<dle.ContextStack; ++i)
					logLine << "	";


				if (dle.LogDate == 0)
					logLine << "OpenContext "<<logDef.getLogName()<<":"<<logDef.getLogText()<<":";
				else if (dle.LogDate == ~0)
					logLine << "CloseContext "<<logDef.getLogName()<<":";
				else
					logLine << formatDate(dle.LogDate)<< "("<<CTime::getHumanRelativeTime(now-dle.LogDate)<<"):"<<logDef.getLogName()<<":"<<logDef.getLogText()<<":";

				for (uint j=0; j<dle.ParamIndex.size(); ++j)
				{
					const LGS::TParamDesc &paramDesc = logDef.getParams()[j];
					CLogStorage::TLogParamId lpi;
					lpi.ParamName = paramDesc.getName();
					lpi.ParamType = paramDesc.getType();
					nlassert(ls._ParamTables.find(lpi) != ls._ParamTables.end());
					if (lpi.ParamType == LGS::TSupportedParamType::spt_string)
						logLine << " "<<paramDesc.getName()<<"=\""<<ls._ParamTables.find(lpi)->second[dle.ParamIndex[j]].toString()<<"\"";
					else
						logLine << " "<<paramDesc.getName()<<"="<<ls._ParamTables.find(lpi)->second[dle.ParamIndex[j]].toString();
				}
				for (uint j=0; j<dle.ListParamIndex.size(); ++j)
				{
					const LGS::TParamDesc &paramDesc = logDef.getListParams()[j];
					CLogStorage::TLogParamId lpi;
					lpi.ParamName = paramDesc.getName();
					lpi.ParamType = paramDesc.getType();
					nlassert(ls._ParamTables.find(lpi) != ls._ParamTables.end());
					logLine << " "<<paramDesc.getName()<<"=";

					for (uint k=0; k<dle.ListParamIndex[j].size(); ++k)
					{
						if (lpi.ParamType == LGS::TSupportedParamType::spt_string)
							logLine << "\""<<ls._ParamTables.find(lpi)->second[dle.ListParamIndex[j][k]].toString()<<"\"";
						else
							logLine << ls._ParamTables.find(lpi)->second[dle.ListParamIndex[j][k]].toString();
						if (k < dle.ListParamIndex[j].size()-1)
							logLine << ", ";
					}
				}
				result.push_back(string());
				result.back().swap(logLine);
			}
		}
	}

	void executeQuery(std::string query, uint32 queryNumber)
	{
		CQueryParser optionsQp(_LogDefs);
		CQueryParser::TParserResult queryOptions = optionsQp.parseQuery(query, true);

		_QueryStatus.write(TThreadStatus(qs_pop_state, "ErrorWritingQueryResult"));
		_QueryStatus.write(TThreadStatus(qs_lql_state, toString("Starting executing query %u", queryNumber)));

		// prepare the output file
		string outputFile = queryOptions.OutputPrefix+LogQueryResultFile.get();
		FILE *fp = fopen(outputFile.c_str(), "wt");;
		if (fp == NULL)
		{
			_QueryStatus.write(TThreadStatus(qs_push_state, "ErrorWritingQueryResult"));
			_QueryStatus.write(TThreadStatus(qs_lql_state, toString("Failed to open output file '%s'", outputFile.c_str())));
			return;
		}
		fclose(fp);

		list<string> result;
		try
		{
			uint32 totalLogParsed =0;
			uint32 totalLogSelected =0;
			uint32 totalLogOutput =0;
			uint32 totalFileSelected =0;
			uint32 totalFileFound =0;

			// write the query at the start of the result set
			result.push_back(query);
			result.push_back(string("================================================================================"));

			// prepare a request tree for file date checking
			CQueryParser dateQp(_LogDefs);
			CQueryParser::TParserResult parserResult = dateQp.parseQuery(query, false);
			// build the time line for the request
			TTimeLine timeLine = parserResult.QueryTree->evalDate();

			if (timeLine.empty())
			{
				_QueryStatus.write(TThreadStatus(qs_log, toString("No valid time line defined")));
				return;
			}

			if (timeLine.size() == 1 && timeLine[0].StartDate == 0 && timeLine[0].EndDate == ~0)
			{
				_QueryStatus.write(TThreadStatus(qs_log, toString("No LogDate specified, request will be limited to the last 24 hours files")));
				// no time range defined, force a 24 hours time line
//				query = "LogDate > yesterday and ("+query+")";

				result.push_back(string("WARNING : last 24 hours only : no time predicate found, result is automaticaly limited to last day"));
				result.push_back(string("================================================================================"));

				// set start date to yesterday
				timeLine[0].StartDate = CTime::getSecondsSince1970()-60*60*24;
			}

			_QueryStatus.write(TThreadStatus(qs_push_state, "Querying"));
			/// dump the time line
			for (uint i=0; i<timeLine.size(); ++i)
			{
				nlinfo("TimeLine item %2u : [%s - %s)", i, formatDate(timeLine[i].StartDate).c_str(), formatDate(timeLine[i].EndDate).c_str());
			}

			time_t now = CTime::getSecondsSince1970();

			vector<string>	files;
			CPath::getPathContent(CLogStorage::getLogRoot(), false, false, true, files, NULL, true);

			// first loop to select the file to read according to the date 
//			set<uint32>	selectedFileIndex;
			/// This is the ordered by date file selection
			map<uint32, uint32> selectedFile;

			uint32 previousDate = 0;

			for (uint i=0; i<files.size(); ++i)
			{
				if (files[i].substr(files[i].size()-7) == ".binlog" 
					&& files[i].find("hourly_") != string::npos)
				{
					// extract the date from the file name
					string fileName = CFile::getFilenameWithoutExtension(files[i]);

					struct tm fileDate;
					memset(&fileDate, 0, sizeof(fileDate));
					int nbField = sscanf(fileName.c_str(), "hourly_%u-%u-%u_%u-%u-%u", 
						&fileDate.tm_year,
						&fileDate.tm_mon,
						&fileDate.tm_mday,
						&fileDate.tm_hour,
						&fileDate.tm_min,
						&fileDate.tm_sec
						);

					if (nbField != 6)
					{
						nlwarning("Invalid log file '%s' found in log directory", fileName.c_str());
						continue;
					}

					++totalFileFound;

					// adjust the year offset
					fileDate.tm_year -= 1900;
					// adjust month
					fileDate.tm_mon -= 1;
					// let the CRunt time compute the daylight saving offset
					fileDate.tm_isdst  = -1;

					uint32 date = (uint32)mktime(&fileDate);

					// check that the date is within a selected time slice
					for (uint j=0; j<timeLine.size(); ++j)
					{
						if (!(	(date < timeLine[j].StartDate && previousDate < timeLine[j].StartDate)
							|| (date > timeLine[j].EndDate && previousDate > timeLine[j].EndDate))
							)
						{
							nlinfo("File %s from (%s to %s] is accepted in the range [%s - %s)",
								fileName.c_str(),
								formatDate(previousDate).c_str(),
								formatDate(date).c_str(),
								formatDate(timeLine[j].StartDate).c_str(),
								formatDate(timeLine[j].EndDate).c_str());

							selectedFile.insert(make_pair(date, i));
							break;
						}
						else
						{
							nlinfo("File %s from (%s to %s] is REJECTED from the range [%s - %s)",
								fileName.c_str(),
								formatDate(previousDate).c_str(),
								formatDate(date).c_str(),
								formatDate(timeLine[j].StartDate).c_str(),
								formatDate(timeLine[j].EndDate).c_str());

						}
					}

					previousDate = date;
				}
			}

			totalFileSelected = (uint32)selectedFile.size();
			// now, do the real job, query inside each selected file in ascending date order
			map<uint32, uint32>::iterator first(selectedFile.begin()), last(selectedFile.end());
			for (uint counter=0; first != last; ++first, ++counter)
			{
				_QueryStatus.write(TThreadStatus(qs_lql_state, toString("Reading log file %u/%u", counter+1, selectedFile.size())));

				CLogStorage ls(_LogDefs);
				ls.loadLogs(files[first->second]);

				// check the timeline limits
				if (!ls._DiskLogEntries.empty() 
					&& 
						(ls._DiskLogEntries.begin()->LogDate >  timeLine.rbegin()->EndDate
						|| ls._DiskLogEntries.rbegin()->LogDate <  timeLine.begin()->StartDate)
						)
				{
					// no log can match inside this file
					continue;
				}

				_QueryStatus.write(TThreadStatus(qs_lql_state, toString("Processing log file %u/%u", counter+1, selectedFile.size())));
				queryLogs(query, ls, result, totalLogParsed, totalLogSelected, totalLogOutput);

				// check the command channel for interrupt request
				TThreadCommand qs;
				if (_QueryCommands.read(qs, TQueryInterupPred()))
				{
					_QueryStatus.write(TThreadStatus(qs_lql_state, toString("Query %u interrupted", queryNumber)));
					goto endQuery;
				}
			}

			// process the log not saved (i.e the logs only in memory)
			{
				CLogStorage ls(_LogDefs);
				{
					CAutoMutex<CMutex> lock(_LogMutex);
					_QueryStatus.write(TThreadStatus(qs_lql_state, toString("Processing %u logs still in memory...", _LogInfosSize)));
					
					TLogInfos::const_iterator first(_LogInfos.begin()), last(_LogInfos.end());
					for (; first != last; ++first)
					{
						ls.storeLog(*first);
					}
				}

				queryLogs(query, ls, result, totalLogParsed, totalLogSelected, totalLogOutput);

				// check the command channel for interrupt request
				TThreadCommand qs;
				if (_QueryCommands.read(qs, TQueryInterupPred()))
				{
					_QueryStatus.write(TThreadStatus(qs_lql_state, toString("Query %u interrupted", queryNumber)));
					goto endQuery;
				}
			}

			uint32 afterSelect = CTime::getSecondsSince1970();
	
			_QueryStatus.write(TThreadStatus(qs_pop_state, "Querying"));
			_QueryStatus.write(TThreadStatus(qs_push_state, "WritingResult"));


			result.push_back("===============================================================================");
			result.push_back("Query stats :");
			result.push_back(toString("%u log files found, %u log file read", totalFileFound, totalFileSelected));
			result.push_back(toString("%u log parsed, %u log selected, %u log written to output", totalLogParsed, totalLogSelected, totalLogOutput));


			/// output the result
			{

				TThreadResult threadResult;
				threadResult.OutputFilename = outputFile;
				threadResult.QueryNumber = queryNumber;
				threadResult.Lines = &result;

				// write the result in the channel and block until the main thread
				// has read it.
				_QueryResult.syncWrite(threadResult);

			}

			_QueryStatus.write(TThreadStatus(qs_log, toString("Selected %u logs in %u seconds, %u seconds to output result", totalLogSelected, afterSelect-now, CTime::getSecondsSince1970()-afterSelect)));

			_QueryStatus.write(TThreadStatus(qs_lql_state, toString("Finished query %u", queryNumber)));

		}
		catch (const CQueryParser::EInvalidQuery &iq)
		{
			uint index= (uint)(iq.It - query.begin());
			_QueryStatus.write(TThreadStatus(qs_log, toString("Error will parsing query near char %u : %s", index+1, iq.ErrorStr)));
			_QueryStatus.write(TThreadStatus(qs_log, query));
			CSString err;
			for (uint i=0; i<index; ++i)
				err << " ";
			err << "^";
			_QueryStatus.write(TThreadStatus(qs_log, err));

		}

endQuery:
		_QueryStatus.write(TThreadStatus(qs_pop_state, "Querying"));
		_QueryStatus.write(TThreadStatus(qs_pop_state, "WritingResult"));

	}



	/*************************************************************************/
	/* Commands handler														 */
	/*************************************************************************/
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CLoggerServiceMod, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, dump, "Dump the module internal state", "");
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, displayLog, "Display the last log entries", "[duration(s)=10]");
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, fillTestLog, "Fill the logger with some test log. Logger must be just started with no clients", "");
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, queryLogs, "Test the query parser", "<a full query>");
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, interruptQuery, "Stop the current running query", "");
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, saveLogs, "Save the logs", "");
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, loadLogs, "Load a log file", "<filename>");
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, displayLogFormat, "Display the format of log", "");
		NLMISC_COMMAND_HANDLER_ADD(CLoggerServiceMod, generateManyLogs, "Generate a big number of random logs", "<nbLogsToGenerate> <nbOutputFile>");
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(generateManyLogs)
	{
		CRandom rand;
		rand.srand(CTime::getSecondsSince1970());


		if (args.size() != 2)
			return false;

		if (_LogDefs.size() == 0)
		{
			log.displayNL("Can't generate log because no log definition found");
			return true;
		}
		
		uint32 nbLogs, nbFiles;
		NLMISC::fromString(args[0], nbLogs);
		NLMISC::fromString(args[1], nbFiles);

		if (nbLogs == 0 || nbFiles == 0)
		{
			log.displayNL("Invalid parameters");
			return false;
		}

		uint32 nbLogsPerFile = nbLogs / nbFiles;

		/// Build the table of 'random' value
		vector<CEntityId> entityIds;
		for (uint i=0; i<25000; ++i)
		{
			entityIds.push_back(CEntityId(0, randu64(rand, UINT64_CONSTANT(0xffffffffffffffff)), 0, 0));
		}
		vector<INVENTORIES::TItemId> itemIds;
		for (uint i=0; i<60000; ++i)
		{
			itemIds.push_back(INVENTORIES::TItemId(randu64(rand, UINT64_CONSTANT(0xffffffffffffffff))));
		}

		std::vector <CSheetId> allSheets;
		CSheetId::buildIdVector(allSheets);

		vector<CSheetId> sheetIds;
		for (uint i=0; i<3000; ++i)
		{
			sheetIds.push_back(allSheets[randu32(rand, (uint32)allSheets.size())]);
		}


		for (uint i=0; i<nbFiles; ++i)
		{
			vector<uint32>	contextStack;
			log.displayNL("Generating log file %u/%u", i+1, nbFiles);
			for (uint j=0; j<nbLogsPerFile; ++j)
			{
				if (j%1000 == 0)
					log.displayNL("Generating log entry %u/%u", j+1, nbLogsPerFile);

				if (!contextStack.empty()
					&& randu32(rand, 5) < contextStack.size())
				{
					LGS::TLogDefinition &ld = _LogDefs[contextStack.back()];
					// unstack a level of context
					TLogEntry le;
					le.LogInfo.setLogName(ld.getLogName());
					le.LogInfo.setTimeStamp(~0);
					le.ShardId = 666;
					le.LogId = _LogId++;
					_LogInfos.push_back(le);

					contextStack.pop_back();
				}
				else
				{
					// select a random log type
					uint32 logType = randu32(rand, (uint32)_LogDefs.size());

					LGS::TLogDefinition &ld = _LogDefs[logType];

					if (ld.getContext())
					{
						// this is a context log, stack a context
						TLogEntry le;
						le.LogInfo.setLogName(ld.getLogName());
						le.LogInfo.setTimeStamp(0);
						le.ShardId = 666;
						le.LogId = _LogId++;
						_LogInfos.push_back(le);
						contextStack.push_back(logType);
					}
					else
					{
						// this is a normal log, build a random list of parameter
						TLogEntry le;
						le.LogInfo.setLogName(ld.getLogName());
						le.LogInfo.setTimeStamp(CTime::getSecondsSince1970());;
						le.ShardId = 666;
						le.LogId = _LogId++;

						le.LogInfo.getParams().reserve(ld.getParams().size());
						for (uint k=0; k<ld.getParams().size(); ++k)
						{
							LGS::TParamDesc &pd = ld.getParams()[k];

							switch (pd.getType().getValue())
							{
							case LGS::TSupportedParamType::spt_uint32:
								le.LogInfo.getParams().push_back(TParamValue(randu32(rand, 0xffffffff)));
								break;
							case LGS::TSupportedParamType::spt_uint64:
								le.LogInfo.getParams().push_back(TParamValue(randu64(rand, UINT64_CONSTANT(0xffffffffffffffff))));
								break;
							case LGS::TSupportedParamType::spt_sint32:
								le.LogInfo.getParams().push_back(TParamValue(rands32(rand, 0xffffffff)));
								break;
							case LGS::TSupportedParamType::spt_float:
								le.LogInfo.getParams().push_back(TParamValue(rand.frand()*1000.0f));
								break;
							case LGS::TSupportedParamType::spt_string:
								{
									char buffer[2];
									buffer[1] =0;
									buffer[0] = 32+(char)rand.rand(96);
									le.LogInfo.getParams().push_back(TParamValue(string(buffer)));
								}
								break;
							case LGS::TSupportedParamType::spt_entityId:
								le.LogInfo.getParams().push_back(TParamValue(entityIds[randu32(rand, (uint32)entityIds.size())]));
								break;
							case LGS::TSupportedParamType::spt_sheetId:
								le.LogInfo.getParams().push_back(TParamValue(sheetIds[randu32(rand, (uint32)sheetIds.size())]));
								break;
							case LGS::TSupportedParamType::spt_itemId:
								le.LogInfo.getParams().push_back(TParamValue(itemIds[randu32(rand, (uint32)itemIds.size())]));
								break;
							}
						}

						le.LogInfo.getListParams().resize(ld.getListParams().size());
						for (uint k=0; k<ld.getListParams().size(); ++k)
						{
							LGS::TParamDesc &pd = ld.getListParams()[k];
							uint32 listSize = randu32(rand, 20);

							TListParamValues &lpv = le.LogInfo.getListParams()[k];

							for (uint l=0; l<listSize; ++l)
							{
								switch (pd.getType().getValue())
								{
								case LGS::TSupportedParamType::spt_uint32:
									lpv.getParams().push_back(TParamValue(randu32(rand, 0xffffffff)));
									break;
								case LGS::TSupportedParamType::spt_uint64:
									lpv.getParams().push_back(TParamValue(randu64(rand, UINT64_CONSTANT(0xffffffffffffffff))));
									break;
								case LGS::TSupportedParamType::spt_sint32:
									lpv.getParams().push_back(TParamValue(rands32(rand, 0xffffffff)));
									break;
								case LGS::TSupportedParamType::spt_float:
									lpv.getParams().push_back(TParamValue(rand.frand()*1000.0f));
									break;
								case LGS::TSupportedParamType::spt_string:
									{
										char buffer[2];
										buffer[1] =0;
										buffer[0] = 32+(char)rand.rand(96);
										lpv.getParams().push_back(TParamValue(string(buffer)));
									}
									break;
								case LGS::TSupportedParamType::spt_entityId:
									lpv.getParams().push_back(TParamValue(entityIds[randu32(rand, (uint32)entityIds.size())]));
									break;
								case LGS::TSupportedParamType::spt_sheetId:
									lpv.getParams().push_back(TParamValue(sheetIds[randu32(rand, (uint32)sheetIds.size())]));
									break;
								case LGS::TSupportedParamType::spt_itemId:
									lpv.getParams().push_back(TParamValue(itemIds[randu32(rand, (uint32)itemIds.size())]));
									break;
								}
							}
						}

						_LogInfos.push_back(le);
					}
				}

			}

			// empty the context stack
			while (!contextStack.empty())
			{
				LGS::TLogDefinition &ld = _LogDefs[contextStack.back()];
				// unstack a level of context
				TLogEntry le;
				le.LogInfo.setLogName(ld.getLogName());
				le.LogInfo.setTimeStamp(~0);
				le.ShardId = 666;
				le.LogId = _LogId++;
				_LogInfos.push_back(le);

				contextStack.pop_back();
			}

			log.displayNL("Writing log file %u/%u", i+1, nbFiles);
			doHourlyProcess();

			// wait at least one second before continuing
			nlSleep(1000);
		}

		return true;
	}


	NLMISC_CLASS_COMMAND_DECL(interruptQuery)
	{
		TThreadCommand tc;
		tc.QueryCommand = qc_interupt;
		_QueryCommands.write(tc);

		log.displayNL("Query interruption sent.");

		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(displayLogFormat)
	{
		set<string> allParamNames;
		log.displayNL("Definition of %5u logs :", _LogDefs.size());
		log.displayNL("--------------------------", _LogDefs.size());
		for (uint i=0; i<_LogDefs.size(); ++i)
		{
			const TLogDefinition &ld = _LogDefs[i];

			log.displayNL("LogName=%s, Text=%s, %u parameters, %u list parameters", 
				ld.getLogName().c_str(), 
				ld.getLogText().c_str(), 
				ld.getParams().size(),
				ld.getListParams().size());

			if (!ld.getParams().empty())
			{
				for (uint j=0; j<ld.getParams().size(); ++j)
				{
					const TParamDesc&pd = ld.getParams()[j];
					log.display("  ParamName=%s, Type=%s", pd.getName().c_str(), pd.getType().toString().c_str());

					allParamNames.insert(pd.getName());
				}
				log.displayNL("");
			}
			if (!ld.getListParams().empty())
			{
				log.displayNL("  List params :");
				for (uint j=0; j<ld.getListParams().size(); ++j)
				{
					const TParamDesc&pd = ld.getListParams()[j];
					log.display("  ParamName=%s, Type=%s", pd.getName().c_str(), pd.getType().toString().c_str());

					allParamNames.insert(pd.getName());
				}
				log.displayNL("");
			}
		}
		log.displayNL("");
		log.displayNL("List of all param name :", _LogDefs.size());
		log.displayNL("------------------------", _LogDefs.size());

		set<string>::iterator first(allParamNames.begin()), last(allParamNames.end());
		CSString line;
		for (; first != last; ++first)
		{
			line << *first << "  ";
			if (line.size() > 80)
			{
				log.displayNL("%s", line.c_str());
				line = "";
			}
		}
		if (!line.empty())
			log.displayNL("%s", line.c_str());


		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(loadLogs)
	{
		if (args.size() != 1)
			return false;

		CLogStorage ls;
		ls.loadLogs(args[0]);

		ls.dumpLogs(log);

		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(saveLogs)
	{
		CAutoMutex<CMutex> lock(_LogMutex);
		CLogStorage ls(_LogDefs);
		ls.saveHourly(_LogInfos);
//		ls.storeLogs(0, 0x7fffffff, _LogInfos);

		ls.dumpLogs(log);

		return true;
	}
		
	NLMISC_CLASS_COMMAND_DECL(queryLogs)
	{
		if (args.size() == 1 && args[0] == "help")
		{
			// output the LQL documentation
			CConfigFile::CVar *var = IService::getInstance()->ConfigFile.getVarPtr("LogQueryLanguageHelp");
			if (var == NULL)
			{
				log.displayNL("Can't find documentation !");
				return true;
			}

			for (uint i=0; i<var->size(); ++i)
			{
				log.displayNL("%s", var->asString(i).c_str());
			}

			return true;
		}
		string query = rawCommandString.substr(rawCommandString.find(" "));

		TThreadCommand tc;
		tc.QueryCommand = qc_query;
		tc.QueryNumber = ++_LastQueryNumber;
		tc.QueryString = query;

		_QueryCommands.write(tc);

		log.displayNL("Started query %u", _LastQueryNumber);

		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(dump)
	{
		NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

		log.displayNL("-----------------------------");
		log.displayNL("Dumping logger service :");
		log.displayNL("-----------------------------");


		log.displayNL("There is %u connected and accepted clients :", _Clients.size());
		TLogClients::iterator first(_Clients.begin()), last(_Clients.end());
		for (; first != last; ++first)
		{
			IModuleProxy *proxy = first->first;
			TShardId shardId = first->second;
			log.displayNL("  Client : '%s' for shard %u", 
				proxy? proxy->getModuleName().c_str() : "NULL", 
				shardId);
		}

		log.displayNL("There are %u log entry in memory.", _LogInfosSize);

		return true;
	}

	
	NLMISC_CLASS_COMMAND_DECL(displayLog)
	{
		CAutoMutex<CMutex> lock(_LogMutex);

		uint32 now = CTime::getSecondsSince1970();
		uint32 nbSec = 10;
		if (args.size() > 0)
			NLMISC::fromString(args[0], nbSec);

		std::string tabs;

		TLogInfos::reverse_iterator rit = _LogInfos.rbegin();
		// advance cursor up to the requested time
		while (rit != _LogInfos.rend() 
			&& (rit->LogInfo.getTimeStamp() == 0 
				|| rit->LogInfo.getTimeStamp() == ~0 
				|| rit->LogInfo.getTimeStamp()+nbSec > now))
			++rit;

		// now convert to forward iterator
		TLogInfos::iterator it = rit.base();


		while (it != _LogInfos.end() 
			&& (it->LogInfo.getTimeStamp() == 0 
				|| it->LogInfo.getTimeStamp() == ~0 
				|| it->LogInfo.getTimeStamp()+nbSec > now))
		{
			const TLogInfo &li = it->LogInfo;

			// retrieve the log definition
			size_t index = _LogDefIndex[li.getLogName()];
			nlassert(index < _LogDefs.size());
			const TLogDefinition &ld = _LogDefs[index];

			if (li.getTimeStamp() == 0)
			{
				log.displayNL("%sOpening log context %s", tabs.c_str(), ld.getLogName().c_str());
				tabs += '\t';
			}
			else if (li.getTimeStamp() == ~0)
			{
				tabs.erase(tabs.size()-1);
				log.displayNL("%sClosing log context %s", tabs.c_str(), ld.getLogName().c_str());
			}
			else
			{
				log.display("%s%s(%ds ago), LOG : %s", tabs.c_str(), formatDate(li.getTimeStamp()).c_str(), now-li.getTimeStamp(), ld.getLogText().c_str());

				for (uint i=0; i<ld.getParams().size(); ++i)
				{
					log.display(", %s = '%s'", ld.getParams()[i].getName().c_str(), li.getParams()[i].toString().c_str());
				}
				log.displayNL("");
			}

			// advance to next log
			++it;
		}

		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(fillTestLog)
	{	
		vector<TLogDefinition>	logDefs;

		logDefs.resize(4);
		// define log 1
		{
			TLogDefinition &ld = logDefs[0];

			ld.setContext(false);
			ld.setLogName("L1");
			ld.setLogText("Two character are in a boat");
			ld.getParams().resize(3);
			ld.getParams()[0].setName("Char1");
			ld.getParams()[0].setType(TSupportedParamType::spt_entityId);
			ld.getParams()[1].setName("Char2");
			ld.getParams()[1].setType(TSupportedParamType::spt_entityId);
			ld.getParams()[2].setName("BoatName");
			ld.getParams()[2].setType(TSupportedParamType::spt_string);
		}
		// define log 
		{
			TLogDefinition &ld = logDefs[1];

			ld.setContext(false);
			ld.setLogName("L2");
			ld.setLogText("A Character win a stack of item at national lottery");
			ld.getParams().resize(5);
			ld.getParams()[0].setName("Character");
			ld.getParams()[0].setType(TSupportedParamType::spt_entityId);
			ld.getParams()[1].setName("ItemId");
			ld.getParams()[1].setType(TSupportedParamType::spt_itemId);
			ld.getParams()[2].setName("ItemSheet");
			ld.getParams()[2].setType(TSupportedParamType::spt_sheetId);
			ld.getParams()[3].setName("Quality");
			ld.getParams()[3].setType(TSupportedParamType::spt_uint32);
			ld.getParams()[4].setName("Quantity");
			ld.getParams()[4].setType(TSupportedParamType::spt_uint32);
		}
		// define log 
		{
			TLogDefinition &ld = logDefs[2];

			ld.setContext(false);
			ld.setLogName("L3");
			ld.setLogText("A variable size parameter following two normal parameters");
			ld.getParams().resize(2);
			ld.getParams()[0].setName("Character");
			ld.getParams()[0].setType(TSupportedParamType::spt_entityId);
			ld.getParams()[1].setName("ChatText");
			ld.getParams()[1].setType(TSupportedParamType::spt_string);
			ld.getListParams().resize(1);
			ld.getListParams()[0].setName("Listeners");
			ld.getListParams()[0].setType(TSupportedParamType::spt_entityId);
			ld.getListParams()[0].setList(true);
		}
		// define log 
		{
			TLogDefinition &ld = logDefs[3];

			ld.setContext(false);
			ld.setLogName("L4");
			ld.setLogText("Two variables size parameter following two normal parameters");
			ld.getParams().resize(2);
			ld.getParams()[0].setName("Character");
			ld.getParams()[0].setType(TSupportedParamType::spt_entityId);
			ld.getParams()[1].setName("ChatText");
			ld.getParams()[1].setType(TSupportedParamType::spt_string);
			ld.getListParams().resize(2);
			ld.getListParams()[0].setName("Listeners");
			ld.getListParams()[0].setType(TSupportedParamType::spt_entityId);
			ld.getListParams()[0].setList(true);
			ld.getListParams()[1].setName("ListOfInt");
			ld.getListParams()[1].setType(TSupportedParamType::spt_uint32);
			ld.getListParams()[1].setList(true);
		}
		
		// register the client
		registerClient( NULL, 101, logDefs);

		// now, send some logs
		{
			TLogInfo li;
			li.setLogName("L1");
			li.setTimeStamp(CTime::getSecondsSince1970()-20);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x1234)));
			li.getParams().push_back(TParamValue(CEntityId(0, 0x5678)));
			li.getParams().push_back(TParamValue(string("Ho mon bateau")));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			TLogInfo li;
			li.setLogName("L1");
			li.setTimeStamp(CTime::getSecondsSince1970()-15);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x1111)));
			li.getParams().push_back(TParamValue(CEntityId(0, 0x2222)));
			li.getParams().push_back(TParamValue(string("Titanic")));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			TLogInfo li;
			li.setLogName("L2");
			li.setTimeStamp(CTime::getSecondsSince1970()-10);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x1111)));
			li.getParams().push_back(TParamValue(TItemId(UINT64_CONSTANT(123456789012345678))));
			li.getParams().push_back(TParamValue(CSheetId(1)));
			li.getParams().push_back(TParamValue(uint32(10)));
			li.getParams().push_back(TParamValue(uint32(1)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			TLogInfo li;
			li.setLogName("L2");
			li.setTimeStamp(CTime::getSecondsSince1970()-5);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x2222)));
			li.getParams().push_back(TParamValue(TItemId()));
			li.getParams().push_back(TParamValue(CSheetId(2)));
			li.getParams().push_back(TParamValue(uint32(100)));
			li.getParams().push_back(TParamValue(uint32(10)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}

		// send log with variable params
		{
			TLogInfo li;
			li.setLogName("L3");
			li.setTimeStamp(CTime::getSecondsSince1970()-4);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x3333)));
			li.getParams().push_back(TParamValue(string("Hello, this is a cool chat entry!")));
			li.getListParams().resize(1);
			for (uint i=0; i<10; ++i)
				li.getListParams()[0].getParams().push_back(TParamValue(CEntityId(0, 0x4444+i)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			TLogInfo li;
			li.setLogName("L4");
			li.setTimeStamp(CTime::getSecondsSince1970()-4);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x4444)));
			li.getParams().push_back(TParamValue(string("Hello, this is another also cool chat entry!")));
			li.getListParams().resize(2);
			for (uint i=0; i<10; ++i)
				li.getListParams()[0].getParams().push_back(TParamValue(CEntityId(0, 0x5555+i)));
			for (uint i=0; i<5; ++i)
				li.getListParams()[1].getParams().push_back(TParamValue(uint32(i)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}

		// send some missformed logs
		{
			// not enought param
			TLogInfo li;
			li.setLogName("L2");
			li.setTimeStamp(CTime::getSecondsSince1970()-5);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x2222)));
			li.getParams().push_back(TParamValue(TItemId(UINT64_CONSTANT(765432109876543210))));
			li.getParams().push_back(TParamValue(CSheetId(3)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			// not enought list param
			TLogInfo li;
			li.setLogName("L3");
			li.setTimeStamp(CTime::getSecondsSince1970()-4);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x3333)));
			li.getParams().push_back(TParamValue(string("Hello, this is a cool chat entry!")));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			// too many param
			TLogInfo li;
			li.setLogName("L2");
			li.setTimeStamp(CTime::getSecondsSince1970()-5);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x2222)));
			li.getParams().push_back(TParamValue(TItemId(UINT64_CONSTANT(765432109876543210))));
			li.getParams().push_back(TParamValue(uint32(100)));
			li.getParams().push_back(TParamValue(uint32(10)));
			li.getParams().push_back(TParamValue(CSheetId(4)));
			li.getParams().push_back(TParamValue(uint32(100)));
			li.getParams().push_back(TParamValue(uint32(10)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			// too many list param
			TLogInfo li;
			li.setLogName("L3");
			li.setTimeStamp(CTime::getSecondsSince1970()-4);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x3333)));
			li.getParams().push_back(TParamValue(string("Hello, this is a cool chat entry!")));
			li.getListParams().resize(2);
			for (uint i=0; i<10; ++i)
				li.getListParams()[0].getParams().push_back(TParamValue(CEntityId(0, 0x4444+i)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			// unknow log
			TLogInfo li;
			li.setLogName("L500");
			li.setTimeStamp(CTime::getSecondsSince1970()-5);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x2222)));
			li.getParams().push_back(TParamValue(TItemId(UINT64_CONSTANT(765432109876543210))));
			li.getParams().push_back(TParamValue(CSheetId(5)));
			li.getParams().push_back(TParamValue(uint32(100)));
			li.getParams().push_back(TParamValue(uint32(10)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			// invalid param
			TLogInfo li;
			li.setLogName("L2");
			li.setTimeStamp(CTime::getSecondsSince1970()-5);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x2222)));
			li.getParams().push_back(TParamValue(TItemId(UINT64_CONSTANT(765432109876543210))));
			li.getParams().push_back(TParamValue(CSheetId(6)));
			li.getParams().push_back(TParamValue(CSheetId(7)));
			li.getParams().push_back(TParamValue(uint32(10)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}
		{
			// invalid list param
			TLogInfo li;
			li.setLogName("L3");
			li.setTimeStamp(CTime::getSecondsSince1970()-4);
			li.getParams().push_back(TParamValue(CEntityId(0, 0x3333)));
			li.getParams().push_back(TParamValue(string("Hello, this is a cool chat entry!")));
			li.getListParams().resize(1);
			for (uint i=0; i<10; ++i)
				li.getListParams()[0].getParams().push_back(TParamValue(uint32(i)));

			reportLog(NULL, vector<TLogInfo>(&li, &li+1));
		}

		if (args.size() == 1 && args[0] == "many")
		{
			// fill a very big deal of logs
			for (uint i=0; i<50000; ++i)
			{
				TLogInfo li;
				li.setLogName("L2");
				li.setTimeStamp(CTime::getSecondsSince1970()-50000+i);
				li.getParams().push_back(TParamValue(CEntityId(0, 0x2222)));
				li.getParams().push_back(TParamValue(TItemId()));
				li.getParams().push_back(TParamValue(CSheetId(2)));
				li.getParams().push_back(TParamValue(uint32(100+i)));
				li.getParams().push_back(TParamValue(uint32(10)));

				reportLog(NULL, vector<TLogInfo>(&li, &li+1));
			}
		}


		return true;
	}

};

NLNET_REGISTER_MODULE_FACTORY(CLoggerServiceMod, "LoggerService");




// the logger service
class CLoggerService : public IService
{
public:

	/**
	 * Init
	 */
	void init()
	{
		// init the sheet manager without worying about aving the sheet_id.bin file
//		CSheetId::initWithoutSheet();
		CSheetId::init(false);

		CSingletonRegistry::getInstance()->init();

		CShardNames::getInstance().init(ConfigFile);
	}

	/**
	 * Update
	 */
	bool update ()
	{
		CSingletonRegistry::getInstance()->tickUpdate();

		return true;
	}

	void release()
	{
		CSingletonRegistry::getInstance()->release();

	}

	virtual std::string					getServiceStatusString() const
	{
		return toString("LQLState=%s LastFinishedQuery=%u", LQLState.c_str(), LastFinishedQuery.get());
	}

};










/// Logger servive
//
NLNET_SERVICE_MAIN (CLoggerService, "LGS", "logger_service", 0, EmptyCallbackArray, "", "");
