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
	virtual void			connect( const CInetHost& addr ) NL_OVERRIDE;

	// Dummy disconnection
	virtual void			disconnect() NL_OVERRIDE;

	// Nothing
	virtual void			setNoDelay( bool /* value */ ) NL_OVERRIDE {}

	// Nothing
	virtual void			close() NL_OVERRIDE {}

};


} // NLNET


#endif // NL_DUMMY_TCP_SOCK_H

/* End of dummy_tcp_sock.h */
