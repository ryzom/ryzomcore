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

#ifndef NL_DUMMY_TCP_SOCK_H
#define NL_DUMMY_TCP_SOCK_H

#include "nel/misc/types_nl.h"
#include "tcp_sock.h"


namespace NLNET {


/**
 * Dummy CTcpSock replacement for replay mode
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CDummyTcpSock : public CTcpSock
{
public:

	// Constructor
	CDummyTcpSock( bool logging = true ) : CTcpSock(logging) {}

	// Dummy connection
	virtual void			connect( const CInetAddress& addr );

	// Dummy disconnection
	virtual void			disconnect();

	// Nothing
	virtual void			setNoDelay( bool /* value */ ) {}

	// Nothing
	virtual void			close() {}

};


} // NLNET


#endif // NL_DUMMY_TCP_SOCK_H

/* End of dummy_tcp_sock.h */
