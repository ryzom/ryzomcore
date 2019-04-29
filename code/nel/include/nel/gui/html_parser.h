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

#ifndef CL_HTML_PARSER_H
#define CL_HTML_PARSER_H

#include "nel/misc/types_nl.h"

namespace NLGUI
{
	class CGroupHTML;

	/**
	 * \brief HTML parsing
	 * \date 2019-03-15 10:50 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CHtmlParser
	{
	public:
		CHtmlParser(CGroupHTML *group) : _GroupHtml(group)
		{}

		bool parseHtml(std::string htmlString);

	private:
		// libxml2 html parser functions
		void htmlElement(xmlNode *node, int element_number);
		void parseNode(xmlNode *a_node);

	private:

		CGroupHTML *_GroupHtml;
	};

}

#endif

