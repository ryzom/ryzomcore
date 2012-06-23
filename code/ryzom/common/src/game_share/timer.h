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

/*
  Intro:
  ------
  This file contains a complete timer system.
  It is robust and can be used widely.
  The user creates classes derived from CTimerEvent to represent their event handlers.
  When the classes' set() or setRemaining() methods are called an entry is added to the event system provoking
  a callback of the user's timerCallback() method at the given time.
  If a CTimer object is destroyed its callback is automatically canceled.
  A single timer may only have a single time set for it. Setting the time more than once will cancel
  the previous time.
  NOTE: The only requirement for the system is that the CTimerManager::tickUpdate() method is called every tick


  Example 1: - a class containing a timer
  ----------
	class CMyClass
	{
	public:
		void startTimer()
		{
			// set the timer to trigger in 50 ticks
			_Timer.setRemaining(50);
		}
		void update()
		{
			if (_Timer.isActive())
			{
				nlinfo("time remaining: ",_Timer.getTimeRemaining());
			}
			else
			{
				// set the timer to trigger in 20 ticks
				_Timer.setRemaining(20);
			}
		}
	private:
		CTimer _Timer;
	};


  Example 2: - a class using a timer and a custom event handler to trigger regular actions
  ----------
	class CMyClass;
	class CMyTimerEvent;

	class CMyTimerEvent: public CTimerEvent
	{
	public:
		CMyTimerEvent(CMyClass* parent);
		void timerCallback(CTimer* owner);
	private:
		CMyClass* _Parent;
	}

	class CMyClass
	{
	public:
		void startTimer()
		{
			// set the timer to trigger in 50 ticks
			_Timer.setRemaining(50,new CMyTimerEvent(this));
		}
		void doSomething(CTimerEvent* event)
		{
			nlinfo("hello world");
			// set the timer to trigger in 20 ticks with the same event handler
			_Timer.setRemaining(20,event);
		}
	private:
		CMyTimer _Timer;
	};

	void CMyTimerEvent::CMyTimerEvent(CMyClass* parent)
	{
		_Parent=parent;
	}
	void CMyTimerEvent::timerCallback(CTimer* owner)
	{
		_Parent->doSomething(this);
	}
*/

#ifndef TIMER_H
#define TIMER_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

// stl
#include <vector>
// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/smart_ptr.h"
// game share
#include "utils.h"
#include "tick_event_handler.h"
#include "singleton_registry.h"


//-------------------------------------------------------------------------------------------------
// forward class declarations
//-------------------------------------------------------------------------------------------------

class CTimer;
class CTimerEvent;
class CTimerManager;


//-------------------------------------------------------------------------------------------------
// class CTimer
//-------------------------------------------------------------------------------------------------
// generic timer class

class CTimer
{
public:
	// ctor and dtor
	CTimer();
	virtual ~CTimer();

	// set the timer for a given target time & clear the custom event object
	void set(NLMISC::TGameCycle time);

	// set the timer for somewhere between (time) and (time + variation)
	// also clear the custom event object
	void set(NLMISC::TGameCycle time,uint32 variation);

	// set the timer for a given target time with given custom event handler object
	void set(NLMISC::TGameCycle time,CTimerEvent* eventObject);

	// set the timer for somewhere between (time) and (time + variation)
	// also set given custom event handler object
	void set(NLMISC::TGameCycle time,CTimerEvent* eventObject,uint32 variation);

	// set the timer for (current time + time) & clear the custom event object
	void setRemaining(NLMISC::TGameCycle time);

	// set the timer for somewhere between (current time + time) and (current time + time + variation)
	// also clear the custom event object
	void setRemaining(NLMISC::TGameCycle time,uint32 variation);

	// set the timer for (current time + time) with given custom event handler object
	void setRemaining(NLMISC::TGameCycle time,CTimerEvent* eventObject);

	// set the timer for somewhere between (current time + time) and (current time + time + variation)
	// also set given custom event handler object
	void setRemaining(NLMISC::TGameCycle time,CTimerEvent* eventObject,uint32 variation);

	// test whether timer is running
	bool isActive() const;

	// reset the timer and clear the custom timer event
	void reset();

	// get the target time for the timer - return 0 if the event is not active
	NLMISC::TGameCycle getTime() const;

	// get the time remaining before the target time for the timer - return 0 if the event is not active
	NLMISC::TGameCycle getTimeRemaining() const;

	// get a pointer to the current custom event object
	CTimerEvent* getEvent();

private:
	// prohibit copy
	CTimer(const CTimer&);

	CTimer& operator=(const CTimer&);

	// a smart pointer to the timer event - the event is reffed by a second smart ptr from the event system
	NLMISC::CSmartPtr<CTimerEvent> _Event;
};


//-------------------------------------------------------------------------------------------------
// class CTimerEvent
//-------------------------------------------------------------------------------------------------
// specialisable timer event class

class CTimerEvent: public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(CTimerEvent);
public:
	// callback to be specialised
	virtual void timerCallback(CTimer* owner) {}

public:
	// ctor
	CTimerEvent();

	// dtor
	virtual ~CTimerEvent();

	// read accessors
	NLMISC::TGameCycle getTime() const;

	// get hold of the object that created the event (or NULL if the object has been deleted)
	CTimer* getOwner() const;

	// return true if the event is active otherwise false
	bool isActive() const;

private:
	// the following interface is reserved for use by the CTimer class
	friend class CTimer;

	// setup the event
	void set(CTimer* owner,NLMISC::TGameCycle time);

	// setup the event, selecting a time between 'time' and 'time'+'variation'
	void set(CTimer* owner,NLMISC::TGameCycle time,uint32 variation);

	// clear the event - mark it for deletion
	void clear();

private:
	// the following interface is reserved for use by the CTimerManager class
	friend class CTimerManager;

	// method used by the event system to call the timer callback and kill off the event
	void processEvent();

private:
	// the correct time for executing the event
	NLMISC::TGameCycle _Time;

	// a pointer to the owner object - is NULL if owner object has been deleted or this event has been invalidated
	CTimer* _Owner;
};


//-------------------------------------------------------------------------------------------------
// class CTimerManager
//-------------------------------------------------------------------------------------------------
// singleton timer manager

class CTimerManager: public IServiceSingleton
{
public:
	// update called each tick in service update
	// updates events and cleans out event vectors
	virtual void tickUpdate();

	// get the singleton instance...
	static CTimerManager* getInstance();

	// callback called when the tick service connects - used to ajust time values of event objects
	void syncTick();

private:
	// this is a singleton so prohibit construction
	CTimerManager();

	// the type of the event vector for a given time hash
	typedef std::vector<NLMISC::CSmartPtr<CTimerEvent> > TEventVector;

	// the event vector is filled directly by CTimerEvent objects
	friend class CTimerEvent;

	// singleton encapsulation of event vector set
	TEventVector& getEventVector(NLMISC::TGameCycle time);

	// data
	NLMISC::TGameCycle _LastTick;
	TEventVector _EventVectors[256];
};


//-------------------------------------------------------------------------------------------------
// class CTimer
//-------------------------------------------------------------------------------------------------

inline CTimer::CTimer()
{
}

inline CTimer::~CTimer()
{
	reset();
}

inline void CTimer::set(NLMISC::TGameCycle time)
{
	set(time,new CTimerEvent);
}

inline void CTimer::set(NLMISC::TGameCycle time,uint32 variation)
{
	set(time,new CTimerEvent,variation);
}

inline void CTimer::set(NLMISC::TGameCycle time,CTimerEvent* eventObject)
{
	if (_Event!=NULL)
		_Event->clear();
	_Event= eventObject;
	eventObject->set(this,time);
}

inline void CTimer::set(NLMISC::TGameCycle time,CTimerEvent* eventObject,uint32 variation)
{
	if (_Event!=NULL)
		_Event->clear();
	_Event= eventObject;
	eventObject->set(this,time,variation);
}

inline void CTimer::setRemaining(NLMISC::TGameCycle time)
{
	set(CTickEventHandler::getGameCycle()+time);
}

inline void CTimer::setRemaining(NLMISC::TGameCycle time,uint32 variation)
{
	set(CTickEventHandler::getGameCycle()+time,variation);
}

inline void CTimer::setRemaining(NLMISC::TGameCycle time,CTimerEvent* eventObject)
{
	set(CTickEventHandler::getGameCycle()+time,eventObject);
}

inline void CTimer::setRemaining(NLMISC::TGameCycle time,CTimerEvent* eventObject,uint32 variation)
{
	set(CTickEventHandler::getGameCycle()+time,eventObject,variation);
}

inline void CTimer::reset()
{
	if (_Event==NULL)
		return;
	_Event->clear();
	_Event=NULL;
}

inline bool CTimer::isActive() const
{
	return (_Event!=NULL) && _Event->isActive();
}

inline NLMISC::TGameCycle CTimer::getTime() const
{
	if (!isActive())
		return 0;
	return _Event->getTime();
}

inline NLMISC::TGameCycle CTimer::getTimeRemaining() const
{
	if (!isActive())
		return 0;
	return getTime()-CTickEventHandler::getGameCycle();
}

inline CTimerEvent* CTimer::getEvent()
{
	return _Event;
}

//-------------------------------------------------------------------------------------------------
// inlines CTimerEvent
//-------------------------------------------------------------------------------------------------

inline CTimerEvent::CTimerEvent()
{
	_Owner	= NULL;
	_Time	= 0;
}

inline CTimerEvent::~CTimerEvent()
{
}

inline void CTimerEvent::set(CTimer* owner,NLMISC::TGameCycle time)
{
	BOMB_IF(owner==NULL,"Impossible to set a timer with a NULL owner",return);
	BOMB_IF(_Owner!=NULL && _Owner!=owner,"Attempt to change owner of an active event",return);
	_Owner	= owner;
	_Time	= time;
	CTimerManager::getInstance()->getEventVector(time).push_back(this);
}

inline void CTimerEvent::set(CTimer* owner,NLMISC::TGameCycle time,uint32 variation)
{
	BOMB_IF(variation==0,"shouldn't call this method with variation value of 0", set(owner,time));
	BOMB_IF(variation>256,"shouldn't call this method with variation value of >256", variation=256);

	BOMB_IF(owner==NULL,"Impossible to set a timer with a NULL owner", return);
	BOMB_IF(_Owner!=NULL && _Owner!=owner,"Attempt to change owner of an active event", return);
	_Owner	= owner;

	CTimerManager* mgr=CTimerManager::getInstance();
	CTimerManager::TEventVector *best=NULL;
	uint32 bestLength=~0u;

	for (uint32 i=0;i<variation;++i)
	{
		CTimerManager::TEventVector& vect= mgr->getEventVector(time+i);
		uint32 length=(uint32)vect.size();
		if (length<=bestLength)
		{
			bestLength= length;
			best=&vect;
			_Time = time + i;
		}
	}
	BOMB_IF(best==NULL,"BUG: This can never happen!",return)
	best->push_back(this);
}

inline NLMISC::TGameCycle CTimerEvent::getTime() const
{
	return _Time;
}

inline CTimer* CTimerEvent::getOwner() const
{
	return _Owner;
}

inline bool CTimerEvent::isActive() const
{
	return _Owner!=NULL;
}

inline void CTimerEvent::clear()
{
	_Owner=NULL;
}


inline void CTimerEvent::processEvent()
{
	CTimer* owner=_Owner;
	BOMB_IF(owner==NULL,"Attempt to process an event that no longer has a valid owner",return)

	// mark the event as expired - the state may be chnaged during the timer callback...
	// NOTE: This operation results in '_Owner' being set to NULL
	_Owner->reset();

	// call the virtual callback
	timerCallback(owner);
}


//-------------------------------------------------------------------------------------------------
// inlines CTimerManager
//-------------------------------------------------------------------------------------------------

inline CTimerManager::CTimerManager()
{
	_LastTick= CTickEventHandler::getGameCycle();
}

inline CTimerManager* CTimerManager::getInstance()
{
	static CTimerManager* instance= NULL;
	if (instance==NULL)
	{
		instance=new CTimerManager;
	}
	return instance;
}

inline CTimerManager::TEventVector& CTimerManager::getEventVector(NLMISC::TGameCycle time)
{
	return _EventVectors[uint8(time&0xff)];
}


//-------------------------------------------------------------------------------------------------
#endif
