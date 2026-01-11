// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdnet.h"

#include "nel/misc/hierarchical_timer.h"

#include "nel/net/buf_client.h"
#include "nel/misc/thread.h"
#include "nel/net/dummy_tcp_sock.h"
#include "nel/net/net_log.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#elif defined NL_OS_UNIX
#	include <netinet/in.h>
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace std;


namespace NLNET {


uint32 	NbClientReceiveTask = 0;


/***************************************************************************************************
 * User main thread (initialization)
 **************************************************************************************************/

/*
 * Constructor
 */
#ifdef NL_OS_UNIX
CBufClient::CBufClient( bool nodelay, bool replaymode, bool initPipeForDataAvailable ) :
	CBufNetBase( initPipeForDataAvailable ),
#else
CBufClient::CBufClient( bool nodelay, bool replaymode, bool ) :
	CBufNetBase(),
#endif
	_NoDelay( nodelay ),
	_PrevBytesDownloaded( 0 ),
	_PrevBytesUploaded( 0 ),
	_RecvTask( NULL ),
	_RecvThread( NULL )
	/*_PrevBytesReceived( 0 ),
	_PrevBytesSent( 0 )*/
{
	nlnettrace( "CBufClient::CBufClient" ); // don't define a global object

	if ( replaymode )
	{
		_BufSock = new CNonBlockingBufSock( new CDummyTcpSock(), CBufNetBase::DefaultMaxExpectedBlockSize );
	}
	else
	{

		_BufSock = new CNonBlockingBufSock( NULL, CBufNetBase::DefaultMaxExpectedBlockSize );
		_RecvTask = new CClientReceiveTask( this, _BufSock );
	}
}


/*
 * Connects to the specified host
 * Precond: not connected
 */
void CBufClient::connect( const CInetAddress& addr )
{
	nlnettrace( "CBufClient::connect" );
	nlassert( ! _BufSock->Sock->connected() );
	_BufSock->setMaxExpectedBlockSize( maxExpectedBlockSize() );
	_BufSock->connect( addr, _NoDelay, true );
	_BufSock->setNonBlocking(); // ADDED: non-blocking client connection
	_PrevBytesDownloaded = 0;
	_PrevBytesUploaded = 0;
	/*_PrevBytesReceived = 0;
	_PrevBytesSent = 0;*/

	// Allow reconnection
	if ( _RecvThread != NULL )
	{
		delete _RecvThread;
	}

	_RecvThread = IThread::create( _RecvTask, 1024*4*4 );
	_RecvThread->start();
}


/***************************************************************************************************
 * User main thread (running)
 **************************************************************************************************/

void CBufClient::displayThreadStat (NLMISC::CLog *log)
{
	log->displayNL ("client thread %p nbloop %d", _RecvTask, _RecvTask->NbLoop);
}


/*
 * Sends a message to the remote host
 */
void CBufClient::send( const NLMISC::CMemStream& buffer )
{
	nlnettrace( "CBufClient::send" );
	nlassert( buffer.length() > 0 );
	nlassert( buffer.length() <= maxSentBlockSize() );

	// slow down the layer H_AUTO (CBufServer_send);

	if ( ! _BufSock->pushBuffer( buffer ) )
	{
		// Disconnection event if disconnected
		_BufSock->advertiseDisconnection( this, NULL );
	}
}


/*
 * Checks if there are some data to receive
 */
bool CBufClient::dataAvailable()
{
	// slow down the layer H_AUTO (CBufClient_dataAvailable);
	{
		/* If no data available, enter the 'while' loop and return false (1 volatile test)
		 * If there are user data available, enter the 'while' and return true immediately (1 volatile test + 1 short locking)
		 * If there is a disconnection event (rare), call the callback and loop
		 */
		while ( dataAvailableFlag() )
		{
			// Because _DataAvailable is true, the receive queue is not empty at this point
			uint8 val;
			{
				CFifoAccessor recvfifo( &receiveQueue() );
				val = recvfifo.value().frontLast ();
			}

#ifdef NL_OS_UNIX
			uint8 b;
			if ( read( _DataAvailablePipeHandle[PipeRead], &b, 1 ) == -1 )
				nlwarning( "LNETL1: Read pipe failed in dataAvailable" );
			//nldebug( "Pipe: 1 byte read (client %p)", this );
#endif

			// Test if it the next block is a system event
			switch ( val )
			{

			// Normal message available
			case CBufNetBase::User:
				return true; // return immediatly, do not extract the message

			// Process disconnection event
			case CBufNetBase::Disconnection:

				LNETL1_DEBUG( "LNETL1: Disconnection event" );
				_BufSock->setConnectedState( false );

				// Call callback if needed
				if ( disconnectionCallback() != NULL )
				{
					disconnectionCallback()( id(), argOfDisconnectionCallback() );
				}

				// Unlike the server version, we do not delete the CBufSock object here,
				// it will be done in the destructor of CBufClient

				break;

			default: // should not occur
				{
					CFifoAccessor recvfifo( &receiveQueue() );
					vector<uint8> buffer;
					recvfifo.value().front (buffer);
					LNETL1_INFO( "LNETL1: Invalid block type: %hu (should be = %hu)", (uint16)(buffer[buffer.size()-1]), (uint16)val );
					LNETL1_INFO( "LNETL1: Buffer (%d B): [%s]", buffer.size(), stringFromVector(buffer).c_str() );
					LNETL1_INFO( "LNETL1: Receive queue:" );
					recvfifo.value().display();
					nlerror( "LNETL1: Invalid system event type in client receive queue" );
				}
			}
			// Extract system event
			{
				CFifoAccessor recvfifo( &receiveQueue() );
				recvfifo.value().pop();
				setDataAvailableFlag( ! recvfifo.value().empty() );
			}

		}
		// _DataAvailable is false here
		return false;
	}
}


#ifdef NL_OS_UNIX
/* Wait until the receive queue contains something to read (implemented with a select()).
 * This is where the connection/disconnection callbacks can be called.
 * \param usecMax Max time to wait in microsecond (up to 1 sec)
 */
void	CBufClient::sleepUntilDataAvailable( uint usecMax )
{
	// Prevent looping infinitely if the system time was changed
	if ( usecMax > 999999 ) // limit not told in Linux man but here: http://docs.hp.com/en/B9106-90009/select.2.html
		usecMax = 999999;

	fd_set readers;
	timeval tv;
	do
	{
		FD_ZERO( &readers );
		FD_SET( _DataAvailablePipeHandle[PipeRead], &readers );
		tv.tv_sec = 0;
		tv.tv_usec = usecMax;
		int res = ::select( _DataAvailablePipeHandle[PipeRead]+1, &readers, NULL, NULL, &tv );
		if ( res == -1 )
			nlerror( "LNETL1: Select failed in sleepUntilDataAvailable (code %u)", CSock::getLastError() );
	}
	while ( ! dataAvailable() ); // will loop if only a connection/disconnection event was read
}
#endif


/*
 * Receives next block of data in the specified buffer (resizes the vector)
 * Precond: dataAvailable() has returned true
 */
void CBufClient::receive( NLMISC::CMemStream& buffer )
{
	nlnettrace( "CBufClient::receive" );
	//nlassert( dataAvailable() );

	// Extract buffer from the receive queue
	{
		CFifoAccessor recvfifo( &receiveQueue() );
		nlassert( ! recvfifo.value().empty() );
		recvfifo.value().front( buffer );
		recvfifo.value().pop();
		setDataAvailableFlag( ! recvfifo.value().empty() );
	}

	// Extract event type
	nlassert( buffer.buffer()[buffer.size()-1] == CBufNetBase::User );
	//commented for optimisation LNETL1_DEBUG( "LNETL1: Client read buffer (%d+%d B)", buffer.size(), sizeof(TSockId)+1 );
	buffer.resize( buffer.size()-1 );
}


/*
 * Update the network (call this method evenly)
 */
void CBufClient::update()
{
	//nlnettrace( "CBufClient::update" );

	// Update sending
	bool sendingok = _BufSock->update();

	// Disconnection event if disconnected
	if ( ! ( _BufSock->Sock->connected() && sendingok ) )
	{
		if ( _BufSock->Sock->connected() )
		{
			_BufSock->Sock->disconnect();
		}
		_BufSock->advertiseDisconnection( this, NULL );
	}
}


/*
 * Disconnect the remote host
 */
void CBufClient::disconnect( bool quick )
{
	nlnettrace( "CBufClient::disconnect" );

	// Do not allow to disconnect a socket that is not connected
	nlassert( _BufSock->connectedState() );

	// When the NS tells us to remove this connection AND the connection has physically
	// disconnected but not yet logically (i.e. disconnection event not processed yet),
	// skip flushing and physical active disconnection
	if ( _BufSock->Sock->connected() )
	{
		// Flush sending is asked for
		if ( ! quick )
		{
			_BufSock->flush();
		}

		// Disconnect and prevent from advertising the disconnection
		_BufSock->disconnect( false );
	}

	// Empty the receive queue
	{
		CFifoAccessor recvfifo( &receiveQueue() );
		recvfifo.value().clear();
		setDataAvailableFlag( false );
	}
}


// Utility function for newBytes...()
inline uint64 updateStatCounter( uint64& counter, uint64 newvalue )
{
	uint64 result = newvalue - counter;
	counter = newvalue;
	return result;
}


/*
 * Returns the number of bytes downloaded since the previous call to this method
 */
uint64 CBufClient::newBytesDownloaded()
{
	return updateStatCounter( _PrevBytesDownloaded, bytesDownloaded() );
}


/*
 * Returns the number of bytes uploaded since the previous call to this method
 */
uint64 CBufClient::newBytesUploaded()
{
	return updateStatCounter( _PrevBytesUploaded, bytesUploaded() );
}


/*
 * Returns the number of bytes popped by receive() since the previous call to this method
 */
/*uint64 CBufClient::newBytesReceived()
{
	return updateStatCounter( _PrevBytesReceived, bytesReceived() );
}*/


/*
 * Returns the number of bytes pushed by send() since the previous call to this method
 */
/*uint64 CBufClient::newBytesSent()
{
	return updateStatCounter( _PrevBytesSent, bytesSent() );
}*/


/*
 * Destructor
 */
CBufClient::~CBufClient()
{
	nlnettrace( "CBufClient::~CBufClient" );

	// Disconnect if not done
	if ( _BufSock->Sock->connected() )
	{
		nlassert( _BufSock->connectedState() );

		disconnect( true );
	}

	// Clean thread termination
	if ( _RecvThread != NULL )
	{
		LNETL1_DEBUG( "LNETL1: Waiting for the end of the receive thread..." );
		_RecvThread->wait();
	}

	if ( _RecvTask != NULL )
		delete _RecvTask;

	if ( _RecvThread != NULL )
		delete _RecvThread;

	if ( _BufSock != NULL )
		delete _BufSock;

	nlnettrace( "Exiting CBufClient::~CBufClient" );
}


/***************************************************************************************************
 * Receive thread
 **************************************************************************************************/


/*
 * Code of receiving thread for clients
 */
void CClientReceiveTask::run()
{
	NbClientReceiveTask++;
	NbNetworkTask++;
	nlnettrace( "CClientReceiveTask::run" );

	// 18/08/2005 : sonix : Changed time out from 60s to 1s, in some case, it
	//						can generate a 60 s wait on destruction of the CBufSock
	//						By the way, checking every 1s is not a time consuming
	_NBBufSock->Sock->setTimeOutValue( 1, 0 );

	bool connected = true;
	while ( connected && _NBBufSock->Sock->connected())
	{
		try
		{
			// ADDED: non-blocking client connection

			// Wait until some data are received (sleepin' select inside)
			while ( ! _NBBufSock->Sock->dataAvailable() )
			{
				if ( ! _NBBufSock->Sock->connected() )
				{
					LNETL1_DEBUG( "LNETL1: Client connection %s closed", sockId()->asString().c_str() );
					// The socket went to _Connected=false when throwing the exception
					connected = false;
					break;
				}
			}

			// Process the data received
			if ( _NBBufSock->receivePart( 1 ) ) // 1 for the event type
			{
				//commented out for optimisation: LNETL1_DEBUG( "LNETL1: Client %s received buffer (%u bytes)", _SockId->asString().c_str(), buffer.size()/*, stringFromVector(buffer).c_str()*/ );
				// Add event type
				_NBBufSock->fillEventTypeOnly();

				// Push message into receive queue
				_Client->pushMessageIntoReceiveQueue( _NBBufSock->receivedBuffer() );
			}

			NbLoop++;
		}
		catch (const ESocket&)
		{
			LNETL1_DEBUG( "LNETL1: Client connection %s broken", sockId()->asString().c_str() );
			sockId()->Sock->disconnect();
			connected = false;
		}
	}

	nlnettrace( "Exiting CClientReceiveTask::run()" );
	NbClientReceiveTask--;
	NbNetworkTask--;
}

NLMISC_CATEGORISED_VARIABLE(nel, uint32, NbClientReceiveTask, "Number of client receive thread");



} // NLNET
