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

#include <string>
#include <queue>

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>

#include <nel/net/udp_sock.h>

//
// Using
//

using namespace std;

using namespace NLMISC;
using namespace NLNET;

//
// Variables
//

static uint32	Lag = 0;
static uint8	PacketLoss = 0;
static uint8	PacketDuplication = 0;
static uint8	PacketDisordering = 0;

//
// Class
//

struct CBufferizedPacket
{
	CBufferizedPacket (CUdpSock *client, const uint8 *packet, uint32 packetSize, uint32 delay, const CInetAddress *addr):
		Client(client), PacketSize(packetSize), Time(CTime::getLocalTime()+delay)
	{
		nlassert (packetSize > 0);
		nlassert (packet != NULL);
		nlassert (client != NULL);

		Packet = new uint8[packetSize];
		memcpy (Packet, packet, packetSize);

		if (addr != NULL)
		{
			Addr = new CInetAddress;
			*Addr = *addr;
		}
		else
		{
			Addr = NULL;
		}
	}

	~CBufferizedPacket ()
	{
		nlassert (Packet != NULL);
		delete [] Packet;
		Packet = NULL;
		Client = NULL;
		PacketSize = 0;
		Time = 0;
		if (Addr != NULL)
			delete Addr;
	}

	CUdpSock	*Client;
	uint8		*Packet;
	uint32		 PacketSize;
	TTime		 Time;
	CInetAddress	*Addr;
};

//
// Variables
//

static queue<CBufferizedPacket*> BufferizedPackets;

//
// Prototypes
//

void sendUDPNow (CUdpSock *client, const uint8 *packet, uint32 packetSize, const CInetAddress *addr);

//
// Functions
//

void updateBufferizedPackets ()
{
	TTime ct = CTime::getLocalTime ();
	while (!BufferizedPackets.empty())
	{
		CBufferizedPacket *bp = BufferizedPackets.front ();
		if (bp->Time <= ct)
		{
			// time to send the message
			sendUDPNow (bp->Client, bp->Packet, bp->PacketSize, bp->Addr);
			delete bp;
			BufferizedPackets.pop ();
		}
		else
		{
			break;
		}
	}
}

void setSimlagValues (sint32 lag, sint8 packetLoss, sint8 packetDuplication, sint8 packetDisordering)
{
	if (lag != -1) Lag = lag;
	if (packetLoss != -1) PacketLoss = packetLoss;
	if (packetDuplication != -1) PacketDuplication = packetDuplication;
	if (packetDisordering != -1) PacketDisordering = packetDisordering;
}

void sendUDPNow (CUdpSock *client, const uint8 *packet, uint32 packetSize, const CInetAddress *addr)
{
	if (addr == NULL)
		client->send (packet, packetSize);
	else
		client->sendTo (packet, packetSize, *addr);

	uint32 packetNumber = *(uint32 *)packet;
//	nlinfo ("time %" NL_I64 "u sending now packet %5u", CTime::getLocalTime (), packetNumber);
}

void sendUDP (CUdpSock *client, const uint8 *packet, uint32 packetSize, const CInetAddress *addr)
{
	nlassert (client != NULL);
	nlassert (packet != NULL);
	nlassert (packetSize > 0);

	if ((float)rand()/(float)(RAND_MAX)*100.0f >= PacketLoss)
	{
		sint32 lag = Lag /*+ (rand()%40) - 20*/;// void disordering
		
		if (lag > 100)
		{
			// send the packet later

			CBufferizedPacket *bp = new CBufferizedPacket (client, packet, packetSize, lag, addr);

			// duplicate the packet
			if ((float)rand()/(float)(RAND_MAX)*100.0f < PacketDisordering && BufferizedPackets.size() > 0)
			{
				CBufferizedPacket *bp2 = BufferizedPackets.back();

				// exange the time
				TTime t = bp->Time;
				bp->Time = bp2->Time;
				bp2->Time = t;

				// exange packet in the buffer
				BufferizedPackets.back() = bp;
				bp = bp2;
			}

			BufferizedPackets.push (bp);

			// duplicate the packet
			if ((float)rand()/(float)(RAND_MAX)*100.0f < PacketDuplication)
			{
				CBufferizedPacket *bp = new CBufferizedPacket (client, packet, packetSize, lag, addr);
				BufferizedPackets.push (bp);
			}
		}
		else
		{
			// send the packet NOW

			sendUDPNow (client, packet, packetSize, addr);

			// duplicate the packet
			if ((float)rand()/(float)(RAND_MAX)*100.0f < PacketDuplication)
			{
				sendUDPNow (client, packet, packetSize, addr);
			}
		}
	}
}
