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



#ifndef RY_TIMER_H
#define RY_TIMER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"

#include "time_interface.h"


extern NLMISC::CVariable<int> TimerSpeedUp;

//--------------------------------------------------------------------------
// CAITimer class
//--------------------------------------------------------------------------

class CAITimer
{
public:
	CAITimer(uint32 dt = 0);

	// SET TIMER
	// set timer to current time +dt (in ticks)
	void set(uint32 dt);

	// EXTEND TIMER END TIME
	// add dt to timer's current value (in ticks)
	void add(uint32 dt);

	// EXTEND TIMER END TIME
	// sub dt to timer's current value (in ticks)
	void sub(uint32 dt);


	// TEST TIMER
	// return true if the timer expired at or before time
	bool test(uint32 time=CTimeInterface::gameCycle()) const;

	// GET TICK COUNT CAPPED BY TIMER
	// if test() then return (timer strat time + dt) - oldTime (in ticks)
	// else return current time - oldTime (in ticks)
	uint32 ticksSince(uint32 oldTime) const;

	// Returns the time spend since we start the Timer.
	sint32 timeSinceStart() const;
		
	// GET TIME REMAINING
	// if test() then return 0
	// else return time remaining
	sint32 timeRemaining() const; 

	// GET TOTAL TIME
	// return total time
	sint32 totalTime() const;
	
	// TIME REMAINING AS STRING FOR DEBUG
	std::string toString(uint padding=0) const; 

protected:
	uint32 _start;
	uint32 _dt;
};


//--------------------------------------------------------------------------
// CAITimerWithEnable class
//--------------------------------------------------------------------------

class CAITimerExtended: public CAITimer
{
public:
	CAITimerExtended(uint32 dt);
	CAITimerExtended();

	// SET TIMER
	// set timer to current time +dt (in ticks) and set the '_enabled' flag, clear 'suspended' state
	void set(uint32 dt);

	// EXTEND TIMER END TIME
	// add dt to timer's current value (in ticks) and set the '_enabled' flag
	// if suspended add to suspended dt (but don't resume)
	void add(uint32 dt);

	// EXTEND TIMER END TIME
	// remove dt to timer's current value (in ticks) and set the '_enabled' flag
	// if suspended remove to suspended dt (but don't resume)
	void sub(uint32 dt);


	// DISABLE
	// clear the '_enabled' flag
	void disable();

	// SUSPEND
	// if suspended() or not enabled() do nothing and return
	// store the remaining time for use in resume()
	void suspend();

	// RESUME
	// set the timer with the remaining time stored by suspend()
	void resume();

	// TEST TIMER ONCE
	// return true if enabled() and not suspended() set and the timer expired at or before time
	// if test result is true '_enabled' flag is cleared before the routine returns 
	bool testOnce(uint32 time=CTimeInterface::gameCycle());

	// GET TIME REMAINING
	// if test() then return 0
	// else if suspended() return suspended dt 
	// else return time remaining
	sint32 timeRemaining() const; 

	// ENABLED
	// return state of '_enabled' flag
	bool isEnabled() const;

	// SUSPENDED
	// return the state of the '_suspend' flag
	bool isSuspended() const;
		
	// TIME REMAINING AS STRING FOR DEBUG
	std::string toString(uint padding=0) const; 

protected:
	bool	_enabled;
	uint32	_suspendRemainingTime;
};


//--------------------------------------------------------------------------
// CAITimer Inlines
//--------------------------------------------------------------------------

inline CAITimer::CAITimer(uint32 dt /*= 0*/)
{
	set(dt);
}

inline void CAITimer::set(uint32 dt)
{
	_start = CTimeInterface::gameCycle();
	_dt = dt;
}

inline void CAITimer::add(uint32 dt)
{
	_start += _dt;
	_dt = dt;
}

inline void CAITimer::sub(uint32 dt)
{
	if (_start > dt)
	{
		_start -= dt;
		//_dt = dt;
	}
	else
	{
		_start = 0;
	}
	
}



inline bool CAITimer::test(uint32 time) const
{
	return ((sint32)(time - _start-(_dt/TimerSpeedUp))>=0);
}

inline uint32 CAITimer::ticksSince(uint32 oldTime) const
{
	const	uint32	curent = CTimeInterface::gameCycle();
	if (_dt>=(curent-_start))
		return curent-oldTime;
	if ( (sint32)(_start+_dt-oldTime) > 0 )
		return _start+_dt-oldTime;
	return 0;
}

inline sint32 CAITimer::timeRemaining() const 
{
	const	uint32 curent = CTimeInterface::gameCycle();
	const	uint32 elapsed = curent - _start;
	if (elapsed >= (_dt/TimerSpeedUp))
		return 0;
	return (_dt/TimerSpeedUp)-elapsed;
}

inline sint32 CAITimer::timeSinceStart() const 
{
	return	CTimeInterface::gameCycle()-_start;
}

inline sint32 CAITimer::totalTime() const 
{
	return	_dt;
}


inline std::string CAITimer::toString(uint padding) const
{
	std::string s=NLMISC::toString(timeRemaining());
	while (s.size()<padding)
		s=std::string()+' '+s;
	return s;
}

//--------------------------------------------------------------------------
// CAITimerExtended Inlines
//--------------------------------------------------------------------------

inline CAITimerExtended::CAITimerExtended(uint32 dt)
{
	set(dt);
}

inline CAITimerExtended::CAITimerExtended()
{
	disable();
}

inline void CAITimerExtended::set(uint32 dt)
{
	CAITimer::set(dt);
	_enabled=true;
	_suspendRemainingTime=0;

	if (dt == 0)
		disable();
}

inline void CAITimerExtended::add(uint32 dt)
{
	if (isSuspended())
		_suspendRemainingTime+=dt;
	else
	{
		CAITimer::add(dt);
		_enabled=true;
	}
}


inline void CAITimerExtended::sub(uint32 dt)
{
	if (isSuspended())
	{
		if (_suspendRemainingTime > dt)
		{
			_suspendRemainingTime -= dt;
		}
		else
		{
			_suspendRemainingTime=0;
		}
		
	}
	else
	{
		CAITimer::sub(dt);
		_enabled=true;
	}
}




inline void CAITimerExtended::disable()
{
	_enabled=false;
	_suspendRemainingTime=0;
}

inline void CAITimerExtended::suspend()
{
	if (isEnabled() && !isSuspended())
		_suspendRemainingTime=CAITimer::timeRemaining();
}

inline void CAITimerExtended::resume()
{
	if (isEnabled() && isSuspended())
		set(_suspendRemainingTime);
	_suspendRemainingTime=0;
}

inline bool CAITimerExtended::testOnce(uint32 time)
{
	if (!isEnabled() || isSuspended() || !test())
		return false;

	_enabled=false;
	return true;
}

inline sint32 CAITimerExtended::timeRemaining() const
{
	if (isSuspended())		return _suspendRemainingTime;
	else if (isEnabled())	return CAITimer::timeRemaining();
	else				 	return 0;
}

inline bool CAITimerExtended::isEnabled() const
{
	return _enabled;
}

inline bool CAITimerExtended::isSuspended() const
{
	return _suspendRemainingTime!=0;
}


inline std::string CAITimerExtended::toString(uint padding) const
{
	if (!isEnabled())
		return NLMISC::toString("%*s",padding,"DISABLED");
	if (isSuspended())
		return NLMISC::toString("HOLD:%*i",padding-5>0?padding-5:1,_suspendRemainingTime);
	return CAITimer::toString(padding);
}

//--------------------------------------------------------------------------
#endif
