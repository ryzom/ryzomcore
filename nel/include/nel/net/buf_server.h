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

#ifndef NL_BUF_SERVER_H
#define NL_BUF_SERVER_H

#include "nel/misc/types_nl.h"
#include "buf_net_base.h"
#include "listen_sock.h"
#include "buf_sock.h"
#include "net_log.h"

#include <list>
#include <set>

// POLL1 (ignore POLL comments)

namespace NLNET {


class CInetAddress;
class CBufServer;


/**
 * Common part of CListenTask and CServerReceiveTask
 */
class CServerTask
{
public:

	/// Destructor
	virtual ~CServerTask();

	/// Tells the task to exit
	void	requireExit() { _ExitRequired = true; }

#ifdef NL_OS_UNIX
	/// Wake the thread up, when blocked in select (Unix only)
	void	wakeUp();
#endif

	uint32 NbLoop;

protected:

	/// Constructor
	CServerTask();

	/// Returns true if the requireExit() has been called
	bool	exitRequired() const { return _ExitRequired; }

#ifdef NL_OS_UNIX
	/// Pipe for select wake-up (Unix only)
	int		        _WakeUpPipeHandle [2];
#endif

private:

	volatile bool	_ExitRequired;
};


/**
 * Code of listening thread
 */
class CListenTask : public NLMISC::IRunnable, public CServerTask
{
public:

	/// Constructor
	CListenTask( CBufServer *server ) : CServerTask(), _Server(server) {}

	/// Begins to listen on the specified port (call before running thread)
	void			init( uint16 port, sint32 maxExpectedBlockSize );

	/// Run (exits when the listening socket disconnects)
	virtual void	run();

	/// Close listening socket
	void			close();

	/// Returns the listening address
	const CInetAddress&	localAddr() { return _ListenSock.localAddr(); }

private:

	CBufServer			*_Server;
	CListenSock			_ListenSock;
	uint32				_MaxExpectedBlockSize;

};


typedef std::vector<NLMISC::IThread*> CThreadPool;


// Mode: Small server
#undef PRESET_BIG_SERVER

#ifdef PRESET_BIG_SERVER
// Big server
#define DEFAULT_STRATEGY SpreadSockets
#define DEFAULT_MAX_THREADS 64
#define DEFAULT_MAX_SOCKETS_PER_THREADS 64
#else
// Small server
#define DEFAULT_STRATEGY FillThreads
#define DEFAULT_MAX_THREADS 64
#define DEFAULT_MAX_SOCKETS_PER_THREADS 16
#endif


/**
 * Server class for layer 1
 *
 * Listening socket and accepted connections, with packet scheme.
 * The provided buffers are sent raw (no endianness conversion).
 * By default, the size time trigger is disabled, the time trigger is set to 20 ms.
 *
 * Where do the methods take place:
 * \code
 * send(),	                           ->  send buffer   ->  update(), flush()
 * bytesSent(), newBytesSent()
 *
 * receive(), dataAvailable(),         <- receive buffer <-  receive thread,
 * dataAvailable(),
 * bytesReceived(), newBytesReceived(),
 * connection callback, disconnection callback
 * \endcode
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CBufServer : public CBufNetBase
{
public:

	enum TThreadStategy { SpreadSockets, FillThreads };

	/** Constructor
	 * Set nodelay to true to disable the Nagle buffering algorithm (see CTcpSock documentation)
	 * initPipeForDataAvailable is for Linux only. Set it to false if you provide an external pipe with
	 * setExternalPipeForDataAvailable().
	 */
	CBufServer( TThreadStategy strategy=DEFAULT_STRATEGY,
				uint16 max_threads=DEFAULT_MAX_THREADS,
				uint16 max_sockets_per_thread=DEFAULT_MAX_SOCKETS_PER_THREADS,
				bool nodelay=true, bool replaymode=false, bool initPipeForDataAvailable=true );

	/// Destructor
	virtual ~CBufServer();

	/// Listens on the specified port
	void	init( uint16 port );

	/** Disconnect a connection
	 * Set hostid to InvalidSockId to disconnect all connections.
	 * If hostid is not InvalidSockId and the socket is not connected, the method does nothing.
	 * If quick is true, any pending data will not be sent before disconnecting.
	 * If quick is false, a flush() will be called. In case of network congestion, the entire pending
	 * data may not be flushed. If this is a problem, call flush() multiple times until it returns 0
	 * before calling disconnect().
	 */
	void	disconnect( TSockId hostid, bool quick=false );

	/// Sets callback for incoming connections (or NULL to disable callback)
	void	setConnectionCallback( TNetCallback cb, void* arg ) { _ConnectionCallback = cb; _ConnectionCbArg = arg; }



	/** Send a message to the specified host, or to all hosts if hostid is InvalidSockId
	 */
	//void	send( const std::vector<uint8>& buffer, TSockId hostid );
	void	send( const NLMISC::CMemStream& buffer, TSockId hostid );

	/** Checks if there is some data to receive. Returns false if the receive queue is empty.
	 * This is where the connection/disconnection callbacks can be called.
	 */
	bool	dataAvailable();

#ifdef NL_OS_UNIX
	/** Wait until the receive queue contains something to read (implemented with a select()).
	 * This is where the connection/disconnection callbacks can be called.
	 * If you use this method (blocking scheme), don't use dataAvailable() (non-blocking scheme).
	 * \param usecMax Max time to wait in microsecond (up to 1 sec)
	 */
	void	sleepUntilDataAvailable( uint usecMax=100000 );
#endif

	/** Receives next block of data in the specified (resizes the vector)
	 * You must call dataAvailable() or sleepUntilDataAvailable() before every call to receive()
	 */
	void	receive( NLMISC::CMemStream& buffer, TSockId* hostid );

	/// Update the network (call this method evenly)
	void	update();




	// Returns the size in bytes of the data stored in the send queue.
	uint32	getSendQueueSize( TSockId destid );

	void	displaySendQueueStat( NLMISC::CLog *log = NLMISC::InfoLog, TSockId destid = InvalidSockId);

	void displayThreadStat (NLMISC::CLog *log = NLMISC::InfoLog);


	/** Sets the time flush trigger (in millisecond). When this time is elapsed,
	 * all data in the send queue is automatically sent (-1 to disable this trigger)
	 */
	void	setTimeFlushTrigger( TSockId destid, sint32 ms );

	/** Sets the size flush trigger. When the size of the send queue reaches or exceeds this
	 * value, all data in the send queue is automatically sent (-1 to disable this trigger )
	 */
	void	setSizeFlushTrigger( TSockId destid, sint32 size );

	/** Force to send data pending in the send queue now. If all the data could not be sent immediately,
	 * the returned nbBytesRemaining value is non-zero.
	 * \param destid The identifier of the destination connection.
	 * \param nbBytesRemaining If the pointer is not NULL, the method sets the number of bytes still pending after the flush attempt.
	 * \returns False if an error has occurred (e.g. the remote host is disconnected).
	 * To retrieve the reason of the error, call CSock::getLastError() and/or CSock::errorString()
	 */
	bool	flush( TSockId destid, uint *nbBytesRemaining=NULL );




	/// Returns the internet address of the listening socket
	const CInetAddress&	listenAddress() const { return _ListenTask->localAddr(); }

	/// Returns the address of the specified host
	const CInetAddress& hostAddress( TSockId hostid );

	/*
	/// Returns the number of bytes pushed into the receive queue since the beginning (mutexed)
	uint32	bytesDownloaded()
	{
		NLMISC::CSynchronized<uint32>::CAccessor syncbpi ( &_BytesPushedIn );
		return syncbpi.value();
	}
	/// Returns the number of bytes downloaded since the previous call to this method
	uint32	newBytesDownloaded();
	*/

	/// Returns the number of bytes popped by receive() since the beginning
	uint64	bytesReceived() const { return _BytesPoppedIn; }

	/// Returns the number of bytes popped by receive() since the previous call to this method
	uint64	newBytesReceived();

	/// Returns the number of bytes pushed by send() since the beginning
	uint64	bytesSent() const { return _BytesPushedOut; }

	/// Returns the number of bytes pushed by send() since the previous call to this method
	uint64	newBytesSent();

	/// Returns the number of connections (at the last update())
	uint32	nbConnections() const { return _NbConnections; }

protected:

	friend class CServerBufSock;
	friend class CListenTask;
	friend class CServerReceiveTask;

	/// Returns the TCP_NODELAY flag
	bool				noDelay() const { return _NoDelay; }

	/** Binds a new socket and send buffer to an existing or a new thread (that starts)
	 * Note: this method is called in the listening thread.
	 */
	void				dispatchNewSocket( CServerBufSock *bufsock );

	/// Returns the receive task corresponding to a particular thread
	CServerReceiveTask	*receiveTask( std::vector<NLMISC::IThread*>::iterator ipt )
	{
		return ((CServerReceiveTask*)((*ipt)->getRunnable()));
	}

	/// Pushes a buffer to the specified host's send queue and update (unless not connected)
	/*void pushBufferToHost( const std::vector<uint8>& buffer, TSockId hostid )
	{
		if ( hostid->pushBuffer( buffer ) )
		{
			_BytesPushedOut += buffer.size() + sizeof(TBlockSize); // statistics
		}
	}*/

	void pushBufferToHost( const NLMISC::CMemStream& buffer, TSockId hostid )
	{
		nlassert( hostid != InvalidSockId );
		if ( hostid->pushBuffer( buffer ) )
		{
			_BytesPushedOut += buffer.length() + sizeof(TBlockSize); // statistics
		}
	}

	// Creates a new task and run a new thread for it
	void				addNewThread( CThreadPool& threadpool, CServerBufSock *bufsock );

	/// Returns the connection callback
	TNetCallback		connectionCallback() const { return _ConnectionCallback; }

	/// Returns the argument of the connection callback
	void*				argOfConnectionCallback() const { return _ConnectionCbArg; }

	/*/// Returns the synchronized number of bytes pushed into the receive queue
	NLMISC::CSynchronized<uint32>&	syncBytesPushedIn() { return _BytesPushedIn; }
	*/

private:

	typedef std::set<TSockId>		TClientSet;
	/// List of currently connected client
	TClientSet						_ConnectedClients;


	/// Thread socket-handling strategy
	TThreadStategy					_ThreadStrategy;

	/// Max number of threads
	uint16							_MaxThreads;

	/// Max number of sockets handled by one thread
	uint16							_MaxSocketsPerThread;

	/// Listen task
	CListenTask						*_ListenTask;

	/// Listen thread
	NLMISC::IThread					*_ListenThread;

	/* Vector of receiving threads.
	 * Thread: thread control
	 * Thread->Runnable: access to the CServerReceiveTask object
	 * Thread->getRunnable()->sock(): access to the socket
	 * The purpose of this list is to delete the objects after use.
	 */
	NLMISC::CSynchronized<CThreadPool>		_ThreadPool;

	/// Connection callback
	TNetCallback					_ConnectionCallback;

	/// Argument of the connection callback
	void*							_ConnectionCbArg;

	/// Number of bytes pushed by send() since the beginning
	uint64							_BytesPushedOut;

	/// Number of bytes popped by receive() since the beginning
	uint64							_BytesPoppedIn;

	/// Previous number of bytes received
	uint64							_PrevBytesPoppedIn;

	/// Previous number of bytes sent
	uint64							_PrevBytesPushedOut;

	/// Number of connections (debug stat)
	uint32							_NbConnections;

	/// TCP_NODELAY
	bool							_NoDelay;

	/// Replay mode flag
	bool							_ReplayMode;

  /*
	/// Number of bytes pushed into the receive queue (by the receive threads) since the beginning.
	NLMISC::CSynchronized<uint32>	_BytesPushedIn;

	/// Previous number of bytes received
	uint32							_PrevBytesPushedIn;
	*/
};


typedef std::set<TSockId>					CConnections;


// POLL2


/**
 * Code of receiving threads for servers.
 * Note: the methods locations in the classes do not correspond to the threads where they are
 * executed, but to the data they use.
 */
class CServerReceiveTask : public NLMISC::IRunnable, public CServerTask
{
public:

	/// Constructor
	CServerReceiveTask( CBufServer *server ) : CServerTask(), _Server(server), _Connections("CServerReceiveTask::_Connections"), _RemoveSet("CServerReceiveTask::_RemoveSet") {}

	/// Run
	virtual void run();

	/// Returns the number of connections handled by the thread (mutexed on _Connections)
	uint	numberOfConnections()
	{
		uint nb;
		{
			NLMISC::CSynchronized<CConnections>::CAccessor connectionssync( &_Connections );
			nb = (uint)connectionssync.value().size();
		}
		return nb;
	}

	/// Add a new connection into this thread (mutexed on _Connections)
	void	addNewSocket( TSockId sockid )
	{
		//nlnettrace( "CServerReceiveTask::addNewSocket" );
		nlassert( sockid != InvalidSockId );
		{
			NLMISC::CSynchronized<CConnections>::CAccessor connectionssync( &_Connections );
			connectionssync.value().insert( sockid );
		}
		// POLL3
	}

// POLL4

	/** Add connection to the remove set (mutexed on _RemoveSet)
	 * Note: you must not call this method within a mutual exclusion on _Connections, or
	 * there will be a deadlock (see clearClosedConnection())
	 */
	void	addToRemoveSet( TSockId sockid )
	{
		nlnettrace( "CServerReceiveTask::addToRemoveSet" );
		nlassert( sockid != InvalidSockId );
		{
			// Three possibilities :
			// - The value is inserted into the set.
			// - The value is already present in the set.
			// - The set is locked by a receive thread which is removing the closed connections.
			//   When the set gets unlocked, it is empty so the value is inserted. It means the
			//   value could be already in the set before it was cleared.
			//   Note: with a fonction such as tryAcquire(), we could avoid to enter the mutex
			//   when it is already locked
			// See clearClosedConnections().
			NLMISC::CSynchronized<CConnections>::CAccessor removesetsync( &_RemoveSet );
			removesetsync.value().insert( sockid );
			//LNETL1_DEBUG( "LNETL1: ic: %p - RemoveSet.size(): %d", ic, removesetsync.value().size() );
		}
#ifdef NL_OS_UNIX
		wakeUp();
#endif
	}

	/// Delete all connections referenced in the remove list (mutexed on _RemoveSet and on _Connections)
	void	clearClosedConnections();

	/// Access to the server
	CBufServer	*server()	{ return _Server; }

	friend	class CBufServer;

private:

	CBufServer								*_Server;

	/* List of sockets and send buffer.
	 * A TSockId is a pointer to a CBufSock object
	 */
	NLMISC::CSynchronized<CConnections>		_Connections;

	// Connections to remove
	NLMISC::CSynchronized<CConnections>		_RemoveSet;

	// POLL5

};


} // NLNET


#endif // NL_BUF_SERVER_H

/* End of buf_server.h */
