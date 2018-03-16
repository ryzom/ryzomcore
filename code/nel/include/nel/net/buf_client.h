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

#ifndef NL_BUF_CLIENT_H
#define NL_BUF_CLIENT_H

#include "nel/misc/types_nl.h"
#include "buf_net_base.h"
#include "tcp_sock.h"
#include "buf_sock.h"


namespace NLNET {


class CInetAddress;
class CBufClient;


/**
 * Code of receiving thread for clients
 */
class CClientReceiveTask : public NLMISC::IRunnable
{
public:

	/// Constructor (increments the reference to the object pointed to by the smart pointer sockid)
	CClientReceiveTask( CBufClient *client, CNonBlockingBufSock *bufsock ) : NbLoop(0), _Client(client), _NBBufSock(bufsock) {} // CHANGED: non-blocking client connection

	/// Run
	virtual void run();

	/// Returns a pointer to the bufsock object
	CNonBlockingBufSock		*bufSock() { return _NBBufSock; } // CHANGED: non-blocking client connection (previously, returned _SockId->Sock)

	/// Returns the socket identifier
	TSockId					sockId() { return (TSockId)_NBBufSock; }

	uint32	NbLoop;

private:

	CBufClient					*_Client;

	CNonBlockingBufSock			*_NBBufSock; // CHANGED: non-blocking client connection
};



/**
 * Client class for layer 1
 *
 * Active connection with packet scheme and buffering.
 * The provided buffers are sent raw (no endianness conversion).
 * By default, the size time trigger is disabled, the time trigger is set to 20 ms.
 *
 * Where do the methods take place:
 * \code
 * send()             ->  send buffer   ->  update(), flush(),
 *                                          bytesUploaded(), newBytesUploaded()
 *
 * receive(),         <- receive buffer <-  receive thread,
 * dataAvailable(),                         bytesDownloaded(), newBytesDownloaded()
 * disconnection callback
 * \endcode
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CBufClient : public CBufNetBase
{
public:

	/** Constructor. Set nodelay to true to disable the Nagle buffering algorithm (see CTcpSock documentation)
	 * initPipeForDataAvailable is for Linux only. Set it to false if you provide an external pipe with
	 * setExternalPipeForDataAvailable().
	 */
	CBufClient( bool nodelay=true, bool replaymode=false, bool initPipeForDataAvailable=true );

	/// Destructor
	virtual ~CBufClient();

	/// Connects to the specified host
	void	connect( const CInetAddress& addr );

	/** Disconnects the remote host and empties the receive queue.
	 * Before that, tries to flush pending data to send unless quick is true.
	 * In case of network congestion, the entire pending data may not be flushed.
	 * If this is a problem, call flush() multiple times until it returns 0 before calling disconnect().
	 * The disconnection callback will *not* be called.
	 * Do not call if the socket is not connected.
	 */
	void	disconnect( bool quick=false );

	/** Sends a message to the remote host (in fact the message is buffered into the send queue)
	 */
	//void	send( const std::vector<uint8>& buffer );
	void	send( const NLMISC::CMemStream& buffer );

	/** Checks if there is some data to receive. Returns false if the receive queue is empty.
	 * This is where the connection/disconnection callbacks can be called
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

	/** Receives next block of data in the specified buffer (resizes the vector)
	 * You must call dataAvailable() before every call to receive()
	 */
	//void	receive( std::vector<uint8>& buffer );
	void	receive( NLMISC::CMemStream& buffer );

	/// Update the network (call this method evenly)
	void	update();



	// Returns the size in bytes of the data stored in the send queue.
	uint32	getSendQueueSize() const { return _BufSock->SendFifo.size(); }

	void displaySendQueueStat (NLMISC::CLog *log = NLMISC::InfoLog)
	{
		_BufSock->SendFifo.displayStats(log);
	}

	void displayThreadStat (NLMISC::CLog *log);

	/** Sets the time flush trigger (in millisecond). When this time is elapsed,
	 * all data in the send queue is automatically sent (-1 to disable this trigger)
	 */
	void	setTimeFlushTrigger( sint32 ms ) { _BufSock->setTimeFlushTrigger( ms ); }

	/** Sets the size flush trigger. When the size of the send queue reaches or exceeds this
	 * calue, all data in the send queue is automatically sent (-1 to disable this trigger )
	 */
	void	setSizeFlushTrigger( sint32 size ) { _BufSock->setSizeFlushTrigger( size ); }

	/** Force to send data pending in the send queue now. If all the data could not be sent immediately,
	 * the returned nbBytesRemaining value is non-zero.
	 * \param nbBytesRemaining If the pointer is not NULL, the method sets the number of bytes still pending after the flush attempt.
	 * \returns False if an error has occurred (e.g. the remote host is disconnected).
	 * To retrieve the reason of the error, call CSock::getLastError() and/or CSock::errorString()
	 */
	bool	flush( uint *nbBytesRemaining=NULL ) { return _BufSock->flush( nbBytesRemaining ); }



	/** Returns true if the connection is still connected (changed when a disconnection
	 * event has reached the front of the receive queue, just before calling the disconnection callback
	 * if there is one)
	 */
	bool	connected() const { return _BufSock->connectedState(); }

	/// Returns the address of the remote host
	const CInetAddress&	remoteAddress() const { return _BufSock->Sock->remoteAddr(); }

	/// Returns the number of bytes downloaded (read or still in the receive buffer) since the latest connection
	uint64	bytesDownloaded() const { return _BufSock->Sock->bytesReceived(); }

	/// Returns the number of bytes uploaded (flushed) since the latest connection
	uint64	bytesUploaded() const { return _BufSock->Sock->bytesSent(); }

	/// Returns the number of bytes downloaded since the previous call to this method
	uint64	newBytesDownloaded();

	/// Returns the number of bytes uploaded since the previous call to this method
	uint64	newBytesUploaded();

	/*//Not right because we add callbacks in the receive queue
	/// Returns the number of bytes popped by receive() since the beginning (mutexed on the receive queue)
	uint64	bytesReceived() {  }

	/// Returns the number of bytes pushed by send() since the beginning
	uint64	bytesSent() { return bytesUploaded() + getSendQueueSize(); }

	/// Returns the number of bytes popped by receive() since the previous call to this method
	uint64	newBytesReceived();

	/// Returns the number of bytes pushed by send() since the previous call to this method
	uint64	newBytesSent();
	*/

	/// Returns the id of the connection
	TSockId	id() const { return _BufSock; /*_RecvTask->sockId();*/ }


protected:

	friend class CClientReceiveTask;

	/// Send buffer and connection
	CNonBlockingBufSock *_BufSock; // ADDED: non-blocking client connection

	/// True when the Nagle algorithm must be disabled (TCP_NODELAY)
	bool				_NoDelay;

	/// Previous number of bytes downloaded
	uint64				_PrevBytesDownloaded;

	/// Previous number of bytes uploaded
	uint64				_PrevBytesUploaded;

	/*
	/// Previous number of bytes received
	uint32				_PrevBytesReceived;

	/// Previous number of bytes sent
	uint32				_PrevBytesSent;
	*/

private:

	/// Receive task
	CClientReceiveTask	*_RecvTask;

	/// Receive thread
	NLMISC::IThread		*_RecvThread;

};



} // NLNET


#endif // NL_BUF_CLIENT_H

/* End of buf_client.h */
