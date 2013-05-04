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



#ifndef NL_FE_TYPES_H
#define NL_FE_TYPES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/net/inet_address.h"
#include "game_share/entity_types.h"
#include "game_share/mirror.h"
#include <vector>

typedef NLMISC::CBitMemStream TOutBox;

#undef SIMUL_CLIENTS
//#define SIMUL_CLIENTS


typedef uint16 TClientId; // range about 1..1000
extern uint MaxNbClients; // config file controlled
const uint MAX_NB_CLIENTS = 1000; // constant for raw arrays
const uint MAX_SEEN_ENTITIES_PER_CLIENT = 255; // per client

const uint16 INVALID_CLIENT = 0xFFFF;



class CClientHost;

/// Type of client container indexed by TClientId
typedef std::vector<CClientHost*> TClientIdCont;
#define GETCLIENTI(it) (*it)


/// Retrieving client id by entity id (initialized once and for all)
typedef std::vector<TClientId> TEntityToClient;


// Type of indexed in property receiver (old)
//typedef uint32 TEntityIndex;
//const TEntityIndex INVALID_ENTITY_INDEX = 0xFFFFFFFF;

/// Entity index type
typedef TDataSetRow TEntityIndex;



/// User account identifier
typedef uint32 TUid;



/**
 * CInetAddress hash function
 */
class CInetAddressHashMapTraits
{
public:
	static const size_t bucket_size = 4;
	static const size_t min_buckets = 8;
	inline size_t operator() ( const NLNET::CInetAddress& x ) const
	{
		//return x.port();
		return x.internalIPAddress();
	}
// 	bool operator() (const NLNET::CInetAddress &x1, const NLNET::CInetAddress &x2) const
// 	{
// 		return classId1 < classId2;
// 	}
};

/// Type of client map by address
typedef CHashMap<NLNET::CInetAddress,CClientHost*,CInetAddressHashMapTraits> THostMap;
#define GETCLIENTA(it) (*it).second

/// TEMP! fe/client time/tick types
//typedef sint64		TGameTime;
//typedef uint32		TGameTicks;

#endif // NL_FE_TYPES_H

/* End of fe_types.h */
