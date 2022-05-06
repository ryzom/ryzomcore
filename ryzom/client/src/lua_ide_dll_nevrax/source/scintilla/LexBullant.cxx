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

// SciTE - Scintilla based Text Editor
// LexBullant.cxx - lexer for Bullant

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#include "Platform.h"

#include "PropSet.h"
#include "Accessor.h"
#include "KeyWords.h"
#include "Scintilla.h"
#include "SciLexer.h"


static int classifyWordBullant(unsigned int start, unsigned int end, WordList &keywords, Accessor &styler) {
	char s[100];
	for (unsigned int i = 0; i < end - start + 1 && i < 30; i++) {
		s[i] = static_cast<char>(tolower(styler[start + i]));
		s[i + 1] = '\0';
	}
	int lev= 0;
	char chAttr = SCE_C_IDENTIFIER;
	if (isdigit(s[0]) || (s[0] == '.')){
		chAttr = SCE_C_NUMBER;
	}
	else {
		if (keywords.InList(s)) {
			chAttr = SCE_C_WORD;
/*			if (strcmp(s, "end method") == 0 ||
				strcmp(s, "end case") == 0 ||
				strcmp(s, "end class") == 0 ||
				strcmp(s, "end debug") == 0 ||
				strcmp(s, "end test") == 0 ||
				strcmp(s, "end if") == 0 ||
				strcmp(s, "end lock") == 0 ||
				strcmp(s, "end transaction") == 0 ||
				strcmp(s, "end trap") == 0 ||
				strcmp(s, "end until") == 0 ||
				strcmp(s, "end while") == 0)
				lev = -1;*/
			if (strcmp(s, "end") == 0)
				lev = -1;
			else if (strcmp(s, "method") == 0 ||
				strcmp(s, "case") == 0 ||
				strcmp(s, "class") == 0 ||
				strcmp(s, "debug") == 0 ||
				strcmp(s, "test") == 0 ||
				strcmp(s, "if") == 0 ||
				strcmp(s, "lock") == 0 ||
				strcmp(s, "transaction") == 0 ||
				strcmp(s, "trap") == 0 ||
				strcmp(s, "until") == 0 ||
				strcmp(s, "while") == 0)
				lev = 1;
		}
	}
	styler.ColourTo(end, chAttr);
	return lev;
}

static void ColouriseBullantDoc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[],
	Accessor &styler) {
	WordList &keywords = *keywordlists[0];

	styler.StartAt(startPos);

	bool fold = styler.GetPropertyInt("fold") != 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelPrev = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;

	int state = initStyle;
	if (state == SCE_C_STRINGEOL)	// Does not leak onto next line
		state = SCE_C_DEFAULT;
	char chPrev = ' ';
	char chNext = styler[startPos];
	unsigned int lengthDoc = startPos + length;
	int visibleChars = 0;
	// int blockChange = 0;
	styler.StartSegment(startPos);
	int endFoundThisLine = 0;
	for (unsigned int i = startPos; i < lengthDoc; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		if ((ch == '\r' && chNext != '\n') || (ch == '\n')) {
			// Trigger on CR only (Mac style) or either on LF from CR+LF (Dos/Win) or on LF alone (Unix)
			// Avoid triggering two times on Dos/Win
			// End of line
			endFoundThisLine = 0;
			if (state == SCE_C_STRINGEOL) {
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
			}
			if (fold) {
				int lev = levelPrev;
				if (visibleChars == 0)
					lev |= SC_FOLDLEVELWHITEFLAG;
				if ((levelCurrent > levelPrev) && (visibleChars > 0))
					lev |= SC_FOLDLEVELHEADERFLAG;
				styler.SetLevel(lineCurrent, lev);
				lineCurrent++;
				levelPrev = levelCurrent;
			}
			visibleChars = 0;

/*			int indentBlock = GetLineIndentation(lineCurrent);
			if (blockChange==1){
				lineCurrent++;
				int pos=SetLineIndentation(lineCurrent, indentBlock + indentSize);
			} else if (blockChange==-1) {
				indentBlock -= indentSize;
				if (indentBlock < 0)
					indentBlock = 0;
				SetLineIndentation(lineCurrent, indentBlock);
				lineCurrent++;
			}
			blockChange=0;
*/		}
		if (!isspace(ch))
			visibleChars++;

		if (styler.IsLeadByte(ch)) {
			chNext = styler.SafeGetCharAt(i + 2);
			chPrev = ' ';
			i += 1;
			continue;
		}

		if (state == SCE_C_DEFAULT) {
			if (iswordstart(ch)) {
				styler.ColourTo(i-1, state);
					state = SCE_C_IDENTIFIER;
			} else if (ch == '@' && chNext == 'o') {
				if ((styler.SafeGetCharAt(i+2) =='f') && (styler.SafeGetCharAt(i+3) == 'f')) {
					styler.ColourTo(i-1, state);
					state = SCE_C_COMMENT;
				}
			} else if (ch == '#') {
				styler.ColourTo(i-1, state);
				state = SCE_C_COMMENTLINE;
			} else if (ch == '\"') {
				styler.ColourTo(i-1, state);
				state = SCE_C_STRING;
			} else if (ch == '\'') {
				styler.ColourTo(i-1, state);
				state = SCE_C_CHARACTER;
			} else if (isoperator(ch)) {
				styler.ColourTo(i-1, state);
				styler.ColourTo(i, SCE_C_OPERATOR);
			}
		} else if (state == SCE_C_IDENTIFIER) {
			if (!iswordchar(ch)) {
				int levelChange = classifyWordBullant(styler.GetStartSegment(), i - 1, keywords, styler);
				state = SCE_C_DEFAULT;
				chNext = styler.SafeGetCharAt(i + 1);
				if (ch == '#') {
					state = SCE_C_COMMENTLINE;
				} else if (ch == '\"') {
					state = SCE_C_STRING;
				} else if (ch == '\'') {
					state = SCE_C_CHARACTER;
				} else if (isoperator(ch)) {
					styler.ColourTo(i, SCE_C_OPERATOR);
				}
				if (endFoundThisLine == 0)
					levelCurrent+=levelChange;
				if (levelChange == -1)
					endFoundThisLine=1;
			}
		} else if (state == SCE_C_COMMENT) {
			if (ch == '@' && chNext == 'o') {
				if (styler.SafeGetCharAt(i+2) == 'n') {
					styler.ColourTo(i+2, state);
					state = SCE_C_DEFAULT;
					i+=2;
				}
			}
		} else if (state == SCE_C_COMMENTLINE) {
			if (ch == '\r' || ch == '\n') {
				endFoundThisLine = 0;
				styler.ColourTo(i-1, state);
				state = SCE_C_DEFAULT;
			}
		} else if (state == SCE_C_STRING) {
			if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (ch == '\"') {
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
			} else if (chNext == '\r' || chNext == '\n') {
				endFoundThisLine = 0;
				styler.ColourTo(i-1, SCE_C_STRINGEOL);
				state = SCE_C_STRINGEOL;
			}
		} else if (state == SCE_C_CHARACTER) {
			if ((ch == '\r' || ch == '\n') && (chPrev != '\\')) {
				endFoundThisLine = 0;
				styler.ColourTo(i-1, SCE_C_STRINGEOL);
				state = SCE_C_STRINGEOL;
			} else if (ch == '\\') {
				if (chNext == '\"' || chNext == '\'' || chNext == '\\') {
					i++;
					ch = chNext;
					chNext = styler.SafeGetCharAt(i + 1);
				}
			} else if (ch == '\'') {
				styler.ColourTo(i, state);
				state = SCE_C_DEFAULT;
			}
		}
		chPrev = ch;
	}
	styler.ColourTo(lengthDoc - 1, state);

	// Fill in the real level of the next line, keeping the current flags as they will be filled in later
	if (fold) {
		int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
		//styler.SetLevel(lineCurrent, levelCurrent | flagsNext);
		styler.SetLevel(lineCurrent, levelPrev | flagsNext);

	}
}

LexerModule lmBullant(SCLEX_BULLANT, ColouriseBullantDoc, "bullant");
