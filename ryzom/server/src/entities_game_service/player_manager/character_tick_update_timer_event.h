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

#ifndef CHARACTER_TICK_UPDATE_TIMER_EVENT_H
#define CHARACTER_TICK_UPDATE_TIMER_EVENT_H

//
// Includes
//

#include "game_share/timer.h"

class CCharacter;

// ***************************************************************************
// different low frequency update
class CCharacterTickUpdateTimerEvent : public CTimerEvent
{
	NL_INSTANCE_COUNTER_DECL(CCharacterTickUpdateTimerEvent);
public:

	CCharacterTickUpdateTimerEvent(CCharacter *parent);
	void timerCallback(CTimer *owner);
private:
	CCharacter *_Parent;
};

// ***************************************************************************
// frequent update of the database
class CCharacterDbUpdateTimerEvent:public CTimerEvent
{
	NL_INSTANCE_COUNTER_DECL(CCharacterDbUpdateTimerEvent);
public:
	CCharacterDbUpdateTimerEvent(CCharacter *parent);
	void timerCallback(CTimer *owner);
private:
	CCharacter *_Parent;
};

// ***************************************************************************
// frequent update of the player HP/STA/SAP/FOCUS bars
class CCharacterBarUpdateTimerEvent : public CTimerEvent
{
	NL_INSTANCE_COUNTER_DECL(CCharacterBarUpdateTimerEvent);
public:
	CCharacterBarUpdateTimerEvent(CCharacter *parent);
	void timerCallback(CTimer *owner);
private:
	CCharacter *_Parent;
};

#endif
