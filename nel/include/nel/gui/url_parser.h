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

#ifndef CL_URL_PARSER_H
#define CL_URL_PARSER_H

#include <string>

namespace NLGUI
{
	/**
	 * Simple URL parser
	 * \author Meelis Mägi
	 * \date 2015
	 */
	class CUrlParser
	{
	public:
		CUrlParser(){}

		// parse uri to components
		CUrlParser(const std::string &url);

		// parse uri to components
		void parse(std::string uri);

		// serialize URL back to string
		std::string toString() const;

		// inherit scheme, domain, path from given url
		void inherit(const std::string &url);

		// if current parts can compose absolute url or not
		bool isAbsolute() const;

		// resolve relative path like './a/../b' to absolute path '/a/b'
		static void resolveRelativePath(std::string &path);

	public:
		std::string scheme;
		std::string authority;
		std::string host;
		std::string path;
		std::string query;
		std::string hash;
	};

}// namespace

#endif // CL_URL_PARSER_H

