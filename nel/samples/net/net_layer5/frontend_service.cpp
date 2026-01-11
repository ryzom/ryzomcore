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


/*
 * Layer 5 and Service example, front-end server.
 *
 * This front-end server expects pings, and forward them to
 * the real ping server. When the ping server sends a pong back,
 * the front-end server forwards it to the client.
 *
 * Even if the connection to the ping server is broken, our
 * front-end server will keep storing the ping messages and
 * will forward them when the connection is restored.
 *
 * To run this program, ensure there is a file "frontend_service.cfg"
 * containing the location of the naming service (NSHost, NSPort)
 * in the working directory. The naming service must be running.
 */


// We're using the NeL Service framework and layer 5.
#include "nel/net/service.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/bit_mem_stream.h"

#include <deque>

using namespace std;
using namespace NLNET;
using namespace NLMISC;


//
TTime	pingDate;

/*
 * Callback function called when receiving a "PONG" message
 *
 * Arguments:
 * - msgin:	the incoming message (coming from the ping server)
 * - from: the "sockid" of the sender (usually useless for a CCallbackClient)
 * - clientofthepingserver: the CCallbackNetBase object (which really is a CCallbackClient object)
 *
 * Input (expected message from the ping server): PONG
 * - uint32: ping counter
 * - TSockId: "sock id" of the client who sent the ping
 *
 * Output (sent message to a client): PONG
 * - uint32: ping counter
 */
void cbPong(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32	counter;
	msgin.serial( counter );
	TTime	pingTime = CTime::getLocalTime()-pingDate;

	nlinfo("Received PONG %u (%u ms)", counter, pingTime);
}

void sendPing()
{
	pingDate = CTime::getLocalTime();
	uint32 counter = 0;
	CMessage msgout("PING");
	msgout.serial( counter );
	nlinfo( "Send PING 0");
	CUnifiedNetwork::getInstance()->send("PS", msgout);
}

//
void cbPos(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// decode the message

	TCPUCycle v1 = CTime::getPerformanceTime ();

	uint32 nbid;
	msgin.serial (nbid);

	for (uint i = 0; i < nbid; i++)
	{
		uint64 id;
		msgin.serial (id);
	}
	
	TCPUCycle v2 = CTime::getPerformanceTime ();

	nlinfo("Received POS from %s (serial: %.2fs)", serviceName.c_str(), CTime::ticksToSecond(v2-v1));
}

TTime t = 0;

void sendRequestVision ()
{
//	nlSleep (1000);
	CMessage msgout("ASK_VISION");
	CUnifiedNetwork::getInstance()->send("GPMS", msgout);
	nlinfo ("ask a new vision");
	t = CTime::getLocalTime ();
}

void cbVision(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32 NbValue;
	uint32 Value;
	
	t = CTime::getLocalTime() - t;

	//H_BEFORE (Vision);

	TCPUCycle v1 = CTime::getPerformanceTime ();
	//H_BEFORE (serial);
	msgin.serial (NbValue);
	//H_AFTER (serial);
	//H_BEFORE (serials);
	for (uint i = 0; i < NbValue; i++)
		msgin.serial( Value );
	//H_AFTER (serials);
	TCPUCycle v2 = CTime::getPerformanceTime ();

	nlinfo("%dms of lag, Received Vision with %d values in %.2fms", (uint32) t, NbValue, CTime::ticksToSecond (v2-v1)*1000.0f);
//	sendRequestVision();

	//H_AFTER (Vision);
}


void sendPos()
{
	nlinfo("Simulate receive pos from client, send POS to GPMS");
	CMessage msgout("POS");
	CUnifiedNetwork::getInstance()->send("GPMS", msgout);
}

//
void cbUpGPMS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo( "GPMS connecting.");
	sendRequestVision ();
}


//
void cbUpPS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo( "Ping Service connecting.");
	sendPing();
}


void cbDownPS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo( "Ping Service disconnecting." );
}

//
void cbUpService(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Service %s %d is up", serviceName.c_str(), sid.get());
}

void cbDownService(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Service %s %d is down", serviceName.c_str(), sid.get());
}


/*
 * Callback array for message received from the ping service
 */
NLNET::TUnifiedCallbackItem CallbackArray[] =
{
	{ "POS", cbPos },
	{ "PONG", cbPong },
	{ "VISION", cbVision }
};


/*
 * CFrontEndService, based on IService
 */
class CFrontEndService : public NLNET::IService
{
public:

	bool	update()
	{
		static TTime	lastPing = CTime::getLocalTime();
		static TTime	lastGetPos = CTime::getLocalTime();

		TTime	ctime = CTime::getLocalTime();

		// check vision every 2 seconds
		if (ctime - lastPing> 2000)
		{
			sendRequestVision();
			lastPing = ctime;
		}

/*
		// check ping every 15 seconds
		if (ctime - lastPing> 15000)
		{
			sendPing();
			lastPing = ctime;
		}

		// do as if receive a position every second
		if (ctime - lastGetPos > 1000)
		{
			sendPos();
			lastGetPos = ctime;
		}
*/
		return true;
	}

	/*
	 * Initialization
	 */
	void init()
	{
/*		uint32 u = 0xFFFFFFFF;
		uint32 z = 0;

		uint32 res = 0;
		CBitMemStream bms2;

		bms2.serial (u, 1);
		bms2.serial (z, 18);
		bms2.serial (u, 4);
		bms2.serial (z, 3);

		nlinfo ("len %d", bms2.length());
		bms2.invert ();
		nlinfo ("len %d", bms2.length());
		bms2.invert ();
		nlinfo ("len %d", bms2.length());
*/
/*		CBitMemStream bms;

		nlinfo ("len %d", bms.length());

		bms.serial (u, 1);
		bms.serial (z, 18);
		bms.serial (u, 4);
		bms.serial (z, 3);
		bms.serial (u, 30);
		nlinfo ("len %d", bms.length());

		bms.clear ();
		nlinfo ("len %d", bms.length());

		bms.serial (z, 1);
		bms.serial (u, 18);
		bms.serial (z, 4);
		bms.serial (u, 3);
		bms.serial (z, 30);
		nlinfo ("len %d", bms.length());


		vector<uint32> cont;
		for(uint i=0;i<32;i++) cont.push_back(i);
		bms.serialCont (cont);

		nlinfo ("len %d", bms.length());


		bms.invert ();
		nlinfo ("len %d", bms.length());

		while (bms.getPosInBit() != 30+3+4+18+1)
		{
			nlinfo ("%d", bms.getPosInBit());
			bms.serial (res, 1);
			nlinfo ((res==0)?"0":"1");
		}
		nlinfo ("%d", bms.getPosInBit());

		vector<uint32> cont2;
		bms.serialCont (cont2);
		nlinfo ("%d", bms.getPosInBit());
		for(uint j=0;j<cont2.size();j++) nlinfo ("const %d %d",j, cont2[j]);

		nlinfo ("%d", bms.getPosInBit());
*/
		// Connect to the ping service
		NLNET::CUnifiedNetwork	*instance = NLNET::CUnifiedNetwork::getInstance();

		instance->setServiceUpCallback("PS", cbUpPS, NULL);
		instance->setServiceDownCallback("PS", cbDownPS, NULL);

		instance->setServiceUpCallback("GPMS", cbUpGPMS, NULL);

		instance->setServiceUpCallback("*", cbUpService, NULL);
		instance->setServiceDownCallback("*", cbDownService, NULL);
	}
};
 
/*
 * Declare a service with the class CFrontEndService, the names "FS" (short) and "frontend_service" (long).
 * The port is set to 37000 and the main callback array is CallbackArray.
 */
NLNET_SERVICE_MAIN( CFrontEndService, "FS", "frontend_service", 37000, CallbackArray, "", "" )
