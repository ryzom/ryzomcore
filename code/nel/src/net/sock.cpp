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

#include "nel/net/sock.h"
#include "nel/net/net_log.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/hierarchical_timer.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <winsock2.h>
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

namespace NLNET {


bool CSock::_Initialized = false;


/*
 * ESocket constructor
 */
ESocket::ESocket( const char *reason, bool systemerror, CInetAddress *addr )
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
		smprintf( str, 256, reason, addr->asString().c_str() ); // reason *must* contain "%s"
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
	if ( ! CSock::_Initialized )
	{
#ifdef NL_OS_WINDOWS
		WORD winsock_version = MAKEWORD( 2, 0 );
		WSADATA wsaData;
		if ( WSAStartup( winsock_version, &wsaData ) != 0 )
		{
			throw ESocket( "Winsock initialization failed" );
		}
#endif
		CSock::_Initialized = true;
	}
}

/*
 * Releases the network engine
 */
void CSock::releaseNetwork()
{
#ifdef NL_OS_WINDOWS
	WSACleanup();
#endif
	CSock::_Initialized = false;
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
CSock::CSock( bool logging ) :
	_Sock( INVALID_SOCKET ),
	_Logging( logging ),
	_NonBlocking( false ),
	_BytesReceived( 0 ),
	_BytesSent( 0 ),
	_TimeoutS( 0 ),
	_TimeoutUs( 0 ),
	_MaxReceiveTime( 0 ),
	_MaxSendTime( 0 ),
	_Blocking( false )
	{
	nlassert( CSock::_Initialized );
	/*{
		CSynchronized<bool>::CAccessor sync( &_SyncConnected );
		sync.value() = false;
	}*/
	_Connected = false;
}


/*
 * Construct a CSock object using an existing connected socket descriptor and its associated remote address
 */
CSock::CSock( SOCKET sock, const CInetAddress& remoteaddr ) :
	_Sock( sock ),
	_RemoteAddr( remoteaddr ),
	_Logging( true ),
	_NonBlocking( false ),
	_BytesReceived( 0 ),
	_BytesSent( 0 ),
	_MaxReceiveTime( 0 ),
	_MaxSendTime( 0 )
{
	nlassert( CSock::_Initialized );
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


/*
 * Creates the socket and get a valid descriptor
 */
void CSock::createSocket( int type, int protocol )
{
	nlassert( _Sock == INVALID_SOCKET );

	_Sock = (SOCKET)socket( AF_INET, type, protocol ); // or IPPROTO_IP (=0) ?
	if ( _Sock == INVALID_SOCKET )
	{
		throw ESocket( "Socket creation failed" );
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
	}
}


/*
 * Connection
 */
void CSock::connect( const CInetAddress& addr )
{
	LNETL0_DEBUG( "LNETL0: Socket %d connecting to %s...", _Sock, addr.asString().c_str() );

	// Check address
	if ( ! addr.isValid() )
	{
		throw ESocket( "Unable to connect to invalid address", false );
	}

#ifndef NL_OS_WINDOWS
	// Set Reuse Address On (does not work on Win98 and is useless on Win2000)
	int value = true;
	if ( setsockopt( _Sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value) ) == SOCKET_ERROR )
	{
		throw ESocket( "ReuseAddr failed" );
	}
#endif

	// Connection (when _Sock is a datagram socket, connect establishes a default destination address)
	if ( ::connect( _Sock, (const sockaddr *)(addr.sockAddr()), sizeof(sockaddr_in) ) != 0 )
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

		throw ESocketConnectionFailed( addr );
	}
	setLocalAddress();
	if ( _Logging )
	{
		LNETL0_DEBUG( "LNETL0: Socket %d connected to %s (local %s)", _Sock, addr.asString().c_str(), _LocalAddr.asString().c_str() );
	}
	_RemoteAddr = addr;

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
	sockaddr saddr;
	socklen_t saddrlen = sizeof(saddr);
	if ( getsockname( _Sock, &saddr, &saddrlen ) != 0 )
	{
		throw ESocket( "Unable to find local address" );
	}
	_LocalAddr.setSockAddr( (const sockaddr_in *)&saddr );
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
  socklen_t bufsize;
  getsockopt( _Sock, SOL_SOCKET, SO_SNDBUF, (char*)(&size), &bufsize );
  return size;
}

} // NLNET
