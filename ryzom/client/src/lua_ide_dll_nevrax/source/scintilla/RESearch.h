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
/** @file RESearch.h
 ** Interface to the regular expression search library.
 **/
// Written by Neil Hodgson <neilh@scintilla.org>
// Based on the work of Ozan S. Yigit.
// This file is in the public domain.

#ifndef RESEARCH_H
#define RESEARCH_H

/*
 * The following defines are not meant to be changeable.
 * They are for readability only.
 */
#define MAXCHR	128
#define CHRBIT	8
#define BITBLK	MAXCHR/CHRBIT

class CharacterIndexer {
public: 
	virtual char CharAt(int index)=0;
};

class RESearch {

public:
	RESearch();
	~RESearch();
	void Init();
	void Clear();
	bool GrabMatches(CharacterIndexer &ci);
	void ChSet(char c);
	void ChSetWithCase(char c, bool caseSensitive);
	const char *Compile(const char *pat, int length, bool caseSensitive);
	int Execute(CharacterIndexer &ci, int lp, int endp);
	void ModifyWord(char *s);
	int Substitute(CharacterIndexer &ci, char *src, char *dst);

	enum {MAXTAG=10};
	enum {MAXNFA=2048};
	enum {NOTFOUND=-1};

	int bopat[MAXTAG];
	int eopat[MAXTAG];
	char *pat[MAXTAG];

private:
	int PMatch(CharacterIndexer &ci, int lp, int endp, char *ap);

	int bol;
	int  tagstk[MAXTAG];             /* subpat tag stack..*/
	char nfa[MAXNFA];		/* automaton..       */
	int sta;
	char bittab[BITBLK];		/* bit table for CCL */
						/* pre-set bits...   */
	int failure;
};

#endif
