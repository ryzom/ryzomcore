// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

#include "stdmisc.h"

#include "nel/misc/json.h"

#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <cstring>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC {

namespace {

const uint s_MaxDepth = 256;

void appendUtf8(std::string &out, uint32 code)
{
	if (code < 0x80)
	{
		out += (char)code;
	}
	else if (code < 0x800)
	{
		out += (char)(0xC0 | (code >> 6));
		out += (char)(0x80 | (code & 0x3F));
	}
	else if (code < 0x10000)
	{
		out += (char)(0xE0 | (code >> 12));
		out += (char)(0x80 | ((code >> 6) & 0x3F));
		out += (char)(0x80 | (code & 0x3F));
	}
	else
	{
		out += (char)(0xF0 | (code >> 18));
		out += (char)(0x80 | ((code >> 12) & 0x3F));
		out += (char)(0x80 | ((code >> 6) & 0x3F));
		out += (char)(0x80 | (code & 0x3F));
	}
}

/// The length of the UTF-8 sequence at a position, 0 when it is not one (overlong, surrogate, cut short)
uint utf8SequenceLength(const std::string &s, std::string::size_type i)
{
	uint8 c = (uint8)s[i];
	uint len;
	uint32 code;
	if (c < 0x80)
		return 1;
	else if ((c & 0xE0) == 0xC0)
	{
		len = 2;
		code = c & 0x1F;
	}
	else if ((c & 0xF0) == 0xE0)
	{
		len = 3;
		code = c & 0x0F;
	}
	else if ((c & 0xF8) == 0xF0)
	{
		len = 4;
		code = c & 0x07;
	}
	else
		return 0;
	if (i + len > s.size())
		return 0;
	for (uint k = 1; k < len; ++k)
	{
		uint8 cc = (uint8)s[i + k];
		if ((cc & 0xC0) != 0x80)
			return 0;
		code = (code << 6) | (cc & 0x3F);
	}
	if ((len == 2 && code < 0x80) || (len == 3 && code < 0x800) || (len == 4 && (code < 0x10000 || code > 0x10FFFF)))
		return 0;
	if (code >= 0xD800 && code <= 0xDFFF)
		return 0;
	return len;
}

} // anonymous namespace

/// Recursive descent over the text; the depth is bounded so a hostile text cannot exhaust the stack
class CJsonParser
{
public:
	CJsonParser(const std::string &text)
	    : m_Text(text)
	    , m_Pos(0)
	{
	}

	bool parseDocument(CJsonValue &out, std::string &error)
	{
		skipSpace();
		if (!parseValue(out, 0))
		{
			error = m_Error;
			return false;
		}
		skipSpace();
		if (m_Pos != m_Text.size())
		{
			error = toString("unexpected text after the value at offset %u", (uint)m_Pos);
			return false;
		}
		return true;
	}

private:
	const std::string &m_Text;
	std::string::size_type m_Pos;
	std::string m_Error;

	bool fail(const char *what)
	{
		m_Error = toString("%s at offset %u", what, (uint)m_Pos);
		return false;
	}

	void skipSpace()
	{
		while (m_Pos < m_Text.size())
		{
			char c = m_Text[m_Pos];
			if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
				break;
			++m_Pos;
		}
	}

	bool literal(const char *word)
	{
		std::string::size_type len = strlen(word);
		if (m_Text.compare(m_Pos, len, word) != 0)
			return fail("invalid literal");
		m_Pos += len;
		return true;
	}

	bool parseValue(CJsonValue &out, uint depth)
	{
		if (depth > s_MaxDepth)
			return fail("nesting too deep");
		if (m_Pos >= m_Text.size())
			return fail("unexpected end");
		char c = m_Text[m_Pos];
		switch (c)
		{
		case 'n':
			out = CJsonValue();
			return literal("null");
		case 't':
			out = CJsonValue(true);
			return literal("true");
		case 'f':
			out = CJsonValue(false);
			return literal("false");
		case '"': {
			std::string s;
			if (!parseString(s))
				return false;
			out = CJsonValue(s);
			return true;
		}
		case '[': {
			out = CJsonValue::array();
			++m_Pos;
			skipSpace();
			if (m_Pos < m_Text.size() && m_Text[m_Pos] == ']')
			{
				++m_Pos;
				return true;
			}
			for (;;)
			{
				skipSpace();
				out.m_Items.push_back(CJsonValue());
				if (!parseValue(out.m_Items.back(), depth + 1))
					return false;
				skipSpace();
				if (m_Pos >= m_Text.size())
					return fail("unexpected end in an array");
				if (m_Text[m_Pos] == ',')
				{
					++m_Pos;
					continue;
				}
				if (m_Text[m_Pos] == ']')
				{
					++m_Pos;
					return true;
				}
				return fail("expected ',' or ']'");
			}
		}
		case '{': {
			out = CJsonValue::object();
			++m_Pos;
			skipSpace();
			if (m_Pos < m_Text.size() && m_Text[m_Pos] == '}')
			{
				++m_Pos;
				return true;
			}
			for (;;)
			{
				skipSpace();
				if (m_Pos >= m_Text.size() || m_Text[m_Pos] != '"')
					return fail("expected a member name");
				std::string key;
				if (!parseString(key))
					return false;
				skipSpace();
				if (m_Pos >= m_Text.size() || m_Text[m_Pos] != ':')
					return fail("expected ':'");
				++m_Pos;
				skipSpace();
				CJsonValue value;
				if (!parseValue(value, depth + 1))
					return false;
				// a repeated name keeps the last value, as most readers do
				out.set(key, value);
				skipSpace();
				if (m_Pos >= m_Text.size())
					return fail("unexpected end in an object");
				if (m_Text[m_Pos] == ',')
				{
					++m_Pos;
					continue;
				}
				if (m_Text[m_Pos] == '}')
				{
					++m_Pos;
					return true;
				}
				return fail("expected ',' or '}'");
			}
		}
		default:
			if (c == '-' || (c >= '0' && c <= '9'))
				return parseNumber(out);
			return fail("unexpected character");
		}
	}

	bool parseNumber(CJsonValue &out)
	{
		std::string::size_type start = m_Pos;
		if (m_Text[m_Pos] == '-')
			++m_Pos;
		if (m_Pos >= m_Text.size() || m_Text[m_Pos] < '0' || m_Text[m_Pos] > '9')
			return fail("invalid number");
		if (m_Text[m_Pos] == '0')
			++m_Pos;
		else
			while (m_Pos < m_Text.size() && m_Text[m_Pos] >= '0' && m_Text[m_Pos] <= '9')
				++m_Pos;
		if (m_Pos < m_Text.size() && m_Text[m_Pos] == '.')
		{
			++m_Pos;
			if (m_Pos >= m_Text.size() || m_Text[m_Pos] < '0' || m_Text[m_Pos] > '9')
				return fail("invalid number");
			while (m_Pos < m_Text.size() && m_Text[m_Pos] >= '0' && m_Text[m_Pos] <= '9')
				++m_Pos;
		}
		if (m_Pos < m_Text.size() && (m_Text[m_Pos] == 'e' || m_Text[m_Pos] == 'E'))
		{
			++m_Pos;
			if (m_Pos < m_Text.size() && (m_Text[m_Pos] == '+' || m_Text[m_Pos] == '-'))
				++m_Pos;
			if (m_Pos >= m_Text.size() || m_Text[m_Pos] < '0' || m_Text[m_Pos] > '9')
				return fail("invalid number");
			while (m_Pos < m_Text.size() && m_Text[m_Pos] >= '0' && m_Text[m_Pos] <= '9')
				++m_Pos;
		}
		// strtod reads the decimal point of the locale; the grammar above already checked the text
		std::string number = m_Text.substr(start, m_Pos - start);
		const char *point = localeconv()->decimal_point;
		if (point != nullptr && point[0] != '.' && point[0] != '\0')
		{
			std::string::size_type dot = number.find('.');
			if (dot != std::string::npos)
				number.replace(dot, 1, point);
		}
		out = CJsonValue(strtod(number.c_str(), nullptr));
		return true;
	}

	bool parseHex4(uint32 &code)
	{
		if (m_Pos + 4 > m_Text.size())
			return fail("unexpected end in an escape");
		code = 0;
		for (uint i = 0; i < 4; ++i)
		{
			char c = m_Text[m_Pos + i];
			code <<= 4;
			if (c >= '0' && c <= '9')
				code |= (uint32)(c - '0');
			else if (c >= 'a' && c <= 'f')
				code |= (uint32)(c - 'a' + 10);
			else if (c >= 'A' && c <= 'F')
				code |= (uint32)(c - 'A' + 10);
			else
				return fail("invalid \\u escape");
		}
		m_Pos += 4;
		return true;
	}

	bool parseString(std::string &out)
	{
		// at the opening quote
		++m_Pos;
		for (;;)
		{
			if (m_Pos >= m_Text.size())
				return fail("unexpected end in a string");
			char c = m_Text[m_Pos];
			if (c == '"')
			{
				++m_Pos;
				return true;
			}
			if ((uint8)c < 0x20)
				return fail("control character in a string");
			if (c != '\\')
			{
				out += c;
				++m_Pos;
				continue;
			}
			++m_Pos;
			if (m_Pos >= m_Text.size())
				return fail("unexpected end in an escape");
			char e = m_Text[m_Pos++];
			switch (e)
			{
			case '"': out += '"'; break;
			case '\\': out += '\\'; break;
			case '/': out += '/'; break;
			case 'b': out += '\b'; break;
			case 'f': out += '\f'; break;
			case 'n': out += '\n'; break;
			case 'r': out += '\r'; break;
			case 't': out += '\t'; break;
			case 'u': {
				uint32 code;
				if (!parseHex4(code))
					return false;
				if (code >= 0xD800 && code <= 0xDBFF)
				{
					// a high surrogate needs its low half
					uint32 low;
					if (m_Pos + 2 > m_Text.size() || m_Text[m_Pos] != '\\' || m_Text[m_Pos + 1] != 'u')
						return fail("unpaired surrogate");
					m_Pos += 2;
					if (!parseHex4(low))
						return false;
					if (low < 0xDC00 || low > 0xDFFF)
						return fail("unpaired surrogate");
					code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
				}
				else if (code >= 0xDC00 && code <= 0xDFFF)
				{
					return fail("unpaired surrogate");
				}
				appendUtf8(out, code);
				break;
			}
			default:
				return fail("invalid escape");
			}
		}
	}
};

// *** construction ***

CJsonValue::CJsonValue()
    : m_Type(Null)
    , m_Bool(false)
    , m_Number(0.0)
{
}

CJsonValue::CJsonValue(bool value)
    : m_Type(Bool)
    , m_Bool(value)
    , m_Number(0.0)
{
}

CJsonValue::CJsonValue(sint32 value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number((double)value)
{
}

CJsonValue::CJsonValue(uint32 value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number((double)value)
{
}

CJsonValue::CJsonValue(sint64 value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number((double)value)
{
}

CJsonValue::CJsonValue(uint64 value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number((double)value)
{
}

CJsonValue::CJsonValue(double value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number(value)
{
}

CJsonValue::CJsonValue(const std::string &value)
    : m_Type(String)
    , m_Bool(false)
    , m_Number(0.0)
    , m_String(value)
{
}

CJsonValue::CJsonValue(const char *value)
    : m_Type(String)
    , m_Bool(false)
    , m_Number(0.0)
    , m_String(value ? value : "")
{
}

CJsonValue CJsonValue::array()
{
	CJsonValue v;
	v.m_Type = Array;
	return v;
}

CJsonValue CJsonValue::object()
{
	CJsonValue v;
	v.m_Type = Object;
	return v;
}

// at file scope, not in the functions: built before main and gone after every atexit handler, which a thread
// still running in one may need
static const CJsonValue s_Null;
static const std::string s_Empty;

const CJsonValue &CJsonValue::null()
{
	return s_Null;
}

// *** access ***

bool CJsonValue::getBool(bool defaultValue) const
{
	return m_Type == Bool ? m_Bool : defaultValue;
}

double CJsonValue::getNumber(double defaultValue) const
{
	return m_Type == Number ? m_Number : defaultValue;
}

sint64 CJsonValue::getInt(sint64 defaultValue) const
{
	if (m_Type != Number || m_Number != m_Number)
		return defaultValue;
	if (m_Number >= 9.2233720368547758e18)
		return (sint64)0x7FFFFFFFFFFFFFFFLL;
	if (m_Number <= -9.2233720368547758e18)
		return -(sint64)0x7FFFFFFFFFFFFFFFLL - 1;
	return (sint64)m_Number;
}

const std::string &CJsonValue::getString() const
{
	return m_Type == String ? m_String : s_Empty;
}

std::string CJsonValue::getString(const std::string &defaultValue) const
{
	return m_Type == String ? m_String : defaultValue;
}

uint CJsonValue::size() const
{
	if (m_Type == Array)
		return (uint)m_Items.size();
	if (m_Type == Object)
		return (uint)m_Members.size();
	return 0;
}

const CJsonValue &CJsonValue::at(uint index) const
{
	if (m_Type != Array || index >= m_Items.size())
		return null();
	return m_Items[index];
}

CJsonValue &CJsonValue::push(const CJsonValue &value)
{
	if (m_Type == Null)
		m_Type = Array;
	nlassert(m_Type == Array);
	m_Items.push_back(value);
	return m_Items.back();
}

bool CJsonValue::has(const std::string &key) const
{
	if (m_Type != Object)
		return false;
	for (TMembers::const_iterator it = m_Members.begin(); it != m_Members.end(); ++it)
		if (it->first == key)
			return true;
	return false;
}

const CJsonValue &CJsonValue::get(const std::string &key) const
{
	if (m_Type != Object)
		return null();
	for (TMembers::const_iterator it = m_Members.begin(); it != m_Members.end(); ++it)
		if (it->first == key)
			return it->second;
	return null();
}

CJsonValue &CJsonValue::set(const std::string &key, const CJsonValue &value)
{
	member(key) = value;
	return *this;
}

CJsonValue &CJsonValue::member(const std::string &key)
{
	if (m_Type == Null)
		m_Type = Object;
	nlassert(m_Type == Object);
	for (TMembers::iterator it = m_Members.begin(); it != m_Members.end(); ++it)
		if (it->first == key)
			return it->second;
	m_Members.push_back(std::make_pair(key, CJsonValue()));
	return m_Members.back().second;
}

bool CJsonValue::erase(const std::string &key)
{
	if (m_Type != Object)
		return false;
	for (TMembers::iterator it = m_Members.begin(); it != m_Members.end(); ++it)
	{
		if (it->first == key)
		{
			m_Members.erase(it);
			return true;
		}
	}
	return false;
}

// *** text ***

std::string CJsonValue::toString() const
{
	std::string out;
	write(out);
	return out;
}

void CJsonValue::writeString(std::string &out, const std::string &value)
{
	static const char *const s_Hex = "0123456789abcdef";
	out += '"';
	std::string::size_type i = 0;
	while (i < value.size())
	{
		uint8 c = (uint8)value[i];
		if (c >= 0x80)
		{
			uint len = utf8SequenceLength(value, i);
			if (len == 0)
			{
				// not UTF-8: one replacement character per byte, so the length of the damage shows
				out += "\xEF\xBF\xBD";
				++i;
			}
			else
			{
				out.append(value, i, len);
				i += len;
			}
			continue;
		}
		switch (c)
		{
		case '"': out += "\\\""; break;
		case '\\': out += "\\\\"; break;
		case '\n': out += "\\n"; break;
		case '\r': out += "\\r"; break;
		case '\t': out += "\\t"; break;
		case '\b': out += "\\b"; break;
		case '\f': out += "\\f"; break;
		default:
			if (c < 0x20 || c == 0x7F)
			{
				out += "\\u00";
				out += s_Hex[c >> 4];
				out += s_Hex[c & 0xF];
			}
			else
			{
				out += (char)c;
			}
			break;
		}
		++i;
	}
	out += '"';
}

void CJsonValue::write(std::string &out) const
{
	switch (m_Type)
	{
	case Null:
		out += "null";
		break;
	case Bool:
		out += m_Bool ? "true" : "false";
		break;
	case Number: {
		// JSON has no NaN or infinity
		if (m_Number != m_Number || m_Number > 1.7976931348623157e308 || m_Number < -1.7976931348623157e308)
		{
			out += "null";
			break;
		}
		char buffer[64];
		if (m_Number == std::floor(m_Number) && std::fabs(m_Number) < 1e15)
			smprintf(buffer, sizeof(buffer), "%" NL_I64 "d", (sint64)m_Number);
		else
		{
			// the shortest of the two that reads back the same
			smprintf(buffer, sizeof(buffer), "%.15g", m_Number);
			if (strtod(buffer, nullptr) != m_Number)
				smprintf(buffer, sizeof(buffer), "%.17g", m_Number);
			// a locale with a decimal comma writes one
			const char *point = localeconv()->decimal_point;
			if (point != nullptr && point[0] != '.' && point[0] != '\0')
			{
				char *p = strstr(buffer, point);
				if (p != nullptr)
				{
					std::string fixed(buffer, p - buffer);
					fixed += '.';
					fixed += p + strlen(point);
					out += fixed;
					break;
				}
			}
		}
		out += buffer;
		break;
	}
	case String:
		writeString(out, m_String);
		break;
	case Array:
		out += '[';
		for (uint i = 0; i < m_Items.size(); ++i)
		{
			if (i)
				out += ',';
			m_Items[i].write(out);
		}
		out += ']';
		break;
	case Object:
		out += '{';
		for (uint i = 0; i < m_Members.size(); ++i)
		{
			if (i)
				out += ',';
			writeString(out, m_Members[i].first);
			out += ':';
			m_Members[i].second.write(out);
		}
		out += '}';
		break;
	}
}

bool CJsonValue::parse(const std::string &text, std::string &error)
{
	CJsonValue result;
	CJsonParser parser(text);
	if (!parser.parseDocument(result, error))
	{
		*this = CJsonValue();
		return false;
	}
	*this = result;
	return true;
}

} // NLMISC

/* End of json.cpp */
