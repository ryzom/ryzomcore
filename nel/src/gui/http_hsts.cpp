// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2018  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/http_hsts.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI {
	CStrictTransportSecurity* CStrictTransportSecurity::instance = NULL;
	CStrictTransportSecurity* CStrictTransportSecurity::getInstance()
	{
		if (!instance)
		{
			instance= new CStrictTransportSecurity();
		}
		return instance;
	}

	void CStrictTransportSecurity::release()
	{
		delete instance;
		instance = NULL;
	}

	CStrictTransportSecurity::~CStrictTransportSecurity()
	{
		save();
	}

	// ************************************************************************
	bool CStrictTransportSecurity::isSecureHost(const std::string &domain) const
	{
		SHSTSObject hsts;
		if (get(domain, hsts))
		{
			time_t currentTime;
			time(&currentTime);

			return (hsts.Expires > currentTime);
		}

		return false;
	}

	// ************************************************************************
	void CStrictTransportSecurity::erase(const std::string &domain)
	{
		if (_Domains.count(domain) > 0)
		{
			_Domains.erase(domain);
		}
	}

	void CStrictTransportSecurity::set(const std::string &domain, uint64 expires, bool includeSubDomains)
	{
		if (expires == 0)
		{
			erase(domain);
			return;
		}

		_Domains[domain].Expires = expires;
		_Domains[domain].IncludeSubDomains = includeSubDomains;
	}

	bool CStrictTransportSecurity::get(const std::string &domain, SHSTSObject &hsts) const
	{
		if (domain.empty() || _Domains.empty())
			return false;

		THSTSObjectMap::const_iterator itHsts;
		
		itHsts = _Domains.find(domain);
		if (itHsts != _Domains.end())
		{
			hsts = itHsts->second;
			return true;
		}

		size_t firstOf = domain.find_first_of(".");
		size_t lastOf = domain.find_last_of(".");
		while(firstOf != lastOf)
		{
			std::string tmp;
			tmp = domain.substr(firstOf+1);
			itHsts = _Domains.find(tmp);
			if (itHsts != _Domains.end())
			{
				if (itHsts->second.IncludeSubDomains)
				{
					hsts = itHsts->second;
					return true;
				}

				return false;
			}

			firstOf = domain.find_first_of(".",  firstOf + 1);
		}

		return false;
	}

	void CStrictTransportSecurity::init(const std::string &fname)
	{
		_Domains.clear();
		_Filename = fname;

		if (_Filename.empty() || !CFile::fileExists(_Filename))
		{
			return;
		}

		CIFile in;
		if (!in.open(_Filename))
		{
			nlwarning("Unable to open %s for reading", _Filename.c_str());
			return;
		}

		serial(in);
	}

	void CStrictTransportSecurity::save()
	{
		if (_Filename.empty())
			return;

		if (_Domains.empty())
		{
			CFile::deleteFile(_Filename);
			return;
		}

		COFile out;
		if (!out.open(_Filename))
		{
			nlwarning("Unable to open %s for writing", _Filename.c_str());
			return;
		}

		serial(out);
		out.close();
	}

	void CStrictTransportSecurity::serial(NLMISC::IStream& f)
	{
		try
		{
			f.serialVersion(1);
			// HSTS
			f.serialCheck(NELID("STSH"));

			if (f.isReading())
			{
				uint32 nbItems;
				f.serial(nbItems);
				for(uint32 k = 0; k < nbItems; ++k)
				{
					std::string domain;
					f.serial(domain);
					f.serial(_Domains[domain].Expires);
					f.serial(_Domains[domain].IncludeSubDomains);
				}
			}
			else
			{
				uint32 nbItems = _Domains.size();
				f.serial(nbItems);
				for (THSTSObjectMap::iterator it = _Domains.begin(); it != _Domains.end(); ++it)
				{
					std::string domain(it->first);
					f.serial(domain);
					f.serial(_Domains[domain].Expires);
					f.serial(_Domains[domain].IncludeSubDomains);
				}
			}
		}
		catch (...)
		{
			_Domains.clear();
			nlwarning("Invalid HTST file format (%s)", _Filename.c_str());
		}
	}

	// ***************************************************************************
	void CStrictTransportSecurity::setFromHeader(const std::string &domain, const std::string &header)
	{
		// max-age=<seconds>; includeSubdomains; preload;
		std::vector<std::string> elements;
		NLMISC::splitString(toLowerAscii(header), ";", elements);
		if (elements.empty()) return;

		time_t currentTime;
		time(&currentTime);

		uint64 expire = 0;
		bool includeSubDomains = false;

		for(uint i=0; i< elements.size(); ++i)
		{
			std::string str(trim(elements[i]));
			if (str.substr(0, 8) == "max-age=")
			{
				uint64 ttl;
				if (fromString(str.substr(8), ttl))
				{
					if (ttl > 0)
					{
						expire = currentTime + ttl;
					}
				}
			}
			else if (str == "includesubdomains")
			{
				includeSubDomains = true;
			}
		}

		if (expire == 0)
		{
			erase(domain);
		}
		else
		{
			set(domain, expire, includeSubDomains);
		}
	}

}
