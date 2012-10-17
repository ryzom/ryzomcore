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

#ifndef NL_INET_ADDRESS_H
#define NL_INET_ADDRESS_H

#include "nel/misc/types_nl.h"

#include <string>
#include <vector>


struct sockaddr_in;
struct in_addr;


#ifdef NL_OS_WINDOWS
// automatically add the win socket library if you use nel network part
#pragma comment(lib, "ws2_32.lib")
#endif

namespace NLMISC
{
	class IStream;
}


namespace NLNET
{

struct ESocket;


/**
 * Internet address (IP + port).
 * The structure sockaddr_in is internally in network byte order
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CInetAddress
{
public:

	/// Default Constructor. The address is set to INADDR_ANY
	CInetAddress();

	/// Alternate constructor (calls setByName())
	CInetAddress( const std::string& hostName, uint16 port );

	/// Alternate constructor (calls setByName())
	/// example: CInetAddress("www.nevrax.com:80")
	CInetAddress( const std::string& hostNameAndPort );

	/// Copy constructor
	CInetAddress( const CInetAddress& other );

	/// Assignment operator
	CInetAddress& operator=( const CInetAddress& other );

	/// Comparison == operator
	friend bool operator==( const CInetAddress& a1, const CInetAddress& a2 );

	/// Comparison < operator
	friend bool operator<( const CInetAddress& a1, const CInetAddress& a2 );

	/// Destructor
	~CInetAddress();

	/// Resolves a name
	CInetAddress&		setByName( const std::string& hostname );

	/// Sets port
	void				setPort( uint16 port );

	/// Sets hostname and port (ex: www.nevrax.com:80)
	void				setNameAndPort( const std::string& hostNameAndPort );

	/** Sets internal socket address directly (contents is copied).
	 * It also retrieves the host name if CInetAddress::RetrieveNames is true.
	 */
	void				setSockAddr( const sockaddr_in* saddr );

	/// Returns if object (address and port) is valid
	bool				isValid() const;

	/// Returns internal socket address (read only)
	const sockaddr_in	 *sockAddr() const;

	/// Returns internal IP address
	uint32				internalIPAddress() const;

	/// Returns the internal network address (it s the network address for example 192.168.0.0 for a C class)
	uint32				internalNetAddress () const;

	/// Returns readable IP address. (ex: "195.68.21.195")
	std::string			ipAddress() const;

	/// Returns hostname. (ex: "www.nevrax.org")
	const std::string&	hostName() const;

	/// Returns port
	uint16				port() const;

	/// Returns hostname and port as a string. (ex: "www.nevrax.org:80 (195.68.21.195)")
	std::string			asString() const;

	/// Returns IP address and port as a string. (ex: "195.68.21.195:80")
	std::string			asIPString() const;

	/// Serialize
	void serial( NLMISC::IStream& s );

	/// Returns true if this CInetAddress is 127.0.0.1
	bool is127001 () const;

	/// Creates a CInetAddress object with local host address, port=0
	static CInetAddress	localHost();

	/** Returns the list of the local host addresses (with port=0)
	 * (especially useful if the host is multihomed)
	 */
	static std::vector<CInetAddress> localAddresses();

	/// If true, setSockAddr() always tries to retrieve the host name from the address
	static bool RetrieveNames;

protected:

	/// Constructor with ip address, port=0
	CInetAddress( const in_addr *ip, const char *hostname = 0);

	/// Update _HostName from _SockAddr
	void				updateHostName();

private:

	// Called in all constructors. Calls CBaseSocket::init().
	void				init();

	std::string			_HostName;
	sockaddr_in			*_SockAddr;
	bool				_Valid;

};

/// Take a internet dot string and convert it in an uint32 internal format for example "128.64.32.16" -> 0xF0804020
uint32 stringToInternalIPAddress (const std::string &addr);

/// Take an internal address and convert it to a internet dot string
std::string internalIPAddressToString (uint32 addr);

std::string vectorCInetAddressToString(const std::vector<CInetAddress> &addrs);

}

#endif // NL_INET_ADDRESS_H

/* End of inet_address.h */
