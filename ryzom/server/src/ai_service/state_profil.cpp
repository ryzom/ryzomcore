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


std::string	CAIStateProfile::getIndexString	()	const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":%u", getChildIndex());
}


void CAIStateProfile::grpKeywordFilterAdd(const std::string &keyword)
{
	CKeywordFilter filter;

	if (!CAIKeywords::groupFilter(keyword, filter))
	{
		nlwarning("There are some keyword error in '%s'", getAliasNode()->fullName().c_str());
	}

	_grpKeywordFilter+=filter;
}


bool CAIStateProfile::testCompatibility(const CGroupNpc &grp) const
{
	const std::string &groupName	=	grp.getName();	
	bool grpOK = _namedGrps.empty() && _grpKeywordFilter.isEmpty();

	// as long as group not flagged OK - test the list of named groups to see if we match
	for (uint i=0;!grpOK && i<_namedGrps.size();++i)
		grpOK = (groupName==_namedGrps[i]);

	// if all else fails try the keyword test 
	return	(	grpOK
			||	(	!_grpKeywordFilter.isEmpty()
				&& _grpKeywordFilter.test(grp.getKeywords())
				)
			);
}
