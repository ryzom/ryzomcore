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

#ifndef NL_BUF_SOCK_H
#define NL_BUF_SOCK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/hierarchical_timer.h"

#include "buf_net_base.h"
#include "tcp_sock.h"
#include "net_log.h"


//#include <deque>

namespace NLNET {


#define nlnettrace(__msg) //LNETL1_DEBUG("LNETL1: %s",__msg);


class CTcpSock;
class CServerReceiveTask;
class CBufNetBase;


/**
 * CBufSock
 * A socket and its sending buffer
 */
class CBufSock
{
public:

	/// Destructor
	virtual ~CBufSock();

	/// Sets the application identifier
	void					setAppId( uint64 id ) { _AppId = id; }

	/// Returns the application identifier
	uint64					appId() const { return _AppId; }

	/// Returns a string with the characteristics of the object
	std::string				asString() const;

	/// get the TCP sock object
	const CTcpSock			*getTcpSock() const { return Sock;}

	/// Little tricky but this string is used by Layer4 to know which callback is authorized.
	/// This is empty when all callback are authorized.
	std::string				AuthorizedCallback;

protected:

	friend class CBufClient;
	friend class CBufServer;
	friend class CClientReceiveTask;
	friend class CServerReceiveTask;

	friend class CCallbackClient;
	friend class CCallbackServer;
	friend class CCallbackNetBase;

	/** Constructor
	 * \param sock To provide an external socket. Set it to NULL to create it internally.
	 */
	CBufSock( CTcpSock *sock=NULL );

	///@name Sending data
	//@{

	/// Update the network sending (call this method evenly). Returns false if an error occured.
	bool	update();

	/** Sets the time flush trigger (in millisecond). When this time is elapsed,
	 * all data in the send queue is automatically sent (-1 to disable this trigger)
	 */
	void	setTimeFlushTrigger( sint32 ms );

	/** Sets the size flush trigger. When the size of the send queue reaches or exceeds this
	 * calue, all data in the send queue is automatically sent (-1 to disable this trigger )
	 */
	void	setSizeFlushTrigger( sint32 size ) { _TriggerSize = size; }

	/** Force to send data pending in the send queue now. In the case of a non-blocking socket
	 * (see CNonBlockingBufSock), if all the data could not be sent immediately,
	 * the returned nbBytesRemaining value is non-zero.
	 * \param nbBytesRemaining If the pointer is not NULL, the method sets the number of bytes still pending after the flush attempt.
	 * \returns False if an error has occured (e.g. the remote host is disconnected).
	 * To retrieve the reason of the error, call CSock::getLastError() and/or CSock::errorString()
	 */
	bool	flush( uint *nbBytesRemaining=NULL );

	//@}

	/// Returns "CLT " (client)
	virtual std::string typeStr() const { return "CLT "; }

	/** Pushes a disconnection message into bnb's receive queue, if it has not already been done
	 * (returns true in this case). You can either specify a sockid (for server) or InvalidSockId (for client)
	 */
	bool advertiseDisconnection( CBufNetBase *bnb, TSockId sockid )
	{
#ifdef NL_DEBUG
		if ( sockid != InvalidSockId )
		{
			nlassert( sockid == this );
		}
#endif
		return advertiseSystemEvent( bnb, sockid, _KnowConnected, true, CBufNetBase::Disconnection );
	}


	/** Pushes a system message into bnb's receive queue, if the flags meets the condition, then
	 * resets the flag and returns true. You can either specify a sockid (for server) or InvalidSockId (for client).
	 */
	bool advertiseSystemEvent(
		CBufNetBase *bnb, TSockId sockid, bool& flag, bool condition, CBufNetBase::TEventType event )
	{
#ifdef NL_DEBUG
		if ( sockid != InvalidSockId )
		{
			nlassert( sockid == this );
		}
#endif
		// Test flag
		if ( flag==condition )
		{
			LNETL1_DEBUG( "LNETL1: Pushing event to %s", asString().c_str() );
			std::vector<uint8> buffer;
			if ( sockid == InvalidSockId )
			{
				// Client: event type only
				buffer.resize( 1 );
				buffer[0] = uint8(event);
			}
			else
			{
				// Server: sockid + event type
				buffer.resize( sizeof(TSockId) + 1 );
				memcpy( &*buffer.begin(), &sockid, sizeof(TSockId) );
				buffer[sizeof(TSockId)] = uint8(event);
			}
			// Push
			bnb->pushMessageIntoReceiveQueue( buffer );

			// Reset flag
			flag = !condition;
			return true;
		}
		else
		{
			return false;
		}
	}

	/** Pushes a buffer to the send queue and update,
	 * or returns false if the socket is not physically connected the or an error occured during sending
	 */
	bool pushBuffer( const NLMISC::CMemStream& buffer )
	{
		nlassert (this != InvalidSockId);	// invalid bufsock
//		LNETL1_DEBUG( "LNETL1: Pushing buffer to %s", asString().c_str() );

		static uint32 biggerBufferSize = 64000;
		if (buffer.length() > biggerBufferSize)
		{
			biggerBufferSize = buffer.length();
			LNETL1_DEBUG ("LNETL1: new record! bigger network message pushed (sent) is %u bytes", biggerBufferSize);
		}

		if ( Sock->connected() )
		{
			// Push into host's send queue
			SendFifo.push( buffer );

			// Update sending
			bool res = update ();
			return res; // not checking the result as in CBufServer::update()
		}
		return false;
	}

	/*bool pushBuffer( const std::vector<uint8>& buffer )
	{
		nlassert (this != InvalidSockId);	// invalid bufsock
//		LNETL1_DEBUG( "LNETL1: Pushing buffer to %s", asString().c_str() );

		static uint32 biggerBufferSize = 64000;
		if (buffer.size() > biggerBufferSize)
		{
			biggerBufferSize = buffer.size();
			nlwarning ("LNETL1: new record! bigger network message pushed (sent) is %u bytes", biggerBufferSize);
		}

		if ( Sock->connected() )
		{
			// Push into host's send queue
			SendFifo.push( buffer );

			// Update sending
			bool res = update ();
			return res; // not checking the result as in CBufServer::update()
		}
		return false;
	}*/


	/// Connects to the specified addr; set connectedstate to true if no connection advertising is needed
	void connect( const CInetAddress& addr, bool nodelay, bool connectedstate );

	/// Disconnects; set connectedstate to false if no disconnection advertising is needed
	void disconnect( bool connectedstate );

	/// Sets the "logically connected" state (changed when processing a connection/disconnection callback)
	void setConnectedState( bool connectedstate ) { _ConnectedState = connectedstate; }

	/// Returns the "logically connected" state (changed when processing a connection/disconnection callback)
	bool connectedState() const { return _ConnectedState; }

	// Send queue
	NLMISC::CBufFIFO	SendFifo;

	// Socket (pointer because it can be allocated by an accept())
	CTcpSock			*Sock;

	// Prevents from pushing a connection/disconnection event twice
	bool				_KnowConnected;

private:

#ifdef NL_DEBUG
	enum TFlushTrigger { FTTime, FTSize, FTManual };
	TFlushTrigger		_FlushTrigger;
#endif

	NLMISC::TTime		_LastFlushTime; // updated only if time trigger is enabled (TriggerTime!=-1)
	NLMISC::TTime		_TriggerTime;
	sint32				_TriggerSize;

	NLMISC::CObjectVector<uint8> _ReadyToSendBuffer;
	TBlockSize			_RTSBIndex;

	uint64				_AppId;

	// Connected state (from the user's point of view, i.e. changed when the connection/disconnection event is at the front of the receive queue)
	bool				_ConnectedState;
};


/**
 * CNonBlockingBufSock
 * A socket, its send buffer plus a nonblocking receiving system
 */
class CNonBlockingBufSock : public CBufSock
{
protected:

	friend class CBufClient;
	friend class CClientReceiveTask;

	/** Constructor
     * \param sock To provide an external socket. Set it to NULL to create it internally.
     * \maxExpectedBlockSize Default value: receiving limited to 10 M per block)
     */
	CNonBlockingBufSock( CTcpSock *sock=NULL, uint32 maxExpectedBlockSize=10485760 );

	/** Call this method after connecting (for a client connection) to set the non-blocking mode.
	 * For a server connection, call it as soon as the object is constructed
	 */
	void						setNonBlocking() { Sock->setNonBlockingMode( true ); }

	/// Set the size limit for received blocks
	void						setMaxExpectedBlockSize( sint32 limit ) { _MaxExpectedBlockSize = limit; }

	/** Receives a part of a message (nonblocking socket only)
	 * \param nbExtraBytes Number of bytes to reserve for extra information such as the event type
	 * \return True if the message has been completely received
	 */
	bool						receivePart( uint32 nbExtraBytes );

	/// Fill the event type byte at pos length()(for a client connection)
	void						fillEventTypeOnly() { _ReceiveBuffer[_Length] = (uint8)CBufNetBase::User; }

	/** Return the length of the received block (call after receivePart() returns true).
	 * The total size of received buffer is length() + nbExtraBytes (passed to receivePart()).
	 */
	uint32						length() const { return _Length; }

	/** Returns the filled buffer (call after receivePart() returns true).
	 * Its size is length()+1.
	 */
	const std::vector<uint8>	receivedBuffer() const { nlnettrace( "CServerBufSock::receivedBuffer" ); return _ReceiveBuffer; }

	// Buffer for nonblocking receives
	std::vector<uint8>			_ReceiveBuffer;

	// Max payload size than can be received in a block
	uint32						_MaxExpectedBlockSize;

private:

	// True if the length prefix has already been read
	bool						_NowReadingBuffer;

	// Counts the number of bytes read for the current element (length prefix or buffer)
	TBlockSize					_BytesRead;

	// Length of buffer to read
	TBlockSize					_Length;

};


class CBufServer;


/**
 * CServerBufSock
 * A socket, its send buffer plus a nonblocking receiving system for a server connection
 */
class CServerBufSock : public CNonBlockingBufSock
{
protected:

	friend class CBufServer;
	friend class CListenTask;
	friend class CServerReceiveTask;

	/** Constructor with an existing socket (created by an accept()).
	 * Don't forget to call setOwnerTask().
	 */
	CServerBufSock( CTcpSock *sock );

	/// Sets the task that "owns" the CServerBufSock object
	void						setOwnerTask( CServerReceiveTask* owner ) { _OwnerTask = owner; }

	/// Returns the task that "owns" the CServerBufSock object
	CServerReceiveTask			*ownerTask() { return _OwnerTask; }

	/** Pushes a connection message into bnb's receive queue, if it has not already been done
	 * (returns true in this case).
	 */
	bool						advertiseConnection( CBufServer *bnb )
	{
		return advertiseSystemEvent( (CBufNetBase*)bnb, this, _KnowConnected, false, CBufNetBase::Connection );
	}

	/// Returns "SRV " (server)
	virtual std::string			typeStr() const { return "SRV "; }

	/// Fill the sockid and the event type byte at the end of the buffer
	void						fillSockIdAndEventType( TSockId sockId )
	{
		memcpy( (&*_ReceiveBuffer.begin()) + length(), &sockId, sizeof(TSockId) );
		_ReceiveBuffer[length() + sizeof(TSockId)] = (uint8)CBufNetBase::User;
	}

private:

	/// True after a connection callback has been sent to the user, for this connection
	bool				_Advertised;

	// The task that "owns" the CServerBufSock object
	CServerReceiveTask	*_OwnerTask;

};


} // NLNET

#endif // NL_BUF_SOCK_H

/* End of buf_sock.h */
