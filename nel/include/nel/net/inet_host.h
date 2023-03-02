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
#ifndef NL_INET_HOST_H
#define NL_INET_HOST_H

#include "nel/misc/types_nl.h"

#include "nel/net/inet_address.h"

namespace NLNET {

/**
 * A host name and a set of addresses.
 * It contains at minimum one address, which may be null.
 * \author Jan BOON (Kaetemi)
 * \date 2023
 */
class CInetHost
{
public:
	CInetHost();
	CInetHost(const char *hostnameAndPort);
	CInetHost(const std::string &hostnameAndPort);
	CInetHost(const std::string &hostname, uint16 port);

	/// Create a CInetHost from a known address
	/// This will do a reverse lookup to find the hostname
	CInetHost(const CInetAddress &address);

	/// Create a CInetHost from a known address
	/// This will do a reverse lookup to find the hostname
	/// If lookup is set, it will also do a lookup to find all other addresses
	CInetHost(const CInetAddress &address, bool lookup);
	
	/// Set multiple existing addresses, for backwards compatibility
	CInetHost(const std::vector<CInetAddress> &address, bool lookup);

public:
	/// Sets hostname and port (ex: www.nevrax.com:80)
	void set(const std::string &hostnameAndPort);

	/// Sets hostname and port (ex: www.nevrax.com:80)
	void set(const std::string &hostname, uint16 port);
	
	/// This will do a reverse lookup to find the hostname
	/// If lookup is set, it will also do a lookup to find all other addresses
	void set(const CInetAddress &address, bool lookup = false);

	/// Set multiple existing addresses, for backwards compatibility
	void set(const std::vector<CInetAddress> &addresses, bool lookup);

	void clear();

	/// Returns if the host and port are valid, check isAddressValid if you only care about the hostname
	inline bool isValid() const { return m_Addresses[0].isValid(); }

	/// Returns if the host is valid
	inline bool isAddressValid() const { return m_Addresses[0].getAddress().isValid(); }

	/// Returns all local addresses
	static CInetHost localAddresses(uint16 port = 0, bool sort = true, bool loopback = true);

	/// Returns the list of addresses
	inline const std::vector<CInetAddress> &addresses() const { return m_Addresses; }

	/// Returns the first address
	inline const CInetAddress &address() const { return m_Addresses[0]; }

	/// Hostname
	const std::string &hostname() const { return m_Hostname; }

	/// Port
	inline uint16 port() const { return m_Addresses[0].port(); }
	void setPort(uint16 port);

	/// Get a CInetHost with a single address. This is for compatibility purposes
	CInetHost at(size_t i) const;

	/// Number of addresses available
	inline size_t size() const { return m_Addresses.size(); }

	/// Get a vector of this CInetHost separated into individual copies for each address. This is for compatibility purposes
	std::vector<CInetHost> split() const;

	/// Serialize
	void serial(NLMISC::IStream &s);

	/// To string hostname:port (ip, ip, ip)
	std::string toStringLong() const;

	/// To string hostname:port (ip)
	std::string toStringLong(size_t i) const;
	
	/// To string short hostname:port
	std::string toString() const;

	/// Local host name
	static std::string localHostName();

	/// Comparison == operator
	bool operator==(const CInetHost &other) const;
	inline bool operator!=(const CInetHost &other) const { return !(*this == other); }

	/// Assignment operator
	CInetHost &operator=(const CInetHost &other);

private:
	void updateAddressesHostname(std::string &hostname);

private:
	std::string m_Hostname;
	std::vector<CInetAddress> m_Addresses;
};

// DEPRECATED. For compatibility
inline std::string vectorCInetAddressToString(const CInetHost &host)
{
	return host.toStringLong();
}

} /* namespace NLNET */

#endif /* #ifndef NL_INET_HOST_H */

/* end of file */
