// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2025  Winch Gate Property Limited
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef MEMCACHED_WRAPPER_H
#define MEMCACHED_WRAPPER_H

#ifdef HAVE_MEMCACHED

#include <libmemcached/memcached.h>

class CMemC {
public:
	static void init();
	static void disconnect();
	static bool set(const std::string& key, const std::string& value, time_t expiration = 0);
	static bool setWithIndex(const std::string& key, const std::string& value);
	static std::string get(const std::string& key);
	static std::string getWithIndex(const std::string& key);
	static uint64 incr(const std::string& key, uint32 offset = 1);
	static std::string add(const std::string& key, const std::string& value);
	static std::string del(const std::string& key, const std::string& value);

private:
	static memcached_st* memc;
};

#endif

#endif //  MEMCACHED_WRAPPER_H
