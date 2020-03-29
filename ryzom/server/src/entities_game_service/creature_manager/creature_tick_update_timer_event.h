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

#ifndef CREATURE_TICK_UPDATE_TIMER_EVENT_H
#define CREATURE_TICK_UPDATE_TIMER_EVENT_H

//
// Includes
//

#include "game_share/timer.h"

class CCreature;

class CCreatureTickUpdateTimerEvent:public CTimerEvent
{
	NL_INSTANCE_COUNTER_DECL(CCreatureTickUpdateTimerEvent);
public:

	CCreatureTickUpdateTimerEvent(CCreature *parent);
	void timerCallback(CTimer *owner);
private:
	CCreature *_Parent;
};

#endif
