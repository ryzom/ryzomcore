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
/** @file ContractionState.h
 ** Manages visibility of lines for folding.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef CONTRACTIONSTATE_H
#define CONTRACTIONSTATE_H

/**
 */
class OneLine {
public:
	int displayLine;	///< Position within set of visible lines
	//int docLine;		///< Inverse of @a displayLine
	int height;	///< Number of display lines needed to show all of the line
	bool visible;
	bool expanded;

	OneLine();
	virtual ~OneLine() {}
};

/**
 */
class ContractionState {
	void Grow(int sizeNew);
	enum { growSize = 4000 };
	int linesInDoc;
	mutable int linesInDisplay;
	mutable OneLine *lines;
	int size;
	mutable int *docLines;
	mutable int sizeDocLines;
	mutable bool valid;
	void MakeValid() const;

public:
	ContractionState();
	virtual ~ContractionState();

	void Clear();

	int LinesInDoc() const;
	int LinesDisplayed() const;
	int DisplayFromDoc(int lineDoc) const;
	int DocFromDisplay(int lineDisplay) const;

	void InsertLines(int lineDoc, int lineCount);
	void DeleteLines(int lineDoc, int lineCount);

	bool GetVisible(int lineDoc) const;
	bool SetVisible(int lineDocStart, int lineDocEnd, bool visible);

	bool GetExpanded(int lineDoc) const;
	bool SetExpanded(int lineDoc, bool expanded);

	int GetHeight(int lineDoc) const;
	bool SetHeight(int lineDoc, int height);

	void ShowAll();
};

#endif
