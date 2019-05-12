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
#include "nel/gui/css_parser.h"
#include "nel/gui/css_style.h"
#include "nel/gui/css_selector.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	// ***************************************************************************
	// Parse style declarations style, eg. "color:red; font-size: 10px;"
	//
	// key is converted to lowercase
	// value is left as is
	TStyle CCssParser::parseDecls(const std::string &styleString)
	{
		TStyle styles;
		std::vector<std::string> elements;
		NLMISC::splitString(styleString, ";", elements);

		for(uint i = 0; i < elements.size(); ++i)
		{
			std::string::size_type pos;
			pos = elements[i].find_first_of(':');
			if (pos != std::string::npos)
			{
				std::string key = trim(toLower(elements[i].substr(0, pos)));
				std::string value = trim(elements[i].substr(pos+1));
				styles[key] = value;
			}
		}

		return styles;
	}

	// ***************************************************************************
	// Parse stylesheet, eg content from main.css file
	//
	// Return all found rules
	void CCssParser::parseStylesheet(const std::string &cssString, std::vector<CCssStyle::SStyleRule> &result)
	{
		_Rules.clear();
		_Style.clear();

		_Style.fromUtf8(cssString);
		preprocess();

		_Position = 0;
		while(!is_eof())
		{
			skipWhitespace();

			if (_Style[_Position] == (ucchar)'@')
				readAtRule();
			else
				readRule();
		}

		result.insert(result.end(), _Rules.begin(), _Rules.end());
		_Rules.clear();
	}

	// ***************************************************************************
	// Parse selector with style string
	// selector: "a#id .class"
	// style:    "color: red; font-size: 10px;"
	//
	// @internal
	void CCssParser::parseRule(const ucstring &selectorString, const ucstring &styleString)
	{
		std::vector<ucstring> selectors;
		NLMISC::explode(selectorString, ucstring(","), selectors);

		TStyle props;
		props = parseDecls(styleString.toUtf8());

		// duplicate props to each selector in selector list,
		// example 'div > p, h1' creates 'div>p' and 'h1'
		for(uint i=0; i<selectors.size(); ++i)
		{
			CCssStyle::SStyleRule rule;

			rule.Selector = parse_selector(trim(selectors[i]), rule.PseudoElement);
			rule.Properties = props;

			if (!rule.Selector.empty())
			{
				_Rules.push_back(rule);
			}
		}
	}

	// ***************************************************************************
	// Skip over at-rule
	// @import ... ;
	// @charset ... ;
	// @media query { .. }
	//
	// @internal
	void CCssParser::readAtRule()
	{
		// skip '@'
		_Position++;

		// skip 'import', 'media', etc
		skipIdentifier();

		// skip at-rule statement
		while(!is_eof() && _Style[_Position] != (ucchar)';')
		{
			if (maybe_escape())
			{
				escape();
			}
			else if (is_quote(_Style[_Position]))
			{
				skipString();
			}
			else if (is_block_open(_Style[_Position]))
			{
				bool mustBreak = (_Style[_Position] == '{');
				skipBlock();

				if(mustBreak)
				{
					break;
				}
			}
			else
			{
				_Position++;
			}
		}

		// skip ';' or '}'
		_Position++;
	}

	// ***************************************************************************
	// skip over "elm#id.selector[attr]:peseudo, .sel2 { rule }" block
	// @internal
	void CCssParser::readRule()
	{
		size_t start;

		// selector
		start = _Position;
		while(!is_eof())
		{
			if (maybe_escape())
				_Position++;
			else if (is_quote(_Style[_Position]))
				skipString();
			else if (_Style[_Position] == (ucchar)'[')
				skipBlock();
			else if (_Style[_Position] == (ucchar)'{')
				break;
			else
				_Position++;
		}

		if (!is_eof())
		{
			ucstring selector;
			selector.append(_Style, start, _Position - start);

			skipWhitespace();

			// declaration block
			start = _Position;
			skipBlock();
			if (_Position <= _Style.size())
			{
				ucstring rules;
				rules.append(_Style, start + 1, _Position - start - 2);

				parseRule(selector, rules);
			}
		}
	}

	// ***************************************************************************
	// skip over \abcdef escaped sequence or escaped newline char
	// @internal
	void CCssParser::escape()
	{
		// skip '\'
		_Position++;
		if (is_hex(_Style[_Position]))
		{
			// TODO: '\abc def' should be considered one string
			for(uint i=0; i<6 && is_hex(_Style[_Position]); i++)
				_Position++;

			if (_Style[_Position] == (ucchar)' ')
				_Position++;
		}
		else if (_Style[_Position] != 0x0A)
			_Position++;
	}

	// ***************************************************************************
	// @internal
	bool CCssParser::skipIdentifier()
	{
		size_t start = _Position;
		bool valid = true;
		while(!is_eof() && valid)
		{
			if (maybe_escape())
			{
				escape();
				continue;
			}
			else if (is_alpha(_Style[_Position]))
			{
				// valid
			}
			else if (is_digit(_Style[_Position]))
			{
				if (_Position == start)
				{
					// cannot start with digit
					valid = false;
				}
				else if ((_Position - start) == 0 && _Style[_Position-1] == (ucchar)'-')
				{
					// cannot start with -#
					valid = false;
				}
			}
			else if (_Style[_Position] == (ucchar)'_')
			{
				// valid
			}
			else if (_Style[_Position] >= 0x0080)
			{
				// valid
			}
			else if (_Style[_Position] == (ucchar)'-')
			{
				if ((_Position - start) == 1 && _Style[_Position-1] == (ucchar)'-')
				{
					// cannot start with --
					valid = false;
				}
			}
			else
			{
				// we're done
				break;
			}

			_Position++;
		}

		return valid && !is_eof();
	}

	// ***************************************************************************
	// skip over (..), [..], or {..} blocks
	// @internal
	void CCssParser::skipBlock()
	{
		ucchar startChar = _Style[_Position];

		// block start
		_Position++;
		while(!is_eof() && !is_block_close(_Style[_Position], startChar))
		{
			if (maybe_escape())
				// skip backslash and next char
				_Position += 2;
			else if (is_quote(_Style[_Position]))
				skipString();
			else if (is_block_open(_Style[_Position]))
				skipBlock();
			else
				_Position++;
		}

		// block end
		_Position++;
	}

	// ***************************************************************************
	// skip over quoted string
	// @internal
	void CCssParser::skipString()
	{
		ucchar endChar = _Style[_Position];

		// quote start
		_Position++;
		while(!is_eof() && _Style[_Position] != endChar)
		{
			if (maybe_escape())
				_Position++;

			_Position++;
		}

		// quote end
		_Position++;
	}

	// ***************************************************************************
	// @internal
	void CCssParser::skipWhitespace()
	{
		while(!is_eof() && is_whitespace(_Style[_Position]))
			_Position++;
	}

	// ***************************************************************************
	// parse selector list
	// @internal
	std::vector<CCssSelector> CCssParser::parse_selector(const ucstring &sel, std::string &pseudoElement) const
	{
		std::vector<CCssSelector> result;
		CCssSelector current;

		pseudoElement.clear();

		bool failed = false;
		ucstring::size_type start = 0, pos = 0;
		while(pos < sel.size())
		{
			ucstring uc;
			uc = sel[pos];
			if (is_nmchar(sel[pos]) && current.empty())
			{
				pos++;

				while(pos < sel.size() && is_nmchar(sel[pos]))
					pos++;

				current.Element = toLower(sel.substr(start, pos - start).toUtf8());
				start = pos;
				continue;
			}

			if(sel[pos] == '#')
			{
				pos++;
				start=pos;

				while(pos < sel.size() && is_nmchar(sel[pos]))
					pos++;

				current.Id = toLower(sel.substr(start, pos - start).toUtf8());
				start = pos;
			}
			else if (sel[pos] == '.')
			{
				pos++;
				start=pos;

				// .classA.classB
				while(pos < sel.size() && (is_nmchar(sel[pos]) || sel[pos] == '.'))
					pos++;

				current.setClass(toLower(sel.substr(start, pos - start).toUtf8()));
				start = pos;
			}
			else if (sel[pos] == '[')
			{
				pos++;
				start = pos;

				if (is_whitespace(sel[pos]))
				{
					while(pos < sel.size() && is_whitespace(sel[pos]))
						pos++;

					start = pos;
				}

				ucstring key;
				ucstring value;
				ucchar op = ' ';

				// key
				while(pos < sel.size() && is_nmchar(sel[pos]))
					pos++;

				key = sel.substr(start, pos - start);
				if (pos == sel.size()) break;

				if (is_whitespace(sel[pos]))
				{
					while(pos < sel.size() && is_whitespace(sel[pos]))
						pos++;

					if (pos == sel.size()) break;
				}

				if (sel[pos] == ']')
				{
					current.addAttribute(key.toUtf8());
				}
				else
				{
					// operand
					op = sel[pos];
					if (op == '~' || op == '|' || op == '^' || op == '$' || op == '*')
					{
						pos++;
						if (pos == sel.size()) break;
					}

					// invalid rule?, eg [attr^value]
					if (sel[pos] != '=')
					{
						while(pos < sel.size() && sel[pos] != ']')
							pos++;

						if (pos == sel.size()) break;

						start = pos;
					}
					else
					{
						// skip '='
						pos++;

						if (is_whitespace(sel[pos]))
						{
							while(pos < sel.size() && is_whitespace(sel[pos]))
								pos++;

							if (pos == sel.size()) break;
						}

						// value
						start = pos;
						bool quote = false;
						char quoteOpen;
						while(pos < sel.size())
						{
							if (sel[pos] == '\'' || sel[pos] == '"')
							{
								// skip over quoted value
								start = pos;
								pos++;
								while(pos < sel.size() && sel[pos] != sel[start])
								{
									if (sel[pos] == '\\')
									{
										pos++;
									}
									pos++;
								}

								if (pos == sel.size()) break;
							}
							else if (sel[pos] == '\\')
							{
								pos++;
							}
							else if (!quote && sel[pos] == ']')
							{
								value = sel.substr(start, pos - start);
								break;
							}

							pos++;
						} // while 'value'

						if (pos == sel.size()) break;

						bool cs = true;
						// [value="attr" i]
						if (value.size() > 2 && value[value.size()-2] == ' ')
						{
							ucchar lastChar = value[value.size()-1];
							if (lastChar == 'i' || lastChar == 'I' || lastChar == 's' || lastChar == 'S')
							{
								value = value.substr(0, value.size()-2);
								cs = !((lastChar == 'i' || lastChar == 'I'));
							}
						}
						current.addAttribute(key.toUtf8(), trimQuotes(value).toUtf8(), (char)op, cs);
					} // op error
				} // no value

				// skip ']'
				pos++;

				start = pos;
			}
			else if (sel[pos] == ':')
			{
				pos++;
				start=pos;
				// pseudo element, eg '::before'
				if (pos < sel.size() && sel[pos] == ':')
				{
					pos++;
				}
				// :first-child
				// :nth-child(2n+0)
				// :not(h1, div#main)
				// :not(:nth-child(2n+0))
				// has no support for quotes, eg  :not(h1[attr=")"]) fails
				while(pos < sel.size() && (is_nmchar(sel[pos]) || sel[pos] == '('))
				{
					if (sel[pos] == '(')
					{
						uint open = 1;
						pos++;
						while(pos < sel.size() && open > 0)
						{
							if (sel[pos] == ')')
								open--;
							else if (sel[pos] == '(')
								open++;

							pos++;
						}
						break;
					}
					else
					{
						pos++;
					}
				}

				std::string key = toLower(sel.substr(start, pos - start).toUtf8());
				if (key.empty())
				{
					failed = true;
					break;
				}

				if (key[0] == ':' || key == "after" || key == "before" || key == "cue" || key == "first-letter" || key == "first-line")
				{
					if (!pseudoElement.empty())
					{
						failed = true;
						break;
					}
					if (key[0] != ':')
					{
						pseudoElement = ":" + key;
					}
					else
					{
						pseudoElement = key;
					}
				}
				else
				{
					current.addPseudoClass(key);
				}

				start = pos;
			}
			else if (!current.empty())
			{
				// pseudo element like ':before' can only be set on the last selector
				// user action pseudo classes can be used after pseudo element (ie, :focus, :hover)
				// there is no support for those and its safe to just fail the selector
				if (!result.empty() && !pseudoElement.empty())
				{
					failed = true;
					break;
				}

				// start new selector as combinator is part of next selector
				result.push_back(current);
				current = CCssSelector();

				// detect and remove whitespace around combinator, eg ' > '
				bool isSpace = is_whitespace(sel[pos]);;
				while(pos < sel.size() && is_whitespace(sel[pos]))
					pos++;

				if (sel[pos] == '>' || sel[pos] == '+' || sel[pos] == '~')
				{
					current.Combinator = sel[pos];
					pos++;

					while(pos < sel.size() && is_whitespace(sel[pos]))
						pos++;
				}
				else if (isSpace)
				{
					current.Combinator = ' ';
				}
				else
				{
					// unknown
					current.Combinator = sel[pos];
					pos++;
				}

				start = pos;
			}
			else
			{
				pos++;
			}
		}

		if (failed)
		{
			result.clear();
		}
		else if (!current.empty())
		{
			result.push_back(current);
		}

		return result;
	}

	// ***************************************************************************
	// @internal
	void CCssParser::preprocess()
	{
		_Position = 0;

		size_t start;
		size_t charsLeft;
		bool quote = false;
		ucchar quoteChar;
		while(!is_eof())
		{
			charsLeft = _Style.size() - _Position - 1;

			// FF, CR
			if (_Style[_Position] == 0x0C || _Style[_Position] == 0x0D)
			{
				uint len = 1;
				// CR, LF
				if (charsLeft >= 1 && _Style[_Position] == 0x0D && _Style[_Position+1] == 0x0A)
					len++;

				ucstring tmp;
				tmp += 0x000A;
				_Style.replace(_Position, 1, tmp);
			}
			else if (_Style[_Position] == 0x00)
			{
				// Unicode replacement character
				_Style[_Position] = 0xFFFD;
			}
			else
			{
				// strip comments for easier parsing
				if (_Style[_Position] == '\\')
				{
					_Position++;
				}
				else if (is_quote(_Style[_Position]))
				{
					if (!quote)
						quoteChar = _Style[_Position];

					if (quote && _Style[_Position] == quoteChar)
						quote = !quote;
				}
				else if (!quote && is_comment_open())
				{
					size_t pos = _Style.find(ucstring("*/"), _Position + 2);
					if (pos == std::string::npos)
						pos = _Style.size();

					_Style.erase(_Position, pos - _Position + 2);
					ucstring uc;
					uc = _Style[_Position];

					// _Position is already at correct place
					continue;
				}
			}

			_Position++;
		}
	}
} // namespace

