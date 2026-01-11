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
/** @file WindowAccessor.h
 ** Implementation of BufferAccess and StylingAccess on a Scintilla
 ** rapid easy access to contents of a Scintilla.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

/**
 */
class WindowAccessor : public Accessor {
	// Private so WindowAccessor objects can not be copied
	WindowAccessor(const WindowAccessor &source) : Accessor(), props(source.props) {}
	WindowAccessor &operator=(const WindowAccessor &) { return *this; }
protected:
	WindowID id;
	PropSet &props;
	int lenDoc;

	char styleBuf[bufferSize];
	int validLen;
	char chFlags;
	char chWhile;
	unsigned int startSeg;

	bool InternalIsLeadByte(char ch);
	void Fill(int position);
public:
	WindowAccessor(WindowID id_, PropSet &props_) : 
		Accessor(), id(id_), props(props_), 
		lenDoc(-1), validLen(0), chFlags(0), chWhile(0) {
	}
	~WindowAccessor();
	char StyleAt(int position);
	int GetLine(int position);
	int LineStart(int line);
	int LevelAt(int line);
	int Length();
	void Flush();
	int GetLineState(int line);
	int SetLineState(int line, int state);
	int GetPropertyInt(const char *key, int defaultValue=0) { 
		return props.GetInt(key, defaultValue); 
	}
	char *GetProperties() {
		return props.ToString();
	}

	void StartAt(unsigned int start, char chMask=31);
	void SetFlags(char chFlags_, char chWhile_) {chFlags = chFlags_; chWhile = chWhile_; };
	unsigned int GetStartSegment() { return startSeg; }
	void StartSegment(unsigned int pos);
	void ColourTo(unsigned int pos, int chAttr);
	void SetLevel(int line, int level);
	int IndentAmount(int line, int *flags, PFNIsCommentLeader pfnIsCommentLeader = 0);
};
