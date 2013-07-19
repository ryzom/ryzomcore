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

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#elif defined (NL_OS_UNIX)
#	include <sys/time.h>
#	include <unistd.h>
#endif

#ifdef NL_OS_MAC
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

namespace {
#ifdef NL_OS_WINDOWS
bool a_HaveQueryPerformance = false;
LARGE_INTEGER a_QueryPerformanceFrequency;
#endif
#ifdef NL_OS_UNIX
#	if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
#	if defined(_POSIX_MONOTONIC_CLOCK) && (_POSIX_MONOTONIC_CLOCK >= 0)
#		define NL_MONOTONIC_CLOCK
#	endif
#	endif
#	ifdef NL_MONOTONIC_CLOCK
bool a_CheckedMonotonicClock = false;
bool a_HasMonotonicClock = false;
uint64 a_MonotonicClockFrequency = 0;
uint64 a_MonotonicClockResolutionNs = 0;
bool hasMonotonicClock()
{
	if (!a_CheckedMonotonicClock)
	{
		/* Initialize the local time engine.
		* On Unix, this method will find out if the Monotonic Clock is supported
		* (seems supported by kernel 2.6, not by kernel 2.4). See getLocalTime().
		*/
		struct timespec tv;
		if ((clock_gettime( CLOCK_MONOTONIC, &tv ) == 0) &&
			 (clock_getres( CLOCK_MONOTONIC, &tv ) == 0))
		{
//			nldebug( "Monotonic local time supported (resolution %.6f ms)", ((float)tv.tv_sec)*1000.0f + ((float)tv.tv_nsec)/1000000.0f );

			if (tv.tv_sec > 0)
			{
				nlwarning("Monotonic clock not ok, resolution > 1s");
				a_HasMonotonicClock = false;
			}
			else
			{
				uint64 nsPerTick = tv.tv_nsec;
				uint64 nsPerSec = 1000000000L;
				uint64 tickPerSec = nsPerSec / nsPerTick;
				a_MonotonicClockFrequency = tickPerSec;
				a_MonotonicClockResolutionNs = nsPerTick;
				a_HasMonotonicClock = true;
			}
		}
		else
		{
			a_HasMonotonicClock = false;
		}
		a_CheckedMonotonicClock = true;
	}
	return a_HasMonotonicClock;
}
#	endif
#endif
}

void CTime::probeTimerInfo(CTime::CTimerInfo &result)
{
	breakable
	{
#ifdef NL_OS_WINDOWS
		LARGE_INTEGER winPerfFreq;
		LARGE_INTEGER winPerfCount;
		DWORD lowResTime;
		if (!QueryPerformanceFrequency(&winPerfFreq))
		{
			nldebug("Cannot query performance frequency");
			result.IsHighPrecisionAvailable = false;
		}
		else
		{
			result.HighPrecisionResolution = winPerfFreq.QuadPart;
		}
		if (winPerfFreq.QuadPart == 1000)
		{
			nldebug("Higher precision timer not available, OS defaulted to GetTickCount");
			result.IsHighPrecisionAvailable = false;
		}
		if (!QueryPerformanceCounter(&winPerfCount))
		{
			nldebug("Cannot query performance counter");
			result.IsHighPrecisionAvailable = false;
			result.HighPrecisionResolution = 1000;
		}
		a_HaveQueryPerformance = result.IsHighPrecisionAvailable;
		a_QueryPerformanceFrequency.QuadPart = winPerfFreq.QuadPart;
		if (!result.IsHighPrecisionAvailable)
		{
			lowResTime = timeGetTime();
		}
#else

		// Other platforms are awesome. Generic implementation for now.
		TTime localTime = getLocalTime();
		result.IsHighPrecisionAvailable = true;
		result.HighPrecisionResolution = 0;

#	ifdef NL_MONOTONIC_CLOCK
		timespec monoClock;
		if (hasMonotonicClock())
		{
			clock_gettime(CLOCK_MONOTONIC, &monoClock);
			result.HighPrecisionResolution = a_MonotonicClockFrequency;
		}
		else
		{
			nldebug("Monotonic clock not available");
		}
#	endif

#endif

		uint64 cpuMask = IProcess::getCurrentProcess()->getCPUMask();
#ifdef NL_OS_WINDOWS
		uint64 threadMask = IThread::getCurrentThread()->getCPUMask(); // broken on linux, don't expect it to work anywhere
#else
		uint64 threadMask = cpuMask;
#endif

		uint identical = 0; // Identical stamps may indicate the os handling backwards glitches.
		uint backwards = 0; // Happens when the timers are not always in sync and the implementation is faulty.
		uint regular = 0; // How many times the number advanced normally.
		uint skipping = 0; // Does not really mean anything necessarily.
		uint frequencybug = 0; // Should never happen.
		// uint badcore = 0; // Affinity does not work.

		// Cycle 32 times trough all cores, and verify if the timing remains consistent.
		for (uint i = 32; i; --i)
		{
			uint64 currentBit = 1;
			for (uint j = 64; j; --j)
			{
				if (cpuMask & currentBit)
				{
#ifdef NL_OS_WINDOWS
					if (!IThread::getCurrentThread()->setCPUMask(currentBit))
#else
					if (!IProcess::getCurrentProcess()->setCPUMask(currentBit))
#endif
						break; // Thread was set to last cpu.
#ifdef NL_OS_WINDOWS
					// Make sure the thread is rescheduled.
					SwitchToThread();
					Sleep(0);
					// Verify the core
					/* Can only verify on 2003, Vista and higher.
					if (1 << GetCurrentProcessorNumber() != currentBit)
						++badcore;
					*/
					// Check if the timer is still sane.
					if (result.IsHighPrecisionAvailable)
					{
						LARGE_INTEGER winPerfFreqN;
						LARGE_INTEGER winPerfCountN;
						QueryPerformanceFrequency(&winPerfFreqN);
						if (winPerfFreqN.QuadPart != winPerfFreq.QuadPart)
							++frequencybug;
						QueryPerformanceCounter(&winPerfCountN);
						if (winPerfCountN.QuadPart == winPerfCount.QuadPart)
							++identical;
						if (winPerfCountN.QuadPart < winPerfCount.QuadPart || winPerfCountN.QuadPart - winPerfCount.QuadPart < 0)
							++backwards;
						if (winPerfCountN.QuadPart - winPerfCount.QuadPart > winPerfFreq.QuadPart / 20) // 50ms skipping check
							++skipping;
						else if (winPerfCountN.QuadPart > winPerfCount.QuadPart)
							++regular;
						winPerfCount.QuadPart = winPerfCountN.QuadPart;
					}
					else
					{
						DWORD lowResTimeN;
						lowResTimeN = timeGetTime();
						if (lowResTimeN == lowResTime)
							++identical;
						if (lowResTimeN < lowResTime || lowResTimeN - lowResTime < 0)
							++backwards;
						if (lowResTimeN - lowResTime > 50)
							++skipping;
						else if (lowResTimeN > lowResTime)
							++regular;
						lowResTime = lowResTimeN;
					}
#else
#ifdef NL_OS_UNIX
					sched_yield();
#else
					nlSleep(0);
#endif
#	ifdef NL_MONOTONIC_CLOCK
					if (hasMonotonicClock())
					{
						timespec monoClockN;
						clock_gettime(CLOCK_MONOTONIC, &monoClockN);
						if (monoClock.tv_sec == monoClockN.tv_sec && monoClock.tv_nsec == monoClockN.tv_nsec)
							++identical;
						if (monoClockN.tv_sec < monoClock.tv_sec || (monoClock.tv_sec == monoClockN.tv_sec && monoClockN.tv_nsec < monoClock.tv_nsec))
							++backwards;
						if (monoClock.tv_sec == monoClockN.tv_sec && (monoClockN.tv_nsec - monoClock.tv_nsec > 50000000L))
							++skipping;
						else if ((monoClock.tv_sec == monoClockN.tv_sec && monoClock.tv_nsec < monoClockN.tv_nsec) || monoClock.tv_sec < monoClockN.tv_sec)
							++regular;
						monoClock.tv_sec = monoClockN.tv_sec;
						monoClock.tv_nsec = monoClockN.tv_nsec;
					}
					else
#	endif
					{
						TTime localTimeN = getLocalTime();
						if (localTimeN == localTime)
							++identical;
						if (localTimeN < localTime || localTimeN - localTime < 0)
							++backwards;
						if (localTimeN - localTime > 50)
							++skipping;
						else if (localTimeN > localTime)
							++regular;
						localTime = localTimeN;
					}
#endif
				}
				currentBit <<= 1;
			}
		}

#ifdef NL_OS_WINDOWS
		IThread::getCurrentThread()->setCPUMask(threadMask);
#else
		IProcess::getCurrentProcess()->setCPUMask(threadMask);
#endif

		nldebug("Timer resolution: %i Hz", (int)(result.HighPrecisionResolution));
		nldebug("Time identical: %i, backwards: %i, regular: %i, skipping: %i, frequency bug: %i", identical, backwards, regular, skipping, frequencybug);
		if (identical > regular)
			nlwarning("The system timer is of relatively low resolution, you may experience issues");
		if (backwards > 0 || frequencybug > 0)
		{
			nlwarning("The current system timer is not reliable across multiple cpu cores");
			result.RequiresSingleCore = true;
		}
		else result.RequiresSingleCore = false;

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
}

/* Return the number of second since midnight (00:00:00), January 1, 1970,
 * coordinated universal time, according to the system clock.
 * This values is the same on all computer if computers are synchronized (with NTP for example).
 */
uint32 CTime::getSecondsSince1970 ()
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

/* Return the local time in milliseconds.
 * Use it only to measure time difference, the absolute value does not mean anything.
 * On Unix, getLocalTime() will try to use the Monotonic Clock if available, otherwise
 * the value can jump backwards if the system time is changed by a user or a NTP time sync process.
 * The value is different on 2 different computers; use the CUniTime class to get a universal
 * time that is the same on all computers.
 * \warning On Win32, the value is on 32 bits only. It wraps around to 0 every about 49.71 days.
 */
TTime CTime::getLocalTime ()
{

#ifdef NL_OS_WINDOWS

	//static bool initdone = false;
	//static bool byperfcounter;
	// Initialization
	//if ( ! initdone )
	//{
		//byperfcounter = (getPerformanceTime() != 0);
		//initdone = true;
	//}

	/* Retrieve time is ms
     * Why do we prefer getPerformanceTime() to timeGetTime() ? Because on one dual-processor Win2k
	 * PC, we have noticed that timeGetTime() slows down when the client is running !!!
	 */
	/* Now we have noticed that on all WinNT4 PC the getPerformanceTime can give us value that
	 * are less than previous
	 */

	//if ( byperfcounter )
	//{
	//	return (TTime)(ticksToSecond(getPerformanceTime()) * 1000.0f);
	//}
	//else
	//{
		// This is not affected by system time changes. But it cycles every 49 days.
		// return timeGetTime(); // Only this was left active before it was commented.
	//}

	/*
	 * The above is no longer relevant.
	 */

	if (a_HaveQueryPerformance)
	{
		// On a (fast) 15MHz timer this rolls over after 7000 days.
		// If my calculations are right.
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		counter.QuadPart *= (LONGLONG)1000L;
		counter.QuadPart /= a_QueryPerformanceFrequency.QuadPart;
		return counter.QuadPart;
	}
	else
	{
		// Use default reliable low resolution timer.
		return timeGetTime();
	}

#elif defined (NL_OS_UNIX)

#ifdef NL_MONOTONIC_CLOCK

	if (hasMonotonicClock())
	{
		timespec tv;
		// This is not affected by system time changes.
		if ( clock_gettime( CLOCK_MONOTONIC, &tv ) != 0 )
			nlerror ("Can't get clock time again");
	    return (TTime)tv.tv_sec * (TTime)1000 + (TTime)((tv.tv_nsec/*+500*/) / 1000000);
	}

#endif

	// This is affected by system time changes.
	struct timeval tv;
	if ( gettimeofday( &tv, NULL) != 0 )
		nlerror ("Can't get time of day");
	return (TTime)tv.tv_sec * (TTime)1000 + (TTime)tv.tv_usec / (TTime)1000;

#endif
}

/* Return the time in processor ticks. Use it for profile purpose.
 * If the performance time is not supported on this hardware, it returns 0.
 * \warning On a multiprocessor system, the value returned by each processor may
 * be different. The only way to workaround this is to set a processor affinity
 * to the measured thread.
 * \warning The speed of tick increase can vary (especially on laptops or CPUs with
 * power management), so profiling several times and computing the average could be
 * a wise choice.
 */
TTicks CTime::getPerformanceTime ()
{
#ifdef NL_OS_WINDOWS
	LARGE_INTEGER ret;
	if (QueryPerformanceCounter (&ret))
		return ret.QuadPart;
	else
		return 0;
#elif defined(NL_OS_MAC)
	return mach_absolute_time();
#else
#if defined(HAVE_X86_64)
	uint64 hi, lo;
	__asm__ volatile (".byte 0x0f, 0x31" : "=a" (lo), "=d" (hi));
	return (hi << 32) | (lo & 0xffffffff);
#elif defined(HAVE_X86) and !defined(NL_OS_MAC)
	uint64 x;
	// RDTSC - Read time-stamp counter into EDX:EAX.
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
#else // HAVE_X86
	static bool firstWarn = true;
	if (firstWarn)
	{
		nlwarning ("TTicks CTime::getPerformanceTime () is not implemented for your processor, returning 0");
		firstWarn = false;
	}
	return 0;
#endif // HAVE_X86

#endif // NL_OS_WINDOWS
}
/*
#define GETTICKS(t) asm volatile ("push %%esi\n\t" "mov %0, %%esi" : : "r" (t)); \
                      asm volatile ("push %eax\n\t" "push %edx"); \
                      asm volatile ("rdtsc"); \
                      asm volatile ("movl %eax, (%esi)\n\t" "movl %edx, 4(%esi)"); \
                      asm volatile ("pop %edx\n\t" "pop %eax\n\t" "pop %esi");
*/


/* Convert a ticks count into second. If the performance time is not supported on this
 * hardware, it returns 0.0.
 */
double CTime::ticksToSecond (TTicks ticks)
{
#ifdef NL_OS_WINDOWS
	LARGE_INTEGER ret;
	if (QueryPerformanceFrequency(&ret))
	{
		return (double)(sint64)ticks/(double)ret.QuadPart;
	}
	else
#elif defined(NL_OS_MAC)
	{
		static double factor = 0.0;
		if (factor == 0.0)
		{
			mach_timebase_info_data_t tbInfo;
			mach_timebase_info(&tbInfo);
			factor = 1000000000.0 * (double)tbInfo.numer / (double)tbInfo.denom;
		}
		return double(ticks / factor);
	}
#endif // NL_OS_WINDOWS
	{
		static bool benchFrequency = true;
		static sint64 freq = 0;
		if (benchFrequency)
		{
			// try to have an estimation of the cpu frequency

			TTicks tickBefore = getPerformanceTime ();
			TTicks tickAfter = tickBefore;
			TTime timeBefore = getLocalTime ();
			TTime timeAfter = timeBefore;
			for(;;)
			{
				if (timeAfter - timeBefore > 1000)
					break;
				timeAfter = getLocalTime ();
				tickAfter = getPerformanceTime ();
			}

			TTime timeDelta = timeAfter - timeBefore;
			TTicks tickDelta = tickAfter - tickBefore;

			freq = 1000 * tickDelta / timeDelta;
			benchFrequency = false;
		}

		return (double)(sint64)ticks/(double)freq;
	}
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
