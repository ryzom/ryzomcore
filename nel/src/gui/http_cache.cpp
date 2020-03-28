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
#include "nel/gui/http_cache.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

#if defined(GCC_VERSION) && !defined(CLANG_VERSION) && defined(NL_ISO_CPP0X_AVAILABLE) && (GCC_VERSION <= 40804)
// hack to fix std::map::erase wrong return type (void instead of iterator in C++11) in GCC 4.8.4
#undef NL_ISO_CPP0X_AVAILABLE
#endif

namespace NLGUI
{
	CHttpCache* CHttpCache::instance = NULL;

	CHttpCache* CHttpCache::getInstance()
	{
		if (!instance)
		{
			instance = new CHttpCache();
		}

		return instance;
	}

	void CHttpCache::release()
	{
		delete instance;
		instance = NULL;
	}

	CHttpCache::CHttpCache()
		: _Initialized(false)
		, _MaxObjects(100)
	{ };

	CHttpCache::~CHttpCache()
	{
		flushCache();
	}

	void CHttpCache::setCacheIndex(const std::string& fname)
	{
		_IndexFilename = fname;
		_Initialized = false;
	}

	CHttpCacheObject CHttpCache::lookup(const std::string& fname)
	{
		if (!_Initialized)
			init();

		if (_List.count(fname) > 0)
			return _List[fname];

		return CHttpCacheObject();
	}

	void CHttpCache::store(const std::string& fname, const CHttpCacheObject& data)
	{
		if (!_Initialized)
			init();

		_List[fname] = data;
	}

	void CHttpCache::init()
	{
		if (_Initialized)
			return;

		_Initialized = true;

		if (_IndexFilename.empty() || !CFile::fileExists(_IndexFilename))
			return;

		CIFile in;
		if (!in.open(_IndexFilename)) {
			nlwarning("Unable to open %s for reading", _IndexFilename.c_str());
			return;
		}

		serial(in);
	}

	void CHttpCacheObject::serial(NLMISC::IStream& f)
	{
		f.serialVersion(1);
		f.serial(Expires);
		f.serial(LastModified);
		f.serial(Etag);
	}

	void CHttpCache::serial(NLMISC::IStream& f)
	{
		// saved state is ignored when version checks fail
		try {
			f.serialVersion(1);

			// CacheIdx
			f.serialCheck(NELID("hcaC"));
			f.serialCheck(NELID("xdIe"));

			if (f.isReading())
			{
				uint32 numFiles;
				f.serial(numFiles);

				_List.clear();
				for (uint k = 0; k < numFiles; ++k)
				{
					std::string fname;
					f.serial(fname);

					CHttpCacheObject obj;
					obj.serial(f);

					_List[fname] = obj;
				}
			}
			else
			{
				uint32 numFiles = _List.size();
				f.serial(numFiles);

				for (THttpCacheMap::iterator it = _List.begin(); it != _List.end(); ++it)
				{
					std::string fname(it->first);
					f.serial(fname);

					(*it).second.serial(f);
				}
			}
		} catch (...) {
			_List.clear();
			nlwarning("Invalid cache index format (%s)", _IndexFilename.c_str());
			return;
		}
	}

	void CHttpCache::pruneCache()
	{
		if (_List.size() < _MaxObjects)
			return;

		size_t mustDrop = _List.size() - _MaxObjects;

		time_t currentTime;
		time(&currentTime);

		// if we over object limit, then start removing expired objects
		// this does not guarantee that max limit is reached
		for (THttpCacheMap::iterator it = _List.begin(); it != _List.end();)
		{
			if (it->second.Expires <= currentTime)
			{
#ifdef NL_ISO_CPP0X_AVAILABLE
				it = _List.erase(it);
#else
				THttpCacheMap::iterator itToErase = it++;
				_List.erase(itToErase);
#endif

				--mustDrop;
				if (mustDrop == 0)
					break;
			}
			else
			{
				++it;
			}
		}
	}

	void CHttpCache::flushCache()
	{
		if (_IndexFilename.empty())
			return;

		pruneCache();

		COFile out;
		if (!out.open(_IndexFilename))
		{
			nlwarning("Unable to open %s for writing", _IndexFilename.c_str());
			return;
		}

		serial(out);
		out.close();
	}
}
