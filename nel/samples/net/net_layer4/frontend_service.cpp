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
 * Layer 4 and Service example, front-end server.
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
 *
 * DEPRECATED: You should use layer5 (take a look in the layer5 sample directory)
 *
 */


// We're using the NeL Service framework and layer 4.
#include "nel/net/service.h"
#include "nel/net/net_manager.h"
using namespace NLNET;

#include <deque>
using namespace std;


// Storage (a queue because the connection to the ping service is reliable, the order is preserved)
deque< pair<TSockId,uint32> > ClientIds;


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
	ClientIds.push_back( make_pair( from, counter ) ); // store client sockid

	// Output
	CMessage msgout( "PING" );
	msgout.serial( counter );
	CNetManager::send( "PS", msgout ); // does not send if not connected

	nlinfo( "Received PING number %u from %s", counter, frontendserver.hostAddress(from).asString().c_str() );
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

	// Do not send in case of double ping service reply (see onDisconnectPS())
	// (does not work if two clients send the same counter at the same time)
	if ( ( !ClientIds.empty() ) && (ClientIds.front().second == counter) )
	{
		clientfrom = ClientIds.front().first; // retrieve client sockid
		ClientIds.pop_front();

		// Output: send the reply to the client
		CMessage msgout( "PONG" );
		msgout.serial( counter );
		CNetManager::send( "FS", msgout, clientfrom );

		nlinfo( "Sent PONG number %u to %s", counter, clientfrom->asString().c_str() );
	}
}


/*
 * Disonnection callback for the ping service
 */
void onDisconnectPS( const std::string &serviceName, TSockId from, void *arg )
{
	/* Note: the pings already forwarded should get no reply, but it may occur
	 * (e.g. if the server reconnects before the forwarding of a PING and
	 * the reconnection callbacks is called after that). Then onReconnectPS()
	 * may send PINGs that have already been sent and the front-end may get
	 * the same PONG twice. This is partially handled in cbPong.
	 */

	nlinfo( "Ping Service disconnecting: pongs will be delayed until reconnection" );
}


/*
 * Connection callback for the ping service
 */
void onReconnectPS( const std::string &serviceName, TSockId from, void *arg )
{
	uint32 i;
	uint32 counter;

	// Output: forward all stored pings to the reconnected ping service
	for ( i=0; i!=ClientIds.size(); i++ )
	{
		CMessage msgout( "PING" );
		counter = ClientIds[i].second;
		msgout.serial( counter );
		CNetManager::send( "PS", msgout );
	}

	nlinfo( "Ping Service reconnected: %d pings forwarded", ClientIds.size() );
}


/*
 * Disonnection callback for a client
 */
void onDisconnectClient( const std::string &serviceName, TSockId from, void *arg )
{
	// Erase all associated elements in the queue
	deque< pair<TSockId,uint32> >::iterator iq = ClientIds.begin();
 	while ( iq!=ClientIds.end() )
	{
		if ( (*iq).first == from )
		{
			iq = ClientIds.erase( iq );
		}
		else
		{
			iq++;
		}
	}

	nlinfo( "A client has disconnected" );
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
		CNetManager::addClient( "PS" );
		CNetManager::addCallbackArray( "PS", PingServiceCallbackArray, sizeof(PingServiceCallbackArray)/sizeof(PingServiceCallbackArray[0]) );
		CNetManager::setConnectionCallback( "PS", onReconnectPS, NULL );
		CNetManager::setDisconnectionCallback( "PS", onDisconnectPS, NULL );

		CNetManager::setDisconnectionCallback( "FS", onDisconnectClient, NULL );

	}
};

 
/*
 * Declare a service with the class CFrontEndService, the names "FS" (short) and "frontend_service" (long).
 * The port is set to 37000 and the main callback array is CallbackArray.
 */
NLNET_OLD_SERVICE_MAIN( CFrontEndService, "FS", "frontend_service", 37000, CallbackArray, "", "" )
