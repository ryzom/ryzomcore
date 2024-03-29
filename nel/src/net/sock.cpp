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

#include "nel/net/sock.h"
#include "nel/net/net_log.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/atomic.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <winsock2.h>
#   include <ws2ipdef.h>
#	include <windows.h>
#	define socklen_t int
#	define ERROR_NUM WSAGetLastError()
#	define ERROR_WOULDBLOCK WSAEWOULDBLOCK

#elif defined NL_OS_UNIX

#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <sys/ioctl.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <fcntl.h>
#	include <cerrno>

#	define SOCKET_ERROR -1
#	define INVALID_SOCKET -1
#	define ERROR_NUM errno
#	define ERROR_WOULDBLOCK EWOULDBLOCK
#	define ERROR_MSG strerror(errno)

// BSD compatible constant
#	ifndef FNDELAY
#		define FNDELAY O_NDELAY
#	endif

typedef int SOCKET;

#endif

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

#if defined(_WIN32_WINNT) && (_WIN32_WINNT < 0x0501)
#define IPPROTO_IPV6 41
#endif

#ifdef NL_OS_WINDOWS
// automatically add the win socket library if you use nel network part
#pragma comment(lib, "ws2_32.lib")
#endif

namespace NLNET {


namespace /* anonymous */ {
NLMISC::CAtomicBool s_Initialized;
#ifdef NL_OS_WINDOWS
NLMISC::CAtomicInt s_WsaInitCount;
#endif
} /* anonymous namespace */

/*
 * ESocket constructor
 */
ESocket::ESocket( const char *reason, bool systemerror, CInetHost *addr )
{
/*it doesnt work on linux, should do something more cool
  	std::stringstream ss;
	ss << "Socket error: " << reason;
	if ( systemerror )
	{
		ss << " (" << ERROR_NUM;
#ifdef NL_OS_UNIX
		ss << ": " << ERROR_MSG;
#endif
		ss << ") " << std::endl;
	}
	_Reason = ss.str();
  */
  	_Reason = "Socket error: ";
	uint errornum = CSock::getLastError();
	char str[256];
	if ( addr != NULL )
	{
		// Version with address
		smprintf( str, 256, reason, addr->toStringLong().c_str() ); // reason *must* contain "%s"
		_Reason += str;
	}
	else
	{
		// Version without address
		_Reason += reason;
	}
	if ( systemerror )
	{
		_Reason += " (";
		smprintf( str, 256, "%d", errornum );
		_Reason += str;
		if ( errornum != 0 )
		{
			_Reason += ": ";
			_Reason += CSock::errorString( errornum );
		}
		_Reason += ")";
	}
	LNETL0_INFO( "LNETL0: Exception will be launched: %s", _Reason.c_str() );

}


/*
 * Initializes the network engine if it is not already done (under Windows, calls WSAStartup()).
 */
void CSock::initNetwork()
{
#ifdef NL_OS_WINDOWS
	if (!s_Initialized.load(NLMISC::TMemoryOrderRelaxed))
	{
		WORD winsock_version = MAKEWORD( 2, 2 );
		WSADATA wsaData;
		if ( WSAStartup( winsock_version, &wsaData ) != 0 )
		{
			throw ESocket( "Winsock initialization failed" );
		}
		s_Initialized = true;
		// Ok if it gets initialized more than once due to multiple threads reaching here,
		// just need to release multiple times!
		++s_WsaInitCount;
	}
#else
	if (!s_Initialized.load(NLMISC::TMemoryOrderRelaxed))
	{
		s_Initialized = true;
	}
#endif
}

/*
 * Releases the network engine
 */
void CSock::releaseNetwork()
{
#ifdef NL_OS_WINDOWS
	s_Initialized = false;
	while (int previous = (s_WsaInitCount--))
	{
		if (previous > 0)
		{
			WSACleanup();
		}
		else // Oops, went too far!
		{
			++s_WsaInitCount;
		}
	}
	// Twice, in case called concurrently with initNetwork (don't do this, though)
	s_Initialized = false;
#else
	s_Initialized = false;
#endif
}


/* Returns the code of the last error that has occurred.
 * Note: This code is platform-dependant. On Unix, it is errno; on Windows it is the Winsock error code.
 * See also errorString()
 */
uint CSock::getLastError()
{
	return (uint)ERROR_NUM;
}


/*
 * Returns a string explaining the network error (see getLastError())
 */
std::string CSock::errorString( uint errorcode )
{
#ifdef NL_OS_WINDOWS
	switch( errorcode )
	{
	case WSAEINTR		 /*10004*/: return "Blocking operation interrupted";
	case WSAEINVAL		 /*10022*/: return "Invalid socket (maybe not bound) or argument";
	case WSAEMFILE		 /*10024*/: return "Too many open sockets";
	case WSAENOTSOCK	 /*10038*/: return "Socket operation on nonsocket (maybe invalid select descriptor)";
	case WSAEMSGSIZE	 /*10040*/: return "Message too long";
	case WSAEADDRINUSE   /*10048*/: return "Address already in use (is this service already running in this computer?)";
	case WSAEADDRNOTAVAIL/*10049*/: return "Address not available";
	case WSAENETDOWN	 /*10050*/: return "Network is down";
	case WSAENETUNREACH  /*10051*/: return "Network is unreachable";
	case WSAECONNRESET   /*10054*/: return "Connection reset by peer";
	case WSAENOBUFS		 /*10055*/: return "No buffer space available; please close applications or reboot";
	case WSAESHUTDOWN	 /*10058*/: return "Cannot send/receive after socket shutdown";
	case WSAETIMEDOUT	 /*10060*/: return "Connection timed-out";
	case WSAECONNREFUSED /*10061*/:	return "Connection refused, the server may be offline";
	case WSAEHOSTUNREACH /*10065*/: return "Remote host is unreachable";
	case WSANOTINITIALISED /*093*/: return "'Windows Sockets' not initialized";
	default:						return "";
	}
#elif defined NL_OS_UNIX
	return std::string( strerror( errorcode ) );
#endif

}



/*
 * Constructor
 */
CSock::CSock(bool logging)
    : _Sock(INVALID_SOCKET)
	, _LocalAddr(false)
	, _RemoteAddr(false)
    , _Logging(logging)
    , _NonBlocking(false)
    , _BytesReceived(0)
    , _BytesSent(0)
    , _TimeoutS(0)
    , _TimeoutUs(0)
    , _MaxReceiveTime(0)
    , _MaxSendTime(0)
    , _Blocking(false)
    , _AddressFamily(AF_UNSPEC)
{
	nlassert(s_Initialized);
	/*{
		CSynchronized<bool>::CAccessor sync( &_SyncConnected );
		sync.value() = false;
	}*/
	_Connected = false;
}

/*
 * Construct a CSock object using an existing connected socket descriptor and its associated remote address
 */
CSock::CSock(SOCKET sock, const CInetAddress &remoteaddr)
    : _Sock(sock)
    , _RemoteAddr(remoteaddr)
    , _Logging(true)
    , _NonBlocking(false)
    , _BytesReceived(0)
    , _BytesSent(0)
    , _MaxReceiveTime(0)
    , _MaxSendTime(0)
    , _AddressFamily(AF_UNSPEC)
{
	nlassert(s_Initialized);
	/*{
		CSynchronized<bool>::CAccessor sync( &_SyncConnected );
		sync.value() = true;
	}*/
	_Connected = true;

	// Check remote address
	if ( ! _RemoteAddr.isValid() )
	{
		throw ESocket( "Could not init a socket object with an invalid address", false );
	}

	// Get local socket name
	setLocalAddress();

#ifdef NL_OS_UNIX
	// We set the close-on-exec flag on the socket to be sure that when
	// we call the exec() to spawn an application in the AES for example,
	// that the AES listen socket will be close and not given to the child.
	// From google:
	// Manipulate the close-on-exec flag to determine if a file descriptor
	// should be closed as part of the normal processing of the exec subroutine.
	// If the flag is set, the file descriptor is closed.
	// If the flag is clear, the file descriptor is left open
	ioctl(_Sock, FIOCLEX, NULL);
	// fcntl should be more portable but not tested fcntl(_Sock, F_SETFD, FD_CLOEXEC);
#endif
}

static SOCKET createSocketInternal(int &family, int familyA, int familyB, int type, int protocol)
{
	// First try IPv6
	SOCKET res = (SOCKET)socket(familyA, type, protocol); // or IPPROTO_IP (=0) ?
	if (res == INVALID_SOCKET)
	{
		// Fallback to IPv4
		nlwarning("Could not create a socket of the preferred socket family %i, create a socket with family %i, the system may lack IPv6 support", familyA , familyB);
		res = (SOCKET)socket(familyB, type, protocol);
		if (res == INVALID_SOCKET)
		{
			family = AF_UNSPEC;
			throw ESocket("Could not create socket");
		}
		family = familyB;
	}
	else
	{
		family = familyA;
	}
	return res;
}

inline static int sizeOfSockAddr(const sockaddr_storage &storage)
{
	if (storage.ss_family == AF_INET6)
		return sizeof(sockaddr_in6);
	if (storage.ss_family == AF_INET)
		return sizeof(sockaddr_in);
	return sizeof(storage);
}

/*
 * Creates the socket and get a valid descriptor
 */
void CSock::createSocket(int type, int protocol)
{
	nlassert(_Sock == INVALID_SOCKET);

	// Create socket, throws in case of failure
	_Sock = createSocketInternal(_AddressFamily, AF_INET6, AF_INET, type, protocol);

	if (_AddressFamily == AF_INET6)
	{
		// Ensure this is dual stack IPv6 and IPv4
		// Disable IPv6 ONLY flag
		int no = 0;
		if (setsockopt(_Sock, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&no, sizeof(no)) != 0)
		{
			nlwarning("Could not disable IPV6_V6ONLY flag, this means we're running on a legacy OS, and IPv4 is preferred");
#ifdef NL_OS_WINDOWS
			closesocket(_Sock);
#elif defined NL_OS_UNIX
			::close(_Sock);
#endif
			_Sock = INVALID_SOCKET;
			_Sock = createSocketInternal(_AddressFamily, AF_INET, AF_INET6, type, protocol);
		}
	}

	if ( _Logging )
	{
//		LNETL0_DEBUG( "LNETL0: Socket %d open (TCP)", _Sock );
	}

#ifdef NL_OS_UNIX
	// We set the close-on-exec flag on the socket to be sure that when
	// we call the exec() to spawn an application in the AES for example,
	// that the AES listen socket will be close and not given the to child.
	// From google:
	// Manipulate the close-on-exec flag to determine if a file descriptor
	// should be closed as part of the normal processing of the exec subroutine.
	// If the flag is set, the file descriptor is closed.
	// If the flag is clear, the file descriptor is left open
	ioctl(_Sock, FIOCLEX, NULL);
	// fcntl should be more portable but not tested fcntl(_Sock, F_SETFD, FD_CLOEXEC);
#endif

}


/*
 * Closes the listening socket
 */
void CSock::close()
{
	if ( _Logging )
	{
		LNETL0_DEBUG( "LNETL0: Socket %d closing for %s at %s", _Sock, _RemoteAddr.asString().c_str(), _LocalAddr.asString().c_str() );
	}
	SOCKET sockToClose = _Sock;
	// preset to invalid to bypass exception in listen thread
	_Sock = INVALID_SOCKET;
#ifdef NL_OS_WINDOWS
	closesocket( sockToClose );
#elif defined NL_OS_UNIX
	::close( sockToClose );
#endif
	_Connected = false;
	_AddressFamily = AF_UNSPEC;
}


/*
 * Destructor
 */
CSock::~CSock()
{
	//nlinfo( "Report for %s socket %s: Max send time: %u Max recv time: %u", _NonBlocking?"non-blocking":"blocking", remoteAddr().asString().c_str(), _MaxSendTime, _MaxReceiveTime );
	//nlinfo( "Max send time: %u", _MaxSendTime);
	if ( _Sock != INVALID_SOCKET )
	{
		if ( _Logging )
		{
			LNETL0_DEBUG( "LNETL0: Socket %d closing for %s at %s", _Sock, _RemoteAddr.asString().c_str(), _LocalAddr.asString().c_str() );
		}

		if ( connected() )
		{
#ifdef NL_OS_WINDOWS
			shutdown( _Sock, SD_BOTH );
		}
		closesocket( _Sock );
#elif defined NL_OS_UNIX
			shutdown( _Sock, SHUT_RDWR );
		}
		::close( _Sock );
#endif
		_Sock = INVALID_SOCKET;
		_AddressFamily = AF_UNSPEC;
	}
}

/*
 * Connection
 */
void CSock::connect(const CInetHost &addrs)
{
	if (_Connected)
	{
		throw ESocket("Already connected", false);
	}

	bool attempted = false;
	bool connected = false;
	_LocalAddr.setNull();
	_RemoteAddr.setNull();
	for (size_t ai = 0; ai < addrs.size(); ++ai)
	{
		const CInetAddress &addr = addrs.addresses()[ai];
		sockaddr_storage sockAddr;

		LNETL0_DEBUG("LNETL0: Socket %d connecting to %s...", _Sock, addrs.toStringLong(ai).c_str());

		// Check address
		if (!addr.toSockAddrStorage(&sockAddr, _AddressFamily))
		{
			continue;
		}

#ifndef NL_OS_WINDOWS
		// Set Reuse Address On (does not work on Win98 and is useless on Win2000)
		int value = true;
		if (setsockopt(_Sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) == SOCKET_ERROR)
		{
			throw ESocket("ReuseAddr failed");
		}
#endif

		attempted = true;
		// Connection (when _Sock is a datagram socket, connect establishes a default destination address)
		if (::connect(_Sock, (const sockaddr *)(&sockAddr), sizeOfSockAddr(sockAddr)) != 0)
		{
			/*		if ( _Logging )
					{
			#ifdef NL_OS_WINDOWS
						nldebug( "Impossible to connect socket %d to %s %s (%d)", _Sock, addr.hostName().c_str(), addr.asIPString().c_str(), ERROR_NUM );
			#elif defined NL_OS_UNIX
						nldebug( "Impossible to connect socket %d to %s %s (%d:%s)", _Sock, addr.hostName().c_str(), addr.asIPString().c_str(), ERROR_NUM, strerror(ERROR_NUM) );
			#endif
					}
			*/
			LNETL0_DEBUG("LNETL0: Socket %d failed to connect to %s", _Sock, addrs.toStringLong(ai).c_str());
			continue;
		}
		setLocalAddress();
		if (_Logging)
		{
			LNETL0_DEBUG("LNETL0: Socket %d connected to %s (local %s)", _Sock, addrs.toStringLong(ai).c_str(), _LocalAddr.asString().c_str());
		}
		_RemoteAddr = addr;
		connected = true;
		break;
	}
	if (!connected)
	{
		if (!attempted)
		{
			throw ESocket("Unable to connect to invalid address", false);
		}
		throw ESocketConnectionFailed(addrs);
	}

	_BytesReceived = 0;
	_BytesSent = 0;

	/*CSynchronized<bool>::CAccessor sync( &_SyncConnected );
	sync.value() = true;*/
	_Connected = true;
}


/*
 * Checks if there is some data to receive
 */
bool CSock::dataAvailable()
{
	if (_Sock == INVALID_SOCKET)
		throw ESocket("CSock::dataAvailable(): invalid socket");
	
	fd_set fdset;
	FD_ZERO( &fdset );
	FD_SET( _Sock, &fdset );
	timeval tv;
	tv.tv_sec = _TimeoutS;
	tv.tv_usec = _TimeoutUs;

	// Test for message received.
	int res = select( _Sock+1, &fdset, NULL, NULL, &tv );
	switch ( res  )
	{
		case  0 : return false;
		case -1 : throw ESocket( "CSock::dataAvailable(): select failed" );
		default : return true;
	}
}


/*
 * Sets the local address
 */
void CSock::setLocalAddress()
{
	sockaddr_storage storage;
	socklen_t saddrlen = sizeof(storage);
	if (getsockname(_Sock, (sockaddr *)(&storage), &saddrlen) != 0)
	{
		throw ESocket("Unable to find local address");
	}
	if (storage.ss_family == AF_INET6)
	{
		_LocalAddr.fromSockAddrInet6((sockaddr_in6 *)(&storage));
	}
	else if (storage.ss_family == AF_INET)
	{
		_LocalAddr.fromSockAddrInet((sockaddr_in *)(&storage));
	}
	else
	{
		throw ESocket("Unknown socket address family");
	}
}


/*
 * Sends data, or returns false if it would block
 */
CSock::TSockResult CSock::send( const uint8 *buffer, uint32& len, bool throw_exception )
{
	TTicks before = CTime::getPerformanceTime();
	len = ::send( _Sock, (const char*)buffer, len, 0 );
	_MaxSendTime = max( (uint32)(CTime::ticksToSecond(CTime::getPerformanceTime()-before)*1000.0f), _MaxSendTime );

//	nldebug ("CSock::send(): Sent %d bytes to %d (%d)", len, _Sock, ERROR_NUM);

	if ( _Logging )
	{
//		LNETL0_DEBUG ("LNETL0: CSock::send(): Sent %d bytes to %d res: %d (%d)", realLen, _Sock, len, ERROR_NUM);
	}

	if ( ((int)len) == SOCKET_ERROR )
	{
		if ( ERROR_NUM == ERROR_WOULDBLOCK )
		{
			H_AUTO(L0SendWouldBlock);
			len = 0;
			//nlSleep(10);
			if (!_Blocking)
			{
				//nldebug("SendWouldBlock - %s / %s Entering snooze mode",_LocalAddr.asString().c_str(),_RemoteAddr.asString().c_str());
				_Blocking= true;
			}
			return Ok;
		}
		if ( throw_exception )
		{
#ifdef NL_OS_WINDOWS
			throw ESocket( NLMISC::toString( "Unable to send data: error %u", GetLastError() ).c_str() );
#else
			throw ESocket( "Unable to send data" );
#endif
		}
		return Error;
	}
	_BytesSent += len;

	if (_Blocking)
	{
		//nldebug("SendWouldBlock - %s / %s Leaving snooze mode",_LocalAddr.asString().c_str(),_RemoteAddr.asString().c_str());
		_Blocking= false;
	}
	return Ok;
}



/*
 * Receives data
 */
CSock::TSockResult CSock::receive( uint8 *buffer, uint32& len, bool throw_exception )
{
	if ( _NonBlocking )
	{
		// Receive incoming message (only the received part)
		TTicks before = CTime::getPerformanceTime();

		sint retLen = ::recv( _Sock, (char*)buffer, len, 0 );

		//nlinfo ("CSock::receive(): NBM Received %d bytes to %d res: %d (%d)", realLen, _Sock, len, ERROR_NUM);

		if ( _Logging )
		{
//			LNETL0_DEBUG ("LNETL0: CSock::receive(): NBM Received %d bytes to %d res: %d (%d)", realLen, _Sock, len, ERROR_NUM);
		}

		_MaxReceiveTime = max( (uint32)(CTime::ticksToSecond(CTime::getPerformanceTime()-before)*1000.0f), _MaxReceiveTime );

		switch (retLen)
		{
			// Graceful disconnection
			case 0 :
			{
				/*{
					CSynchronized<bool>::CAccessor sync( &_SyncConnected );
					sync.value() = false;
				}*/
				_Connected = false;
				if ( throw_exception )
				{
					throw ESocketConnectionClosed();
				}
				return CSock::ConnectionClosed;
			}

			// Socket error or call would block
			case SOCKET_ERROR :
			{
				len = 0;
				if ( ERROR_NUM == ERROR_WOULDBLOCK )
				{
					// Call would block
					return CSock::WouldBlock;
				}
				else
				{
					// Socket error
					if ( throw_exception )
					{
						throw ESocket( "Unable to receive data" );
					}
					return CSock::Error;
				}
			}
		}

		len = (uint32)retLen;
	}
	else // Blocking Mode
	{
		// Receive incoming message, waiting until a complete message has arrived
		uint total = 0;
		sint brecvd;

		while ( total < len )
		{
			TTicks before = CTime::getPerformanceTime();
			brecvd = ::recv( _Sock, (char*)(buffer+total), len-total, 0 );

//			nlinfo ("CSock::receive(): BM Received %d bytes to %d res: %d (%d) total %d", len, _Sock, brecvd, ERROR_NUM, total);

			_MaxReceiveTime = max( (uint32)(CTime::ticksToSecond(CTime::getPerformanceTime()-before)*1000.0f), _MaxReceiveTime );

			switch ( brecvd )
			{
				// Graceful disconnection
				case 0 :
				{
					/*{
						CSynchronized<bool>::CAccessor sync( &_SyncConnected );
						sync.value() = false;
					}*/
					_Connected = false;
					len = total;
					_BytesReceived += len;

					if ( throw_exception )
					{
						throw ESocketConnectionClosed();
					}
					return CSock::ConnectionClosed;
				}

				// Socket error
				case SOCKET_ERROR :
				{
					len = total;
					_BytesReceived += len;

					if ( throw_exception )
					{
						throw ESocket( "Unable to receive data" );
					}
					return CSock::Error;
				}
			}
			total += brecvd;
		}
	}

	/*if ( _Logging )
	{
		LNETL0_DEBUG( "LNETL0: Socket %d received %d bytes", _Sock, len );
	}*/
	_BytesReceived += len;
	return CSock::Ok;
}


/*
 * Sets the socket in nonblocking mode
 */
void CSock::setNonBlockingMode ( bool bm )
{
	if ( _NonBlocking != bm )
	{
#ifdef NL_OS_WINDOWS
		u_long b = bm;
		if ( ioctlsocket( _Sock, FIONBIO, &b ) != 0 )
#else
		if ( fcntl( _Sock, F_SETFL, FNDELAY | fcntl( _Sock, F_GETFL, 0 ) ) == -1 )
#endif
		{
			throw ESocket( "Cannot set nonblocking mode" );
		}
		_NonBlocking = bm;
	}
}


/*
 * Sets the send buffer size
 */
void CSock::setSendBufferSize( sint32 size )
{
  setsockopt( _Sock, SOL_SOCKET, SO_SNDBUF, (char*)(&size), (socklen_t)sizeof(size) );
}

/*
 * Gets the send buffer size
 */
sint32 CSock::getSendBufferSize()
{
  int size = -1;
	socklen_t bufsize = sizeof(size);
	getsockopt(_Sock, SOL_SOCKET, SO_SNDBUF, (char *)(&size), &bufsize);
	return size;
}

} // NLNET
