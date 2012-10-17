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

#ifndef NL_SOCK_H
#define NL_SOCK_H

#include "nel/misc/common.h"
#include "nel/misc/mutex.h"
#include "inet_address.h"
//#include <sstream>

/// This namespace contains all network class
namespace NLNET {


/**
 * Network exceptions
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
struct ESocket : public NLMISC::Exception
{
	/** Constructor
	 * You can provide an internet address. If so, reason *must* contain "%s"
	 * where the address should be written. Moreover, the length of reason plus
	 * the length of the address when displayed by asString() should no exceed 256.
	 */
	ESocket( const char *reason="", bool systemerror=true, CInetAddress *addr=NULL );
};


/// Exception raised when connect() fails
struct ESocketConnectionFailed : public ESocket
{
	ESocketConnectionFailed( CInetAddress addr ) : ESocket( "Connection to %s failed", true, &addr ) {}
};


/// Exception raised when a connection is gracefully closed by peer
struct ESocketConnectionClosed : public ESocket
{
	ESocketConnectionClosed() : ESocket( "Connection closed" ) {}
};


/// Exception raised when an unauthorized access has been done
struct EAccessDenied : public ESocket
{
	EAccessDenied( std::string s ): ESocket( (std::string("Access denied: ")+s).c_str(), false ) {}
};


/// Exception raised when a the NS does not find the service looked-up
struct EServiceNotFound : public ESocket
{
	EServiceNotFound( std::string s ): ESocket( (std::string("Service not found: ")+s).c_str(), false ) {}
};


//typedef SOCKET;
#ifdef NL_OS_WINDOWS
	typedef uint SOCKET;
#elif defined NL_OS_UNIX
	typedef int SOCKET;
#endif



/**
 * CSock: base socket class.
 * One CSock object represents a communication between two hosts, the local one and the remote one.
 * This class implements layer 0 of the NeL Network Engine.
 * This class does not handle conversion between big endian and little endian ; the provided
 * buffers are sent raw.
 *
 * The "logging" boolean value is necessary because in this implementation we always log
 * to one single global CLog object : there is not one CLog object per socket. Therefore
 * we must prevent the socket used in CNetDisplayer from logging itself... otherwise we
 * would have an infinite recursion.
 *
 * The "connected" property may have a different meaning whether the socket is a stream socket
 * (e.g. using TCP) or it is a connectionless datagram socket (e.g. using UDP). In the latter,
 * "connected" only means that the local and the remote addresses have been set.
 *
 * Important note: this class is thread-safe, meaning you can access to a CSock object
 * from multiple threads BUT the only things you are allow to do in parallel are
 * receive/send and read the connected() property.
 *
 * You must call CSock::initNetwork() before using any network class (even CInetAddress).
 * You must call CSock::releaseNetwork() at the end of your program.
 *
 * By default, a socket is in blocking mode. Call setNonBlockingMode() to change this
 * behaviour.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000-2001
 */
class CSock
{
public:

	enum TSockResult { Ok, WouldBlock, ConnectionClosed, Error };

	/// Initialize the network engine, if it is not already done
	static void			initNetwork();

	/// Releases the network engine
	static void			releaseNetwork();

	/** Returns the code of the last error that has occured.
	 * Note: This code is platform-dependant. On Unix, it is errno; on Windows it is the Winsock error code.
	 * See also errorString()
	 */
	static uint			getLastError();

	/// Returns a string explaining the network error (see getLastError())
	static std::string	errorString( uint errorcode );

	/// Change the time out value used in getDataAvailable(), which is 0 by default
	void				setTimeOutValue( long sec, long ms )
	{
		_TimeoutS = sec;
		if ( ms > 999 )
			ms = 999;
		_TimeoutUs = ms * 1000;
	}

	/// @name Socket setup
	//@{

	/** Connection.
	 * This method does not return a boolean, otherwise a programmer could ignore the result and no
	 * exception would be thrown if connection fails :
	 * - If addr is not valid, an exception ESocket is thrown
	 * - If connect() fails for another reason, an exception ESocketConnectionFailed is thrown
	 */
	virtual void		connect( const CInetAddress& addr );

	/** Sets the socket in nonblocking mode. Call this method *after* connect(), otherwise you will get
	 * an "would block" error (10035 on Windows). In nonblocking mode, use received() and sent() instead of receive() and send()
	 */
	void				setNonBlockingMode ( bool bm );

	/// Returns the nonblocking mode
	bool				nonBlockingMode() const { return _NonBlocking; }

	/** Closes the socket (without shutdown)
	 * In general you don't need to call this method. But you can call it to:
	 * - close a listening socket (i.e. stop accepting connections), or
	 * - stop a select() in progress in another thread (in this case, just calling the destructor is not enough)
	 */
	virtual void		close();

	/// Destructor (shutdown + close)
	virtual ~CSock();

	//@}


	/// @name Receiving data
	//@{

	/// Checks if there is some data to receive, waiting (blocking) at most for the time out value.
	bool				dataAvailable();

	/** Receive a partial or an entire block of data, depending on nonblocking mode.
	 *
	 * In blocking mode: the method waits until 'len' bytes have been received.
	 *
	 * In nonblocking mode: the method reads the bytes that have already been received only, and
	 * resets 'len' to the number of bytes read. The actual length may be smaller than the demanded
	 * length. In no data is available, the return value is CSock::WouldBlock. If dataAvailable()
	 * returns true, you are sure that receive() will not return CSock::WouldBlock.
	 *
	 * In case of graceful disconnection:
	 * - connected() become false
	 * - the return value is CSock::ConnectionClosed or an ESocketConnectionClosed exception is thrown.
	 *
	 * In case of failure (e.g. connection reset by peer) :
	 * - the return value is CSock::Error or an ESocket exception is thrown.
	 * You may want to close the connection manually.
	 */
	CSock::TSockResult	receive( uint8 *buffer, uint32& len, bool throw_exception=true );

	//@}


	/// @name Sending data
	//@{

	/** Sends a message.
	 *
	 * In blocking mode: the method waits until 'len' bytes have been sent.
	 *
	 * In nonblocking mode : the method resets len to the actual number of bytes sent.
	 * Even if less bytes than expected have been sent, it returns CSock::Ok. The caller
	 * is expected to test the actual len to check if the remaining data must be resent.
	 *
	 * \return CSock::Ok or CSock::Error (in case of failure).
	 * When throw_exception is true, the method throws an ESocket exception in case of failure.
     */
	CSock::TSockResult	send( const uint8 *buffer, uint32& len, bool throw_exception=true );

	//@}


	/// @name Properties
	//@{

	/// Returns if the socket is connected (volatile)
	bool				connected() { return _Connected; }

	/// Returns a const reference on the local address
	const CInetAddress&	localAddr() const {	return _LocalAddr; }

	/// Returns the address of the remote host
	const CInetAddress&	remoteAddr() const { return _RemoteAddr; }

	/// Returns the socket descriptor
	SOCKET				descriptor() const { return _Sock; }

	/// Returns the time out value in millisecond
	uint32				timeOutValue() const { return _TimeoutS*1000 + _TimeoutUs/1000; }

	//@}

	/// Returns the number of bytes received since the latest connection
	uint64				bytesReceived() const { return _BytesReceived; }

	/// Returns the number of bytes sent since the latest connection
	uint64				bytesSent() const { return _BytesSent; }

	/// Sets the send buffer size
	void				setSendBufferSize( sint32 size );

	/// Gets the send buffer size
	sint32				getSendBufferSize();

	/// Returns true if the network engine is initialized
	static bool			initialized() { return CSock::_Initialized; }

protected:

	/**
	 * Constructor.
	 * \param logging Disable logging if the server socket object is used by the logging system, to avoid infinite recursion
	 */
	CSock( bool logging = true );

	/// Construct a CSock object using an existing connected socket descriptor and its associated remote address
	CSock( SOCKET sock, const CInetAddress& remoteaddr );

	/// Creates the socket and get a valid descriptor
	void			createSocket( int type, int protocol );

	/// Sets the local address
	void			setLocalAddress();

	/// Socket descriptor
	SOCKET			_Sock;

	/// Address of local host (valid if connected)
	CInetAddress	_LocalAddr;

	/// Address of the remote host (valid if connected)
	CInetAddress	_RemoteAddr;

	/// If false, do not log any information
	bool			_Logging;

	/// If true, the socket is in nonblocking mode
	bool			_NonBlocking;

	/// True after calling connect()
	//NLMISC::CSynchronized<bool>	_SyncConnected;
	volatile bool	_Connected;

	/// Number of bytes received on this socket
	uint64			_BytesReceived;

	/// Number of bytes sent on this socket
	uint64			_BytesSent;

	/// Main time out value (sec) for select in dataAvailable()
	long			_TimeoutS;

	/// Secondary time out value (microsec) for select in dataAvailable()
	long			_TimeoutUs;

private:

	/// True if the network library has been initialized
	static bool		_Initialized;

	// Test: send & receive duration (ms)
	uint32			_MaxReceiveTime;
	uint32			_MaxSendTime;

	/// Flag used to determine the moments at which sends atrta an stop blocking
	bool			_Blocking;

};


} // NLNET


#endif // NL_SOCK_H

/* End of sock.h */
