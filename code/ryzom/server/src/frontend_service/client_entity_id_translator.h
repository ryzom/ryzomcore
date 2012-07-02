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



#ifndef NL_CLIENT_ENTITY_ID_TRANSLATOR_H
#define NL_CLIENT_ENTITY_ID_TRANSLATOR_H

//#include <map>
#include <vector>
#include "nel/misc/types_nl.h"

#include "game_share/entity_types.h"
#include "fe_types.h"

//const TFTEntityId	UnusedEntityId = 0xFFFF;
//const CLFECOMMON::TCLEntityId	InvalidCEId = 0;

const CLFECOMMON::TCLEntityId	FirstUsedCEId = 0;
const CLFECOMMON::TCLEntityId	LastUsedCEId = 255;


/**
 * Allows to use shorter entity ids (TCLEntityId).
 * An entity can be referred to by a pair (ClientId, CLEntityId) when the
 * client ClientId "can see" the entity CLEntityId. Consequently, entities
 * that are not seen by any client won't be stored in CClientEntityIdTranslator.
 *
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CClientEntityIdTranslator
{
public:
	typedef std::deque<CLFECOMMON::TCLEntityId>	TCEIdQueue;
/*	// CHANGED BEN
	class CEntityInfo
	{
	public:
		CEntityInfo(const TEntityIndex &index = TEntityIndex())
		{
			clearSlot(index);
			//AssociationState = UnusedAssociation;
			//AssociationChannel = -1;
		}

		TEntityIndex			EntityInSlot;

		//bool					Used;

		//sint8					AssociationChannel;	// the impulse channel from which the association was sent
		//uint8					AssociationState;


		enum
		{
			UnusedAssociation = 0,			// Pair is not used
			AwaitingAssAck = 1,				// Association was sent to the client
			NormalAssociation = 2,			// Pair is used
			AwaitingDisAck = 3,				// Disassociation was sent to the client
			//SubstitutionBeforeDisAck = 4,	// Disassociation then association were sent to the client
			//SubstitutionAfterDisAck = 5,	// The client acknowledged disassociation of substitution
			//CancelledSubstitution = 6		// A disassociation occurs before the client acknowledges the disassociation of substitution
		};

		void	clearSlot( const TEntityIndex& index )
		{
			EntityInSlot = index;
			//AssociationState = UnusedAssociation; // handled by vision provider
			// Keep association channel for message ordering
			//Used = false;
		}
	};
*/

private:

	class CHash
	{
	public:
		size_t	operator () ( const TEntityIndex& index ) const { return index.getIndex(); }
	};

	// table to map CEntityId to TCLEntityId
	typedef CHashMap<TEntityIndex, CLFECOMMON::TCLEntityId, CHash>	TCEIdMap;

	/// Associates a CEntityId to a unique TCLEntityId
	TCEIdMap		_ClientEntityIdMap;

	/*
	 *
	 */
	class CCEIdBackMap
	{
	private:
		//std::vector<CEntityInfo>		_Table;	// CHANGED BEN
		//CEntityInfo						_Table[MAX_SEEN_ENTITIES_PER_CLIENT];	// CHANGED BEN
		TEntityIndex					_Table[MAX_SEEN_ENTITIES_PER_CLIENT];

		TCEIdQueue						_Free, _Used;

		uint							_NbUsed;

	public:
		//CCEIdBackMap() : _Table(MAX_SEEN_ENTITIES_PER_CLIENT), _NbUsed(0) {}
		CCEIdBackMap() : _NbUsed(0) {}

		//CLFECOMMON::TCLEntityId		allocCEId(const TEntityIndex& id);

		void						setCEIdIndex( CLFECOMMON::TCLEntityId ceid, const TEntityIndex& entityindex )
		{
			//_Table[ceid].clearSlot( entityindex );	// CHANGED BEN
			_Table[ceid] = entityindex;
			++_NbUsed;
		}

		void						freeCEId(CLFECOMMON::TCLEntityId ceid)
		{
			//_Table[ceid].clearSlot( TEntityIndex() );	// CHANGED BEN
			_Table[ceid] = TEntityIndex();
			--_NbUsed;
		}

		//TEntityIndex				convertId(CLFECOMMON::TCLEntityId id) const { return _Table[id].EntityIndex; }
		bool						isUsed(CLFECOMMON::TCLEntityId id) const	{ return (id < MAX_SEEN_ENTITIES_PER_CLIENT) /*&& (_Table[id].Used)*/; }
		uint						usedId() const								{ return _NbUsed; }
		//CEntityInfo					&getInfo(CLFECOMMON::TCLEntityId id)		{ return _Table[id]; }	// CHANGED BEN
		//const CEntityInfo			&getInfo(CLFECOMMON::TCLEntityId id) const	{ return _Table[id]; }	// CHANGED BEN

		//
		const TCEIdQueue	&getUsed() const									{ return _Used; }
	};



	/// Associates a TCLEntityId to a unique CEntityId
	CCEIdBackMap	_ClientEntityIdBackMap;


	/// Id of the translator
	uint32			_Id;


public:

	/// Constructor
	CClientEntityIdTranslator();

	/// Set the translator Id
	void			setId(uint32 id) { _Id = id; }

	// Allocates a new id for the given entity dedicated to the associated client, returns InvalidCEId if it failed
	//CLFECOMMON::TCLEntityId		getNewCEId(const TEntityIndex& id);

	/// Notify the given entity is no longer used
	void			releaseId(const TEntityIndex& id);

	// Notify the given entity is no longer used
	//void			releaseId(CLFECOMMON::TCLEntityId id);


	/// Tells if the given id (CEntityId) is currently associated to a TCLEntityId
	//bool			isUsed(const TEntityIndex& id) const	{ return _ClientEntityIdMap.find(id) != _ClientEntityIdMap.end(); }

	/// Tells if the given id (client id) is currently associated to a CEntityId
	bool			isUsed(CLFECOMMON::TCLEntityId id) const	{ return _ClientEntityIdBackMap.isUsed(id); }

	/// Returns the number of association currently used
	uint			usedCEId() const						{ return _ClientEntityIdBackMap.usedId(); }

	/// Returns a deque of used CLEntityIds
	const TCEIdQueue	&getUsedIds() const					{ return _ClientEntityIdBackMap.getUsed(); }

/*
	// Returns the frontent entity id associated to the given client entity id
	TEntityIndex	getEntityId(CLFECOMMON::TCLEntityId id) const
	{
		TEntityIndex	index = _ClientEntityIdBackMap.convertId(id);
		nlassertex(index != INVALID_ENTITY_INDEX, ("_Id=%d CLEntityId=%d", _Id, id));
		return index;
	}
*/

	/// Returns the client entity id associated to the given frontend entity id
	CLFECOMMON::TCLEntityId		getCEId(const TEntityIndex& id) const
	{
		//nlinfo( "Searching for id %u", id );
		//TCEIdMap::const_iterator	it;// = _ClientEntityIdMap.find(id);
		//for ( it=_ClientEntityIdMap.begin(); it!=_ClientEntityIdMap.end(); ++it )
		//{
		//	nlinfo( "%u %hu", (*it).first, (uint16)(*it).second );
		//}

		TCEIdMap::const_iterator it = _ClientEntityIdMap.find(id);
//		nlassertex(it != _ClientEntityIdMap.end(), ("_Id=%d FTEntityId=%d", _Id, id));
		if (it == _ClientEntityIdMap.end())
		{
			//nlstopex( ("not found id=%u", id) );
			return CLFECOMMON::INVALID_SLOT;
		}
		return (*it).second;
	}

	// Returns the client entity id associated to the given frontend entity id (and creates one if not yet associated, returning UnusedEntityId if it failed)
	/*CLFECOMMON::TCLEntityId	getAlwaysCEId (const TEntityIndex& id)
	{
		TCEIdMap::iterator	it = _ClientEntityIdMap.find(id);
		return (it != _ClientEntityIdMap.end()) ? (*it).second : getNewCEId(id);
	}*/

	///
	bool				acquireCEId( const TEntityIndex& entityindex, CLFECOMMON::TCLEntityId ceid )
	{
		//nlinfo( "Acquiring %u with %hu", entityindex, (uint16)ceid );
		if ( ! _ClientEntityIdMap.insert( std::make_pair( entityindex, ceid ) ).second )
		{
			nlwarning( "Client %d: Cannot acquire entity %u; already used with slot %hu", _Id, entityindex.getIndex(), (uint16)ceid );
			return false;
		}
		_ClientEntityIdBackMap.setCEIdIndex( ceid, entityindex );
		return true;
	}

	// Returns the client entity id associated to the given frontend entity id (and creates one if not yet associated)
	//CLFECOMMON::TCLEntityId	operator [] (const TEntityIndex& id);

	/// Returns the TFTEntityInfo associated to a TCLEntityId
	//CEntityInfo			&getInfo(CLFECOMMON::TCLEntityId id)			{ return _ClientEntityIdBackMap.getInfo(id); }	// CHANGED BEN

	/// Returns the TFTEntityInfo associated to a TCLEntityId (const version)
	//const CEntityInfo	&getInfo(CLFECOMMON::TCLEntityId id) const		{ return _ClientEntityIdBackMap.getInfo(id); }	// CHANGED BEN

	/// Returns the TFTEntityInfo associated to a TFTEntityId
/*	CEntityInfo		&getInfo(const TEntityIndex& id)	// CHANGED BEN
	{
		CLFECOMMON::TCLEntityId	clid = getCEId(id);
		return _ClientEntityIdBackMap.getInfo(clid);
	}
*/

	void			displayEntityIdMap( TClientId clientid )
	{
		nlinfo( "EntityIdMap of client %d", _Id );
		TCEIdMap::iterator im;
		for ( im=_ClientEntityIdMap.begin(); im!=_ClientEntityIdMap.end(); ++im )
		{
			nlinfo( "Entity %u: slot %hu", (*im).first.getIndex(), (uint16)((*im).second) );
		}
	}

};

#endif // NL_CLIENT_ENTITY_ID_TRANSLATOR_H

/* End of client_entity_id_translator.h */
