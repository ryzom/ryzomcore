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

#include "nel/misc/common.h"
#include "nel/misc/wang_hash.h"

#include "nel/net/inet_address.h"
#include "nel/net/sock.h"
#include "nel/net/net_log.h"



#ifdef NL_OS_WINDOWS
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <ws2ipdef.h>
// for Windows 2000 compatibility
#	include <wspiapi.h>

#if !defined(NTDDI_VISTA) || (NTDDI_VERSION < NTDDI_VISTA)

// inet_pton and inet_ntop not defined in winsock DLL before Vista

// taken from http://stackoverflow.com/questions/13731243/what-is-the-windows-xp-equivalent-of-inet-pton-or-inetpton
int inet_pton(int af, const char *src, void *dst)
{
	struct sockaddr_storage ss;
	int size = sizeof(ss);
	char src_copy[INET6_ADDRSTRLEN+1];

	ZeroMemory(&ss, sizeof(ss));
	// stupid non-const API
	strncpy (src_copy, src, INET6_ADDRSTRLEN+1);
	src_copy[INET6_ADDRSTRLEN] = 0;

	if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0)
	{
		switch(af)
		{
			case AF_INET:
			*(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
			return 1;

			case AF_INET6:
			*(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
			return 1;
		}
	}

	return 0;
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
	struct sockaddr_storage ss;
	unsigned long s = size;

	ZeroMemory(&ss, sizeof(ss));
	ss.ss_family = af;

	switch(af)
	{
		case AF_INET:
		((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
		break;

		case AF_INET6:
		((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
		break;

		default:
		return NULL;
	}
	
	// cannot directly use &size because of strict aliasing rules
	return WSAAddressToString((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0 ? dst : NULL;
}

BOOLEAN IN6_IS_ADDR_UNSPECIFIED(CONST IN6_ADDR *a)
{
	//
	// We can't use the in6addr_any variable, since that would
	// require existing callers to link with a specific library.
	//
	return (BOOLEAN)((a->s6_words[0] == 0) &&
		(a->s6_words[1] == 0) &&
		(a->s6_words[2] == 0) &&
		(a->s6_words[3] == 0) &&
		(a->s6_words[4] == 0) &&
		(a->s6_words[5] == 0) &&
		(a->s6_words[6] == 0) &&
		(a->s6_words[7] == 0));
}

#endif

#elif defined NL_OS_UNIX
#	include <unistd.h>
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <netdb.h>
#	define WSAGetLastError() 0
#	define SOCKET_ERROR -1
#endif

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

#ifndef NI_MAXHOST
#	define NI_MAXHOST 1025
#endif

namespace NLNET
{

bool CInetAddress::RetrieveNames = false;


/*
 * Constructor
 */
CInetAddress::CInetAddress()
{
	init();
	
	// IPv4
	_SockAddr->sin_port = 0; // same as htons(0)
	memset( &_SockAddr->sin_addr, 0, sizeof(in_addr) ); // same as htonl(INADDR_ANY)

	// IPv6
	_SockAddr6->sin6_port = 0;
	memset( &_SockAddr6->sin6_addr, 0, sizeof(in6_addr) ); // same as htonl(INADDR_ANY)
}


/*
 * Constructor with IPv4 address, port=0
 */
CInetAddress::CInetAddress( const in_addr *ip, const char *hostname )
{
	init();

	// IPv4
	_SockAddr->sin_port = 0;
	memcpy( &_SockAddr->sin_addr, ip, sizeof(in_addr) );

	// invalid IPv6
	_SockAddr6->sin6_port = 0;
	memset( &_SockAddr6->sin6_addr, 0, sizeof(in6_addr) );

	// get the host name to be displayed
	if(hostname)
	{
		_HostName = hostname;
	}
	else
	{
		updateHostName();
	}

	_Valid = 4;
}


/*
 * Constructor with IPv6 address, port=0
 */
CInetAddress::CInetAddress( const in6_addr *ip, const char *hostname )
{
	init();

	// IPv6
	_SockAddr6->sin6_port = 0;
	memcpy( &_SockAddr6->sin6_addr, ip, sizeof(in6_addr) );

	// invalid IPv4
	_SockAddr->sin_port = 0;
	memset( &_SockAddr->sin_addr, 0, sizeof(in_addr) );

	// get the host name to be displayed
	if(hostname)
	{
		_HostName = hostname;
	}
	else
	{
		updateHostName();
	}

	_Valid = 6;
}


/*
 * Update _HostName from _SockAddr current value 
 */
void CInetAddress::updateHostName()
{
	char host[NI_MAXHOST];

	// if unable to resolve DNS, returns an error and use IP address instead
	sint status = 1;
	
	// check if IPv4 is valid
	if (_SockAddr->sin_addr.s_addr != 0)
	{
		// IPv4
		status = getnameinfo((struct sockaddr *) _SockAddr, sizeof (sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICSERV | NI_NAMEREQD);
	}
	else if (!IN6_IS_ADDR_UNSPECIFIED(&_SockAddr6->sin6_addr))
	{
		// IPv6
		status = getnameinfo((struct sockaddr *) _SockAddr6, sizeof (sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICSERV | NI_NAMEREQD);
	}

	if ( status )
	{
		_HostName = ipAddress();
	}
	else
	{
		_HostName = string( host );
	}
}


/*
 * Alternate constructor (calls setByName())
 */
CInetAddress::CInetAddress( const std::string& hostName, uint16 port )
{
	init();
	setPort( port );
	setByName( hostName );
}


/*
 * Alternate constructor (calls setNameAndPort())
 */
CInetAddress::CInetAddress( const std::string& hostNameAndPort )
{
	init();
	setNameAndPort( hostNameAndPort );
}


/*
 * Copy constructor
 */
CInetAddress::CInetAddress( const CInetAddress& other )
{
	init();
	_HostName = other._HostName;
	memcpy( _SockAddr, other._SockAddr, sizeof( *_SockAddr ) );
	memcpy( _SockAddr6, other._SockAddr6, sizeof( *_SockAddr6 ) );
	_Valid = other._Valid;
}


/*
 * Assignment operator
 */
CInetAddress& CInetAddress::operator=( const CInetAddress& other )
{
	_HostName = other._HostName;
	memcpy( _SockAddr, other._SockAddr, sizeof( *_SockAddr ) );
	memcpy( _SockAddr6, other._SockAddr6, sizeof( *_SockAddr6 ) );
	_Valid = other._Valid;
	return *this;
}


/*
 * Comparison == operator
 */
bool operator==( const CInetAddress& a1, const CInetAddress& a2 )
{
	// Compares the sockaddr structure except the last 8 bytes equal to zero.
	// FIXME: return ( memcmp( a1._SockAddr, a2._SockAddr, sizeof(sockaddr_in)-8 ) == 0 );
	if (a1._Valid != a2._Valid)
	{
		return false;
	}
	else if (a1._Valid == 4)
	{
		return (a1._SockAddr->sin_port == a2._SockAddr->sin_port)
			&& (a1._SockAddr->sin_addr.s_addr == a2._SockAddr->sin_addr.s_addr);
	}
	else if (a1._Valid == 6)
	{
		return (a1._SockAddr6->sin6_port == a2._SockAddr6->sin6_port)
			&& (a1._SockAddr6->sin6_addr.s6_addr == a2._SockAddr6->sin6_addr.s6_addr);
	}
	return a1.port() == a2.port();
}


/*
 * Comparison < operator
 */
bool operator<( const CInetAddress& a1, const CInetAddress& a2 )
{
	if (a1._Valid != a2._Valid)
	{
		return a1._Valid > a2._Valid;
	}
	else if (a1.isLoopbackIPAddress() != a2.isLoopbackIPAddress())
	{
		return a1.isLoopbackIPAddress();
	}
	else if (a1._Valid == 4)
	{
		if (a1._SockAddr->sin_addr.s_addr == a2._SockAddr->sin_addr.s_addr)
		{
			return (a1.port() < a2.port());
		}
		else
		{
			return (a1._SockAddr->sin_addr.s_addr < a2._SockAddr->sin_addr.s_addr);
		}
	}
	else if (a1._Valid == 6)
	{
		if (a1._SockAddr6->sin6_addr.s6_addr == a2._SockAddr6->sin6_addr.s6_addr)
		{
			return (a1.port() < a2.port());
		}
		else
		{
			return (a1._SockAddr6->sin6_addr.s6_addr < a2._SockAddr6->sin6_addr.s6_addr);
		}
	}
	return a1.port() < a2.port();
}


/*
 * Constructor contents
 */
void CInetAddress::init()
{
	CSock::initNetwork();

	_Valid = 0;

	// IPv4
	_SockAddr = new sockaddr_in;
	memset(_SockAddr, 0, sizeof(sockaddr_in));
	_SockAddr->sin_family = AF_INET;

	// IPv6
	_SockAddr6 = new sockaddr_in6;
	memset(_SockAddr6, 0, sizeof(sockaddr_in6));
	_SockAddr6->sin6_family = AF_INET6;
}


/*
 * Destructor
 */
CInetAddress::~CInetAddress()
{
	delete _SockAddr;
	delete _SockAddr6;
	// _Valid = 0;
}

/*
 * Sets hostname and port (ex: www.nevrax.com:80, 192.168.0.2:80, [::1]:80)
 */
void CInetAddress::setNameAndPort(const std::string &hostNameAndPort)
{
	string::size_type pos6end = hostNameAndPort.find_last_of(']');
	string::size_type pos = hostNameAndPort.find_last_of(':');
	if (pos != string::npos && (pos6end == string::npos || pos > pos6end))
	{
		uint16 port;
		fromString(hostNameAndPort.substr(pos + 1), port);
		setPort(port);
		if (pos6end != string::npos)
		{
			string::size_type pos6begin = hostNameAndPort.find_first_of('[');
			setByName(hostNameAndPort.substr(pos6begin + 1, pos6end - pos6begin - 1));
		}
		else
		{
			setByName(hostNameAndPort.substr(0, pos));
		}
	}
	else
	{
		setPort(0);
		setByName(hostNameAndPort);
	}
}


/*
 * Resolves a name
 */
CInetAddress& CInetAddress::setByName(const std::string& hostName)
{
	// invalid IPv4
	memset(&_SockAddr->sin_addr, 0, sizeof(in_addr));

	// invalid IPv6
	memset(&_SockAddr6->sin6_addr, 0, sizeof(in6_addr));

	if (hostName.empty())
	{
		_Valid = 0;
		_HostName.clear();
		return *this;
	}

	// Try to convert directly for addresses such as a.b.c.d and a:b:c:d:e:f:g:h
	in_addr ipv4;
	sint res = inet_pton(AF_INET, hostName.c_str(), &ipv4);

	if (res == 1)
	{
		// hostname is a valid IPv4
		memcpy(&_SockAddr->sin_addr, &ipv4, sizeof(in_addr));
		_Valid = 4;
	}
	else
	{
		in6_addr ipv6;
		res = inet_pton(AF_INET6, hostName.c_str(), &ipv6);

		if (res == 1)
		{
			// hostname is a valid IPv6
			memcpy(&_SockAddr6->sin6_addr, &ipv6, sizeof(in6_addr));
			_Valid = 6;
		}
	}

	if (res == 1)
	{
		// use IPv4 or IPv6 as hostname
		_HostName = hostName;
	}
	else
	{
		// Otherwise use the traditional DNS look-up
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
		hints.ai_socktype = SOCK_STREAM;

		struct addrinfo *res = NULL;
		sint status = getaddrinfo(hostName.c_str(), NULL, &hints, &res);

		if (status)
		{
			_Valid = 0;
			_HostName.clear();
			LNETL0_DEBUG( "LNETL0: Network error: resolution of hostname '%s' failed: %s", hostName.c_str(), gai_strerror(status) );
			// return *this;
			throw ESocket( (string("Hostname resolution failed for ")+hostName).c_str() );
		}

		// hostname is valid, use it
		_HostName = hostName;

		struct addrinfo *p = res;

		// process all addresses, use the first one
		while (p != NULL)
		{
			// check address family
			if (p->ai_family == AF_INET)
			{
				// ipv4
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
				memcpy(&_SockAddr->sin_addr, &ipv4->sin_addr, sizeof(in_addr));
				memset(&_SockAddr6->sin6_addr, 0, sizeof(in6_addr));
				_Valid = 4;
				break;
			}
			else if (p->ai_family == AF_INET6)
			{
				// ipv6
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
				memcpy(&_SockAddr6->sin6_addr, &ipv6->sin6_addr, sizeof(in6_addr));
				memset(&_SockAddr->sin_addr, 0, sizeof(in_addr));
				_Valid = 6;
				break;
			}

			// process next address
			p = p->ai_next;
		}

		// free the linked list
		freeaddrinfo(res);
	}

	return *this;
}


/*
 * Sets port
 */
void CInetAddress::setPort(uint16 port)
{
	_SockAddr->sin_port = htons(port);
	_SockAddr6->sin6_port = htons(port);
}


/* Sets internal socket address directly (contents is copied).
 * It also retrieves the host name if CInetAddress::RetrieveNames is true.
 */
void CInetAddress::setSockAddr( const sockaddr_in* saddr )
{
	memcpy(_SockAddr, saddr, sizeof(*saddr) );

	// invalid IPv6
	memset(&_SockAddr6->sin6_addr, 0, sizeof(in6_addr));

	// Get host name
	// Warning: when it can't find it, it take more than 4 seconds
	if ( CInetAddress::RetrieveNames )
	{
		updateHostName();
	}

	_Valid = 4;
}


/* Sets internal socket address directly (contents is copied).
 * It also retrieves the host name if CInetAddress::RetrieveNames is true.
 */
void CInetAddress::setSockAddr6( const sockaddr_in6* saddr6 )
{
	memcpy( _SockAddr6, saddr6, sizeof(*saddr6) );

	// invalid IPv4
	memset(&_SockAddr->sin_addr, 0, sizeof(in_addr));

	// Get host name
	// Warning: when it can't find it, it take more than 4 seconds
	if ( CInetAddress::RetrieveNames )
	{
		updateHostName();
	}

	_Valid = 6;
}


/*
 * Returns if object (address and port) is valid
 */
bool CInetAddress::isValid() const
{
	return ( _Valid && _SockAddr->sin_port!=0 ); // same as ntohs(0)
}


/*
 * Returns internal IPv4 socket address (read only)
 */
const sockaddr_in *CInetAddress::sockAddr() const
{
	return _SockAddr;
}


/*
 * Returns internal IPv6 socket address (read only)
 */
const sockaddr_in6 *CInetAddress::sockAddr6() const
{
	return _SockAddr6;
}

bool CInetAddress::isIPAddressEqual(const CInetAddress &other) const
{
	if (other._Valid != _Valid)
	{
		return false;
	}
	if (_Valid == 4)
	{
		return (memcmp(&_SockAddr->sin_addr, &other._SockAddr->sin_addr, sizeof(in_addr)) == 0);
	}
	else if (_Valid == 6)
	{
		return (memcmp(&_SockAddr6->sin6_addr, &other._SockAddr6->sin6_addr, sizeof(in6_addr)) == 0);
	}
	return false;
}

size_t CInetAddress::hash() const
{
	if (sizeof(size_t) == sizeof(uint64))
	{
		uint64 hash = wangHash64((uint64)port());
		if (_Valid == 4)
		{
			hash = wangHash(hash ^ (uint64)_SockAddr->sin_addr.s_addr);
		}
		else
		{
			uint64 *addr = reinterpret_cast<uint64 *>(_SockAddr6->sin6_addr.s6_addr);
			hash = wangHash(hash ^ addr[0]);
			hash = wangHash(hash ^ addr[1]);
		}
		return (size_t)hash;
	}
	else
	{
		uint32 hash = wangHash((uint32)port());
		if (_Valid == 4)
		{
			hash = wangHash(hash ^ (uint32)_SockAddr->sin_addr.s_addr);
		}
		else
		{
			uint32 *addr = reinterpret_cast<uint32 *>(_SockAddr6->sin6_addr.s6_addr);
			hash = wangHash(hash ^ addr[0]);
			hash = wangHash(hash ^ addr[1]);
			hash = wangHash(hash ^ addr[2]);
			hash = wangHash(hash ^ addr[3]);
		}
		return (size_t)hash;
	}
}

/*
 * Returns internal IP address
 */
uint32 CInetAddress::internalIPv4Address() const
{
	return _SockAddr->sin_addr.s_addr;
}

uint32 CInetAddress::internalNetV4Address() const
{
	uint32 ip = internalIPv4Address();
	if ((ip&0x00000080) == 0)
	{
		// A class
		return ip & 0x000000FF;
	}
	else if ((ip&0x00000040) == 0)
	{
		// B class
		return ip & 0x0000FFFF;
	}
	else if ((ip&0x00000020) == 0)
	{
		// C class
		return ip & 0x00FFFFFF;
	}
	else if ((ip&0x00000010) == 0)
	{
		// D class
		return ip & 0xFFFFFFFF;
	}
	else
	{
		return ip;
	}
}

/*
 * Returns readable IP address. (ex: "195.68.21.195")
 */
string CInetAddress::ipAddress() const
{
	// longer size is IPv6
	char straddr[INET6_ADDRSTRLEN];
	const char *name = _SockAddr->sin_addr.s_addr ? inet_ntop(AF_INET, &_SockAddr->sin_addr, straddr, INET_ADDRSTRLEN) : NULL;

	// IPv4 is invalid, return IPv6
	if (name == NULL || strcmp(name, "0.0.0.0") == 0) name = inet_ntop(AF_INET6, &_SockAddr6->sin6_addr, straddr, INET6_ADDRSTRLEN);

	return name ? string (name) : "";
}


/*
 * Returns host name. (ex: "www.nevrax.org")
 */
const string& CInetAddress::hostName() const
{
	return _HostName;
}


/*
 * Returns port
 */
uint16 CInetAddress::port() const
{
	return ntohs( _SockAddr->sin_port );
}


/*
 * Returns hostname and port as a string. (ex: "www.nevrax.org:80 (195.68.21.195)")
 */
std::string CInetAddress::asString() const
{
	return hostName() + ":" + NLMISC::toString(port()) + " (" + ipAddress() + ")";
}


/*
 * Returns IP address and port as a string. (ex: "195.68.21.195:80")
 */
std::string CInetAddress::asIPString() const
{
	return (_Valid == 6 ? "[" : "") + ipAddress() + (_Valid == 6 ? "]:" : ":") + NLMISC::toString(port());
}


/*
 * Serialize
 */
void CInetAddress::serial( NLMISC::IStream& s )
{
	NLMISC::CMemStream *ms = dynamic_cast<NLMISC::CMemStream*>(&s);
	if ( ms && ms->stringMode() )
	{
		// String stream
		string addrs;
		if ( ms->isReading() )
		{
			ms->serial( addrs );
			setNameAndPort( addrs );
		}
		else
		{
			addrs = asIPString();
			ms->serial( addrs );
		}
		s.serial( _Valid );
	}
	else
	{
		// Binary stream
		s.serialBuffer((uint8 *)_SockAddr, sizeof(*_SockAddr)); // this is possible only because the contents of _SockAddr is platform-independant !
		s.serialBuffer((uint8 *)_SockAddr6, sizeof(*_SockAddr6)); // this is possible only because the contents of _SockAddr6 is platform-independant !
		s.serial(_Valid);

		if (s.isReading())
		{
			if (_Valid == 4)
			{
				// retreive the fullname
				setSockAddr(_SockAddr);
			}
			else if (_Valid == 6)
			{
				// retreive the fullname
				setSockAddr6(_SockAddr6);
			}
		}
	}
}


/*
 * Creates a CInetAddress object with local host address, port=0
 */
CInetAddress CInetAddress::localHost()
{
	const uint maxlength = 80;
	char localhost [maxlength];
	if ( gethostname( localhost, maxlength ) != 0 )
		throw ESocket( "Unable to get local hostname" );
	CInetAddress localaddr = CInetAddress( string(localhost) );

	if ( localaddr.ipAddress() == "127.0.0.1" )
	{
		nlwarning ("LNETL0: No network card detected! using localhost (127.0.0.1)");
	}

	return localaddr;
}

/// Creates a CInetAddress object with a loopback address, port=0
CInetAddress CInetAddress::loopback()
{
	// Loop through local addresses, only return IPv6 loopback if we have IPv6 hosts
	std::vector<CInetAddress> addr = localAddresses(true);
	std::sort(addr.begin(), addr.end());
	
	for (uint i = 0; i < addr.size(); i++)
	{
		if (addr[i].isLoopbackIPAddress())
		{
			return addr[i];
		}
	}

	return loopbackIPv4();
}

CInetAddress CInetAddress::loopbackIPv4()
{
	// 1. Get local host name
	const uint maxlength = 80;
	char localhost [maxlength];
	if (gethostname(localhost, maxlength) == SOCKET_ERROR)
	{
		throw ESocket("Unable to get local hostname");
	}

	struct in_addr psin_addrIPv4;
	psin_addrIPv4.s_addr = htonl(INADDR_LOOPBACK);
	return CInetAddress(&psin_addrIPv4, localhost);
}

CInetAddress CInetAddress::loopbackIPv6()
{
	// 1. Get local host name
	const uint maxlength = 80;
	char localhost [maxlength];
	if (gethostname(localhost, maxlength) == SOCKET_ERROR)
	{
		throw ESocket("Unable to get local hostname");
	}

	struct in6_addr psin_addrIPv6 = IN6ADDR_LOOPBACK_INIT;
	return CInetAddress(&psin_addrIPv6, localhost);
}


/* Returns the list of the local host addresses (with port=0)
 * (especially useful if the host is multihomed)
 */
std::vector<CInetAddress> CInetAddress::localAddresses(bool loopback)
{
	// 1. Get local host name
	const uint maxlength = 80;
	char localhost [maxlength];
	if ( gethostname( localhost, maxlength ) == SOCKET_ERROR )
	{
		throw ESocket( "Unable to get local hostname" );
	}

	// 2. Get address list
	vector<CInetAddress> vect;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *res = NULL;
	sint status = getaddrinfo(localhost, NULL, &hints, &res);

	if (status)
	{
		// will come here if the local hostname (/etc/hostname in Linux) is not the real name
		throw ESocket( toString("Hostname resolution failed for %s", localhost).c_str() );
	}

	struct addrinfo *p = res;

	// for loopback ipv4
	bool IPv4LoopbackAdded = false;

	// for loopback ipv6
	bool IPv6LoopbackAdded = false;

	// process all addresses
	while (p != NULL)
	{
		// check address family
		if (p->ai_family == AF_INET)
		{
			// loopback ipv4
			if (!IPv4LoopbackAdded && loopback)
			{
				// add loopback address only once
				struct in_addr psin_addrIPv4;
				psin_addrIPv4.s_addr = htonl(INADDR_LOOPBACK);
				vect.push_back(CInetAddress(&psin_addrIPv4, localhost));

				IPv4LoopbackAdded = true;
			}
			
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			
			if (ipv4->sin_addr.s_addr != htonl(INADDR_LOOPBACK))
			{
				vect.push_back(CInetAddress(&ipv4->sin_addr, localhost));
			}
		}
		else if (p->ai_family == AF_INET6)
		{
			// loopback ipv6
			if (!IPv6LoopbackAdded && loopback)
			{
				// add loopback address only once
				struct in6_addr psin_addrIPv6 = IN6ADDR_LOOPBACK_INIT;
				vect.push_back(CInetAddress(&psin_addrIPv6, localhost));

				IPv6LoopbackAdded = true;
			}

			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;

			// add if not loopback
			if (!IN6_IS_ADDR_LOOPBACK(&ipv6->sin6_addr))
			{
				vect.push_back(CInetAddress(&ipv6->sin6_addr, localhost));
			}
		}

		// process next address
		p = p->ai_next;
	}

	// free the linked list
	freeaddrinfo(res);

	if(vect.empty())
	{
		throw ESocket(toString("No network card detected for %s", localhost).c_str() );
	}

	return vect;
}

/// Loopback addresses first, IPv6 first, keep same order otherwise O(4*N)
std::vector<CInetAddress> CInetAddress::sortPriority(std::vector<CInetAddress> addr)
{
	std::vector<CInetAddress> res;
	res.reserve(addr.size());
	for (size_t i = 0; i < addr.size(); i++)
		if (addr[i].isLoopbackIPAddress() && addr[i].isIPv6())
			res.push_back(addr[i]);
	for (size_t i = 0; i < addr.size(); i++)
		if (addr[i].isLoopbackIPAddress() && addr[i].isIPv4())
			res.push_back(addr[i]);
	for (size_t i = 0; i < addr.size(); i++)
		if (!addr[i].isLoopbackIPAddress() && addr[i].isIPv6())
			res.push_back(addr[i]);
	for (size_t i = 0; i < addr.size(); i++)
		if (!addr[i].isLoopbackIPAddress() && addr[i].isIPv4())
			res.push_back(addr[i]);
	return res;
}

#if 0
bool CInetAddress::is127001 () const
{
	return (internalIPAddress () == htonl(0x7F000001));
}
#endif

bool CInetAddress::isLoopbackIPAddress () const
{
	if (_Valid == 4)
	{
		return (_SockAddr->sin_addr.s_addr == htonl(INADDR_LOOPBACK));
	}
	else if (_Valid == 6)
	{
		return (IN6_IS_ADDR_LOOPBACK(&_SockAddr6->sin6_addr));
	}
	else
	{
		return false;
	}
}


std::string vectorCInetAddressToString(const std::vector<CInetAddress> &addrs)
{
	string str;

	for (uint i = 0; i < addrs.size(); i++)
	{
		if (i != 0)
			str += " ";
		str += addrs[i].asString().c_str ();
	}
	return str;
}

uint32 stringToInternalIPAddress (const std::string &addr)
{
	return inet_addr( addr.c_str() );
}

std::string internalIPAddressToString (uint32 addr)
{
	string res;
	res = toString((addr)&0xFF);
	res += ".";
	res += toString((addr>>8)&0xFF);
	res += ".";
	res += toString((addr>>16)&0xFF);
	res += ".";
	res += toString((addr>>24)&0xFF);
	return res;
}


} // NLNET
