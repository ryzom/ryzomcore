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
#include <nel/misc/algo.h>
#include "memc_wrapper.h"

#ifdef HAVE_MEMCACHED

using namespace std;
using namespace NLMISC;

memcached_st* CMemC::memc = NULL;

void CMemC::init()
{
	if (memc != NULL) return;
	memc = memcached_create(NULL);
	if (!memc)
		nlinfo("memcached: init failed, cannot create it");

	memcached_return rc;
	memcached_server_st* servers = memcached_server_list_append(NULL, "127.0.0.1", 11211, &rc);
	if (rc != MEMCACHED_SUCCESS)
		nlinfo("memcached: init failed, cannot access it");

	rc = memcached_server_push(memc, servers);
	memcached_server_list_free(servers);

	if (rc != MEMCACHED_SUCCESS)
		nlinfo("memcached: init failed, connection failed");
}

void CMemC::disconnect()
{
	if (memc)
	{
		memcached_free(memc);
		memc = NULL;
	}
}

bool CMemC::set(const string& key, const string& value, time_t expiration)
{
	if (!memc)
	{
		nlinfo("memcached: not connected");
		return false;
	}

	nlinfo("MemC: Set %s = %s", key.c_str(), value.c_str());
	memcached_return rc = memcached_set(memc,
		key.c_str(), key.size(),
		value.c_str(), value.size(),
		expiration, 0);
	return rc == MEMCACHED_SUCCESS;
}

bool CMemC::setWithIndex(const string& key, const string& value)
{
	nlinfo("MemC: Set With Index %s-Last = %s", key.c_str(), value.c_str());
	string last = key+"-Last";
	uint64 lastId = incr(last);
	if (lastId == 0)
	{
		lastId = 1;
		set(last, "1");
	}
	nlinfo("MemC: Set With Index %s-%d = %s", key.c_str(), lastId, value.c_str());
	return set(toString("%s-%d", key.c_str(), lastId), value);
}

string CMemC::get(const string& key)
{
	if (!memc)
	{
		nlinfo("memcached: not connected");
		return "";
	}

	size_t value_length;
	uint32 flags;
	memcached_return rc;
	char* result = memcached_get(memc,
		key.c_str(), key.size(),
		&value_length, &flags, &rc);
	if (!result)
		return "";
	string value(result, value_length);
	free(result);
	nlinfo("MemC: Get %s = %s", key.c_str(), value.c_str());
	return value;
}

string CMemC::getWithIndex(const string& key)
{
	string last = key+"-Last";
	string saved = get(last);
	uint64 lastId;
	if (saved.empty())
		lastId = 0;
	else
		fromString(saved, lastId);

	string value = get(toString("%s-%d", key.c_str(), lastId));
	nlinfo("MemC: Get with index %s-%d = %s", key.c_str(), lastId, value.c_str());

}

uint64 CMemC::incr(const string& key, uint32 offset)
{
	if (!memc)
	{
		nlinfo("memcached: not connected");
		return 0;
	}

	uint64 new_value = 0;
	memcached_return rc = memcached_increment(memc,
		key.c_str(), key.size(),
		offset, &new_value);
	if (rc == MEMCACHED_NOTFOUND)
	{
		nlinfo("MemC: Incr %s not exist", key.c_str());
		return 0;
	}
	if (rc != MEMCACHED_SUCCESS)
	{
		nlinfo("memcached: increment error");
		return 0;
	}
	nlinfo("MemC: Incr %s =  %" NL_I64 "d", key.c_str(), new_value);
	return new_value;
}

string CMemC::add(const string& key, const string& value)
{
	string saved = get(key);
	vector<string> parts;
	NLMISC::splitString(saved, "|", parts);
	for (uint i=0; i<parts.size(); i++)
	{
		if (parts[i] == value)
			return "";
	}

	if (set(key, saved+"|"+value))
		return saved+"|"+value;
	return "";
}

string CMemC::del(const string& key, const string& value)
{
	string saved = get(key);
	vector<string> parts;
	NLMISC::splitString(saved, "|", parts);
	string new_value;
	for (uint i=0; i<parts.size(); i++)
	{
		if (parts[i] != value)
		{
			if (new_value.empty())
				new_value += value;
			else
				new_value += "|"+value;
		}
	}

	if (set(key, new_value))
		return new_value;
	return "";
}


#endif
