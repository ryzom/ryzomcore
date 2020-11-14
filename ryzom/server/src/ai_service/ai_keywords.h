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




#ifndef RYAI_KEYWORDS_H
#define RYAI_KEYWORDS_H

#include "keyword.h"

class CAIKeywords
{
public:
	static void init();
	static void updateKeywordsFromCfg();
	static void display();

	static bool botMask(const std::string &word, CKeywordMask &result)		{ return _botKeywords.stringToMask(word, result); }
	static bool groupMask(const std::string &word, CKeywordMask &result)	{ return _groupKeywords.stringToMask(word, result); }
	static bool stateMask(const std::string &word, CKeywordMask &result)	{ return _stateKeywords.stringToMask(word, result); }

	static bool botFilter(const std::string &word, CKeywordFilter &result)	{ return _botKeywords.stringToFilter(word, result); }
	static bool groupFilter(const std::string &word, CKeywordFilter &result)	{ return _groupKeywords.stringToFilter(word, result); }
	static bool stateFilter(const std::string &word, CKeywordFilter &result)	{ return _stateKeywords.stringToFilter(word, result); }

private:
	static CKeywordSet _botKeywords;
	static CKeywordSet _groupKeywords;
	static CKeywordSet _stateKeywords;

private:
	// this is a singleton so prohibit construction
	CAIKeywords();
};	

#endif
