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

#include "nel/net/tcp_sock.h"
#include "nel/net/net_log.h"

#ifdef NL_OS_WINDOWS
#	include <winsock2.h>
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#	define socklen_t int
#	define ERROR_NUM WSAGetLastError()
#elif defined NL_OS_UNIX
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <cerrno>
#	define SOCKET_ERROR -1
#	define INVALID_SOCKET -1
#	define ERROR_NUM errno
#	define ERROR_MSG strerror(errno)
typedef int SOCKET;
#endif

using namespace NLMISC;

namespace NLNET {


/*
 * Constructor
 */
CTcpSock::CTcpSock( bool logging ) :
	CSock( logging )
{}


/*
 * Construct a CTcpSocket object using an already connected socket
 */
CTcpSock::CTcpSock( SOCKET sock, const CInetAddress& remoteaddr ) :
	CSock( sock, remoteaddr )
{}


/* Connection. You can reconnect a socket after being disconnected.
 * This method does not return a boolean, otherwise a programmer could ignore the result and no
 * exception would be thrown if connection fails :
 * - If addr is not valid, an exception ESocket is thrown
 * - If connect() fails for another reason, an exception ESocketConnectionFailed is thrown
 */
void CTcpSock::connect( const CInetAddress& addr )
{
	// Create a new socket
	if ( _Sock != INVALID_SOCKET )
	{
	  if ( _Logging )
	    {
//		LNETL0_DEBUG( "LNETL0: Closing socket %d before reconnecting", _Sock );
	    }
	  close();
	}
	createSocket( SOCK_STREAM, IPPROTO_TCP );

	// activate keep alive
	setKeepAlive(true);

	// Connection
	CSock::connect( addr );
}


/*
 * Active disconnection. After disconnecting, you can't connect back with the same socket.
 */
void CTcpSock::disconnect()
{
	LNETL0_DEBUG( "LNETL0: Socket %d disconnecting from %s...", _Sock, _RemoteAddr.asString().c_str() );

	// This shutdown resets the connection immediately (not a graceful closure)
#ifdef NL_OS_WINDOWS
	::shutdown( _Sock, SD_BOTH );
#elif defined NL_OS_UNIX
	::shutdown( _Sock, SHUT_RDWR );
#endif
	/*CSynchronized<bool>::CAccessor sync( &_SyncConnected );
	sync.value() = false;*/
	_Connected = false;
}


/*
 * Active disconnection for download way only
 */
void CTcpSock::shutdownReceiving()
{
#ifdef NL_OS_WINDOWS
	::shutdown( _Sock, SD_RECEIVE );
#elif defined NL_OS_UNIX
	::shutdown( _Sock, SHUT_RD );
#endif
}


/*
 * Active disconnection for upload way only
 */
void CTcpSock::shutdownSending()
{
#ifdef NL_OS_WINDOWS
	::shutdown( _Sock, SD_SEND );
#elif defined NL_OS_UNIX
	::shutdown( _Sock, SHUT_WR );
#endif
}



void CTcpSock::setKeepAlive( bool keepAlive)
{
	nlassert(_Sock != INVALID_SOCKET);
	int b = keepAlive?1:0;
	if ( setsockopt( _Sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&b, sizeof(b) ) != 0 )
	{
		throw ESocket( "setKeepAlive failed" );
	}
}

/*
 * Sets/unsets TCP_NODELAY
 */
void CTcpSock::setNoDelay( bool value )
{
	int b = value?1:0;
	if ( setsockopt( _Sock, IPPROTO_TCP, TCP_NODELAY, (char*)&b, sizeof(b) ) != 0 )
	{
		throw ESocket( "setNoDelay failed" );
	}
}


/* Sets a custom TCP Window size (SO_RCVBUF and SO_SNDBUF).
 * You must close the socket is necessary, before calling this method.
 *
 * See http://www.ncsa.uiuc.edu/People/vwelch/net_perf/tcp_windows.html
 */
void CTcpSock::connectWithCustomWindowSize( const CInetAddress& addr, int windowsize )
{
	// Create socket
	if ( _Sock != INVALID_SOCKET )
	{
		nlerror( "Cannot connect with custom window size when already connected" );
	}
	createSocket( SOCK_STREAM, IPPROTO_TCP );

	// Change window size
	if ( setsockopt( _Sock, SOL_SOCKET, SO_SNDBUF, (char*)&windowsize, sizeof(windowsize) ) != 0
	  || setsockopt( _Sock, SOL_SOCKET, SO_RCVBUF, (char*)&windowsize, sizeof(windowsize) ) != 0 )
	{
		throw ESocket( "setWindowSize failed" );
	}

	// Connection
	CSock::connect( addr );
}


/*
 * Returns the TCP Window Size for the current socket
 */
uint32 CTcpSock::getWindowSize()
{
	int windowsize = 0;
	socklen_t len = sizeof( windowsize );

	/* send buffer -- query for buffer size */
	if ( getsockopt( _Sock, SOL_SOCKET, SO_SNDBUF, (char*) &windowsize, &len ) == 0 )
	{
		return windowsize;
	}
	else
	{
		return 0;
	}
}


} // NLNET
