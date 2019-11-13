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

#ifndef CL_HTTP_HSTS_H
#define CL_HTTP_HSTS_H

#include "nel/misc/types_nl.h"

namespace NLGUI
{
	// ********************************************************************************
	struct SHSTSObject
	{
	public:
		SHSTSObject(uint64 expires = 0, bool includeSubDomains = false)
			: Expires(expires)
			, IncludeSubDomains(includeSubDomains)
		{ }

		uint64 Expires;
		bool IncludeSubDomains;
	};

	/**
	 * Keeping track of HSTS header
	 * \author Meelis Mägi (nimetu)
	 * \date 2017
	 */
	class CStrictTransportSecurity
	{
	public:
		typedef std::map<std::string, SHSTSObject> THSTSObjectMap;

		static CStrictTransportSecurity* getInstance();
		static void release();

	public:
		bool isSecureHost(const std::string &domain) const;

		// ************************************************************************
		void init(const std::string& fname);
		void save();

		void erase(const std::string &domain);
		void set(const std::string &domain, uint64 expires, bool includeSubDomains);
		bool get(const std::string &domain, SHSTSObject &hsts) const;
		void setFromHeader(const std::string &domain, const std::string &header);

		void serial(NLMISC::IStream& f);
	private:
		static CStrictTransportSecurity* instance;

		~CStrictTransportSecurity();

		std::string _Filename;
		THSTSObjectMap _Domains;
	};

}
#endif
