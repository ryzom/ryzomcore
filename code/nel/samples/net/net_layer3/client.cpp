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
 * Layer 3 example, client.
 *
 * This client connects to a front-end server at localhost:37000.
 * It sends pings and expects pongs (ping replies).
 */


// We're using NeL
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
using namespace NLMISC;

// We're using CInetAddress, CMessage and CCallbackClient (because we're building a client)
#include "nel/net/inet_address.h"
#include "nel/net/message.h"
#include "nel/net/callback_client.h"
using namespace NLNET;


/*
 * Callback function called when receiving a "PONG" message
 *
 * Arguments:
 * - msgin:	the incoming message
 * - from: the "sockid" of the sender (usually useless for a CCallbackClient)
 * - client: the CCallbackNetBase object (which really is a CCallbackClient object, for a client)
 *
 * Input (expected message): PONG
 * - uint32: ping counter
 *
 * Output (sent message): PING
 * - uint32: new ping counter
 */
void cbPong( CMessage& msgin, TSockId from, CCallbackNetBase& client )
{
	uint32 counter;

	// Input
	msgin.serial( counter );
	nlinfo( "Received PONG number %u", counter );
	counter++;

	// Output
	CMessage msgout ( "PING" );
	msgout.serial( counter );
	client.send( msgout );
	nlinfo( "Sent PING number %u", counter );
}


/*
 * Callback array
 */
TCallbackItem CallbackArray[] =
{
	{ "PONG", cbPong }	// when receiving a "PONG" message, call cbPong()
};


/*
 * main
 */
void main( int argc, char **argv )
{
	try
	{
		// Initialize and connect to the front-end server at localhost:37000
		CCallbackClient client;
		client.addCallbackArray( CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]) );
		client.connect( CInetAddress( "localhost" , 37000 ) );

		// Send a PING message
		uint32 counter = 0;
		CMessage msg( "PING" );	// create the message
		msg.serial( counter );						// serialize the counter into the message
		client.send( msg );							// put into the send queue
		nlinfo( "Sent PING number %u", counter );

		// Main loop
		while ( client.connected() )
		{
			// Perform sends and receives, call callbacks
			client.update();
		}

		nlinfo( "Disconnected" );
	}
	catch ( Exception &e )
	{
		nlerror( "Error: %s", e.what() );
	}
}
