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
 * Layer 3 and Service example, front-end server.
 *
 * This front-end server expects pings, and forward them to
 * the real ping server. When the ping server sends a pong back,
 * the front-end server forwards it to the client.
 *
 * To run this program, ensure there is a file "frontend_service.cfg"
 * containing the location of the naming service (NSHost, NSPort)
 * in the working directory. The naming service must be running.
 */


// We're building a server, using the NeL Service framework.
// We need the naming service to know where the ping service is, so we're using CNamingClient.
// We're using CCallbackClient when we talk to the naming service.
// We're using CCallbackServer when we talk to our clients.
#include "nel/net/service.h"
#include "nel/net/naming_client.h"
#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"
using namespace NLNET;

#include <deque>
using namespace std;


// The front-end server is also a client of the ping service
CCallbackClient *ToPingService;

// Temp storage (a queue because the connection to the ping service is reliable, the order is preserved)
deque<TSockId> ClientIds;


/*
 * Callback function called when receiving a "PING" message
 *
 * Arguments:
 * - msgin:	the incoming message (coming from a client)
 * - from: the "sockid" of the sender client
 * - frontendserver: the CCallbackNetBase object (which really is a CCallbackServer object, for a server)
 *
 * Input (expected message from a client): PING
 * - uint32: ping counter
 *
 * Output (sent message to the ping server): PONG
 * - uint32: ping counter
 */
void cbPing( CMessage& msgin, TSockId from, CCallbackNetBase& frontendserver )
{
	uint32 counter;

	// Input
	msgin.serial( counter );
	ClientIds.push_back( from ); // store client sockid

	// Output
	CMessage msgout( "PING" );
	msgout.serial( counter );
	vector<uint8> vect( 400000 );
	msgout.serialCont( vect );
	ToPingService->send( msgout );

	nlinfo( "Received PING number %u from %s", counter, frontendserver.hostAddress(from).asString().c_str() );
}


/*
 * Disconnection callback, called when a client disconnects
 */
void discCallback( TSockId from, void *p )
{
	// Remove all occurences of from in the queue
	deque<TSockId>::iterator iq;
	for ( iq=ClientIds.begin(); iq!=ClientIds.end(); )
	{
		if ( *iq == from )
		{
			iq = ClientIds.erase( iq );
		}
		else
		{
			iq++;
		}
	}
}


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
void cbPong( CMessage& msgin, TSockId from, CCallbackNetBase& clientofthepingserver )
{
	uint32 counter;
	TSockId clientfrom;

	// Input: process the reply of the ping service
	msgin.serial( counter );
	clientfrom = ClientIds.front(); // retrieve client sockid
	ClientIds.pop_front();

	// Output: send the reply to the client
	CCallbackServer *server = IService::getInstance()->getServer();
	CMessage msgout( "PONG" );
	msgout.serial( counter );
	server->send( msgout, clientfrom );

	nlinfo( "Sent PONG number %u to %s", counter, clientfrom->asString().c_str() );
}


/*
 * Callback array for messages received from a client
 */
TCallbackItem CallbackArray[] =
{
	{ "PING", cbPing }	// when receiving a "PING" message, call cbPing()
};


/*
 * Callback array for message received from the ping service
 */
TCallbackItem PingServiceCallbackArray[] =
{
	{ "PONG", cbPong }	// when receiving a "PONG" message, call cbPong()
};


/*
 * CFrontEndService, based on IService
 */
class CFrontEndService : public IService
{
public:

	/*
	 * Initialization
	 */
	void init()
	{
		// Connect to the ping service
		ToPingService = new CCallbackClient( IService::getRecordingState(), "PS.nmr" );
		ToPingService->addCallbackArray( PingServiceCallbackArray, sizeof(PingServiceCallbackArray)/sizeof(PingServiceCallbackArray[0]) );
		if ( ! CNamingClient::lookupAndConnect( "PS", *ToPingService ) )
		{
			nlerror( "Ping Service not available" );
		}

		// Disconnection callback for the clients
		IService::getServer()->setDisconnectionCallback( discCallback, NULL );
	}

	/*
	 * Update
	 */
	bool update()
	{
		ToPingService->update( 20 ); // 20 ms max
		return ToPingService->connected(); // true continues, false stops the service
	}

	/*
	 * Finalization
	 */
	void release()
	{
		delete ToPingService;
	}
};

 
/*
 * Declare a service with the class CFrontEndService, the names "FS" (short) and "frontend_service" (long).
 * The port is set to 37000 and the main callback array is CallbackArray.
 */
NLNET_OLD_SERVICE_MAIN( CFrontEndService, "FS", "frontend_service", 37000, CallbackArray, "", "" )
