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




#ifndef TICK_PROXY_TIME_MEASURE_H
#define TICK_PROXY_TIME_MEASURE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/net/unified_network.h"

//
typedef NLMISC::TTime TAccurateTime;

// The accuracy can be tuned in this function (using rdtsc, getPerformanceTime, etc.)
inline TAccurateTime getAccurateTime()
{
	return NLMISC::CTime::getLocalTime();
}

// Depending of the unit of getAccurateTimeInTicks()
inline uint16 accTimeToMs( TAccurateTime timeInAccurateUnit )
{
	return uint16(timeInAccurateUnit&0xffff);
}


typedef const char* *TMeasureTypeToCString;

enum TServiceTimeMeasureType { TickTockInterval, TickUpdateDuration, PrevProcessMirrorUpdateDuration, PrevReceiveMsgsViaMirrorDuration, PrevTotalGameCycleDuration, NbServiceTimeMeasureTypes };
enum TMirrorTimeMeasureType { WaitAllReceivedDeltaDuration, BuildAndSendDeltaDuration, PrevApplyReceivedDeltaDuration, PrevSendUMMDuration, NbMirrorTimeMeasureTypes };


/**
 *
 */
template <int N>
class CTimeMeasure
{
public:

	uint16	V [N];

	///
	CTimeMeasure( uint16 constant=0 ) { for ( uint i=0; i!=N; ++i ) V[i] = constant; }

	///
	uint16&			operator[] ( uint i ) { return V[i]; }

	///
	const uint16&	operator[] ( uint i ) const { return V[i]; }

	///
	uint16			size() const { return N; }

	///
	void			serial( NLMISC::IStream& s ) { for ( uint i=0; i!=N; ++i ) s.serial( V[i] ); }

	///
	void			displayStat( NLMISC::CLog *log, TMeasureTypeToCString toCstr, uint divideBy=1 ) const
	{
		if ( divideBy != 0 )
		{
			for ( uint i=0; i!=N; ++i )
				log->displayRawNL( "\t\t%s: %hu", toCstr[i], V[i] / divideBy );
		}
		else
			log->displayRawNL( "\t\t<No data>" );
	}

	///
	CTimeMeasure&	operator= ( uint16 constant ) { for ( uint i=0; i!=N; ++i ) V[i] = constant; return *this; }

	///
	CTimeMeasure	operator / (uint16 constant) const
	{
		CTimeMeasure<N> nm;
		for ( uint i=0; i!=N; ++i )
			nm.V[i] = V[i] / constant;
		return nm;
	}

/*#if defined( _MSC_VER )
	friend CTimeMeasure	operator/ ( const CTimeMeasure& m, uint16 constant );
#else
	template <int M> friend CTimeMeasure operator/ ( const CTimeMeasure& m, uint16 constant );
#endif
*/
	///
	void	operator+= ( const CTimeMeasure& other )
	{
		for ( uint i=0; i!=N; ++i )
			V[i] += other.V[i];
	}

	///
	void			copyLowerValues( const CTimeMeasure& src )
	{
		for ( uint i=0; i!=N; ++i )
		{
			if ( src.V[i] < V[i] )
				V[i] = src.V[i];
		}
	}

	///
	void			copyHigherValues( const CTimeMeasure& src )
	{
		for ( uint i=0; i!=N; ++i )
		{
			if ( src.V[i] > V[i] )
				V[i] = src.V[i];
		}
	}
};

///
//template <int N>
//CTimeMeasure<N>	operator/ ( const CTimeMeasure<N>& m, uint16 constant )
//{
//	CTimeMeasure<N> nm;
//	for ( uint i=0; i!=N; ++i )
//		nm.V[i] = m.V[i] / constant;
//	return nm;
//}

typedef CTimeMeasure<NbServiceTimeMeasureTypes> CServiceTimeMeasure;
typedef CTimeMeasure<NbMirrorTimeMeasureTypes> CMirrorTimeMeasure;

/**
 *
 */
class CServiceGameCycleTimeMeasure
{
public:


	CServiceTimeMeasure	ServiceMeasure;

	NLNET::TServiceId	ClientServiceId;

	///
	void			serial( NLMISC::IStream& s )
	{
		s.serial( ServiceMeasure );
		s.serial( ClientServiceId );
	}
};


/**
 *
 */
class CMirrorGameCycleTimeMeasure
{
public:

	std::vector<CServiceGameCycleTimeMeasure>	ServiceMeasures;
	CMirrorTimeMeasure							MirrorMeasure;

	///
	void			serial( NLMISC::IStream& s )
	{
		s.serialCont( ServiceMeasures );
		s.serial( MirrorMeasure );
	}
};


#endif





















