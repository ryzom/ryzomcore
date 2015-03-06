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

#include "stdnet.h"

#include "nel/misc/hierarchical_timer.h"

#include "nel/net/buf_server.h"
#include "nel/net/net_log.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#elif defined NL_OS_UNIX
#	include <unistd.h>
#	include <cerrno>
#	include <sys/types.h>
#	include <sys/time.h>
#endif

/*
 * On Linux, the default limit of descriptors is usually 1024, you can increase it with ulimit
 */

using namespace NLMISC;
using namespace std;

namespace NLNET {

uint32 	NbServerListenTask = 0;
uint32 	NbServerReceiveTask = 0;

/***************************************************************************************************
 * User main thread (initialization)
 **************************************************************************************************/

/*
 * Constructor
 */
CBufServer::CBufServer( TThreadStategy strategy,
	uint16 max_threads, uint16 max_sockets_per_thread, bool nodelay, bool replaymode, bool initPipeForDataAvailable ) :
#ifdef NL_OS_UNIX
	CBufNetBase( initPipeForDataAvailable ),
#else
	CBufNetBase(),
#endif
	_ThreadStrategy( strategy ),
	_MaxThreads( max_threads ),
	_MaxSocketsPerThread( max_sockets_per_thread ),
	_ListenTask( NULL ),
	_ListenThread( NULL ),
	_ThreadPool("CBufServer::_ThreadPool"),
	_ConnectionCallback( NULL ),
	_ConnectionCbArg( NULL ),
	_BytesPushedOut( 0 ),
	_BytesPoppedIn( 0 ),
	_PrevBytesPoppedIn( 0 ),
	_PrevBytesPushedOut( 0 ),
	_NbConnections (0),
	_NoDelay( nodelay ),
	_ReplayMode( replaymode )
{
	nlnettrace( "CBufServer::CBufServer" );
	if ( ! _ReplayMode )
	{
		_ListenTask = new CListenTask( this );
		_ListenThread = IThread::create( _ListenTask, 1024*4*4 );
	}
	/*{
		CSynchronized<uint32>::CAccessor syncbpi ( &_BytesPushedIn );
		syncbpi.value() = 0;
	}*/
}


/*
 * Listens on the specified port
 */
void CBufServer::init( uint16 port )
{
	nlnettrace( "CBufServer::init" );
	if ( ! _ReplayMode )
	{
		_ListenTask->init( port, maxExpectedBlockSize() );
		_ListenThread->start();
	}
	else
	{
		LNETL1_DEBUG( "LNETL1: Binding listen socket to any address, port %hu", port );
	}
}


/*
 * Begins to listen on the specified port (call before running thread)
 */
void CListenTask::init( uint16 port, sint32 maxExpectedBlockSize )
{
	nlnettrace( "CListenTask::init" );
	_ListenSock.init( port );
	_MaxExpectedBlockSize = maxExpectedBlockSize;
}


/***************************************************************************************************
 * User main thread (running)
 **************************************************************************************************/


/*
 * Constructor
 */
CServerTask::CServerTask() : NbLoop (0), _ExitRequired(false)
{
#ifdef NL_OS_UNIX
	if (pipe( _WakeUpPipeHandle ) == -1)
	{
		nlwarning("LNETL1: pipe() failed: code=%d '%s'", errno, strerror(errno));
	}
#endif
}



#ifdef NL_OS_UNIX
/*
 * Wake the thread up, when blocked in select (Unix only)
 */
void CServerTask::wakeUp()
{
	uint8 b;
	if ( write( _WakeUpPipeHandle[PipeWrite], &b, 1 ) == -1 )
	{
		LNETL1_DEBUG( "LNETL1: In CServerTask::wakeUp(): write() failed" );
	}
}
#endif


/*
 * Destructor
 */
CServerTask::~CServerTask()
{
#ifdef NL_OS_UNIX
	close( _WakeUpPipeHandle[PipeRead] );
	close( _WakeUpPipeHandle[PipeWrite] );
#endif
}


/*
 * Destructor
 */
CBufServer::~CBufServer()
{
	nlnettrace( "CBufServer::~CBufServer" );

	// Clean listen thread exit
	if ( ! _ReplayMode )
	{
		((CListenTask*)(_ListenThread->getRunnable()))->requireExit();
		((CListenTask*)(_ListenThread->getRunnable()))->close();
#ifdef NL_OS_UNIX
		_ListenTask->wakeUp();
#endif
		_ListenThread->wait();
		delete _ListenThread;
		delete _ListenTask;

		// Clean receive thread exits
		CThreadPool::iterator ipt;
		{
			LNETL1_DEBUG( "LNETL1: Waiting for end of threads..." );
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// Tell the threads to exit and wake them up
				CServerReceiveTask *task = receiveTask(ipt);
				nlnettrace( "Requiring exit" );
				task->requireExit();

				// Wake the threads up
	#ifdef NL_OS_UNIX
				task->wakeUp();
	#else
				CConnections::iterator ipb;
				nlnettrace( "Disconnecting sockets (Win32)" );
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						(*ipb)->Sock->disconnect();
					}
				}
	#endif

			}

			nlnettrace( "Waiting" );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// Wait until the threads have exited
				(*ipt)->wait();
			}

			LNETL1_DEBUG( "LNETL1: Deleting sockets, tasks and threads..." );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// Delete the socket objects
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						delete (*ipb); // closes and deletes the socket
					}
				}

				// Delete the task objects
				delete task;

				// Delete the thread objects
				delete (*ipt);
			}
		}
	}

	nlnettrace( "Exiting CBufServer::~CBufServer" );
}


/*
 * Disconnect the specified host
 * Set hostid to NULL to disconnect all connections.
 * If hostid is not null and the socket is not connected, the method does nothing.
 * If quick is true, any pending data will not be sent before disconnecting.
 */
void CBufServer::disconnect( TSockId hostid, bool quick )
{
	nlnettrace( "CBufServer::disconnect" );
	if ( hostid != InvalidSockId )
	{
		if (_ConnectedClients.find(hostid) == _ConnectedClients.end())
		{
			// this host is not connected
			return;
		}

		// Disconnect only if physically connected
		if ( hostid->Sock->connected() )
		{
			if ( ! quick )
			{
				hostid->flush();
			}
			hostid->Sock->disconnect(); // the connection will be removed by the next call of update()
		}
	}
	else
	{
		// Disconnect all
		CThreadPool::iterator ipt;
		{
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						if ( (*ipb)->Sock->connected() )
						{
							if ( ! quick )
							{
								(*ipb)->flush();
							}
							(*ipb)->Sock->disconnect();
						}
					}
				}
			}
		}
	}
}


/*
 * Send a message to the specified host
 */
void CBufServer::send( const CMemStream& buffer, TSockId hostid )
{
	nlnettrace( "CBufServer::send" );
	nlassert( buffer.length() > 0 );
	nlassertex( buffer.length() <= maxSentBlockSize(), ("length=%u max=%u", buffer.length(), maxSentBlockSize()) );

	// slow down the layer H_AUTO (CBufServer_send);

	if ( hostid != InvalidSockId )
	{
		if (_ConnectedClients.find(hostid) == _ConnectedClients.end())
		{
			// this host is not connected
			return;
		}

		pushBufferToHost( buffer, hostid );
	}
	else
	{
		// Push into all send queues
		CThreadPool::iterator ipt;
		{
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						// Send only if the socket is logically connected
						if ( (*ipb)->connectedState() )
						{
							pushBufferToHost( buffer, *ipb );
						}
					}
				}
			}
		}
	}
}


/*
 * Checks if there are some data to receive
 */
bool CBufServer::dataAvailable()
{
	// slow down the layer H_AUTO (CBufServer_dataAvailable);
	{
		/* If no data available, enter the 'while' loop and return false (1 volatile test)
		 * If there are user data available, enter the 'while' and return true immediately (1 volatile test + 1 short locking)
		 * If there is a connection/disconnection event (rare), call the callback and loop
		 */
		while ( dataAvailableFlag() )
		{
			// Because _DataAvailable is true, the receive queue is not empty at this point
			vector<uint8> buffer;
			uint8 val;
			{
				CFifoAccessor recvfifo( &receiveQueue() );
				val = recvfifo.value().frontLast();
				if ( val != CBufNetBase::User )
				{
					recvfifo.value().front( buffer );
				}
			}

			/*sint32 mbsize = recvfifo.value().size() / 1048576;
			if ( mbsize > 0 )
			{
			  nlwarning( "The receive queue size exceeds %d MB", mbsize );
			}*/

			/*vector<uint8> buffer;
			recvfifo.value().front( buffer );*/

#ifdef NL_OS_UNIX
			uint8 b;
			if ( read( _DataAvailablePipeHandle[PipeRead], &b, 1 ) == -1 )
				nlwarning( "LNETL1: Read pipe failed in dataAvailable" );
			//nldebug( "Pipe: 1 byte read (server %p)", this );
#endif

			// Test if it the next block is a system event
			//switch ( buffer[buffer.size()-1] )
			switch ( val )
			{

			// Normal message available
			case CBufNetBase::User:
				{
					return true; // return immediately, do not extract the message
				}

			// Process disconnection event
			case CBufNetBase::Disconnection:
				{
					TSockId sockid = *((TSockId*)(&*buffer.begin()));
					LNETL1_DEBUG( "LNETL1: Disconnection event for %p %s", sockid, sockid->asString().c_str());

					sockid->setConnectedState( false );

					// Call callback if needed
					if ( disconnectionCallback() != NULL )
					{
						disconnectionCallback()( sockid, argOfDisconnectionCallback() );
					}

					// remove from the list of valid client
					nlverify(_ConnectedClients.erase(sockid) == 1);

					// Add socket object into the synchronized remove list
					LNETL1_DEBUG( "LNETL1: Adding the connection to the remove list" );
					nlassert( ((CServerBufSock*)sockid)->ownerTask() != NULL );
					((CServerBufSock*)sockid)->ownerTask()->addToRemoveSet( sockid );
					break;
				}
			// Process connection event
			case CBufNetBase::Connection:
				{
					TSockId sockid = *((TSockId*)(&*buffer.begin()));
					LNETL1_DEBUG( "LNETL1: Connection event for %p %s", sockid, sockid->asString().c_str());

					// add this socket in the list of client
					nlverify(_ConnectedClients.insert(sockid).second);

					sockid->setConnectedState( true );

					// Call callback if needed
					if ( connectionCallback() != NULL )
					{
						connectionCallback()( sockid, argOfConnectionCallback() );
					}
					break;
				}
			default: // should not occur
				LNETL1_INFO( "LNETL1: Invalid block type: %hu (should be = to %hu", (uint16)(buffer[buffer.size()-1]), (uint16)(val) );
				LNETL1_INFO( "LNETL1: Buffer (%d B): [%s]", buffer.size(), stringFromVector(buffer).c_str() );
				LNETL1_INFO( "LNETL1: Receive queue:" );
				{
					CFifoAccessor recvfifo( &receiveQueue() );
					recvfifo.value().display();
				}
				nlerror( "LNETL1: Invalid system event type in server receive queue" );

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
void	CBufServer::sleepUntilDataAvailable( uint usecMax )
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
 * Receives next block of data in the specified. The length and hostid are output arguments.
 * Precond: dataAvailable() has returned true, phostid not null
 */
void CBufServer::receive( CMemStream& buffer, TSockId* phostid )
{
	nlnettrace( "CBufServer::receive" );
	//nlassert( dataAvailable() );
	nlassert( phostid != NULL );

	{
		CFifoAccessor recvfifo( &receiveQueue() );
		nlassert( ! recvfifo.value().empty() );
		recvfifo.value().front( buffer );
		recvfifo.value().pop();
		setDataAvailableFlag( ! recvfifo.value().empty() );
	}

	// Extract hostid (and event type)
	*phostid = *((TSockId*)&(buffer.buffer()[buffer.size()-sizeof(TSockId)-1]));
	nlassert( buffer.buffer()[buffer.size()-1] == CBufNetBase::User );

	buffer.resize( buffer.size()-sizeof(TSockId)-1 );

	// TODO OPTIM remove the nldebug for speed
	//commented for optimisation LNETL1_DEBUG( "LNETL1: Read buffer (%d+%d B) from %s", buffer.size(), sizeof(TSockId)+1, /*stringFromVector(buffer).c_str(), */(*phostid)->asString().c_str() );

	// Statistics
	_BytesPoppedIn += buffer.size() + sizeof(TBlockSize);
}


/*
 * Update the network (call this method evenly)
 */
void CBufServer::update()
{
	//nlnettrace( "CBufServer::update-BEGIN" );

	_NbConnections = 0;

	// For each thread
	CThreadPool::iterator ipt;
	{
	  //nldebug( "UPD: Acquiring the Thread Pool" );
		CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
		//nldebug( "UPD: Acquired." );
		for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
		{
			// For each thread of the pool
			CServerReceiveTask *task = receiveTask(ipt);
			CConnections::iterator ipb;
			{
				CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
				for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
				{
				    // For each socket of the thread, update sending
				    if ( ! ((*ipb)->Sock->connected() && (*ipb)->update()) )
				    {
						// Update did not work or the socket is not connected anymore
				        LNETL1_DEBUG( "LNETL1: Socket %s is disconnected", (*ipb)->asString().c_str() );
						// Disconnection event if disconnected (known either from flush (in update) or when receiving data)
						(*ipb)->advertiseDisconnection( this, *ipb );

						/*if ( (*ipb)->advertiseDisconnection( this, *ipb ) )
						{
							// Now the connection removal is in dataAvailable()
							// POLL6
						}*/
				    }
				    else
				    {
						_NbConnections++;
				    }
				}
			}
		}
	}

	//nlnettrace( "CBufServer::update-END" );
}

uint32 CBufServer::getSendQueueSize( TSockId destid )
{
	if ( destid != InvalidSockId )
	{
		if (_ConnectedClients.find(destid) == _ConnectedClients.end())
		{
			// this host is not connected
			return 0;
		}

		return destid->SendFifo.size();
	}
	else
	{
		// add all client buffers

		uint32 total = 0;

		// For each thread
		CThreadPool::iterator ipt;
		{
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// For each thread of the pool
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						// For each socket of the thread, update sending
						total = (*ipb)->SendFifo.size ();
					}
				}
			}
		}
		return total;
	}
}

void CBufServer::displayThreadStat (NLMISC::CLog *log)
{
	// For each thread
	CThreadPool::iterator ipt;
	{
		CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
		for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
		{
			// For each thread of the pool
			CServerReceiveTask *task = receiveTask(ipt);
			// For each socket of the thread, update sending
			log->displayNL ("server receive thread %p nbloop %d", task, task->NbLoop);
		}
	}

	log->displayNL ("server listen thread %p nbloop %d", _ListenTask, _ListenTask->NbLoop);
}

void CBufServer::setTimeFlushTrigger( TSockId destid, sint32 ms )
{
	nlassert( destid != InvalidSockId );
	if (_ConnectedClients.find(destid) != _ConnectedClients.end())
		destid->setTimeFlushTrigger( ms );
}

void CBufServer::setSizeFlushTrigger( TSockId destid, sint32 size )
{
	nlassert( destid != InvalidSockId );
	if (_ConnectedClients.find(destid) != _ConnectedClients.end())
		destid->setSizeFlushTrigger( size );
}

bool CBufServer::flush( TSockId destid, uint *nbBytesRemaining)
{
	nlassert( destid != InvalidSockId );
	if (_ConnectedClients.find(destid) != _ConnectedClients.end())
		return destid->flush( nbBytesRemaining );
	else
		return true;
}
const CInetAddress& CBufServer::hostAddress( TSockId hostid )
{
	nlassert( hostid != InvalidSockId );
	if (_ConnectedClients.find(hostid) != _ConnectedClients.end())
		return hostid->Sock->remoteAddr();

	static CInetAddress nullAddr;
	return nullAddr;
}

void CBufServer::displaySendQueueStat (NLMISC::CLog *log, TSockId destid)
{
	if ( destid != InvalidSockId )
	{
		if (_ConnectedClients.find(destid) == _ConnectedClients.end())
		{
			// this host is not connected
			return;
		}

		destid->SendFifo.displayStats(log);
	}
	else
	{
		// add all client buffers

		// For each thread
		CThreadPool::iterator ipt;
		{
			CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
			for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
			{
				// For each thread of the pool
				CServerReceiveTask *task = receiveTask(ipt);
				CConnections::iterator ipb;
				{
					CSynchronized<CConnections>::CAccessor connectionssync( &task->_Connections );
					for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
					{
						// For each socket of the thread, update sending
						(*ipb)->SendFifo.displayStats(log);
					}
				}
			}
		}
	}
}


/*
 * Returns the number of bytes received since the previous call to this method
 */
uint64 CBufServer::newBytesReceived()
{
	uint64 b = bytesReceived();
	uint64 nbrecvd = b - _PrevBytesPoppedIn;
	//nlinfo( "b: %"NL_I64"u   new: %"NL_I64"u", b, nbrecvd );
	_PrevBytesPoppedIn = b;
	return nbrecvd;
}

/*
 * Returns the number of bytes sent since the previous call to this method
 */
uint64 CBufServer::newBytesSent()
{
	uint64 b = bytesSent();
	uint64 nbsent = b - _PrevBytesPushedOut;
	//nlinfo( "b: %"NL_I64"u   new: %"NL_I64"u", b, nbsent );
	_PrevBytesPushedOut = b;
	return nbsent;
}


/***************************************************************************************************
 * Listen thread
 **************************************************************************************************/


/*
 * Code of listening thread
 */
void CListenTask::run()
{
	NbNetworkTask++;
	NbServerListenTask++;

	nlnettrace( "CListenTask::run" );

	fd_set readers;
#ifdef NL_OS_UNIX
	SOCKET descmax;
	descmax = _ListenSock.descriptor()>_WakeUpPipeHandle[PipeRead]?_ListenSock.descriptor():_WakeUpPipeHandle[PipeRead];
#endif

	// Accept connections
	while ( ! exitRequired() )
	{
		try
		{
			LNETL1_DEBUG( "LNETL1: Waiting incoming connection..." );
			// Get and setup the new socket
#ifdef NL_OS_UNIX
			FD_ZERO( &readers );
			FD_SET( _ListenSock.descriptor(), &readers );
			FD_SET( _WakeUpPipeHandle[PipeRead], &readers );
			int res = ::select( descmax+1, &readers, NULL, NULL, NULL ); /// Wait indefinitely

			switch ( res )
			{
			//case  0 : continue; // time-out expired, no results
			case -1 :
				// we'll ignore message (Interrupted system call) caused by a CTRL-C
				if (CSock::getLastError() == 4)
				{
					LNETL1_DEBUG ("LNETL1: Select failed (in listen thread): %s (code %u) but IGNORED", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError());
					continue;
				}
				nlerror( "LNETL1: Select failed (in listen thread): %s (code %u)", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError() );
			}

			if ( FD_ISSET( _WakeUpPipeHandle[PipeRead], &readers ) )
			{
				uint8 b;
				if ( read( _WakeUpPipeHandle[PipeRead], &b, 1 ) == -1 ) // we were woken-up by the wake-up pipe
				{
					LNETL1_DEBUG( "LNETL1: In CListenTask::run(): read() failed" );
				}
				LNETL1_DEBUG( "LNETL1: listen thread select woken-up" );
				continue;
			}
#elif defined (NL_OS_WINDOWS)
			FD_ZERO( &readers );
			FD_SET( _ListenSock.descriptor(), &readers );
			int res = ::select( 1, &readers, NULL, NULL, NULL ); /// Wait indefinitely

			if ( res == -1)
			{
				nlerror( "LNETL1: Select failed (in listen thread): %s (code %u)", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError() );
				continue;
			}
#endif
			LNETL1_DEBUG( "LNETL1: Accepting an incoming connection..." );
			CTcpSock *newSock = _ListenSock.accept();
			if (newSock != NULL)
			{
				CServerBufSock *bufsock = new CServerBufSock( newSock );
				LNETL1_DEBUG( "LNETL1: New connection : %s", bufsock->asString().c_str() );
				bufsock->setNonBlocking();
				bufsock->setMaxExpectedBlockSize( _MaxExpectedBlockSize );
				if ( _Server->noDelay() )
				{
					bufsock->Sock->setNoDelay( true );
				}

				// Notify the new connection
				bufsock->advertiseConnection( _Server );

				// Dispatch the socket into the thread pool
				_Server->dispatchNewSocket( bufsock );
			}

			NbLoop++;
		}
		catch (const ESocket &e)
		{
			LNETL1_INFO( "LNETL1: Exception in listen thread: %s", e.what() );
			// It can occur when too many sockets are open (e.g. 885 connections)
		}
	}

	nlnettrace( "Exiting CListenTask::run" );
	NbServerListenTask--;
	NbNetworkTask--;
}

/// Close listening socket
void CListenTask::close()
{
	_ListenSock.close();
//	_ListenSock.disconnect();
}


/*
 * Binds a new socket and send buffer to an existing or a new thread
 * Note: this method is called in the listening thread.
 */
void CBufServer::dispatchNewSocket( CServerBufSock *bufsock )
{
	nlnettrace( "CBufServer::dispatchNewSocket" );

	CSynchronized<CThreadPool>::CAccessor poolsync( &_ThreadPool );
	if ( _ThreadStrategy == SpreadSockets )
	{
		// Find the thread with the smallest number of connections and check if all
		// threads do not have the same number of connections
		uint min = 0xFFFFFFFF;
		uint max = 0;
		CThreadPool::iterator ipt, iptmin, iptmax;
		for ( iptmin=iptmax=ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
		{
			uint noc = receiveTask(ipt)->numberOfConnections();
			if ( noc < min )
			{
				min = noc;
				iptmin = ipt;
			}
			if ( noc > max )
			{
				max = noc;
				iptmax = ipt;
			}
		}

		// Check if we make the pool of threads grow (if we have not found vacant room
		// and if it is allowed to)
		if ( (poolsync.value().empty()) ||
			 ((min == max) && (poolsync.value().size() < _MaxThreads)) )
		{
			addNewThread( poolsync.value(), bufsock );
		}
		else
		{
			// Dispatch socket to an existing thread of the pool
			CServerReceiveTask *task = receiveTask(iptmin);
			bufsock->setOwnerTask( task );
			task->addNewSocket( bufsock );
#ifdef NL_OS_UNIX
			task->wakeUp();
#endif

			if ( min >= (uint)_MaxSocketsPerThread )
			{
				nlwarning( "LNETL1: Exceeding the maximum number of sockets per thread" );
			}
			LNETL1_DEBUG( "LNETL1: New socket dispatched to thread %d", iptmin-poolsync.value().begin() );
		}

	}
	else // _ThreadStrategy == FillThreads
	{
		CThreadPool::iterator ipt;
		for ( ipt=poolsync.value().begin(); ipt!=poolsync.value().end(); ++ipt )
		{
			uint noc = receiveTask(ipt)->numberOfConnections();
			if ( noc < _MaxSocketsPerThread )
			{
				break;
			}
		}

		// Check if we have to make the thread pool grow (if we have not found vacant room)
		if ( ipt == poolsync.value().end() )
		{
			if ( poolsync.value().size() == _MaxThreads )
			{
				nlwarning( "LNETL1: Exceeding the maximum number of threads" );
			}
			addNewThread( poolsync.value(), bufsock );
		}
		else
		{
			// Dispatch socket to an existing thread of the pool
			CServerReceiveTask *task = receiveTask(ipt);
			bufsock->setOwnerTask( task );
			task->addNewSocket( bufsock );
#ifdef NL_OS_UNIX
			task->wakeUp();
#endif
			LNETL1_DEBUG( "LNETL1: New socket dispatched to thread %d", ipt-poolsync.value().begin() );
		}
	}
}


/*
 * Creates a new task and run a new thread for it
 * Precond: bufsock not null
 */
void CBufServer::addNewThread( CThreadPool& threadpool, CServerBufSock *bufsock )
{
	nlnettrace( "CBufServer::addNewThread" );
	nlassert( bufsock != NULL );

	// Create new task and dispatch the socket to it
	CServerReceiveTask *task = new CServerReceiveTask( this );
	bufsock->setOwnerTask( task );
	task->addNewSocket( bufsock );

	// Add a new thread to the pool, with this task
	IThread *thr = IThread::create( task, 1024*4*4 );
	{
		threadpool.push_back( thr );
		thr->start();
		LNETL1_DEBUG( "LNETL1: Added a new thread; pool size is %d", threadpool.size() );
		LNETL1_DEBUG( "LNETL1: New socket dispatched to thread %d", threadpool.size()-1 );
	}
}


/***************************************************************************************************
 * Receive threads
 **************************************************************************************************/


/*
 * Code of receiving threads for servers
 */
void CServerReceiveTask::run()
{
	NbNetworkTask++;
	NbServerReceiveTask++;
	nlnettrace( "CServerReceiveTask::run" );

	SOCKET descmax;
	fd_set readers;

#if defined NL_OS_UNIX
	// POLL7
	if (nice( 2 ) == -1) // is this really useful as long as select() sleeps?
	{
		nlwarning("LNETL1: nice() failed: code=%d '%s'", errno, strerror(errno));
	}
#endif // NL_OS_UNIX

	// Copy of _Connections
	vector<TSockId>	connections_copy;

	while ( ! exitRequired() )
	{
		// 1. Remove closed connections
		clearClosedConnections();

		// POLL8

		// 2-SELECT-VERSION : select() on the sockets handled in the present thread

		descmax = 0;
		FD_ZERO( &readers );
		bool skip;
		bool alldisconnected = true;
		CConnections::iterator ipb;
		{
			// Lock _Connections
			CSynchronized<CConnections>::CAccessor connectionssync( &_Connections );

			// Prepare to avoid select if there is no connection
			skip = connectionssync.value().empty();

			// Fill the select array and copy _Connections
			connections_copy.clear();
			for ( ipb=connectionssync.value().begin(); ipb!=connectionssync.value().end(); ++ipb )
			{
				if ( (*ipb)->Sock->connected() ) // exclude disconnected sockets that are not deleted
				{
					alldisconnected = false;
					// Copy _Connections element
					connections_copy.push_back( *ipb );

					// Add socket descriptor to the select array
					FD_SET( (*ipb)->Sock->descriptor(), &readers );

					// Calculate descmax for select
					if ( (*ipb)->Sock->descriptor() > descmax )
					{
						descmax = (*ipb)->Sock->descriptor();
					}
				}
			}

#ifdef NL_OS_UNIX
			// Add the wake-up pipe into the select array
			FD_SET( _WakeUpPipeHandle[PipeRead], &readers );
			if ( _WakeUpPipeHandle[PipeRead]>descmax )
			{
				descmax = _WakeUpPipeHandle[PipeRead];
			}
#endif

			// Unlock _Connections, use connections_copy instead
		}

#ifndef NL_OS_UNIX
		// Avoid select if there is no connection (Windows only)
		if ( skip || alldisconnected )
		{
			nlSleep( 1 ); // nice
			continue;
		}
#endif

#ifdef NL_OS_WINDOWS
		TIMEVAL tv;
		tv.tv_sec = 0; // short time because the newly added connections can't be added to the select fd_set
		tv.tv_usec = 10000;

		// Call select
		int res = ::select( descmax+1, &readers, NULL, NULL, &tv );

#elif defined NL_OS_UNIX

		// Call select
		int res = ::select( descmax+1, &readers, NULL, NULL, NULL );

#endif // NL_OS_WINDOWS


		// POLL9

		// 3. Test the result
		switch ( res )
		{
#ifdef NL_OS_WINDOWS
			case  0 : continue; // time-out expired, no results
#endif
			case -1 :
				// we'll ignore message (Interrupted system call) caused by a CTRL-C
				/*if (CSock::getLastError() == 4)
				{
					LNETL1_DEBUG ("LNETL1: Select failed (in receive thread): %s (code %u) but IGNORED", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError());
					continue;
				}*/
				//nlerror( "LNETL1: Select failed (in receive thread): %s (code %u)", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError() );
				LNETL1_DEBUG( "LNETL1: Select failed (in receive thread): %s (code %u)", CSock::errorString( CSock::getLastError() ).c_str(), CSock::getLastError() );
				goto end;
		}

		// 4. Get results

		vector<TSockId>::iterator ic;
		for ( ic=connections_copy.begin(); ic!=connections_copy.end(); ++ic )
		{
			if ( FD_ISSET( (*ic)->Sock->descriptor(), &readers ) != 0 )
			{
				CServerBufSock *serverbufsock = static_cast<CServerBufSock*>(static_cast<CBufSock*>(*ic));
				try
				{
					// 4. Receive data
					if ( serverbufsock->receivePart( sizeof(TSockId) + 1 ) ) // +1 for the event type
					{
						serverbufsock->fillSockIdAndEventType( *ic );

						// Push message into receive queue
						//uint32 bufsize;
						//sint32 mbsize;

						_Server->pushMessageIntoReceiveQueue( serverbufsock->receivedBuffer() );

						//recvfifo.value().display();
						//bufsize = serverbufsock->receivedBuffer().size();
						//mbsize = recvfifo.value().size() / 1048576;
						//nldebug( "RCV: Released." );
						/*if ( mbsize > 1 )
						{
							nlwarning( "The receive queue size exceeds %d MB", mbsize );
						}*/
						/*
						// Statistics
						{
							CSynchronized<uint32>::CAccessor syncbpi ( &_Server->syncBytesPushedIn() );
							syncbpi.value() += bufsize;
						}
						*/
					}
				}
//				catch (const ESocketConnectionClosed&)
//				{
//					LNETL1_DEBUG( "LNETL1: Connection %s closed", serverbufsock->asString().c_str() );
//					// The socket went to _Connected=false when throwing the exception
//				}
				catch (const ESocket&)
				{
					LNETL1_DEBUG( "LNETL1: Connection %s broken", serverbufsock->asString().c_str() );
					(*ic)->Sock->disconnect();
				}
/*
#ifdef NL_OS_UNIX
				skip = true; // don't check _WakeUpPipeHandle (yes, check it to read any written byte)
#endif

*/
			}
		}

#ifdef NL_OS_UNIX
		// Test wake-up pipe
		if ( (!skip) && (FD_ISSET( _WakeUpPipeHandle[PipeRead], &readers )) )
		{
			uint8 b;
			if ( read( _WakeUpPipeHandle[PipeRead], &b, 1 ) == -1 ) // we were woken-up by the wake-up pipe
			{
				LNETL1_DEBUG( "LNETL1: In CServerReceiveTask::run(): read() failed" );
			}
			LNETL1_DEBUG( "LNETL1: Receive thread select woken-up" );
		}
#endif

		NbLoop++;
	}
end:
	nlnettrace( "Exiting CServerReceiveTask::run" );
	NbServerReceiveTask--;
	NbNetworkTask--;
}


/*
 * Delete all connections referenced in the remove list (double-mutexed)
 */

void CServerReceiveTask::clearClosedConnections()
{
	CConnections::iterator ic;
	{
		NLMISC::CSynchronized<CConnections>::CAccessor removesetsync( &_RemoveSet );
		{
			if ( ! removesetsync.value().empty() )
			{
				// Delete closed connections
				NLMISC::CSynchronized<CConnections>::CAccessor connectionssync( &_Connections );
				for ( ic=removesetsync.value().begin(); ic!=removesetsync.value().end(); ++ic )
				{
					LNETL1_DEBUG( "LNETL1: Removing a connection" );

					TSockId sid = (*ic);

					// Remove from the connection list
					connectionssync.value().erase( *ic );

					// Delete the socket object
					delete sid;
				}
				// Clear remove list
				removesetsync.value().clear();
			}
		}
	}
}

NLMISC_CATEGORISED_VARIABLE(nel, uint32, NbServerListenTask, "Number of server listen thread");
NLMISC_CATEGORISED_VARIABLE(nel, uint32, NbServerReceiveTask, "Number of server receive thread");

} // NLNET
