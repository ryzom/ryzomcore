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


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stat_char_filter_factory.h"
#include "stat_character.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

//-------------------------------------------------------------------------------------------------
// FILTER Implementations
//-------------------------------------------------------------------------------------------------

FILTER(Guild,"<guildId>[ <guildId>[ ...]]")
{
	CSString idTxt= NLMISC::toString(c->_GuildId);
	CVectorSString args;
	_RawArgs.splitWords(args);
	for (uint32 i=0;i<args.size();++i)
		if (idTxt==args[i])
			return true;
	return false;
}

FILTER(Money,"<min> [<max>]")
{
	CVectorSString args;
	CSString s= _RawArgs;
	CSString s0= s.firstWord(true);
	CSString s1= s.firstWord(true);
	uint32 min= s0.atoi();
	uint32 max= (s1.empty()? ~0u: s1.atoi());
	if ( (min==0 && s0!="0") ||  (max==0 && s1!="0") || !s.strip().empty() )
	{
		nlwarning("Bad arguments in filter: should be <min> [<max>]");
		return false;
	}
	return (c->_Money>= min) && (c->_Money<= max);
}

FILTER(BestSkill,"<min> [<max>]")
{
	// decypher the command line arguments
	CVectorSString args;
	CSString s= _RawArgs;
	CSString s0= s.firstWord(true);
	CSString s1= s.firstWord(true);
	sint32 min= s0.atoi();
	sint32 max= (s1.empty()? ~0u/2: s1.atoi());
	if ( (min==0 && s0!="0") ||  (max==0 && s1!="0") || !s.strip().empty() )
	{
		nlwarning("Bad arguments in filter: should be <min> [<max>]");
		return false;
	}

	// calculate the best skill
	sint32 bestSkill=0;
	typedef std::map<NLMISC::CSString,CStatsScanSkillsEntry> TSkills;
	const TSkills& skills=c->EntityBase._Skills.Skills;
	for(TSkills::const_iterator it=skills.begin();it!=skills.end();++it)
	{
		if (bestSkill<(*it).second.Current)
			bestSkill=(*it).second.Current;
	}

	// perform compareson...
	return (bestSkill>= min) && (bestSkill<= max);
}

