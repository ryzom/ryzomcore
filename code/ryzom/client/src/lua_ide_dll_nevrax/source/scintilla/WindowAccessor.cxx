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
/** @file WindowAccessor.cxx
 ** Rapid easy access to contents of a Scintilla.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <stdio.h>

#include "Platform.h"

#include "PropSet.h"
#include "Accessor.h"
#include "WindowAccessor.h"
#include "Scintilla.h"

WindowAccessor::~WindowAccessor() {
}

bool WindowAccessor::InternalIsLeadByte(char ch) {
	if (SC_CP_UTF8 == codePage)
		// For lexing, all characters >= 0x80 are treated the
		// same so none is considered a lead byte.
		return false;	
	else
		return Platform::IsDBCSLeadByte(codePage, ch);
}

void WindowAccessor::Fill(int position) {
	if (lenDoc == -1)
		lenDoc = Platform::SendScintilla(id, SCI_GETTEXTLENGTH, 0, 0);
	startPos = position - slopSize;
	if (startPos + bufferSize > lenDoc)
		startPos = lenDoc - bufferSize;
	if (startPos < 0)
		startPos = 0;
	endPos = startPos + bufferSize;
	if (endPos > lenDoc)
		endPos = lenDoc;

	TextRange tr = {{startPos, endPos}, buf};
	Platform::SendScintilla(id, SCI_GETTEXTRANGE, 0, reinterpret_cast<long>(&tr));
}

char WindowAccessor::StyleAt(int position) {
	return static_cast<char>(Platform::SendScintilla(
		id, SCI_GETSTYLEAT, position, 0));
}

int WindowAccessor::GetLine(int position) {
	return Platform::SendScintilla(id, SCI_LINEFROMPOSITION, position, 0);
}

int WindowAccessor::LineStart(int line) {
	return Platform::SendScintilla(id, SCI_POSITIONFROMLINE, line, 0);
}

int WindowAccessor::LevelAt(int line) {
	return Platform::SendScintilla(id, SCI_GETFOLDLEVEL, line, 0);
}

int WindowAccessor::Length() { 
	if (lenDoc == -1) 
		lenDoc = Platform::SendScintilla(id, SCI_GETTEXTLENGTH, 0, 0);
	return lenDoc; 
}

int WindowAccessor::GetLineState(int line) {
	return Platform::SendScintilla(id, SCI_GETLINESTATE, line);
}

int WindowAccessor::SetLineState(int line, int state) {
	return Platform::SendScintilla(id, SCI_SETLINESTATE, line, state);
}

void WindowAccessor::StartAt(unsigned int start, char chMask) {
	Platform::SendScintilla(id, SCI_STARTSTYLING, start, chMask);
}

void WindowAccessor::StartSegment(unsigned int pos) {
	startSeg = pos;
}

void WindowAccessor::ColourTo(unsigned int pos, int chAttr) {
	// Only perform styling if non empty range
	if (pos != startSeg - 1) {
		if (pos < startSeg) {
			Platform::DebugPrintf("Bad colour positions %d - %d\n", startSeg, pos);
		}

		if (validLen + (pos - startSeg + 1) >= bufferSize)
			Flush();
		if (validLen + (pos - startSeg + 1) >= bufferSize) {
			// Too big for buffer so send directly
			Platform::SendScintilla(id, SCI_SETSTYLING, pos - startSeg + 1, chAttr);
		} else {
			if (chAttr != chWhile)
				chFlags = 0;
			chAttr |= chFlags;
			for (unsigned int i = startSeg; i <= pos; i++) {
				styleBuf[validLen++] = static_cast<char>(chAttr);
			}
		}
	}
	startSeg = pos+1;
}

void WindowAccessor::SetLevel(int line, int level) {
	Platform::SendScintilla(id, SCI_SETFOLDLEVEL, line, level);
}

void WindowAccessor::Flush() {
	startPos = extremePosition;
	lenDoc = -1;
	if (validLen > 0) {
		Platform::SendScintilla(id, SCI_SETSTYLINGEX, validLen, 
			reinterpret_cast<long>(styleBuf));
		validLen = 0;
	}
}

int WindowAccessor::IndentAmount(int line, int *flags, PFNIsCommentLeader pfnIsCommentLeader) {
	int end = Length();
	int spaceFlags = 0;
	
	// Determines the indentation level of the current line and also checks for consistent 
	// indentation compared to the previous line.
	// Indentation is judged consistent when the indentation whitespace of each line lines 
	// the same or the indentation of one line is a prefix of the other.
	
	int pos = LineStart(line);
	char ch = (*this)[pos];
	int indent = 0;
	bool inPrevPrefix = line > 0;
	int posPrev = inPrevPrefix ? LineStart(line-1) : 0;
	while ((ch == ' ' || ch == '\t') && (pos < end)) {
		if (inPrevPrefix) {
			char chPrev = (*this)[posPrev++];
			if (chPrev == ' ' || chPrev == '\t') {
				if (chPrev != ch)
					spaceFlags |= wsInconsistent;
			} else {
				inPrevPrefix = false;
			}
		}
		if (ch == ' ') {
			spaceFlags |= wsSpace;
			indent++;
		} else {	// Tab
			spaceFlags |= wsTab;
			if (spaceFlags & wsSpace)
				spaceFlags |= wsSpaceTab;
			indent = (indent / 8 + 1) * 8;
		}
		ch = (*this)[++pos];
	}
	
	*flags = spaceFlags;
	indent += SC_FOLDLEVELBASE;
	// if completely empty line or the start of a comment...
	if (isspace(ch) || (pfnIsCommentLeader && (*pfnIsCommentLeader)(*this, pos, end-pos)) )
		return indent | SC_FOLDLEVELWHITEFLAG;
	else
		return indent;
}

