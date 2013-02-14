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

#include "nel/misc/stop_watch.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {


/*
 * Constructor
 */
CStopWatch::CStopWatch( uint queueLength ) :
	_BeginTime( 0 ),
	_ElapsedTicks( 0 ),
	_SumTicks( 0 ),
	_MeasurementNumber( 0 ),
	_Queue(),
	_QLength( queueLength )
{}


/*
 * Begin measurement
 */
void	CStopWatch::start()
{
	_BeginTime = CTime::getPerformanceTime();
	_ElapsedTicks = 0;
}


/*
 * Pause
 */
void	CStopWatch::pause()
{
	_ElapsedTicks += (TTickDuration)(CTime::getPerformanceTime() - _BeginTime);
}


/*
 * Resume
 */
void	CStopWatch::resume()
{
	_BeginTime = CTime::getPerformanceTime();
}


/*
 * Add time (in TTicks unit) to the current measurement
 */
void	CStopWatch::addTime( TTickDuration t )
{
	_ElapsedTicks += t;
}


/*
 * End measurement
 */
void	CStopWatch::stop()
{
	_ElapsedTicks += (TTickDuration)(CTime::getPerformanceTime() - _BeginTime);

	// Setup average
	_SumTicks += _ElapsedTicks;
	++_MeasurementNumber;

	// Setup partial average
	if ( _QLength != 0 )
	{
		_Queue.push_back( _ElapsedTicks );
		if ( _Queue.size() > _QLength )
		{
			_Queue.pop_front();
		}
	}
}


/*
 * Add an external duration (in TTicks unit) to the average queue
 */
void	CStopWatch::addMeasurement( TTickDuration t )
{
	// Setup average
	_SumTicks += t;
	++_MeasurementNumber;

	// Setup partial average
	if ( _QLength != 0 )
	{
		_Queue.push_back( t );
		if ( _Queue.size() > _QLength )
		{
			_Queue.pop_front();
		}
	}

}


/*
 * Elapsed time in millisecond (call it after stop())
 */
TMsDuration	CStopWatch::getDuration() const
{
	return (TMsDuration)(CTime::ticksToSecond( _ElapsedTicks ) * 1000.0);
}


/*
 * Average of the queueLength last durations (using the queueLength argument specified in the constructor)
 */
TMsDuration	CStopWatch::getPartialAverage() const
{
	if (_Queue.size() == 0)
		return (TMsDuration)0;
	else
		return (TMsDuration)(CTime::ticksToSecond( accumulate( _Queue.begin(), _Queue.end(), 0 ) / _Queue.size() ) * 1000.0);
}


/*
 * Average of the duration
 */
TMsDuration	CStopWatch::getAverageDuration() const
{
	return (TMsDuration)(CTime::ticksToSecond( _SumTicks / _MeasurementNumber ) * 1000.0);
}



} // NLMISC
