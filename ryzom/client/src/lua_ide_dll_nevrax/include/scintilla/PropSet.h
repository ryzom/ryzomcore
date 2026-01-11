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
/** @file PropSet.h
 ** A Java style properties file module.
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef PROPSET_H
#define PROPSET_H
#include "SString.h"

bool EqualCaseInsensitive(const char *a, const char *b);

bool isprefix(const char *target, const char *prefix);

struct Property {
	unsigned int hash;
	char *key;
	char *val;
	Property *next;
	Property() : hash(0), key(0), val(0), next(0) {}
};

/**
 */
class PropSet {
private:
	enum { hashRoots=31 };
	Property *props[hashRoots];
	Property *enumnext;
	int enumhash;
public:
	PropSet *superPS;
	PropSet();
	~PropSet();
	void Set(const char *key, const char *val, int lenKey=-1, int lenVal=-1);
	void Set(const char *keyVal);
	void SetMultiple(const char *s);
	SString Get(const char *key);
	SString GetExpanded(const char *key);
	SString Expand(const char *withVars);
	int GetInt(const char *key, int defaultValue=0);
	SString GetWild(const char *keybase, const char *filename);
	SString GetNewExpand(const char *keybase, const char *filename="");
	void Clear();
	char *ToString();	// Caller must delete[] the return value
	bool GetFirst(char **key, char **val);
	bool GetNext(char **key, char **val);
};

/**
 */
class WordList {
public:
	// Each word contains at least one character - a empty word acts as sentinel at the end.
	char **words;
	char **wordsNoCase;
	char *list;
	int len;
	bool onlyLineEnds;	///< Delimited by any white space or only line ends
	bool sorted;
	int starts[256];
	WordList(bool onlyLineEnds_ = false) : 
		words(0), wordsNoCase(0), list(0), len(0), onlyLineEnds(onlyLineEnds_), sorted(false) {}
	~WordList() { Clear(); }
	operator bool() { return len ? true : false; }
	char *operator[](int ind) { return words[ind]; }
	void Clear();
	void Set(const char *s);
	char *Allocate(int size);
	void SetFromAllocated();
	bool InList(const char *s);
	const char *GetNearestWord(const char *wordStart, int searchLen = -1, 
		bool ignoreCase = false, SString wordCharacters="");
	char *GetNearestWords(const char *wordStart, int searchLen=-1, 
		bool ignoreCase=false, char otherSeparator='\0');
};

inline bool nonFuncChar(char ch) {
	return strchr("\t\n\r !\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~", ch) != NULL;
}

inline bool IsAlphabetic(unsigned int ch) {
	return ((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z'));
}

#endif
