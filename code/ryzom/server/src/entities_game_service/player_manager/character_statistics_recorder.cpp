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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "character_statistics_recorder.h"
#include "player_manager/character.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLNET;
using namespace NLMISC;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

// If the following macro is defined then the macros such as PERSISTENT_CLASS, PERSISTENT_DATA, PERSISTENT_PRE_STORE, etc
// are undefined after inclusion of perstent_data_template.h
#define PERSISTENT_MACROS_AUTO_UNDEF


//-----------------------------------------------------------------------------
// methods CCharaterStatisticsRecorderRecord
//-----------------------------------------------------------------------------

CCharaterStatisticsRecorderRecord::CCharaterStatisticsRecorderRecord()
{
	clear();
}

CCharaterStatisticsRecorderRecord::~CCharaterStatisticsRecorderRecord()
{
}

void CCharaterStatisticsRecorderRecord::clear()
{
	_TicksInGame=0;
	_TimeInGame=0;
	_TotalXP=0;
}

void CCharaterStatisticsRecorderRecord::build(const CCharaterStatisticsRecorderRecord& last, CCharacter* character)
{
	uint32 dTicks= CTickEventHandler::getGameCycle() /*-character->getLoadTime()*/;
	uint64 dTime= CTime::getLocalTime() /*-character->getLoadTick()*/;
	if (dTime<100*dTicks)
		dTime=100*dTicks;
	_TicksInGame= last._TicksInGame+ dTicks;
	_TimeInGame= last._TimeInGame+ dTime;
}


//-----------------------------------------------------------------------------
// Persistent data for CCharaterStatisticsRecorderRecord
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CCharaterStatisticsRecorderRecord

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP_GAME_CYCLE_COMP(_TicksInGame)\
	PROP(sint32,_TotalXP)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// methods CCharaterStatisticsRecorderContainer
//-----------------------------------------------------------------------------

CCharaterStatisticsRecorderContainer::CCharaterStatisticsRecorderContainer()
{
	clear();
}

CCharaterStatisticsRecorderContainer::~CCharaterStatisticsRecorderContainer()
{
}

void CCharaterStatisticsRecorderContainer::clear()
{
	_Counter=1; // note that it is important than we start at 1 and not at 0!
	_Sessions1.clear(); _Sessions1.resize(10);
	_Sessions10.clear(); _Sessions10.resize(10);
	_Sessions100.clear(); _Sessions100.resize(10);
}

void CCharaterStatisticsRecorderContainer::add(CCharacter* character)
{
	if (_Counter%10==0)
	{
		if ((_Counter/10)%10==0)
		{
			// add to Sessions100 vector
			_Sessions100[(_Counter/100)%10]= _Sessions10[0];
		}

		// add to Sessions10 vector
		_Sessions10[(_Counter/10)%10]= _Sessions1[0];
	}

	// add to Sessions1 vector
	_Sessions1[_Counter%10].build(_Sessions1[(_Counter+9)%10],character);

	++_Counter;
}


//-----------------------------------------------------------------------------
// Persistent data for CCharaterStatisticsRecorderContainer
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CCharaterStatisticsRecorderContainer

#define PERSISTENT_DATA\
	FLAG0(CLEAR,clear())\
	PROP(sint32,_Counter)\
	STRUCT_INDEXED_VECT(_Sessions1)\
	STRUCT_INDEXED_VECT(_Sessions10)\
	STRUCT_INDEXED_VECT(_Sessions100)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
