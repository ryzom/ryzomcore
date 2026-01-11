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

#include "nel/net/dummy_tcp_sock.h"
#include "nel/net/net_log.h"

using namespace NLMISC;


namespace NLNET {


/*
 * Set only the remote address
 */
void CDummyTcpSock::connect( const CInetAddress& addr )
{
	_RemoteAddr = addr;
	_Sock = 100;

	_BytesReceived = 0;
	_BytesSent = 0;

	//CSynchronized<bool>::CAccessor sync( &_SyncConnected );
	//sync.value() = true;
	_Connected = true;

	LNETL0_DEBUG( "LNETL0: Socket connected to %s", addr.asString().c_str() );
}


/*
 *Dummy disconnection
 */
void CDummyTcpSock::disconnect()
{
	LNETL0_DEBUG( "LNETL0: Socket disconnecting from %s...", _RemoteAddr.asString().c_str() );

	//CSynchronized<bool>::CAccessor sync( &_SyncConnected );
	//sync.value() = false;
	_Connected = false;
}



} // NLNET
