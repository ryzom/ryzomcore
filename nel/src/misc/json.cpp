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

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <cstring>
#include <map>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC {

namespace {

const uint s_MaxDepth = 256;

// an object gets an index of its keys from this many members on; below, a scan is faster
const uint s_IndexFrom = 16;

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

/// The value of four hex digits at a position, false when they are not that
bool readHex4(const std::string &text, std::string::size_type pos, uint32 &code)
{
	if (pos + 4 > text.size())
		return false;
	code = 0;
	for (uint i = 0; i < 4; ++i)
	{
		char c = text[pos + i];
		code <<= 4;
		if (c >= '0' && c <= '9')
			code |= (uint32)(c - '0');
		else if (c >= 'a' && c <= 'f')
			code |= (uint32)(c - 'a' + 10);
		else if (c >= 'A' && c <= 'F')
			code |= (uint32)(c - 'A' + 10);
		else
			return false;
	}
	return true;
}

} // anonymous namespace

struct CJsonValue::CMemberIndex
{
	std::map<std::string, uint> Positions;
};

/// Recursive descent over the text; the depth is bounded so a hostile text cannot exhaust the stack. Values are
/// parsed in their place in the result, so nothing is copied on the way up.
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

	/// Parses a value into out, which is null
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
			return literal("null");
		case 't':
			out.m_Type = CJsonValue::Bool;
			out.m_Bool = true;
			return literal("true");
		case 'f':
			out.m_Type = CJsonValue::Bool;
			out.m_Bool = false;
			return literal("false");
		case '"':
			out.m_Type = CJsonValue::String;
			return parseString(out.m_String);
		case '[': {
			out.m_Type = CJsonValue::Array;
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
			out.m_Type = CJsonValue::Object;
			++m_Pos;
			skipSpace();
			if (m_Pos < m_Text.size() && m_Text[m_Pos] == '}')
			{
				++m_Pos;
				return true;
			}
			std::string key;
			for (;;)
			{
				skipSpace();
				if (m_Pos >= m_Text.size() || m_Text[m_Pos] != '"')
					return fail("expected a member name");
				key.clear();
				if (!parseString(key))
					return false;
				skipSpace();
				if (m_Pos >= m_Text.size() || m_Text[m_Pos] != ':')
					return fail("expected ':'");
				++m_Pos;
				skipSpace();
				// a repeated name keeps the last value, in the place of the first, as most readers do
				CJsonValue &value = out.member(key);
				CJsonValue().swap(value);
				if (!parseValue(value, depth + 1))
					return false;
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
		double value = strtod(number.c_str(), nullptr);
		// what overflows a double is refused: an infinity would reach the code as a number, and JSON has none.
		// What underflows reads as 0 or a denormal, which is a number.
		if (value > DBL_MAX || value < -DBL_MAX)
		{
			m_Pos = start;
			return fail("number out of the range of a double");
		}
		out.m_Type = CJsonValue::Number;
		out.m_Number = value;
		return true;
	}

	bool parseHex4(uint32 &code)
	{
		if (m_Pos + 4 > m_Text.size())
			return fail("unexpected end in an escape");
		if (!readHex4(m_Text, m_Pos, code))
			return fail("invalid \\u escape");
		m_Pos += 4;
		return true;
	}

	/// Appends to out the string at the opening quote
	bool parseString(std::string &out)
	{
		++m_Pos;
		for (;;)
		{
			// the plain run up to the next quote, escape or control character, in one append
			std::string::size_type run = m_Pos;
			while (run < m_Text.size() && m_Text[run] != '"' && m_Text[run] != '\\' && (uint8)m_Text[run] >= 0x20)
				++run;
			out.append(m_Text, m_Pos, run - m_Pos);
			m_Pos = run;
			if (m_Pos >= m_Text.size())
				return fail("unexpected end in a string");
			char c = m_Text[m_Pos];
			if (c == '"')
			{
				++m_Pos;
				return true;
			}
			if (c != '\\')
				return fail("control character in a string");
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
					// a high surrogate with its low half is one character
					uint32 low;
					if (m_Pos + 6 <= m_Text.size() && m_Text[m_Pos] == '\\' && m_Text[m_Pos + 1] == 'u' && readHex4(m_Text, m_Pos + 2, low) && low >= 0xDC00
					    && low <= 0xDFFF)
					{
						m_Pos += 6;
						code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
					}
					else
					{
						// without it the half is no character: U+FFFD, as the writer does for bytes that are not
						// UTF-8, so a message with one is still read and answered; what follows is read as itself
						code = 0xFFFD;
					}
				}
				else if (code >= 0xDC00 && code <= 0xDFFF)
				{
					code = 0xFFFD;
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
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(bool value)
    : m_Type(Bool)
    , m_Bool(value)
    , m_Number(0.0)
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(sint32 value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number((double)value)
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(uint32 value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number((double)value)
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(sint64 value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number((double)value)
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(uint64 value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number((double)value)
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(double value)
    : m_Type(Number)
    , m_Bool(false)
    , m_Number(value)
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(const std::string &value)
    : m_Type(String)
    , m_Bool(false)
    , m_Number(0.0)
    , m_String(value)
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(const char *value)
    : m_Type(String)
    , m_Bool(false)
    , m_Number(0.0)
    , m_String(value ? value : "")
    , m_Index(nullptr)
{
}

CJsonValue::CJsonValue(const CJsonValue &other)
    : m_Type(other.m_Type)
    , m_Bool(other.m_Bool)
    , m_Number(other.m_Number)
    , m_String(other.m_String)
    , m_Items(other.m_Items)
    , m_Members(other.m_Members)
    , m_Index(other.m_Index != nullptr ? new CMemberIndex(*other.m_Index) : nullptr)
{
}

CJsonValue &CJsonValue::operator=(const CJsonValue &other)
{
	// a copy first: other may be a part of this value
	if (this != &other)
	{
		CJsonValue copy(other);
		swap(copy);
	}
	return *this;
}

#ifdef NL_CPP11
CJsonValue::CJsonValue(CJsonValue &&other) noexcept
    : m_Type(other.m_Type)
    , m_Bool(other.m_Bool)
    , m_Number(other.m_Number)
    , m_String(std::move(other.m_String))
    , m_Items(std::move(other.m_Items))
    , m_Members(std::move(other.m_Members))
    , m_Index(other.m_Index)
{
	other.m_Type = Null;
	other.m_String.clear();
	other.m_Items.clear();
	other.m_Members.clear();
	other.m_Index = nullptr;
}

CJsonValue &CJsonValue::operator=(CJsonValue &&other) noexcept
{
	// what this held goes away with other
	swap(other);
	return *this;
}
#endif

CJsonValue::~CJsonValue()
{
	delete m_Index;
}

void CJsonValue::swap(CJsonValue &other)
{
	std::swap(m_Type, other.m_Type);
	std::swap(m_Bool, other.m_Bool);
	std::swap(m_Number, other.m_Number);
	m_String.swap(other.m_String);
	m_Items.swap(other.m_Items);
	m_Members.swap(other.m_Members);
	std::swap(m_Index, other.m_Index);
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

sint32 CJsonValue::findMember(const std::string &key) const
{
	if (m_Index != nullptr)
	{
		std::map<std::string, uint>::const_iterator it = m_Index->Positions.find(key);
		return it == m_Index->Positions.end() ? -1 : (sint32)it->second;
	}
	for (uint i = 0; i < m_Members.size(); ++i)
		if (m_Members[i].first == key)
			return (sint32)i;
	return -1;
}

void CJsonValue::buildIndex()
{
	CMemberIndex *index = new CMemberIndex();
	for (uint i = 0; i < m_Members.size(); ++i)
		index->Positions.insert(std::make_pair(m_Members[i].first, i));
	delete m_Index;
	m_Index = index;
}

bool CJsonValue::has(const std::string &key) const
{
	return m_Type == Object && findMember(key) >= 0;
}

const CJsonValue &CJsonValue::get(const std::string &key) const
{
	if (m_Type != Object)
		return null();
	sint32 pos = findMember(key);
	return pos < 0 ? null() : m_Members[pos].second;
}

CJsonValue &CJsonValue::set(const std::string &key, const CJsonValue &value)
{
	// a copy first: value may be a member of this object, which adding a member moves
	CJsonValue copy(value);
	member(key).swap(copy);
	return *this;
}

CJsonValue &CJsonValue::member(const std::string &key)
{
	if (m_Type == Null)
		m_Type = Object;
	nlassert(m_Type == Object);
	sint32 pos = findMember(key);
	if (pos >= 0)
		return m_Members[pos].second;
	m_Members.push_back(TMember(key, CJsonValue()));
	if (m_Index != nullptr)
		m_Index->Positions.insert(std::make_pair(key, (uint)m_Members.size() - 1));
	else if (m_Members.size() >= s_IndexFrom)
		buildIndex();
	return m_Members.back().second;
}

bool CJsonValue::erase(const std::string &key)
{
	if (m_Type != Object)
		return false;
	sint32 pos = findMember(key);
	if (pos < 0)
		return false;
	m_Members.erase(m_Members.begin() + pos);
	if (m_Index != nullptr)
	{
		// the members after it move down by one
		m_Index->Positions.erase(key);
		for (std::map<std::string, uint>::iterator it = m_Index->Positions.begin(); it != m_Index->Positions.end(); ++it)
			if (it->second > (uint)pos)
				--it->second;
	}
	return true;
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
		if (m_Number != m_Number || m_Number > DBL_MAX || m_Number < -DBL_MAX)
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
	// into a value of its own, since text may be a part of this one
	CJsonValue result;
	CJsonParser parser(text);
	if (!parser.parseDocument(result, error))
	{
		CJsonValue().swap(*this);
		return false;
	}
	swap(result);
	return true;
}

} // NLMISC

/* End of json.cpp */
