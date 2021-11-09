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
/** @file AutoComplete.h
 ** Defines the auto completion list box.
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

/**
 */
class AutoComplete {
	bool active;
	char stopChars[256];
	char fillUpChars[256];
	char separator;

public:
	bool ignoreCase;
	bool chooseSingle;
	ListBox lb;
	int posStart;
	int startLen;
	/// Should autocompletion be canceled if editor's currentPos <= startPos?
	bool cancelAtStartPos;
	bool autoHide;
	bool dropRestOfWord;

	AutoComplete();
	~AutoComplete();

	/// Is the auto completion list displayed?
	bool Active();

	/// Display the auto completion list positioned to be near a character position
	void Start(Window &parent, int ctrlID, int position, int startLen_);

	/// The stop chars are characters which, when typed, cause the auto completion list to disappear
	void SetStopChars(const char *stopChars_);
	bool IsStopChar(char ch);

	/// The fillup chars are characters which, when typed, fill up the selected word
	void SetFillUpChars(const char *fillUpChars_);
	bool IsFillUpChar(char ch);

	/// The separator character is used when interpreting the list in SetList
	void SetSeparator(char separator_);
	char GetSeparator();

	/// The list string contains a sequence of words separated by the separator character
	void SetList(const char *list);

	void Show();
	void Cancel();

	/// Move the current list element by delta, scrolling appropriately
	void Move(int delta);

	/// Select a list element that starts with word as the current element
	void Select(const char *word);
};

#endif
