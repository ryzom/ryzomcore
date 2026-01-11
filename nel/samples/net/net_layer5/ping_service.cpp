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
 * Layer 5 and Service example, ping server.
 *
 * This ping service expects pings, sends pongs back.
 *
 * To run this program, ensure there is a file "ping_service.cfg"
 * containing the location of the naming service (NSHost, NSPort)
 * in the working directory. The naming service must be running.
 */


// We're using the NeL Service framework, and layer 5
#include "nel/net/service.h"
#include "nel/misc/displayer.h"

using namespace NLNET;
using namespace NLMISC;

CFileDisplayer fd("ps.log");

/*
 * Callback function called when receiving a "PING" message
 *
 * Arguments:
 * - msgin:	the incoming message (coming from a client)
 * - from: the "sockid" of the sender client
 * - server: the CCallbackNetBase object (which really is a CCallbackServer object, for a server)
 *
 * Input (expected message from a client): PING
 * - uint32: ping counter
 *
 * Output (sent message to the ping server): PONG
 * - uint32: ping counter
 */
void cbPing(CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32 counter;

	// Input
	msgin.serial( counter );

	// Output (uses layer 4 but this is not really necessary, see server.cpp in layer 3 example)
	CMessage msgout("PONG");
	msgout.serial( counter );
	CUnifiedNetwork::getInstance()->send(sid, msgout);

	nlinfo( "PING -> PONG %u", counter );
}

//
void cbUpService(const std::string &serviceName, TServiceId sid, void *arg)
{
	nlinfo("Service %s %d is up", serviceName.c_str(), sid.get());

	// Output (uses layer 4 but this is not really necessary, see server.cpp in layer 3 example)
	CMessage msgout("PONG");
	uint32 counter = 0xFFFFFFFF;
	msgout.serial( counter );
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
	{ "PING", cbPing }
};


//
class CPingService : public IService
{
public:

	/*
	 * Initialization
	 */
	void init()
	{
		DebugLog->addDisplayer (&fd);
		InfoLog->addDisplayer (&fd);
		WarningLog->addDisplayer (&fd);
		ErrorLog->addDisplayer (&fd);

		// Connect to the ping service
		CUnifiedNetwork	*instance = CUnifiedNetwork::getInstance();

		instance->setServiceUpCallback("*", cbUpService, NULL);
		instance->setServiceDownCallback("*", cbDownService, NULL);
	}
};


/*
 * Declare a service with the class IService, the names "PS" (short) and "ping_service" (long).
 * The port is automatically allocated (0) and the main callback array is CallbackArray.
 */
NLNET_SERVICE_MAIN( CPingService, "PS", "ping_service", 0, CallbackArray, "", "" )
