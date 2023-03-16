// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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
#include "nel/net/inet_host.h"

#include "nel/misc/string_common.h"
#include "nel/misc/string_view.h"
#include "nel/misc/stream.h"

#include "nel/net/sock.h"
#include "nel/net/net_log.h"

#ifdef NL_OS_WINDOWS
// Windows includes for `sockaddr_in6` and `WSAStringToAddressW`
#include <ws2ipdef.h>
// Windows includes for `inet_pton` and `getaddrinfo`
#include <ws2tcpip.h>
// For Windows 2000 compatibility
#include <wspiapi.h>
#else
// Linux includes for `sockaddr_in`, `sockaddr_in6`, `inet_pton`, and `inet_ntop`
#include <arpa/inet.h>
#include <netinet/in.h>
// #include <sys/socket.h>
// Linux includes for `getaddrinfo`
#include <netdb.h>
// Linux includes for `getifaddrs`
#include <ifaddrs.h>
#endif

#define NLNET_IPV6_LOOKUP (1)

namespace NLNET {

namespace /* anonymous */ {

typedef struct sockaddr_in TSockAddrIn;
typedef struct sockaddr_in6 TSockAddrIn6;
typedef struct sockaddr_storage TSockAddrStorage;

// Find a hostname from an existing address
std::string findHostname(const CInetAddress &address)
{
	CSock::initNetwork();

	TSockAddrStorage storage;
	if (!address.getAddress().isValid()
	    || !address.getAddress().toSockAddrInet((TSockAddrIn *)(&storage))
	    || !address.getAddress().toSockAddrInet6((TSockAddrIn6 *)(&storage)))
		return std::string();
	nlassert(storage.ss_family == AF_INET || storage.ss_family == AF_INET6);

#if defined(NL_OS_WINDOWS) && defined(GetNameInfo)
	WCHAR host[NI_MAXHOST];
	INT res;
#else
	char host[NI_MAXHOST];
	int res;
#endif
	host[0] = 0;

	if (storage.ss_family == AF_INET)
	{
		((TSockAddrIn *)&storage)->sin_port = 0;
#if defined(NL_OS_WINDOWS) && defined(GetNameInfo)
		res = GetNameInfoW((SOCKADDR *)&storage, sizeof(TSockAddrIn), host, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
#else
		res = getnameinfo((struct sockaddr *)&storage, sizeof(TSockAddrIn), host, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
#endif
	}
	else if (storage.ss_family == AF_INET6)
	{
		((TSockAddrIn6 *)&storage)->sin6_port = 0;
#if defined(NL_OS_WINDOWS) && defined(GetNameInfo)
		res = GetNameInfoW((SOCKADDR *)&storage, sizeof(TSockAddrIn6), host, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
#else
		res = getnameinfo((struct sockaddr *)&storage, sizeof(TSockAddrIn6), host, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
#endif
	}

	if (res || !host[0])
	{
		return std::string();
	}
	else
	{
#ifdef NL_OS_WINDOWS
#ifdef GetNameInfo
		return NLMISC::wideToUtf8(host);
#else
		return NLMISC::mbcsToUtf8(host);
#endif
#else
		return host;
#endif
	}
}

} /* anonymous namespace */

CInetHost::CInetHost()
{
	m_Addresses.push_back(CInetAddress(false));
}

CInetHost::CInetHost(const char *hostnameAndPort)
{
	set(std::string(hostnameAndPort));
	nlassert(m_Addresses.size());
}

CInetHost::CInetHost(const std::string &hostnameAndPort)
{
	set(hostnameAndPort);
	nlassert(m_Addresses.size());
}

CInetHost::CInetHost(const std::string &hostname, uint16 port)
{
	set(hostname, port);
	nlassert(m_Addresses.size());
}

CInetHost::CInetHost(const CInetAddress &address)
{
	set(address);
	nlassert(m_Addresses.size());
}

CInetHost::CInetHost(const CInetAddress &address, bool lookup)
{
	set(address, lookup);
	nlassert(m_Addresses.size());
}

CInetHost::CInetHost(const std::vector<CInetAddress> &address, bool lookup)
{
	set(address, lookup);
	nlassert(m_Addresses.size());
}

void CInetHost::set(const std::string &hostnameAndPort)
{
	std::string hostname;
	uint16 port;
	CInetAddress::parseNameAndPort(hostname, port, hostnameAndPort);
	set(hostname, port);
}

void CInetHost::set(const std::string &hostname, uint16 port)
{
	m_Hostname = hostname;
	m_Addresses.clear();

	CInetAddress addr(CIPv6Address(hostname), port);

	if (addr.getAddress().isValid())
	{
		// The hostname is a raw IPv4 or IPv6 address
		m_Addresses.push_back(addr);
		return;
	}

	// Otherwise use the traditional DNS look-up
#if defined(NL_OS_WINDOWS) && defined(GetAddrInfo)
	ADDRINFOW *res = NULL;
	ADDRINFOW hints;
#else
	struct addrinfo *res = NULL;
	struct addrinfo hints;
#endif
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;

#ifdef NL_OS_WINDOWS
#ifdef GetAddrInfo
	INT status = GetAddrInfoW(nlUtf8ToWide(hostname), NULL, &hints, &res);
#else
	INT status = getaddrinfo(nlUtf8ToMbcs(hostname), NULL, &hints, &res);
#endif
#else
	sint status = getaddrinfo(hostname.c_str(), NULL, &hints, &res);
#endif

	if (status)
	{
		if (m_Addresses.empty())
		{
			m_Addresses.push_back(CInetAddress(CIPv6Address(), port));
		}
		LNETL0_DEBUG("LNETL0: Network error: resolution of hostname '%s' failed: %s", hostname.c_str(), gai_strerror(status));
		throw ESocket((std::string("Hostname resolution failed for ") + hostname).c_str());
	}

#if defined(NL_OS_WINDOWS) && defined(GetAddrInfo)
	ADDRINFOW *p = res;
#else
	struct addrinfo *p = res;
#endif

	// process all addresses
	while (p != NULL)
	{
		// check address family
		if (p->ai_family == AF_INET)
		{
			// IPv4
			addr.fromSockAddrInet((struct sockaddr_in *)p->ai_addr);
			addr.setPort(port);
			m_Addresses.push_back(addr);
		}
#if NLNET_IPV6_LOOKUP
		else if (p->ai_family == AF_INET6)
		{
			// IPv6
			addr.fromSockAddrInet6((struct sockaddr_in6 *)p->ai_addr);
			addr.setPort(port);
			m_Addresses.push_back(addr);
		}
#endif

		// process next address
		p = p->ai_next;
	}

	// free the linked list
#if defined(NL_OS_WINDOWS) && defined(GetAddrInfo)
	FreeAddrInfoW(res);
#else
	freeaddrinfo(res);
#endif
	
	if (m_Addresses.empty())
	{
		m_Addresses.push_back(CInetAddress(CIPv6Address(), port));
	}
}

void CInetHost::set(const CInetAddress &address, bool lookup)
{
	// Reverse lookup the address
	m_Hostname = findHostname(address);
	m_Addresses.clear();
	if (lookup && m_Hostname.empty())
	{
		CInetHost localhost = CInetHost::localAddresses(address.port(), true, true);
		if (std::find(localhost.addresses().begin(), localhost.addresses().end(), address) != localhost.addresses().end())
		{
			// It's me!
			m_Hostname = localhost.m_Hostname;
			m_Addresses.swap(localhost.m_Addresses);
			lookup = false;
		}
	}
	if (m_Hostname.empty())
	{
		// Use the IP address in case of failure
		m_Hostname = address.getAddress().toString();
		if (m_Hostname[0] == 'n') // "null" invalid IP
		{
			m_Hostname = nlstr("invalid.invalid"); // "invalid.invalid" invalid hostname
		}
		// Never lookup, since we don't have a hostname
		lookup = false;
	}
	if (lookup)
	{
		// Use lookup to add all the addresses
		std::string hostname = m_Hostname;
		uint16 port = address.port();
		set(hostname, port);
	}
	if (std::find(m_Addresses.begin(), m_Addresses.end(), address) == m_Addresses.end())
	{
		m_Addresses.insert(m_Addresses.begin(), address);
	}
}

void CInetHost::set(const std::vector<CInetAddress> &addresses, bool lookup)
{
	m_Addresses = addresses;
	m_Hostname.clear();
	if (m_Addresses.empty())
	{
		m_Addresses.push_back(CInetAddress(false));
	}
	if (lookup)
	{
		std::string hostname;
		updateAddressesHostname(hostname);
		m_Hostname = hostname;
	}
}

void CInetHost::clear()
{
	m_Hostname.clear();
	m_Addresses.clear();
	m_Addresses.push_back(CInetAddress(false));
}

std::string CInetHost::localHostName()
{
	char localhost[NI_MAXHOST];
	if (gethostname(localhost, NI_MAXHOST) == 0)
	{
		// Save hostname as UTF-8, from locale
		return NLMISC::mbcsToUtf8(localhost);
	}
	return localAddresses(0, false, false).address().getAddress().toString();
}

CInetHost CInetHost::localAddresses(uint16 port, bool sort, bool loopback)
{
	CSock::initNetwork();
	
	CInetHost host;
	std::set<CIPv6Address> addresses;
	std::vector<CInetAddress> skipLoopback;
	host.m_Addresses.clear();

	addrinfo *result = NULL;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_family = AF_UNSPEC;

	// Add loopback addresses
	std::string loopbackHostname;
	bool haveLoopback = false;
	bool haveLoopback6 = false;
	if (getaddrinfo(NULL, NULL, &hints, &result) == 0)
	{
		for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			if (ptr->ai_family == AF_INET)
			{
				CInetAddress addr(false);
				addr.fromSockAddrInet((sockaddr_in *)ptr->ai_addr);
				addr.setPort(port);
				if (loopbackHostname.empty())
				{
					loopbackHostname = findHostname(addr);
				}
				if (addresses.find(addr.getAddress()) == addresses.end())
				{
					addresses.insert(addr.getAddress());
					if (loopback)
						host.m_Addresses.push_back(addr);
					else
						skipLoopback.push_back(addr);
				}
				haveLoopback = true;
			}
#if NLNET_IPV6_LOOKUP
			else if (ptr->ai_family == AF_INET6)
			{
				CInetAddress addr(false);
				addr.fromSockAddrInet6((sockaddr_in6 *)ptr->ai_addr);
				addr.setPort(port);
				if (loopbackHostname.empty())
				{
					loopbackHostname = findHostname(addr);
				}
				if (addresses.find(addr.getAddress()) == addresses.end())
				{
					addresses.insert(addr.getAddress());
					if (loopback)
						host.m_Addresses.push_back(addr);
					else
						skipLoopback.push_back(addr);
				}
				haveLoopback6 = true;
			}
#endif
		}
		freeaddrinfo(result);
	}

	// Get local host name
	char localhost[NI_MAXHOST];
	if (gethostname(localhost, NI_MAXHOST) == 0)
	{
		// Save hostname as UTF-8, from locale
		host.m_Hostname = NLMISC::mbcsToUtf8(localhost);
		result = NULL;
		memset(&hints, 0, sizeof(hints));
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_family = AF_UNSPEC;

		// Get all addresses, and add them all
		if (getaddrinfo(localhost, NULL, &hints, &result) == 0)
		{
			for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next)
			{
				if (ptr->ai_family == AF_INET)
				{
					if (!haveLoopback)
					{
						CInetAddress loopbackAddr(CIPv6Address::loopbackIPv4(), port);
						if (addresses.find(loopbackAddr.getAddress()) == addresses.end())
						{
							addresses.insert(loopbackAddr.getAddress());
							if (loopback)
								host.m_Addresses.push_back(loopbackAddr);
							else
								skipLoopback.push_back(loopbackAddr);
						}
						haveLoopback = true;
					}
					CInetAddress addr(false);
					addr.fromSockAddrInet((sockaddr_in *)ptr->ai_addr);
					addr.setPort(port);
					if (addresses.find(addr.getAddress()) == addresses.end())
					{
						addresses.insert(addr.getAddress());
						if (loopback || !addr.isLoopbackIPAddress())
							host.m_Addresses.push_back(addr);
						else
							skipLoopback.push_back(addr);
					}
				}
#if NLNET_IPV6_LOOKUP
				else if (ptr->ai_family == AF_INET6)
				{
					if (!haveLoopback6)
					{
						CInetAddress loopbackAddr(CIPv6Address::loopbackIPv6(), port);
						if (addresses.find(loopbackAddr.getAddress()) == addresses.end())
						{
							addresses.insert(loopbackAddr.getAddress());
							if (loopback)
								host.m_Addresses.push_back(loopbackAddr);
							else
								skipLoopback.push_back(loopbackAddr);
						}
						haveLoopback6 = true;
					}
					CInetAddress addr(false);
					addr.fromSockAddrInet6((sockaddr_in6 *)ptr->ai_addr);
					addr.setPort(port);
					if (addresses.find(addr.getAddress()) == addresses.end())
					{
						addresses.insert(addr.getAddress());
						if (loopback || !addr.isLoopbackIPAddress())
							host.m_Addresses.push_back(addr);
						else
							skipLoopback.push_back(addr);
					}
				}
#endif
			}
			freeaddrinfo(result);
		}
		else
		{
			// nlwarning("Failed to get addresses for local hostname");
		}
	}
	else
	{
		// nlwarning("Failed to get local hostname");
	}

	if (host.m_Hostname.empty())
	{
		host.m_Hostname = loopbackHostname;
	}

#ifndef NL_OS_WINDOWS
	// Local hostname might be mapped to only return a loopback address, try interface addresses as well
	struct ifaddrs *ifaddr, *ifa;
	if (getifaddrs(&ifaddr) == 0)
	{
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
		{
			if (ifa->ifa_addr == NULL)
				continue;
			if (ifa->ifa_addr->sa_family == AF_INET)
			{
				CInetAddress addr(false);
				addr.fromSockAddrInet((sockaddr_in *)ifa->ifa_addr);
				addr.setPort(port);
				if (addresses.find(addr.getAddress()) == addresses.end())
				{
					addresses.insert(addr.getAddress());
					if (loopback || !addr.isLoopbackIPAddress())
						host.m_Addresses.push_back(addr);
					else
						skipLoopback.push_back(addr);
				}
			}
#if NLNET_IPV6_LOOKUP
			else if (ifa->ifa_addr->sa_family == AF_INET6)
			{
				CInetAddress addr(false);
				addr.fromSockAddrInet6((sockaddr_in6 *)ifa->ifa_addr);
				addr.setPort(port);
				if (addresses.find(addr.getAddress()) == addresses.end())
				{
					addresses.insert(addr.getAddress());
					if (loopback || !addr.isLoopbackIPAddress())
						host.m_Addresses.push_back(addr);
					else
						skipLoopback.push_back(addr);
				}
			}
#endif
		}
		freeifaddrs(ifaddr);
	}
#endif
	
	if (!host.m_Addresses.size() && skipLoopback.size())
	{
		nlwarning("The local hostname only resolves to %i loopback addresses, using them anyway", (int)skipLoopback.size());
		host.m_Addresses.swap(skipLoopback);
	}

	if (sort)
	{
		// Sort them, discard invalid
		std::vector<CInetAddress> sorting;
		sorting.reserve(host.m_Addresses.size());
		for (size_t i = 0; i < host.m_Addresses.size(); ++i)
		{
			// IPv6 first
			if (!host.m_Addresses[i].getAddress().isIPv4() && host.m_Addresses[i].getAddress().isValid())
				sorting.push_back(host.m_Addresses[i]);
		}
		for (size_t i = 0; i < host.m_Addresses.size(); ++i)
		{
			// Then IPv4
			if (host.m_Addresses[i].getAddress().isIPv4())
				sorting.push_back(host.m_Addresses[i]);
		}

		if (sorting.size() < host.m_Addresses.size())
		{
			nlwarning("Discarded %i local host addresses (phase 1)", (int)(host.m_Addresses.size() - sorting.size()));
		}

		// Second sorting pass, local addresses first
		// Other than that, keep OS order
		host.m_Addresses.clear();
		for (size_t i = 0; i < sorting.size(); ++i)
		{
			// Filter only IPv4 for loopback first, since [::1] appears to have throttling on connect on Ubuntu
			if (sorting[i].getAddress().getType() == CIPv6Address::Loopback && sorting[i].getAddress().isIPv4())
			{
				host.m_Addresses.push_back(sorting[i]);
				break; // One loopback is enough
			}
		}
		if (host.m_Addresses.empty())
		{
			// If there's no IPv4 loopback, include all
			for (size_t i = 0; i < sorting.size(); ++i)
			{
				if (sorting[i].getAddress().getType() == CIPv6Address::Loopback)
				{
					host.m_Addresses.push_back(sorting[i]);
					break; // One loopback is enough
				}
			}
		}
		for (size_t i = 0; i < sorting.size(); ++i)
		{
			if (sorting[i].getAddress().getType() == CIPv6Address::UniqueLocal)
				host.m_Addresses.push_back(sorting[i]);
		}
		for (size_t i = 0; i < sorting.size(); ++i)
		{
			if (sorting[i].getAddress().getType() == CIPv6Address::SiteLocal)
				host.m_Addresses.push_back(sorting[i]);
		}
#if 0
		for (size_t i = 0; i < sorting.size(); ++i)
		{
			// Not sure if we should keep link local addresses or just discard them...
			if (sorting[i].getAddress().getType() == CIPv6Address::LinkLocal)
				host.m_Addresses.push_back(sorting[i]);
		}
#endif
		for (size_t i = 0; i < sorting.size(); ++i)
		{
			if (sorting[i].getAddress().getType() == CIPv6Address::Internet)
				host.m_Addresses.push_back(sorting[i]);
		}

		if (host.m_Addresses.size() < sorting.size())
		{
			// nlwarning("Discarded %i local host addresses (phase 2)", (int)(sorting.size() - host.m_Addresses.size()));
		}
	}

	if (host.m_Addresses.empty())
	{
		throw ESocket(NLMISC::toString("No network card detected for %s", localhost).c_str(), false);
	}

	return host;
}

void CInetHost::setPort(uint16 port)
{
	// Set port on all addresses
	nlassert(m_Addresses.size());
	for (size_t i = 0; i < m_Addresses.size(); ++i)
	{
		m_Addresses[i].setPort(port);
	}
}

CInetHost CInetHost::at(size_t i) const
{
	CInetHost host;
	host.m_Addresses.clear();
	host.m_Addresses.push_back(m_Addresses[i]);
	host.m_Hostname = m_Hostname;
	return host;
}

std::vector<CInetHost> CInetHost::split() const
{
	std::vector<CInetHost> res;
	for (size_t i = 0; i < m_Addresses.size(); ++i)
	{
		res.push_back(at(i));
	}
	return res;
}

void CInetHost::serial(NLMISC::IStream &s)
{
	// Don't write "localhost", when reading
	// When hostname is empty, try to reverse lookup starting from the *last *address, which is most likely a public one
	std::string hostname = m_Hostname == nlstr("localhost") ? std::string() : m_Hostname;
	s.serial(hostname);
	s.serialCont(m_Addresses);
	if (s.isReading())
	{
		if (m_Addresses.empty())
		{
			m_Addresses.push_back(CInetAddress(false));
		}
		updateAddressesHostname(hostname);
	}
	m_Hostname = hostname;
}

std::string CInetHost::toStringLong() const
{
	std::string res = toString();
	res += nlstr(" (");
	for (size_t i = 0; i < m_Addresses.size(); ++i)
	{
		if (i != 0) res += nlstr(", ");
		res += m_Addresses[i].getAddress().toString();
	}
	res += nlstr(")");
	return res;
}

std::string CInetHost::toStringLong(size_t i) const
{
	std::string res = toString();
	res += nlstr(" (");
	res += m_Addresses[i].getAddress().toString();
	res += nlstr(")");
	return res;
}

std::string CInetHost::toString() const
{
	std::string hostname = m_Hostname.empty() ? nlstr("invalid.invalid") : m_Hostname;
	if (hostname.find(':') != std::string::npos)
	{
		// IPv6
		hostname = nlstr("[") + hostname + nlstr("]");
	}
	uint16 port_ = port();
	if (port_ != 0)
	{
		hostname += nlstr(":") + NLMISC::toString(port_);
	}
	return hostname;
}

bool CInetHost::operator==(const CInetHost &other) const
{
	return m_Addresses == other.m_Addresses
		|| (isAddressValid() && (m_Hostname == other.m_Hostname));
}

CInetHost &CInetHost::operator=(const CInetHost &other)
{
	m_Hostname = other.m_Hostname;
	m_Addresses = other.m_Addresses;
	nlassert(m_Addresses.size());
	return *this;
}

void CInetHost::updateAddressesHostname(std::string &hostname)
{
	// Sanitizes addresses structure
	// Attempts to find a good hostname
	if (hostname.empty())
	{
		// Loop addresses from the back
		for (size_t i = m_Addresses.size(); i > 0; --i)
		{
			hostname = findHostname(m_Addresses[0]);
			if (!hostname.empty())
				break;
		}
	}
	if (hostname.empty())
	{
		// Use the first IP address in case of failure
		hostname = m_Addresses[0].getAddress().toString();
		if (hostname[0] == 'n') // "null" invalid IP
		{
			hostname = nlstr("invalid.invalid"); // "invalid.invalid" invalid hostname
		}
	}
}

} /* namespace NLNET */

/* end of file */
