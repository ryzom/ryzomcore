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

#include "nel/misc/common.h"

#include "game_share/utils.h"

#include "move_checker.h"


/****************************************************************\
					Static Stats Stuff (temp)
\****************************************************************/

class CStatsRecord
{
private:
	std::vector<uint32> _Distances;
	uint32 _Counter;
	uint32 _LastTick;
	bool _Teleporting;
	uint32 _X,_Y;
	uint32 _MaxDist;
	uint32 _MaxDistTime;

public:
	CStatsRecord()
	{
		_Distances.resize(50);
		_Counter=0;
		_LastTick=0;
		_Teleporting=true;
		_X=0;
		_Y=0;
		_MaxDist=0;
		_MaxDistTime=(uint32)_Distances.size()/2;
	}

	void start(TDataSetRow entityIndex, sint32 x, sint32 y, uint32 tick)
	{
		std::fill(_Distances.begin(),_Distances.end(),0);
		_Counter=0;
		_LastTick=tick;
		_Teleporting=true;
		_X=x;
		_Y=y;
		_MaxDist=0;
		_MaxDistTime=(uint32)_Distances.size()/2;
	}

	void add(TDataSetRow entityIndex, sint32 x, sint32 y, uint32 tick)
	{
		// if we've just teleported then there's no point worrying about the time since last update...
		if (_Teleporting)
		{
			// flag teleporting as finished
			_Teleporting= false;
		}
		else
		{
			// check whether entity has been updated recently
//			if((tick-_LastTick)!=1)
//			{
//				nlwarning("Move Checker Stats: entity %s has only received position update after %d ticks",entityIndex.toString().c_str(),tick-_LastTick);
//			}
		}

		// calculate the distance moved and check whether its a new record
		uint32 squaredist= (_X-x)*(_X-x) + (_Y-y)*(_Y-y);
		if (squaredist>_MaxDist)
		{
			_MaxDist=squaredist;
			_MaxDistTime= std::max((uint32)_Counter,(uint32)_Distances.size()/2);
		}
		_Distances[_Counter%_Distances.size()]= squaredist;

		// store away basic info for next time round
		_X=x;
		_Y=y;
		_LastTick=tick;

		// increment our counter...
		++_Counter;

		// if we've hit a new record recently then display a log message
		if ( (_Counter-_MaxDistTime) == (_Distances.size()/2) )
		{
			std::string s;
			for (uint32 i=(uint32)_Distances.size();i!=0;--i)
			{
				s+=NLMISC::toString(" %d",_Distances[(_Counter-i)%_Distances.size()]);
			}
			nlinfo("SpeedStats record for %s: %d (%.2f m/s): %s",entityIndex.toString().c_str(),_MaxDist,sqrt((double)_MaxDist)/200,s.c_str());
		}
	}
};

typedef std::map<TDataSetRow,CStatsRecord> TStats;
TStats Stats;

static void startStats(TDataSetRow entityIndex, sint32 x, sint32 y, uint32 tick)
{
	Stats[entityIndex].start(entityIndex,x,y,tick);
}

static void addStats(TDataSetRow entityIndex, sint32 x, sint32 y, uint32 tick)
{
	Stats[entityIndex].add(entityIndex,x,y,tick);
}

static void endStats(TDataSetRow entityIndex)
{
	Stats.erase(entityIndex);
}


/****************************************************************\
					CMoveChecker Methods 
\****************************************************************/

void CMoveChecker::teleport(TDataSetRow entityIndex, sint32 x, sint32 y, uint32 tick)
{
	// get hold of the entity's record in the move checker (and creat a new entry if need be)
	SPosition& thePosition= _Positions[entityIndex];

	nlinfo("Move checker teleporting player: %s from (%d,%d) @tick: %d to (%d,%d) @tick: %d",
		entityIndex.toString().c_str(),thePosition.X,thePosition.Y,thePosition.Tick,x,y,tick);

	// record the new position
	thePosition.X= x;
	thePosition.Y= y;
	thePosition.Tick= tick;

	// generate a few stats
	startStats(entityIndex,x,y,tick);
}

bool CMoveChecker::checkMove(TDataSetRow entityIndex, sint32& x, sint32& y, uint32 tick)
{
	// setup a refference to the entity's position record, and bomb if it can't be found
	TPositions::iterator positionIt= _Positions.find(entityIndex);
	BOMB_IF(positionIt==_Positions.end(),"Ignoring call to 'checkMove' for an entity who doesn't exist in the move checker",return false);
	SPosition& thePosition= positionIt->second;

	// if the character hasn't moved then just return
	if ( (x==thePosition.X) && (y==thePosition.Y) )
	{
		return true;
	}

//	nlinfo("Checking player move: %s from (%d,%d) @tick: %d to (%d,%d) @tick: %d",
//		entityIndex.toString().c_str(),thePosition.X,thePosition.Y,thePosition.Tick,x,y,tick);

	// if the tick value is out of order (for instance, after an advanced tp request) then ignore the move
	DROP_IF(sint32(tick-thePosition.Tick)<0,"Ignoring out of order move for character "+entityIndex.toString(),return false);

// *** todo: perform a speed test here
// *** todo: perform collision test here 
// *** NOTE: If move not legal then we need to change values of x and y and return false

	// record the new position
	thePosition.X= x;
	thePosition.Y= y;
	thePosition.Tick= tick;

	// generate a few stats
	addStats(entityIndex,x,y,tick);

	return true;
}

void CMoveChecker::remove(TDataSetRow entityIndex)
{
	// remove the entity from our positions map
	_Positions.erase(entityIndex);

	// generate a few stats
	endStats(entityIndex);
}
