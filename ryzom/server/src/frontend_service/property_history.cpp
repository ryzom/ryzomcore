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
#include "nel/misc/vector.h"

#include "property_history.h"
#include "history.h"
#include "game_share/action.h"
#include "game_share/continuous_action.h"
#include "game_share/action_position.h"

#include <nel/misc/hierarchical_timer.h>

using namespace std;
using namespace NLMISC;
using namespace CLFECOMMON;

/*
 * Constructor
 */
CPropertyHistory::CPropertyHistory()
{
	_MaxDeltaSend = DefaultMaxDeltaSend;
}

void	CPropertyHistory::clear()
{
	uint32	num = (uint32)_ClientEntries.size();
	_ClientEntries.clear();
	_ClientEntries.resize(num);
}

void	CPropertyHistory::setMaximumClient(uint maxClient)
{
	_ClientEntries.resize(maxClient);
}

//

void	CPropertyHistory::addClient(TClientId clientId)
{
	nlassert(clientId < _ClientEntries.size() && !_ClientEntries[clientId].EntryUsed);

	CClientEntry	&entry = _ClientEntries[clientId];
	entry.reset();
	entry.EntryUsed = true;
}

void	CPropertyHistory::removeClient(TClientId clientId)
{
	nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);
	_ClientEntries[clientId].reset();
}

void	CPropertyHistory::resetClient(TClientId clientId)
{
	nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

	CClientEntry	&entry = _ClientEntries[clientId];
	uint	i;
	for (i=0; i<MAX_SEEN_ENTITIES_PER_CLIENT; ++i)
		entry.Entities[i].clearEntityEntry();
}

bool	CPropertyHistory::isValidClient(TClientId clientId)
{
	return (clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);
}

//

bool	CPropertyHistory::addEntityToClient(TCLEntityId entityId, TClientId clientId)
{
	nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

	// adds entity to the table in the client entry
	CEntityEntry	&entity = _ClientEntries[clientId].Entities[entityId];
	if (entity.Used)
	{
		return false;	
	}
	entity.resetEntityEntry();
	entity.Used = true;
	return true;
}

void	CPropertyHistory::removeEntityOfClient(TCLEntityId entityId, TClientId clientId)
{
	nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

	CEntityEntry	&entity = _ClientEntries[clientId].Entities[entityId];
	if ( ! entity.Used )
		nlwarning( "Removing entity S%hu of client C%hu in history, but not used", (uint16)entityId, clientId );
	entity.resetEntityEntry();
}

//
/*
bool	CPropertyHistory::packDelta(TClientId clientId, CAction &action, bool allowPack)
{
	nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

	if (!action.isContinuous())
	{
		return false;
	}

	// cast action
	CContinuousAction	&act = static_cast<CContinuousAction &>(action);

	if (act.isDelta())
	{
		return true;
	}

	// search if entity is already used
	CEntityEntry		&entity = _ClientEntries[clientId].Entities[action.CLEntityId];
	nlassert(entity.Used);

	// This asumes the continuous properties are the same for everyone
	uint16				propIndex = act.PropertyCode;
	CPropertyEntry		&entry = entity.Properties[propIndex];

	if (entry.AllowDelta > 0 && allowPack)
	{
		if (--entry.AllowDelta > 0)
		{
			act.packDelta(entry.Garanted);
			return true;
		}
		else
		{
			// disable next incoming garanted packets
			// until we send a garanted packet
			entry.Packet = 0xFFFFFFFF;
			return false;
		}
	}
	else
	{
		return false;
	}
}

//

bool	CPropertyHistory::packDelta(TClientId clientId, CActionPosition &action, bool allowPack)
{
	nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

	if (action.isDelta())
	{
		return true;
	}

	// search if entity is already used
	CEntityEntry		&entity = _ClientEntries[clientId].Entities[action.CLEntityId];
	nlassert(entity.Used);

	CPropertyEntry		&ex = entity.Properties[PROPERTY_POSX];

	if (ex.AllowDelta > 0 && allowPack)
	{
		if (--ex.AllowDelta > 0)
		{
			action.packDelta(entity.Properties[PROPERTY_POSX].Garanted,
							 entity.Properties[PROPERTY_POSY].Garanted,
							 entity.Properties[PROPERTY_POSZ].Garanted);
			return true;
		}
		else
		{
			// disable next incoming garanted packets
			// until we send a garanted packet
			ex.Packet = 0xFFFFFFFF;
			return false;
		}
	}
	else
	{
		return false;
	}
}
*/
//

//


//

void	CPropertyHistory::ackProperty(TClientId clientId, TCLEntityId entityId, uint32 packet, TPropIndex propId)
{
	//nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);
/*
	if (!isContinuousProperty(propId))
		return;

	CEntityEntry		&entity = _ClientEntries[clientId].Entities[entityId];

	// search if entity exists already
	if (!entity.Used)
		return;

	CPropertyEntry		&entry = entity.Properties[propId];

	// assume all previous packet have been ack'ed (ack- or ack+)
	if (packet == entry.Packet)
	{
		if (entry.AllowDelta == 0)
			entry.AllowDelta = _MaxDeltaSend;

		if (propId == PROPERTY_POSITION)
		{
			entity.garantyPosition();
		}
		else
		{
			entry.Garanted = entry.ToGaranty;
		}

		entry.Packet = 0xFFFFFFFF;
//		nldebug("FECONTH: Ack Garanted CL=%d CEId=%d Pk=%d (%.2f,%.2f)", clientId, entityId, packet, entry.Garanted.Float);
	}
*/
}

void	CPropertyHistory::negAckProperty(TClientId clientId, TCLEntityId entityId, uint32 packet, TPropIndex propId)
{
/*
	nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

	CEntityEntry		&entity = _ClientEntries[clientId].Entities[entityId];

	// search if entity exists already
	if (!entity.Used)
		return;

	CPropertyEntry		&entry = entity.Properties[propId];

	if (packet == entry.Packet)
	{
		entry.Packet = 0xFFFFFFFF;
//		nldebug("FECONTH: negAcked CL=%d CEId=%d Pk=%d", entityId, propId, packet);
	}
*/
}


void	CPropertyHistory::ackProperties(TClientId clientId, TCLEntityId entityId, uint32 packet, const vector<TPropIndex> &ids)
{
/*
	nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

	CEntityEntry	&entity = _ClientEntries[clientId].Entities[entityId];

	// search if entity exists already
	nlassert(entity.Used);

	uint	i;
	for (i=0; i<ids.size(); ++i)
	{
		TPropIndex	id = ids[i];
		if (!isContinuousProperty(id))
			continue;

		if (packet == entity.Properties[id].Packet)
		{
			entity.Properties[id].Garanted = entity.Properties[id].ToGaranty;
			entity.Properties[id].Packet = 0xFFFFFFFF;
		}
	}
*/
}

//

/*
void	CPropertyHistory::setPropertyConversion(uint32 property, sint8 conversion)
{
	_PropertiesTranslation[property] = (conversion >= 0 ? conversion : -1);
}

void	CPropertyHistory::setPropertyConversion(CPropertyTranslation *properties, sint numProperties)
{
	sint	i;
	for (i=0; i<numProperties; ++i)
		_PropertiesTranslation[properties[i].Property] = (properties[i].Translation >= 0 ? properties[i].Translation : -1);
}
*/

