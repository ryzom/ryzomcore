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



#ifndef NL_HISTORY_H
#define NL_HISTORY_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

#include "property_history.h"
#include "packet_history.h"
#include "fe_types.h"

namespace CLFECOMMON
{
	class CAction;
	class CActionPacket;
}

struct CPropertyTranslation
{
	uint32	Property;
	sint8	Translation;
};


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CHistory
{
#ifdef TEST_LOST_PACKET
public:
#else
private:
#endif
	CPacketHistory		_PacketHistory;
	CPropertyHistory	_PropertyHistory;

	uint32				_MaxClientId;

public:

	/// Constructor
 	CHistory();

	/// Clears the whole history.
	void	clear();

	/// Initialises the history with at maximum maxClient (also clears the history).
	void	init(uint maxClient) { setMaximumClient(maxClient); clear(); }

	/// Setup the maximum number of clients (history is not changed till init clear is called).
	void	setMaximumClient(uint maxClient);

	/// Adds the client clientId to the history
	void	addClient(TClientId clientId);

	/// Removes the clientId
	void	removeClient(TClientId clientId);

	/// Resets a client (after a lag)
	void	resetClient(TClientId clientId);

	/// Checks is a client is used
	bool	isUsed(TClientId clientId) { return _PacketHistory.isValidClient(clientId); }

	/// Adds the entity entityId to the given clientId (the entityId is specific to the client)
	bool	addEntityToClient(CLFECOMMON::TCLEntityId entityId, TClientId clientId);

	/// Removes the entity entityId of the given clientId (the entityId is specific to the client)
	void	removeEntityOfClient(CLFECOMMON::TCLEntityId entityId, TClientId clientId);

	/// Stores action in the history (except position action)
	void	store(TClientId clientId, uint32 packetNumber, CLFECOMMON::CAction *action)
	{
		_PacketHistory.store(clientId, packetNumber, action);
		_PropertyHistory.updateProperty(clientId, packetNumber, *action);
	}

	/// Stores a position action in the history
	void	storePosition(TClientId clientId, uint32 packetNumber, CLFECOMMON::CActionPosition *action, uint32 mileage, bool storeByTimestamp, NLMISC::TGameCycle timestamp)
	{
		_PacketHistory.store(clientId, packetNumber, action);
		_PropertyHistory.updatePosition(clientId, packetNumber, *action, mileage, storeByTimestamp, timestamp);
	}

	/// Stores a disassociation
	void	storeDisassociation(TClientId clientId, CLFECOMMON::TCLEntityId slot, uint32 packetNumber, uint8 newAssociationChangeBits )
	{
		_PacketHistory.storeDisassociation(clientId, slot, packetNumber );
		_PropertyHistory.updateAssociationChangeBits(clientId, slot, newAssociationChangeBits);
	}

	/// Removes a packet from history
	void	remove(TClientId clientId, uint32 packet);

	/// Acknoledges a received packet to history (with full ack bit field)
	void	ack(TClientId clientId, uint32 packet, uint32 bits, uint ackBitWidth = 8);

	/// Ack a single packet
	void	ack(TClientId clientId, uint32 packet, bool ackvalue);

	/// Get the entity entry (only for continuous properties).
	const CPropertyHistory::CEntityEntry	&getEntityEntry(TClientId client, CLFECOMMON::TCLEntityId entity) const { return _PropertyHistory.getEntityEntry(client, entity); }

	/// Get the property entry (only for continuous properties).
	const CPropertyHistory::CPropertyEntry	&getPropertyEntry(TClientId client, CLFECOMMON::TCLEntityId entity, CLFECOMMON::TPropIndex property, bool &hasValue) const { return _PropertyHistory.getPropertyEntry(client, entity, property, hasValue); }

	/// Get the position stored in history
	bool	getPosition(TClientId client, CLFECOMMON::TCLEntityId entity, CLFECOMMON::CAction::TValue &posx, CLFECOMMON::CAction::TValue &posy, CLFECOMMON::CAction::TValue &posz) const { return _PropertyHistory.getPosition(client, entity, posx, posy, posz); }

	/// Get the cumulated delta stored in history
	uint32	getMileage(TClientId client, CLFECOMMON::TCLEntityId entity) const { return _PropertyHistory.getMileage(client, entity); }

	/// Get the position known by the client of its target at the specified tick
	void	getTargetPosAtTick( TClientId clientId, NLMISC::TGameCycle tick, CLFECOMMON::TCoord& x, CLFECOMMON::TCoord& y ) const { _PropertyHistory.getTargetPosAtTick( clientId, tick, x, y ); }
};


#endif // NL_HISTORY_H

/* End of history.h */
