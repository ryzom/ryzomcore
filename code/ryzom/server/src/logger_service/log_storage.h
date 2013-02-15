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

#ifndef LOG_STORAGE_H
#define LOG_STORAGE_H

#include "logger_service.h"

/** Class to store logs in a 'compact' form on disk
 * The log stoage consiste of a series of 3 main chunks :
 *	- the log description chunk that contains the description of each log type
 *	- the log entry table, that contains an entry for each log with list of
 *		index in each parameter table (see next chunk)
 *	- the parameters tables. One table for each log name/type.
 */
class CLogStorage
{
public:
	/// Identity of a log parameter table
	struct TLogParamId
	{
		/// Name of the parameter
		std::string					ParamName;
		/// Type of the parameter
		LGS::TSupportedParamType	ParamType;
		
		/// Strict ordering to be used as map key
		bool operator < (const TLogParamId &other) const
		{
			if (ParamName < other.ParamName)
				return true;
			else if (ParamName == other.ParamName)
				return ParamType < other.ParamType;
			return false;
		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(ParamName);
			s.serial(ParamType);
		}
	};

	typedef std::vector<uint32>	TParamIndex;

	/// The log entries with parameter stored as table index
	struct TDiskLogEntry
	{
		/// Index in the log definition vector
		uint32			LogType;
		/// The date of the log
		uint32			LogDate;
		/// The size of the context stack
		uint32			ContextStack;	
		/// The shard id that sent this log
		uint32			ShardId;	
		/// Vector of index in each parameter table
		TParamIndex					ParamIndex;
		/// Vector of vector of index in each parameter table for list param
		std::vector<TParamIndex>	ListParamIndex;

		void serial(NLMISC::IStream &s)
		{
			s.serial(LogType);
			s.serial(LogDate);
			s.serial(ContextStack);
			s.serial(ShardId);
			s.serialCont(ParamIndex);
			uint32 nbList = (uint32)ListParamIndex.size();
			s.serial(nbList);
			ListParamIndex.resize(nbList);
			for (uint i=0; i<ListParamIndex.size(); ++i)
				s.serialCont(ListParamIndex[i]);
		}
	};

	/// The list of log to store on disk
	std::vector<TDiskLogEntry>	_DiskLogEntries;
	

	typedef std::vector<LGS::TParamValue>		TParamsTable;
	typedef std::map<TLogParamId, TParamsTable >	TParamsTables;

	/// The tables of all log params
	TParamsTables	_ParamTables;


	/// The log definition
	TLogDefinitions			_LogDefs;

	typedef std::map<std::string, size_t>	TLogDefLindex;
	// the log def index
	TLogDefLindex			_LogDefIndex;

	/// The current size of the context stack
	uint32					_ContextStack;


	struct EInvalidLogName
	{};

	/// Constructor used to store log file
	CLogStorage(const TLogDefinitions &logDefs)
		:	_LogDefs(logDefs),
			_ContextStack(0)
	{
		// fill the log definition
		for (uint i=0; i<logDefs.size(); ++i)
		{
			_LogDefIndex.insert(std::make_pair(logDefs[i].getLogName(), i));
		}
	}

	// Constructor used for loading log file
	CLogStorage()
		:	_ContextStack(0)
	{
	}

	static std::string getLogRoot()
	{
		return NLNET::IService::getInstance()->SaveFilesDirectory.toString()+"/logs";
	}

	const LGS::TLogDefinition &getLogDef(const std::string &logName)
	{
		TLogDefLindex::iterator logDefIt = _LogDefIndex.find(logName);
		if (logDefIt == _LogDefIndex.end())
			throw EInvalidLogName();
		
		nlassert(logDefIt->second < _LogDefs.size());
		return _LogDefs[logDefIt->second];
	}

	void loadLogs(const std::string &fileName)
	{
		// open the input file
		NLMISC::CIFile ifile(fileName);

		// and read the logs
		ifile.serial(*this);

		nldebug("Loaded %u logs from file %s", _DiskLogEntries.size(), fileName.c_str());
	}

	/// Save the last minute of logs on disk
	void saveMinutely(const TLogInfos &logInfos)
	{
		uint32 now = NLMISC::CTime::getSecondsSince1970();

		// A flag to skip saving if only context open/close are selected 
		// (because they have date 0 or ~0 that do not match correctlry the 
		// time test).
		bool atLeastOneSelected = false;;

		// go back to up to 1 minute ago
		TLogInfos::const_reverse_iterator it = logInfos.rbegin();
		while (it != logInfos.rend() && 
			(it->LogInfo.getTimeStamp() == 0
			|| it->LogInfo.getTimeStamp() == ~0
			|| it->LogInfo.getTimeStamp() > now-60))
		{
			if (it->LogInfo.getTimeStamp() != 0 && it->LogInfo.getTimeStamp() != ~0)
				atLeastOneSelected = true;
			++it;
		}

		if (atLeastOneSelected)
		{
			TLogInfos::const_iterator first(it.base()), last(logInfos.end());
			// prepare the logs to save
			for (; first != last; ++first)
			{
				storeLog(*first);
			}
		}

		// write the container on disk
		saveLogfile("minutely_");
	}

	// Save all logs in memory on disk
	void saveHourly(const TLogInfos &logInfos)
	{
		uint32 now = NLMISC::CTime::getSecondsSince1970();

//		// go back to up to 1 minute ago
//		TLogInfos::const_reverse_iterator it = logInfos.rbegin();
//		while (it != logInfos.rend() && 
//			(it->LogInfo.getTimeStamp() == 0
//			|| it->LogInfo.getTimeStamp() == ~0
//			|| it->LogInfo.getTimeStamp() > now-60*60))
//			++it;

		TLogInfos::const_iterator first(logInfos.begin()), last(logInfos.end());
		// prepare the logs to save
		for (; first != last; ++first)
		{
			storeLog(*first);
		}

		// write the container on disk
		saveLogfile("hourly_");
	}

	void saveLogfile(const std::string &prefix)
	{
		if (_DiskLogEntries.empty())
			// no log, do not store anything
			return;

		NLMISC::CSString fileName;

		// set the save directory and create it if needed
		fileName << getLogRoot() <<"/";
		NLMISC::CFile::createDirectoryTree(fileName);

		// build the file name from the current date
		char dateStr[1024];
		time_t now = time(NULL);
		struct tm *_tm = localtime(&now); 

		strftime(dateStr, 1024, "%Y-%m-%d_%H-%M-%S", _tm);
		
		fileName << prefix << dateStr <<".binlog";

		nldebug("Storing %u logs in file %s", _DiskLogEntries.size(), fileName.c_str());

		// open the output file
		{
			NLMISC::COFile of(fileName+".tmp");
			// and serial the logs
			of.serial(*this);
		}
		// rename the 'tmp" into finale output file
		NLMISC::CFile::moveFile(fileName.c_str(), (fileName+".tmp").c_str());
	}


	void storeLog(const TLogEntry &logEntry)
	{
		// NB : not optimized at all, just a place holder before replacement with
		// PDR 2 storage system (witch allow storage of table)

		const LGS::TLogDefinition &ld = getLogDef(logEntry.LogInfo.getLogName());

		// create a log entry
		TDiskLogEntry dle;

		// set the shard id
		dle.ShardId = logEntry.ShardId;
		// set the log date
		dle.LogDate = logEntry.LogInfo.getTimeStamp();

		// pre decrement if close context
		if (dle.LogDate == ~0 && _ContextStack > 0)
			--_ContextStack;

		dle.ContextStack = _ContextStack;
		// set the type of the log
		dle.LogType = (uint32)_LogDefIndex[logEntry.LogInfo.getLogName()];

		// post increment if open context
		if (dle.LogDate == 0)
			++_ContextStack;

		// store each parameter
		for (uint i=0; i<logEntry.LogInfo.getParams().size(); ++i)
		{
			TLogParamId lpi;
			const LGS::TParamValue &pv = logEntry.LogInfo.getParams()[i];
			lpi.ParamName = ld.getParams()[i].getName();
			lpi.ParamType = pv.getType();

			TParamsTable &pt = _ParamTables[lpi];
			uint32 index = (uint32)pt.size();
			pt.push_back(pv);

			// store the index in the persistent log entry
			dle.ParamIndex.push_back(index);
		}

		// store each variable parameter
		dle.ListParamIndex.resize(ld.getListParams().size());
		for (uint i=0; i<logEntry.LogInfo.getListParams().size(); ++i)
		{
			TLogParamId lpi;
			const LGS::TListParamValues &lpv = logEntry.LogInfo.getListParams()[i];
			lpi.ParamName = ld.getListParams()[i].getName();
			lpi.ParamType = ld.getListParams()[i].getType();
			
			// get the parameter table for the type of parameter
			TParamsTable &pt = _ParamTables[lpi];

			std::list < LGS::TParamValue >::const_iterator first(lpv.getParams().begin()), last(lpv.getParams().end());
			for (; first != last; ++first)
			{
				uint32 index = (uint32)pt.size();
				pt.push_back(*first);

				// store the index in the persistent log entry
				dle.ListParamIndex[i].push_back(index);
			}
		}


		// store the entry in the container
		_DiskLogEntries.push_back(dle);
	}

	void serial(NLMISC::IStream &s)
	{
		// serial the log definition
		s.serialCont(_LogDefs);
		// serial the log entryes
		s.serialCont(_DiskLogEntries);
		// serial the param tables
		uint32 nbTable = (uint32)_ParamTables.size();
		if (s.isReading())
		{
			s.serial(nbTable);
			for (uint i=0; i<nbTable; ++i)
			{
				// read the table header
				TLogParamId lpi;
				s.serial(lpi);

				TParamsTable &pt = _ParamTables[lpi];

				s.serialCont(pt);
			}
		}
		else
		{
			nlWrite(s, serial, nbTable);
			{
				TParamsTables::iterator first(_ParamTables.begin()), last(_ParamTables.end());
				for (; first != last; ++first)
				{
					nlWrite(s, serial, first->first);
					s.serialCont(first->second);
				}
			}
		}
	}

	void dumpLogs(NLMISC::CLog &log)
	{
		for (uint i=0; i<_DiskLogEntries.size(); ++i)
		{
			const TDiskLogEntry &dle = _DiskLogEntries[i];

			// get the descriptor
			nlassert(dle.LogType < _LogDefs.size());
			const LGS::TLogDefinition &ld = _LogDefs[dle.LogType];

			nlassert(dle.ParamIndex.size() == ld.getParams().size());

			// dump the log
			if (dle.LogDate == 0)
				log.display("Open Context : %s", ld.getLogName().c_str());
			else if (dle.LogDate == ~0)
				log.display("Close Context : %s", ld.getLogName().c_str());
			else
				log.display("%s : %s : %s ", formatDate(dle.LogDate).c_str(), ld.getLogName().c_str(), ld.getLogText().c_str());

			for (uint j=0; j<ld.getParams().size(); ++j)
			{
				const LGS::TParamDesc &pd = ld.getParams()[j];

				TLogParamId lpi;
				lpi.ParamName = pd.getName();
				lpi.ParamType = pd.getType();

				nlassert(_ParamTables.find(lpi) != _ParamTables.end());
				TParamsTable &pt = _ParamTables[lpi];

				log.display(" %s=%s", pd.getName().c_str(), pt[dle.ParamIndex[j]].toString().c_str());
			}
			log.displayNL("");
		}
	}
};

#endif //LOG_STORAGE_H
