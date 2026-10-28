// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "scenario_text_parser.h"

#include "nel/misc/string_common.h"

using namespace NLMISC;

namespace R2 {

namespace {

bool isAsciiSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

bool isAsciiDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool isAsciiIdentStart(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isAsciiIdentChar(char c)
{
	return isAsciiIdentStart(c) || isAsciiDigit(c);
}

sint asciiHexValue(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

} // namespace

// ****************************************************************************
CObject::TSmartPtr CScenarioTextParser::parse(const std::string &text, CObjectFactory *factory, std::string &errorMsg)
{
	CScenarioTextParser parser(text, factory);
	CObject::TSmartPtr result = parser.run();
	if (!result)
		errorMsg = parser.m_Error;
	return result;
}

// ****************************************************************************
CScenarioTextParser::CScenarioTextParser(const std::string &text, CObjectFactory *factory)
    : m_Text(text)
    , m_Factory(factory)
    , m_Pos(0)
    , m_Line(1)
    , m_Col(1)
    , m_Nodes(0)
{
}

// ****************************************************************************
CObject::TSmartPtr CScenarioTextParser::run()
{
	const CObject::TSmartPtr none;

	// A UTF-8 byte order mark, which an editor may have put in front of the file
	if (m_Text.size() >= 3 && (uint8)m_Text[0] == 0xEF && (uint8)m_Text[1] == 0xBB && (uint8)m_Text[2] == 0xBF)
		m_Pos = 3;

	skipSpaceAndComments();
	std::string ident;
	if (!readIdentifier(ident) || ident != "scenario")
	{
		fail("expected 'scenario ='");
		return none;
	}
	skipSpaceAndComments();
	if (!accept('='))
	{
		fail("expected '=' after 'scenario'");
		return none;
	}
	skipSpaceAndComments();
	if (!accept('{'))
	{
		fail("expected '{'");
		return none;
	}

	CObject::TSmartPtr root = newTable();
	if (!root)
	{
		fail("cannot create a table");
		return none;
	}

	std::vector<CFrame> stack;
	stack.push_back(CFrame(static_cast<CObjectTable *>(static_cast<CObject *>(root))));

	while (!stack.empty())
	{
		skipSpaceAndComments();
		if (atEnd())
		{
			fail("unexpected end of text inside a table");
			return none;
		}

		CFrame &frame = stack.back();
		char c = peek();

		if (frame.ExpectSeparator)
		{
			if (c == ',')
			{
				advance();
				frame.ExpectSeparator = false;
				continue;
			}
			if (c != '}')
			{
				fail("expected ',' or '}' after a value");
				return none;
			}
		}

		if (c == '}' && !frame.HasKey)
		{
			// The entries stay in file order, which is the order the writer sorted them in
			advance();
			stack.pop_back();
			continue;
		}

		CObject::TSmartPtr value;

		if (c == '{')
		{
			// A nested table: it joins its parent now and becomes the open frame
			advance();
			if (stack.size() >= MaxDepth)
			{
				fail("tables nested too deep");
				return none;
			}
			value = newTable();
			if (!value)
			{
				fail("cannot create a table");
				return none;
			}
			if (!addValue(frame, value))
				return none;
			stack.push_back(CFrame(static_cast<CObjectTable *>(static_cast<CObject *>(value))));
			continue;
		}

		if (isAsciiIdentStart(c))
		{
			if (!readIdentifier(ident))
				return none;
			skipSpaceAndComments();
			if (!frame.HasKey && peek() == '=' && peek(1) != '=')
			{
				// "key =": the value follows
				advance();
				frame.Key = ident;
				frame.HasKey = true;
				continue;
			}
			if (!valueFromIdentifier(ident, value))
				return none;
		}
		else if (c == '[' || c == '"' || c == '\'')
		{
			std::string str;
			if (!readString(str))
				return none;
			value = newString(str);
			if (!value)
			{
				fail("cannot create a string");
				return none;
			}
		}
		else if (c == '-' || c == '.' || isAsciiDigit(c))
		{
			if (!readNumber(value))
				return none;
		}
		else
		{
			fail("unexpected character");
			return none;
		}

		if (!addValue(frame, value))
			return none;
	}

	skipSpaceAndComments();
	if (!atEnd())
	{
		fail("text after the scenario table");
		return none;
	}
	return root;
}

// ****************************************************************************
void CScenarioTextParser::advance()
{
	if (atEnd())
		return;
	if (m_Text[m_Pos] == '\n')
	{
		++m_Line;
		m_Col = 1;
	}
	else
	{
		++m_Col;
	}
	++m_Pos;
}

// ****************************************************************************
bool CScenarioTextParser::accept(char c)
{
	if (atEnd() || peek() != c)
		return false;
	advance();
	return true;
}

// ****************************************************************************
void CScenarioTextParser::skipSpaceAndComments()
{
	for (;;)
	{
		char c = peek();
		if (isAsciiSpace(c))
		{
			advance();
			continue;
		}
		if (c == '-' && peek(1) == '-')
		{
			advance();
			advance();
			if (peek() == '[' && peek(1) == '[')
			{
				// block comment, up to the closing "]]"
				advance();
				advance();
				while (!atEnd() && !(peek() == ']' && peek(1) == ']'))
					advance();
				if (!atEnd())
				{
					advance();
					advance();
				}
				continue;
			}
			// line comment
			while (!atEnd() && peek() != '\n')
				advance();
			continue;
		}
		return;
	}
}

// ****************************************************************************
bool CScenarioTextParser::readIdentifier(std::string &out)
{
	out.clear();
	if (!isAsciiIdentStart(peek()))
		return fail("expected a name");
	while (isAsciiIdentChar(peek()))
	{
		out += peek();
		advance();
	}
	return true;
}

// ****************************************************************************
bool CScenarioTextParser::readString(std::string &out)
{
	out.clear();
	for (;;)
	{
		char c = peek();
		if (c == '[')
		{
			if (!readLongBracket(out))
				return false;
		}
		else if (c == '"' || c == '\'')
		{
			if (!readQuoted(out))
				return false;
		}
		else
		{
			return fail("expected a string");
		}

		// The writer splits a string around '[' and ']' with "..", so fragments concatenate
		skipSpaceAndComments();
		if (peek() == '.' && peek(1) == '.')
		{
			advance();
			advance();
			skipSpaceAndComments();
			continue;
		}
		return true;
	}
}

// ****************************************************************************
bool CScenarioTextParser::readLongBracket(std::string &out)
{
	// only "[[ ]]", the form the writer produces; a level like "[=[" is not accepted
	advance(); // '['
	if (!accept('['))
		return fail("expected '[[' to open a long bracket string");

	// As in Lua, a newline right after the opening bracket is not part of the string
	if (peek() == '\r')
	{
		advance();
		if (peek() == '\n')
			advance();
	}
	else if (peek() == '\n')
	{
		advance();
	}

	for (;;)
	{
		if (atEnd())
			return fail("unterminated long bracket string");
		if (peek() == ']' && peek(1) == ']')
		{
			advance();
			advance();
			return true;
		}
		out += peek();
		advance();
	}
}

// ****************************************************************************
bool CScenarioTextParser::readQuoted(std::string &out)
{
	char quote = peek();
	advance();
	for (;;)
	{
		if (atEnd())
			return fail("unterminated quoted string");
		char c = peek();
		if (c == quote)
		{
			advance();
			return true;
		}
		if (c == '\n' || c == '\r')
			return fail("newline inside a quoted string");
		if (c != '\\')
		{
			out += c;
			advance();
			continue;
		}

		// an escape sequence
		advance();
		char e = peek();
		switch (e)
		{
			// clang-format off
		case 'n': out += '\n'; advance(); break;
		case 't': out += '\t'; advance(); break;
		case 'r': out += '\r'; advance(); break;
		case 'a': out += '\a'; advance(); break;
		case 'b': out += '\b'; advance(); break;
		case 'f': out += '\f'; advance(); break;
		case 'v': out += '\v'; advance(); break;
		case '\\': out += '\\'; advance(); break;
		case '"': out += '"'; advance(); break;
		case '\'': out += '\''; advance(); break;
		case '\n': out += '\n'; advance(); break;
		// clang-format on
		case '\r':
			out += '\n';
			advance();
			if (peek() == '\n')
				advance();
			break;
		case 'x': {
			advance();
			sint high = asciiHexValue(peek());
			if (high < 0)
				return fail("bad hexadecimal escape");
			advance();
			sint low = asciiHexValue(peek());
			if (low < 0)
				return fail("bad hexadecimal escape");
			advance();
			out += (char)(high * 16 + low);
			break;
		}
		case 'z':
			advance();
			while (isAsciiSpace(peek()))
				advance();
			break;
		default: {
			if (!isAsciiDigit(e))
				return fail("unknown escape sequence");
			uint value = 0;
			uint digits = 0;
			while (digits < 3 && isAsciiDigit(peek()))
			{
				value = value * 10 + (peek() - '0');
				advance();
				++digits;
			}
			if (value > 255)
				return fail("decimal escape out of range");
			out += (char)value;
			break;
		}
		}
	}
}

// ****************************************************************************
bool CScenarioTextParser::readNumber(CObject::TSmartPtr &out)
{
	std::string literal;
	uint digits = 0;
	bool isInteger = true;

	if (peek() == '-')
	{
		literal += '-';
		advance();
	}
	while (isAsciiDigit(peek()))
	{
		literal += peek();
		advance();
		++digits;
	}
	if (peek() == '.')
	{
		isInteger = false;
		literal += '.';
		advance();
		while (isAsciiDigit(peek()))
		{
			literal += peek();
			advance();
			++digits;
		}
	}
	if (digits == 0)
		return fail("expected a number");
	if (peek() == 'e' || peek() == 'E')
	{
		isInteger = false;
		literal += 'e';
		advance();
		if (peek() == '+' || peek() == '-')
		{
			literal += peek();
			advance();
		}
		uint exponentDigits = 0;
		while (isAsciiDigit(peek()))
		{
			literal += peek();
			advance();
			++exponentDigits;
		}
		if (exponentDigits == 0)
			return fail("expected exponent digits");
	}
	if (isAsciiIdentChar(peek()) || peek() == '.')
		return fail("bad number");

	if (isInteger && digits <= 19)
	{
		// Up to 18 digits always fit a sint64. 19 digits may not, so the value is only taken
		// when it prints back the same; otherwise it is kept as a double below.
		sint64 value;
		if (fromString(literal, value) && (digits <= 18 || toString(value) == literal))
		{
			out = newInteger(value);
			if (!out)
				return fail("cannot create a number");
			return true;
		}
	}

	double value;
	if (!fromString(literal, value))
		return fail("bad number");
	out = newNumber(value);
	if (!out)
		return fail("cannot create a number");
	return true;
}

// ****************************************************************************
bool CScenarioTextParser::readRefId(CObject::TSmartPtr &out)
{
	// "r2" was read: ".RefId(" string ")" follows
	skipSpaceAndComments();
	if (!accept('.'))
		return fail("expected 'r2.RefId'");
	skipSpaceAndComments();
	std::string ident;
	if (!readIdentifier(ident) || ident != "RefId")
		return fail("expected 'r2.RefId'");
	skipSpaceAndComments();
	if (!accept('('))
		return fail("expected '(' after 'r2.RefId'");
	skipSpaceAndComments();
	std::string value;
	if (!readString(value))
		return false;
	skipSpaceAndComments();
	if (!accept(')'))
		return fail("expected ')' after the ref id");
	out = newRefId(value);
	if (!out)
		return fail("cannot create a ref id");
	return true;
}

// ****************************************************************************
bool CScenarioTextParser::valueFromIdentifier(const std::string &ident, CObject::TSmartPtr &out)
{
	if (ident == "true" || ident == "false")
	{
		// The Lua loader turned booleans into numbers too
		out = newInteger(ident == "true" ? 1 : 0);
		if (!out)
			return fail("cannot create a number");
		return true;
	}
	if (ident == "r2")
		return readRefId(out);
	return fail("unexpected name where a value was expected");
}

// ****************************************************************************
CObject::TSmartPtr CScenarioTextParser::newTable()
{
	CObject::TSmartPtr obj = m_Factory ? m_Factory->newBasic("Table") : new CObjectTable();
	if (!obj || !obj->isTable())
		return CObject::TSmartPtr();
	return obj;
}

// ****************************************************************************
CObject::TSmartPtr CScenarioTextParser::newString(const std::string &value)
{
	CObject::TSmartPtr obj = m_Factory ? m_Factory->newBasic("String") : new CObjectString("");
	if (obj)
		obj->set("", value);
	return obj;
}

// ****************************************************************************
CObject::TSmartPtr CScenarioTextParser::newRefId(const std::string &value)
{
	CObject::TSmartPtr obj = m_Factory ? m_Factory->newBasic("RefId") : new CObjectRefId("");
	if (obj)
		obj->set("", value);
	return obj;
}

// ****************************************************************************
CObject::TSmartPtr CScenarioTextParser::newInteger(sint64 value)
{
	CObject::TSmartPtr obj = m_Factory ? m_Factory->newBasic("Number") : new CObjectNumber((sint64)0);
	if (obj)
		obj->set("", value);
	return obj;
}

// ****************************************************************************
CObject::TSmartPtr CScenarioTextParser::newNumber(double value)
{
	CObject::TSmartPtr obj = m_Factory ? m_Factory->newBasic("Number") : new CObjectNumber((sint64)0);
	if (obj)
		obj->set("", value);
	return obj;
}

// ****************************************************************************
bool CScenarioTextParser::addValue(CFrame &frame, const CObject::TSmartPtr &value)
{
	if (++m_Nodes > MaxNodes)
		return fail("too many values");
	// A key given twice would be the last one in Lua and the first one in getAttr(): neither
	// is what the writer produces, so it is refused
	if (!frame.Key.empty() && frame.Table->getAttr(frame.Key))
		return fail("duplicate key");
	frame.Table->add(frame.Key, value);
	frame.Key.clear();
	frame.HasKey = false;
	frame.ExpectSeparator = true;
	return true;
}

// ****************************************************************************
bool CScenarioTextParser::fail(const char *what)
{
	if (m_Error.empty())
		m_Error = toString("%s at line %u column %u", what, m_Line, m_Col);
	return false;
}

} // namespace R2
