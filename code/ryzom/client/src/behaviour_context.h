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



#ifndef CL_BEHAVIOUR_CONTEXT
#define CL_BEHAVIOUR_CONTEXT

#include "game_share/mode_and_behaviour.h"
#include "game_share/entity_types.h"
#include "game_share/magic_fx.h"
#include "game_share/multi_target.h"


// a behaviour and its associatted context
class CBehaviourContext
{
public:
	MBEHAV::CBehaviour Behav;
	double			   BehavTime; // Time in second at which the behaviour is occuring
	// list of multi targets for spells
	CMultiTarget	   Targets;
public:
	// ctor
	CBehaviourContext();

};




#endif
