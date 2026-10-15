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
#include "nel/net/ipv6_address.h"

#include "nel/misc/string_common.h"
#include "nel/misc/string_view.h"
#include "nel/misc/wang_hash.h"

#include "nel/net/sock.h"

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
#endif

namespace NLNET {

namespace /* anonymous */ {

bool addressFromString(uint8 address[], const char *str, size_t len)
{
	// Empty value may be used to specify null address
	// Text "null", or just any text starting with 'n', may also be used to specify null address
	if (str[0] == '\0' || str[0] == 'n')
		return false;

	// Strip excess square brackets
	if (str[0] == '[' && str[len - 1] == ']')
	{
		++str;
		len -= 2;
	}

	CSock::initNetwork();

#ifdef NL_OS_WINDOWS

	// Parse using WSAStringToAddressW
	CIPv6Address::TSockAddrStorage sockaddr;
	sockaddr.ss_family = AF_INET6;
	INT addressLen = sizeof(sockaddr);
	std::wstring wstr = NLMISC::utf8ToWide(str, len);
	if (WSAStringToAddressW((LPWSTR)wstr.c_str(), AF_INET6, NULL, (LPSOCKADDR)&sockaddr, &addressLen) != 0)
	{
		// Failed to parse as IPv6, try IPv4 format
		sockaddr.ss_family = AF_INET;
		if (WSAStringToAddressW((LPWSTR)wstr.c_str(), AF_INET, NULL, (LPSOCKADDR)&sockaddr, &addressLen) != 0)
		{
			// Fallback to `inet_addr`
			std::string lstr = NLMISC::utf8ToMbcs(str, len);
			uint32 addr = inet_addr(lstr.c_str());

			// Failed to parse
			if (addr == INADDR_NONE)
				return false;

			// Convert to IPv4-mapped IPv6 address
			memset(address, 0, 10);
			address[10] = 0xFF;
			address[11] = 0xFF;
			memcpy(address + 12, &addr, 4);
			return true;
		}
		else
		{
			// IPv4
			memset(address, 0, 10);
			address[10] = 0xFF;
			address[11] = 0xFF;
			memcpy(&address[12], &((sockaddr_in *)&sockaddr)->sin_addr.s_addr, 4);
			return true;
		}
	}

	// IPv6
	memcpy(address, &((sockaddr_in6 *)&sockaddr)->sin6_addr.s6_addr, 16);

#else

	// Parse using inet_pton
	std::string lstr = NLMISC::utf8ToMbcs(str, len); // Limit by length, not null character
	if (inet_pton(AF_INET6, lstr.c_str(), address) != 1)
	{
		// Failed to parse as IPv6
		// Try IPv4 format
		if (inet_pton(AF_INET, lstr.c_str(), &address[12]) != 1)
		{
			// Fallback to `inet_addr`
			uint32 addr = inet_addr(lstr.c_str());
			
			// Failed to parse
			if (addr == INADDR_NONE)
				return false;
			
			// Convert to IPv4-mapped IPv6 address
			memset(address, 0, 10);
			address[10] = 0xFF;
			address[11] = 0xFF;
			memcpy(address + 12, &addr, 4);
			return true;
		}
		else
		{
			// IPv4
			memset(address, 0, 10);
			address[10] = 0xFF;
			address[11] = 0xFF;
			return true;
		}
	}

#endif

	return true;
}

} /* anonymous namespace */

// Constructs a valid loopback address
CIPv6Address CIPv6Address::loopback()
{
	// Check if the IPv6 loopback address is available
	// If not, use a IPv4 loopback address, or the standard IPv4 loopback address

	addrinfo *result = NULL;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	CSock::initNetwork();

	hints.ai_family = AF_INET6;
	if (getaddrinfo(NULL, NULL, &hints, &result) == 0)
	{
		// IPv6
		CIPv6Address addr;
		memcpy(addr.m_Address, &((sockaddr_in6 *)result->ai_addr)->sin6_addr.s6_addr, 16);
		addr.m_Valid = true;
		freeaddrinfo(result);
		return addr;
	}

	hints.ai_family = AF_INET;
	if (getaddrinfo(NULL, NULL, &hints, &result) == 0)
	{
		// IPv4
		CIPv6Address addr;
		memset(addr.m_Address, 0, 10);
		addr.m_Address[10] = 0xFF;
		addr.m_Address[11] = 0xFF;
		memcpy(&addr.m_Address[12], &((sockaddr_in *)result->ai_addr)->sin_addr.s_addr, 4);
		addr.m_Valid = true;
		freeaddrinfo(result);
		return addr;
	}

	// No IPv6 or IPv4 loopback address found, return the standard IPv4 loopback address
	CIPv6Address addr;
	memset(addr.m_Address, 0, sizeof(addr.m_Address) - 6);
	addr.m_Address[10] = 0xFF;
	addr.m_Address[11] = 0xFF;
	addr.m_Address[12] = 127;
	addr.m_Address[13] = 0;
	addr.m_Address[14] = 0;
	addr.m_Address[15] = 1;
	addr.m_Valid = true;
	return addr;
}

// Constructs a valid loopback address
CIPv6Address CIPv6Address::loopbackIPv6()
{
	CIPv6Address addr;
	memset(addr.m_Address, 0, sizeof(addr.m_Address));
	addr.m_Address[15] = 1;
	addr.m_Valid = true;
	return addr;
}

// Constructs a valid loopback address
CIPv6Address CIPv6Address::loopbackIPv4()
{
	CIPv6Address addr;
	memset(addr.m_Address, 0, sizeof(addr.m_Address) - 6);
	addr.m_Address[10] = 0xFF;
	addr.m_Address[11] = 0xFF;
	addr.m_Address[12] = 127;
	addr.m_Address[13] = 0;
	addr.m_Address[14] = 0;
	addr.m_Address[15] = 1;
	addr.m_Valid = true;
	return addr;
}

// Constructs an address suitable for listening on any interface
CIPv6Address CIPv6Address::any()
{
	addrinfo *result = NULL;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	CSock::initNetwork();

	hints.ai_family = AF_INET6;
	if (getaddrinfo(NULL, NULL, &hints, &result) == 0)
	{
		// IPv6
		CIPv6Address addr;
		memset(addr.m_Address, 0, 16);
		addr.m_Valid = true;
		freeaddrinfo(result);
		return addr;
	}

	// No IPv6 address found, return the standard IPv4 any address
	CIPv6Address addr;
	memset(addr.m_Address, 0, 16);
	addr.m_Address[10] = 0xFF;
	addr.m_Address[11] = 0xFF;

	addr.m_Valid = true;
	return addr;
}

// Constructs an address suitable for listening on any interface
CIPv6Address CIPv6Address::anyIPv6()
{
	// Return the standard IPv6 any address
	CIPv6Address addr;
	memset(addr.m_Address, 0, 16);
	addr.m_Valid = true;
	return addr;
}

// Constructs an address suitable for listening on any interface
CIPv6Address CIPv6Address::anyIPv4()
{
	// Return the standard IPv4 any address
	CIPv6Address addr;
	memset(addr.m_Address, 0, 16);
	addr.m_Address[10] = 0xFF;
	addr.m_Address[11] = 0xFF;
	addr.m_Valid = true;
	return addr;
}

bool CIPv6Address::set(const std::string &str)
{
	m_Valid = addressFromString(m_Address, str.c_str(), str.size());
	return m_Valid;
}

bool CIPv6Address::set(const uint8 *addr, size_t len)
{
	if (len == 16)
	{
		memcpy(m_Address, addr, 16);
		m_Valid = true;
	}
	else if (len == 4)
	{
		memset(m_Address, 0, 10);
		m_Address[10] = 0xFF;
		m_Address[11] = 0xFF;
		memcpy(&m_Address[12], addr, 4);
		m_Valid = true;
	}
	else
	{
		m_Valid = false;
	}
	return m_Valid;
}

std::string CIPv6Address::toIPv4String() const
{
	if (!isIPv4() && !isAny()) return nlstr("null");

	CSock::initNetwork();

#ifdef NL_OS_WINDOWS

	// Use WSAAddressToString
	TSockAddrStorage sockAddr;
	toSockAddrInet((TSockAddrIn *)(&sockAddr));
	((TSockAddrIn *)(&sockAddr))->sin_port = 0;
	DWORD addressLen = sizeof(sockAddr);
	WCHAR addressStr[INET_ADDRSTRLEN];
	DWORD addressStrLen = sizeof(addressStr) / sizeof(WCHAR);
	if (WSAAddressToStringW((LPSOCKADDR)(&sockAddr), addressLen, NULL, addressStr, &addressStrLen) != 0)
	{
		// Failed to convert
		return nlstr("null");
	}

	return NLMISC::wideToUtf8(addressStr, addressStrLen - 1);

#else

	// Use inet_ntop
	char addressStr[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &m_Address[12], addressStr, sizeof(addressStr)) == NULL)
	{
		// Failed to convert
		return nlstr("null");
	}

	return addressStr;

#endif
}

std::string CIPv6Address::toIPv6String() const
{
	if (!isValid()) return nlstr("null");

	CSock::initNetwork();

#ifdef NL_OS_WINDOWS

	// Use WSAAddressToString
	TSockAddrStorage sockAddr;
	toSockAddrInet6((TSockAddrIn6 *)(&sockAddr));
	((TSockAddrIn6 *)(&sockAddr))->sin6_port = 0;
	DWORD addressLen = sizeof(sockAddr);
	WCHAR addressStr[INET6_ADDRSTRLEN];
	DWORD addressStrLen = sizeof(addressStr) / sizeof(WCHAR);
	if (WSAAddressToStringW((LPSOCKADDR)(&sockAddr), addressLen, NULL, addressStr, &addressStrLen) != 0)
	{
		// Failed to convert
		return nlstr("null");
	}

	return NLMISC::wideToUtf8(addressStr, addressStrLen - 1);

#else

	// Use inet_ntop
	char addressStr[INET6_ADDRSTRLEN];
	if (inet_ntop(AF_INET6, m_Address, addressStr, sizeof(addressStr)) == NULL)
	{
		// Failed to convert
		return nlstr("null");
	}

	return addressStr;

#endif
}

void CIPv6Address::serial(NLMISC::IStream &s)
{
	NLMISC::CMemStream *const ms = dynamic_cast<NLMISC::CMemStream *>(&s);
	if (ms && ms->stringMode())
	{
		std::string address;
		if (ms->isReading())
		{
			ms->serial(address);
			set(address);
		}
		else
		{
			address = toString();
			ms->serial(address);
		}
	}
	else
	{
		uint8 family;
		if (!s.isReading())
		{
			family = isNull() ? ~0 : (isIPv4() ? AF_INET : AF_INET6);
		}
		s.serial(family);
		if (family == AF_INET)
		{
			s.serialBuffer((uint8 *)&m_Address[12], 4);
			if (s.isReading())
			{
				memset(m_Address, 0, 10);
				m_Address[10] = 0xFF;
				m_Address[11] = 0xFF;
				m_Valid = true;
			}
		}
		else if (family == AF_INET6)
		{
			s.serialBuffer(m_Address, 16);
			if (s.isReading())
			{
				m_Valid = true;
			}
		}
		else
		{
			if (s.isReading())
			{
				m_Valid = false;
			}
		}
	}
}

bool CIPv6Address::toSockAddrInet(TSockAddrIn *addr) const
{
	if (!isIPv4() && !isAny())
	{
		// IPv6 addresses cannot be converted to IPv4, except for the IPv6 Any address
		addr->sin_family = AF_UNSPEC;
		return false;
	}
	// IPv4, IPv4 Any, and IPv6 Any address can be represented
	addr->sin_family = AF_INET;
	memcpy(&addr->sin_addr.s_addr, &m_Address[12], 4);
	return true;
}

bool CIPv6Address::toSockAddrInet6(TSockAddrIn6 *addr) const
{
	if (!isValid())
	{
		// Invalid addresses cannot be represented as IPv6
		addr->sin6_family = AF_UNSPEC;
		return false;
	}
	// All valid IPv4 and IPv6 addresses can be represented as IPv6
	addr->sin6_family = AF_INET6;
	addr->sin6_flowinfo = 0;
	memcpy(&addr->sin6_addr.s6_addr, m_Address, 16);
	addr->sin6_scope_id = 0;
	return true;
}

void CIPv6Address::fromSockAddrInet(const TSockAddrIn *addr)
{
	memset(m_Address, 0, 10);
	m_Address[10] = 0xFF;
	m_Address[11] = 0xFF;
	memcpy(&m_Address[12], &addr->sin_addr.s_addr, 4);
	m_Valid = (addr->sin_family == AF_INET);
}

void CIPv6Address::fromSockAddrInet6(const TSockAddrIn6 *addr)
{
	memcpy(m_Address, &addr->sin6_addr.s6_addr, 16);
	m_Valid = (addr->sin6_family == AF_INET6);
}

CIPv6Address::TType CIPv6Address::getType() const
{
	if (!isValid()) return Invalid;
	if (isIPv4())
	{
		if (m_Address[12] == 127) return Loopback;
		if (m_Address[12] == 10) return SiteLocal;
		if (m_Address[12] == 172 && (m_Address[13] & 0xF0) == 16) return SiteLocal;
		if (m_Address[12] == 192 && m_Address[13] == 168) return SiteLocal;
		if (m_Address[12] == 169 && m_Address[13] == 254) return LinkLocal;
	}
	else
	{
		NL_ALIGNAS(uint64_t) static const uint8 loopback[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
#ifndef NL_CPP14
		if (memcmp(m_Address, loopback, 16) == 0) return Loopback;
#else
		if (((const uint64_t *)m_Address)[0] == ((const uint64_t *)loopback)[0]
			&& ((const uint64_t *)m_Address)[1] == ((const uint64_t *)loopback)[1])
			return Loopback;
#endif
		if ((m_Address[0] & 0xFE) == 0xFC) return UniqueLocal;
		if (m_Address[0] == 0xFE && (m_Address[1] & 0xC0) == 0x80) return LinkLocal;
	}
	if (isAny()) return Any;
	return Internet;
}

CIPv6Address CIPv6Address::subnet() const
{
	if (!isValid()) return CIPv6Address();

	if (isIPv4())
	{
		if (m_Address[12] == 10)
		{
			CIPv6Address subnet = *this;
			subnet.m_Address[13] = 0;
			subnet.m_Address[14] = 0;
			subnet.m_Address[15] = 0;
			return subnet;
		}
		if (m_Address[12] == 172 && (m_Address[13] & 0xF0) == 16)
		{
			CIPv6Address subnet = *this;
			subnet.m_Address[14] = 0;
			subnet.m_Address[15] = 0;
			return subnet;
		}
		if (m_Address[12] == 192 && m_Address[13] == 168)
		{
			CIPv6Address subnet = *this;
			subnet.m_Address[15] = 0;
			return subnet;
		}
	}
	else
	{
		if ((m_Address[0] & 0xFE) == 0xFC)
		{
			CIPv6Address subnet = *this;
			subnet.m_Address[6] = 0;
			subnet.m_Address[7] = 0;
			subnet.m_Address[8] = 0;
			subnet.m_Address[9] = 0;
			subnet.m_Address[10] = 0;
			subnet.m_Address[11] = 0;
			subnet.m_Address[12] = 0;
			subnet.m_Address[13] = 0;
			subnet.m_Address[14] = 0;
			subnet.m_Address[15] = 0;
			return subnet;
		}
	}
	return CIPv6Address();
}

bool CIPv6Address::isAny() const
{
	if (!isValid()) return false;
	if (isIPv4())
	{
		return m_Address[12] == 0 && m_Address[13] == 0 && m_Address[14] == 0 && m_Address[15] == 0;
	}
	else
	{
		NL_ALIGNAS(uint64_t) static const uint8 any[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#ifndef NL_CPP14
		return memcmp(m_Address, any, 16) == 0;
#else
		return ((const uint64_t *)m_Address)[0] == ((const uint64_t *)any)[0]
			&& ((const uint64_t *)m_Address)[1] == ((const uint64_t *)any)[1];
#endif
	}
}

uint32 CIPv6Address::hash32() const
{
	if (!m_Valid) return 0;
	uint32 hash = NLMISC::wangHash(((uint32)m_Address[0] | ((uint32)m_Address[1] << 8) | ((uint32)m_Address[2] << 16) | ((uint32)m_Address[3] << 24)));
	hash = NLMISC::wangHash(hash ^ ((uint32)m_Address[4] | ((uint32)m_Address[5] << 8) | ((uint32)m_Address[6] << 16) | ((uint32)m_Address[7] << 24)));
	hash = NLMISC::wangHash(hash ^ ((uint32)m_Address[8] | ((uint32)m_Address[9] << 8) | ((uint32)m_Address[10] << 16) | (uint32)(m_Address[11] << 24)));
	hash = NLMISC::wangHash(hash ^ ((uint32)m_Address[12] | ((uint32)m_Address[13] << 8) | ((uint32)m_Address[14] << 16) | ((uint32)m_Address[15] << 24)));
	return hash;
}

uint64 CIPv6Address::hash64() const
{
	if (!m_Valid) return 0;
	uint64 hash = NLMISC::wangHash64(((uint64)m_Address[0] | ((uint64)m_Address[1] << 8) | ((uint64)m_Address[2] << 16) | ((uint64)m_Address[3] << 24) | ((uint64)m_Address[4] << 32) | ((uint64)m_Address[5] << 40) | ((uint64)m_Address[6] << 48) | ((uint64)m_Address[7] << 56)));
	hash = NLMISC::wangHash64(hash ^ ((uint64)m_Address[8] | ((uint64)m_Address[9] << 8) | ((uint64)m_Address[10] << 16) | ((uint64)m_Address[11] << 24) | ((uint64)m_Address[12] << 32) | ((uint64)m_Address[13] << 40) | ((uint64)m_Address[14] << 48) | ((uint64)m_Address[15] << 56)));
	return hash;
}

} /* namespace NLNET */

/* end of file */
