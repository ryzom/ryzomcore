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
/** @file Style.h
 ** Defines the font and colour style for a class of text.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef STYLE_H
#define STYLE_H

/**
 */
class Style {
public:
	ColourPair fore;
	ColourPair back;
	bool aliasOfDefaultFont;
	bool bold;
	bool italic;
	int size;
	const char *fontName;
	int characterSet;
	bool eolFilled;
	bool underline;
	enum ecaseForced {caseMixed, caseUpper, caseLower};
	ecaseForced caseForce;
	bool visible;
	bool changeable;

	Font font;
	int sizeZoomed;
	unsigned int lineHeight;
	unsigned int ascent;
	unsigned int descent;
	unsigned int externalLeading;
	unsigned int aveCharWidth;
	unsigned int spaceWidth;

	Style();
	Style(const Style &source);
	~Style();
	Style &operator=(const Style &source);
	void Clear(ColourDesired fore_, ColourDesired back_,
	           int size_,
	           const char *fontName_, int characterSet_,
	           bool bold_, bool italic_, bool eolFilled_, 
	           bool underline_, ecaseForced caseForce_, 
		   bool visible_, bool changeable_);
	void ClearTo(const Style &source);
	bool EquivalentFontTo(const Style *other) const;
	void Realise(Surface &surface, int zoomLevel, Style *defaultStyle = 0);
	bool IsProtected() { return !(changeable && visible);} ;
};

#endif
