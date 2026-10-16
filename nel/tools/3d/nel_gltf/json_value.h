/**
 * \file json_value.h
 * \brief Minimal ordered JSON document model for the NeL glTF toolchain: build, write, parse.
 * Object keys keep insertion order (stable emission), numbers distinguish integer from float
 * (floats emit as %.9g with a forced decimal marker so a float32 value round-trips exactly and
 * downstream typed readers see a floating-point token — see wiki drafts/nel_gltf_extras.md).
 * This is deliberately not a general-purpose JSON library: it supports exactly what the glTF
 * writer (pipeline_max_export_gltf) and the nel-extras glTF reader (mesh_utils) need, with no
 * external dependencies.
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

#ifndef NL_GLTF_JSON_VALUE_H
#define NL_GLTF_JSON_VALUE_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>
#include <utility>

namespace NLGLTF {

class CJsonValue
{
public:
	enum TType { Null, Bool, Int, Double, String, Array, Object };

	CJsonValue() : m_Type(Null), m_Bool(false), m_Int(0), m_Double(0.0) { }
	explicit CJsonValue(TType t) : m_Type(t), m_Bool(false), m_Int(0), m_Double(0.0) { }
	~CJsonValue();

	TType type() const { return m_Type; }
	bool isNull() const { return m_Type == Null; }
	bool isBool() const { return m_Type == Bool; }
	bool isInt() const { return m_Type == Int; }
	bool isNumber() const { return m_Type == Int || m_Type == Double; }
	bool isString() const { return m_Type == String; }
	bool isArray() const { return m_Type == Array; }
	bool isObject() const { return m_Type == Object; }

	// Scalar access (asDouble accepts Int too; asInt accepts an integral Double)
	bool asBool() const { return m_Bool; }
	sint64 asInt() const;
	double asDouble() const;
	float asFloat() const { return (float)asDouble(); }
	const std::string &asString() const { return m_String; }

	// Scalar setup
	void setNull() { clear(); m_Type = Null; }
	void setBool(bool b) { clear(); m_Type = Bool; m_Bool = b; }
	void setInt(sint64 i) { clear(); m_Type = Int; m_Int = i; }
	void setDouble(double d) { clear(); m_Type = Double; m_Double = d; }
	void setString(const std::string &s) { clear(); m_Type = String; m_String = s; }

	// Array access/build. Elements are owned by this value.
	size_t size() const { return m_Type == Array ? m_Array.size() : (m_Type == Object ? m_Object.size() : 0); }
	const CJsonValue *at(size_t i) const { return i < m_Array.size() ? m_Array[i] : nullptr; }
	CJsonValue *push(); // append a new null element, return it
	void pushInt(sint64 i) { push()->setInt(i); }
	void pushDouble(double d) { push()->setDouble(d); }
	void pushString(const std::string &s) { push()->setString(s); }

	// Object access/build. Members are owned by this value; keys keep insertion order.
	// get returns NULL when the key is absent (or this is not an object).
	const CJsonValue *get(const char *key) const;
	CJsonValue *set(const char *key); // add or replace, returns the (new empty) member
	void setBool(const char *key, bool b) { set(key)->setBool(b); }
	void setInt(const char *key, sint64 i) { set(key)->setInt(i); }
	void setDouble(const char *key, double d) { set(key)->setDouble(d); }
	void setString(const char *key, const std::string &s) { set(key)->setString(s); }
	CJsonValue *setArray(const char *key) { CJsonValue *v = set(key); v->clear(); v->m_Type = Array; return v; }
	CJsonValue *setObject(const char *key) { CJsonValue *v = set(key); v->clear(); v->m_Type = Object; return v; }
	CJsonValue *getMutable(const char *key); // NULL when absent
	// Existing object member, or a fresh object member — never clears existing content.
	CJsonValue *ensureObject(const char *key);
	const std::vector<std::pair<std::string, CJsonValue *> > &members() const { return m_Object; }

	// Typed convenience getters with defaults (object only)
	bool getBool(const char *key, bool def) const;
	sint64 getInt(const char *key, sint64 def) const;
	double getDouble(const char *key, double def) const;
	std::string getString(const char *key, const std::string &def) const;

	// Serialization. Floats (Double) are written %.9g with a forced decimal marker; strings are
	// escaped minimally (", \, control chars).
	void write(std::string &out) const;

	// Strict-enough parser for glTF JSON. Returns false on malformed input; *this becomes the
	// root value on success.
	bool parse(const std::string &in, std::string *err = nullptr);

	void clear();

private:
	CJsonValue(const CJsonValue &);            // non-copyable (owned children)
	CJsonValue &operator=(const CJsonValue &); // non-copyable

	TType m_Type;
	bool m_Bool;
	sint64 m_Int;
	double m_Double;
	std::string m_String;
	std::vector<CJsonValue *> m_Array;
	std::vector<std::pair<std::string, CJsonValue *> > m_Object;

	bool parseValue(const char *&p, const char *end, std::string *err, int depth);
};

// %.9g with forced decimal marker — the canonical float-for-JSON formatting of the NeL glTF
// convention (round-trips float32 exactly; the token reads back as a floating-point type).
std::string formatJsonFloat(float v);

} /* namespace NLGLTF */

#endif /* NL_GLTF_JSON_VALUE_H */

/* end of file */
