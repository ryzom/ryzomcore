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



#ifndef RY_TIME_AND_SEASON_H
#define RY_TIME_AND_SEASON_H

#include "nel/misc/types_nl.h"

const uint		RYZOM_HOURS_IN_TICKS	= 1800;
const uint		RYZOM_DAY_IN_HOUR		= 24;
const uint		RYZOM_DAY_IN_TICKS		= RYZOM_HOURS_IN_TICKS * RYZOM_DAY_IN_HOUR;
const uint		RYZOM_SEASON_IN_DAY		= 90;
const uint		RYZOM_MONTH_IN_DAY		= 30;
const uint		RYZOM_YEAR_IN_MONTH		= 48;
const uint		RYZOM_WEEK_IN_DAY		= 6;
const uint		RYZOM_YEAR_IN_DAY		= RYZOM_MONTH_IN_DAY*RYZOM_YEAR_IN_MONTH;
const uint		RYZOM_CYCLE_IN_MONTH	= 12;

const uint		RYZOM_START_YEAR		= 2570-2;
const uint		RYZOM_START_SPRING		= 61;
const uint		RYZOM_START_DAY			= 1111111;
const uint		RYZOM_START_HOUR		= 9;

#include "../season.h"
#include "static_light_cycle.h"



namespace MONTH
{
	// Month
	enum EMonth
	{
		Winderly = 0,
		Germinally,
		Folially,
		Floris,
		Medis,
		Thermis,
		Harvestor,
		Frutor,
		Fallenor,
		Pluvia,
		Mystia,
		Nivia,

		UNKNOWN,
		NUM_MONTH = UNKNOWN
	};


	/**
	 * get the right month type from the input string
	 * \param str the input string
	 * \return the EMonth associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	EMonth toMonth(const std::string &str);

	const std::string &toString(EMonth e);

}; // MONTH



namespace WEEKDAY
{
	// Week day
	enum EWeekDay
	{
		Prima = 0,
		Dua,
		Tria,
		Quarta,
		Quinteth,
		Holeth,

		UNKNOWN,
		NUM_WEEKDAY = UNKNOWN
	};


	/**
	 * get the right day type from the input string
	 * \param str the input string
	 * \return the EWeekDay associated to this string (UNKNOWN if the string cannot be interpreted)
	 */
	EWeekDay toWeekDay(const std::string &str);

	const std::string &toString(EWeekDay e);

}; // WEEKDAY



/**
 * Ryzom Time, date and season
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CRyzomTime
{
public:

	typedef EGSPD::CSeason::TSeason ESeason; // see game_share/season.h

	enum ETimeOfDay
	{
		dawn = 0, day, evening, nightfall, night
	};

	enum EWeather
	{
		best = 0, good, bad, worst, unknownWeather, nbWeather=unknownWeather
	};


	// default constructor
	CRyzomTime()
	{
		_RyzomDay = 0;
		_RyzomTime = 0;
		_TickOffset = 0;
		_RyzomTime = 0;
	}

	// Update ryzom clock when tick occurs, local time must be given if localUpdateRyzomClock() and getLocalRyzomTime() is used
	void updateRyzomClock( uint32 gameCyle, double localTime = 0 )
	{
		float time = ( gameCyle + _TickOffset ) / float(RYZOM_HOURS_IN_TICKS);
		_RyzomDay = (uint32) ( time / 24.0f ) - RYZOM_START_SPRING;
		_RyzomTime = (float) fmod( time, 24.0f );
		_LocalTime = localTime;
	}


	// get ryzom time (synchronized with server)
	inline float getRyzomTime() const { return _RyzomTime; }

	// get ryzom day
	inline uint32 getRyzomDay() const { return _RyzomDay; }

	// get ryzom Year
	inline uint32 getRyzomYear() const { return _RyzomDay / RYZOM_YEAR_IN_DAY + RYZOM_START_YEAR; }

	// get ryzom week
	inline uint32 getRyzomWeek() const { return (_RyzomDay % RYZOM_YEAR_IN_DAY) / RYZOM_WEEK_IN_DAY; }

	// get Season
	inline ESeason getRyzomSeason() const { return getSeasonByDay(_RyzomDay); }

	// get Season
	static inline ESeason getSeasonByDay(uint32 day) { return (ESeason) ( ( ( day % RYZOM_YEAR_IN_DAY ) / RYZOM_SEASON_IN_DAY ) % (EGSPD::CSeason::Invalid) ); }

	// get ryzom month
	inline uint getRyzomMonth() const { return ( _RyzomDay % RYZOM_YEAR_IN_DAY ) / RYZOM_MONTH_IN_DAY ; }

	// get ryzom month in cycle
	inline MONTH::EMonth getRyzomMonthInCurrentCycle() const { return (MONTH::EMonth) ( getRyzomMonth() % RYZOM_CYCLE_IN_MONTH ); }

	// get ryzom cycle
	inline uint32 getRyzomCycle() const { return getRyzomMonth() / RYZOM_CYCLE_IN_MONTH; }

	// get ryzom day of week
	inline WEEKDAY::EWeekDay getRyzomDayOfWeek() const { return (WEEKDAY::EWeekDay) ( _RyzomDay % RYZOM_WEEK_IN_DAY ); }

	// get ryzom day of month
	inline uint32 getRyzomDayOfMonth() const { return ( _RyzomDay % RYZOM_MONTH_IN_DAY ); }

	// get ryzom day of season
	inline uint32 getRyzomDayOfSeason() const { return ( _RyzomDay % RYZOM_SEASON_IN_DAY ); }

	// get ryzom day of year
	inline uint32 getRyzomDayOfYear() const { return ( _RyzomDay % RYZOM_YEAR_IN_DAY ); }

	// inc tick offet
	inline void increaseTickOffset( uint32 offset ) { _TickOffset += offset; }

	// dec tick offet
	inline void decreaseTickOffset( uint32 offset ) { _TickOffset -= offset; }

	// reset tick offet
	inline void resetTickOffset() { _TickOffset = 0; }


private:
	CStaticLightCycle	_StaticLightCycleSheet;

	uint32				_RyzomDay;
	float				_RyzomTime;
	double				_LocalTime;
	uint32				_TickOffset;
};

extern CRyzomTime RyzomTime;

#endif // RY_TIME_AND_SEASON_H

/* End of time_and_season.h */

