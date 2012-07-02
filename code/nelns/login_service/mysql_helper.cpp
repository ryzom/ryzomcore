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

#include "nel/misc/types_nl.h"

#include "nel/net/service.h"

#include "mysql_helper.h"

#include "mysql_version.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Variables
//

static string DatabaseName, DatabaseHost, DatabaseLogin, DatabasePassword;

MYSQL *DatabaseConnection = NULL;


//
// Functions
//

string sqlQuery(const string &query)
{
	CMysqlResult result;
	MYSQL_ROW row;
	sint32 nbrow;
	return sqlQuery(query, nbrow, row, result);
}

string sqlQuery(const string &query, sint32 &nbRow, MYSQL_ROW &firstRow, CMysqlResult &result)
{
	nlassert(DatabaseConnection);
	//nlinfo("sqlQuery: '%s'", query.c_str());
	sint ret = mysql_query(DatabaseConnection, query.c_str());
	if(ret != 0)
	{
		nlwarning("mysql_query() failed: '%s' (%s)", mysql_error(DatabaseConnection), query.c_str());
		return toString("mysql_query() failed: '%s' (%s)", mysql_error(DatabaseConnection), query.c_str());
	}

	if(query.find("select") == 0)
	{
		// store result on select query
		result = mysql_store_result(DatabaseConnection);
		if(result == 0)
		{
			nlwarning("mysql_store_result() failed: '%s' (%s)", mysql_error(DatabaseConnection), query.c_str ());
			return toString("mysql_store_result() failed: '%s' (%s)", mysql_error(DatabaseConnection), query.c_str ());
		}

		nbRow = (sint32)mysql_num_rows(result);

		if(nbRow > 0)
		{
			firstRow = mysql_fetch_row(result);
			if(firstRow == 0)
			{
				nlwarning("mysql_fetch_row failed: %s (%s)", mysql_error(DatabaseConnection), query.c_str());
				return toString("mysql_fetch_row failed: %s (%s)", mysql_error(DatabaseConnection), query.c_str());
			}
		}
		else
		{
			firstRow = 0;
		}
	}

	return "";
}

string resetDatabase()
{
	// Reset all shards database
	string reason = sqlQuery("update shard set NbPlayers=0, Online=0");
	if(!reason.empty()) return reason;

	// Reset all user database
	reason = sqlQuery("update user set State='Offline', ShardId=-1, Cookie='' where State!='Offline'");
	if(!reason.empty()) return reason;

	return "";
}

static void cbDatabaseVar(CConfigFile::CVar &var)
{
	DatabaseName = IService::getInstance()->ConfigFile.getVar("DatabaseName").asString ();
	DatabaseHost = IService::getInstance()->ConfigFile.getVar("DatabaseHost").asString ();
	DatabaseLogin = IService::getInstance()->ConfigFile.getVar("DatabaseLogin").asString ();
	DatabasePassword = IService::getInstance()->ConfigFile.getVar("DatabasePassword").asString ();

	if(DatabaseConnection)
	{
		mysql_close(DatabaseConnection);
		DatabaseConnection = 0;
	}
	MYSQL *db = mysql_init(0);
	if(db == 0)
	{
		nlwarning("mysql_init() failed");
		return;
	}

	my_bool opt = true;
	if (mysql_options (db, MYSQL_OPT_RECONNECT, &opt))
	{
		mysql_close(db);
		DatabaseConnection = 0;
		nlerror("mysql_options() failed for database connection to '%s'", DatabaseHost.c_str());
		return;
	}


	DatabaseConnection = mysql_real_connect(db, DatabaseHost.c_str(), DatabaseLogin.c_str(), DatabasePassword.c_str(), DatabaseName.c_str(),0,0,0);
	if (DatabaseConnection == 0 || DatabaseConnection != db)
	{
		mysql_close(db);
		DatabaseConnection = 0;
		nlerror("mysql_real_connect() failed to '%s' with login '%s' and database name '%s'", DatabaseHost.c_str(), DatabaseLogin.c_str(), DatabaseName.c_str());
		return;
	}

#if MYSQL_VERSION_ID < 50019
	opt = true;
	if (mysql_options (DatabaseConnection, MYSQL_OPT_RECONNECT, &opt))
	{
		mysql_close(db);
		DatabaseConnection = 0;
		nlerror("mysql_options() failed for database connection to '%s'", DatabaseHost.c_str());
		return;
	}
#endif


	sqlQuery("set names utf8");
}

void sqlInit()
{
	IService::getInstance()->ConfigFile.setCallback ("ForceDatabaseReconnection", cbDatabaseVar);
	cbDatabaseVar (IService::getInstance()->ConfigFile.getVar ("ForceDatabaseReconnection"));
	resetDatabase();
}
