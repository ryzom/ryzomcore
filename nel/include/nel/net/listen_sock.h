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

#ifndef NL_LISTEN_SOCK_H
#define NL_LISTEN_SOCK_H


#include "tcp_sock.h"

namespace NLNET
{


/**
 * CListenSock: listening socket for servers.
 * How to accept connections in a simple server:
 * -# Create a CListenSock object
 * -# Listen on the port you want the clients to connect
 * -# In a loop, accept a connection and store the new socket
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000-2001
 */
class CListenSock : public CTcpSock
{
public:

	/// Constructor
	CListenSock();

	///@name Socket setup
	//@{

	/// Prepares to receive connections on a specified port (bind+listen)
	void			init( uint16 port );

	/// Prepares to receive connections on a specified address/port (useful when the host has several addresses)
	void			init( const CInetAddress& addr );

	/// Sets the number of the pending connections queue, or -1 for the maximum possible value.
	void			setBacklog( sint backlog );

	/// Returns the pending connections queue.
	sint			backlog() const { return _BackLog; }

	//@}

	/// Blocks until an incoming connection is requested, accepts it, and creates a new socket (you have to delete it after use)
	CTcpSock		*accept();

private:

	bool			_Bound;

	sint			_BackLog;
};

} // NLNET

#endif // NL_LISTEN_SOCK_H

/* End of listen_sock.h */
