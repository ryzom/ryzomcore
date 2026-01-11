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

#include "nel/misc/thread.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;


//
const uint			NumThreads = 10;
volatile TTime		PingDates[NumThreads];
volatile bool		ServiceReady = false;

void cbPong(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32	counter;
	msgin.serial( counter );
	TTime	pingTime = CTime::getLocalTime()-PingDates[counter];
	PingDates[counter] = 0;
	nlinfo("Received PONG %u (%u ms)", counter, pingTime);

	CMessage msgout("ACK_POS");
	CUnifiedNetwork::getInstance()->send("PLS", msgout);
}

void sendPing(uint i)
{
	PingDates[i] = CTime::getLocalTime();
	uint32 counter = i;
	CMessage msgout("PING");
	msgout.serial( counter );
	nlinfo( "Send PING %d", counter);
	CUnifiedNetwork::getInstance()->send("PS", msgout);
	nlinfo( "PING %d sent", counter);
}

class CPinger : public IRunnable
{
private:
	static volatile uint	_PingerCount;
	uint					_PingerId;

public:
	CPinger()
	{
		_PingerId = _PingerCount++;
	}

	void	run()
	{
		uint	i;
		uint	totalPing = 0;

		while (!ServiceReady)
		{
			nlSleep(10);
		}

		while (totalPing < 200)
		{
			i = rand()*200/RAND_MAX+200;
			nlSleep(i);
			if (PingDates[_PingerId] == 0)
			{
				sendPing(_PingerId);
				++totalPing;
			}
		}
	}
};

volatile uint	CPinger::_PingerCount = 0;

//
void cbPos(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	CMessage msgout("POS");
	CUnifiedNetwork::getInstance()->send("GPMS", msgout);

	nlinfo( "Received POS from %s, send POS to GPMS", serviceName.c_str());
}

void cbAckPos(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	nlinfo( "Received ACK_POS from %s", serviceName.c_str());
}

//
void cbUpPS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Ping Service connecting");
}

void cbDownPS(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Ping Service disconnecting");
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
 * Callback array for messages received from a client
 */
TUnifiedCallbackItem CallbackArray[] =
{
	{ "PONG", cbPong },
	{ "POS", cbPos },
	{ "ACK_POS", cbAckPos }
};


//
class CFloodService : public IService
{
public:

	bool	update()
	{
		ServiceReady = true;
		return true;
	}

	/*
	 * Initialization
	 */
	void init()
	{
		// Connect to the ping service
		CUnifiedNetwork	*instance = CUnifiedNetwork::getInstance();

		instance->setServiceUpCallback("PS", cbUpPS, NULL);
		instance->setServiceDownCallback("PS", cbDownPS, NULL);

		instance->setServiceUpCallback("*", cbUpService, NULL);
		instance->setServiceDownCallback("*", cbDownService, NULL);

		uint	i;

		for (i=0; i<NumThreads; ++i)
		{
			IRunnable	*runnable = (IRunnable *)(new CPinger());
			IThread		*thread = IThread::create(runnable);
			thread->start();
		}
	}
};


/*
 * Declare a service with the class IService, the names "PS" (short) and "ping_service" (long).
 * The port is automatically allocated (0) and the main callback array is CallbackArray.
 */
NLNET_SERVICE_MAIN( CFloodService, "FLS", "flood_service", 0, CallbackArray, "", "" )
