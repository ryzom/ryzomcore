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



#ifndef NL_VISION_ARRAY_H
#define NL_VISION_ARRAY_H

#include "nel/misc/types_nl.h"
#include "game_share/entity_types.h"
#include "fe_types.h"
#include "client_host.h"


/*
 * Constants and basic types definitions
 */
typedef float TPriority;


const CLFECOMMON::TCoord UNSET_DISTANCE = 99999999;


/**
 * Vision array item union.
 * For property index DISTANCE_CE_PI, contains the DistanceCE.
 * For property index ENTITY_INDEX_PI, contains EntityIndex.
 * For other property indices, contains PropState.
 */
struct TPairState
{
	/// Absolute distance between client and entity (in mm)
	CLFECOMMON::TCoord				DistanceCE;

//private:
	/// Current priority
	TPriority						Priority;
//public:

	/// Entity index, to reference the seen entity in the entity container
	TEntityIndex					EntityIndex;

	/// Last priority before resetting
	TPriority						LastPriority;

	enum TAssociationState
	{
		UnusedAssociation = 0,			// Pair is not used
		AwaitingAssAck = 1,				// Association was sent to the client
		NormalAssociation = 2,			// Pair is used
		AwaitingDisAck = 3,				// Disassociation was sent to the client
		//SubstitutionBeforeDisAck = 4,	// Disassociation then association were sent to the client
		//SubstitutionAfterDisAck = 5,	// The client acknowledged disassociation of substitution
		//CancelledSubstitution = 6		// A disassociation occurs before the client acknowledges the disassociation of substitution
	};

//#ifdef NL_DEBUG
	uint32							PrevAssociationBits;
//#endif

	/// Association State
	uint8							AssociationState;

	/// Association changebits (2 LSBits serialized)
	uint8							AssociationChangeBits;

	/// Association used
	bool							SlotUsed;

	///
	TPairState()
	{
		resetItem();
		resetAssociation();
	}

	///
	void							resetItem()
	{
		DistanceCE = UNSET_DISTANCE;
		Priority = 0;
		EntityIndex = TEntityIndex();
		LastPriority = 0;
		SlotUsed = false;
	}

	TPriority						getPrio() const
	{
		return Priority;
	}

	/// Used for slot 0, instead of updatePrio()...
	void							setSteadyPrio( TPriority prio )
	{
		LastPriority = prio;
		Priority = prio;
	}

	/// 
	void							resetPrio()
	{
		LastPriority = Priority;
		Priority = 0;
	}

	///
	void							revertPrio()
	{
		// Not a problem if this is done several times for the same pair
		Priority = LastPriority;
	}

	///
	void							updatePrio()
	{
		if ( DistanceCE < 100.0f )
		{
			// < 0.1 m : prio += 100.0
			Priority += 100.0f;
		}
		else
		{
			// UNSET_DISTANCE (9999 m) : prio += 0.001
			// 100 m : prio +=  0.1
			//  10 m : prio +=  1.0
			//   1 m : prio += 10.0
			Priority += 10000.0f / (float)DistanceCE; // (distance in mm)
		}
	}

	///
	void							changeAssociation()
	{
		++AssociationChangeBits; // increment counter
	}

	/// Return true if the association was suppressed (using unassociate()) and no new association was done yet
	bool							associationSuppressed() const
	{
		return !SlotUsed;
	}

	///
	void							associate()
	{
		SlotUsed = true;
	}
	
	///
	void							unassociate()
	{
		changeAssociation();
		SlotUsed = false;
	}

	///
	void							resetAssociation()
	{
		AssociationState = UnusedAssociation;
		AssociationChangeBits = 0;
//#ifdef NL_DEBUG
		PrevAssociationBits = 0;
//#endif
	}
};


/**
 * Vision Array.
 * It allows to access some data about the entities seen by the clients,
 * and the priorities of their properties. It is a 3D table.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CVisionArray
{
public:

	/// Constructor
	CVisionArray();

	/// Initialization
	void				init() {}

	/// Return the status of an item (see enum in TBKEntityInfo in client_entity_id_translator.h)
	uint8				getAssociationState( TClientId clientid, CLFECOMMON::TCLEntityId ceid ) const
	{
		return _Array[clientid][ceid].AssociationState;

/*
		return client->IdTranslator.isUsed(ceid) ?
					client->IdTranslator.getInfo(ceid).AssociationState :
					CClientEntityIdTranslator::CEntityInfo::UnusedAssociation;
*/
	}

	/// Set the status of an item (see enum in TBKEntityInfo in client_entity_id_translator.h)
	void				setAssociationState(TClientId clientid, CLFECOMMON::TCLEntityId ceid, uint8 state )
	{
		_Array[clientid][ceid].AssociationState = state;
/*
		client->IdTranslator.getInfo(ceid).AssociationState = state;
*/
	}


	/// Return the Entity Index of an entity
	void				setEntityIndex( TClientId clientid, CLFECOMMON::TCLEntityId slot, TEntityIndex entityindex )
						{ _Array[clientid][slot].EntityIndex = entityindex; }

	/// Return the Entity Index of an entity
	TEntityIndex		getEntityIndex( TClientId clientid, CLFECOMMON::TCLEntityId slot ) const
						{ return _Array[clientid][slot].EntityIndex; }

	/// Print the contents of an item, except the properties (debugging)
	//void				printItem( TClientId clientid, CLFECOMMON::TCLEntityId ceid ) const;

	/// Access

	/* Return the seen entity that has the biggest distance with the client, and the corresponding
	 * entity id as an out argument, or NULL if there is no visible entity at all.
	 * Takes into account the entites in NormalState or PendingAssociation.
	 */
	//TVAItem				*currentFurthestSeenEntity( TClientId clientid, TCLEntityId& resultceid );

	// Set new priority
	//void				setPriority( TClientId clientid, CLFECOMMON::TCLEntityId ceid, CLFECOMMON::TPropIndex propindex, TPriority newprio );


	///
	TPairState&			getPairState( TClientId clientId, CLFECOMMON::TCLEntityId slot )
	{
		return _Array[clientId][slot];
	}

	///
	TPairState*			getClientStateArray(TClientId clientId)
	{
		return _Array[clientId];
	}

protected:

	/// Easy and safe access to the client host object
	static CClientHost	*clientHost( TClientId clientid );

private:

	/// The array
	TPairState				_Array [MAX_NB_CLIENTS+1] [256];

};


#endif // NL_VISION_ARRAY_H

/* End of vision_array.h */
