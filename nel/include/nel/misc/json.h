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

#ifndef NL_JSON_H
#define NL_JSON_H

#include "types_nl.h"

#include <string>
#include <utility>
#include <vector>

namespace NLMISC {

/**
 * A JSON value: null, a boolean, a number, a string, an array or an object.
 *
 * Objects keep their members in the order they were set, so a result reads in the order it
 * was built. Numbers are doubles, so integers are exact up to 2^53; larger ones, such as
 * entity ids, travel as strings. Strings are UTF-8. Writing replaces a byte sequence that is
 * not UTF-8 with U+FFFD, so the text is always valid JSON for a strict reader.
 */
class CJsonValue
{
public:
	enum TType
	{
		Null,
		Bool,
		Number,
		String,
		Array,
		Object
	};

	typedef std::vector<CJsonValue> TItems;
	typedef std::vector<std::pair<std::string, CJsonValue>> TMembers;

	CJsonValue();
	CJsonValue(bool value);
	CJsonValue(sint32 value);
	CJsonValue(uint32 value);
	CJsonValue(sint64 value);
	CJsonValue(uint64 value);
	CJsonValue(double value);
	CJsonValue(const std::string &value);
	CJsonValue(const char *value);

	static CJsonValue array();
	static CJsonValue object();
	/// The value a lookup returns when there is nothing there
	static const CJsonValue &null();

	TType getType() const { return m_Type; }
	bool isNull() const { return m_Type == Null; }
	bool isBool() const { return m_Type == Bool; }
	bool isNumber() const { return m_Type == Number; }
	bool isString() const { return m_Type == String; }
	bool isArray() const { return m_Type == Array; }
	bool isObject() const { return m_Type == Object; }

	/// The value, or the default when the type differs
	bool getBool(bool defaultValue = false) const;
	double getNumber(double defaultValue = 0.0) const;
	/// A number rounded toward zero, or the default when the type differs
	sint64 getInt(sint64 defaultValue = 0) const;
	const std::string &getString() const;
	std::string getString(const std::string &defaultValue) const;

	/// The item count of an array, the member count of an object, 0 otherwise
	uint size() const;

	// *** arrays ***

	/// The item at an index, null when out of range or not an array
	const CJsonValue &at(uint index) const;
	/// Appends an item, turning a null into an array first; returns the stored item
	CJsonValue &push(const CJsonValue &value);
	const TItems &items() const { return m_Items; }

	// *** objects ***

	bool has(const std::string &key) const;
	/// The member of a key, null when absent or not an object
	const CJsonValue &get(const std::string &key) const;
	/// Sets a member, turning a null into an object first; returns this value, for chaining
	CJsonValue &set(const std::string &key, const CJsonValue &value);
	/// The member of a key, created as null when absent
	CJsonValue &member(const std::string &key);
	bool erase(const std::string &key);
	const TMembers &members() const { return m_Members; }

	// *** text ***

	/// Compact JSON text
	std::string toString() const;
	void write(std::string &out) const;
	/// Writes a JSON string literal, quotes included
	static void writeString(std::string &out, const std::string &value);

	/// Parses JSON text into this value; on failure the value is null and the error says where
	bool parse(const std::string &text, std::string &error);

private:
	friend class CJsonParser;

	TType m_Type;
	bool m_Bool;
	double m_Number;
	std::string m_String;
	TItems m_Items;
	TMembers m_Members;
};

} // NLMISC

#endif // NL_JSON_H

/* End of json.h */
