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
/** @file PropSet.cxx
 ** A Java style properties file module.
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

// Maintain a dictionary of properties

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "Platform.h"

#include "PropSet.h"

// The comparison and case changing functions here assume ASCII
// or extended ASCII such as the normal Windows code page.

static inline char MakeUpperCase(char ch) {
	if (ch < 'a' || ch > 'z')
		return ch;
	else
		return static_cast<char>(ch - 'a' + 'A');
}

static inline bool IsLetter(char ch) {
	return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}

int CompareCaseInsensitive(const char *a, const char *b) {
	while (*a && *b) {
		if (*a != *b) {
			char upperA = MakeUpperCase(*a);
			char upperB = MakeUpperCase(*b);
			if (upperA != upperB)
				return upperA - upperB;
		}
		a++;
		b++;
	}
	// Either *a or *b is nul
	return *a - *b;
}

int CompareNCaseInsensitive(const char *a, const char *b, size_t len) {
	while (*a && *b && len) {
		if (*a != *b) {
			char upperA = MakeUpperCase(*a);
			char upperB = MakeUpperCase(*b);
			if (upperA != upperB)
				return upperA - upperB;
		}
		a++;
		b++;
		len--;
	}
	if (len == 0)
		return 0;
	else
		// Either *a or *b is nul
		return *a - *b;
}

bool EqualCaseInsensitive(const char *a, const char *b) {
	return 0 == CompareCaseInsensitive(a, b);
}

inline unsigned int HashString(const char *s, size_t len) {
	unsigned int ret = 0;
	while (len--) {
		ret <<= 4;
		ret ^= *s;
		s++;
	}
	return ret;
}

PropSet::PropSet() {
	superPS = 0;
	for (int root = 0; root < hashRoots; root++)
		props[root] = 0;
}

PropSet::~PropSet() {
	superPS = 0;
	Clear();
}

void PropSet::Set(const char *key, const char *val, int lenKey, int lenVal) {
	if (!*key)	// Empty keys are not supported
		return;
	if (lenKey == -1)
		lenKey = static_cast<int>(strlen(key));
	if (lenVal == -1)
		lenVal = static_cast<int>(strlen(val));
	unsigned int hash = HashString(key, lenKey);
	for (Property *p = props[hash % hashRoots]; p; p = p->next) {
		if ((hash == p->hash) && 
			((strlen(p->key) == static_cast<unsigned int>(lenKey)) && 
				(0 == strncmp(p->key, key, lenKey)))) {
			// Replace current value
			delete [](p->val);
			p->val = StringDup(val, lenVal);
			return ;
		}
	}
	// Not found
	Property *pNew = new Property;
	if (pNew) {
		pNew->hash = hash;
		pNew->key = StringDup(key, lenKey);
		pNew->val = StringDup(val, lenVal);
		pNew->next = props[hash % hashRoots];
		props[hash % hashRoots] = pNew;
	}
}

void PropSet::Set(const char *keyVal) {
	while (isspace(*keyVal))
		keyVal++;
	const char *endVal = keyVal;
	while (*endVal && (*endVal != '\n'))
		endVal++;
	const char *eqAt = strchr(keyVal, '=');
	if (eqAt) {
		Set(keyVal, eqAt + 1, eqAt-keyVal, endVal - eqAt - 1);
	} else if (*keyVal) {	// No '=' so assume '=1'
		Set(keyVal, "1", endVal-keyVal, 1);
	}
}

void PropSet::SetMultiple(const char *s) {
	const char *eol = strchr(s, '\n');
	while (eol) {
		Set(s);
		s = eol + 1;
		eol = strchr(s, '\n');
	}
	Set(s);
}

SString PropSet::Get(const char *key) {
	unsigned int hash = HashString(key, strlen(key));
	for (Property *p = props[hash % hashRoots]; p; p = p->next) {
		if ((hash == p->hash) && (0 == strcmp(p->key, key))) {
			return p->val;
		}
	}
	if (superPS) {
		// Failed here, so try in base property set
		return superPS->Get(key);
	} else {
		return "";
	}
}

static bool IncludesVar(const char *value, const char *key) {
	const char *var = strstr(value, "$(");
	while (var) {
		if (isprefix(var + 2, key) && (var[2 + strlen(key)] == ')')) {
			// Found $(key) which would lead to an infinite loop so exit
			return true;
		}
		var = strstr(var + 2, ")");
		if (var)
			var = strstr(var + 1, "$(");
	}
	return false;
}

SString PropSet::GetExpanded(const char *key) {
	SString val = Get(key);
	if (IncludesVar(val.c_str(), key))
		return val;
	return Expand(val.c_str());
}

SString PropSet::Expand(const char *withVars) {
	char *base = StringDup(withVars);
	char *cpvar = strstr(base, "$(");
	while (cpvar) {
		char *cpendvar = strchr(cpvar, ')');
		if (cpendvar) {
			int lenvar = cpendvar - cpvar - 2;  	// Subtract the $()
			char *var = StringDup(cpvar + 2, lenvar);
			SString val = GetExpanded(var);
			size_t newlenbase = strlen(base) + val.length() - lenvar;
			char *newbase = new char[newlenbase];
			strncpy(newbase, base, cpvar - base);
			strcpy(newbase + (cpvar - base), val.c_str());
			strcpy(newbase + (cpvar - base) + val.length(), cpendvar + 1);
			delete []var;
			delete []base;
			base = newbase;
		}
		cpvar = strstr(base, "$(");
	}
	SString sret = base;
	delete []base;
	return sret;
}

int PropSet::GetInt(const char *key, int defaultValue) {
	SString val = GetExpanded(key);
	if (val.length())
		return val.value();
	return defaultValue;
}

bool isprefix(const char *target, const char *prefix) {
	while (*target && *prefix) {
		if (*target != *prefix)
			return false;
		target++;
		prefix++;
	}
	if (*prefix)
		return false;
	else
		return true;
}

static bool IsSuffixCaseInsensitive(const char *target, const char *suffix) {
	size_t lentarget = strlen(target);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lentarget)
		return false;
	for (int i = lensuffix - 1; i >= 0; i--) {
		if (MakeUpperCase(target[i + lentarget - lensuffix]) !=
		        MakeUpperCase(suffix[i]))
			return false;
	}
	return true;
}

SString PropSet::GetWild(const char *keybase, const char *filename) {
	for (int root = 0; root < hashRoots; root++) {
		for (Property *p = props[root]; p; p = p->next) {
			if (isprefix(p->key, keybase)) {
				char * orgkeyfile = p->key + strlen(keybase);
				char *keyfile = NULL;

				if (strstr(orgkeyfile, "$(") == orgkeyfile) {
					char *cpendvar = strchr(orgkeyfile, ')');
					if (cpendvar) {
						*cpendvar = '\0';
						SString s = GetExpanded(orgkeyfile + 2);
						*cpendvar = ')';
						keyfile = StringDup(s.c_str());
					}
				}
				char *keyptr = keyfile;

				if (keyfile == NULL)
					keyfile = orgkeyfile;

				for (; ; ) {
					char *del = strchr(keyfile, ';');
					if (del == NULL)
						del = keyfile + strlen(keyfile);
					char delchr = *del;
					*del = '\0';
					if (*keyfile == '*') {
						if (IsSuffixCaseInsensitive(filename, keyfile + 1)) {
							*del = delchr;
							delete []keyptr;
							return p->val;
						}
					} else if (0 == strcmp(keyfile, filename)) {
						*del = delchr;
						delete []keyptr;
						return p->val;
					}
					if (delchr == '\0')
						break;
					*del = delchr;
					keyfile = del + 1;
				}
				delete []keyptr;

				if (0 == strcmp(p->key, keybase)) {
					return p->val;
				}
			}
		}
	}
	if (superPS) {
		// Failed here, so try in base property set
		return superPS->GetWild(keybase, filename);
	} else {
		return "";
	}
}

// GetNewExpand does not use Expand as it has to use GetWild with the filename for each
// variable reference found.
SString PropSet::GetNewExpand(const char *keybase, const char *filename) {
	char *base = StringDup(GetWild(keybase, filename).c_str());
	char *cpvar = strstr(base, "$(");
	while (cpvar) {
		char *cpendvar = strchr(cpvar, ')');
		if (cpendvar) {
			int lenvar = cpendvar - cpvar - 2;  	// Subtract the $()
			char *var = StringDup(cpvar + 2, lenvar);
			SString val = GetWild(var, filename);
			size_t newlenbase = strlen(base) + val.length() - lenvar;
			char *newbase = new char[newlenbase];
			strncpy(newbase, base, cpvar - base);
			strcpy(newbase + (cpvar - base), val.c_str());
			strcpy(newbase + (cpvar - base) + val.length(), cpendvar + 1);
			delete []var;
			delete []base;
			base = newbase;
		}
		cpvar = strstr(base, "$(");
	}
	SString sret = base;
	delete []base;
	return sret;
}

void PropSet::Clear() {
	for (int root = 0; root < hashRoots; root++) {
		Property *p = props[root];
		while (p) {
			Property *pNext = p->next;
			p->hash = 0;
			delete p->key;
			p->key = 0;
			delete p->val;
			p->val = 0;
			delete p;
			p = pNext;
		}
		props[root] = 0;
	}
}

char *PropSet::ToString() {
	size_t len=0;
	for (int r = 0; r < hashRoots; r++) {
		for (Property *p = props[r]; p; p = p->next) {
			len += strlen(p->key) + 1;
			len += strlen(p->val) + 1;
		}
	}
	if (len == 0)
		len = 1;	// Return as empty string
	char *ret = new char [len];
	if (ret) {
		char *w = ret;
		for (int root = 0; root < hashRoots; root++) {
			for (Property *p = props[root]; p; p = p->next) {
				strcpy(w, p->key);
				w += strlen(p->key);
				*w++ = '=';
				strcpy(w, p->val);
				w += strlen(p->val);
				*w++ = '\n';
			}
		}
		ret[len-1] = '\0';
	}
	return ret;
}

/**
 * Initiate enumeration.
 */
bool PropSet::GetFirst(char **key, char **val) {
	for (int i = 0; i < hashRoots; i++) {
		for (Property *p = props[i]; p; p = p->next) {
			if (p) {
				*key = p->key;
				*val = p->val;
				enumnext = p->next; // GetNext will begin here ...
				enumhash = i;		  // ... in this block
				return true;
			}
		}
	}
	return false;
}

/**
 * Continue enumeration.
 */
bool PropSet::GetNext(char ** key, char ** val) {
	bool firstloop = true;

	// search begins where we left it : in enumhash block
	for (int i = enumhash; i < hashRoots; i++) {
		if (!firstloop)
			enumnext = props[i]; // Begin with first property in block
		// else : begin where we left
		firstloop = false;

		for (Property *p = enumnext; p; p = p->next) {
			if (p) {
				*key = p->key;
				*val = p->val;
				enumnext = p->next; // for GetNext
				enumhash = i;
				return true;
			}
		}
	}
	return false;
}

static bool iswordsep(char ch, bool onlyLineEnds) {
	if (!isspace(ch))
		return false;
	if (!onlyLineEnds)
		return true;
	return ch == '\r' || ch == '\n';
}

/**
 * Creates an array that points into each word in the string and puts \0 terminators
 * after each word.
 */
static char **ArrayFromWordList(char *wordlist, int *len, bool onlyLineEnds = false) {
	char prev = '\n';
	int words = 0;
	for (int j = 0; wordlist[j]; j++) {
		if (!iswordsep(wordlist[j], onlyLineEnds) && iswordsep(prev, onlyLineEnds))
			words++;
		prev = wordlist[j];
	}
	char **keywords = new char * [words + 1];
	if (keywords) {
		words = 0;
		prev = '\0';
		size_t slen = strlen(wordlist);
		for (size_t k = 0; k < slen; k++) {
			if (!iswordsep(wordlist[k], onlyLineEnds)) {
				if (!prev) {
					keywords[words] = &wordlist[k];
					words++;
				}
			} else {
				wordlist[k] = '\0';
			}
			prev = wordlist[k];
		}
		keywords[words] = &wordlist[slen];
		*len = words;
	} else {
		*len = 0;
	}
	return keywords;
}

void WordList::Clear() {
	if (words) {
		delete []list;
		delete []words;
		delete []wordsNoCase;
	}
	words = 0;
	wordsNoCase = 0;
	list = 0;
	len = 0;
	sorted = false;
}

void WordList::Set(const char *s) {
	list = StringDup(s);
	sorted = false;
	words = ArrayFromWordList(list, &len, onlyLineEnds);
	wordsNoCase = new char * [len + 1];
	memcpy(wordsNoCase, words, (len + 1) * sizeof (*words));
}

char *WordList::Allocate(int size) {
	list = new char[size + 1];
	list[size] = '\0';
	return list;
}

void WordList::SetFromAllocated() {
	sorted = false;
	words = ArrayFromWordList(list, &len, onlyLineEnds);
	wordsNoCase = new char * [len + 1];
	memcpy(wordsNoCase, words, (len + 1) * sizeof (*words));
}

int cmpString(const void *a1, const void *a2) {
	// Can't work out the correct incantation to use modern casts here
	return strcmp(*(char**)(a1), *(char**)(a2));
}

int cmpStringNoCase(const void *a1, const void *a2) {
	// Can't work out the correct incantation to use modern casts here
	return CompareCaseInsensitive(*(char**)(a1), *(char**)(a2));
}

static void SortWordList(char **words, char **wordsNoCase, unsigned int len) {
	qsort(reinterpret_cast<void*>(words), len, sizeof(*words),
	      cmpString);
	qsort(reinterpret_cast<void*>(wordsNoCase), len, sizeof(*wordsNoCase),
	      cmpStringNoCase);
}

bool WordList::InList(const char *s) {
	if (0 == words)
		return false;
	if (!sorted) {
		sorted = true;
		SortWordList(words, wordsNoCase, len);
		for (unsigned int k = 0; k < (sizeof(starts) / sizeof(starts[0])); k++)
			starts[k] = -1;
		for (int l = len - 1; l >= 0; l--) {
			unsigned char indexChar = words[l][0];
			starts[indexChar] = l;
		}
	}
	unsigned char firstChar = s[0];
	int j = starts[firstChar];
	if (j >= 0) {
		while (words[j][0] == firstChar) {
			if (s[1] == words[j][1]) {
				const char *a = words[j] + 1;
				const char *b = s + 1;
				while (*a && *a == *b) {
					a++;
					b++;
				}
				if (!*a && !*b)
					return true;
			}
			j++;
		}
	}
	j = starts['^'];
	if (j >= 0) {
		while (words[j][0] == '^') {
			const char *a = words[j] + 1;
			const char *b = s;
			while (*a && *a == *b) {
				a++;
				b++;
			}
			if (!*a)
				return true;
			j++;
		}
	}
	return false;
}

/**
 * Returns an element (complete) of the wordlist array which has
 * the same beginning as the passed string.
 * The length of the word to compare is passed too.
 * Letter case can be ignored or preserved (default).
 */
const char *WordList::GetNearestWord(const char *wordStart, int searchLen /*= -1*/, bool ignoreCase /*= false*/, SString wordCharacters /*='/0' */) {
	int start = 0; // lower bound of the api array block to search
	int end = len - 1; // upper bound of the api array block to search
	int pivot; // index of api array element just being compared
	int cond; // comparison result (in the sense of strcmp() result)
	const char *word; // api array element just being compared

	if (0 == words)
		return NULL;
	if (!sorted) {
		sorted = true;
		SortWordList(words, wordsNoCase, len);
	}
	if (ignoreCase) {
		while (start <= end) { // binary searching loop
			pivot = (start + end) >> 1;
			word = wordsNoCase[pivot];
			cond = CompareNCaseInsensitive(wordStart, word, searchLen);
			if (!cond && (!wordCharacters.contains(word[searchLen])))
					return word; // result must not be freed with free()
			else if (cond > 0)
				start = pivot + 1;
			else if (cond <= 0)
				end = pivot - 1;
		}
	} else { // preserve the letter case
		while (start <= end) { // binary searching loop
			pivot = (start + end) >> 1;
			word = words[pivot];
			cond = strncmp(wordStart, word, searchLen);
			if (!cond && (!wordCharacters.contains(word[searchLen])))
				return word; // result must not be freed with free()
			else if (cond > 0)
				start = pivot + 1;
			else if (cond <= 0)
				end = pivot - 1;
		}
	}
	return NULL;
}

/**
 * Find the length of a 'word' which is actually an identifier in a string
 * which looks like "identifier(..." or "identifier:" or "identifier" and where
 * there may be extra spaces after the identifier that should not be
 * counted in the length.
 */
static unsigned int LengthWord(const char *word, char otherSeparator) {
	// Find a '(', or ':'. If that fails go to the end of the string.
	const char *endWord = strchr(word, '(');
	if (!endWord)
		endWord = strchr(word, ':');
	if (!endWord && otherSeparator)
		endWord = strchr(word, otherSeparator);
	if (!endWord)
		endWord = word + strlen(word);
	// Last case always succeeds so endWord != 0

	// Drop any space characters.
	if (endWord > word) {
		endWord--;	// Back from the '(', ':', or '\0'
		// Move backwards over any spaces
		while ((endWord > word) && (isspace(*endWord))) {
			endWord--;
		}
	}
	return endWord - word;
}

/**
 * Returns elements (first words of them) of the wordlist array which have
 * the same beginning as the passed string.
 * The length of the word to compare is passed too.
 * Letter case can be ignored or preserved (default).
 * If there are more words meeting the condition they are returned all of
 * them in the ascending order separated with spaces.
 *
 * NOTE: returned buffer has to be freed with delete[].
 */
char *WordList::GetNearestWords(
    const char *wordStart,
    int searchLen /*= -1*/,
    bool ignoreCase /*= false*/,
    char otherSeparator /*= '\0'*/) {
	int wordlen; // length of the word part (before the '(' brace) of the api array element
	SString wordsNear;
	wordsNear.setsizegrowth(1000);
	int start = 0; // lower bound of the api array block to search
	int end = len - 1; // upper bound of the api array block to search
	int pivot; // index of api array element just being compared
	int cond; // comparison result (in the sense of strcmp() result)

	if (0 == words)
		return NULL;
	if (!sorted) {
		sorted = true;
		SortWordList(words, wordsNoCase, len);
	}
	if (ignoreCase) {
		while (start <= end) { // Binary searching loop
			pivot = (start + end) / 2;
			cond = CompareNCaseInsensitive(wordStart, wordsNoCase[pivot], searchLen);
			if (!cond) {
				// Find first match
				while ((pivot > start) &&
					(0 == CompareNCaseInsensitive(wordStart, 
						wordsNoCase[pivot-1], searchLen))) {
					--pivot;
				}
				// Grab each match
				while ((pivot <= end) &&
					(0 == CompareNCaseInsensitive(wordStart, 
						wordsNoCase[pivot], searchLen))) {
					wordlen = LengthWord(wordsNoCase[pivot], otherSeparator) + 1;
					wordsNear.append(wordsNoCase[pivot], wordlen, ' ');
					++pivot;
				}
				return wordsNear.detach();
			} else if (cond < 0) {
				end = pivot - 1;
			} else if (cond > 0) {
				start = pivot + 1;
			}
		}
	} else {	// Preserve the letter case
		while (start <= end) { // Binary searching loop
			pivot = (start + end) / 2;
			cond = strncmp(wordStart, words[pivot], searchLen);
			if (!cond) {
				// Find first match
				while ((pivot > start) &&
					(0 == strncmp(wordStart, 
						words[pivot-1], searchLen))) { 
					--pivot;
				}
				// Grab each match
				while ((pivot <= end) &&
					(0 == strncmp(wordStart, 
						words[pivot], searchLen))) { 
					wordlen = LengthWord(words[pivot], otherSeparator) + 1;
					wordsNear.append(words[pivot], wordlen, ' ');
					++pivot;
				}
				return wordsNear.detach();
			} else if (cond < 0) {
				end = pivot - 1;
			} else if (cond > 0) {
				start = pivot + 1;
			}
		}
	}
	return NULL;
}
