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
#include "nel/misc/common.h"
#include "nel/gui/url_parser.h"

using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	// ***************************************************************************
	CUrlParser::CUrlParser(const std::string &uri)
	{
		parse(uri);
	}

	// ***************************************************************************
	void CUrlParser::parse(std::string uri)
	{
		const size_t npos = std::string::npos;
		size_t pos;
		size_t offset = 0;

		// strip fragment if present
		pos = uri.find("#");
		if (pos != npos)
		{
			hash = uri.substr(pos + 1);
			uri = uri.substr(0, pos);
		}

		// scan for scheme
		pos = uri.find(":");
		if (pos != npos && pos >= 1)
		{
			for (uint i=0; i<pos; i++)
			{
				if (!isalnum(uri[i]))
				{
					pos = npos;
					break;
				}
			}
			if (pos != npos)
			{
				scheme = uri.substr(0, pos);
				uri = uri.substr(pos + 1);
			}
		}

		// scan for authority
		if (uri.substr(0, 2) == "//")
		{
			pos = uri.find_first_of("/?", 2);
			authority = uri.substr(0, pos);
			if (pos != npos)
				uri = uri.substr(pos);
			else
				uri.clear();

			// strip empty port from authority
			if (authority.find_last_of(":") == authority.length() - 1)
				authority = authority.substr(0, authority.length() - 1);

			// extract host from user:pass@host:port
			pos = authority.find("@");
			if (pos != npos)
				host = authority.substr(pos + 1);
			else
				host = authority.substr(2);

			// case-insensitive
			host = NLMISC::toLowerAscii(host);

			pos = host.find(":");
			if (pos != npos)
				host = host.substr(0, pos);
		}

		// scan for query
		pos = uri.find("?");
		if (pos != npos)
		{
			query = uri.substr(pos);
			uri = uri.substr(0, pos);
		}

		// all that is remaining is path
		path = uri;
	}

	void CUrlParser::inherit(const std::string &url)
	{
		// we have scheme, so we already absolute url
		if (!scheme.empty())
			return;

		const size_t npos = std::string::npos;
		size_t pos;

		CUrlParser base(url);

		scheme = base.scheme;

		// if we already have authority, then ignore base path
		if (!authority.empty())
			return;

		authority = base.authority;
		if (path.empty())
		{
			path = base.path;
			if (query.empty())
				query = base.query;
		}
		else
		if (path[0] != '/')
		{
			// find start of last path segment from base path
			// if not found, then dont inherit base path at all
			pos = base.path.find_last_of("/");
			if (pos != npos)
				path = base.path.substr(0, pos) + "/" + path;
		}

		resolveRelativePath(path);
	}

	void CUrlParser::resolveRelativePath(std::string &path)
	{
		const size_t npos = std::string::npos;

		// no relative components in path. filename.ext is also matched, but that's fine
		size_t pos = path.find(".");
		if (pos == npos)
			return;

		// normalize path
		size_t lhp = 0;
		while(pos < path.size())
		{
			if (path[pos] == '.')
			{
				// scan ahead to see what we have
				std::string sub = path.substr(pos, 2);
				if (sub == "./" || sub == ".")
				{
					// starts with
					if (pos == 0)
						path.replace(pos, sub.size(), "/");
					else
					{
						// full or last segment
						sub = path.substr(pos-1, 3);
						if (sub == "/./" || sub == "/.")
						{
							path.replace(pos, sub.size()-1, "");
							// we just removed char that pos was pointing, so rewind
							pos--;
						}
					}
				}
				else
				if (sub == "..")
				{
					// starts with
					if (pos == 0 && path.substr(pos, 3) == "../")
						path.replace(pos, 3, "/");
					else
					if (pos > 0)
					{
						// full or last segment
						sub = path.substr(pos-1, 4);
						if (sub == "/../" || sub == "/..")
						{
							if (pos > 1)
								lhp = path.find_last_of("/", pos - 2);
							else
								lhp = 0;

							// pos points to first dot in ..
							// lhp points to start slash (/) of last segment
							pos += sub.size() - 1;
							path.replace(lhp, pos - lhp, "/");
							pos = lhp;
						}
					}
				}// sub == ".."
			} // path[pos] == '.'
			pos++;
		}// while
	}

	bool CUrlParser::isAbsolute() const
	{
		return !scheme.empty() && !authority.empty();
	}

	// serialize URL back to string
	std::string CUrlParser::toString() const
	{
		std::string result;
		if (!scheme.empty())
			result += scheme + ":";

		if (!authority.empty())
		{
			result += authority;
		}

		// path already has leading slash
		if (!path.empty())
		{
			result += path;
		}

		if (!query.empty())
		{
			if (query.find_first_of("?") != 0) result += "?";

			result += query;
		}

		if (!hash.empty())
		{
			result += "#" + hash;
		}

		return result;
	}

}// namespace

