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

#include <string>
#include "nel/misc/types_nl.h"
#include "nel/gui/css_selector.h"
#include "nel/gui/html_element.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	CCssSelector::CCssSelector(std::string elm, std::string id, std::string cls, char comb)
		: Element(elm), Id(id), Class(), Attr(), PseudoClass(), Combinator(comb)
	{
		if (!cls.empty())
		{
			setClass(cls);
		}
	}

	uint32 CCssSelector::specificity() const
	{
		uint ret = 0;

		if (!Element.empty() && Element != "*") ret += 0x000001;
		// Pseudo Element is added in CCssStyle
		//if (!PseudoElement.empty())             ret += 0x000001;

		if (!Class.empty())                     ret += 0x000100 * Class.size();
		if (!Attr.empty())                      ret += 0x000100 * Attr.size();
		// TODO: has different cases
		if (!PseudoClass.empty())               ret += 0x000100 * PseudoClass.size();

		if (!Id.empty())                        ret += 0x010000;

		return ret;
	}

	void CCssSelector::setClass(const std::string &cls)
	{
		std::vector<std::string> parts;
		NLMISC::splitString(toLowerAscii(cls), ".", parts);

		for(uint i = 0; i< parts.size(); i++)
		{
			std::string cname = trim(parts[i]);
			if (!cname.empty())
			{
				Class.push_back(cname);
			}
		}
	}

	void CCssSelector::addAttribute(const std::string &key, const std::string &val, char op, bool cs)
	{
		if (cs)
		{
			// case sensitive match
			Attr.push_back(SAttribute(key, val, op, cs));
		}
		else
		{
			Attr.push_back(SAttribute(key, toLowerAscii(val), op, cs));
		}
	}

	void CCssSelector::addPseudoClass(const std::string &key)
	{
		if (key.empty()) return;

		PseudoClass.push_back(key);
	}

	bool CCssSelector::match(const CHtmlElement &elm) const
	{
		if (!Element.empty() && Element != "*" && Element != elm.Value)
		{
			return false;
		}

		if (!Id.empty() && Id != elm.getAttribute("id"))
		{
			return false;
		}

		if (!Class.empty() && !matchClass(elm))
		{
			return false;
		}

		if (!Attr.empty() && !matchAttributes(elm))
		{
			return false;
		}

		if (!PseudoClass.empty() && !matchPseudoClass(elm))
		{
			return false;
		}

		return true;
	}

	bool CCssSelector::matchClass(const CHtmlElement &elm) const
	{
		// make sure all class names we have, other has as well
		for(uint i = 0; i< Class.size(); ++i)
		{
			if (!elm.hasClass(Class[i]))
			{
				return false;
			}
		}

		return true;
	}

	bool CCssSelector::matchAttributes(const CHtmlElement &elm) const
	{
		// TODO: refactor into matchAttributeSelector
		for(uint i = 0; i< Attr.size(); ++i)
		{
			if (!elm.hasAttribute(Attr[i].key)) return false;

			std::string value = elm.getAttribute(Attr[i].key);
			// case-insensitive compare, Attr.value is already lowercased
			if (!Attr[i].caseSensitive)
			{
				value = toLowerAscii(value);
			}

			switch(Attr[i].op)
			{
				case '=':
					if (Attr[i].value != value) return false;
					break;
				case '~':
					{
						// exact match to any of whitespace separated words from element attribute
						if (Attr[i].value.empty()) return false;

						std::vector<std::string> parts;
						NLMISC::splitString(value, " ", parts);
						bool found = false;
						for(uint j = 0; j < parts.size(); j++)
						{
							if (Attr[i].value == parts[j])
							{
								found = true;
								break;
							}
						}
						if (!found) return false;
					}
					break;
				case '|':
					// exact value, or start with val+'-'
					if (value != Attr[i].value && value.find(Attr[i].value + "-") == std::string::npos) return false;
					break;
				case '^':
					// prefix, starts with
					if (Attr[i].value.empty()) return false;
					if (value.find(Attr[i].value) != 0) return false;
					break;
				case '$':
					// suffic, ends with
					if (Attr[i].value.empty() || value.size() < Attr[i].value.size()) return false;
					if (Attr[i].value == value.substr(value.size() - Attr[i].value.size())) return false;
					break;
				case '*':
					if (Attr[i].value.empty()) return false;
					if (value.find(Attr[i].value) == std::string::npos) return false;
					break;
				case ' ':
					// contains key, ignore value
					break;
				default:
					// unknown comparison
					return false;
			}
		}

		return true;
	}

	bool CCssSelector::matchPseudoClass(const CHtmlElement &elm) const
	{
		for(uint i = 0; i< PseudoClass.size(); i++)
		{
			if (PseudoClass[i] == "root")
			{
				// ':root' is just 'html' with higher specificity
				if (elm.Value != "html") return false;
			}
			else if (PseudoClass[i] == "only-child")
			{
				if (elm.parent && !elm.parent->Children.empty()) return false;
			}
			else
			{
				if (PseudoClass[i] == "first-child")
				{
					if (elm.previousSibling) return false;
				}
				else if (PseudoClass[i] == "last-child")
				{
					if (elm.nextSibling) return false;
				}
				else if (PseudoClass[i].find("nth-child(") != std::string::npos)
				{
					sint a, b;
					// TODO: there might be multiple :nth-child() on single selector, so current can't cache it
					parseNth(PseudoClass[i], a, b);

					// 1st child should be '1' and not '0'
					if (!matchNth(elm.childIndex+1, a, b)) return false;
				}
				else
				{
					return false;
				}
			}
		}

		return true;
	}

	void CCssSelector::parseNth(const std::string &pseudo, sint &a, sint &b) const
	{
		a = 0;
		b = 0;

		std::string::size_type start = pseudo.find_first_of("(") + 1;
		std::string::size_type end = pseudo.find_first_of(")");

		if (start == std::string::npos) return;

		std::string expr = toLowerAscii(pseudo.substr(start, end - start));

		if (expr.empty()) return;

		if (expr == "even")
		{
			// 2n+0
			a = 2;
			b = 0;
		}
		else if (expr == "odd")
		{
			// 2n+1
			a = 2;
			b = 1;
		}
		else
		{
			// -An+B, An+B, An-B
			std::string::size_type pos;

			start = 0;
			pos = expr.find_first_of("n", start);
			if (pos == std::string::npos)
			{
				fromString(expr, b);
			}
			else if (pos == 0)
			{
				// 'n' == '1n'
				a = 1;
			}
			else if (expr[0] == '-' && pos == 1)
			{
				// '-n' == '-1n'
				a = -1;
			}
			else
			{
				fromString(expr.substr(start, pos - start), a);
			}

			start = pos;
			pos = expr.find_first_of("+-", start);
			if (pos != std::string::npos && (expr[pos] == '+' || expr[pos] == '-'))
			{
				// copy with sign char
				fromString(expr.substr(pos, end - pos), b);
			}
		}
	}

	bool CCssSelector::matchNth(sint childNr, sint a, sint b) const
	{
		if (a == 0)
		{
			return childNr == b;
		}
		else if (a > 0)
		{
			return childNr >= b && (childNr - b) % a == 0;
		}
		else
		{
			// a is negative from '-An+B'
			return childNr <= b && (b - childNr) % (-a) == 0;
		}
	}

	std::string CCssSelector::toString() const
	{
		std::string ret;
		ret += Element;
		ret += Id;
		if (!Class.empty())
		{
			for(uint i = 0; i<Class.size(); i++)
				ret += "." + Class[i];
		}
		if (!Attr.empty())
		{
			for(uint i = 0; i<Attr.size(); ++i)
			{
				ret += "[" + Attr[i].key;
				if (Attr[i].op != ' ')
				{
					ret += Attr[i].op + Attr[i].value;
				}
				ret += "]";
			}
		}
		if (!PseudoClass.empty())
		{
			for(uint i = 0; i<PseudoClass.size(); ++i)
			{
				ret += ":" + PseudoClass[i];
			}
		}
		if (Combinator != '\0')
		{
			ret += Combinator;
		}

		// ret += ":" + PseudoClass;
		return ret;
	}

} // namespace

