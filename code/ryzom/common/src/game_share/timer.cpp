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

#include "stdpch.h"

#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/variable.h"
#include "timer.h"


NLMISC::CVariable<uint32> NbProcessedEventsInTimerManagerUpdate("egs", "NbProcessedEventInTimerManagerUpdate", "", 0);
NLMISC::CVariable<uint32> NbEventsToProcessInTimerManagerUpdate("egs", "NbEventsToProcessInTimerManagerUpdate", "", 0);

NL_INSTANCE_COUNTER_IMPL(CTimerEvent);

//-------------------------------------------------------------------------------------------------
//	syncTick()
//-------------------------------------------------------------------------------------------------
void CTimerManager::syncTick()
{
	uint32 delta= CTickEventHandler::getGameCycle()-_LastTick;

	// update the time values
	for (NLMISC::TGameCycle i=0;i<256;++i)
	{
		TEventVector& vect= getEventVector(i);
		for (uint32 j=0;j<vect.size();++j)
			vect[j]->_Time+= delta;
	}

	// re-locate the event vectors to line them back up with the time values that they represent
	for (NLMISC::TGameCycle i=0;i<256;++i)
	{
		TEventVector& vect= _EventVectors[i];
		while (!vect.empty() && ((uint8)vect[0]->_Time)!=i)
		{
			uint8 swapPos= (uint8)_EventVectors[i][0]->_Time;
			std::swap(_EventVectors[i],_EventVectors[swapPos]);
		}
	}

	_LastTick= CTickEventHandler::getGameCycle();
}

//-------------------------------------------------------------------------------------------------
//	tickUpdate()
//-------------------------------------------------------------------------------------------------
void CTimerManager::tickUpdate()
{
	H_AUTO(CTimerManagerUpdate);

	// select this game cycle's phrase event vector
	NLMISC::TGameCycle time= CTickEventHandler::getGameCycle();
	TEventVector &vect= getInstance()->getEventVector(time);

	// iterate through the vector processing its events
	uint32 nextFreeSlot=0;
	uint32 size=(uint32)vect.size();
	for (uint32 i=0;i<size;++i)
	{
		NLMISC::CSmartPtr<CTimerEvent> eventPtr=vect[i];

		// if the event is no longer valid then just skip it
		if (eventPtr->getOwner()==NULL)
			continue;

		// if the event isn't valid yet then keep it for later
		// BUG when event time is too high, like 0xffffffff which is used in special cases, so change the test
		//if ((sint32)(eventPtr->getTime()-time)>0)
		if (eventPtr->getTime() > time)
		{
			vect[nextFreeSlot]=eventPtr;
			++nextFreeSlot;
			continue;
		}

		// process the event
		eventPtr->processEvent();
	}
	if (!vect.empty())
	{
		//nlinfo("TimerManagerUpdate: Processed %d of %d events",vect.size()-nextFreeSlot, vect.size());
		NbEventsToProcessInTimerManagerUpdate = (uint32)vect.size();
		NbProcessedEventsInTimerManagerUpdate = NbEventsToProcessInTimerManagerUpdate.get() - nextFreeSlot;
	}
	// resize the vector back down to keep only the events that we haven't dealt with yet
	vect.resize(nextFreeSlot);
}

