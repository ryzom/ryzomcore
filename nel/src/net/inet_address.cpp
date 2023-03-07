// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/net/inet_address.h"

#include "nel/misc/common.h"
#include "nel/misc/string_view.h"
#include "nel/misc/wang_hash.h"

#include "nel/net/inet_host.h"
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

#define NLNET_IPV6_LOOKUP (1)

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

/*
 * Alternate constructor (calls setByName())
 */
CInetAddress::CInetAddress(const std::string &hostname, uint16 port)
{
	init();
	setByName(hostname);
	m_Port = port;
}

CInetAddress::CInetAddress(const char *hostname, uint16 port)
{
	init();
	setByName(hostname);
	m_Port = port;
}

/*
 * Alternate constructor (calls setNameAndPort())
 */
CInetAddress::CInetAddress(const std::string &hostNameAndPort)
{
	init();
	setNameAndPort(hostNameAndPort);
}

CInetAddress::CInetAddress(const char *hostnameAndPort)
{
	init();
	setNameAndPort(hostnameAndPort);
}

CInetAddress::CInetAddress(const CIPv6Address &ipv6Address, uint16 port)
{
	init();
	m_Address = ipv6Address;
	m_Port = port;
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
	if (pos != string::npos
		&& ((pos6end == string::npos && pos == hostnameAndPort.find(':')) || pos > pos6end)
	    && (pos == 0 || hostnameAndPort[pos - 1] != ':')
		&& (hostnameAndPort.find('.', pos) == string::npos))
	{
		if (pos + 1 < hostnameAndPort.size())
		{
			fromString(hostnameAndPort.substr(pos + 1), port);
		}
		else
		{
			port = 0;
		}
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
	CInetHost host(hostname, m_Port);
	nlassert(host.addresses().size());
	m_Address = host.address().m_Address;
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

void CInetAddress::fromSockAddrStorage(const TSockAddrStorage *saddr)
{
	if (saddr->ss_family == AF_INET)
	{
		fromSockAddrInet((const TSockAddrIn *)saddr);
	}
	else if (saddr->ss_family == AF_INET6)
	{
		fromSockAddrInet6((const TSockAddrIn6 *)saddr);
	}
	else
	{
		if (saddr->ss_family != AF_UNSPEC)
		{
			nlwarning("CInetAddress::fromSockAddrStorage(): unknown address family %d", saddr->ss_family);
		}
		m_Address.setNull();
		m_Port = 0;
	}
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

bool CInetAddress::toSockAddrStorage(TSockAddrStorage *addr, int family) const
{
	if (family == AF_INET)
	{
		return toSockAddrInet((TSockAddrIn *)addr);
	}
	else if (family == AF_INET6)
	{
		return toSockAddrInet6((TSockAddrIn6 *)addr);
	}
	else
	{
		addr->ss_family = AF_UNSPEC;
		return false;
	}
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

CInetAddress CInetAddress::loopback(uint16 port)
{
	CInetAddress addr(false);
	addr.m_Address = CIPv6Address::loopback();
	addr.m_Port = port;
	return addr;
}

/*
 * Creates a CInetAddress object with local host address, port=0
 */
CInetAddress CInetAddress::localHost(uint16 port)
{
	CInetHost localAddrs = CInetHost::localAddresses(port, false, false);
	if (localAddrs.isAddressValid())
		return localAddrs.address(); // First one from unsorted list will be used
	else
		return CInetAddress::loopback(port);
}

/* Returns the list of the local host addresses (with port=0)
 * (especially useful if the host is multihomed)
 */
std::vector<CInetAddress> CInetAddress::localAddresses()
{
	CInetHost res = CInetHost::localAddresses();
	if (!res.isAddressValid()) // Hide the invalid address
		return std::vector<CInetAddress>();
	return res.addresses();
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
