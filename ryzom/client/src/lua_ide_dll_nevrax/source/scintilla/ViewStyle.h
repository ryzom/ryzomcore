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
/** @file ViewStyle.h
 ** Store information on how the document is to be viewed.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef VIEWSTYLE_H
#define VIEWSTYLE_H

/**
 */
class MarginStyle {
public:
	bool symbol;
	int width;
	int mask;
	bool sensitive;
	MarginStyle();
};

/**
 */
class FontNames {
private:
	char *names[STYLE_MAX + 1];
	int max;

public:
	FontNames();
	~FontNames();
	void Clear();
	const char *Save(const char *name);
};

enum WhiteSpaceVisibility {wsInvisible=0, wsVisibleAlways=1, wsVisibleAfterIndent=2};

/**
 */
class ViewStyle {
public:
	FontNames fontNames;
	Style styles[STYLE_MAX + 1];
	LineMarker markers[MARKER_MAX + 1];
	Indicator indicators[INDIC_MAX + 1];
	int lineHeight;
	unsigned int maxAscent;
	unsigned int maxDescent;
	unsigned int aveCharWidth;
	unsigned int spaceWidth;
	bool selforeset;
	ColourPair selforeground;
	bool selbackset;
	ColourPair selbackground;
	ColourPair selbackground2;
	ColourPair selbar;
	ColourPair selbarlight;
	/// Margins are ordered: Line Numbers, Selection Margin, Spacing Margin
	enum { margins=3 };
	int leftMarginWidth;	///< Spacing margin on left of text
	int rightMarginWidth;	///< Spacing margin on left of text
	bool symbolMargin;
	int maskInLine;	///< Mask for markers to be put into text because there is nowhere for them to go in margin
	MarginStyle ms[margins];
	int fixedColumnWidth;
	int zoomLevel;
	WhiteSpaceVisibility viewWhitespace;
	bool viewIndentationGuides;
	bool viewEOL;
	bool showMarkedLines;
	ColourPair caretcolour;
	bool showCaretLineBackground;
	ColourPair caretLineBackground;
	ColourPair edgecolour;
	int edgeState;
	int caretWidth;
	
	ViewStyle();
	ViewStyle(const ViewStyle &source);
	~ViewStyle();
	void Init();
	void RefreshColourPalette(Palette &pal, bool want);
	void Refresh(Surface &surface);
	void ResetDefaultStyle();
	void ClearStyles();
	void SetStyleFontName(int styleIndex, const char *name);
};

#endif
