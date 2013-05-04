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

#include "nel/net/inet_address.h"
#include "nel/net/sock.h"
#include "nel/net/net_log.h"



#ifdef NL_OS_WINDOWS
#	include <winsock2.h>
#	include <ws2tcpip.h>
// for Windows 2000 compatibility
#	include <wspiapi.h>
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
	_SockAddr->sin_port = 0; // same as htons(0)
	memset( &_SockAddr->sin_addr, 0, sizeof(in_addr) ); // same as htonl(INADDR_ANY)
}


/*
 * Constructor with ip address, port=0
 */
CInetAddress::CInetAddress( const in_addr *ip, const char *hostname )
{
	init();
	_SockAddr->sin_port = 0;
	memcpy( &_SockAddr->sin_addr, ip, sizeof(in_addr) );

	// get the host name to be displayed
	if(hostname)
	{
		_HostName = hostname;
	}
	else
	{
		updateHostName();
	}
	_Valid = true;
}


/*
 * Update _HostName from _SockAddr current value 
 */
void CInetAddress::updateHostName()
{
	char host[NI_MAXHOST];

	sint status = getnameinfo((struct sockaddr *) _SockAddr, sizeof (struct sockaddr), host, NI_MAXHOST, NULL, 0, NI_NUMERICSERV);

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
	_Valid = other._Valid;
}


/*
 * Assignment operator
 */
CInetAddress& CInetAddress::operator=( const CInetAddress& other )
{
	_HostName = other._HostName;
	memcpy( _SockAddr, other._SockAddr, sizeof( *_SockAddr ) );
	_Valid = other._Valid;
	return *this;
}


/*
 * Comparison == operator
 */
bool operator==( const CInetAddress& a1, const CInetAddress& a2 )
{
	// Compares the sockaddr structure except the last 8 bytes equal to zero.
	return ( memcmp( a1._SockAddr, a2._SockAddr, sizeof(sockaddr_in)-8 ) == 0 );
}


/*
 * Comparison < operator
 */
bool operator<( const CInetAddress& a1, const CInetAddress& a2 )
{
	if ( a1._SockAddr->sin_addr.s_addr == a2._SockAddr->sin_addr.s_addr )
	{
		return ( a1.port() < a2.port() );
	}
	else
	{
		return ( a1._SockAddr->sin_addr.s_addr < a2._SockAddr->sin_addr.s_addr );
	}
}


/*
 * Constructor contents
 */
void CInetAddress::init()
{
	CSock::initNetwork();

	_Valid = false;

	_SockAddr = new sockaddr_in;
	_SockAddr->sin_family = AF_INET;
	memset( _SockAddr->sin_zero, 0, 8 );
}


/*
 * Destructor
 */
CInetAddress::~CInetAddress()
{
	delete _SockAddr;
	// _Valid = false;
}

/*
 * Sets hostname and port (ex: www.nevrax.com:80)
 */
void CInetAddress::setNameAndPort( const std::string& hostNameAndPort )
{
	string::size_type pos = hostNameAndPort.find_first_of (':');
	if (pos != string::npos)
	{
		uint16 port;
		fromString(hostNameAndPort.substr(pos + 1), port);
		setPort( port );
	}
	else
	{
		setPort( 0 );
	}

	// if pos == -1, it will copy all the string
	setByName( hostNameAndPort.substr (0, pos) );
}


/*
 * Resolves a name
 */
CInetAddress& CInetAddress::setByName( const std::string& hostName )
{
	// Try to convert directly for addresses such as a.b.c.d
	in_addr iaddr;
	iaddr.s_addr = inet_addr( hostName.c_str() );
	if ( iaddr.s_addr == INADDR_NONE )
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
			_Valid = false;
			LNETL0_DEBUG( "LNETL0: Network error: resolution of hostname '%s' failed: %s", hostName.c_str(), gai_strerror(status) );
			// return *this;
			throw ESocket( (string("Hostname resolution failed for ")+hostName).c_str() );
		}

		struct addrinfo *p = res;

		// process all addresses
		while (p != NULL)
		{
			// check address family
			if (p->ai_family == AF_INET)
			{
				// ipv4
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

				// convert the IP to a string
				_HostName = string(inet_ntoa(ipv4->sin_addr));
				memcpy( &_SockAddr->sin_addr, &ipv4->sin_addr, sizeof(in_addr) );
			}
			else if (p->ai_family == AF_INET6)
			{
				// ipv6
				// TODO: modify class to be able to handle IPv6

				// struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;

				// convert the IP to a string
				// inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
				// memcpy( &_SockAddr->sin_addr, &ipv6->sin_addr, sizeof(in_addr) );
			}

			// process next address
			p = p->ai_next;
		}
 
		// free the linked list
		freeaddrinfo(res);
	}
	else
	{
		_HostName = hostName;
		memcpy( &_SockAddr->sin_addr, &iaddr, sizeof(iaddr) );
	}
	_Valid = true;
	return *this;
}


/*
 * Sets port
 */
void CInetAddress::setPort( uint16 port )
{
	_SockAddr->sin_port = htons( port );

}


/* Sets internal socket address directly (contents is copied).
 * It also retrieves the host name if CInetAddress::RetrieveNames is true.
 */
void CInetAddress::setSockAddr( const sockaddr_in* saddr )
{
	memcpy( _SockAddr, saddr, sizeof(*saddr) );

	// Get host name
	// Warning: when it can't find it, it take more than 4 seconds
	if ( CInetAddress::RetrieveNames )
	{
		updateHostName();
	}
	_Valid = true;
}


/*
 * Returns if object (address and port) is valid
 */
bool CInetAddress::isValid() const
{
	return ( _Valid && _SockAddr->sin_port!=0 ); // same as ntohs(0)
}


/*
 * Returns internal socket address (read only)
 */
const sockaddr_in *CInetAddress::sockAddr() const
{
	return _SockAddr;
}


/*
 * Returns internal IP address
 */
uint32 CInetAddress::internalIPAddress() const
{
	return _SockAddr->sin_addr.s_addr;
}

uint32 CInetAddress::internalNetAddress() const
{
	uint32 ip = internalIPAddress();
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
	/*stringstream ss; // or use inet_ntoa
	ss << inet_ntoa ( _SockAddr->sin_addr );
	return ss.str();*/
	const char *name = inet_ntoa ( _SockAddr->sin_addr );
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
//	stringstream ss;
//	ss << hostName() << ":" << port() << " (" << ipAddress() << ")";
//	return ss.str();
	return hostName() + ":" + NLMISC::toString(port()) + " (" + ipAddress() + ")";
}


/*
 * Returns IP address and port as a string. (ex: "195.68.21.195:80")
 */
std::string CInetAddress::asIPString() const
{
//	stringstream ss;
//	ss << ipAddress() << ":" << port();
//	return ss.str();
	return ipAddress() + ":" + NLMISC::toString(port());
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
		s.serialBuffer( (uint8*)_SockAddr, sizeof(*_SockAddr) ); // this is possible only because the contents of _SockAddr is platform-independant !
		s.serial( _Valid );

		if(_Valid)
		{
			// retreive the fullname
			setSockAddr (_SockAddr);
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


/* Returns the list of the local host addresses (with port=0)
 * (especially useful if the host is multihomed)
 */
std::vector<CInetAddress> CInetAddress::localAddresses()
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
		throw ESocket( (string("Hostname resolution failed for ")+string(localhost)).c_str() );
	}

	struct addrinfo *p = res;

	// process all addresses
	while (p != NULL)
	{
		// check address family
		if (p->ai_family == AF_INET)
		{
			// ipv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

			vect.push_back( CInetAddress( &ipv4->sin_addr, localhost ) );
		}
		else if (p->ai_family == AF_INET6)
		{
			// ipv6
			// TODO: modify class to be able to handle IPv6

			// struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;

			// convert the IP to a string
			// inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
			// memcpy( &_SockAddr->sin_addr, &ipv6->sin_addr, sizeof(in_addr) );
		}

		// process next address
		p = p->ai_next;
	}

	// free the linked list
	freeaddrinfo(res);

	if(vect.empty())
	{
		throw ESocket( (string("No network card detected for ")+string(localhost)).c_str() );
	}

	return vect;
}

bool CInetAddress::is127001 () const
{
	return (internalIPAddress () == htonl(0x7F000001));
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
