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

//
// Includes
//

#include "character_tick_update_timer_event.h"
#include "player_manager/character.h"

NL_INSTANCE_COUNTER_IMPL(CCharacterTickUpdateTimerEvent);
NL_INSTANCE_COUNTER_IMPL(CCharacterDbUpdateTimerEvent);
NL_INSTANCE_COUNTER_IMPL(CCharacterBarUpdateTimerEvent);

// ***************************************************************************
CCharacterTickUpdateTimerEvent:: CCharacterTickUpdateTimerEvent(CCharacter *parent)
{
	_Parent = parent;
}

void CCharacterTickUpdateTimerEvent::timerCallback(CTimer* owner)
{
	H_AUTO(CCharacterTickUpdateTimerEvent);

	uint32 timeToNextTick= _Parent->tickUpdate();
	owner->setRemaining(timeToNextTick,this);
}

// ***************************************************************************
CCharacterDbUpdateTimerEvent:: CCharacterDbUpdateTimerEvent(CCharacter *parent)
{
	_Parent = parent;
}

void CCharacterDbUpdateTimerEvent::timerCallback(CTimer* owner)
{
	H_AUTO(CharacterDbUpdateTimerEvent);
	
	_Parent->databaseUpdate();
	owner->setRemaining(2,this);
}


// ***************************************************************************
CCharacterBarUpdateTimerEvent::CCharacterBarUpdateTimerEvent(CCharacter *parent)
{
	_Parent= parent;
}

void CCharacterBarUpdateTimerEvent::timerCallback(CTimer *owner)
{
	H_AUTO(CCharacterBarUpdateTimerEvent);

	_Parent->barUpdate();
	owner->setRemaining(2,this);
}

