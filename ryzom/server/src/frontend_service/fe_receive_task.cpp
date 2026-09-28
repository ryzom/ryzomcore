// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "fe_receive_task.h"
#include "fe_types.h"

#ifdef NL_OS_WINDOWS
#	include <winsock2.h>
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
// Windows includes for `sockaddr_in6` and `WSAStringToAddressW`
#	include <ws2ipdef.h>
#elif defined NL_OS_UNIX

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define WSAGetLastError() 0

#endif


using namespace NLMISC;
using namespace NLNET;


volatile uint32 CFEReceiveTask::LastUDPPacketReceived = 0;
volatile uint32 CFEReceiveTask::PendingCookieReceived = 0;

/*
 * TReceivedMessage
 */

/// Constructor
TReceivedMessage::TReceivedMessage() : QuicUser(nullptr)
{
	VAddrFrom.resize(sizeof(sockaddr_in6));
}

/// Return a vector containing the address info
void	TReceivedMessage::addressToVector()
{
	AddrFrom.toSockAddrInet6((sockaddr_in6 *)(&*VAddrFrom.begin()));
}

/// Set address with address info from specified vector
void	TReceivedMessage::vectorToAddress()
{
	AddrFrom.fromSockAddrInet6((sockaddr_in6 *)&*VAddrFrom.begin());
}


/*
 * Constructor (note: called from the main thread)
 */
CFEReceiveTask::CFEReceiveTask( uint16 firstAcceptablePort, uint16 lastAcceptablePort, uint32 msgsize ) :
	_ReceivedMessage(),
	_WriteQueue( "WriteQueue" ), // value unspecified
	_DatagramLength( msgsize ),
	_ExitRequired( false ),
	_RebindRequired( false ),
	_NbRejectedDatagrams( 0 ),
	_BoundPort( 0 )
{
	// Socket
	DataSock = new CUdpSock( false );
	nlassert( DataSock );

	// Test of multihomed host: bind the first address
/*	vector<CInetAddress> addrlist;
	addrlist = CInetAddress::localAddresses();
	vector<CInetAddress>::iterator ivi;
	nlinfo( "Listing local interfaces:" );
	for ( ivi=addrlist.begin(); ivi!=addrlist.end(); ++ivi )
	{
		nlinfo( "%s", (*ivi).asIPString().c_str() );
	}
	addrlist[0].setPort( port );
	DataSock->bind( addrlist[0] );
*/

	// Bind on all network interfaces (TODO: find a simple way to bind only on the external interface; note: change releaase code for Linux in fe_receive_sub.cpp as well)
	uint16 actualPort;
	for ( actualPort=firstAcceptablePort; actualPort<=lastAcceptablePort; ++actualPort )
	{
		try
		{
			DataSock->bind( actualPort );
			break;
		}
		catch (const ESocket &e)
		{
			nlinfo( "Port %u not available: %s", actualPort, e.what() );
		}
	}
	if ( actualPort > lastAcceptablePort )
		nlerror( "Could not find an available port between %hu and %hu", firstAcceptablePort, lastAcceptablePort );
	_BoundPort = actualPort;
	nlinfo( "Binding all network interfaces on port %hu (%hu asked)", actualPort, firstAcceptablePort );

	setRecvTimeout();
}


/*
 * Set a receive timeout on the UDP socket so recvfrom() doesn't block forever.
 * This ensures the receive thread can periodically check for exit/rebind requests
 * and prevents the thread from getting permanently stuck if the socket enters a bad state.
 */
void CFEReceiveTask::setRecvTimeout()
{
#ifdef NL_OS_WINDOWS
	DWORD tv = 1000; // milliseconds
	if (setsockopt(DataSock->descriptor(), SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) != 0)
		nlwarning("Failed to set SO_RCVTIMEO on UDP socket");
#elif defined NL_OS_UNIX
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(DataSock->descriptor(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0)
		nlwarning("Failed to set SO_RCVTIMEO on UDP socket: %s", strerror(errno));
#endif
}



/*
 * Destructor
 */
CFEReceiveTask::~CFEReceiveTask()
{
	nlassert( DataSock != NULL );
	delete DataSock;
	DataSock = NULL;
}


/*
 * Run
 */
void CFEReceiveTask::run()
{
	uint maxrecvlength = _DatagramLength;
	while ( ! _ExitRequired )
	{
#ifndef SIMUL_CLIENTS

#ifdef MEASURE_RECEIVE_TASK
		static sint32 loopcount = 0;
		++loopcount;
		static TTime lastdisplay = CTime::getLocalTime();
		TTime tn = CTime::getLocalTime();
		uint32 diff = (uint32)(tn - lastdisplay);
		if ( diff > 2000 )
		{
			nlinfo("Reads by second: %.1f => LoopTime = %.2f ms LoopCount = %u Diff = %u ms",(float)loopcount * 1000.0f / (float)diff, (float)diff / loopcount, loopcount, diff);
			loopcount = 0;
			lastdisplay = tn;
		}

#endif
		// Check if the main thread has requested a socket rebind to recover from persistent failure
		if ( _RebindRequired )
		{
			rebindSocket();
			_RebindRequired = false;
		}

		try
		{
			// Receive into _ReceivedMessage
			_DatagramLength = maxrecvlength;
			_ReceivedMessage.resizeData( _DatagramLength );
			_ReceivedMessage.setTypeEvent( TReceivedMessage::User );
			DataSock->receivedFrom( _ReceivedMessage.userDataW(), _DatagramLength, _ReceivedMessage.AddrFrom );

			// Successfully received a packet — comms are working.
			// Update the last receive timestamp and clear any pending cookie.
			LastUDPPacketReceived = CTime::getSecondsSince1970();
			PendingCookieReceived = 0;
		}
		catch (const ESocket&)
		{
			// Originally, this catch block generated a RemoveClient event:
			//   _ReceivedMessage.setTypeEvent( TReceivedMessage::RemoveClient );
			//   _DatagramLength = 0;
			// That was intended to disconnect a client whose address caused the
			// error (e.g. ICMP "port unreachable"). However, on an unconnected
			// UDP socket (which serves all clients), recvfrom() errors don't
			// carry a valid source address — AddrFrom retains the stale value
			// from the previous successful receive. The RemoveClient event would
			// disconnect the wrong client. With SO_RCVTIMEO, the timeout
			// (EAGAIN) also triggers this path every second, which would flood
			// the queue with spurious disconnect events.
			//
			// Instead, we just continue the loop. The SO_RCVTIMEO ensures we
			// periodically return here to check for exit/rebind requests, and
			// the comms failure watchdog (PendingCookieReceived) handles the
			// case where the socket is persistently broken.
			continue;
		}

		// Check the size. Consider a big size as a hacked message
//		if ( _DatagramLength < 512 )
		{
			// Push into the write queue
			_ReceivedMessage.addressToVector();
			_ReceivedMessage.resizeData( _DatagramLength ); // _DatagramLength was modified by receivedFrom()
			{
				CSynchronized<CBufFIFO*>::CAccessor wq( &_WriteQueue );
				wq.value()->push( _ReceivedMessage.data() );
				wq.value()->push( _ReceivedMessage.VAddrFrom );
			}
		}
//		else
//		{
//			// Reject message
//			++_NbRejectedDatagrams;
//		}

#else
		nlSleep( 1000 );
#endif
	}

	nlinfo( "Exiting from front-end receive task" );
}


/*
 * Close and rebind the UDP socket to recover from persistent failure.
 * This resets any corrupted kernel socket state that may have accumulated
 * over months of runtime.
 */
void CFEReceiveTask::rebindSocket()
{
	nlwarning("Rebinding UDP socket on port %hu to recover from persistent failure", _BoundPort);

	// Close and delete the old socket
	DataSock->close();
	delete DataSock;

	// Create a new socket and bind to the same port
	DataSock = new CUdpSock( false );
	nlassert( DataSock );

	try
	{
		DataSock->bind( _BoundPort );
	}
	catch (const ESocket &e)
	{
		nlwarning("Failed to rebind UDP socket on port %hu: %s", _BoundPort, e.what());
		// Socket is now in a bad state, but we'll keep trying on next rebind request
		return;
	}

	setRecvTimeout();

	nlinfo("UDP socket successfully rebound on port %hu", _BoundPort);
}


/*
 * Set new write queue
 */
CBufFIFO *CFEReceiveTask::swapWriteQueue(CBufFIFO *writeQueue)
{
	CSynchronized<CBufFIFO *>::CAccessor wq(&_WriteQueue);
	CBufFIFO *previous = wq.value();
	wq.value() = writeQueue;
	return previous;
}

/* end of file */
