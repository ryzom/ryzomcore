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

#include "fe_receive_task.h"
#include "fe_types.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
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

/*
 * TReceivedMessage
 */

/// Constructor
TReceivedMessage::TReceivedMessage()
{
	VAddrFrom.resize( sizeof(sockaddr_in) );
}

/// Return a vector containing the address info
void	TReceivedMessage::addressToVector()
{
	memcpy( &*VAddrFrom.begin(), AddrFrom.sockAddr(), sizeof(sockaddr_in) );
}

/// Set address with address info from specified vector
void	TReceivedMessage::vectorToAddress()
{
	AddrFrom.setSockAddr( (sockaddr_in*)&*VAddrFrom.begin() );
}


/*
 * Constructor (note: called from the main thread)
 */
CFEReceiveTask::CFEReceiveTask( uint16 firstAcceptablePort, uint16 lastAcceptablePort, uint32 msgsize ) :
	_ReceivedMessage(),
	_WriteQueue( "WriteQueue" ), // value unspecified
	_DatagramLength( msgsize ),
	_ExitRequired( false ),
	_NbRejectedDatagrams( 0 )
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
	nlinfo( "Binding all network interfaces on port %hu (%hu asked)", actualPort, firstAcceptablePort );
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
		try
		{
			// Receive into _ReceivedMessage
			_DatagramLength = maxrecvlength;
			_ReceivedMessage.resizeData( _DatagramLength );
			_ReceivedMessage.setTypeEvent( TReceivedMessage::User );
			DataSock->receivedFrom( _ReceivedMessage.userDataW(), _DatagramLength, _ReceivedMessage.AddrFrom );
		}
		catch (const ESocket&)
		{
			// Remove the client corresponding to the address
			_ReceivedMessage.setTypeEvent( TReceivedMessage::RemoveClient );
			_DatagramLength = 0;
		}
		
		// update the last datagram receive date
		LastUDPPacketReceived = CTime::getSecondsSince1970();

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
 * Set new write queue
 */
void CFEReceiveTask::setWriteQueue( CBufFIFO *writequeue )
{
	CSynchronized<CBufFIFO*>::CAccessor wq( &_WriteQueue );
	wq.value() = writequeue;
}

