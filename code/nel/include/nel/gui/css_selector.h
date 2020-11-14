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

#ifndef CL_CSS_SELECTOR_H
#define CL_CSS_SELECTOR_H

#include "nel/misc/types_nl.h"

namespace NLGUI
{
	class CHtmlElement;

	/**
	 * \brief CSS selector
	 * \date 2019-03-15 10:50 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CCssSelector
	{
	public:
		enum ECombinator {
			NONE = 0,
			GENERAL_CHILD,
			ADJACENT_SIBLING,
			GENERAL_SIBLING,
			CHILD_OF
		};

		struct SAttribute {
			std::string key;
			std::string value;
			char op; // =, ~, |, ^, $, *
			bool caseSensitive;
			SAttribute(const std::string &k, const std::string &v, char o, bool cs)
				:key(k),value(v),op(o), caseSensitive(cs)
			{}
		};

		std::string Element;
		std::string Id;
		std::vector<std::string> Class;
		std::vector<SAttribute> Attr;
		std::vector<std::string> PseudoClass;

		// css combinator or \0 missing (first element)
		char Combinator;

	public:
		// TODO: rewrite for ECombinator enum
		CCssSelector(std::string elm="", std::string id="", std::string cls="", char comb = '\0');

		// helper for sorting
		uint32 specificity() const;

		// set classes used, eg 'class1 class2'
		void setClass(const std::string &cls);

		// add attribute to selector
		// ' ' op means 'key exists, ignore value'
		// cs case-sensitive true|false
		void addAttribute(const std::string &key, const std::string &val = "", char op = ' ', bool cs = true);

		// add pseudo class to selector, eg 'first-child'
		void addPseudoClass(const std::string &key);

		// true if no rules have been defined
		bool empty() const
		{
			return Element.empty() && Id.empty() && Class.empty() && Attr.empty() && PseudoClass.empty();
		}

		// Test current selector to html DOM element
		// NOTE: Does not check combinator
		bool match(const CHtmlElement &elm) const;

		// debug
		std::string toString() const;

	private:
		bool matchClass(const CHtmlElement &elm) const;
		bool matchAttributes(const CHtmlElement &elm) const;
		bool matchPseudoClass(const CHtmlElement &elm) const;

		// match An+B rule to child index (1 based)
		bool matchNth(sint childNr, sint a, sint b) const;

		// parse nth-child string to 'a' and 'b' components
		// :nth-child(odd)
		// :nth-child(even)
		// :nth-child(An+B)
		// :nth-child(-An+b)
		void parseNth(const std::string &pseudo, sint &a, sint &b) const;

	};

}//namespace

#endif // CL_CSS_SELECTOR_H

