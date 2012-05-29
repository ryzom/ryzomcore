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



#ifndef NL_PROPERTY_HISTORY_H
#define NL_PROPERTY_HISTORY_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

#include "game_share/action.h"
#include "game_share/action_position.h"
#include "game_share/entity_types.h"

#include "fe_types.h"

const uint	MaxTranslationProperties = 1024;
const uint	DefaultMaxDeltaSend = 100;

struct CPropertyTranslation;

/**
 * Stores continuous properties (using guaranted position system.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CPropertyHistory
{
public:
	/// A property entry, containing guaranted value and last sent value
	class CPropertyEntry
	{
	public:
		CPropertyEntry() { reset(); }

		CLFECOMMON::CAction::TValue	LastSent;
//		uint32						Packet;
		bool						HasValue;

		template<typename T>
		void	getValue(T& v) const		{ v = *((T*)(&LastSent)); }

		template<typename T>
		void	setValue(T& v)				{ *((T*)(&LastSent)) = v; }

		void	reset()
		{
//			Packet = 0xffffffff;
			HasValue = false;
			LastSent = 0;
		}
	};

	/// An entity entry, containing various properties
	class CEntityEntry
	{
	public:
		CEntityEntry() : Used(false), AssociationBitsSent(0) {}

		CPropertyEntry	Properties[CLFECOMMON::MAX_PROPERTIES_PER_ENTITY];
		uint32			Mileage;
		bool			Used;
		uint8			AssociationBitsSent;

		void	clearEntityEntry()
		{
			uint	i;
			for (i=0; i<CLFECOMMON::MAX_PROPERTIES_PER_ENTITY; ++i)
				Properties[i].reset();
		}

		void	resetEntityEntry()
		{
			Used = false;
			Mileage = 0;
			// Do not reset AssociationBitsSent, it must be kept between different entity assignments of the same client
			// It is reset in CClientEntry::reset().
			clearEntityEntry();
		}

		void	getLastSentPosition(CLFECOMMON::CAction::TValue &posx, CLFECOMMON::CAction::TValue &posy, CLFECOMMON::CAction::TValue &posz) const
		{
			posx = Properties[CLFECOMMON::PROPERTY_POSX].LastSent;
			posy = Properties[CLFECOMMON::PROPERTY_POSY].LastSent;
			posz = Properties[CLFECOMMON::PROPERTY_POSZ].LastSent;
		}

		void			setLastSentPosition(CLFECOMMON::CAction::TValue posx, CLFECOMMON::CAction::TValue posy, CLFECOMMON::CAction::TValue posz)
		{
			Properties[CLFECOMMON::PROPERTY_POSX].LastSent = posx;
			Properties[CLFECOMMON::PROPERTY_POSY].LastSent = posy;
			Properties[CLFECOMMON::PROPERTY_POSZ].LastSent = posz;
		}
	};

private:


	typedef CEntityEntry TCLEntityTable [MAX_SEEN_ENTITIES_PER_CLIENT];
	struct TTimestampedPos
	{
		NLMISC::TGameCycle		Timestamp;
		CLFECOMMON::TCoord		X, Y;
	};
	typedef std::deque< TTimestampedPos > TPositionsByTimestamp;

	/**
	 * A client entry, containing history for each entity the client sees
	 */
	class CClientEntry
	{
	public:
		TCLEntityTable								Entities;
	private:
		TPositionsByTimestamp						_TargetPositionsByTimestamp;
	public:
		bool										EntryUsed;

	public:

		/// Constructor
		CClientEntry()
		{
			reset();
		}

		/// Reset the client entry
		void	reset()
		{
			uint	i;
			for (i=0; i<MAX_SEEN_ENTITIES_PER_CLIENT; ++i)
			{
				Entities[i].resetEntityEntry();
				Entities[i].AssociationBitsSent = 0;
			}
			_TargetPositionsByTimestamp.clear();
			EntryUsed = false;
		}

		/// Add a target position
		void	storeTargetPosition( const CLFECOMMON::TCoord& x, const CLFECOMMON::TCoord& y, const NLMISC::TGameCycle& timestamp )
		{
			// Remove old positions (keep at least one)
			while ( (_TargetPositionsByTimestamp.size() > 1)
				 && (_TargetPositionsByTimestamp.front().Timestamp < timestamp - 80) ) // 8 seconds (except the latest sent pos)
			{
				_TargetPositionsByTimestamp.pop_front();
			}

			// Add the new pos at the end (they are sent in chronological order, except when the target changes)
			TTimestampedPos tp;
			tp.Timestamp = timestamp;
			tp.X = x;
			tp.Y = y;
			_TargetPositionsByTimestamp.push_back( tp );
		}

		/// Retrieve a sent target position by timestamp
		void	getTargetPosition( const NLMISC::TGameCycle& timestamp, CLFECOMMON::TCoord& x, CLFECOMMON::TCoord& y ) const
		{
			// Browse the queue from the back downto the first timestamp <= than the searched one
			TPositionsByTimestamp::const_reverse_iterator irevp;
			for ( irevp=_TargetPositionsByTimestamp.rbegin(); irevp!=_TargetPositionsByTimestamp.rend(); ++irevp )
			{
				if ( (*irevp).Timestamp <= timestamp )
				{
					x = (*irevp).X;
					y = (*irevp).Y;
					return;
				}
			}

			// Not found = no position sent yet
			x = 0;
			y = 0;
		}
	};

	/// The client entries
	std::vector<CClientEntry>			_ClientEntries;

	//
	//sint8								_PropertiesTranslation[MaxTranslationProperties];

	//
	uint32								_PositionPropertyId;

	//
	uint16								_MaxDeltaSend;

public:

	/// Constructor
	CPropertyHistory();

	/// Reinitialises the history
	void		clear();
	void		init(uint maxClient) { setMaximumClient(maxClient); clear(); }
	void		setMaximumClient(uint maxClient);

	/// @name Client management
	// @{

	/// Adds a client
	void		addClient(TClientId clientId);
	/// Removes a client
	void		removeClient(TClientId clientId);
	/// Resets a client (after a lag)
	void		resetClient(TClientId clientId);
	/// Checks if a clientId is valid
	bool		isValidClient(TClientId clientId);

	// @}


	/// @name Client management
	// @{

	/// Adds an entity to a client
	bool		addEntityToClient(CLFECOMMON::TCLEntityId entityId, TClientId clientId);
	/// Removes an entity of a client
	void		removeEntityOfClient(CLFECOMMON::TCLEntityId entityId, TClientId clientId);

	// @}

	/// Set the new association changebits sent
	void		updateAssociationChangeBits(TClientId clientId, CLFECOMMON::TCLEntityId slot, uint8 newAssociationChangeBits)
	{
		CEntityEntry		&entity = _ClientEntries[clientId].Entities[slot];
		entity.AssociationBitsSent = newAssociationChangeBits;
	}

	/// Updates a property using client, packet number and action
	void		updateProperty(TClientId clientId, uint32 packet, CLFECOMMON::CAction &action)
	{
		//nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

		CEntityEntry		&entity = _ClientEntries[clientId].Entities[action.Slot];
		// search if entity exists already
		//nlassert(entity.Used);

		CPropertyEntry		&entry = entity.Properties[action.PropertyCode];

		entry.LastSent = action.getValue();
		entry.HasValue = true;

		/*if ( action.PropIndex == 3 )
			nlinfo( "Storing orientation for %hu %hu", clientId, (uint16)action.CLEntityId );*/
	}

	/// Updates a property using client, packet number and action
	void		updatePosition(TClientId clientId, uint32 packet, CLFECOMMON::CActionPosition &action, uint32 mileage, bool storeByTimestamp, NLMISC::TGameCycle timestamp)
	{
		//nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);

		CEntityEntry		&entity = _ClientEntries[clientId].Entities[action.Slot];
		// search if entity exists already
		//nlassert(entity.Used);

		if ( action.IsRelative )
		{
			entity.Properties[0].HasValue = false; // not storing the relative pos (useless for distance calculation)
		}
		else
		{
			entity.setLastSentPosition(action.Position[0], action.Position[1], action.Position[2]);
			entity.Properties[0].HasValue = true;
		}
		entity.Mileage = mileage;

		// Store target pos by timestamp if required
		if ( storeByTimestamp )
		{
			_ClientEntries[clientId].storeTargetPosition( timestamp, action.Position[0], action.Position[1] );
		}
	}

	//
	void		ackProperty(TClientId clientId, CLFECOMMON::TCLEntityId entityId, uint32 packet, CLFECOMMON::TPropIndex propId);
	void		negAckProperty(TClientId clientId, CLFECOMMON::TCLEntityId entityId, uint32 packet, CLFECOMMON::TPropIndex propId);
	void		ackProperties(TClientId clientId, CLFECOMMON::TCLEntityId entityId, uint32 packet, const std::vector<CLFECOMMON::TPropIndex> &ids);

	//
	/*bool		isContinuousProperty(CLFECOMMON::TPropIndex property) const
	{ 
		// Deprecated //return (property >= CLFECOMMON::FIRST_CONTINUOUS_PROPERTY && property <= CLFECOMMON::LAST_CONTINUOUS_PROPERTY);
	}*/

	//
	const CEntityEntry		&getEntityEntry(TClientId clientId, CLFECOMMON::TCLEntityId entityId) const
	{
		return _ClientEntries[clientId].Entities[entityId];
	}

	//
	const CPropertyEntry	&getPropertyEntry(TClientId clientId, CLFECOMMON::TCLEntityId entityId, CLFECOMMON::TPropIndex property, bool &hasValue) const
	{
		//nlassert(isContinuousProperty(property));
		//nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);
		const CEntityEntry		&entity = _ClientEntries[clientId].Entities[entityId];
		//nlassertex(entity.Used, ("client=%d, entity=%d property=%d", clientId, entityId, property));
		const CPropertyEntry	&entry = entity.Properties[property];
		hasValue = entry.HasValue;
		return entry;
	}

	//
	bool					getPosition(TClientId clientId, CLFECOMMON::TCLEntityId entityId, CLFECOMMON::CAction::TValue &posx, CLFECOMMON::CAction::TValue &posy, CLFECOMMON::CAction::TValue &posz) const
	{
		//nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);
		const CEntityEntry		&entity = _ClientEntries[clientId].Entities[entityId];
		posx = entity.Properties[CLFECOMMON::PROPERTY_POSX].LastSent;
		posy = entity.Properties[CLFECOMMON::PROPERTY_POSY].LastSent;
		posz = entity.Properties[CLFECOMMON::PROPERTY_POSZ].LastSent;
		return entity.Properties[CLFECOMMON::PROPERTY_POSX].HasValue;
	}

	/// Get the cumulated delta stored in history
	uint32					getMileage(TClientId clientId, CLFECOMMON::TCLEntityId entityId) const
	{
		//nlassert(clientId < _ClientEntries.size() && _ClientEntries[clientId].EntryUsed);
		const CEntityEntry		&entity = _ClientEntries[clientId].Entities[entityId];
		return entity.Mileage;
	}

	/// Get the position known by the client of its target at the specified tick
	void					getTargetPosAtTick( TClientId clientId, NLMISC::TGameCycle tick, CLFECOMMON::TCoord& x, CLFECOMMON::TCoord& y ) const
	{
		//nlassert(clientId < _ClientEntries.size());
		_ClientEntries[clientId].getTargetPosition( tick, x, y );
	}

	/// Set HasValue to false to force a sending
	bool					resetValue( TClientId clientId, CLFECOMMON::TCLEntityId entityId, CLFECOMMON::TPropIndex property, uint8 currentAssociationBits )
	{
		CEntityEntry		&entity = _ClientEntries[clientId].Entities[entityId];
		if ( entity.AssociationBitsSent == currentAssociationBits )
		{
			//nlassertex(entity.Used, ("client=%d, entity=%d property=%d", clientId, entityId, property));
			switch ( property )
			{
			case CLFECOMMON::PROPERTY_DISASSOCIATION:
				break;
			case CLFECOMMON::PROPERTY_POSITION:
				entity.Mileage = 0;
				break;
			default:
				{
				CPropertyEntry	&entry = entity.Properties[property];
				entry.HasValue = false;
				}
			}
			return true;
		}
		else
		{
			return false; // the association has changed since the sending
		}
	}
};

#endif // NL_PROPERTY_HISTORY_H

/* End of property_history.h */
