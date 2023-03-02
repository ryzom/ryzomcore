// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/net/udp_sock.h"
#include "nel/net/net_log.h"

#ifdef NL_OS_WINDOWS
#	include <winsock2.h>
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
// Windows includes for `sockaddr_in6` and `WSAStringToAddressW`
#	include <ws2ipdef.h>
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
//#include <fcntl.h>
#	define SOCKET_ERROR -1
#	define INVALID_SOCKET -1
#	define ERROR_NUM errno
#	define ERROR_MSG strerror(errno)
typedef int SOCKET;
#endif

using namespace NLMISC;

namespace NLNET {

inline static int sizeOfSockAddr(const sockaddr_storage &storage)
{
	if (storage.ss_family == AF_INET6)
		return sizeof(sockaddr_in6);
	if (storage.ss_family == AF_INET)
		return sizeof(sockaddr_in);
	return sizeof(storage);
}


/*
 * Constructor
 */
CUdpSock::CUdpSock( bool logging ) :
	CSock( logging ),
	_Bound( false )
{
	// Socket creation
	createSocket( SOCK_DGRAM, IPPROTO_UDP );
}


/** Binds the socket to the specified port. Call bind() for an unreliable socket if the host acts as a server and waits for
 * messages. If the host acts as a client, call sendTo(), there is no need to bind the socket.
 */
void CUdpSock::bind( uint16 port )
{
	CInetAddress addr; // any IP address
	addr.setPort( port );
	bind( addr );
	setLocalAddress(); // will not set the address if the host is multihomed, use bind(CInetAddress) instead
}


/*
 * Same as bind(uint16) but binds on a specified address/port (useful when the host has several addresses)
 */
void CUdpSock::bind( const CInetAddress& addr )
{
	sockaddr_storage sockAddr;
	
	if (!addr.toSockAddrStorage(&sockAddr, _AddressFamily))
	{
		throw ESocket("Cannot bind to an invalid address", false);
	}
	
#ifndef NL_OS_WINDOWS
	// Set Reuse Address On (does not work on Win98 and is useless on Win2000)
	int value = true;
	if ( setsockopt( _Sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value) ) != 0 )
	{
		throw ESocket( "ReuseAddr failed" );
	}
#endif

	// Bind the socket
	if (::bind(_Sock, (sockaddr *)(&sockAddr), sizeOfSockAddr(sockAddr)) != 0)
	{
		if (_AddressFamily == AF_INET6 && addr.getAddress().isAny() && !addr.getAddress().isIPv4())
		{
			// Try to bind to IPv4 Any address if a dual stack listen (default) was attempted and failed
			CInetAddress anyIPv4 = CInetAddress(CIPv6Address::anyIPv4(), addr.port());
			anyIPv4.toSockAddrInet6((sockaddr_in6 *)(&sockAddr));
			if (::bind(_Sock, (sockaddr *)&sockAddr, sizeof(sockaddr_in6)) != 0)
			{
				throw ESocket("Unable to bind listen socket to to port");
			}
			_LocalAddr = anyIPv4;
		}
		else
		{
			throw ESocket("Unable to bind listen socket to port");
		}
	}
	else
	{
		_LocalAddr = addr;
	}
	_Bound = true;
	if ( _Logging )
	{
		LNETL0_DEBUG( "LNETL0: Socket %d bound at %s", _Sock, _LocalAddr.asString().c_str() );
	}
}


/*
 * Sends a message
 */
void CUdpSock::sendTo( const uint8 *buffer, uint len, const CInetAddress& addr )
{
	sockaddr_storage sockAddr;

	if (!addr.toSockAddrStorage(&sockAddr, _AddressFamily))
	{
		throw ESocket("Cannot send datagram to an invalid address");
	}

	//  Send
	if (::sendto(_Sock, (const char *)buffer, len, 0, (sockaddr *)(&sockAddr), sizeOfSockAddr(sockAddr)) != (sint32)len)
	{
		throw ESocket("Unable to send datagram");
	}
	_BytesSent += len;

	if ( _Logging )
	{
		LNETL0_DEBUG( "LNETL0: Socket %d sent %d bytes to %s", _Sock, len, addr.asString().c_str() );
	}

	// If socket is unbound, retrieve local address
	if ( ! _Bound )
	{
		setLocalAddress();
		_Bound = true;
	}

#ifdef NL_OS_WINDOWS
	// temporary by ace to know size of SO_MAX_MSG_SIZE
	static bool first = true;
	if (first)
	{
		uint MMS, SB;
		int  size = sizeof (MMS);
		getsockopt (_Sock, SOL_SOCKET, SO_SNDBUF, (char *)&SB, &size);
		getsockopt (_Sock, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&MMS, &size);
		LNETL0_INFO ("LNETL0: The udp SO_MAX_MSG_SIZE=%u, SO_SNDBUF=%u", MMS, SB);
		first = false;
	}
#endif
}


/*
 * Receives data from the peer. (blocking function)
 */
bool CUdpSock::receive( uint8 *buffer, uint32& len, bool throw_exception )
{
	nlassert( _Connected && (buffer!=NULL) );

	// Receive incoming message
	len = ::recv( _Sock, (char*)buffer, len , 0 );

	// Check for errors (after setting the address)
	if ( ((int)len) == SOCKET_ERROR )
	{
		if ( throw_exception )
			throw ESocket( "Cannot receive data" );
		return false;
	}

	_BytesReceived += len;
	if ( _Logging )
	{
		LNETL0_DEBUG( "LNETL0: Socket %d received %d bytes from peer %s", _Sock, len, _RemoteAddr.asString().c_str() );
	}
	return true;
}


/*
 * Receives data and say who the sender is. (blocking function)
 */
bool CUdpSock::receivedFrom( uint8 *buffer, uint& len, CInetAddress& addr, bool throw_exception )
{
	// Receive incoming message
	sockaddr_storage sockAddr;
	socklen_t saddrlen = sizeof(sockAddr);

	len = ::recvfrom(_Sock, (char *)buffer, len, 0, (sockaddr *)(&sockAddr), &saddrlen);

	// If an error occurs, the saddr is not valid
	// When the remote socket is closed, get sender's address to know who is quitting
	addr.fromSockAddrStorage(&sockAddr);

	// Check for errors (after setting the address)
	if ( ((int)len) == SOCKET_ERROR )
	{
		if ( throw_exception )
			throw ESocket( "Cannot receive data" );
		return false;
	}

	_BytesReceived += len;
	if ( _Logging )
	{
		LNETL0_DEBUG( "LNETL0: Socket %d received %d bytes from %s", _Sock, len, addr.asString().c_str() );
	}
	return true;
}


} // NLNET
