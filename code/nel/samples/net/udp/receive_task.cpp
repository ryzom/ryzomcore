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

#include "receive_task.h"


#ifdef NL_OS_WINDOWS
#	define NOMINMAX
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
 * Constructor
 */
CReceiveTask::CReceiveTask( uint16 port, uint32 msgsize ) :
	_DatagramLength( msgsize ),
	_ReceivedMessage(),
	_WriteQueue( "WriteQueue" ), // value unspecified
	_ExitRequired( false )
{
	// Socket
	DataSock = new CUdpSock( false );
	nlassert( DataSock );

	DataSock->bind( port );
}



/*
 * Destructor
 */
CReceiveTask::~CReceiveTask()
{
	nlassert( DataSock != NULL );
	delete DataSock;
	DataSock = NULL;

}


/*
 * Run
 */
void CReceiveTask::run()
{
	uint maxrecvlength = _DatagramLength;
	while ( ! _ExitRequired )
	{
		sint64 d;
		try
		{
			// Receive into _ReceivedMessage
			_DatagramLength = maxrecvlength;
			_ReceivedMessage.resizeData( _DatagramLength );
			_ReceivedMessage.setTypeEvent( TReceivedMessage::User );
			DataSock->receivedFrom( _ReceivedMessage.userDataW(), _DatagramLength, _ReceivedMessage.AddrFrom );
			d = CTime::getLocalTime ();
		}
		catch (const ESocket&)
		{
			// Remove the client corresponding to the address
			_ReceivedMessage.setTypeEvent( TReceivedMessage::RemoveClient );
			_DatagramLength = 0;
		}

		// Push into the write queue
		_ReceivedMessage.addressToVector();
		_ReceivedMessage.resizeData( _DatagramLength ); // _DatagramLength was modified by receivedFrom()
		_ReceivedMessage.setDate ();
		{
			CSynchronized<CBufFIFO*>::CAccessor wq( &_WriteQueue );
			wq.value()->push( _ReceivedMessage.data() );
			wq.value()->push( _ReceivedMessage.VAddrFrom );
		}
	}

	nlinfo( "Exiting from front-end receive task" );
}


/*
 * Set new write queue
 */
void CReceiveTask::setWriteQueue( CBufFIFO *writequeue )
{
	CSynchronized<CBufFIFO*>::CAccessor wq( &_WriteQueue );
	wq.value() = writequeue;
}

