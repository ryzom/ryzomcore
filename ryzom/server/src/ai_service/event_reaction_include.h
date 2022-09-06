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




#ifndef RYAI_EVENT_REACTION_INCLUDE_H
#define RYAI_EVENT_REACTION_INCLUDE_H

/*#include "ai_mgr.h"
#include "ai.h"
#include "event_reaction.h"
#include "ai_grp.h"
#include "ais_actions.h"	//	for CWorkPtr::instance()
#include "continent.h"*/

template <class TState>
bool	CAIEventReaction::testCompatibility(CStateInstance	*const	stateInstance, const	TState	*const	state) const
{
	if	(!stateInstance)
		return false;

	const	uint32	grpAlias=stateInstance->aliasTreeOwner()->getAlias();
	const	uint32	stateAlias=state->getAlias();

	// if group list is empty and filters are clear then accept all groups
	bool grpOK=(_groups.empty() && _groupFilter.isEmpty());
	
	// as long as group not flagged OK - test the list of named groups to see if we match
	for (uint i=0;!grpOK && i<_groups.size();++i)
		grpOK=(grpAlias==_groups[i]);

	// if all else fails try the keyword test 
	if (!grpOK && !(!_groupFilter.isEmpty() && _groupFilter.test(stateInstance->getPersistentStateInstance()->getKeywords())))
		return false;

	// if state list is empty and filters are clear then accept all states
	bool stateOK=(_states.empty() && _stateFilter.isEmpty());
	
	// as long as state not flagged OK - test the list of named states to see if we match
	for (uint j=0;!stateOK && j<_states.size();++j)
		stateOK=(stateAlias==_states[j]);
	
	// if all else fails try the keyword test 
	return (stateOK || (!_stateFilter.isEmpty() && _stateFilter.test(state->getKeywords())));

}

#endif
