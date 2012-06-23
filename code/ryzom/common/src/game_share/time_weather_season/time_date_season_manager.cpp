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
#include "time_date_season_manager.h"
#include "static_light_cycle.h"

#include "../ryzom_entity_id.h"
#include "../tick_event_handler.h"

#include "nel/net/service.h"
#include "nel/misc/variable.h"

using namespace NLNET;
using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;

CRyzomTime CTimeDateSeasonManager::_RyzomTime;
CRyzomTime::ETimeOfDay CTimeDateSeasonManager::_DayCycle;
std::map< NLMISC::CSheetId, CStaticLightCycle > CTimeDateSeasonManager::_StaticLightCyclesHours;


//-----------------------------------------------
// init RyzomTime / date and weather properties by continent
//-----------------------------------------------
void CTimeDateSeasonManager::init( uint32 /* startDay */, float /* startTime */)
{
	// load light cycle sheet
	string lightCycleFile = IService::getInstance()->WriteFilesDirectory;
	lightCycleFile = lightCycleFile + string("light_cycles.packed_sheets");
	loadForm( "light_cycle", lightCycleFile, _StaticLightCyclesHours );
}


//-----------------------------------------------
// tick update => update ryzom time
//-----------------------------------------------
void CTimeDateSeasonManager::tickUpdate()
{
	H_AUTO(TimeDateSeasonManager);

	_RyzomTime.updateRyzomClock( CTickEventHandler::getGameCycle() );
	setDayCycle( _RyzomTime.getRyzomTime(), _RyzomTime.getRyzomSeason() );
}

//-----------------------------------------------
// Set day cycle depending hour and season
//-----------------------------------------------
void CTimeDateSeasonManager::setDayCycle( float RyzomTime, CRyzomTime::ESeason Season )
{
	map< CSheetId, CStaticLightCycle >::const_iterator itForm = _StaticLightCyclesHours.find( CSheetId("ryzom.light_cycle") );
	if( itForm == _StaticLightCyclesHours.end() )
	{
		nlwarning( "<CTimeDateSeasonManager setDayCycle> The static form for sheet ryzom.light_cycle  is unknown" );
		return;
	}
	const CStaticLightCycle & lc = (*itForm).second;
	if( isInInterval( lc.LightCycles[ Season ].NightToDayHour, lc.LightCycles[ Season ].DayHour, RyzomTime ) )
	{
		_DayCycle = CRyzomTime::dawn;
	}
	else if( isInInterval( lc.LightCycles[ Season ].DayHour, lc.LightCycles[ Season ].DayToDuskHour, RyzomTime ) )
	{
		_DayCycle = CRyzomTime::day;
	}
	else if( isInInterval( lc.LightCycles[ Season ].DayToDuskHour, lc.LightCycles[ Season ].DuskToNightHour, RyzomTime ) )
	{
		_DayCycle = CRyzomTime::evening;
	}
	else if( isInInterval( lc.LightCycles[ Season ].DuskToNightHour, lc.LightCycles[ Season ].NightHour, RyzomTime ) )
	{
		_DayCycle = CRyzomTime::nightfall;
	}
	else if( isInInterval( lc.LightCycles[ Season ].NightHour, lc.LightCycles[ Season ].NightToDayHour, RyzomTime ) )
	{
		_DayCycle = CRyzomTime::night;
	}
}



