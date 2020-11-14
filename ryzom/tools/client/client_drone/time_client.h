/** \file time_client.h
 * <File description>
 *
 * $Id: time_client.h,v 1.1 2005/07/11 15:22:33 cado Exp $
 */




#ifndef CL_TIME_CLIENT_H
#define CL_TIME_CLIENT_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
// Game Share
#include "game_share/time_weather_season/time_and_season.h"
//#include "client_cfg.h"

///////////
// USING //
///////////
using NLMISC::TTime;

class CClientDate;

////////////
// GLOBAL //
////////////
extern sint64	T0;			// Time for the last frame.
extern sint64	T1;			// Time for the current frame.
extern sint64	TS;			// Time since the really first Frame.
extern sint64	DT64;		// Diff time with current and last frame in ms.
extern float	DT;			// Diff time with current and last frame in sec.
extern TTime	TSend;		// Next Time to send motions.
extern TTime	DTSend;		// Delta of time to generate the next time to send motions.
                                 extern double	TimeInSec;	// Time for the current frame in second.
extern double	FirstTimeInSec;	// Game local origin time 


extern TTime	LCT;
extern TTime	LocalTimeStep;
extern TTime	CurrentPacketTime;
extern uint8	CurrentPacket;
extern uint8	LastPacketReceived;

extern NLMISC::TGameCycle LastGameCycle;

extern CRyzomTime			RT;

extern CClientDate          SmoothedClientDate;

///////////
// CLASS //
///////////
class CPacketInfos
{
public:
	uint8	Num;
	TTime	Min;
	TTime	Max;

	CPacketInfos() {};
	CPacketInfos(uint8 num, TTime min, TTime max)
	{
		Num = num;
		Min = min;
		Max = max;
	}
};

/** A representation of time in the client
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CClientDate
{
public:
	sint32 Day;
	float  Hour;
	CClientDate(sint32 day = 0, float hour = 0.f) : Day(day), Hour(hour) {}
};

// 'less' comparison between 2 dates
inline bool operator < (const CClientDate &lhs, const CClientDate &rhs)
{
	if (lhs.Day != rhs.Day) return lhs.Day < rhs.Day;
	return lhs.Hour < rhs.Hour;
}
// equality between 2 dates
inline bool operator == (const CClientDate &lhs, const CClientDate &rhs)
{
	return lhs.Day == rhs.Day && lhs.Hour == rhs.Hour;	
}

///////////////
// FUNCTIONS //
///////////////
void removePacketTime(uint8 numPacket);
///
void insertPacketTime(uint8 numPacket, TTime time);
void ackPacketTimeBefore(uint8 ackPacketNumber);

void removePacket(uint8 numPacket);
/// Insert the packet received informations in a list.
void insertPacket(const CPacketInfos &packetInfos);
/// Return the sent time for a given packet or 0 if the packet is not found
TTime getSentPacketTime(uint8 numPacket);

// Initialize interface properties linked to the client time
void initClientTime();

// This update T0, T1, DT64, DT, TS & TimeInSec
void updateClientTime();

// update smoothed time (useful for sky animation)
//void updateSmoothedTime();

inline TTime ryzomGetLocalTime ()
{	
	return (TTime)(NLMISC::CTime::ticksToSecond (NLMISC::CTime::getPerformanceTime()) * 1000.0);
}

inline NLMISC::TTicks ryzomGetPerformanceTime()
{	
	return NLMISC::CTime::getPerformanceTime();
}

#endif // CL_TIME_CLIENT_H

/* End of time_client.h */
