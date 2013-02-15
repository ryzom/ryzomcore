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


/////////////
// INCLUDE //
/////////////
// Misc;
#include "nel/misc/debug.h"
// client.
#include "time_client.h"
#include "interface_v3/interface_manager.h"
#include "nel/gui/interface_property.h"
#include "client_cfg.h"
#include "net_manager.h"
#include "game_share/zc_shard_common.h"
#include "weather.h"
#include "game_share/light_cycle.h"


///////////
// USING //
///////////
using namespace std;


////////////
// GLOBAL //
////////////
sint64	T0 = 0;				// Time for the last frame.
sint64	T1 = 0;				// Time for the current frame.
sint64	TS = 0;				// Time since the really first Frame.
sint64	DT64 = 0;			// Diff time with current and last frame in ms.
float	DT = 0.f;			// Diff time with current and last frame in sec.
TTime	TSend = 0;			// Next Time to send motions.
TTime	DTSend = 100;		// Delta of time to generate the next time to send motions.
double	TimeInSec = 0.0;	// Time for the current frame in second.
double	FirstTimeInSec = 0.0;	// Game local origin time
float   TimeFactor = 1.f; // Factor applied to current date (local mode only)


TTime	LCT = 1000;				// Default LCT (Lag Compensation time).
TTime	LocalTimeStep = 100;	// Time Step factor on the client.
TTime	CurrentPacketTime = 0;	// Local Time for the current packet.
uint8	CurrentPacket = 0;		// Packet currently valid.
uint8	LastPacketReceived = 0;	// Last received packet.
uint8	LastPacketAck = 0;		// Last acknowledged packet.

NLMISC::TGameCycle LastGameCycle = 0;

CRyzomTime			RT;
CClientDate			SmoothedClientDate;

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Update_Time )


///////////
// LOCAL //
///////////
// Map with all time for each packet sent to the server.
map<uint8, TTime> PacketTime;
// Map with all Infos for each packet received.
map<uint8, CPacketInfos> PacketInfos;


///////////////
// FUNCTIONS //
///////////////
//-----------------------------------------------
// removePacketTime :
// Remove the packet 'numPacket' from the 'PacketTime' list.
//-----------------------------------------------
void removePacketTime(uint8 numPacket)
{
	PacketTime.erase(numPacket);
}// removePacketTime //

//-----------------------------------------------
// insertPacketTime :
// Insert a packet in the PacketTime map.
//-----------------------------------------------
void insertPacketTime(uint8 numPacket, TTime time)
{
	PacketTime.insert(make_pair(numPacket, time));
}// insertPacketTime //


//-----------------------------------------------
// checkLocalTimeStep :
// Check if the Local Time Step is still valid with the new packet.
//-----------------------------------------------
void checkLocalTimeStep(const CPacketInfos &packetInfos)
{
	// Compute how many packets there are since the current packet.
	sint nbPacket = packetInfos.Num - CurrentPacket;
	if(nbPacket < 0)
		nbPacket += 256;

	// \todo GUIGUI : check why this happen.
	if(nbPacket==0)
		return;

	// Compute the predicted Local Time for this packet.
	TTime predictedTime = CurrentPacketTime + LocalTimeStep*nbPacket;

	TTime localTimeStep = LocalTimeStep;

	// If the predicted time is too small according to the min time computed for the packet -> increase 'LocalTimeStep'.
	if(predictedTime < packetInfos.Min)
	{
//		nlwarning("packetInfos.Min : %f", (double)packetInfos.Min);
//		nlwarning("CurrentPacketTime : %f", (double)CurrentPacketTime);
//		nlwarning("packetInfos.Min - CurrentPacketTime : %f", (double)((double)packetInfos.Min - (double)CurrentPacketTime));

		LocalTimeStep = (packetInfos.Min - CurrentPacketTime)/nbPacket;

//		nlwarning("LocalTimeStep : %f", (double)LocalTimeStep);
	}
	// If the predicted time is too big according to the max time computed for the packet -> increase 'LocalTimeStep'.
	else if(predictedTime > packetInfos.Max)
	{
//		nlwarning("packetInfos.Max : %f", (double)packetInfos.Max);
//		nlwarning("CurrentPacketTime : %f", (double)CurrentPacketTime);
//		nlwarning("packetInfos.Max - CurrentPacketTime : %f", (double)((double)packetInfos.Max - (double)CurrentPacketTime));

		LocalTimeStep = (packetInfos.Max - CurrentPacketTime)/nbPacket;

//		nlwarning("LocalTimeStep : %f", (double)LocalTimeStep);
	}

	if(LocalTimeStep == 0)
	{
		LocalTimeStep = localTimeStep;
//		nlwarning("LocalTimeStep (corrected): %f", (double)LocalTimeStep);
	}
}// checkLocalTimeStep //

//-----------------------------------------------
// removePacket :
// Remove the packet 'numPacket' from the 'PacketInfos' list.
//-----------------------------------------------
void removePacket(uint8 numPacket)
{
	PacketInfos.erase(numPacket);
}// removePacket //

//-----------------------------------------------
// insertPacket :
// Insert the packet received information in a list.
//-----------------------------------------------
void insertPacket(const CPacketInfos &packetInfos)
{
	// Upadte the last packet received.
	LastPacketReceived = packetInfos.Num;

	// Insert the packet information.
	PacketInfos.insert(make_pair(LastPacketReceived, packetInfos));

	// Check Local Time Step with the new packet.
	checkLocalTimeStep(packetInfos);
}// insertPacket //


//-----------------------------------------------
// getSentPacketTime :
// Return the sent time for a given packet or 0 if the packet is not found
//-----------------------------------------------
TTime getSentPacketTime(uint8 numPacket)
{
	map<uint8, TTime>::iterator it = PacketTime.find(numPacket);
	if(it != PacketTime.end())
		return (*it).second;
	else
		return 0;
}// getSentPacketTime //

//-----------------------------------------------
// ackPacketTimeBefore :
//
//-----------------------------------------------
void ackPacketTimeBefore(uint8 ackPacketNumber)
{
	// If the acknowledged packet is not in the list -> player do not have send any packet for the time.
	map<uint8, TTime>::iterator itEnd = PacketTime.find(ackPacketNumber);
	if(itEnd != PacketTime.end())
	{
		// If the packet cannot be found -> First valid packet the server is acknowledging
		map<uint8, TTime>::iterator itBegin = PacketTime.find(LastPacketAck);
		if(itBegin != PacketTime.end())
		{
			while(itBegin != itEnd)
			{
				//
				map<uint8, TTime>::iterator itTmp = itBegin;
				itBegin++;
				// Destroy packets between the last acknowledged packet and the recent one.
				PacketTime.erase(itTmp);

				if(itBegin == PacketTime.end())
					itBegin = PacketTime.begin();
			}
		}
	}

	LastPacketAck = ackPacketNumber;
}// ackPacketTimeBefore //

static	CInterfaceProperty	*InterfaceTime = NULL;
static	CInterfaceProperty	*InterfaceServerTick = NULL;
static	CInterfaceProperty	*InterfaceSmoothServerTick = NULL;
static	CInterfaceProperty	*InterfaceDay = NULL;
static	CInterfaceProperty	*InterfaceDayBeforeNextZCDistrib = NULL;

//-----------------------------------------------
// initClientTime
//
//-----------------------------------------------
void initClientTime()
{
	nlassert (InterfaceTime == NULL);
	nlassert (InterfaceServerTick == NULL);
	nlassert (InterfaceSmoothServerTick == NULL);
	nlassert (InterfaceDay== NULL);
	nlassert (InterfaceDayBeforeNextZCDistrib== NULL);

	InterfaceTime = new CInterfaceProperty;
	InterfaceServerTick = new CInterfaceProperty;
	InterfaceSmoothServerTick = new CInterfaceProperty;
	InterfaceDay = new CInterfaceProperty;
	InterfaceDayBeforeNextZCDistrib = new CInterfaceProperty;

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// get a direct link to the database
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CURRENT_TIME");
	InterfaceTime->link("UI:VARIABLES:CURRENT_TIME");
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CURRENT_SERVER_TICK");
	InterfaceServerTick->link("UI:VARIABLES:CURRENT_SERVER_TICK");
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CURRENT_SMOOTH_SERVER_TICK");
	InterfaceSmoothServerTick->link("UI:VARIABLES:CURRENT_SMOOTH_SERVER_TICK");
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CURRENT_DAY");
	InterfaceDay->link("UI:VARIABLES:CURRENT_DAY");
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DAY_BEFORE_ZC_DISTRIB");
	InterfaceDayBeforeNextZCDistrib->link("UI:VARIABLES:DAY_BEFORE_ZC_DISTRIB");
}

//-----------------------------------------------

void releaseClientTime()
{
	if (InterfaceTime)
	{
		delete InterfaceTime;
		InterfaceTime = NULL;
	}
	if (InterfaceServerTick)
	{
		delete InterfaceServerTick;
		InterfaceServerTick = NULL;
	}
	if (InterfaceSmoothServerTick)
	{
		delete InterfaceSmoothServerTick;
		InterfaceSmoothServerTick = NULL;
	}
	if (InterfaceDay)
	{
		delete InterfaceDay;
		InterfaceDay = NULL;
	}
	if (InterfaceDayBeforeNextZCDistrib)
	{
		delete InterfaceDayBeforeNextZCDistrib;
		InterfaceDayBeforeNextZCDistrib = NULL;
	}
}




//-----------------------------------------------
// updateClientTime :
//
//-----------------------------------------------
CWidgetManager::SInterfaceTimes times;

void updateClientTime()
{
	H_AUTO_USE ( RZ_Client_Update_Time )

	// Last T0
	T0        = T1;

	// Constant delta time ?
	if (ClientCfg.ForceDeltaTime == 0)
	{
		T1 = ryzomGetLocalTime();
		if (T1 <= T0)
		{
			// This case is known to occurs if the framerate is very fast (e.g. during the
			// loading stage) or if the machine has got a heavy load (e.g. lots of AGP data)
			// delaying the timer updates.
			//nlwarning ("getLocalTime has returned the same time! START");
			while (T1 <= T0)
			{
				T1 = ryzomGetLocalTime();
				// give up the time slice to let time to other process
				NLMISC::nlSleep(0);
			}
			//nlwarning ("getLocalTime has returned the same time! END");
		}
	}
	else
	{
		// First time ?
		static bool firstTime = true;
		if (firstTime)
		{
			T1 = ryzomGetLocalTime();
			firstTime = false;
		}
		else
		{
			T1 += ClientCfg.ForceDeltaTime;
		}
	}

	if(T0 == T1)
		nlwarning("updateClientTime: T0 should not be = T1.");

	// Update time variables
	DT64      = T1-T0;
	TS        += DT64;
	DT        = ((float)DT64)*0.001f;

	TimeInSec = T1*0.001;

	// First time ?
	if (FirstTimeInSec == 0.0)
		FirstTimeInSec = TimeInSec;

	// Set the InterfaceManager current time.
	if (InterfaceTime)
		InterfaceTime->setSInt64(T1);
	if (InterfaceServerTick)
		InterfaceServerTick->setSInt64(NetMngr.getCurrentServerTick());
	// Set the Smooth ServerTick
	if (InterfaceSmoothServerTick)
		InterfaceSmoothServerTick->setSInt64(NetMngr.getCurrentSmoothServerTick());

	// Day, and Day before ZC distrib
	if(InterfaceDay)
		InterfaceDay->setSInt64(RT.getRyzomDay());
	if(InterfaceDayBeforeNextZCDistrib)
	{
		sint	d= RT.getRyzomDay()%ZCSTATE::ZCDistribPeriod;
		// Must dispaly at least "1".
		d= ZCSTATE::ZCDistribPeriod - d;
		InterfaceDayBeforeNextZCDistrib->setSInt64(d);
	}

#ifdef ENABLE_INCOMING_MSG_RECORDER
	// Init For the Replay.
	if(NetMngr.isReplayStarting())
		NetMngr.startReplay();
#endif

	times.lastFrameMs = T0;
	times.thisFrameMs = T1;
	times.frameDiffMs = DT64;

	CWidgetManager::getInstance()->updateInterfaceTimes( times );

}// updateClientTime //


// update smoothed time
void updateSmoothedTime()
{
	#define REAL_SECONDS_TO_RYZOM_HOUR(s) (double(s) * 10 / double(RYZOM_HOURS_IN_TICKS))
	//
	CClientDate oldSmoothedClientDate = SmoothedClientDate;
	double dHour = REAL_SECONDS_TO_RYZOM_HOUR(DT);
	SmoothedClientDate.Hour += (float) dHour;
	if (SmoothedClientDate.Hour > WorldLightCycle.NumHours)
	{
		SmoothedClientDate.Day += (sint32) ceil(dHour / WorldLightCycle.NumHours);
		SmoothedClientDate.Hour = (float) fmod(SmoothedClientDate.Hour, WorldLightCycle.NumHours);
	}
	double smoothedClientDateInHour = (double) SmoothedClientDate.Day * 24 + SmoothedClientDate.Hour;
	double ryzomDateInHour = (double) RT.getRyzomDay() * 24 + (double) RT.getRyzomTime();
	const double SMOOTHED_DATE_MAX_ERROR_IN_RYZOM_HOUR = REAL_SECONDS_TO_RYZOM_HOUR(10);
	double hourError = fabs(smoothedClientDateInHour - ryzomDateInHour);
	if (hourError > SMOOTHED_DATE_MAX_ERROR_IN_RYZOM_HOUR)
	{
		if (smoothedClientDateInHour < ryzomDateInHour)
		{
			// smoothed time is late
			SmoothedClientDate.Day = RT.getRyzomDay();
			SmoothedClientDate.Hour = RT.getRyzomTime();
		}
		else
		{
			if (hourError > (2 * SMOOTHED_DATE_MAX_ERROR_IN_RYZOM_HOUR))
			{
				// if difference is too big, rescale smoothed time on server time
				SmoothedClientDate.Day = RT.getRyzomDay();
				SmoothedClientDate.Hour = RT.getRyzomTime();
			}
			else
			{
				// cancel time increment (wait until server time is up with smoothed client time)
				SmoothedClientDate = oldSmoothedClientDate;
			}
		}
	}
}





















