// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdmisc.h"

#include "nel/misc/time_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/thread.h"

#include <SDL_timer.h>
#include <SDL_atomic.h>

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

void CTime::probeTimerInfo(CTime::CTimerInfo &result)
{
	result.HighPrecisionResolution = SDL_GetPerformanceFrequency();
	result.IsHighPrecisionAvailable = (result.HighPrecisionResolution > 1000);

	if (result.HighPrecisionResolution == 14318180)
	{
		nldebug("Detected known HPET era timer frequency");
	}
	if (result.HighPrecisionResolution == 3579545)
	{
		nldebug("Detected known AHCI era timer frequency");
	}
	if (result.HighPrecisionResolution == 1193182)
	{
		nldebug("Detected known i8253/i8254 era timer frequency");
	}
}

/* Return the number of second since midnight (00:00:00), January 1, 1970,
 * coordinated universal time, according to the system clock.
 * This values is the same on all computer if computers are synchronized (with NTP for example).
 */
uint32 CTime::getSecondsSince1970()
{
	return uint32(time(NULL));
}

/** Return the number of second since midnight (00:00:00), January 1, 1970,
 * coordinated universal time, according to the system clock.
 * The time returned is UTC (aka GMT+0), ie it does not have the local time ajustement
 * nor it have the daylight saving ajustement.
 * This values is the same on all computer if computers are synchronized (with NTP for example).
 */
//uint32	CTime::getSecondsSince1970UTC ()
//{
//	// get the local time
//	time_t nowLocal = time(NULL);
//	// convert it to GMT time (UTC)
//	struct tm * timeinfo;
//	timeinfo = gmtime(&nowLocal);
//	return nl_mktime(timeinfo);
//}

static uint32 s_LastTicks = 0;
static sint64 s_LocalTime = 0;
SDL_SpinLock s_TimeLock = 0;

/* Return the local time in milliseconds.
 * Use it only to measure time difference, the absolute value does not mean anything.
 * The value is different on 2 different computers; use the CUniTime class to get a universal
 * time that is the same on all computers.
 */
TTime CTime::getLocalTime()
{
	// NOTE: This function is managed to wrap at the TTime boundary
	sint64 localTime;
	if (SDL_AtomicTryLock(&s_TimeLock))
	{
		// Only one thread can check the clock at once. Please cache results of getLocalTime where possible
		uint32 ticks = SDL_GetTicks();
		uint32 delta = ticks - s_LastTicks;
		localTime = s_LocalTime;
		if (delta)
		{
			s_LastTicks = ticks;
			if (delta < (15 * 60 * 1000)) // Time difference since last call must be less than 15 minutes
			{
				localTime += (sint64)delta;
				s_LocalTime = localTime;
			}
		}
		SDL_AtomicUnlock(&s_TimeLock);
	}
	else
	{
		// If another thread is checking the clock simultaneously, return it's result
		SDL_AtomicLock(&s_TimeLock);
		localTime = s_LocalTime;
		SDL_AtomicUnlock(&s_TimeLock);
	}
	return localTime;
}

/* Return the time in processor ticks. Use it for profile purpose.
 * If the performance time is not supported on this hardware, it returns getLocalTime().
 */
TTicks CTime::getPerformanceTime()
{
	return SDL_GetPerformanceCounter();
}

/* Convert a ticks count into second. If the performance time is not supported on this
 * hardware, it returns 0.0.
 */
double CTime::ticksToSecond(TTicks ticks)
{
	return (double)(sint64)ticks / (double)(sint64)SDL_GetPerformanceFrequency();
}


std::string	CTime::getHumanRelativeTime(sint32 nbSeconds)
{
	sint32 delta = nbSeconds;
	if (delta < 0)
		delta = -delta;

	// some constants of time duration in seconds
	const sint32 oneMinute = 60;
	const sint32 oneHour = oneMinute * 60;
	const sint32 oneDay = oneHour * 24;
	const sint32 oneWeek = oneDay * 7;
	const sint32 oneMonth = oneDay * 30; // aprox, a more precise value is 30.416666... but no matter
	const sint32 oneYear = oneDay * 365; // aprox, a more precise value is 365.26.. who care?

	sint32 year, month, week, day, hour, minute;
	year = month = week = day = hour = minute = 0;

	/// compute the different parts
	year = delta / oneYear;
	delta %= oneYear;

	month = delta / oneMonth;
	delta %= oneMonth;

	week = delta / oneWeek;
	delta %= oneWeek;

	day = delta / oneDay;
	delta %= oneDay;

	hour = delta / oneHour;
	delta %= oneHour;

	minute = delta / oneMinute;
	delta %= oneMinute;

	// compute the string
	CSString ret;

	if (year)
		ret << year << " years ";
	if (month)
		ret << month << " months ";
	if (week)
		ret << week << " weeks ";
	if (day)
		ret << day << " days ";
	if (hour)
		ret << hour << " hours ";
	if (minute)
		ret << minute << " minutes ";
	if (delta || ret.empty())
		ret << delta << " seconds ";

	return ret;
}

#ifdef NL_OS_WINDOWS
	/** Return the offset in 10th of micro sec between the windows base time (
	 *	01-01-1601 0:0:0 UTC) and the unix base time (01-01-1970 0:0:0 UTC).
	 *	This value is used to convert windows system and file time back and
	 *	forth to unix time (aka epoch)
	 */
	uint64 CTime::getWindowsToUnixBaseTimeOffset()
	{
		static bool init = false;

		static uint64 offset = 0;

		if (! init)
		{
			// compute the offset to convert windows base time into unix time (aka epoch)
			// build a WIN32 system time for jan 1, 1970
			SYSTEMTIME baseTime;
			baseTime.wYear = 1970;
			baseTime.wMonth = 1;
			baseTime.wDayOfWeek = 0;
			baseTime.wDay = 1;
			baseTime.wHour = 0;
			baseTime.wMinute = 0;
			baseTime.wSecond = 0;
			baseTime.wMilliseconds = 0;

			FILETIME baseFileTime = {0,0};
			// convert it into a FILETIME value
			SystemTimeToFileTime(&baseTime, &baseFileTime);
			offset = baseFileTime.dwLowDateTime | (uint64(baseFileTime.dwHighDateTime)<<32);

			init = true;
		}

		return offset;
	}
#endif


} // NLMISC
