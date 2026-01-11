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

// Scintilla source code edit control
/** @file KeyMap.h
 ** Defines a mapping between keystrokes and commands.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef KEYTOCOMMAND_H
#define KEYTOCOMMAND_H

#define SCI_NORM 0
#define SCI_SHIFT SCMOD_SHIFT
#define SCI_CTRL SCMOD_CTRL
#define SCI_ALT SCMOD_ALT
#define SCI_CSHIFT (SCI_CTRL | SCI_SHIFT)
#define SCI_ASHIFT (SCI_ALT | SCI_SHIFT)

/**
 */
class KeyToCommand {
public:
	int key;
	int modifiers;
	unsigned int msg;
};

/**
 */
class KeyMap {
	KeyToCommand *kmap;
	int len;
	int alloc;
	static const KeyToCommand MapDefault[];

public:
	KeyMap();
	~KeyMap();
	void Clear();
	void AssignCmdKey(int key, int modifiers, unsigned int msg);
	unsigned int Find(int key, int modifiers);	// 0 returned on failure
};

#endif
