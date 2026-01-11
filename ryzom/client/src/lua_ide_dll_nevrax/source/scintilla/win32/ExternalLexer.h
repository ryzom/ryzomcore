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
/** @file ExternalLexer.h
 ** Support external lexers in DLLs.
 **/
// Copyright 2001 Simon Steele <ss@pnotepad.org>, portions copyright Neil Hodgson.
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef EXTERNALLEXER_H
#define EXTERNALLEXER_H

// External Lexer function definitions...
typedef void (__stdcall *ExtLexerFunction)(unsigned int lexer, unsigned int startPos, int length, int initStyle,
                  char *words[], WindowID window, char *props);
typedef void (__stdcall *ExtFoldFunction)(unsigned int lexer, unsigned int startPos, int length, int initStyle,
                  char *words[], WindowID window, char *props);
typedef void* (__stdcall *GetLexerFunction)(unsigned int Index);
typedef int (__stdcall *GetLexerCountFn)();
typedef void (__stdcall *GetLexerNameFn)(unsigned int Index, char *name, int buflength);

// Sub-class of LexerModule to use an external lexer.
class ExternalLexerModule : protected LexerModule {
protected:
	ExtLexerFunction fneLexer;
	ExtFoldFunction fneFolder;
	int externalLanguage;
	char name[100];
public:
	ExternalLexerModule(int language_, LexerFunction fnLexer_, 
		const char *languageName_=0, LexerFunction fnFolder_=0) : LexerModule(language_, fnLexer_, 0, fnFolder_){
		strncpy(name, languageName_, sizeof(name));
		languageName = name;
	};
	virtual void Lex(unsigned int startPos, int lengthDoc, int initStyle,
					WordList *keywordlists[], Accessor &styler) const;
	virtual void Fold(unsigned int startPos, int lengthDoc, int initStyle,
					WordList *keywordlists[], Accessor &styler) const;
	virtual void SetExternal(ExtLexerFunction fLexer, ExtFoldFunction fFolder, int index);
};

// LexerMinder points to an ExternalLexerModule - so we don't leak them.
class LexerMinder {
public:
	ExternalLexerModule *self;
	LexerMinder *next;
};

// LexerLibrary exists for every External Lexer DLL, contains LexerMinders.
class LexerLibrary {
public:
	LexerLibrary(LPCTSTR ModuleName);
	~LexerLibrary();
	void Release();
	// Variables
	LexerLibrary	*next;
	SString			m_sModuleName;
private:
	HMODULE m_hModule;
	LexerMinder *first;
	LexerMinder *last;
};

// LexerManager manages external lexers, contains LexerLibrarys.
class LexerManager {
	friend class LMMinder;
public:
	LexerManager();
	~LexerManager();
	void Load();
	static LexerManager *GetInstance();
private:
	bool m_bLoaded;
	void EnumerateLexers();
	static int UseCount;
	static LexerLibrary *first;
	static LexerLibrary *last;
	static LexerManager *firstlm;
};

class LMMinder {
public:
	~LMMinder();
};

#endif
