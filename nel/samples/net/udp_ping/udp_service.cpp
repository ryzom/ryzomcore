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

//
// Includes
//

#include "nel/misc/types_nl.h"

#include "nel/net/service.h"
#include "nel/net/udp_sim_sock.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//
// Variables
//

CUdpSock			*UdpSock = NULL;

//
// Main Class
//

class CBenchService : public IService
{
public:
	
	void init()
	{
		uint16 port = ConfigFile.getVar("UdpPort").asInt();
		nlinfo ("Starting external UDP socket on port %d", port);
		UdpSock = new CUdpSock (false);
		nlassert (UdpSock);
		UdpSock->bind (port);
	}

	bool update ()
	{
		try
		{
			uint len;
			CInetAddress addr;
			uint8 buffer[1000];
			
			while (UdpSock->dataAvailable())
			{
				len = 1000;
				UdpSock->receivedFrom((uint8*)buffer, len, addr);
				nlinfo ("Received UDP datagram size %d from %s", len, addr.asString().c_str());

				CMemStream msgout;
				uint32 foo = 10;
				msgout.serial (foo);
				uint32 size = msgout.length();
				UdpSock->sendTo (msgout.buffer(), size, addr);
				nldebug ("Sent UDP datagram size %d to %s", size, addr.asString().c_str());
			}
		}
		catch (Exception &e)
		{
			nlwarning ("Exception catched: '%s'", e.what());
		}
		return true;
	}

	void release ()
	{
		if (UdpSock != NULL)
			delete UdpSock;
	}
};


NLNET_SERVICE_MAIN (CBenchService, "UDPS", "udp_service", 0, EmptyCallbackArray, "", "")
