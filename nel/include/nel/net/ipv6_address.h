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

#pragma once
#ifndef NL_IPV6_ADDRESS_H
#define NL_IPV6_ADDRESS_H

#include "nel/misc/types_nl.h"

struct sockaddr_in;
struct sockaddr_in6;
struct sockaddr_storage;

namespace NLMISC {
class IStream;
}

namespace NLNET {

/**
 * An IPv6 address, which could also be an IPv4 address.
 * \author Jan BOON (Kaetemi)
 * \date 2023
 */
class NL_ALIGNAS(uint64_t) CIPv6Address
{
public:
	typedef struct sockaddr_in TSockAddrIn;
	typedef struct sockaddr_in6 TSockAddrIn6;
	typedef struct sockaddr_storage TSockAddrStorage;

	enum TType
	{
		Invalid, // Null
		Any, // Listen address for any interface
		Loopback, // Localhost
		UniqueLocal, // Local area network (fc00::/7)
		SiteLocal, // Local area network (10.0.0.0/8, 172.16.0.0/12, and 192.168.0.0/16)
		LinkLocal, // Unregistered local network (169.254.0.0/16 or fe80::/10)
		Internet, // Publicly accessible address
	};

	// Constructs a null address
	CIPv6Address() : m_Valid(false) { }

	// Constructs an any listener address
	static CIPv6Address any();

	// Constructs an any listener address
	static CIPv6Address anyIPv6();

	// Constructs an any listener address
	static CIPv6Address anyIPv4();

	// Constructs a valid loopback address
	static CIPv6Address loopback();

	// Constructs a valid loopback address
	static CIPv6Address loopbackIPv6();

	// Constructs a valid loopback address
	static CIPv6Address loopbackIPv4();

	// Constructs an address from a string.
	CIPv6Address(const std::string &str)
	    : m_Valid(set(str))
	{
	}

	// Constructs an address from a string.
	CIPv6Address(const char *str)
	    : m_Valid(set(str))
	{
	}

	// Constructs an address from a binary address
	// IPv4 is 4 bytes, IPv6 is 16 bytes
	CIPv6Address(const uint8 *addr, size_t len)
		: m_Valid(set(addr, len))
	{
	}

	// Sets the address to the given string.
	// Returns true if the address was valid, false otherwise.
	// If the address was invalid, the address is set to null.
	bool set(const std::string &str);

	// Constructs an address from a binary address
	// IPv4 is 4 bytes, IPv6 is 16 bytes
	bool set(const uint8 *addr, size_t len);
	inline void setNull() { m_Valid = false; }

	// String format of the address in IPv4 format
	std::string toIPv4String() const;

	// String format of the address in IPv6 format
	std::string toIPv6String() const;

	// String format of the address
	inline std::string toString() const
	{
		if (isIPv4()) return toIPv4String();
		else return toIPv6String();
	}

	// Serialize
	void serial(NLMISC::IStream &s);

	// Convert an IPv4 or the Any listener address to the sockaddr_in structure
	bool toSockAddrInet(TSockAddrIn *addr) const;
	
	// Convert an IPv6 address to the sockaddr_in6 structure
	bool toSockAddrInet6(TSockAddrIn6 *addr) const;

	// Convert from the sockaddr_in structure
	void fromSockAddrInet(const TSockAddrIn *addr);

	// Convert from the sockaddr_in6 structure
	void fromSockAddrInet6(const TSockAddrIn6 *addr);

	inline bool isIPv4() const
	{
		if (!m_Valid)
			return false;
		NL_ALIGNAS(uint64_t) static const uint8 reference[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF };
#ifndef NL_CPP14
		return memcmp(m_Address, reference, 12) == 0;
#else
		return ((const uint64_t *)reference)[0] == ((const uint64_t *)m_Address)[0]
		    && ((const uint32_t *)reference)[2] == ((const uint32_t *)m_Address)[2];
#endif
	}
	
	TType getType() const;

	// Get the default (widest) subnet for this address, returns null address if not applicable for this address
	CIPv6Address subnet() const;

	inline bool isValid() const { return m_Valid; }
	inline bool isNull() const { return !m_Valid; }

	bool isAny() const;

	inline bool operator==(const CIPv6Address &other) const;
	inline bool operator!=(const CIPv6Address &other) const { return !(*this == other); }
	inline bool operator<(const CIPv6Address &other) const;
	inline bool operator>(const CIPv6Address &other) const { return other < *this; }
	inline bool operator<=(const CIPv6Address &other) const;
	inline bool operator>=(const CIPv6Address &other) const { return other <= *this; }
	inline operator bool() const { return m_Valid; }
	inline bool operator!() const { return !m_Valid; }

	uint32 hash32() const;
	uint64 hash64() const;
	inline size_t hash() const
	{
		if (sizeof(size_t) == sizeof(uint64)) return hash64();
		else return hash32();
	}

private:
	// This is the IPv6 address in network byte order (big endian).
	// An IPv4 address is stored in the last 4 bytes, with 0xffff in the 2 bytes before.
	// For example, IPv4 address 192.168.0.1 is stored as
	// 0000:0000:0000:0000:0000:ffff:c0a8:0001
	NL_ALIGNAS(uint64_t) uint8 m_Address[16];
	
	// If false, this structure is considered empty or null.
	bool m_Valid;
};

inline bool CIPv6Address::operator==(const CIPv6Address &other) const
{
	if (m_Valid && other.m_Valid)
	{
#ifndef NL_CPP14
		return memcmp(m_Address, other.m_Address, 16) == 0;
#else
		return ((const uint64_t *)m_Address)[0] == ((const uint64_t *)other.m_Address)[0]
			&& ((const uint64_t *)m_Address)[1] == ((const uint64_t *)other.m_Address)[1];
#endif
	}
	else
	{
		return m_Valid == other.m_Valid;
	}
}

inline bool CIPv6Address::operator<(const CIPv6Address &other) const
{
	if (m_Valid && other.m_Valid)
	{
#ifndef NL_CPP14
		return memcmp(m_Address, other.m_Address, 16) < 0;
#else
		if (((const uint64_t *)m_Address)[0] != ((const uint64_t *)other.m_Address)[0])
			return ((const uint64_t *)m_Address)[0] < ((const uint64_t *)other.m_Address)[0];
		else
			return ((const uint64_t *)m_Address)[1] < ((const uint64_t *)other.m_Address)[1];
#endif
	}
	else
	{
		return m_Valid;
	}
}

inline bool CIPv6Address::operator<=(const CIPv6Address &other) const
{
	if (m_Valid && other.m_Valid)
	{
#ifndef NL_CPP14
		return memcmp(m_Address, other.m_Address, 16) <= 0;
#else
		if (((const uint64_t *)m_Address)[0] != ((const uint64_t *)other.m_Address)[0])
			return ((const uint64_t *)m_Address)[0] < ((const uint64_t *)other.m_Address)[0];
		else
			return ((const uint64_t *)m_Address)[1] <= ((const uint64_t *)other.m_Address)[1];
#endif
	}
	else
	{
		return m_Valid || (m_Valid == other.m_Valid);
	}
}

}

#endif /* #ifndef NL_IPV6_ADDRESS_H */

/* end of file */
