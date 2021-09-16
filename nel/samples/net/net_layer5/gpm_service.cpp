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
 * Layer 4 and Service example, ping server.
 *
 * This ping service expects pings, sends pongs back.
 *
 * To run this program, ensure there is a file "ping_service.cfg"
 * containing the location of the naming service (NSHost, NSPort)
 * in the working directory. The naming service must be running.
 */


// We're using the NeL Service framework, and layer 5
#include "nel/net/service.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/displayer.h"
#include "nel/misc/command.h"
#include "nel/misc/hierarchical_timer.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;

uint32 NbId = 0;

//
TTime	pingDate;

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
	CMessage msgout("ACK_POS");
	CUnifiedNetwork::getInstance()->send("PLS", msgout);

	TCPUCycle v1 = CTime::getPerformanceTime ();

	CMessage msgoutfe("POS", false, CMessage::UseDefault, NbId*8);

	uint64 id = rand();
	msgoutfe.serial (NbId);
	for (uint i = 0; i < NbId; i++)
	{
		msgoutfe.serial (id);
		id++;
	}

	TCPUCycle v2 = CTime::getPerformanceTime ();
	
	CUnifiedNetwork::getInstance()->send("FS", msgoutfe);
	
	TCPUCycle v3 = CTime::getPerformanceTime ();

	nlinfo( "Received POS, Sending POS to FS (serial %.2fs, send %.2fs)", CTime::ticksToSecond (v2-v1), CTime::ticksToSecond (v3-v2));
}

void cbAskVision(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32 Value = '0ACE';

//	H_BEFORE (Vision);

	TCPUCycle v1 = CTime::getPerformanceTime ();

//	H_BEFORE (CMessage);
	CMessage msgout("VISION", false, CMessage::UseDefault, 10000000);
//	H_AFTER (CMessage);
//	H_BEFORE (serial);
	msgout.serial(NbId);
//	H_AFTER (serial);
//	H_BEFORE (serials);
	for (uint i = 0; i < NbId; i++)
		msgout.serial( Value );
//	H_AFTER(serials);
//	H_BEFORE (send);
	CUnifiedNetwork::getInstance()->send("FS", msgout);
//	H_AFTER (send);

	/*
	CMessage msgout("VISION");
	uint32 Nb = 10;

	for (uint j = 0; j < 1000; j++)
	{
		msgout.clear();
		msgout.setType("VISION");
		msgout.serial(Nb);
		for (uint i = 0; i < Nb; i++)
			msgout.serial( Value );
		CUnifiedNetwork::getInstance()->send("FS", msgout);
	}
	*/
	TCPUCycle v2 = CTime::getPerformanceTime ();

//	H_AFTER (Vision);

	// ca prend bcp de cpu un info...
	nlinfo("Sent Vision with %d values in %.2fms", NbId, CTime::ticksToSecond (v2-v1)*1000.0f);
}

//
void cbUpPS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Ping Service connecting");
	sendPing();
}

void cbDownPS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Ping Service disconnecting");
}

//
void cbUpFS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("F Service connecting");
	sendPing();
}

void cbDownFS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("F Service disconnecting");
}

//
void cbUpService(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Service %s %d is up", serviceName.c_str(), sid.get());

	CMessage msgout("TOTO");
	uint32 i = 10;
	msgout.serial(i);
	CUnifiedNetwork::getInstance()->send(sid, msgout);
}

void cbDownService(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Service %s %d is down", serviceName.c_str(), sid.get());
}


/*
 * Callback array for messages received from a client
 */
TUnifiedCallbackItem CallbackArray[] =
{
	{ "PONG", cbPong },
	{ "POS", cbPos },
	{ "ASK_VISION", cbAskVision }
};

void cbVar (CConfigFile::CVar &var)
{
	if (var.Name == "NbId") NbId = var.asInt ();
	else nlstop;
}

//
class CGPMService : public IService
{
public:

	bool	update()
	{
		static TTime	lastPing = CTime::getLocalTime();

		TTime	ctime = CTime::getLocalTime();
/*
		// check ping every 15 seconds
		if (ctime - lastPing > 15000)
		{
			sendPing();
			lastPing = ctime;
		}
*/

		return true;
	}

	/*
	 * Initialization
	 */
	void init()
	{
//		nlerror ("oups");
/*		//nlassert(false);
		char *p=0;
		p[0]=0;
		printf(p);
*/
		ConfigFile.setCallback ("NbId", cbVar);
		cbVar (ConfigFile.getVar ("NbId"));

		// Connect to the ping service
		CUnifiedNetwork	*instance = CUnifiedNetwork::getInstance();

		instance->setServiceUpCallback("PS", cbUpPS, NULL);
		instance->setServiceDownCallback("PS", cbDownPS, NULL);

		instance->setServiceUpCallback("FS", cbUpFS, NULL);
		instance->setServiceDownCallback("FS", cbDownFS, NULL);

		instance->setServiceUpCallback("*", cbUpService, NULL);
		instance->setServiceDownCallback("*", cbDownService, NULL);
	}
};


/*
 * Declare a service with the class IService, the names "PS" (short) and "ping_service" (long).
 * The port is automatically allocated (0) and the main callback array is CallbackArray.
 */
NLNET_SERVICE_MAIN( CGPMService, "GPMS", "gpm_service", 0, CallbackArray, "", "" )

NLMISC_COMMAND (wait, "", "<time>")
{
	if(args.size() != 1) return false;

	nlSleep (atoi (args[0].c_str()));

	return true;
}
