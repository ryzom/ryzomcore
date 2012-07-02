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



#ifndef NL_MIRRORED_DATA_SET_INLINE_H
#define NL_MIRRORED_DATA_SET_INLINE_H

#include <nel/misc/types_nl.h>
//#include <sstream>


const uint MAX_NB_DATASETS = 4;
extern bool VerboseWarnWhenMirrorReturningUnknownEntityId;



/*
 * Accessors (similar to instanciating CMirrorPropValue<T>)
 */
template <class T, class CPropLocationClass>
void CMirroredDataSet::setValue( const TDataSetRow& entityIndex, TPropertyIndex propertyIndex, T value )
{
	CMirrorPropValue<T,CPropLocationClass> propValue ( this, entityIndex, propertyIndex );
	propValue = value;
}

template <class T, class CPropLocationClass>
void CMirroredDataSet::setValue( const NLMISC::CEntityId& entityId, const std::string& propName, T value )
{
	CMirrorPropValue<T,CPropLocationClass> propValue ( this, entityId, propName );
	propValue = value;
}

template <class T>
void CMirroredDataSet::getValue( const NLMISC::CEntityId& entityId, const std::string& propName, T& returnedValue ) const
{
	T *pvalue;
	getPropPointer( &pvalue, getPropertyIndex( propName ), getDataSetRow( entityId ) );
	returnedValue = *pvalue;
}


/*
 * Get the entity id corresponding to entityIndex in the dataset
 */
inline const NLMISC::CEntityId& CMirroredDataSet::getEntityId( const TDataSetRow& entityIndex ) const
{
	if ( !entityIndex.isValid())
	{
#ifdef NL_DEBUG
		if ( VerboseWarnWhenMirrorReturningUnknownEntityId )
			nlwarning( "MIRROR: Returning CEntityId::Unknown" );
#endif
		return NLMISC::CEntityId::Unknown;
	}
#ifdef NL_DEBUG
	nlassertex( (uint32)entityIndex.getIndex() < (uint32)maxNbRows(), ("E%u",entityIndex.getIndex()) );  // Wrong entity (including INVALID_ENTITY_INDEX and LAST_CHANGED)
	nlassert( _PropertyContainer.EntityIdArray.EntityIds );
#else
	/*if ( ! isDataSetRowStillValid( entityIndex ) )
	{
		//nlwarning( "Not returning a new entityId of reassigned row!" );
		//nlstop;
		return NLMISC::CEntityId::Unknown;
	}*/
	if ( (uint32)entityIndex.getIndex() >= (uint32)maxNbRows() )
	{
		nlwarning("ERROR: getEntityId() with row out of range");
		return NLMISC::CEntityId::Unknown;
	}

	if ( ! _PropertyContainer.EntityIdArray.EntityIds )
	{
		nlwarning("ERROR: getEntityId() before the mirror is ready");
		return NLMISC::CEntityId::Unknown;
	}

#endif
	return _PropertyContainer.EntityIdArray.EntityIds[entityIndex.getIndex()];
}


/*
 * Get the entity index corresponding to the specified entity id in the dataset
 */
inline TDataSetRow CMirroredDataSet::getDataSetRow( const NLMISC::CEntityId& entityId ) const
{
	//nldebug( "Searching for %s", entityId.toString().c_str() );
	TEntityIdToEntityIndexMap::const_iterator iei = _EntityIdToEntityIndex.find( entityId );
	if ( iei != _EntityIdToEntityIndex.end() )
	{
#ifdef CHECK_DATASETROW_VALIDITY
		uint32 index = GET_ENTITY_INDEX(iei);
		return TDataSetRow( index, _PropertyContainer.EntityIdArray.Counts[index] );
#else
		return GET_ENTITY_INDEX(iei);
#endif
	}
	else
	{
		return TDataSetRow();
	}
}


/*
 * Return true if the row is currently accessible, meaning the corresponding entity is online now
 * and can be accessed in the mirror. If row.isNull(), return false.
 * If another service B on the same machine despawns the entity in the same game cycle, after
 * isAccessible() returned true in A, the row can still be accessed by A.
 *
 * See WARNING section in .h comment.
 *
 * Implementer's notes:
 * - It implies that accessing the mirror with the row must be possible even if the row counter
 * has been incremented by 1 by the despawn. That's why isDataSetRowStillValid() handles this
 * case.
 * - A mutual exclusion is not necessary for isOnline(), since the counter is changed before the
 * online flag, in shared memory.
 */
inline bool					CMirroredDataSet::isAccessible( const TDataSetRow& row ) const
{
	return //(! row.isNull()) && // guaranteed by (row.getIndex() < maxNbRows()) *as long as the index is unsigned*
		   (row.getIndex() < maxNbRows()) &&
		   (_PropertyContainer.EntityIdArray.isOnline( row.getIndex() )) &&
		   ( (row.counter() == _PropertyContainer.EntityIdArray.Counts[row.getIndex()]) ||
		     (row.counter() == 0) );
}


/*
 * Same as isAccessible(), but does not check if the row is online.
 * Thus it will return true for a row that is created locally but yet unpublished
 * (which is not called "online").
 * Caution: it will also return true if the row is offline, provided the current
 * counter matches the counter in 'row'.
 */
inline bool					CMirroredDataSet::isAccessible2( const TDataSetRow& row ) const
{
	return //(! row.isNull()) && // guaranteed by (row.getIndex() < maxNbRows()) *as long as the index is unsigned*
		   (row.getIndex() < maxNbRows()) &&
		   ( (row.counter() == _PropertyContainer.EntityIdArray.Counts[row.getIndex()]) ||
		     (row.counter() == 0) );
}


/*
 * Get a valid datasetrow usable with the current entity at entity index, if the entity is online.
 * If the entity is offline, the method returns an invalid datasetrow (offline means the row is not used;
 * warning: after despawning an entity by a service A, and before notifying the entity removal in a
 * service B, the entity will be reported as offline).
 */
inline TDataSetRow			CMirroredDataSet::getCurrentDataSetRow( TDataSetIndex entityIndex ) const
{
	if ( entityIndex != INVALID_DATASET_INDEX )
	{
		TDataSetRow datasetrow( entityIndex, getBindingCounter( entityIndex ) );
		if ( _PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
			return datasetrow;
	}
	return TDataSetRow();
}

/*
 * Get a valid datasetrow usable with the current entity at entity index, be the entity online or not.
 * warning: after despawning an entity by a service A, and before notifying the entity removal in a
 * service B, the entity will be reported as offline).
 */
inline TDataSetRow			CMirroredDataSet::forceCurrentDataSetRow( TDataSetIndex entityIndex ) const
{
	if ( entityIndex != INVALID_DATASET_INDEX )
		return TDataSetRow( entityIndex, getBindingCounter( entityIndex ) );
	return TDataSetRow();
}

/*
 * Return true if the specified property represents a list and not a single value
 */
inline bool			CMirroredDataSet::propIsList( TPropertyIndex propIndex ) const
{
#ifdef NL_DEBUG
	nlassert( (uint)propIndex < _PropIsList.size() );
#endif
	return _PropIsList[propIndex];
}



/*
 * Display the values of one property for all entities
 */
template <class T, class CPropLocationClass>
inline void			CMirroredDataSet::displayPropValues( TPropertyIndex propIndex, T* pt, NLMISC::CLog& log ) const
{
//	std::stringstream ss;
//	ss << "Mirror property " << propIndex << ":" << endl;
//	TEntityIdToEntityIndexMap::const_iterator iei;
//	for ( iei=_EntityIdToEntityIndex.begin(); iei!=_EntityIdToEntityIndex.end(); ++iei )
//	{
//		CMirrorPropValue<T,CPropLocationClass> value( *this, (*iei).second, propIndex );
//		ss << "E%d:" << (*iei).second << ": " << value() << endl;
//	}

	std::string str;
	str = "Mirror property " + NLMISC::toString(propIndex) + ":\n";
	TEntityIdToEntityIndexMap::const_iterator iei;
	for ( iei=_EntityIdToEntityIndex.begin(); iei!=_EntityIdToEntityIndex.end(); ++iei )
	{
		CMirrorPropValue<T,CPropLocationClass> value( *this, (*iei).second, propIndex );
		str += "E" + NLMISC::toString((*iei).second) + ": " + NLMISC::toString(value()) << "\n";
	}

	log.displayNL( "%s", str.c_str() );
}


/*
 * Access the map of entities (begin iteration). Use GET_ENTITY_INDEX(it) to access the TDataSetRow index.
 * You can iterate it to get the online entities (use GET_ENTITY_INDEX to obtain a dataset index from
 * the iterator, then call getCurrentDataSetRow() to have a dataset row, and check isValid() in the case
 * when the entity would have been despawned and the service not notified yet.
 */
TEntityIdToEntityIndexMap::const_iterator	CMirroredDataSet::entityBegin() const
{
	return _EntityIdToEntityIndex.begin();
}


/*
 * Access the map of entities (test end of iteration).
 */
TEntityIdToEntityIndexMap::const_iterator	CMirroredDataSet::entityEnd() const
{
	return _EntityIdToEntityIndex.end();
}



extern CMirroredDataSet *DataSetQuickArray [MAX_NB_DATASETS];

inline uint16 datasetToBitIndex( CMirroredDataSet *dataSet )
{
	// Could be stored in CMirroredDataSet, instead of browsing the table
	for ( uint i=0; i!=MAX_NB_DATASETS; ++i )
	{
		if ( DataSetQuickArray[i] == dataSet )
			return (uint16)i;
	}
	return (uint16)~0;
}


inline CMirroredDataSet *bitIndexToDataset( uint16 bitIndex )
{
#ifdef NL_DEBUG
	nlassert( bitIndex < MAX_NB_DATASETS );
#endif
		return DataSetQuickArray[bitIndex]; // we must not use the index MAX_NB_DATASETS-1 to have NULL
}


inline CMirroredDataSet *firstDataset()
{
	return DataSetQuickArray[0];
}


inline void initDatasetQuickArray()
{
	for ( uint i=0; i!=MAX_NB_DATASETS; ++i )
	{
		DataSetQuickArray[0] = NULL;
	}
}

void storeDatasetPtToQuickArray( uint16 bitIndex, CMirroredDataSet *dataSet );


#endif // NL_MIRRORED_DATA_SET_INLINE_H

/* End of mirrored_data_set.h */
