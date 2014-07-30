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



#ifndef RY_TIME_DATE_SEASON_MANAGER_H
#define RY_TIME_DATE_SEASON_MANAGER_H

// Game share
#include "time_and_season.h"
#include "static_light_cycle.h"



/**
 * Ryzom Time, date and season manager
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CTimeDateSeasonManager
{
public:
	// init RyzomTime, date, weather
	static void init( uint32 startDay = RYZOM_START_DAY, float startTime = RYZOM_START_HOUR );
	static void packSheets(const std::string &writeDirectory);

	// tick update => update ryzom time
	static void tickUpdate();

	static const CRyzomTime& getRyzomTimeReference() { return _RyzomTime; }
	static CRyzomTime& getRyzomTimeNoConstReference() { return _RyzomTime; }
	static CRyzomTime::ETimeOfDay getDayCycle() { return _DayCycle; }

private:
	static void setDayCycle( float RyzomTime, CRyzomTime::ESeason Season );
	static bool isInInterval(float start, float end, float value) { return  start <= end ? value >= start && value < end : value >= start || value < end; }

	static CRyzomTime _RyzomTime;
	static CRyzomTime::ETimeOfDay _DayCycle;
	static std::map< NLMISC::CSheetId, CStaticLightCycle > _StaticLightCyclesHours;
};

#endif // RY_TIME_DATE_SEASON_MANAGER_H

/* End of time_date_season_manager.h */

