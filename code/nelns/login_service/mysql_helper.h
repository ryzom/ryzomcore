// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_MYSQL_HELPER_H
#define NL_MYSQL_HELPER_H


//
// Includes
//

// we have to include windows.h because mysql.h uses it but not include it
#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <winsock2.h>
#	include <windows.h>
typedef unsigned long ulong;
#endif

#include <mysql.h>

#include "nel/misc/types_nl.h"


//
// Variables
//

extern MYSQL *DatabaseConnection;


//
// Classes
//

class CMysqlResult
{
public:
	CMysqlResult() : Result(0) { }
	CMysqlResult(MYSQL_RES *result)
	{
		if(Result) mysql_free_result(Result);
		Result = result;
	};

	~CMysqlResult()
	{
		if(Result) mysql_free_result(Result);
	}

	CMysqlResult& operator=(MYSQL_RES *result)
	{
		if(Result) mysql_free_result(Result);
		Result = result;
		return *this;
	}

	operator MYSQL_RES*() { return Result; }

private:
	//we don't want user to do a copy
	CMysqlResult(const CMysqlResult &mysqlResult){}
	MYSQL_RES *Result;
};


//
// Functions
//

void sqlInit();
std::string sqlQuery(const std::string &query);
std::string sqlQuery(const std::string &query, sint32 &nbRow, MYSQL_ROW &firstRow, CMysqlResult &result);

#endif // NL_MYSQL_HELPER_H

/* End of mysql_helper.h */
