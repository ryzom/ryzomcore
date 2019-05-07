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

#ifndef CL_HTML_ELEMENT_H
#define CL_HTML_ELEMENT_H

#include "nel/misc/types_nl.h"
#include "nel/gui/css_style.h"

namespace NLGUI
{
	/**
	 * \brief HTML element
	 * \date 2019-04-25 18:23 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CHtmlElement
	{
	public:
		enum ENodeType {
			NONE = 0,
			ELEMENT_NODE = 1,
			TEXT_NODE = 3,
		};

		uint ID; // libwww element enum
		ENodeType Type;
		std::string Value; // text node value or element node name
		std::map<std::string, std::string> Attributes;
		std::list<CHtmlElement> Children;

		// class names for css matching
		std::set<std::string> ClassNames;

		// defined style and :before/:after pseudo elements
		TStyle Style;

		// hierarchy
		CHtmlElement *parent;
		CHtmlElement *previousSibling;
		CHtmlElement *nextSibling;

		// n'th ELEMENT_NODE in parent.Children, for :nth-child() rules
		uint childIndex;

		CHtmlElement(ENodeType type = NONE, std::string value = "");

		// returns true if rhs is same pointer
		friend bool operator==(const CHtmlElement &lhs, const CHtmlElement &rhs)
		{
			return &lhs == &rhs;
		}

		bool hasAttribute(const std::string &key) const;

		bool hasNonEmptyAttribute(const std::string &key) const;

		std::string getAttribute(const std::string &key) const;

		bool hasClass(const std::string &key) const;

		// update Children index/parent/next/prevSibling pointers
		void reindexChilds();

		// debug
		std::string toString(bool tree = false, uint depth = 0) const;

		// query, get, set pseudo element style rules
		void clearPseudo();
		bool hasPseudo(const std::string &key) const;
		TStyle getPseudo(const std::string &key) const;
		void setPseudo(const std::string &key, const TStyle &style);

	private:
		// pseudo elements like ":before" and ":after"
		std::map<std::string, TStyle> _Pseudo;
	};
}

#endif

