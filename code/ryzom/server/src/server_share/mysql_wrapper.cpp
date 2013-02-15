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

#include "stdpch.h"
#include <memory>
#include "mysql_wrapper.h"
#include <errmsg.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;

CVariable<bool>		MSWStrictMode("msw", "MSWStrictMode", "Set the strict mode on SQL request", true, 0, true);
CVariable<uint32>	MSWRequestDuration("msw", "MSWRequestDuration", "Measure the duration of SQL request", 0, 1000);
CVariable<bool>		MSWAutoReconnect("msw", "MSWAutoReconnect", "MYSQL_OPT_RECONNECT", true, 0, true);


namespace MSW
{
	std::string encodeDate(NLMISC::TTime date)
	{
		time_t baseTime = time_t(date);
		// convert UTC time_t into local time, including daillight adjustment
		struct tm *converted = localtime(&baseTime);

		if (converted == NULL)
		{
			nldebug("Failed to convert %dl into a time structure", date);
			return "0000-00-00 00:00:00";
		}

		std::string str;
		str = NLMISC::toString("%4u-%02u-%02u %u:%02u:%02u", 
			converted->tm_year+1900, 
			converted->tm_mon+1, 
			converted->tm_mday,
			converted->tm_hour,
			converted->tm_min,
			converted->tm_sec);

		return str;
	}

	const std::string &escapeString(const std::string &str, CConnection &dbCnx)
	{
		static string buffer;
		// reserve space in the buffer according to the mysql documentation
		buffer.resize(str.size()*2+1);
		unsigned long resultSize = mysql_escape_string((char*)buffer.data(), str.data(), (unsigned long)str.size());
		
		// now, resize to the real size
		buffer.resize(resultSize);

		// job's done
		return buffer;
	}

	void CConnection::addOption(mysql_option option, const char *value)
	{
		_Options[option] = value;
	}

	void CConnection::clearOption(mysql_option option)
	{
		_Options.erase(option);
	}


	bool CConnection::connect(const TParsedCommandLine &dbInfo)
	{
		const TParsedCommandLine *dbHost= dbInfo.getParam("host");
		const TParsedCommandLine *dbport= dbInfo.getParam("port");
		const TParsedCommandLine *dbUser= dbInfo.getParam("user");
		const TParsedCommandLine *dbPassword= dbInfo.getParam("password");
		const TParsedCommandLine *dbBase= dbInfo.getParam("base");
		if (dbHost == NULL 
			|| dbUser == NULL
			|| dbPassword == NULL
			|| dbBase == NULL)
		{
			nlwarning("LS : Invalid database configuration");
			if (MSWStrictMode)
			{
				nlstopex(("SQL Strict mode, the above error is fatal"));
			}
			return false;
		}

		// connect to the database
		return connect(dbHost->ParamValue, dbUser->ParamValue, dbPassword->ParamValue, dbBase->ParamValue);
	}

	bool CConnection::connect(const std::string &hostName, const std::string &userName, const std::string &password, const std::string &defaultDatabase)
	{
		// store the connection info
		_ConnHostName = hostName;
		_ConnUserName = userName;
		_ConnPassword = password;
		_ConnDefaultDatabase = defaultDatabase;

		nlassert(!_Connected);

		if (MSWAutoReconnect)
		{
			addOption(MYSQL_OPT_RECONNECT, "1");
		}		

		return _connect();
	}

	bool CConnection::_connect()
	{
		_Connected = false;

		if (_MysqlContext != NULL)
		{
			mysql_close(_MysqlContext);
		}

		// init the context
		_MysqlContext = mysql_init(NULL);

		// set the options
		TOptions::iterator first(_Options.begin()), last(_Options.end());
		for (; first != last; ++first)
		{
			const mysql_option &option = first->first;
			const char *value = first->second;
			mysql_options(_MysqlContext, option, value);
		}

		MYSQL *res = mysql_real_connect(_MysqlContext, 
										_ConnHostName.c_str(), 
										_ConnUserName.c_str(),
										_ConnPassword.c_str(), 
										_ConnDefaultDatabase.c_str(), 
										0,
										NULL,
										CLIENT_FOUND_ROWS);

		if (res == NULL)
		{
			nlwarning("Error during connection to database '%s:%s@%s' :", _ConnUserName.c_str(), _ConnDefaultDatabase.c_str(), _ConnHostName.c_str());
			nlwarning("Mysql_real_connect error :%s ", mysql_error(_MysqlContext));
			// the connection failed !
			mysql_close(_MysqlContext);
			_MysqlContext = NULL;
			if (MSWStrictMode)
			{
				nlstopex(("SQL Strict mode, the above error is fatal"));
			}
			return false;
		}

		_Connected = true;
		return true;
	}


	void CConnection::closeConn()
	{
		nlassert(_Connected);

		// the connection failed !
		mysql_close(_MysqlContext);

		_Connected = 0;
		_MysqlContext = NULL;
	}

	bool CConnection::query(const std::string &queryString)
	{
		H_AUTO(CConnection_query);
		nlassert(_Connected);

		nldebug("Executing query : [%s%s", queryString.substr(0, 100).c_str(), 100<queryString.size() ? "]" : "");
		for (uint i=100; i<queryString.size(); i+=100)
			nldebug("\t%s%s", queryString.substr(i, 100).c_str(), i+100>queryString.size() ? "]" : "");

		TTime startDate = CTime::getLocalTime();

		int result = mysql_real_query(_MysqlContext, queryString.c_str(), (unsigned long)queryString.size());
		if (result != 0)
		{
			// in all case, we try to reconnect
			int merrno = mysql_errno(_MysqlContext);
			//if (result == CR_SERVER_GONE_ERROR)
			{
				// reconnect and retry the request
				nlinfo("%p Mysql error errno:%d result:%d : %s, try to reconnect...", _MysqlContext, merrno, result, mysql_error(_MysqlContext));
				if (_connect())
					result = mysql_real_query(_MysqlContext, queryString.c_str(), (unsigned long)queryString.size());
				else
				{
					nlwarning("Failed to reopen closed connection to send query '%s'", queryString.c_str());
					if (MSWStrictMode)
					{
						nlstopex(("SQL Strict mode, the above error is fatal"));
					}
					TTime endDate = CTime::getLocalTime();
					MSWRequestDuration = uint32(endDate - startDate);
					return false;
				}
			}

			if (result != 0)
			{
				nlwarning("Mysql error errno:%d result:%d : %s", merrno, result, mysql_error(_MysqlContext));
				nlwarning("   in query '%s':", queryString.c_str());
				if (MSWStrictMode)
				{
					nlstopex(("SQL Strict mode, the above error is fatal"));
				}
				TTime endDate = CTime::getLocalTime();
				MSWRequestDuration = uint32(endDate - startDate);
				return false;
			}
		}

		TTime endDate = CTime::getLocalTime();
		MSWRequestDuration = uint32(endDate - startDate);

		return true;
	}


	std::auto_ptr<CStoreResult>		CConnection::storeResult()
	{
		H_AUTO(CConnection_storeResult);
		MYSQL_RES *res = mysql_store_result(_MysqlContext);

		std::auto_ptr<CStoreResult> sr = std::auto_ptr<CStoreResult>(new CStoreResult(res));

		return sr;
	}

	std::auto_ptr<CUseResult>		CConnection::useResult()
	{
		H_AUTO(CConnection_useResult);
		MYSQL_RES *res = mysql_use_result(_MysqlContext);

		std::auto_ptr<CUseResult> sr = std::auto_ptr<CUseResult>(new CUseResult(res));

		return sr;
	}
	


	void CResultBase::getDateField(uint32 fieldIndex, uint32 &time)
	{
		const char *str = getRawField(fieldIndex);

		// read from a DATE_TIME field
		MYSQL_FIELD *field = mysql_fetch_field_direct(_Result, fieldIndex);
		nlassert(field != NULL);
		switch (field->type)
		{
		case FIELD_TYPE_DATETIME:
			{
				// format is 'YYYY-MM-DD HH:MM:SS' 
				uint	y, m, d, h, mn, s;
				uint nbScanned = sscanf(str, "%u-%u-%u %u:%u:%u", &y, &m, &d, &h, &mn, &s);
				nlassert(nbScanned == 6);
				tm	myTm;
				myTm.tm_year = y-1900;
				myTm.tm_mon = m-1;
				myTm.tm_mday = d;
				myTm.tm_hour = h;
				myTm.tm_min = mn;
				myTm.tm_sec = s;

				myTm.tm_isdst = -1; // let the C runtime determine daylight adjustment
				myTm.tm_wday = -1;
				myTm.tm_yday = -1;
				// Convert the local date into a UTC unix time
				uint32 t = (uint32)nl_mktime(&myTm);

				time = t;
			}
			break;
		case FIELD_TYPE_DATE:
			{
				// format is 'YYYY-MM-DD' 
				uint	y, m, d;
				uint nbScanned = sscanf(str, "%u-%u-%u", &y, &m, &d);
				nlassert(nbScanned == 3);
				tm	myTm;
				myTm.tm_year = y-1900;
				myTm.tm_mon = m-1;
				myTm.tm_mday = d;
				myTm.tm_hour = 0;
				myTm.tm_min = 0;
				myTm.tm_sec = 0;

				myTm.tm_isdst = -1; // let the C runtime determine daylight adjustment
				myTm.tm_wday = -1;
				myTm.tm_yday = -1;
				// Convert the local date into a UTC unix time
				uint32 t = (uint32)nl_mktime(&myTm);

				time = t;
			}
			break;
		case FIELD_TYPE_TIME:
			{
				// format is 'HH:MM:SS' 
				uint	h, m, s;
				uint nbScanned = sscanf(str, "%u:%u:%u", &h, &m, &s);
				nlassert(nbScanned == 3);
				tm myTm;
				myTm.tm_year = 0;
				myTm.tm_mon = 0;
				myTm.tm_mday = 0;
				myTm.tm_hour = h;
				myTm.tm_min = m;
				myTm.tm_sec = s;

				myTm.tm_isdst = -1; // let the C runtime determine daylight adjustment
				myTm.tm_wday = -1;
				myTm.tm_yday = -1;
				// Convert the local date into a UTC unix time
				uint32 t = (uint32)nl_mktime(&myTm);

				time = t;
			}
			break;
		case FIELD_TYPE_YEAR:
			{
				// format is 'YYYY' 
				uint	y;
				uint nbScanned = sscanf(str, "%u", &y);
				nlassert(nbScanned == 1);
				tm	myTm;
				myTm.tm_year = y-1900;
				myTm.tm_mon = 0;
				myTm.tm_mday = 0;
				myTm.tm_hour = 0;
				myTm.tm_min = 0;
				myTm.tm_sec = 0;

				myTm.tm_isdst = -1; // let the C runtime determine daylight adjustment
				myTm.tm_wday = -1;
				myTm.tm_yday = -1;
				// Convert the local date into a UTC unix time
				uint32 t = (uint32)nl_mktime(&myTm);

				time = t;
			}
			break;
		default:
			{
				// any other case, consider the field as a second unix time (seconds since 1970)
				// format is 'SSSSSSSSSS..' 
				uint32 t;
				getField(fieldIndex, t);
				time = t;
			}
		}
	}

} // namespace MSW

namespace NOPE
{

	// allowed transition table
	bool AllowedTransition [os_nb_state][os_nb_state] =
	{
				//	transient	clean	dirty	removed	released
/*transient*/	{	true,		true,	true,	false,	false	},
/*clean*/		{	false,		true,	true,	true,	true	},
/*dirty*/		{	false,		true,	true,	true,	false	},
/*removed*/		{	false,		false,	false,	false,	false	},
/*released*/	{	false,		false,	false,	false,	false	},
	};

	NLMISC_SAFE_SINGLETON_IMPL(CPersistentCache);

	NLMISC_CATEGORISED_DYNVARIABLE("nope", bool, NOPEAllowReleaseDirtyObject, "Set to true to allow dirty object to be removed from cache (this mean some modified data are lost)")
	{
		if (get)
			*pointer = AllowedTransition[os_dirty][os_released];
		else
			AllowedTransition[os_dirty][os_released] = (*pointer);
	}

} // namespace NOPE
