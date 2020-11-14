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

#ifndef CL_HTTP_CACHE_H
#define CL_HTTP_CACHE_H

#include "nel/misc/types_nl.h"

namespace NLGUI
{
	struct CHttpCacheObject
	{
		CHttpCacheObject(uint32 expires = 0, const std::string& lastModified = "", const std::string& etag = "")
			: Expires(expires)
			, LastModified(lastModified)
			, Etag(etag){};

		uint32 Expires;
		std::string LastModified;
		std::string Etag;

		void serial(NLMISC::IStream& f);
	};

	/**
	 * Keeping track of downloaded files cache related headers
	 * \author Meelis Mägi (nimetu)
	 * \date 2017
	 */
	class CHttpCache
	{
		typedef std::map<std::string, CHttpCacheObject> THttpCacheMap;

	public:
		static CHttpCache* getInstance();
		static void release();

	public:
		void setCacheIndex(const std::string& fname);
		void init();

		CHttpCacheObject lookup(const std::string& fname);
		void store(const std::string& fname, const CHttpCacheObject& data);

		void flushCache();

		void serial(NLMISC::IStream& f);

	private:
		CHttpCache();
		~CHttpCache();

		void pruneCache();

		static CHttpCache* instance;

		THttpCacheMap _List;

		std::string _IndexFilename;
		bool _Initialized;
		size_t _MaxObjects;
	};
}
#endif
