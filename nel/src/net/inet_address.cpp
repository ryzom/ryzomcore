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
#include "nel/misc/string_view.h"
#include "nel/misc/wang_hash.h"

#include "nel/net/inet_address.h"
#include "nel/net/sock.h"
#include "nel/net/net_log.h"

#ifdef NL_OS_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
// for Windows 2000 compatibility
#include <wspiapi.h>
#elif defined NL_OS_UNIX
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define WSAGetLastError() 0
#define SOCKET_ERROR -1
#endif

#define NLNET_IPV6_LOOKUP (0)

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

namespace NLNET {

CInetAddress::CInetAddress()
{
	init();
#if NLNET_IPV6_LOOKUP
	m_Address = CIPv6Address::anyIPv6();
#else
	m_Address = CIPv6Address::anyIPv4();
#endif
}

/*
 * Constructor
 */
CInetAddress::CInetAddress(bool any)
{
	init();
	if (any)
	{
#if NLNET_IPV6_LOOKUP
		m_Address = CIPv6Address::anyIPv6();
#else
		m_Address = CIPv6Address::anyIPv4();
#endif
	}
}

/*
 * Constructor with IPv4 address, port=0
 */
CInetAddress::CInetAddress(const in_addr *ip, const char *hostname)
{
	init();
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr = *ip;
	m_Address.fromSockAddrInet(&addr);
}

/*
 * Constructor with IPv6 address, port=0
 */
CInetAddress::CInetAddress(const in6_addr *ip, const char *hostname)
{
	init();
	sockaddr_in6 addr;
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = *ip;
	m_Address.fromSockAddrInet6(&addr);
}

#if 0
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
#endif

/*
 * Alternate constructor (calls setByName())
 */
CInetAddress::CInetAddress(const std::string &hostName, uint16 port)
{
	init();
	setPort(port);
	setByName(hostName);
}

/*
 * Alternate constructor (calls setNameAndPort())
 */
CInetAddress::CInetAddress(const std::string &hostNameAndPort)
{
	init();
	setNameAndPort(hostNameAndPort);
}

/*
 * Copy constructor
 */
CInetAddress::CInetAddress(const CInetAddress &other)
{
	init();
	m_Address = other.m_Address;
	m_Port = other.m_Port;
}

/*
 * Assignment operator
 */
CInetAddress &CInetAddress::operator=(const CInetAddress &other)
{
	m_Address = other.m_Address;
	m_Port = other.m_Port;
	return *this;
}

/*
 * Comparison == operator
 */
bool operator==(const CInetAddress &a1, const CInetAddress &a2)
{
	return (a1.m_Port == a2.m_Port) && (a1.m_Address == a2.m_Address);
}

/*
 * Comparison < operator
 */
bool operator<(const CInetAddress &a1, const CInetAddress &a2)
{
	if (a1.m_Port != a2.m_Port)
	{
		return a1.m_Port < a2.m_Port;
	}
	else
	{
		return a1.m_Address < a2.m_Address;
	}
}

/*
 * Constructor contents
 */
void CInetAddress::init()
{
	CSock::initNetwork();
	m_Port = 0;
}

/*
 * Destructor
 */
CInetAddress::~CInetAddress()
{
}

void CInetAddress::parseNameAndPort(std::string &hostname, uint16 &port, const std::string &hostnameAndPort)
{
	// Sets hostname and port (ex: www.nevrax.com:80, 192.168.0.2:80, [::1]:80)
	string::size_type pos6end = hostnameAndPort.find_last_of(']');
	string::size_type pos = hostnameAndPort.find_last_of(':');
	if (pos != string::npos && (pos6end == string::npos || pos > pos6end))
	{
		fromString(hostnameAndPort.substr(pos + 1), port);
		if (pos6end != string::npos)
		{
			string::size_type pos6begin = hostnameAndPort.find_first_of('[');
			hostname = hostnameAndPort.substr(pos6begin + 1, pos6end - pos6begin - 1);
		}
		else
		{
			hostname = hostnameAndPort.substr(0, pos);
		}
	}
	else
	{
		port = 0;
		hostname = hostnameAndPort;
	}
}

/*
 * Sets hostname and port (ex: www.nevrax.com:80)
 */
void CInetAddress::setNameAndPort(const std::string &hostnameAndPort)
{
	std::string hostname;
	uint16 port;
	parseNameAndPort(hostname, port, hostnameAndPort);
	setByName(hostname);
	m_Port = port;
}

/*
 * Resolves a name
 */
CInetAddress &CInetAddress::setByName(const std::string &hostname)
{
	if (m_Address.set(hostname))
	{
		// use IPv4 or IPv6
	}
	else
	{
		// Otherwise use the traditional DNS look-up
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
		hints.ai_socktype = SOCK_STREAM;

		struct addrinfo *res = NULL;
		sint status = getaddrinfo(hostname.c_str(), NULL, &hints, &res);

		if (status)
		{
			LNETL0_DEBUG("LNETL0: Network error: resolution of hostname '%s' failed: %s", hostname.c_str(), gai_strerror(status));
			// return *this;
			throw ESocket((string("Hostname resolution failed for ") + hostname).c_str());
		}

		struct addrinfo *p = res;

		// process all addresses
		while (p != NULL)
		{
			// check address family
			if (p->ai_family == AF_INET)
			{
				// ipv4
				m_Address.fromSockAddrInet((sockaddr_in *)p->ai_addr);
			}
#if NLNET_IPV6_LOOKUP
			else if (p->ai_family == AF_INET6)
			{
				// ipv6
				m_Address.fromSockAddrInet6((sockaddr_in6 *)p->ai_addr);
			}
#endif

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
	m_Port = port;
}

/* Sets internal socket address directly (contents is copied).
 * It also retrieves the host name if CInetAddress::RetrieveNames is true.
 */
void CInetAddress::fromSockAddrInet(const TSockAddrIn *saddr)
{
	m_Address.fromSockAddrInet(saddr);
	m_Port = ntohs(saddr->sin_port);
}

/* Sets internal socket address directly (contents is copied).
 * It also retrieves the host name if CInetAddress::RetrieveNames is true.
 */
void CInetAddress::fromSockAddrInet6(const TSockAddrIn6 *saddr6)
{
	m_Address.fromSockAddrInet6(saddr6);
	m_Port = ntohs(saddr6->sin6_port);
}

/*
 * Returns if object (address and port) is valid
 */
bool CInetAddress::isValid() const
{
	return (m_Address.isValid() && m_Port != 0);
}

bool CInetAddress::toSockAddrInet(TSockAddrIn *addr) const
{
	bool res = m_Address.toSockAddrInet(addr);
	addr->sin_port = htons(m_Port);
	return res && m_Port;
}

bool CInetAddress::toSockAddrInet6(TSockAddrIn6 *addr) const
{
	bool res = m_Address.toSockAddrInet6(addr);
	addr->sin6_port = htons(m_Port);
	return res && m_Port;
}

/*
 * Returns internal IP address
 */
uint32 CInetAddress::internalIPAddress() const
{
	TSockAddrIn addr;
	if (toSockAddrInet(&addr))
		return addr.sin_addr.s_addr;
	else
		return m_Address.hash32();
}

uint32 CInetAddress::internalNetAddress() const
{
	uint32 ip = internalIPAddress();
	if ((ip & 0x00000080) == 0)
	{
		// A class
		return ip & 0x000000FF;
	}
	else if ((ip & 0x00000040) == 0)
	{
		// B class
		return ip & 0x0000FFFF;
	}
	else if ((ip & 0x00000020) == 0)
	{
		// C class
		return ip & 0x00FFFFFF;
	}
	else if ((ip & 0x00000010) == 0)
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
	return m_Address.toString();
}

/*
 * Returns host name. (ex: "www.nevrax.org")
 */
string CInetAddress::hostName() const
{
	return ipAddress();
}

/*
 * Returns port
 */
uint16 CInetAddress::port() const
{
	return m_Port;
}

/*
 * Returns hostname and port as a string. (ex: "www.nevrax.org:80 (195.68.21.195)")
 */
std::string CInetAddress::asString() const
{
	return asIPString();
}

/*
 * Returns IP address and port as a string. (ex: "195.68.21.195:80")
 */
std::string CInetAddress::asIPString() const
{
	return ((m_Address.isValid() && !m_Address.isIPv4()) ? "[" + ipAddress() + "]" : ipAddress()) + ":" + NLMISC::toString(port());
}

/*
 * Serialize
 */
void CInetAddress::serial(NLMISC::IStream &s)
{
	s.serial(m_Address);
	s.serial(m_Port);
}

/*
 * Creates a CInetAddress object with local host address, port=0
 */
CInetAddress CInetAddress::localHost()
{
	const uint maxlength = 80;
	char localhost[maxlength];
	if (gethostname(localhost, maxlength) != 0)
		throw ESocket("Unable to get local hostname");
	CInetAddress localaddr = CInetAddress(string(localhost));

	if (localaddr.ipAddress() == "127.0.0.1")
	{
		nlwarning("LNETL0: No network card detected! using localhost (127.0.0.1)");
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
	char localhost[maxlength];
	if (gethostname(localhost, maxlength) == SOCKET_ERROR)
	{
		throw ESocket("Unable to get local hostname");
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
		throw ESocket(toString("Hostname resolution failed for %s", localhost).c_str());
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
			if (!IPv4LoopbackAdded)
			{
				// add loopback address only once
				struct in_addr psin_addrIPv4;
				psin_addrIPv4.s_addr = htonl(INADDR_LOOPBACK);
				vect.push_back(CInetAddress(&psin_addrIPv4, localhost));

				IPv4LoopbackAdded = true;
			}

			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;

			vect.push_back(CInetAddress(&ipv4->sin_addr, localhost));
		}
#if NLNET_IPV6_LOOKUP
		else if (p->ai_family == AF_INET6)
		{
			// loopback ipv6
			if (!IPv6LoopbackAdded)
			{
				// add loopback address only once
				struct in6_addr psin_addrIPv6 = IN6ADDR_LOOPBACK_INIT;
				vect.push_back(CInetAddress(&psin_addrIPv6, localhost));

				IPv6LoopbackAdded = true;
			}

			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;

			vect.push_back(CInetAddress(&ipv6->sin6_addr, localhost));
		}
#endif

		// process next address
		p = p->ai_next;
	}

	// free the linked list
	freeaddrinfo(res);

	if (vect.empty())
	{
		throw ESocket(toString("No network card detected for %s", localhost).c_str());
	}

	return vect;
}

bool CInetAddress::isLoopbackIPAddress() const
{
	return m_Address.getType() == CIPv6Address::Loopback;
}

uint32 CInetAddress::hash32() const
{
	return NLMISC::wangHash(m_Address.hash32() ^ m_Port);
}

uint64 CInetAddress::hash64() const
{
	return NLMISC::wangHash64(m_Address.hash64() ^ m_Port);
}

std::string vectorCInetAddressToString(const std::vector<CInetAddress> &addrs)
{
	string str;

	for (uint i = 0; i < addrs.size(); i++)
	{
		if (i != 0)
			str += " ";
		str += addrs[i].asString().c_str();
	}
	return str;
}

uint32 stringToInternalIPAddress(const std::string &addr)
{
	return inet_addr(addr.c_str());
}

std::string internalIPAddressToString(uint32 addr)
{
	string res;
	res = toString((addr)&0xFF);
	res += ".";
	res += toString((addr >> 8) & 0xFF);
	res += ".";
	res += toString((addr >> 16) & 0xFF);
	res += ".";
	res += toString((addr >> 24) & 0xFF);
	return res;
}

} // NLNET
