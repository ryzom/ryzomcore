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



#ifndef RYAI_KEYWORD_OWNER_H
#define RYAI_KEYWORD_OWNER_H

#include "ai_keywords.h"

class	CKeyWordOwner
{
public:
	CKeyWordOwner()
	{
	}
	
	virtual ~CKeyWordOwner()
	{
	}
	// Keyword management ----------------------------------------------
	inline	void keywordsClear()
	{
		_keywords.clear();
	}

	inline	void keywordsAdd(CKeywordMask	mask)	//const std::string &keyword)
	{
		_keywords|=mask;
	}

	inline	const CKeywordMask &getKeywords() const
	{
		return _keywords;
	}
	
protected:
	CKeywordMask	_keywords;	// set of keywords that characterise the owner.
};


#endif
