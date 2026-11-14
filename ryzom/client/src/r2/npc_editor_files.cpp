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
//
#include "npc_editor_files.h"
//
#include "nel/gui/lua_helper.h"
#include "nel/gui/lua_ihm.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/string_common.h"

#include <cmath>
#include <cstdlib>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace NLGUI;

namespace R2 {

namespace {

// The lists of NPC names of the client data, <name>.txt
const char *const s_NameLists[] = {
	"fyros_male", "fyros_female", "matis_male", "matis_female",
	"tryker_male", "tryker_female", "zorai_male", "zorai_female"
};

// The tables of look presets of the NPC editor, save/<name>.txt
const char *const s_PresetTables[] = {
	"equipmentSets", "tableBodySets", "tableFaceSets", "tableFaceMorph"
};

// A list of names or a preset file larger than this is not read, and no larger preset file is written
const uint32 s_MaxFileSize = 1024 * 1024;

// Tables inside tables in a preset file; the equipment sets, the deepest, have four levels
const uint32 s_MaxPresetDepth = 8;

bool isOneOf(const std::string &name, const char *const *names, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		if (name == names[i]) return true;
	}
	return false;
}

// The whole of a file. False with the reason when it cannot be had.
bool readFile(const std::string &path, std::string &content, std::string &error)
{
	CIFile file;
	if (!file.open(path))
	{
		error = "the file cannot be opened";
		return false;
	}
	uint32 size = file.getFileSize();
	if (size > s_MaxFileSize)
	{
		error = toString("the file has %u bytes, more than the %u a file of this kind can have", size, s_MaxFileSize);
		return false;
	}
	content.resize(size);
	try
	{
		if (size > 0) file.serialBuffer((uint8 *)&content[0], size);
	}
	catch (const Exception &e)
	{
		error = e.what();
		return false;
	}
	return true;
}

// The lines of a text, without their ends
void splitLines(const std::string &text, std::vector<std::string> &lines)
{
	std::string::size_type start = 0;
	while (start < text.size())
	{
		std::string::size_type end = text.find('\n', start);
		if (end == std::string::npos) end = text.size();
		std::string::size_type last = end;
		if (last > start && text[last - 1] == '\r') --last;
		lines.push_back(text.substr(start, last - start));
		start = end + 1;
	}
}

// A number as a preset file has it. False when the text is not a number and nothing else.
bool parseNumber(const std::string &text, double &value)
{
	if (text.empty() || text.size() > 64) return false;
	char *end = NULL;
	value = strtod(text.c_str(), &end);
	return end == text.c_str() + text.size();
}

void pushNumber(CLuaState &ls, double value)
{
	// a whole number is an integer for the scripts, as the number of a slider or of a colour is
	if (value == floor(value) && fabs(value) < 9007199254740992.0)
		ls.push((sint64)value);
	else
		ls.push(value);
}

std::string numberText(double value)
{
	// as the first editor wrote its numbers (Lua 5.1): a whole number without a fraction
	if (value == floor(value) && fabs(value) < 1e15)
		return toString("%.0f", value);
	return toString("%.14g", value);
}

bool hasLineBreak(const std::string &text)
{
	return text.find('\n') != std::string::npos || text.find('\r') != std::string::npos;
}

// Reads the entries of a table, from the line after its "{" to its "}", into the table on the top of
// the stack. line is the next line to read, and is left after the "}".
bool parsePresetTable(CLuaState &ls, const std::vector<std::string> &lines, size_t &line, uint32 depth, std::string &error)
{
	lua_State *state = ls.getStatePointer();
	if (depth > s_MaxPresetDepth)
	{
		error = toString("line %u: tables inside one another, more than %u deep", (uint32)line, s_MaxPresetDepth);
		return false;
	}
	if (!lua_checkstack(state, 4))
	{
		error = "no room on the stack of the scripts";
		return false;
	}
	for (;;)
	{
		if (line >= lines.size())
		{
			error = toString("line %u: the file ends inside a table", (uint32)lines.size());
			return false;
		}
		const std::string &text = lines[line];
		++line;
		if (text == "}") return true;

		// the key: a text between square brackets, or a number
		std::string value;
		if (!text.empty() && text[0] == '[')
		{
			std::string::size_type end = text.find("] = ");
			if (end == std::string::npos)
			{
				error = toString("line %u: a name without its \"] = \"", (uint32)line);
				return false;
			}
			ls.push(text.substr(1, end - 1));
			value = text.substr(end + 4);
		}
		else
		{
			std::string::size_type end = text.find(" = ");
			double number = 0;
			if (end == std::string::npos || !parseNumber(text.substr(0, end), number))
			{
				error = toString("line %u: neither \"}\" nor \"name = value\"", (uint32)line);
				return false;
			}
			pushNumber(ls, number);
			value = text.substr(end + 3);
		}

		// the value: a table, a text between square brackets, or a number
		if (value == "{")
		{
			ls.newTable();
			if (!parsePresetTable(ls, lines, line, depth + 1, error))
			{
				ls.pop(2); // the table and the key
				return false;
			}
		}
		else if (!value.empty() && value[0] == '[')
		{
			if (value.size() < 2 || value[value.size() - 1] != ']')
			{
				error = toString("line %u: a text without its \"]\"", (uint32)line);
				ls.pop(); // the key
				return false;
			}
			ls.push(value.substr(1, value.size() - 2));
		}
		else
		{
			double number = 0;
			if (!parseNumber(value, number))
			{
				error = toString("line %u: the value is not a table, a text or a number", (uint32)line);
				ls.pop(); // the key
				return false;
			}
			pushNumber(ls, number);
		}
		ls.setTable(-3);
	}
}

// Writes the table at the absolute stack index 'index'
bool serializePresetTable(CLuaState &ls, int index, uint32 depth, std::string &text, std::string &error)
{
	lua_State *state = ls.getStatePointer();
	if (depth > s_MaxPresetDepth)
	{
		error = toString("tables inside one another, more than %u deep", s_MaxPresetDepth);
		return false;
	}
	if (!lua_checkstack(state, 4))
	{
		error = "no room on the stack of the scripts";
		return false;
	}
	text += "{\n";
	ls.pushNil();
	while (ls.next(index))
	{
		// the key is at -2 and stays as it is for the next step: a number is not turned into a text in place
		bool ok = true;
		int keyType = ls.type(-2);
		if (keyType == LUA_TSTRING)
		{
			size_t length = 0;
			const char *chars = lua_tolstring(state, -2, &length);
			std::string key(chars, length);
			if (hasLineBreak(key) || key.find("] = ") != std::string::npos)
			{
				error = "the name '" + key + "' has a line break or \"] = \" in it, which a preset file cannot hold";
				ok = false;
			}
			text += "[" + key + "]";
		}
		else if (keyType == LUA_TNUMBER)
		{
			text += numberText(ls.toNumber(-2));
		}
		else
		{
			error = std::string("a ") + ls.getTypename(keyType) + " is the name of an entry, a preset file has texts and numbers";
			ok = false;
		}
		text += " = ";

		if (ok)
		{
			int valueType = ls.type(-1);
			if (valueType == LUA_TNUMBER)
			{
				text += numberText(ls.toNumber(-1));
			}
			else if (valueType == LUA_TSTRING)
			{
				size_t length = 0;
				const char *chars = lua_tolstring(state, -1, &length);
				std::string value(chars, length);
				if (hasLineBreak(value))
				{
					error = "the text '" + value + "' has a line break in it, which a preset file cannot hold";
					ok = false;
				}
				text += "[" + value + "]";
			}
			else if (valueType == LUA_TTABLE)
			{
				ok = serializePresetTable(ls, ls.getTop(), depth + 1, text, error);
			}
			else
			{
				error = std::string("a ") + ls.getTypename(valueType) + " is the value of an entry, a preset file has tables, texts and numbers";
				ok = false;
			}
		}
		if (ok && text.size() > s_MaxFileSize)
		{
			error = toString("the presets make more than the %u bytes a preset file can have", s_MaxFileSize);
			ok = false;
		}
		if (!ok)
		{
			ls.pop(2); // the value and the key
			return false;
		}
		text += "\n";
		ls.pop(); // the value; the key stays for next
	}
	text += "}";
	return true;
}

// The name a script gave as the argument 2 of a method of r2, when it is one of 'names'. The script gets
// an error for any other value: the names are a fixed set, and a path is not one of them.
std::string checkedName(CLuaState &ls, const char *funcName, const char *const *names, size_t count, bool lowerCase)
{
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TSTRING);
	std::string name = ls.toString(2);
	if (lowerCase) name = toLowerAscii(name);
	if (!isOneOf(name, names, count) || !CFile::isSafeFilename(name, false))
	{
		CLuaIHM::fails(ls, "%s: '%s' is not one of the names this function takes", funcName, name.c_str());
	}
	return name;
}

} // namespace

// *********************************************************************************************************
std::string CNpcEditorFiles::presetPath(const std::string &name)
{
	// where the first editor kept them: the save directory of the client, below its working directory
	return "save/" + name + ".txt";
}

// *********************************************************************************************************
bool CNpcEditorFiles::parsePresets(CLuaState &ls, const std::string &text, std::string &error)
{
	std::vector<std::string> lines;
	splitLines(text, lines);
	ls.newTable();
	// a file without a line is a file without presets: the first editor left one when a write failed
	bool empty = true;
	for (size_t i = 0; i < lines.size() && empty; ++i)
	{
		empty = lines[i].empty();
	}
	if (empty) return true;
	if (lines[0] != "{")
	{
		error = "line 1: the file does not start with \"{\"";
		ls.pop();
		return false;
	}
	size_t line = 1;
	if (!parsePresetTable(ls, lines, line, 1, error))
	{
		ls.pop();
		return false;
	}
	return true;
}

// *********************************************************************************************************
bool CNpcEditorFiles::serializePresets(CLuaState &ls, int index, std::string &text, std::string &error)
{
	if (index < 0) index = ls.getTop() + index + 1;
	text.clear();
	return serializePresetTable(ls, index, 1, text, error);
}

// *********************************************************************************************************
int CNpcEditorFiles::luaLoadNpcNames(CLuaState &ls)
{
	const char *funcName = "loadNpcNames";
	CLuaIHM::checkArgCount(ls, funcName, 2); // a method: self and the name
	std::string name = checkedName(ls, funcName, s_NameLists, sizeof(s_NameLists) / sizeof(s_NameLists[0]), true);

	std::string path = CPath::lookup(name + ".txt", false, false);
	std::string content;
	std::string error;
	if (path.empty())
	{
		error = "it is not in the search paths";
	}
	else if (readFile(path, content, error))
	{
		std::vector<std::string> lines;
		splitLines(content, lines);
		ls.newTable();
		int count = 0;
		for (size_t i = 0; i < lines.size(); ++i)
		{
			if (lines[i].empty()) continue; // a name has at least a letter
			ls.push(lines[i]);
			ls.rawSetI(-2, ++count);
		}
		return 1;
	}
	ls.pushNil();
	ls.push(error);
	return 2;
}

// *********************************************************************************************************
int CNpcEditorFiles::luaLoadNpcPresets(CLuaState &ls)
{
	const char *funcName = "loadNpcPresets";
	CLuaIHM::checkArgCount(ls, funcName, 2); // a method: self and the name
	std::string name = checkedName(ls, funcName, s_PresetTables, sizeof(s_PresetTables) / sizeof(s_PresetTables[0]), false);

	std::string path = presetPath(name);
	if (!CFile::fileExists(path))
	{
		// no presets were saved yet
		ls.newTable();
		return 1;
	}
	std::string content;
	std::string error;
	if (readFile(path, content, error) && parsePresets(ls, content, error))
	{
		return 1;
	}
	nlwarning("R2: the NPC look presets of '%s' are not read: %s", path.c_str(), error.c_str());
	ls.pushNil();
	ls.push(CPath::getFullPath(path, false));
	ls.push(error);
	return 3;
}

// *********************************************************************************************************
int CNpcEditorFiles::luaSaveNpcPresets(CLuaState &ls)
{
	const char *funcName = "saveNpcPresets";
	CLuaIHM::checkArgCount(ls, funcName, 3); // a method: self, the name and the table
	std::string name = checkedName(ls, funcName, s_PresetTables, sizeof(s_PresetTables) / sizeof(s_PresetTables[0]), false);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TTABLE);

	std::string path = presetPath(name);
	std::string text;
	std::string error;
	bool ok = serializePresets(ls, 3, text, error);
	if (ok)
	{
		// The file is written beside itself and takes its place when it is whole. The directory is the
		// one the client keeps its files in: it is not made here, and no other place is tried.
		try
		{
			COFile file;
			if (!file.open(path, false, false, true))
			{
				error = "the file cannot be opened for writing";
				ok = false;
			}
			else
			{
				if (!text.empty()) file.serialBuffer((uint8 *)&text[0], (uint)text.size());
				file.close();
			}
		}
		catch (const Exception &e)
		{
			error = e.what();
			ok = false;
		}
	}
	if (ok)
	{
		ls.push(true);
		return 1;
	}
	nlwarning("R2: the NPC look presets of '%s' are not written: %s", path.c_str(), error.c_str());
	ls.push(false);
	ls.push(CPath::getFullPath(path, false));
	ls.push(error);
	return 3;
}

} // namespace R2
