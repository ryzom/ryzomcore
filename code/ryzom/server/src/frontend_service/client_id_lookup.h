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



#ifndef NL_CLIENT_ID_LOOKUP_H
#define NL_CLIENT_ID_LOOKUP_H

#include "nel/misc/types_nl.h"
#include "game_share/ryzom_entity_id.h"
#include "fe_types.h"


/**
 * CClientIdLookup.
 * Allows to translate an entity CEntityId to a TClientId if the specified entity
 * is played by a client connected on this frontend service, and
 * also translates a TEntityIndex to a TClientId.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CClientIdLookup
{
public:

	/// Constructor
	CClientIdLookup() {}

	/// Init
	void					init( TDataSetIndex maxNbRows )
	{
		_EntityIndexToClient.resize( maxNbRows );
		uint i;
		for ( i=0; i!=maxNbRows; ++i )
		{
			_EntityIndexToClient[i] = INVALID_CLIENT;
		}
	}

	/** Return the client id corresponding to the specified CEntityId,
	 * or INVALID_CLIENT if there is no such client on this frontend service.
	 */
	TClientId				getClientId( const NLMISC::CEntityId& id )
	{
		//nlinfo( "get: %u elements", _IdToClientMap.size() );
		TIdToClientMap::const_iterator iecm = _IdToClientMap.find( id );
		if ( iecm != _IdToClientMap.end() )
			return (*iecm).second;
		else
			return INVALID_CLIENT;
	}

	/** Return the client id corresponding to the specified entity index (fast),
	 * or INVALID_CLIENT if there is no such client on this frontend service.
	 */
	TClientId				getClientId( const TEntityIndex& entityindex )
	{
		return _EntityIndexToClient[entityindex.getIndex()];
	}

	/// Test if an entity id is present
	bool					findEntityId( const NLMISC::CEntityId& id )
	{
		return _IdToClientMap.find( id ) != _IdToClientMap.end();
	}

	/// Add an id mapping
	bool					addId( const NLMISC::CEntityId& id, TClientId clientid )
	{
		nlassert( clientid <= MaxNbClients );
		return _IdToClientMap.insert( std::make_pair( id, clientid ) ).second; // false if Entity Index added twice to CClientIdLookup
		//nlinfo( "add: %u elements", _IdToClientMap.size() );
	}

	/// Add an entity index mapping for fast access
	void					addEntityIndex( const TEntityIndex& entityindex, TClientId clientid )
	{
		nlassert( clientid <= MaxNbClients );
		if ( _EntityIndexToClient[entityindex.getIndex()] != INVALID_CLIENT )
			nlwarning( "Trying to remap an entityindex to a different clientid");
		_EntityIndexToClient[entityindex.getIndex()] = clientid;
	}

	/// Remove a id mapping
	void					removeEId( const NLMISC::CEntityId& id )
	{
		if ( _IdToClientMap.erase( id ) == 0 )
			nlwarning ("Cannot find EntityId %s to remove in EntityToClient (multiple use of the same account?)", id.toString().c_str() );
	}

	/// Remove an entity index mapping
	void					removeEntityIndex( const TEntityIndex& entityindex )
	{
		_EntityIndexToClient[entityindex.getIndex()] = INVALID_CLIENT;
	}

	
private:

	typedef std::vector<TClientId> TEntityIndexToClient;

	class CIdHash
	{
	public:

		size_t	operator () ( NLMISC::CEntityId id ) const { return (uint32)id.getShortId(); }
	};

	typedef CHashMap<NLMISC::CEntityId, TClientId, CIdHash>	TIdToClientMap;

	/// Access TClientId indexed by TEntityIndex (if the entity is a client connected on this FS)
	TEntityIndexToClient		_EntityIndexToClient;

	/// Access TClientId using a id
	TIdToClientMap				_IdToClientMap;
};


#endif // NL_CLIENT_ID_LOOKUP_H

/* End of client_id_lookup.h */
