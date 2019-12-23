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

#include "mongo_wrapper.h"

#ifdef HAVE_MONGO

using namespace std;
using namespace NLMISC;
using namespace NLNET;

CVariable<string> MongoPassword("mongo", "MongoPassword", "Set the password to access mongo", string(""), 0, true);
CVariable<string> ChatDbName("mongo", "ChatDbName", "Set the mongo db", string(""), 0, true);
CVariable<string> ChatServer("mongo", "ChatServer", "Set the server host", string(""), 0, true);

DBClientConnection CMongo::conn(true);
string CMongo::dbname;


void CMongo::init()
{
	dbname = ChatDbName.get();
	
	try
	{
		bool res;
		string errmsg;

		res = conn.connect(ChatServer.get(), errmsg);
		if (!res)
			nlerror("mongo: init failed, cannot connect '%s'", errmsg.c_str());
		else
			nlinfo("mongo: connection ok");

		/*res = conn.auth(dbname, "megacorp", MongoPassword.get(), errmsg);
		if(!res) nlerror("mongo: init failed, cannot auth '%s'", errmsg.c_str());
		else nlinfo("mongo: auth ok");*/
	}
	catch(const DBException& e)
	{
		nlerror("mongo: init failed, caught DBException '%s'", e.toString().c_str());
	}
}

void CMongo::insert(const string &collection, const string &json)
{
	nlinfo("mongo: try to insert into '%s': '%s'", collection.c_str(), json.c_str());

	try
	{
		conn.insert(dbname+"."+collection, fromjson(json));
		string e = conn.getLastError();
		if(!e.empty()) nlwarning("mongo: insert failed '%s'", e.c_str());
	}
	catch(const DBException& e)
	{
		nlwarning("mongo: insert failed, caught DBException '%s'", e.toString().c_str());
	}
}

CUniquePtr<DBClientCursor> CMongo::query(const string &collection, const string &json)
{
//	nlinfo("mongo: try to query in '%s': '%s'", collection.c_str(), json.c_str());

	try
	{
		return conn.query(dbname+"."+collection, json);
	}
	catch(const DBException& e)
	{
		nlwarning("mongo: query failed, caught DBException '%s'", e.toString().c_str());
		return CUniquePtr<DBClientCursor>();
	}
}

void CMongo::update(const string &collection, const string &jsonQuery, const string &jsonObj, bool upsert, bool multi)
{
	nlinfo("mongo: try to update in '%s' '%s': '%s'", collection.c_str(), jsonQuery.c_str(), jsonObj.c_str());

	try
	{
		conn.update(dbname+"."+collection, jsonQuery, fromjson(jsonObj), upsert, multi);
	}
	catch(const DBException& e)
	{
		nlwarning("mongo: update failed, caught DBException '%s'", e.toString().c_str());
	}
}

void CMongo::remove(const string &collection, const string &jsonQuery, bool justOne)
{
	nlinfo("mongo: try to delete in '%s' : '%s'", collection.c_str(), jsonQuery.c_str());

	try
	{
		conn.remove(dbname+"."+collection, jsonQuery, justOne);
	}
	catch(const DBException& e)
	{
		nlwarning("mongo: update failed, caught DBException '%s'", e.toString().c_str());
	}
}


string CMongo::quote(const string &s)
{
	string ret;
	for ( std::string::const_iterator i = s.begin(); i != s.end(); ++i )
	{
		switch ( *i )
		{
		case '"':
			ret += "\\\"";
			break;
		case '\'':
			ret += "\\'";
			break;
		case '\\':
			ret += "\\\\";
			break;
		case '\b':
			ret += "\\b";
			break;
		case '\f':
			ret += "\\f";
			break;
		case '\n':
			ret += "\\n";
			break;
		case '\r':
			ret += "\\r";
			break;
		case '\t':
			ret += "\\t";
			break;
		default:
			if ( *i >= 0 && *i <= 0x1f )
			{
				//TODO: these should be utf16 code-units not bytes
				char c = *i;
				ret += "\\u00" + toHexLower(&c, 1);
			}
			else
			{
				ret += *i;
			}
		}
	}
	return ret;
}

#endif
