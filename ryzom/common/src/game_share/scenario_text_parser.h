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

#ifndef R2_SCENARIO_TEXT_PARSER_H
#define R2_SCENARIO_TEXT_PARSER_H

#include "nel/misc/types_nl.h"
#include "object.h"

#include <string>
#include <vector>

namespace R2 {

/**
 * Reads a scenario file (what CObject::serialize() writes after "scenario = ", with the comment
 * header CScenarioValidator puts in front) back into a CObject tree without running Lua.
 *
 * The text looks like a Lua table and the files used to be executed as a Lua script, so a shared
 * scenario could run code in the editor. This parser accepts the constructs the writer produces
 * and nothing else: nested tables, "key = value" entries and bare values, [[long bracket]] strings
 * joined with "..", quoted strings with the usual escapes, r2.RefId([[...]]), decimal numbers, true
 * and false, and "--" comments. A function call, an operator or any other Lua is a parse error, and
 * so is a key given twice in one table.
 *
 * One forward pass over the text, character by character, with an explicit stack for the open
 * tables, so a deep file cannot exhaust the native stack; the depth and the node count are capped.
 * Table entries keep their file order, which is the order the writer sorted them in.
 */
class CScenarioTextParser
{
public:
	/// Largest nesting of tables accepted
	static const uint32 MaxDepth = 256;
	/// Largest number of values accepted in one file (the shipped example scenarios have about 10000)
	static const uint32 MaxNodes = 1024 * 1024;

	/**
	 * Parses "scenario = { ... }", with whitespace and comments allowed around it. The objects come
	 * from the factory when one is given (the client uses its own table and ref id classes), else
	 * from the plain classes. Returns NULL and fills errorMsg, with a line and column, when the text
	 * is not accepted.
	 */
	static CObject::TSmartPtr parse(const std::string &text, CObjectFactory *factory, std::string &errorMsg);

private:
	struct CFrame
	{
		CObjectTable *Table;
		std::string Key;
		bool HasKey; // a "key =" was read and its value is pending
		bool ExpectSeparator; // a value was added, so a ',' or the closing '}' must follow

		CFrame(CObjectTable *table)
		    : Table(table)
		    , HasKey(false)
		    , ExpectSeparator(false)
		{
		}
	};

	CScenarioTextParser(const std::string &text, CObjectFactory *factory);

	CObject::TSmartPtr run();

	// Scanning
	bool atEnd() const { return m_Pos >= m_Text.size(); }
	char peek(size_t ahead = 0) const { return m_Pos + ahead < m_Text.size() ? m_Text[m_Pos + ahead] : '\0'; }
	void advance();
	bool accept(char c);
	void skipSpaceAndComments();
	bool readIdentifier(std::string &out);
	bool readString(std::string &out);
	bool readLongBracket(std::string &out);
	bool readQuoted(std::string &out);
	bool readNumber(CObject::TSmartPtr &out);
	bool readRefId(CObject::TSmartPtr &out);
	bool valueFromIdentifier(const std::string &ident, CObject::TSmartPtr &out);

	// Building
	CObject::TSmartPtr newTable();
	CObject::TSmartPtr newString(const std::string &value);
	CObject::TSmartPtr newRefId(const std::string &value);
	CObject::TSmartPtr newInteger(sint64 value);
	CObject::TSmartPtr newNumber(double value);
	bool addValue(CFrame &frame, const CObject::TSmartPtr &value);

	/// Records what went wrong, with the position, and returns false
	bool fail(const char *what);

	const std::string &m_Text;
	CObjectFactory *m_Factory;
	size_t m_Pos;
	uint32 m_Line;
	uint32 m_Col;
	uint32 m_Nodes;
	std::string m_Error;
};

} // namespace R2

#endif // R2_SCENARIO_TEXT_PARSER_H
