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

#ifndef R2_NPC_EDITOR_FILES_H
#define R2_NPC_EDITOR_FILES_H

#include "nel/misc/types_nl.h"
#include <string>

namespace NLGUI {
class CLuaState;
}

namespace R2 {

/**
 * The files of the NPC editor of the Ring: the lists of NPC names of the client data, and the look
 * presets the author keeps (body, face, morph and equipment sets).
 *
 * The scripts of the interface have no file access of their own (no io library): the same Lua state runs
 * scripts served by the trusted web domains. These are the calls the NPC editor has in its place. A script
 * names a list or a preset table from a fixed set, never a file or a path, and the content of a preset
 * file is written here from a table, in the one form such a file has.
 *
 *   r2:loadNpcNames(list)            list is "<race>_<sex>", as "fyros_male". The names of <list>.txt of
 *                                    the search paths as a table; nil and a reason when the client has
 *                                    no such file or cannot read it.
 *   r2:loadNpcPresets(table)         table is "equipmentSets", "tableBodySets", "tableFaceSets" or
 *                                    "tableFaceMorph". The presets of save/<table>.txt as a table, empty
 *                                    when there is no such file; nil, the path and a reason when the file
 *                                    is there and cannot be read as presets.
 *   r2:saveNpcPresets(table, value)  writes save/<table>.txt, where the editor has always kept it. true;
 *                                    false, the path and a reason when it is not written. The file is
 *                                    replaced in one step: a write that fails leaves it as it was.
 *
 * A preset file is the text the first editor wrote with its own serializer, and a file of an older
 * client is read back: a table is "{", one "key = value" line per entry and "}"; a text is between
 * square brackets, a number is bare, and the value "{" starts a table.
 */
class CNpcEditorFiles
{
public:
	static int luaLoadNpcNames(NLGUI::CLuaState &ls);
	static int luaLoadNpcPresets(NLGUI::CLuaState &ls);
	static int luaSaveNpcPresets(NLGUI::CLuaState &ls);

	/// The file of a table of look presets, relative to the working directory of the client
	static std::string presetPath(const std::string &name);

	/// Reads the text of a preset file into a table that is left on the top of the stack. When it returns
	/// false the stack is as it was and error says what is wrong, with the line.
	static bool parsePresets(NLGUI::CLuaState &ls, const std::string &text, std::string &error);
	/// Writes the table at the given stack index as the text of a preset file. When it returns false
	/// error says which value the file cannot hold.
	static bool serializePresets(NLGUI::CLuaState &ls, int index, std::string &text, std::string &error);
};

} // namespace R2

#endif // R2_NPC_EDITOR_FILES_H
