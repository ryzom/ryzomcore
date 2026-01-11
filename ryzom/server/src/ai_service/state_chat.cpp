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



#include "stdpch.h"
#include "state_profil.h"
#include "ai_bot_npc.h"
#include "ai_grp_npc.h"

std::string	CAIStateChat::getIndexString	()	const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":%u", getChildIndex());
}


void CAIStateChat::botKeywordFilterAdd(const std::string &keyword)
{ 
	CKeywordFilter filter;
	if (!CAIKeywords::botFilter(keyword, filter))
	{
		nlwarning("There are some keyword error in '%s'", getAliasNode()->fullName().c_str());
	}
	_botKeywordFilter+=filter;
} 


bool CAIStateChat::testCompatibility(const CBotNpc &bot) const
{
	const std::string &botName		=	bot.getName();

	uint i;
	bool botOK = _namedBots.empty() && _botKeywordFilter.isEmpty();
	// as long as bot not flagged OK - test the list of named bots to see if we match
//	std::vector<std::string>::iterator	it=find_if(_namedBots.begin(), _namedBots.end(), bind2nd(equal_to,botName));
//	botOK=it!=_namedBots.end();

	for (i=0;!botOK && i<_namedBots.size();++i)
		botOK = (botName==_namedBots[i]);

	// if all else fails try the keyword test 
	return (	botOK
			||	(	!_botKeywordFilter.isEmpty()
				&&	_botKeywordFilter.test(bot.getKeywords())
				)
			);
}
