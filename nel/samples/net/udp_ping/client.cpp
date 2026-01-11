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
#include "nel/misc/debug.h"
#include "nel/misc/mem_stream.h"

#include "nel/net/inet_address.h"
#include "nel/net/udp_sim_sock.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Main
//

int main (int argc, char **argv)
{
	if (argc != 3)
	{
		nlinfo ("%s send udp datagram to a specific port to test a connection between client and server", argv[0]);
		nlinfo ("usage: <ip_address> <port>");
		exit (EXIT_FAILURE);
	}

	CUdpSimSock	*UdpSock = NULL;
	UdpSock = new CUdpSimSock( false );
	try
	{
		UdpSock->connect (CInetAddress (argv[1], atoi(argv[2])));
	}
	catch (Exception &e)
	{
		nlwarning ("Cannot connect to remote UDP host: %s", e.what());
		exit (EXIT_FAILURE);
	}

	uint8 *packet = new uint8[1000];
	uint32 psize;
	
	while(true)
	{
		CMemStream msgout;
		uint32 foo = 10;
		msgout.serial (foo);
		uint32 size = msgout.length();
		UdpSock->send (msgout.buffer(), size);
		nldebug ("Sent UDP datagram size %d to %s", size, UdpSock->localAddr().asString().c_str());

		while (UdpSock->dataAvailable())
		{
			psize = 1000;
			try
			{
				UdpSock->receive (packet, psize);
				nldebug ("Received UDP datagram size %d bytes from %s", psize, UdpSock->localAddr().asString().c_str());
			}
			catch ( Exception& e )
			{
				nlwarning ("Received failed: %s", e.what());
			}
		}
		nlSleep (1000);
	}

	return EXIT_SUCCESS;
}
