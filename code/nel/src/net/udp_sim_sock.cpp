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

#include "nel/misc/config_file.h"

#include "nel/net/udp_sock.h"
#include "nel/net/udp_sim_sock.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {

//
// Class
//

struct CBufferizedOutPacket
{
	CBufferizedOutPacket (CUdpSock *client, const uint8 *packet, uint32 packetSize, uint32 delay, const CInetAddress *addr):
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

	~CBufferizedOutPacket ()
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

	CUdpSock		*Client;
	uint8			*Packet;
	uint32			 PacketSize;
	TTime			 Time;
	CInetAddress	*Addr;
};


//
// Variables
//

//queue<CBufferizedOutPacket*> CUdpSimSock::BufferizedOutPackets;
//queue<CBufferizedOutPacket*> CUdpSimSock::BufferizedInPackets;

uint32	CUdpSimSock::_InLag = 0;
uint8	CUdpSimSock::_InPacketLoss = 0;

uint32	CUdpSimSock::_OutLag = 0;
uint8	CUdpSimSock::_OutPacketLoss = 0;
uint8	CUdpSimSock::_OutPacketDuplication = 0;
uint8	CUdpSimSock::_OutPacketDisordering = 0;

//
// Functions
//

void CUdpSimSock::sendUDPNow (const uint8 *buffer, uint32 len, const CInetAddress *addr)
{
	if (addr == NULL)
		UdpSock.send (buffer, len);
	else
		UdpSock.sendTo (buffer, len, *addr);
}

void CUdpSimSock::sendUDP (const uint8 *buffer, uint32& len, const CInetAddress *addr)
{
	nlassert (buffer != NULL);
	nlassert (len > 0);

	if ((float)rand()/(float)(RAND_MAX)*100.0f >= _OutPacketLoss)
	{
		sint32 lag = _OutLag /*+ (rand()%40) - 20*/;// void disordering

		if (lag > 100)
		{
			// send the packet later

			CBufferizedOutPacket *bp = new CBufferizedOutPacket (&UdpSock, buffer, len, lag, addr);

			// duplicate the packet
			if ((float)rand()/(float)(RAND_MAX)*100.0f < _OutPacketDisordering && _BufferizedOutPackets.size() > 0)
			{
				CBufferizedOutPacket *bp2 = _BufferizedOutPackets.back();

				// exange the time
				TTime t = bp->Time;
				bp->Time = bp2->Time;
				bp2->Time = t;

				// exange packet in the buffer
				_BufferizedOutPackets.back() = bp;
				bp = bp2;
			}

			_BufferizedOutPackets.push (bp);

			// duplicate the packet
			if ((float)rand()/(float)(RAND_MAX)*100.0f < _OutPacketDuplication)
			{
				CBufferizedOutPacket *bp = new CBufferizedOutPacket (&UdpSock, buffer, len, lag, addr);
				_BufferizedOutPackets.push (bp);
			}
		}
		else
		{
			// send the packet NOW

			sendUDPNow (buffer, len, addr);

			// duplicate the packet
			if ((float)rand()/(float)(RAND_MAX)*100.0f < _OutPacketDuplication)
			{
				sendUDPNow (buffer, len, addr);
			}
		}
	}
}



void CUdpSimSock::updateBufferizedPackets ()
{
	TTime ct = CTime::getLocalTime ();
	while (!_BufferizedOutPackets.empty())
	{
		CBufferizedOutPacket *bp = _BufferizedOutPackets.front ();
		if (bp->Time <= ct)
		{
			// time to send the message
			sendUDPNow (bp->Packet, bp->PacketSize, bp->Addr);
			delete bp;
			_BufferizedOutPackets.pop ();
		}
		else
		{
			break;
		}
	}
}

void				cbSimVar (CConfigFile::CVar &var)
{
	     if (var.Name == "SimInLag") CUdpSimSock::_InLag = var.asInt ();
	else if (var.Name == "SimInPacketLost") CUdpSimSock::_InPacketLoss = uint8(var.asInt ());
	else if (var.Name == "SimOutLag") CUdpSimSock::_OutLag = var.asInt ();
	else if (var.Name == "SimOutPacketLost") CUdpSimSock::_OutPacketLoss = uint8(var.asInt ());
	else if (var.Name == "SimOutPacketDuplication") CUdpSimSock::_OutPacketDuplication = uint8(var.asInt ());
	else if (var.Name == "SimOutPacketDisordering") CUdpSimSock::_OutPacketDisordering = uint8(var.asInt ());
	else nlstop;
}

void				CUdpSimSock::setSimValues (NLMISC::CConfigFile &cf)
{
	cf.setCallback ("SimInLag", cbSimVar);
	cf.setCallback ("SimInPacketLost", cbSimVar);
	cf.setCallback ("SimOutLag", cbSimVar);
	cf.setCallback ("SimOutPacketLost", cbSimVar);
	cf.setCallback ("SimOutPacketDuplication", cbSimVar);
	cf.setCallback ("SimOutPacketDisordering", cbSimVar);

	CConfigFile::CVar *pv;
	pv = cf.getVarPtr("SimInLag");
	if( pv )
		cbSimVar( *pv );
	pv = cf.getVarPtr("SimInPacketLost");
	if( pv )
		cbSimVar( *pv );
	pv = cf.getVarPtr("SimOutLag");
	if( pv )
		cbSimVar( *pv );
	pv = cf.getVarPtr("SimOutPacketLost");
	if( pv )
		cbSimVar( *pv );
	pv = cf.getVarPtr("SimOutPacketDuplication");
	if( pv )
		cbSimVar( *pv );
	pv = cf.getVarPtr("SimOutPacketDisordering");
	if( pv )
		cbSimVar( *pv );
}

void				CUdpSimSock::connect( const CInetAddress& addr )
{
	UdpSock.connect (addr);
}

void				CUdpSimSock::close()
{
	UdpSock.close ();
}

uint8 buffer [10000];

bool				CUdpSimSock::dataAvailable ()
{
	updateBufferizedPackets ();

	if (_InLag > 0)
	{
		while (UdpSock.dataAvailable ())
		{
			CInetAddress addr;
			uint len = 10000;
			UdpSock.receivedFrom (buffer, len, addr);

			if ((float)rand()/(float)(RAND_MAX)*100.0f >= _InPacketLoss)
			{
				CBufferizedOutPacket *bp = new CBufferizedOutPacket (&UdpSock, buffer, len, _InLag, &addr);
				_BufferizedInPackets.push (bp);
			}
		}

		TTime ct = CTime::getLocalTime ();
		if (!_BufferizedInPackets.empty() && _BufferizedInPackets.front ()->Time <= ct)
			return true;
		else
			return false;
	}
	else
	{
		if ((float)rand()/(float)(RAND_MAX)*100.0f >= _InPacketLoss)
		{
			return UdpSock.dataAvailable ();
		}
		else
		{
			// consume data
			if (UdpSock.dataAvailable ())
			{
				CInetAddress addr;
				uint len = 10000;
				UdpSock.receivedFrom (buffer, len, addr);
			}

			// packet lost
			return false;
		}
	}
}

bool				CUdpSimSock::receive (uint8 *buffer, uint32& len, bool throw_exception)
{
	if (_InLag> 0)
	{
		if (_BufferizedInPackets.empty())
		{
			if (throw_exception)
				throw Exception ("no data available");
			return false;
		}

		CBufferizedOutPacket *bp = _BufferizedInPackets.front ();
		uint32 s = min (len, bp->PacketSize);
		memcpy (buffer, bp->Packet, s);
		len = s;

		delete bp;
		_BufferizedInPackets.pop ();
		return true;
	}
	else
	{
		return UdpSock.receive(buffer, len, throw_exception);
	}
}

CSock::TSockResult	CUdpSimSock::send (const uint8 *buffer, uint32& len, bool /* throw_exception */)
{
	sendUDP (buffer, len);
	return CSock::Ok;
}

void CUdpSimSock::sendTo (const uint8 *buffer, uint32& len, const CInetAddress& addr)
{
	sendUDP (buffer, len, &addr);
}

bool				CUdpSimSock::connected()
{
	return UdpSock.connected ();
}

} // NLNET
