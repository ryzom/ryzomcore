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

#ifndef MYSQL_WRAPPER_H
#define MYSQL_WRAPPER_H

#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/md5.h"
#include "nel/net/service.h"
#include "nel/net/module_common.h"
#include "game_share/utils.h"
#ifdef NL_OS_WINDOWS
# include <WinSock2.h>
# include <Windows.h>
typedef unsigned long ulong;
#endif

#include <mysql.h>
#include <time.h>

#include "game_share/r2_basic_types.h" // for TSessionId 
namespace MSW
{
	class CStoreResult;
	class CUseResult;
	class CConnection;

	/// Utility function to encode a date
	std::string encodeDate(NLMISC::TTime date);

	/** escape string using 'mysql_real_escape_string' and return a the escaped string.
	 *	Note that the function is internally optimized to quickly build the return string
	 *	with very little overhead.
	 */
	const std::string &escapeString(const std::string &str, CConnection &dbCnx);

	class CConnection
	{
		friend const std::string &escapeString(const std::string &str, CConnection &dbCnx);

		/// Connection info
		std::string		_ConnHostName;
		std::string		_ConnUserName;
		std::string		_ConnPassword;
		std::string		_ConnDefaultDatabase;
		

		/// The mysql connection context
		MYSQL	*_MysqlContext;

		/// Flag for connection open
		bool	_Connected;

		typedef std::map<mysql_option, const char*>	TOptions;
		/// A list of pair name/value of connection option
		TOptions	_Options;

		/// Internale connect method (this one do the job)
		bool _connect();

	public:
		
		CConnection()
			:	_MysqlContext(NULL),
				_Connected(false)
		{
		}

		~CConnection()
		{
			if (_Connected)
				mysql_close(_MysqlContext);
		}

		void addOption(mysql_option option, const char *value);
		void clearOption(mysql_option option);

		bool connect(const std::string &hostName, const std::string &userName, const std::string &password, const std::string &defaultDatabase);
		bool connect(const NLNET::TParsedCommandLine &databaseInfo);

		void closeConn();

		bool query(const std::string &queryString);

		uint32 getLastGeneratedId()
		{
			return uint32(mysql_insert_id(_MysqlContext));
		}

		uint32 getAffectedRows()
		{
			return uint32(mysql_affected_rows(_MysqlContext));
		}

		std::auto_ptr<CStoreResult>		storeResult();
		std::auto_ptr<CUseResult>		useResult();

	};

	// base class contains method to extract info from a fetched row
	class CResultBase
	{
	protected:
		MYSQL_ROW		_CurrentRow;
		unsigned long	*_FieldLength;

		MYSQL_RES		*_Result;


		CResultBase(MYSQL_RES	*result)
			: _CurrentRow(NULL),
			_FieldLength(NULL),
			_Result(result)
		{

		}
	public:

		~CResultBase()
		{
			mysql_free_result(_Result);
		}


		/// return the number of fields in the result set
		uint32 getNumFields()
		{
			return mysql_num_fields(_Result);
		}

		const char *getRawField(uint32 fieldIndex)
		{
			nlassert(_CurrentRow != NULL);
			nlassert(fieldIndex < getNumFields());

			static const char *emptyString = "";

			char *ret = _CurrentRow[fieldIndex];

			return ret != NULL ? ret : emptyString;
		}

		void getField(uint32 fieldIndex, std::string &value)
		{
			value = getRawField(fieldIndex);
		}

		void getField(uint32 fieldIndex, bool &value)
		{
			const char *str = getRawField(fieldIndex);
			if (str[0] == '1')
				value = true;
			else if (str[0] == '0')
				value = false;
			else
				value = NLMISC::nlstricmp(str, "true") == 0;
		}
		void getField(uint32 fieldIndex, uint8 &value)
		{
			const char *str = getRawField(fieldIndex);
			value = uint8(strtoul(str, NULL, 10));
		}
		void getField(uint32 fieldIndex, uint32 &value)
		{
			const char *str = getRawField(fieldIndex);
			value = uint32(strtoul(str, NULL, 10));
		}
		void getField(uint32 fieldIndex, sint8 &value)
		{
			NLMISC::fromString(std::string(getRawField(fieldIndex)), value);
		}
		void getField(uint32 fieldIndex, sint32 &value)
		{
			NLMISC::fromString(std::string(getRawField(fieldIndex)), value);
		}
		
		void getField(uint32 fieldIndex, TSessionId &value)
		{
			sint32 val;
			NLMISC::fromString(std::string(getRawField(fieldIndex)), val);
			value =TSessionId(val);
		}

		void getField(uint32 fieldIndex, std::vector<uint8> &value)
		{
			const char *data = getRawField(fieldIndex);
			long size = _FieldLength[fieldIndex];
			std::vector<uint8> vec(data, data+size);
			value.swap(vec);
		}

		void getMD5Field(uint32 fieldIndex, NLMISC::CHashKeyMD5 &hashKey)
		{
			const char *data = getRawField(fieldIndex);
			long size = _FieldLength[fieldIndex];

			hashKey.fromString(data);
		}

		void getDateField(uint32 fieldIndex, uint32 &time);
	};

	class CUseResult : public CResultBase
	{
		friend class CConnection;

		CUseResult(MYSQL_RES	*result)
			: CResultBase(result)
		{
		}

	public:
		/// Advance to next row in the result set, return true is there is a row
		bool fetchRow()
		{
			_CurrentRow = mysql_fetch_row(_Result);
			if (_CurrentRow == NULL)
				return false;
			_FieldLength = mysql_fetch_lengths(_Result);
			nlassert(_FieldLength != NULL);

			return true;
		}
	};

	class CStoreResult : public CResultBase
	{
		friend class CConnection;


		CStoreResult(MYSQL_RES	*result)
			: CResultBase(result)
		{

		}
	public:


		/// Return the number of row in the result set
		uint32 getNumRows()
		{
			return uint32(mysql_num_rows(_Result));
		}

		/// Advance to next row in the result set
		void fetchRow()
		{
			_CurrentRow = mysql_fetch_row(_Result);
			nlassert(_CurrentRow != NULL);
			_FieldLength = mysql_fetch_lengths(_Result);
			nlassert(_FieldLength != NULL);
		}
	};

	

} // namespace MSW

//NeL simple Object Persistence Engine
namespace NOPE
{
	enum TObjectState
	{
		/// This is a temporary allocated, not persisted object instance
		os_transient,
		/// This is a persistent object, unmodified since it was loaded
		os_clean,
		/// This is a persistent object, modified in transient space that need to be saved to db
		os_dirty,
		/// This is removed object, it should no more be used as it have been removed from db
		os_removed,
		/// This is a persistent object, unmodified but that lie in the object cache.
		os_released,

		// a tag counter MUST BE LAST
		os_nb_state,
	};

	const uint32 INVALID_OBJECT_ID = 0;
	
	extern bool AllowedTransition [os_nb_state][os_nb_state];


	enum TCacheCmd
	{
		cc_update,
		cc_clear,
		cc_dump,
		cc_instance_count,
	};

	typedef uint32 (*TCacheCmdFunc)(TCacheCmd cmd);


	class CPersistentCache : 
		public NLNET::IServiceUpdatable,
		public NLMISC::ICommandsHandler
	{
		NLMISC_SAFE_SINGLETON_DECL(CPersistentCache);

		typedef std::set<TCacheCmdFunc>	TUpdateFuncs;
		TUpdateFuncs	_UpdateFuncs;

		CPersistentCache()
		{
			NLMISC::CCommandRegistry::getInstance().registerNamedCommandHandler(this, "CPersistentCache");
		}
//		virtual const std::string &getCommandHandlerClassName() const
//		{
//			static string name("CPersistentCache");
//			return name;
//		}

		virtual const std::string &getCommandHandlerName() const
		{
			static std::string name("sqlObjectCache");
			return name;
		}

	public:

		void registerCache(TCacheCmdFunc functionPtr)
		{
			_UpdateFuncs.insert(functionPtr);
		}
		
		void serviceLoopUpdate()
		{
			H_AUTO(CPersistentCache_serviceLoopUpdate);

			TUpdateFuncs::iterator first(_UpdateFuncs.begin()), last(_UpdateFuncs.end());
			for (; first != last; ++first)
			{
				TCacheCmdFunc f = *first;
				f(cc_update);
			}
		}

		// delete any unreference object in the cache
		void clearCache()
		{
			TUpdateFuncs::iterator first(_UpdateFuncs.begin()), last(_UpdateFuncs.end());
			for (; first != last; ++first)
			{
				TCacheCmdFunc f = *first;
				f(cc_clear);
			}
		}

		/// Return the grand total of instance in memory (including in use and in cache objects)
		uint32 getInstanceCount()
		{
			uint32 total = 0; 
			TUpdateFuncs::iterator first(_UpdateFuncs.begin()), last(_UpdateFuncs.end());
			for (; first != last; ++first)
			{
				TCacheCmdFunc f = *first;
				total += f(cc_instance_count);
			}

			return total;
		}
		

		NLMISC_COMMAND_HANDLER_TABLE_BEGIN(CPersistentCache)
			NLMISC_COMMAND_HANDLER_ADD(CPersistentCache, clearCache, "remove any unreferenced cached object from memory", "no params")
			NLMISC_COMMAND_HANDLER_ADD(CPersistentCache, dump, "dump cache status", "no params")
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			log.displayNL("Dumping SQL cache for %u class :", _UpdateFuncs.size());

			// redirect all logs to log
			NLMISC::CNLSmartLogOverride logRedirector(&log);
			TUpdateFuncs::iterator first(_UpdateFuncs.begin()), last(_UpdateFuncs.end());
			for (; first != last; ++first)
			{
				TCacheCmdFunc f = *first;
				f(cc_dump);
			}
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(clearCache)
		{
			// hop
			clearCache();
			return true;
		}


	};
} // namespace NOPE

#endif //  MYSQL_WRAPPER_H
