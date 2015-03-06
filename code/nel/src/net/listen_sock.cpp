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

#include "nel/net/listen_sock.h"
#include "nel/net/net_log.h"


#ifdef NL_OS_WINDOWS

#ifndef NL_COMP_MINGW
#	define NOMINMAX
#endif
#include <windows.h>
typedef sint socklen_t;

#elif defined NL_OS_UNIX

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cerrno>
#include <fcntl.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
typedef int SOCKET;

#endif


using namespace std;


namespace NLNET
{


/*
 * Constructor
 */
CListenSock::CListenSock() : CTcpSock(), _Bound( false )
{
	// Create socket
	createSocket( SOCK_STREAM, IPPROTO_TCP );

	setBacklog( -1 );
}


/*
 * Prepares to receive connections on a specified port
 */
void CListenSock::init( uint16 port )
{
    // Use any address
	CInetAddress localaddr; // By default, INETADDR_ANY (useful for gateways that have several ip addresses)
	localaddr.setPort( port );
	init( localaddr );

	// Now set the address visible from outside
	_LocalAddr = CInetAddress::localHost();
	_LocalAddr.setPort( port );
	LNETL0_DEBUG( "LNETL0: Socket %d listen socket is at %s", _Sock, _LocalAddr.asString().c_str() );
}


/*
 * Prepares to receive connections on a specified address/port (useful when the host has several addresses)
 */
void CListenSock::init( const CInetAddress& addr )
{
	if ( ! addr.isValid() )
	{
		LNETL0_DEBUG( "LNETL0: Binding listen socket to any address, port %hu", addr.port() );
	}

#ifndef NL_OS_WINDOWS
	// Set Reuse Address On (does not work on Win98 and is useless on Win2000)
	int value = true;
	if ( setsockopt( _Sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value) ) == SOCKET_ERROR )
	{
		throw ESocket( "ReuseAddr failed" );
	}
#endif

	// Bind socket to port
	if ( ::bind( _Sock, (const sockaddr *)addr.sockAddr(), sizeof(sockaddr_in) ) != 0 )
	{
		throw ESocket( "Unable to bind listen socket to port" );
	}
	_LocalAddr = addr;
	_Bound = true;

	// Listen
	if ( ::listen( _Sock, _BackLog ) != 0 ) // SOMAXCONN = maximum length of the queue of pending connections
	{
		throw ESocket( "Unable to listen on specified port" );
	}
//	LNETL0_DEBUG( "LNETL0: Socket %d listening at %s", _Sock, _LocalAddr.asString().c_str() );
}


/*
 * Accepts an incoming connection, and creates a new socket
 */
CTcpSock *CListenSock::accept()
{
	// Accept connection
	sockaddr_in saddr;
	socklen_t saddrlen = (socklen_t)sizeof(saddr);
	SOCKET newsock = (SOCKET)::accept( _Sock, (sockaddr*)&saddr, &saddrlen );
	if ( newsock == INVALID_SOCKET )
	{
		if (_Sock == INVALID_SOCKET)
			// normal case, the listen sock have been closed, just return NULL.
			return NULL;

	  /*LNETL0_INFO( "LNETL0: Error accepting a connection");
	  // See accept() man on Linux
	  newsock = ::accept( _Sock, (sockaddr*)&saddr, &saddrlen );
	  if ( newsock == INVALID_SOCKET )*/
	    {
			throw ESocket( "Accept returned an invalid socket");
	    }
	}

	// Construct and save a CTcpSock object
	CInetAddress addr;
	addr.setSockAddr( &saddr );
	LNETL0_DEBUG( "LNETL0: Socket %d accepted an incoming connection from %s, opening socket %d", _Sock, addr.asString().c_str(), newsock );
	CTcpSock *connection = new CTcpSock( newsock, addr );
	return connection;
}


/*
 * Sets the number of the pending connections queue. -1 for the maximum possible value.
 */
void CListenSock::setBacklog( sint backlog )
{
	if ( backlog == -1 )
	{
		_BackLog = SOMAXCONN; // SOMAXCONN = maximum length of the queue of pending connections
	}
	else
	{
		_BackLog = backlog;
	}
	if ( _Bound )
	{
		if ( ::listen( _Sock, _BackLog ) != 0 )
		{
			throw ESocket( "Unable to listen on specified port, while changing backlog" );
		}
	}
}


} // NLNET
