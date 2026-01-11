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

#ifndef NL_EXTERNAL_EDITOR_H
#define NL_EXTERNAL_EDITOR_H

/* Launch an external text editor to edit the string passed in parameter. 
 * Returns true if the editing is successful and text is filled with the new text
 * Returns false else.
 * ext is the extension of the temporary file ("txt");
 */
bool EditExternalText (const std::string &editor, std::string &text, const std::string &ext);

#endif // NL_EXTERNAL_EDITOR_H

/* End of external_editor.h */
