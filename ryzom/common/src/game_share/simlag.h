// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef NL_SIMLAG_H
#define NL_SIMLAG_H

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <nel/net/inet_address.h>
#include <nel/net/udp_sock.h>

//
// Variables
//

//
// Functions
//

// Call this function evenly to flush messages that are laged
void updateBufferizedPackets ();

// Set the packet lost and lag values for the simluation
void setSimlagValues (sint32 lag, sint8 packetLoss, sint8 packetDuplication, sint8 packetDisordering);

// Send a message using UDP connection with sim lag
void sendUDP (NLNET::CUdpSock *client, const uint8 *packet, uint32 packetSize, const NLNET::CInetAddress *addr = NULL);


#endif // NL_SIMLAG_H

/* End of simlag.h */
