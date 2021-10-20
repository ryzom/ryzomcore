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
	class CHtmlElement;

	/**
	 * \brief HTML parsing
	 * \date 2019-03-15 10:50 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CHtmlParser
	{
	public:
		// <link rel=stylesheet>
		struct StyleLink
		{
			uint Index;
			std::string Url;
			StyleLink(uint i, const std::string &url) : Index(i), Url(url)
			{ }
		};

		bool parseHtml(std::string htmlString) const;

		// parse html string into DOM, extract <style> and <link stylesheet> urls
		void getDOM(std::string htmlString, CHtmlElement &parent, std::vector<std::string> &styles, std::vector<StyleLink> &links) const;

	private:
		// iterate over libxml html tree, build DOM
		void parseNode(xmlNode *a_node, CHtmlElement &parent, std::vector<std::string> &styles, std::vector<StyleLink> &links) const;

		// read <style> tag and add its content to styleString
		void parseStyle(xmlNode *a_node, std::string &styleString) const;

		// update parent/sibling in elm.Children
		void reindexChilds(CHtmlElement &elm) const;
	};
}

#endif

