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



#include "stdpch.h"

#include "nel/misc/debug.h"

#include "packet_history.h"
#include "property_history.h"
#include "game_share/action.h"

#include "frontend_service.h"
#include "vision_array.h"

using namespace CLFECOMMON;

/*
 * Constructor
 */
CPacketHistory::CPacketHistory()
{
}


void	CPacketHistory::clear()
{
	uint32	num = (uint32)_ClientsHistories.size();
	_ClientsHistories.clear();
	_ClientsHistories.resize(num);
}

void	CPacketHistory::setMaximumClient(uint maxClient)
{
	_ClientsHistories.resize(maxClient);
}


//
/*
void	CPacketHistory::store(TClientId clientId, uint packetNumber, CLFECOMMON::CAction *action)
{
	//	nlassert(clientId < _ClientsHistories.size() && _ClientsHistories[clientId].EntryUsed);

	// get the packet queue of the client
	TPacketQueue	&queue = _ClientsHistories[clientId].Queue;

	nlassert(queue.empty() || packetNumber >= queue.back().Number);

	if (queue.empty() || packetNumber>queue.back().Number)
	{
		queue.push_back(CPacketEntry());
		queue.back().Number = packetNumber;
	}

	queue.back().Packet.push_back(CMessageEntry(action->Slot, action->PropertyCode));
}
*/

void	CPacketHistory::storeDisassociation(TClientId clientId, CLFECOMMON::TCLEntityId slot, uint packetNumber )
{
	// get the packet queue of the client
	TPacketQueue	&queue = _ClientsHistories[clientId].Queue;

	nlassert(queue.empty() || packetNumber >= queue.back().Number);

	if (queue.empty() || packetNumber>queue.back().Number)
	{
		queue.push_back(CPacketEntry());
		queue.back().Number = packetNumber;
	}

	queue.back().Packet.push_back(CMessageEntry(slot, PROPERTY_DISASSOCIATION));
}




void	CPacketHistory::ack(TClientId clientId, uint32 packet, uint32 bits, uint ackBitWidth)
{
	//nlassert(clientId < _ClientsHistories.size() && _ClientsHistories[clientId].EntryUsed);

	TPacketQueue	&queue = _ClientsHistories[clientId].Queue;

	if ( queue.empty() )
		return;

	uint32			firstAck = (sint32)(queue.front().Number);

	// nothing to ack ? just leave...
	if (packet < firstAck)
		return;

	uint			totalUnreceived = packet-firstAck;
	uint			totalRecoverable = std::min(totalUnreceived, ackBitWidth);
	uint			firstRecoverable = packet-totalRecoverable;
	uint			j;

	// negAck all unrecoverable acks
	while (!queue.empty() && queue.front().Number<firstRecoverable)
	{
		uint	thisPacket = queue.front().Number;
		CPacket	&apacket = queue.front().Packet;
		for (j=0; j<apacket.size(); ++j)
			negativeAck(clientId, apacket[j].EntityId, apacket[j].PropIndex, thisPacket);
		queue.pop_front();
	}

	// ack all recoverable acks depending on the bit 
	while (!queue.empty() && queue.front().Number<packet)
	{
		uint	thisPacket = queue.front().Number;
		uint	bitNum = packet-queue.front().Number-1;

		CPacket	&apacket = queue.front().Packet;
		if (bits & (1<<bitNum))
			for (j=0; j<apacket.size(); ++j)
				positiveAck(clientId, apacket[j].EntityId, apacket[j].PropIndex, thisPacket);
		else
			for (j=0; j<apacket.size(); ++j)
				negativeAck(clientId, apacket[j].EntityId, apacket[j].PropIndex, thisPacket);

		queue.pop_front();
	}

	// ack current packet
	if (queue.empty() || queue.front().Number>packet)
		return;

	nlassert(!queue.empty() && queue.front().Number == packet);

	CPacket	&apacket = queue.front().Packet;
	for (j=0; j<apacket.size(); ++j)
		positiveAck(clientId, apacket[j].EntityId, apacket[j].PropIndex, packet);
	queue.pop_front();
}

void	CPacketHistory::ack(TClientId clientId, uint32 packet, bool ackvalue)
{
	TPacketQueue	&queue = _ClientsHistories[clientId].Queue;

	while (!queue.empty() && queue.front().Number<packet)
	{
		nlwarning("FEPKHIST: removed a packet awaiting ack (client %d, packet %d, packet %d being acked)", clientId, queue.front().Number, packet);
		queue.pop_front();
	}

	if (!queue.empty() && queue.front().Number==packet)
	{
		uint	thisPacket = queue.front().Number;
		uint	j;
		CPacket	&apacket = queue.front().Packet;
		if (ackvalue)
			for (j=0; j<apacket.size(); ++j)
				positiveAck(clientId, apacket[j].EntityId, apacket[j].PropIndex, thisPacket);
		else
			for (j=0; j<apacket.size(); ++j)
				negativeAck(clientId, apacket[j].EntityId, apacket[j].PropIndex, thisPacket);

		queue.pop_front();
	}
}

void	CPacketHistory::positiveAck(TClientId clientId, TCLEntityId entityId, TPropIndex propindex, uint32 packet)
{
	/*TPropState& propstate = CFrontEndService::instance()->PrioSub.VisionArray.propState( clientId, entityId, propindex );
	if ( propstate.UpdateStatus == Updating )
		propstate.UpdateStatus = Unchanged;*/

	//if (_PropertyHistory->isContinuousProperty(propertyId))
	//	_PropertyHistory->ackProperty(clientId, entityId, packet, propertyId);
}

void	CPacketHistory::negativeAck(TClientId clientId, TCLEntityId entityId, TPropIndex propindex, uint32 packet)
{
	TPairState& pairState = CFrontEndService::instance()->PrioSub.VisionArray.getPairState( clientId, entityId );

	// Empty the history for each property (to force the sending of the current value)
	if ( _PropertyHistory->resetValue( clientId, entityId, propindex, (uint8)pairState.AssociationChangeBits ) )
	{
		//if ( pairState.AssociationChangeBits & 0x80 == 0 )
		{
			// Set high priority
			LOG_PACKET_LOST( "%u: Reverting prio for C%hu S%hu (packet %u)", CTickEventHandler::getGameCycle(), clientId, (uint16)entityId, packet );
			pairState.revertPrio();

			// Set "negative-acked" bit
			//pairState.AssociationChangeBits |= 0x80;
		}
	}
	else // association has changed
	{
		if ( pairState.associationSuppressed() )
		{
			// Set high priority for whole slot (all properties at once)
			LOG_PACKET_LOST( "%u: Reverting prio for disassociation C%hu S%hu (packet %u)", CTickEventHandler::getGameCycle(), clientId, (uint16)entityId, packet );
			
			//pairState.revertPrio();
			// Set "negative-acked" bit to resend at least the disassociation
			//pairState.AssociationChangeBits |= 0x80;
			pairState.PrevAssociationBits = pairState.AssociationChangeBits - 1; // force them different

			CFrontEndService::instance()->PrioSub.Prioritizer.pushDissociationToResend( clientId, entityId );
		}
	}
}

//

void	CPacketHistory::addClient(TClientId clientId)
{
	//nlassert(!_ClientsHistories[clientId].EntryUsed);

	_ClientsHistories[clientId].EntryUsed = true;
	_ClientsHistories[clientId].Queue.clear();
}

void	CPacketHistory::removeClient(TClientId clientId)
{
	//nlassert(_ClientsHistories[clientId].EntryUsed);

	_ClientsHistories[clientId].EntryUsed = false;
	_ClientsHistories[clientId].Queue.clear();
}

void	CPacketHistory::resetClient(TClientId clientId)
{
	//nlassert(_ClientsHistories[clientId].EntryUsed);
	_ClientsHistories[clientId].Queue.clear();
}

bool	CPacketHistory::isValidClient(TClientId clientId)
{
	return (clientId < _ClientsHistories.size() && _ClientsHistories[clientId].EntryUsed);
}
