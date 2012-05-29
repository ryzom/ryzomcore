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



#ifndef NL_VISION_PROVIDER_H
#define NL_VISION_PROVIDER_H

#include "nel/misc/types_nl.h"
#include "fe_types.h"
#include "game_share/ryzom_entity_id.h"
#include "processing_spreader.h"
#include "client_entity_id_translator.h"
#include "vision_receiver.h"


/*
 * Forward declarations
 */
class CVisionArray;
class CHistory;
class CClientHost;
class CPropertyIdTranslator;
class CClientIdLookup;


/*
 * Container for pending new pairs
 */
/*struct TPairClientEntity
{
	TPairClientEntity() : ClientId(INVALID_CLIENT), SeenEntity() {}

	TPairClientEntity( TClientId clientid, TEntityIndex entityindex )
	{
		ClientId = clientid;
		SeenEntity = entityindex;
	}

	friend bool operator==( const TPairClientEntity& e1, const TPairClientEntity& e2 )
	{
		return (e1.ClientId == e2.ClientId) && (e1.SeenEntity == e2.SeenEntity );
	}

	TClientId		ClientId;
	TEntityIndex	SeenEntity;
};*/


/*
 * Container for items of observer set
 * <ClientId, observer entity slot>
 */
/*struct TPairClientSlot
{
	// Default constructor
	TPairClientSlot() : ClientId(INVALID_CLIENT), Slot(0) {}

	TPairClientSlot( TClientId clientid, CLFECOMMON::TCLEntityId slot )
	{
		ClientId = clientid;
		Slot = slot;
	}

	// Comparison operator
	friend bool operator==( const TPairClientSlot& e1, const TPairClientSlot& e2 )
	{
		return (e1.ClientId == e2.ClientId) && (e1.Slot == e2.Slot );
	}

	// Hash function: ClientId * 256 + CeId
	struct CHash
	{
		size_t		operator() ( TPairClientSlot pair ) const { return (pair.ClientId << 8) + pair.Slot; }
	};

	TClientId					ClientId;
	CLFECOMMON::TCLEntityId		Slot;
};*/

//typedef std::hash_set<TPairClientSlot, TPairClientSlot::CHash > TObserverList;


/*struct TPairClientSlotWithReplace : public TPairClientSlot
{
	TPairClientSlotWithReplace( bool replace=false ) :
								TPairClientSlot(), Replace(replace) {}

	TPairClientSlotWithReplace( TClientId clientid, CLFECOMMON::TCLEntityId slot, bool replace=false ) :
								TPairClientSlot( clientid, slot ), Replace(replace) {}

	bool						Replace;
};*/


/**
 * Vision Provider.
 * It checks the differences in the vision (coming from the back-end)
 * and decides if a new declared entity will be seen by a client.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CVisionProvider
{
public:

	/// Constructor
	CVisionProvider();

	/// Destructor
	~CVisionProvider();
							// _______________________________
							// For the priority subsystem root

	/// Initialization
	void					init( CVisionArray *va, CHistory *h, CClientIdLookup *cl );

	/// Process the vision differences
	void					processVision();

	// Return the observer list for displaying
	//const TObserverList&	observerList( const TEntityIndex& entityindex ) const { return _ObserverList[entityindex]; }

							// _________________________
							// For the receive subsystem

	// Remove item from observer list of entityindex
	//void					removeFromObserverList( TEntityIndex entityindex, TClientId clientid, CLFECOMMON::TCLEntityId ceid );

	/// Reset all slots of all clients (e.g. when GPMS falls down)
	void					resetVision();

	/// Reset all slots of one client (e.g. when a client is unspawned)
	void					resetVision( CClientHost *clienthost );

	/// Display the properties of an entity in the vision
	void					displayEntityInfo( const CEntity& e, const TEntityIndex& entityIndex, NLMISC::CLog *log = NLMISC::InfoLog ) const;

	/// Reset assoc/disas counters
	void					resetAssocCounter();

	/// Display assoc/disas freqs
	void					displayAssocFreqs(NLMISC::CLog *log = NLMISC::InfoLog);

	///
	void					postRemovePair( TClientId clientid, CLFECOMMON::TCLEntityId slot );

	uint32					AssocCounter;
	uint32					DisasCounter;
	NLMISC::TTime			AssocStartTime;

	CProcessingSpreader		DistanceSpreader;

protected:

	/// Remove a pair. The client id cannot be INVALID_CLIENT
	void					removePair( TClientId clientid, CLFECOMMON::TCLEntityId slot );

	/// Add a pair. The argument 'furthestceid' is used when removing an item is required.
	bool					addPair( TClientId clientid, const TEntityIndex& entityindex, CLFECOMMON::TCLEntityId slot );

	/// Replace the entity in a slot (e.g. when transforming a dead character to a sack)
	void					replacePair( TClientId clientid, const TEntityIndex& newEntityIndex, CLFECOMMON::TCLEntityId slot );

	/// Easy access to the client host object. Return NULL and makes warning if not found.
	static CClientHost		*clientHost( TClientId clientid );

	/// Calculate the absolute distance between a client and an entity
	CLFECOMMON::TCoord		calcDistance( CClientHost *client, CLFECOMMON::TCLEntityId slot, const TEntityIndex& seenIndex );

private:

	/// Entities seen by the clients and the priorities of their properties
	CVisionArray				*_VisionArray;

	/// History
	CHistory					*_History;

	/// Entity to client
	CClientIdLookup				*_EntityToClient;

	// Lists of clients who can see the entities, indexed by TEntityIndex (corresponding to the frontend property receiver)
	// NOT NEEDED in the new prioritization method
	//TObserverList				_ObserverList[MAX_NB_ENTITIES];

	CVisionReceiver				_VisionReceiver;

};


#endif // NL_VISION_PROVIDER_H

/* End of vision_provider.h */
