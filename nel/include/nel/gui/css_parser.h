// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef CL_CSS_PARSER_H
#define CL_CSS_PARSER_H

#include "nel/misc/types_nl.h"
#include "nel/gui/css_style.h"
#include "nel/gui/css_selector.h"

namespace NLGUI
{
	/**
	 * \brief CSS style parsing
	 * \date 2019-03-15 10:50 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CCssParser {
	public:
		// parse style declaration, eg "color: red; font-size: 10px;"
		static TStyleVec parseDecls(const std::string &styleString);

		// parse css stylesheet
		void parseStylesheet(const std::string &cssString, std::vector<CCssStyle::SStyleRule> &rules);

	private:
		// stylesheet currently parsed
		std::string _Style;
		// keep track of current position in _Style
		size_t _Position;

		std::vector<CCssStyle::SStyleRule> _Rules;

	private:
		// @media ( .. ) { .. }
		void readAtRule();

		// a#id.class[attr=val] { .. }
		void readRule();

		// move past whitespace
		void skipWhitespace();

		// skip valid IDENT
		bool skipIdentifier();

		// skip over {}, (), or [] block
		void skipBlock();

		// skip over string quoted with ' or "
		void skipString();

		// backslash escape
		void escape();

		// normalize newline chars and remove comments
		void preprocess();

		// parse selectors + combinators
		std::vector<CCssSelector> parse_selector(const std::string &sel, std::string &pseudoElement) const;

		// parse selector and style
		void parseRule(const std::string &selectorString, const std::string &styleString);

		inline bool is_eof() const
		{
			return _Position >= _Style.size();
		}

		inline bool is_whitespace(char ch) const
		{
			return (ch == ' ' || ch == '\t' || ch == '\n');
		}

		inline bool is_hex(char ch) const
		{
			return ((ch >= '0' && ch <= '9') ||
					(ch >= 'a' && ch <= 'f') ||
					(ch >= 'A' && ch <= 'F'));
		}

		inline bool maybe_escape() const
		{
			// escaping newline (\n) only allowed inside strings
			return (_Style.size() - _Position) >= 1 && _Style[_Position] == '\\' && _Style[_Position+1] != '\n';
		}

		inline bool is_quote(char ch) const
		{
			return	ch== '"' || ch == '\'';
		}

		inline bool is_block_open(char ch) const
		{
			return	ch == (char)'{' || ch == (char)'[' || ch == (char)'(';
		}

		inline bool is_block_close(char ch, char open) const
		{
			return ((open == '{' && ch == (char)'}') ||
					(open == '[' && ch == (char)']') ||
					(open == '(' && ch == (char)')'));
		}

		inline bool is_comment_open() const
		{
			if (_Position+1 > _Style.size())
				return false;

			return _Style[_Position] == (char)'/' && _Style[_Position+1] == (char)'*';
		}

		inline bool is_nonascii(char ch) const
		{
			return ch >= 0x80 /*&&  ch <= 255*/;
		}

		inline bool is_alpha(char ch) const
		{
			return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
		}

		inline bool is_digit(char ch) const
		{
			return ch >= '0' && ch <= '9';
		}

		inline bool is_nmchar(char ch) const
		{
			// checking escape here does not check if next char is '\n' or not
			return ch == '_' || ch == '-' || is_alpha(ch) || is_digit(ch) || is_nonascii(ch) || ch == '\\'/*is_escape(ch)*/;
		}
	};
}//namespace

#endif // CL_CSS_PARSER_H

