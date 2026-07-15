/**
 * \file json_value.cpp
 * \brief See json_value.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>

// MSVC 9.0 (VS2008) has no C99 snprintf/strtoll; the MS spellings are equivalent here.
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#define strtoll _strtoi64
#endif
#include "json_value.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

namespace NLGLTF {

std::string formatJsonFloat(float v)
{
	char buf[48];
	// glTF JSON forbids NaN/Inf tokens; 0.0 keeps the file valid (callers should not feed these).
	// Compared as double: the 3.5e38 bound exceeds FLT_MAX (only ±Inf can reach it), and VC90
	// rejects an out-of-range float literal outright.
	if (v != v || (double)v > 3.5e38 || (double)v < -3.5e38)
		return "0.0";
	snprintf(buf, sizeof(buf), "%.9g", (double)v);
	if (!strchr(buf, '.') && !strchr(buf, 'e') && !strchr(buf, 'E'))
	{
		size_t n = strlen(buf);
		buf[n] = '.';
		buf[n + 1] = '0';
		buf[n + 2] = 0;
	}
	return buf;
}

CJsonValue::~CJsonValue()
{
	clear();
}

void CJsonValue::clear()
{
	for (size_t i = 0; i < m_Array.size(); ++i)
		delete m_Array[i];
	m_Array.clear();
	for (size_t i = 0; i < m_Object.size(); ++i)
		delete m_Object[i].second;
	m_Object.clear();
	m_String.clear();
	m_Bool = false;
	m_Int = 0;
	m_Double = 0.0;
	m_Type = Null;
}

sint64 CJsonValue::asInt() const
{
	if (m_Type == Int) return m_Int;
	if (m_Type == Double) return (sint64)m_Double;
	if (m_Type == Bool) return m_Bool ? 1 : 0;
	return 0;
}

double CJsonValue::asDouble() const
{
	if (m_Type == Double) return m_Double;
	if (m_Type == Int) return (double)m_Int;
	return 0.0;
}

CJsonValue *CJsonValue::push()
{
	if (m_Type != Array)
	{
		clear();
		m_Type = Array;
	}
	CJsonValue *v = new CJsonValue();
	m_Array.push_back(v);
	return v;
}

const CJsonValue *CJsonValue::get(const char *key) const
{
	if (m_Type != Object) return NULL;
	for (size_t i = 0; i < m_Object.size(); ++i)
		if (m_Object[i].first == key)
			return m_Object[i].second;
	return NULL;
}

CJsonValue *CJsonValue::set(const char *key)
{
	if (m_Type != Object)
	{
		clear();
		m_Type = Object;
	}
	for (size_t i = 0; i < m_Object.size(); ++i)
	{
		if (m_Object[i].first == key)
		{
			m_Object[i].second->clear();
			return m_Object[i].second;
		}
	}
	CJsonValue *v = new CJsonValue();
	m_Object.push_back(std::make_pair(std::string(key), v));
	return v;
}

CJsonValue *CJsonValue::getMutable(const char *key)
{
	if (m_Type != Object) return NULL;
	for (size_t i = 0; i < m_Object.size(); ++i)
		if (m_Object[i].first == key)
			return m_Object[i].second;
	return NULL;
}

CJsonValue *CJsonValue::ensureObject(const char *key)
{
	CJsonValue *v = getMutable(key);
	if (v && v->isObject()) return v;
	if (v) { v->clear(); v->m_Type = Object; return v; }
	v = set(key);
	v->m_Type = Object;
	return v;
}

bool CJsonValue::getBool(const char *key, bool def) const
{
	const CJsonValue *v = get(key);
	if (!v || !v->isBool()) return def;
	return v->asBool();
}

sint64 CJsonValue::getInt(const char *key, sint64 def) const
{
	const CJsonValue *v = get(key);
	if (!v || !v->isNumber()) return def;
	return v->asInt();
}

double CJsonValue::getDouble(const char *key, double def) const
{
	const CJsonValue *v = get(key);
	if (!v || !v->isNumber()) return def;
	return v->asDouble();
}

std::string CJsonValue::getString(const char *key, const std::string &def) const
{
	const CJsonValue *v = get(key);
	if (!v || !v->isString()) return def;
	return v->asString();
}

// ---------------------------------------------------------------------------------------------
// Writer

static void writeEscaped(const std::string &s, std::string &out)
{
	out += '"';
	for (size_t i = 0; i < s.size(); ++i)
	{
		char c = s[i];
		switch (c)
		{
		case '"': out += "\\\""; break;
		case '\\': out += "\\\\"; break;
		case '\n': out += "\\n"; break;
		case '\r': out += "\\r"; break;
		case '\t': out += "\\t"; break;
		default:
			if ((unsigned char)c < 0x20)
			{
				char buf[8];
				snprintf(buf, sizeof(buf), "\\u%04x", (int)(unsigned char)c);
				out += buf;
			}
			else out += c;
		}
	}
	out += '"';
}

void CJsonValue::write(std::string &out) const
{
	char buf[32];
	switch (m_Type)
	{
	case Null:
		out += "null";
		break;
	case Bool:
		out += m_Bool ? "true" : "false";
		break;
	case Int:
		snprintf(buf, sizeof(buf), "%lld", (long long)m_Int);
		out += buf;
		break;
	case Double:
		out += formatJsonFloat((float)m_Double);
		break;
	case String:
		writeEscaped(m_String, out);
		break;
	case Array:
		out += '[';
		for (size_t i = 0; i < m_Array.size(); ++i)
		{
			if (i) out += ',';
			m_Array[i]->write(out);
		}
		out += ']';
		break;
	case Object:
		out += '{';
		for (size_t i = 0; i < m_Object.size(); ++i)
		{
			if (i) out += ',';
			writeEscaped(m_Object[i].first, out);
			out += ':';
			m_Object[i].second->write(out);
		}
		out += '}';
		break;
	}
}

// ---------------------------------------------------------------------------------------------
// Parser

static void skipWs(const char *&p, const char *end)
{
	while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
		++p;
}

static bool parseString(const char *&p, const char *end, std::string &out, std::string *err)
{
	if (p >= end || *p != '"')
	{
		if (err) *err = "expected string";
		return false;
	}
	++p;
	out.clear();
	while (p < end)
	{
		char c = *p++;
		if (c == '"') return true;
		if (c == '\\')
		{
			if (p >= end) break;
			char e = *p++;
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
			case 'u':
			{
				if (end - p < 4) { if (err) *err = "bad \\u escape"; return false; }
				unsigned int cp = 0;
				for (int k = 0; k < 4; ++k)
				{
					char h = *p++;
					cp <<= 4;
					if (h >= '0' && h <= '9') cp |= (unsigned)(h - '0');
					else if (h >= 'a' && h <= 'f') cp |= (unsigned)(h - 'a' + 10);
					else if (h >= 'A' && h <= 'F') cp |= (unsigned)(h - 'A' + 10);
					else { if (err) *err = "bad \\u escape"; return false; }
				}
				// surrogate pair
				if (cp >= 0xD800 && cp <= 0xDBFF && end - p >= 6 && p[0] == '\\' && p[1] == 'u')
				{
					unsigned int lo = 0;
					const char *q = p + 2;
					bool ok = true;
					for (int k = 0; k < 4; ++k)
					{
						char h = q[k];
						lo <<= 4;
						if (h >= '0' && h <= '9') lo |= (unsigned)(h - '0');
						else if (h >= 'a' && h <= 'f') lo |= (unsigned)(h - 'a' + 10);
						else if (h >= 'A' && h <= 'F') lo |= (unsigned)(h - 'A' + 10);
						else { ok = false; break; }
					}
					if (ok && lo >= 0xDC00 && lo <= 0xDFFF)
					{
						cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
						p = q + 4;
					}
				}
				// UTF-8 encode
				if (cp < 0x80) out += (char)cp;
				else if (cp < 0x800)
				{
					out += (char)(0xC0 | (cp >> 6));
					out += (char)(0x80 | (cp & 0x3F));
				}
				else if (cp < 0x10000)
				{
					out += (char)(0xE0 | (cp >> 12));
					out += (char)(0x80 | ((cp >> 6) & 0x3F));
					out += (char)(0x80 | (cp & 0x3F));
				}
				else
				{
					out += (char)(0xF0 | (cp >> 18));
					out += (char)(0x80 | ((cp >> 12) & 0x3F));
					out += (char)(0x80 | ((cp >> 6) & 0x3F));
					out += (char)(0x80 | (cp & 0x3F));
				}
				break;
			}
			default:
				if (err) *err = "bad escape";
				return false;
			}
		}
		else out += c;
	}
	if (err) *err = "unterminated string";
	return false;
}

bool CJsonValue::parseValue(const char *&p, const char *end, std::string *err, int depth)
{
	if (depth > 128)
	{
		if (err) *err = "nesting too deep";
		return false;
	}
	skipWs(p, end);
	if (p >= end)
	{
		if (err) *err = "unexpected end";
		return false;
	}
	clear();
	char c = *p;
	if (c == '{')
	{
		++p;
		m_Type = Object;
		skipWs(p, end);
		if (p < end && *p == '}') { ++p; return true; }
		for (;;)
		{
			skipWs(p, end);
			std::string key;
			if (!parseString(p, end, key, err)) return false;
			skipWs(p, end);
			if (p >= end || *p != ':') { if (err) *err = "expected ':'"; return false; }
			++p;
			CJsonValue *v = new CJsonValue();
			m_Object.push_back(std::make_pair(key, v));
			if (!v->parseValue(p, end, err, depth + 1)) return false;
			skipWs(p, end);
			if (p < end && *p == ',') { ++p; continue; }
			if (p < end && *p == '}') { ++p; return true; }
			if (err) *err = "expected ',' or '}'";
			return false;
		}
	}
	if (c == '[')
	{
		++p;
		m_Type = Array;
		skipWs(p, end);
		if (p < end && *p == ']') { ++p; return true; }
		for (;;)
		{
			CJsonValue *v = new CJsonValue();
			m_Array.push_back(v);
			if (!v->parseValue(p, end, err, depth + 1)) return false;
			skipWs(p, end);
			if (p < end && *p == ',') { ++p; continue; }
			if (p < end && *p == ']') { ++p; return true; }
			if (err) *err = "expected ',' or ']'";
			return false;
		}
	}
	if (c == '"')
	{
		m_Type = String;
		return parseString(p, end, m_String, err);
	}
	if (c == 't' && end - p >= 4 && !strncmp(p, "true", 4))
	{
		m_Type = Bool;
		m_Bool = true;
		p += 4;
		return true;
	}
	if (c == 'f' && end - p >= 5 && !strncmp(p, "false", 5))
	{
		m_Type = Bool;
		m_Bool = false;
		p += 5;
		return true;
	}
	if (c == 'n' && end - p >= 4 && !strncmp(p, "null", 4))
	{
		m_Type = Null;
		p += 4;
		return true;
	}
	// number
	{
		const char *start = p;
		if (p < end && (*p == '-' || *p == '+')) ++p;
		bool isFloat = false;
		while (p < end && ((*p >= '0' && *p <= '9') || *p == '.' || *p == 'e' || *p == 'E' || *p == '-' || *p == '+'))
		{
			if (*p == '.' || *p == 'e' || *p == 'E') isFloat = true;
			++p;
		}
		if (p == start)
		{
			if (err) *err = "unexpected character";
			return false;
		}
		std::string tok(start, p);
		if (isFloat)
		{
			m_Type = Double;
			m_Double = strtod(tok.c_str(), NULL);
		}
		else
		{
			m_Type = Int;
			m_Int = (sint64)strtoll(tok.c_str(), NULL, 10);
		}
		return true;
	}
}

bool CJsonValue::parse(const std::string &in, std::string *err)
{
	const char *p = in.c_str();
	const char *end = p + in.size();
	if (!parseValue(p, end, err, 0)) return false;
	skipWs(p, end);
	if (p != end)
	{
		if (err) *err = "trailing data";
		return false;
	}
	return true;
}

} /* namespace NLGLTF */

/* end of file */
