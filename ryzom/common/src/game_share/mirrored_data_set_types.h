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




#ifndef NL_MIRRORED_DATASET_TYPES_H
#define NL_MIRRORED_DATASET_TYPES_H

#include <nel/misc/types_nl.h>


#include <nel/misc/entity_id.h>
#include "data_set_base.h"
#include "change_tracker_client.h"

#include <queue>
#include <vector>


#define SEND_MSG_1V_TO_MIRROR( name, type1, value1 ) \
	CMessage msgout##name( #name ); \
	type1 type1##value1 = value1; \
	msgout##name.serial( type1##value1 ); \
	CUnifiedNetwork::getInstance()->send( localMSId(), msgout##name );

#define SEND_MSG_2V_TO_MIRROR( name, type1, value1, type2, value2 ) \
	CMessage msgout##name( #name ); \
	type1 type1##value1 = value1; \
	msgout##name.serial( type1##value1 ); \
	type2 type2##value2 = value2; \
	msgout##name.serial( type2##value2 ); \
	CUnifiedNetwork::getInstance()->send( localMSId(), msgout##name );

#define SEND_MSG_3V_TO_MIRROR( name, type1, value1, type2, value2, type3, value3 ) \
	CMessage msgout##name( #name ); \
	type1 type1##value1 = value1;\
	msgout##name.serial( type1##value1 ); \
	type2 type2##value2 = value2; \
	msgout##name.serial( type2##value2 ); \
	type3 type3##value3 = value3; \
	msgout##name.serial( type3##value3 ); \
	CUnifiedNetwork::getInstance()->send( localMSId(), msgout##name );

#define SEND_MSG_1P_TO_MIRROR( name, value1 ) \
	CMessage msgout##name( #name ); \
	msgout##name.serial( value1 ); \
	CUnifiedNetwork::getInstance()->send( localMSId(), msgout##name );

#define SEND_MSG_2P_TO_MIRROR( name, value1, value2 ) \
	CMessage msgout##name( #name ); \
	msgout##name.serial( value1 ); \
	msgout##name.serial( value2 ); \
	CUnifiedNetwork::getInstance()->send( localMSId(), msgout##name );

#define SEND_MSG_3P_TO_MIRROR( name, value1, value2, value3 ) \
	CMessage msgout##name( #name ); \
	msgout##name.serial( value1 ); \
	msgout##name.serial( value2 ); \
	msgout##name.serial( value3 ); \
	CUnifiedNetwork::getInstance()->send( localMSId(), msgout##name );

#define SEND_MSG_1PC_TO_MIRROR( name, value1 ) \
	CMessage msgout##name( #name ); \
	msgout##name.serialCont( value1 ); \
	CUnifiedNetwork::getInstance()->send( localMSId(), msgout##name );


struct TRowAllocation
{
	TRowAllocation( NLMISC::TGameCycle tick, TDataSetIndex row ) : RemovalDate(tick), EntityIndex(row) {}

	NLMISC::TGameCycle	RemovalDate;
	TDataSetIndex		EntityIndex;
};


/**
 * Row allocation management, within a range.
 * TODO: maybe, push all rows into the FILO at beginning (otherwise when there are
 * only a few entities, the time between releasing a row and reusing it might be short).
 */
class TEntityRange
{
public:

	/// Constructor
	TEntityRange( TDataSetIndex first=0, TDataSetIndex last=0 )
	{
		initRange( first, last );
	}

	/// Init range
	void				initRange( TDataSetIndex first, TDataSetIndex last )
	{
		_First = first; _OuterBound = last+1; _NextFree = first;
	}

	/// Acquire a new row. Set *previouslyUsed to true if the row was used and released before
	TDataSetIndex		getNewEntityIndex( bool *previouslyUsed );

	/// Release a row previously acquired
	void				releaseEntityIndex( TDataSetIndex entityIndex );

	/// Return the next free row (not among previously released rows)
	TDataSetIndex		nextFree() const { return _NextFree; }

	/// Return the first row of the range
	TDataSetIndex		first() const { return _First; }

	/// Return the last row of the range
	TDataSetIndex		last() const { return _OuterBound-1; }

	/// Return the number of rows ready to be given back
	TDataSetIndex		nbReleasedIndexes() const { return (TDataSetIndex)_ReleasedIndexes.size(); }

	/// Serial the range (minimal info)
	void				serial( NLMISC::IStream& s )
	{
		s.serial( _First );
		s.serial( _OuterBound );
	}

private:

	/// The first row that was never acquired before
	TDataSetIndex				_NextFree;

	/// The first row of the range
	TDataSetIndex				_First;

	/// One past the last row of the range
	TDataSetIndex				_OuterBound;

	/// FIFO container for released rows to give back
	std::queue<TRowAllocation>	_ReleasedIndexes;
};


/**
 * Information about declared range
 */
class TDeclaredEntityRange
{
public:

	/// Constructor
	TDeclaredEntityRange( const TEntityRange& src, NLNET::TServiceId8 declaratorService )
	{
		_First = src.first();
		_Size = src.last() - src.first() + 1;
		_ServiceId = declaratorService;
	}

	/// Base index of the range
	TDataSetIndex			baseIndex() const { return _First; }

	/// Size of the range
	TDataSetIndex			size() const { return _Size; }

	/// Declarator service id
	NLNET::TServiceId8					serviceId() const { return _ServiceId; }

	/// Serial
	/*void					serial( NLMISC::IStream& s )
	{
		s.serial( _First );
		s.serial( _Size );
		s.serial( _ServiceId );
	}

	friend bool operator<  ( const TDeclaredEntityRange& r1, const TDeclaredEntityRange& r2 );
	friend bool operator== ( const TDeclaredEntityRange& r1, const TDeclaredEntityRange& r2 );
	*/

private:

	TDataSetIndex			_First;
	TDataSetIndex			_Size;
	NLNET::TServiceId8		_ServiceId;
};


/*
/// Sorting operator
bool operator< ( const TDeclaredEntityRange& r1, const TDeclaredEntityRange& r2 )
{
	return r1.baseIndex() < r2.baseIndex();
}

/// Equality operator
bool operator== ( const TDeclaredEntityRange& r1, const TDeclaredEntityRange& r2 )
{
	return (r1.baseIndex() == r2.baseIndex()) && (r1.size() == r2.size());
}
*/


//typedef std::hash_map< uint8, TEntityRange, std::hash<uint> > TEntityRangeOfType;
typedef std::map< uint8, TEntityRange > TEntityRangeOfType;
#define GET_ENTITY_TYPE_RANGE(it) ((*it).second)

typedef std::multimap< uint8, TDeclaredEntityRange > TDeclaredEntityRangeOfType;
//define GET_ENTITY_TYPE_RANGE(it) ((*it).second)

typedef std::vector< CChangeTrackerClientProp > TSelfPropTrackers;

typedef std::vector< CChangeTrackerClient > TTrackerListForProp;
typedef std::vector< CChangeTrackerClient > TTrackerListForEnt;
//typedef std::vector< CEntityTrackerFilter > TEntityTrackerFilterList;

typedef std::vector< TTrackerListForProp > TTrackersLists;

template <class T, class CPropLocationClass>
class CMirrorPropValue;

class CPropertyAllocatorClient;


/// Service dest id for broacast
const NLNET::TServiceId DEST_MSG_BROADCAST(0);
/// Constant used internaly in broadcast message
const uint MSG_BROADCAST = 0;


#endif // NL_MIRRORED_DATASET_TYPES_H

/* End of mirror.h */
