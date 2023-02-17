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
 * \author Jan BOON (Kaetemi)
 * \date 2023
 */
class CInetHost
{
public:
	CInetHost();
	CInetHost(const std::string &hostnameAndPort);
	CInetHost(const std::string &hostname, uint16 port);

	/// Create a CInetHost from a known address
	/// This will do a reverse lookup to find the hostname
	CInetHost(const CInetAddress &address);

	/// Create a CInetHost from a known address
	/// This will do a reverse lookup to find the hostname
	/// If lookup is set, it will also do a lookup to find all other addresses
	CInetHost(const CInetAddress &address, bool lookup);

public:
	/// Sets hostname and port (ex: www.nevrax.com:80)
	void set(const std::string &hostnameAndPort);

	/// Sets hostname and port (ex: www.nevrax.com:80)
	void set(const std::string &hostname, uint16 port);
	
	/// This will do a reverse lookup to find the hostname
	/// If lookup is set, it will also do a lookup to find all other addresses
	void set(const CInetAddress &address, bool lookup = false);

	/// Returns if the host is valid
	inline bool isValid() const { return m_Addresses.size(); }

	/// Returns all local addresses
	static CInetHost CInetHost::localAddresses(uint16 port = 0, bool sort = true, bool loopback = true);

	/// Returns the list of addresses
	inline const std::vector<CInetAddress> &addresses() const { return m_Addresses; }

private:
	std::string m_Hostname;
	std::vector<CInetAddress> m_Addresses;
};

} /* namespace NLNET */

#endif /* #ifndef NL_INET_HOST_H */

/* end of file */
