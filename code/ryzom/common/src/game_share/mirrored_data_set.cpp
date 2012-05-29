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
#include "mirrored_data_set.h"
#include "property_allocator_client.h"
#include "mirror_prop_value.h"
#include "mirror.h"

#include "tick_event_handler.h"

#include <nel/georges/u_form_elm.h>
#include <nel/misc/command.h>

using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;
using namespace std;


NLMISC::CMemDisplayer *TmpDebugDisplayer = NULL;
//NLMISC::CLog		  *TmpDebugLogger = NULL;


extern CMirror *MirrorInstance;

CMirroredDataSet *DataSetQuickArray [MAX_NB_DATASETS];

bool VerboseMultipleChangesOfAProperty = false;
bool VerboseWarnWhenMirrorReturningUnknownEntityId = false;


#define LOG_TRACE(str) \
nlinfo( "%sTRACE:%u:%s", IService::getInstance()->getServiceShortName().c_str(), CTickEventHandler::getGameCycle(), str );


/*
 * Note: this quota prevents from looping infinitely if a service pushes too many changes per tick.
 * But it could lead to delay notifications (bad for row management!)
 */
//uint ChangeQuotaPerTick = 70000;


#ifdef NL_DEBUG

/*
 * Turn on/off the warning that displays in debug when multiples changes are
 * done to the same (entity, property) at the same tick (whether done by several
 * services or by the same one).
 */
NLMISC_CATEGORISED_VARIABLE(mirror, bool, VerboseMultipleChangesOfAProperty, "Turn on/off the warning that displays in debug when multiples changes are done to the same value at the same tick" );

/*
 * Turn on/off the warning that displays a warning in debug when getEntityId returns CEntityId::Unknown
 */
NLMISC_CATEGORISED_VARIABLE(mirror, bool, VerboseWarnWhenMirrorReturningUnknownEntityId, "Turn on/off the warning that displays a warning in debug when getEntityId returns CEntityId::Unknown" );

#endif


/*
 * Init (must be called when the local MS is known)
 */
void	CMirroredDataSet::init( const NLMISC::CSheetId& sheetId, const TDataSetSheet& properties, CPropertyAllocatorClient *pac )
{
	CDataSetBase::init( sheetId, properties );

	_PropTrackers.resize( properties.NbProperties );
	_ValueToStringCb.resize( properties.NbProperties, NULL );

	// Fill map of properties
	_PropAllocator = pac;
	for ( sint p=0; p!=properties.NbProperties; ++p )
	{
		_PropAllocator->_PropertiesInMirror.insert( make_pair( properties.PropertyNames[p], TPropertyInfo( this, p ) ) );
	}
	_PropAllocator->_PropertiesInMirror.insert( make_pair( string("_EId_")+properties.DataSetName, TPropertyInfo( this, INVALID_PROPERTY_INDEX ) ) );

	// Allocate a segment for entity ids
	// Should be done by the MS (reading the sheets of the datasets)
	_PropAllocator->allocProperty( ENTITYID_PREFIX + _DataSetName, 0, "" );

	/*if ( ! TmpDebugLogger )
	{
		TmpDebugDisplayer = new CMemDisplayer("TmpDebug"); // TODO: delete them in destructor
		TmpDebugDisplayer->setParam( 4000 );
		TmpDebugLogger = new CLog( CLog::LOG_DEBUG );
		TmpDebugLogger->addDisplayer( TmpDebugDisplayer );
	}*/
}


/*
 * Declare that the current service is interested in the specified property
 * (which must belong to the dataset).
 *
 * See doc in header file.
 */
void	CMirroredDataSet::declareProperty( const std::string& propName, TPropSubscribingOptions options,
										   const std::string notifyGroupByPropName )
{
	_PropAllocator->allocProperty( propName, options, notifyGroupByPropName );
	// will be ready when _PropAllocator->getPropertySegment( propName ) will not return NULL
}


/*
 * Add a range for an owned entity type
 */
const TEntityRange&	CMirroredDataSet::addEntityTypeRange( uint8 entityTypeId, TDataSetIndex first, TDataSetIndex last )
{
	if ( last >= _MaxNbRows )
	{
		nlerror( "MIRROR: Range for entity type %hu exceeds rows of dataset '%s' limited to %u (please check maxNbEntities in arguments of declareEntityTypeOwner())", (uint16)entityTypeId, name().c_str(), _MaxNbRows);
	}
	pair<TEntityRangeOfType::iterator,bool> pib = _EntityTypesRanges.insert( make_pair( entityTypeId, TEntityRange( first, last ) ) );
	if ( ! pib.second )
	{
		nlwarning( "MIRROR: addEntityTypeRange overwriting error (please check that there is only ONE range manager service)" );
	}
	return (*pib.first).second;
}


/*
 * Serial out owned entity ranges (without entity types)
 */
uint	CMirroredDataSet::serialOutOwnedRanges( NLMISC::IStream& s ) const
{
	nlassert( ! s.isReading() );
	uint32 nbRanges = (uint32)_EntityTypesRanges.size();
	s.serial( nbRanges );
	TEntityRangeOfType::const_iterator ir;
	for ( ir=_EntityTypesRanges.begin(); ir!=_EntityTypesRanges.end(); ++ir )
	{
		TDataSetIndex	first = GET_ENTITY_TYPE_RANGE(ir).first();
		TDataSetIndex	last = GET_ENTITY_TYPE_RANGE(ir).last();
		s.serial( first, last );
	}
	return nbRanges;
}


/*
 * Scan the rows for entities that we wouldn't know, and display them
 */
void	CMirroredDataSet::displayUnknownEntities( TDataSetIndex first, TDataSetIndex end, NLMISC::CLog& log )
{
	for ( TDataSetIndex i=first; i!=end; ++i )
	{
		const CEntityId& entityId = _PropertyContainer.EntityIdArray.EntityIds[i];
		if ( _PropertyContainer.EntityIdArray.isOnline( i ) &&
			 (_EntityIdToEntityIndex.find( entityId ) == _EntityIdToEntityIndex.end()) )
		{
			log.displayRawNL( "E%u_%hu %s", i, getBindingCounter( i ), entityId.toString().c_str() );
		}
	}
}


/*
 * Scan the rows for entities that we wouldn't know, and return the number of entities added
 */
uint	CMirroredDataSet::scanAndResyncExistingEntities( TDataSetIndex first, TDataSetIndex end, bool deleteGhosts, const std::vector<NLNET::TServiceId8>& spawnerIdsToIgnore )
{
	uint nbChanges = 0;
	for ( TDataSetIndex i=first; i!=end; ++i )
	{
		const CEntityId& entityId = _PropertyContainer.EntityIdArray.EntityIds[i];

		// Add any existing entity that we would not know yet
		if ( _PropertyContainer.EntityIdArray.isOnline( i ) &&
			 (_EntityIdToEntityIndex.find( entityId ) == _EntityIdToEntityIndex.end()) )
		{
			if ( find( spawnerIdsToIgnore.begin(), spawnerIdsToIgnore.end(), getSpawnerServiceId( i ) ) == spawnerIdsToIgnore.end() )
			{
				//nldebug( "ROWMGT: Adding E%d into EntityTrackerAdding (smid %d) for %s", i, entityTrackerAdding.smid(), servStr(serviceId).c_str() );
				_SelfEntityTrackers[ADDING].recordChange( i );
				++nbChanges;
			}
		}
	}

	if ( deleteGhosts )
	{
		// Remove any entity that would be incorrectly be in the map (debug feature)
		vector<TEntityIdToEntityIndexMap::iterator> removelist;
		TEntityIdToEntityIndexMap::iterator iiti;
		for ( iiti=_EntityIdToEntityIndex.begin(); iiti!=_EntityIdToEntityIndex.end(); ++iiti )
		{
			if ( ! _PropertyContainer.EntityIdArray.isOnline( GET_ENTITY_INDEX(iiti) ) )
			{
				if ( find( spawnerIdsToIgnore.begin(), spawnerIdsToIgnore.end(), getSpawnerServiceId( GET_ENTITY_INDEX(iiti) ) ) == spawnerIdsToIgnore.end() )
				{
					removelist.push_back( iiti );
				}
			}
		}
		vector<TEntityIdToEntityIndexMap::iterator>::iterator irl;
		for ( irl=removelist.begin(); irl!=removelist.end(); ++irl )
		{
			_EntityIdToEntityIndex.erase( *irl );
		}
		if ( ! removelist.empty() )
			nlwarning( "Removed %u entities", removelist.size() );
	}

	return nbChanges;
}


/*
 * Scan the rows for 'already set' values for the property, set by the specified service.
 */
uint16	CMirroredDataSet::scanAndResyncProp( TPropertyIndex propIndex, CChangeTrackerClientProp *tracker, NLNET::TServiceId serviceId )
{
	uint nbChanges = 0;
	for ( TDataSetIndex i=0; i!=(uint32)maxNbRows(); ++i )
	{
		// Don't notify properties of offline entities, they will be notified in declareEntity()
		// Property timestamps are reset when an entity is deleted => non-zero timestamps indicate properties for current entity in row
		if ( _PropertyContainer.EntityIdArray.isOnline( i ) &&
			 (_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[i] != 0) )
		{
			// Notify only the values modified by the specified service (will not work if another service overwrites it!)
			if ( _PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[i].get() == serviceId.get() )
			{
				tracker->recordChange( i );
				++nbChanges;
				//nldebug( "PROMPMGT: smid %d: marked E%d as changed", newtracker->smid(), i );
			}
		}
	}
	return nbChanges;
}


/*
 * Add an entity (of owned entity type)
 */
uint	CMirroredDataSet::addEntityToDataSet( bool fillEntityId, NLMISC::CEntityId& entityId, bool declare )
{
	TEntityRangeOfType::iterator ir = _EntityTypesRanges.find( entityId.getType() );
	if ( ir != _EntityTypesRanges.end() )
	{
//#ifdef NL_DEBUG
		if ( !fillEntityId && _EntityIdToEntityIndex.find( entityId ) != _EntityIdToEntityIndex.end() )
		{
			nlwarning( "MIRROR/%s: Cannot add %s (already mapped to E%d)!", name().c_str(), entityId.toString().c_str(), _EntityIdToEntityIndex[entityId] ); // should not occur
			return 0;
		}
//#endif
		// Get new entity index in the range corresponding to the entity type
		bool previouslyUsed;
		TDataSetIndex entityIndex = GET_ENTITY_TYPE_RANGE(ir).getNewEntityIndex( &previouslyUsed );
		if ( entityIndex == INVALID_DATASET_ROW )
		{
			nlwarning( "MIRROR/%s: Cannot add %s (no more space in the range [%u..%u])", name().c_str(), entityId.toString().c_str(), GET_ENTITY_TYPE_RANGE(ir).first(), GET_ENTITY_TYPE_RANGE(ir).last() );
			return 0;
		}
		else
		{
			//++NbAddedPerTick;

			// Increment (modulo 256) the binding count (skip 0)
			uint8& bindingCounter = _PropertyContainer.EntityIdArray.Counts[entityIndex];
			++bindingCounter;
			if ( bindingCounter == 0 )
				++bindingCounter;

			TDataSetRow dsr( entityIndex, bindingCounter );

			if ( fillEntityId )
			{
				// We need to copy the entityIndex into the id part of the eid
				entityId.setShortId( dsr._EntityIndexAndRemCounter );
			}

			// Map CEntityId -> TDataSetRow
			pair<TEntityIdToEntityIndexMap::iterator, bool>	ret;
			ret = _EntityIdToEntityIndex.insert( make_pair( entityId, entityIndex ) );
			if (!ret.second)
			{
				nlwarning( "MIRROR/%s: Cannot add %s (already mapped to E%d)!", name().c_str(), entityId.toString().c_str(), _EntityIdToEntityIndex[entityId] ); // should not occur
				// revert change
				--bindingCounter;
				if (bindingCounter == 0)
					--bindingCounter;
				GET_ENTITY_TYPE_RANGE(ir).releaseEntityIndex(entityIndex);

				return 0;
			}


			// Init/reset entry in shared memory
#ifdef DISPLAY_ROW_CHANGES
			MIRROR_DEBUG( "MIRROR: %u: Creating %s -> %u_%hu", CTickEventHandler::getGameCycle(), entityId.toString().c_str(), entityIndex, (uint16)bindingCounter );
#endif
			_PropertyContainer.EntityIdArray.EntityIds[entityIndex] = entityId;

			// Update online timestamp (necessary for declareEntity() "unpublicized entities" feature)
			_PropertyContainer.EntityIdArray.updateOnlineTimestamp( entityIndex );

			// Set spawner service id
			_PropertyContainer.EntityIdArray.setSpawnerServiceId( entityIndex, IService::getInstance()->getServiceId() );

			// Clear all the properties values of the row (otherwise, a change could fail to setChange()
			// if the value was the same, with MIRROR_ASSIGN_ONLY_IF_CHANGED defined, and even without
			// some services may test if a value is non-zero) .
			// This is done here because if it was done in removeEntityFromDatasSet(), any service would be
			// able to read property values set to zero between removeEntityFromDatasSet() and their
			// notification of the removal.
			// When the addition will be notified, all property timestamps & values will be zero except
			// those that will have been set between addEntityToDataSet() and the notification.
			uint propIndex;
			for ( propIndex=0; propIndex!=_PropertyContainer.PropertyValueArrays.size(); ++propIndex )
			{
				if (_PropertyContainer.PropertyValueArrays[propIndex].Values == NULL)
					continue;

				// Empty list container if it's a list property
				if ( _PropIsList[ propIndex ] )
				{
					killList( forceCurrentDataSetRow( entityIndex ), propIndex );
					TSharedListRow *listHeaders = (TSharedListRow*)(_PropertyContainer.PropertyValueArrays[propIndex].Values);
					listHeaders[entityIndex] = INVALID_SHAREDLIST_ROW; // TODO: remove (already done by killList())
				}
				else
				{
					// Reset values
					uint32 datasize = getDataSizeOfProp( (TPropertyIndex)propIndex );
					uint8 *ptValue = ((uint8*)(_PropertyContainer.PropertyValueArrays[propIndex].Values)) + (uint)(entityIndex*datasize);
					memset( ptValue, 0, datasize );
				}

				// Reset timestamps
				_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex] = 0;

#ifdef STORE_CHANGE_SERVICEIDS
				// Reset serviceids
				_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex] = TServiceId();
#endif
			}

			// Increment counters
			++_NumberOfCurrentCreatedEntities;
			++_NumberOfUndeclaredEntities;

			// Declare if requested
			if ( declare )
			{
				declareEntity( dsr );
			}

			return 1;
		}
	}
	else
	{
		// If the following warning is displayed, it means that:
		// - either the service doesn't have the right to add an entity of the specified type,
		//   i.e. a call to declareEntityTypeOwner() is missing in the mirror init in this service
		// - or createEntity() is called too early, before the range manager has sent back the requested
		//   range, i.e. the function that calls createEntity() is misplaced. It should not be called
		//   before mirror ready at level 2 (level 1 is only ready for mirror init).
		nlwarning( "MIRROR: No owned range found to add entity!" );
		return 0;
	}
}


/*
 * Clear a list, but do not notify changes (used when spawning an entity with a blank list, clear() is necesary for row reuse)
 */
#define KILL_LIST_TYPE(type_name, type)	\
	case type_name: \
	{ \
		CMirrorPropValueList<type> list( *this, entityIndex, propIndex ); \
		list.clear( true ); \
		break; \
	}


/*
 * Empty a list without knowing the type of elements
 */
void	CMirroredDataSet::killList( TDataSetRow entityIndex, TPropertyIndex propIndex )
{
	TTypeOfProp propType = getPropType( propIndex );
	switch( propType )
	{
		KILL_LIST_TYPE(TypeUint8, uint8)
		KILL_LIST_TYPE(TypeSint8, sint8)
		KILL_LIST_TYPE(TypeUint16, uint16)
		KILL_LIST_TYPE(TypeSint16, sint16)
		KILL_LIST_TYPE(TypeUint32, uint32)
		KILL_LIST_TYPE(TypeSint32, sint32)
		KILL_LIST_TYPE(TypeUint64, uint64)
		KILL_LIST_TYPE(TypeSint64, sint64)
		KILL_LIST_TYPE(TypeFloat, float)
		KILL_LIST_TYPE(TypeDouble, double)
		KILL_LIST_TYPE(TypeCEntityId, CEntityId)
		KILL_LIST_TYPE(TypeBool, bool)
		default:
			break;
	}
/*
	TSharedListRow *ptFront;
	TSharedListCell *container;
	dataSet.getPropPointerForList( &ptFront, propIndex, entityIndex );
	container = (TSharedListCell*)(dataSet.getListCellContainer( propIndex ));

	TSharedListRow row = *ptFront;
	while ( row != INVALID_SHAREDLIST_ROW )
	{
		TSharedListRow oldrow = row;
		row = container[oldrow].Next;
		//_Container[oldrow].Value.~T(); // here we don't reset the values (we don't know their type)
		container[oldrow].Next = UNUSED_SHAREDLIST_CELL;
	}
	*ptFront = INVALID_SHAREDLIST_ROW;
*/
}


/*
 * Declare an entity previously added by CMirror::createEntity() in the dataset
 * (if the entity type is matching the dataset) to other services.
 * To obtain the entity index from the entityId, use getDataSetRow().
 * \seealso CMirror::declareEntity() for alternate version.
 */
void	CMirroredDataSet::declareEntity( const TDataSetRow& entityIndex )
{
	//nlassert( MirrorInstance->isRunningSynchronizedCode() ); // if this fails, the code declaring the entity is not at the good place

	TDataSetIndex index = entityIndex.getIndex();
//#ifdef NL_DEBUG
	nlassert( (uint)index < (uint)maxNbRows() ); // check if entityIndex valid
	//nlassert( ! getEntityId( entityIndex ).isUnknownId() ); // check if created before (TODO: better check)
	nlassert( _NumberOfUndeclaredEntities != 0 );
	nlassert( ! isOnline( entityIndex ) ); // check if not declared twice
//#endif

	// Get entity type for tracker filters
	//uint8 entityType = _PropertyContainer.EntityIdArray.EntityIds[index].getType();

	TGameCycle creationTimestamp = getOnlineTimestamp( entityIndex );

	// Set online
	//	nlassert( _PropertyContainer.EntityIdArray.OnlineTimestamps[entityIndex.getIndex()] < CTickEventHandler::getGameCycle() );
	_PropertyContainer.EntityIdArray.setOnline( index, true );
	_PropertyContainer.EntityIdArray.checkSpawnerId( index, IService::getInstance()->getServiceId() );
	--_NumberOfUndeclaredEntities;

	// Notify initial properties (unpublicized entities feature)
	for ( uint propIndex=0; propIndex!=_PropertyContainer.PropertyValueArrays.size(); ++propIndex )
	{
		TGameCycle *propTimestamps = _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps;
		if ( propTimestamps )
		{
			// Initial properties are properties set since the creation
			if ( propTimestamps[index] >= creationTimestamp )
			{
				setChanged( entityIndex, propIndex ); // will notify, as the entity is now online
			}
		}
	}

	// Push change to other services' add lists (the ones that want to know the changes)
//#ifdef DISPLAY_ROW_CHANGES
	MIRROR_DEBUG( "MIRROR: %u: Declaring %u_ into %u trackers", CTickEventHandler::getGameCycle(), entityIndex.getIndex(), _EntityTrackers[ADDING].size() );
//#endif
	for ( TTrackerListForEnt::iterator it=_EntityTrackers[ADDING].begin(); it!=_EntityTrackers[ADDING].end(); ++it )
	{
		//if ( _EntityTrackerFilters[i].isAccepted() )
			(*it).recordChange( index );
	}
}


/*
 * Remove an entity (when the local service is an owner of the entity type/dataset)
 */
uint	CMirroredDataSet::removeEntityFromDataSet( const NLMISC::CEntityId& entityId )
{
	//nlassert( MirrorInstance->isRunningSynchronizedCode() ); // if this fails, the code removing the entity is not at the good place

	TEntityRangeOfType::iterator ir = _EntityTypesRanges.find( entityId.getType() );
	if ( ir != _EntityTypesRanges.end() )
	{
		TDataSetIndex entityIndex = getDataSetRow( entityId ).getIndex();
		if ( (entityIndex < GET_ENTITY_TYPE_RANGE(ir).first()) ||
			 (entityIndex > GET_ENTITY_TYPE_RANGE(ir).last()) )
		{
			nlwarning( "MIRROR: The row E%d you try to remove in dataset %s does not belong to you", entityIndex, name().c_str() );
			return 0;
		}
		GET_ENTITY_TYPE_RANGE(ir).releaseEntityIndex( entityIndex );

		// Increment (modulo 256) the binding count (skip 0)
		uint8& bindingCounter = _PropertyContainer.EntityIdArray.Counts[entityIndex];
		++bindingCounter;
		if ( bindingCounter == 0 )
			++bindingCounter;

//#ifdef DISPLAY_ROW_CHANGES
		MIRROR_DEBUG( "MIRROR: %u: Deleting %s -> %d_%hu into %u trackers", CTickEventHandler::getGameCycle(), entityId.toString().c_str(), entityIndex, bindingCounter, _EntityTrackers[REMOVING].size() );
//#endif

#ifdef NL_OS_WINDOWS
		// Assert to check if despawn not too early
//		nlassert( _PropertyContainer.EntityIdArray.OnlineTimestamps[entityIndex] < CTickEventHandler::getGameCycle() );
#endif
		// Unmap, leave the entityId entry in shard memory for notifications, but set entityId entry offline (for filling existing entities at a service startup)
		_EntityIdToEntityIndex.erase( entityId );
		bool onlineStatus = _PropertyContainer.EntityIdArray.isOnline( entityIndex );
		if ( onlineStatus )
		{
			_PropertyContainer.EntityIdArray.setOnline( entityIndex, false );
		}
		else
		{
			--_NumberOfUndeclaredEntities;
		}
		--_NumberOfCurrentCreatedEntities;

		_PropertyContainer.EntityIdArray.checkSpawnerId( entityIndex, IService::getInstance()->getServiceId() );

		//++NbRemovedPerTick;

		// Clear the timestamp of all properties for the entity
		// to prevent a connecting service to receive values corresponding to no longer entity
		for ( uint propIndex=0; propIndex!=_PropertyContainer.PropertyValueArrays.size(); ++propIndex )
		{
			if ( _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps )
				_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex] = 0;
		}

		// Get entity type for tracker filters
		//uint8 entityType = _PropertyContainer.EntityIdArray.EntityIds[entityIndex].getType();

		// Push change to other services' remove lists (the ones that want to know the changes)
		// Only if the entity was online (test necessary for unpublicized entities feature)
		if ( onlineStatus )
		{
			TTrackerListForEnt::iterator it;
			for ( it=_EntityTrackers[REMOVING].begin(); it!=_EntityTrackers[REMOVING].end(); ++it )
			{
				//if ( _EntityTrackerFilters[i].isAccepted() )
					(*it).recordChange( entityIndex );
			}
		}

		return 1;
	}
	else
	{
		nlwarning( "MIRROR: CMirroredDataSet::removeEntityFromDataSet() error" );
		return 0;
	}
}


//extern uint32 NbReleasedIndexes;

/*
 * Acquire a new row. Set *previouslyUsed to true if the row was used and released before
 */
inline TDataSetIndex			TEntityRange::getNewEntityIndex( bool *previouslyUsed )
{
	// Prevent immediate reallocation of the same row: wait for an arbitrary time
	if ( _ReleasedIndexes.empty() || (CTickEventHandler::getGameCycle()-_ReleasedIndexes.front().RemovalDate<100) ) // (10 s)
	{
		*previouslyUsed = false;
		if ( _NextFree < _OuterBound )
		{
			++_NextFree;
			return _NextFree-1;
		}
		else
		{
			// No more space in the range
			return INVALID_DATASET_INDEX;
		}
	}
	else
	{
		*previouslyUsed = true;
		TDataSetIndex freeindex = _ReleasedIndexes.front().EntityIndex;
		_ReleasedIndexes.pop();
		//NbReleasedIndexes = _ReleasedIndexes.size();
		return freeindex;
	}
}


/*
 * Release a row previously acquired
 */
inline void				TEntityRange::releaseEntityIndex( TDataSetIndex entityIndex )
{
	_ReleasedIndexes.push( TRowAllocation(CTickEventHandler::getGameCycle(),entityIndex) );
	//NbReleasedIndexes = _ReleasedIndexes.size();
}


/*
 * Get the property index corresponding to the specified property name in the dataset
 */
TPropertyIndex CMirroredDataSet::getPropertyIndex( const std::string& propName ) const
{
	TPropertiesInMirror::const_iterator ipim = _PropAllocator->_PropertiesInMirror.find( propName );
	if ( ipim != _PropAllocator->_PropertiesInMirror.end() )
	{
		const TPropertyInfo& propinfo = GET_PROPERTY_INFO(ipim);
		if ( propinfo.DataSet != this )
		{
#ifdef NL_DEBUG
		nlwarning( "MIRROR: Property %s does not belong to the current dataset %s", propName.c_str(), name().c_str() );
#endif
			return INVALID_PROPERTY_INDEX;
		}
		return propinfo.PropertyIndex;
	}
	else
	{
#ifdef NL_DEBUG
		nlwarning( "MIRROR: Cannot find property index for %s", propName.c_str() );
#endif
		return INVALID_PROPERTY_INDEX;
	}
}


/*
 * Tell a property has changed
 */
void	CMirroredDataSet::setChanged( const TDataSetRow& datasetRow, TPropertyIndex propIndex )
{
	TDataSetIndex entityIndex = datasetRow.getIndex();

	// Do not notify other services if the entity is offline.
	// It allows to support unpublicized entities (safer than using temp storage features).
	if ( _PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
	{
		// Record change in all trackers of the specified property
		TTrackerListForProp::iterator it;
		for ( it=_PropTrackers[propIndex].begin(); it!=_PropTrackers[propIndex].end(); ++it )
		{
			(*it).recordChange( entityIndex );
		}
	}

	// Timestamp it!
#ifdef NL_DEBUG
#ifdef STORE_CHANGE_SERVICEIDS
	if ( _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex] == CTickEventHandler::getGameCycle() )
	{
		if ( _PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex] != IService::getInstance()->getServiceId() )
		{
			nlwarning( "MIRROR: You (%s) are changing a value (E%u, prop %hd) that is already being changed in the same cycle %u by %s!",
				IService::getInstance()->getServiceUnifiedName().c_str(),
				(uint32)entityIndex,
				propIndex,
				CTickEventHandler::getGameCycle(),
				CUnifiedNetwork::getInstance()->getServiceUnifiedName( _PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex] ).c_str() );
		}
		else if ( VerboseMultipleChangesOfAProperty )
		{
			nlwarning( "MIRROR: You (%s) are changing a value (E%u, prop %hd) several times in the same cycle %u", IService::getInstance()->getServiceUnifiedName().c_str(), (uint32)entityIndex, propIndex, CTickEventHandler::getGameCycle() );
		}
	}
#else
	if ( VerboseMultipleChangesOfAProperty && _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex] == CTickEventHandler::getGameCycle() )
	{
		nlwarning( "MIRROR: You are changing a value (E%d, prop %hd) that is already being changed in the same cycle %u!", entityIndex, propIndex, CTickEventHandler::getGameCycle() );
	}
#endif
	if ( CTickEventHandler::getGameCycle() == 0 )
		nlwarning( "MIRROR: Setting a value with a null timestamp (check the tick)" );
#endif
	_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex] = CTickEventHandler::getGameCycle();
#ifdef STORE_CHANGE_SERVICEIDS
	_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex] = IService::getInstance()->getServiceId();
	nlassertex ( _PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex].get() > 127, ("%hu",_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex].get()) );
#endif
}


/*
 * Continue browsing the added entities, until the method returns INVALID_DATASET_ROW.
 */
TDataSetRow		CMirroredDataSet::getNextAddedEntity()
{
	TDataSetRow entityIndex( _SelfEntityTrackers[ADDING].getFirstChanged() );
	if ( entityIndex.getIndex() != LAST_CHANGED )
	{
#ifdef CHECK_DATASETROW_VALIDITY
		entityIndex.setCounter( _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
#endif
		//if ( NbAddedPerTick < ChangeQuotaPerTick )
		//{
		//nlassert( _SelfEntityTrackers[ADDING].isAllocated() );

		bool online;
		do
		{
			_SelfEntityTrackers[ADDING].popFirstChanged();

			// Prepare to map
			nlassert( entityIndex.isValid() );
			const CEntityId& entityId = getEntityId( entityIndex );

			if ( (online = isOnline( entityIndex )) )
			{
				// Map row and entity
#ifdef DISPLAY_ROW_CHANGES
				MIRROR_DEBUG( "MIRROR: %u: Adding %s -> %u_%hu", CTickEventHandler::getGameCycle(), entityId.toString().c_str(), (uint32)entityIndex.getIndex(), (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
#endif
				//TmpDebugLogger->displayNL( "MIRROR: %u: Adding %s -> %u_%hu", CTickEventHandler::getGameCycle(), entityId.toString().c_str(), (uint32)entityIndex.getIndex(), (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
				if ( ! _EntityIdToEntityIndex.insert( make_pair( entityId, entityIndex.getIndex() ) ).second )
				{
					nlwarning( "Entity %s already present at E%u (E%u)", entityId.toString().c_str(), _EntityIdToEntityIndex[entityId], entityIndex.getIndex() );
					return entityIndex; // not good
				}
				//++NbAddedPerTick;
			}
			else
			{
				// Skip entities not notified yet (for adding) but already removed
				MIRROR_DEBUG( "MIRROR: %u: Not adding %s -> %u_%hu, lifetime too short", CTickEventHandler::getGameCycle(), entityId.toString().c_str(), (uint32)entityIndex.getIndex(), (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
				//TmpDebugLogger->displayNL( "MIRROR: %u: Not adding %s -> %u_%hu, lifetime too short", CTickEventHandler::getGameCycle(), entityId.toString().c_str(), (uint32)entityIndex.getIndex(), (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
				entityIndex.initFromIndex( _SelfEntityTrackers[ADDING].getFirstChanged() );
				if ( entityIndex.getIndex() == LAST_CHANGED )
					return TDataSetRow(LAST_CHANGED);
#ifdef CHECK_DATASETROW_VALIDITY
				entityIndex.setCounter( _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
#endif
			}
		}
		while ( ! online );

		//}
		//else
		//{
		//	return TDataSetRow(LAST_CHANGED); // the current entityIndex (getFirstChanged()) will be notified at next tick
		//}
	}
	return entityIndex;
}


/*
 *
 */
//void				CMirroredDataSet::endAddedEntities()
//{
	/*if ( NbAddedPerTick != 0 )
	{
		NbOnlineEntities += NbAddedPerTick;
		LOG_TRACE( toString("%u added => %d entities", NbAddedPerTick, NbOnlineEntities).c_str() );
		NbAddedPerTick = 0;
	}*/
//}


/*
 * If you need not browsing the added entities, you must call this method *instead* of
 * beginAddedEntities() + {getNextAddedEntity()} + endAddedEntities()
 */
void				CMirroredDataSet::processAddedEntities()
{
	beginAddedEntities();
	uint32 entityIndex;
	while ( (entityIndex = getNextAddedEntity().getIndex()) != LAST_CHANGED )
	{
		// TODO: could implement a callback system here
	}
	endAddedEntities();
}


/* Continue browsing the removed entities, until the method returns LAST_CHANGED.
 * You must not use the entity index anymore. That is why the method also outputs the
 * entity id of the removed entity (note: it makes the pointer you provide pointing to
 * the entity id. The pointer will not remain valid after calling endRemovedEntities()).
 */
TDataSetRow			CMirroredDataSet::getNextRemovedEntity( NLMISC::CEntityId **entityId )
{
	TDataSetRow entityIndex( _SelfEntityTrackers[REMOVING].getFirstChanged() );
	if ( entityIndex.getIndex() != LAST_CHANGED )
	{
#ifdef CHECK_DATASETROW_VALIDITY
		// Set the counter with the previous value (because incremented by the removal)
		// Note: this would fail if the entityindex could be reassigned before the removal notification
		uint8 newcounter = _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] - 1;
		if ( newcounter == 0 )
			newcounter = 255;
		entityIndex.setCounter( newcounter );
#endif
		//if ( NbRemovedPerTick < ChangeQuotaPerTick )
		//{
		//nlassert( _SelfEntityTrackers[REMOVING].isAllocated() );

		uint nbremoved;
		do
		{
			_SelfEntityTrackers[REMOVING].popFirstChanged();

			// Try to unmap row and entity
			nlassert( entityIndex.isValid() );
			const CEntityId& leavingEntityId = getEntityId( entityIndex );
			nbremoved = (uint)_EntityIdToEntityIndex.erase( leavingEntityId );
			if ( nbremoved != 0 )
			{
				// Return the entity id
#ifdef DISPLAY_ROW_CHANGES
				MIRROR_DEBUG( "MIRROR: %u: Removing %s -> %u_%hu", CTickEventHandler::getGameCycle(), leavingEntityId.toString().c_str(), (uint32)entityIndex.getIndex(), (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
#endif
				//TmpDebugLogger->displayNL( "MIRROR: %u: Removing %s -> %u_%hu", CTickEventHandler::getGameCycle(), leavingEntityId.toString().c_str(), (uint32)entityIndex.getIndex(), (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
				*entityId = const_cast<CEntityId*>(&leavingEntityId);
				//++NbRemovedPerTick;
			}
			else
			{
				// Skip entities not notified yet (for adding) but already removed
				MIRROR_DEBUG( "MIRROR: %u: Not removing %s -> %u_%hu, lifetime too short", CTickEventHandler::getGameCycle(), leavingEntityId.toString().c_str(), (uint32)entityIndex.getIndex(), (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
				//TmpDebugLogger->displayNL( "MIRROR: %u: Not removing %s -> %u_%hu, lifetime too short", CTickEventHandler::getGameCycle(), leavingEntityId.toString().c_str(), (uint32)entityIndex.getIndex(), (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
				entityIndex.initFromIndex( _SelfEntityTrackers[REMOVING].getFirstChanged() );
				if ( entityIndex == LAST_CHANGED )
					return TDataSetRow(LAST_CHANGED);
#ifdef CHECK_DATASETROW_VALIDITY
				// Set the counter with the previous value (because incremented by the removal)
				// Note: this would fail if the entityindex could be reassigned before the removal notification
				uint8 newcounter = _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] - 1;
				if ( newcounter == 0 )
					newcounter = 255;
				entityIndex.setCounter( newcounter );
#endif
			}
		}
		while ( nbremoved == 0 );

		//}
		//else
		//{
		//	return TDataSetRow(LAST_CHANGED); // the current entityIndex (getFirstChanged()) will be notified at next tick
		//}
	}
	return entityIndex;
}


/*
 *
 */
//void				CMirroredDataSet::endRemovedEntities()
//{
	/*if ( NbRemovedPerTick != 0 )
	{
		NbOnlineEntities -= NbRemovedPerTick;
		LOG_TRACE( toString("%u removed => %d entities", NbRemovedPerTick, NbOnlineEntities).c_str() );
		NbRemovedPerTick = 0;
	}*/
//}


/*
 * If you need not browsing the removed entities, you must call this method *instead* of
 * beginRemovedEntities() + {getNextRemovedEntity()} + endRemovedEntities()
 */
void				CMirroredDataSet::processRemovedEntities()
{
	beginRemovedEntities();
	CEntityId *entityId;
	uint32 entityIndex;
	while ( (entityIndex = getNextRemovedEntity( &entityId ).getIndex()) != LAST_CHANGED )
	{
		// TODO: could implement a callback system here
	}
	endRemovedEntities();
}


/*
 * Browse the values of properties that have changed in the dataset since the previous
 * cycle. Only the properties for which you have subscribed with notifyValueChanges set
 * to true will be reported. You may use a different notification scheme instead,
 * specifying for which property you query the changes (see ...forProp() methods below).
 * Don't use both schemes in the same service (they are not compatible).
 */
void				CMirroredDataSet::beginChangedValues()
{
	// Notify: for each property subscribed with notification, report changes
	if ( _SelfPropTrackers.empty() )
	{
		_CurrentChangedPropertyTracker = INVALID_TRACKER;
	}
	else
	{
		_CurrentChangedPropertyTracker = 0;
	}
}


/*
 * Continue browsing the changed values, until the method returns LAST_CHANGED in entityIndex
 */
void				CMirroredDataSet::getNextChangedValue( TDataSetRow& entityIndex, TPropertyIndex& propIndex )
{
	if ( _CurrentChangedPropertyTracker == INVALID_TRACKER )
	{
		entityIndex.initFromIndex( LAST_CHANGED );
		return;
	}

	// Notify
	do
	{
		// A property tracker, when in the _SelfPropTrackers array, is always allocated
		//nlassert( _SelfPropTrackers[_CurrentChangedPropertyTracker].isAllocated() );
		entityIndex.initFromIndex(_SelfPropTrackers[_CurrentChangedPropertyTracker].getFirstChanged());
		if ( entityIndex != LAST_CHANGED )
		{
#ifdef CHECK_DATASETROW_VALIDITY
			entityIndex.setCounter( _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
#endif
			_SelfPropTrackers[_CurrentChangedPropertyTracker].popFirstChanged();
			propIndex = _SelfPropTrackers[_CurrentChangedPropertyTracker].propIndex();
			return;
		}
		else
		{
			// End of tracker detected
			++_CurrentChangedPropertyTracker;
			if ( _CurrentChangedPropertyTracker >= _SelfPropTrackers.size() )
			{
				// No more trackers
				return;
			}
		}
	}
	while ( true );
}


/*
 * End browsing the changed values. Be sure to call this method every cycle.
 */
void				CMirroredDataSet::endChangedValues()
{
}


/*
 * Browse the values for a particular property (for which you have subscribed with
 * notifyValueChanges set to true). Use *either* beginChangedValues()/getNextChangedValue()/
 * endChangedValues() or beginChangedValueForProp()/getNextChangedValueForProp()/
 * endChangedValuesForProp(), not both notifications schemes in the same service
 * (they are not compatible).
 */
void				CMirroredDataSet::beginChangedValuesForProp( TPropertyIndex propIndex )
{
	// Find the tracker corresponding to the property
	uint32 i;
	for ( i=0; i!=_SelfPropTrackers.size(); ++i )
	{
		if ( _SelfPropTrackers[i].propIndex() == propIndex )
			break;
	}
	if ( i == _SelfPropTrackers.size() )
	{
		_CurrentChangedPropertyTracker = INVALID_TRACKER;
#ifdef NL_DEBUG
		nlwarning( "MIRROR: The specified property %hd is not subscribed with notification", propIndex );
#endif
	}
	else
	{
		_CurrentChangedPropertyTracker = i;
	}
}


/*
 * Continue browsing the changed values for the property passed to beginChangedValuesForProp(),
 * until the method returns LAST_CHANGED in entityIndex.
 */
TDataSetRow				CMirroredDataSet::getNextChangedValueForProp()
{
	if ( _CurrentChangedPropertyTracker == INVALID_TRACKER )
	{
		return TDataSetRow(LAST_CHANGED);
	}
	else
	{
		// Notify
		// A property tracker, when in the _SelfPropTrackers array, is always allocated
		//nlassert( _SelfPropTrackers[_CurrentChangedPropertyTracker].isAllocated() );
		TDataSetRow entityIndex(_SelfPropTrackers[_CurrentChangedPropertyTracker].getFirstChanged());
		if ( entityIndex != LAST_CHANGED )
		{
#ifdef CHECK_DATASETROW_VALIDITY
			entityIndex.setCounter( _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()] );
#endif
			_SelfPropTrackers[_CurrentChangedPropertyTracker].popFirstChanged();
		}
		return entityIndex;
	}
}


/*
 * End browsing the changed values for the property passed to beginChangedValuesForProp().
 * Be sure to call this method every cycle.
 */
void				CMirroredDataSet::endChangedValuesForProp()
{
}


/*
 *
 */
void				CMirroredDataSet::serialOutMirrorInfo( NLNET::CMessage& msgout )
{
	// Entity types ranges
	uint16 nbRanges = (uint16)_EntityTypesRanges.size();
	msgout.serial( nbRanges );
	TEntityRangeOfType::iterator itr;
	for ( itr=_EntityTypesRanges.begin(); itr!=_EntityTypesRanges.end(); ++itr )
	{
		msgout.serial( const_cast<uint8&>((*itr).first) );
		msgout.serial( const_cast<TEntityRange&>((*itr).second) );
	}

	// Self prop trackers
	msgout.serialCont( _SelfPropTrackers );

	// Self entity trackers
	msgout.serial( _SelfEntityTrackers[ADDING] );
	msgout.serial( _SelfEntityTrackers[REMOVING] );

	MIRROR_INFO( "MIRROR: Dataset %s: Resyncing %hu ranges, %u self prop trackers, and 2 self entity trackers to MS", name().c_str(), nbRanges, _SelfPropTrackers.size() );
}


/*
 *
 */
inline bool			CMirroredDataSet::entityMatchesFilter ( const NLMISC::CEntityId& entityId, TDataSetIndex entityIndex, uint8 onlyEntityType, uint8 onlyCreatorId, uint8 onlyDynamicId, bool hideUndeclared ) const
{
	return (((onlyEntityType == 0xFF) || (entityId.getType() == onlyEntityType)) &&
 			((onlyCreatorId == 0xFF)  || (entityId.getCreatorId() == onlyCreatorId)) &&
			((onlyDynamicId == 0xFF) || (entityId.getDynamicId() == onlyDynamicId)) &&
			((!hideUndeclared) || _PropertyContainer.EntityIdArray.isOnline( entityIndex )));
}


/*
 * Display all or part of the entities in the dataset (0xFF=no filter). Return the number of entities matching the filters.
 */
uint				CMirroredDataSet::displayEntities( NLMISC::CLog& log, uint8 onlyEntityType, uint8 onlyCreatorId, uint8 onlyDynamicId, bool hideUndeclared, bool displayLocalCreationTimestamp, bool sortByEntityId, bool sortByDatasetRow ) const
{
	if ( !hideUndeclared )
		log.displayNL( "* %u entities in dataset %s *", _EntityIdToEntityIndex.size(), name().c_str() ); // ould prevent to compare the list if hideUndeclared
	uint nbMatchingFilter = 0;
	map<TDataSetIndex,CEntityId> resultSortedByDatasetRow;
	map<CEntityId,TDataSetIndex> resultSortedByEntityId;
	for ( TEntityIdToEntityIndexMap::const_iterator iei=_EntityIdToEntityIndex.begin(); iei!=_EntityIdToEntityIndex.end(); ++iei )
	{
		if ( entityMatchesFilter( (*iei).first, (*iei).second, onlyEntityType, onlyCreatorId, onlyDynamicId, hideUndeclared ) )
		{
			if ( sortByEntityId )
				resultSortedByEntityId.insert( make_pair( (*iei).first, (*iei).second ) );
			else if ( sortByDatasetRow )
				resultSortedByDatasetRow.insert( make_pair( (*iei).second, (*iei).first ) );
			else
				log.displayRawNL( "%s: E%u_%hu %s%s", (*iei).first.toString().c_str(), (*iei).second, (uint16)getBindingCounter((*iei).second), isOnline(TDataSetRow((*iei).second))?"online":"offline", displayLocalCreationTimestamp ? toString( " since tick %u", getOnlineTimestamp(TDataSetRow((*iei).second)) ).c_str() : "" );
			++nbMatchingFilter;
		}
	}
	if ( sortByEntityId )
	{
		for ( map<CEntityId,TDataSetIndex>::const_iterator it=resultSortedByEntityId.begin(); it!=resultSortedByEntityId.end(); ++it )
			log.displayRawNL( "%s: E%u_%hu %s%s", (*it).first.toString().c_str(), (*it).second, (uint16)getBindingCounter((*it).second), isOnline(TDataSetRow((*it).second))?"online":"offline", displayLocalCreationTimestamp ? toString( " since tick %u", getOnlineTimestamp(TDataSetRow((*it).second)) ).c_str() : "" );
	}
	else if ( sortByDatasetRow )
	{
		for ( map<TDataSetIndex,CEntityId>::const_iterator it=resultSortedByDatasetRow.begin(); it!=resultSortedByDatasetRow.end(); ++it )
			log.displayRawNL( "E%u_%hu: %s %s%s", (*it).first, (uint16)getBindingCounter((*it).first), (*it).second.toString().c_str(), isOnline(TDataSetRow((*it).first))?"online":"offline", displayLocalCreationTimestamp ? toString( " since tick %u", getOnlineTimestamp(TDataSetRow((*it).first)) ).c_str() : "" );
	}
	log.displayRawNL( "  %u entities matching filter", nbMatchingFilter );
	return nbMatchingFilter;
}


/*
 * Look for a property value (see displayEntities() for params)
 */
uint				CMirroredDataSet::lookForValue( NLMISC::CLog& log, const std::string& propName, bool anyValue, const std::string& searchedValueStr, uint8 onlyEntityType, uint8 onlyCreatorId, uint8 onlyDynamicId, bool hideUndeclared, bool /* displayLocalCreationTimestamp */, bool sortByEntityId, bool /* sortByDatasetRow */ ) const
{
	TPropertyIndex propIndex = getPropertyIndex( propName );
	if ( propIndex == INVALID_PROPERTY_INDEX )
	{
		log.displayNL( "Property %s not found", propName.c_str() );
		return 0;
	}
	uint nbMatchingFilter = 0;
	map<TDataSetIndex,CEntityId> resultSortedByDatasetRow;
	map<CEntityId,TDataSetIndex> resultSortedByEntityId;
	for ( TEntityIdToEntityIndexMap::const_iterator iei=_EntityIdToEntityIndex.begin(); iei!=_EntityIdToEntityIndex.end(); ++iei )
	{
		if ( entityMatchesFilter( (*iei).first, (*iei).second, onlyEntityType, onlyCreatorId, onlyDynamicId, hideUndeclared ) )
		{
			string valueStr;
			if ( ! anyValue )
			{
				getValueToString( forceCurrentDataSetRow( (*iei).second ), propIndex, valueStr );
			}
			if ( anyValue || (valueStr == searchedValueStr) ) // string comparison
			{
				if ( sortByEntityId )
					resultSortedByEntityId.insert( make_pair( (*iei).first, (*iei).second ) );
				else
					resultSortedByDatasetRow.insert( make_pair( (*iei).second, (*iei).first ) );
				++nbMatchingFilter;
			}
		}
	}
	if ( sortByEntityId )
	{
		for ( map<CEntityId,TDataSetIndex>::const_iterator it=resultSortedByEntityId.begin(); it!=resultSortedByEntityId.end(); ++it )
		{
			displayPropValue( propName, forceCurrentDataSetRow( (*it).second ), propIndex, "", log );
		}
	}
	else
	{
		for ( map<TDataSetIndex,CEntityId>::const_iterator it=resultSortedByDatasetRow.begin(); it!=resultSortedByDatasetRow.end(); ++it )
		{
			displayPropValue( propName, forceCurrentDataSetRow( (*it).first ), propIndex, "", log );
		}
	}
	log.displayRawNL( "  %u entities matching filter", nbMatchingFilter );
	return nbMatchingFilter;
}


/*
 * Display the trackers
 */
void				CMirroredDataSet::displayTrackers( NLMISC::CLog& log ) const
{
	log.displayNL( "*** Dataset %s ***", name().c_str() );

	log.displayNL( "Self entity trackers:" );
	_SelfEntityTrackers[ADDING].displayTrackerInfo( " \tADDING: ", " yet", &log );
	_SelfEntityTrackers[REMOVING].displayTrackerInfo( " \tREMOVING: ", " yet", &log );

	log.displayNL( "Other entity trackers:" );
	sint eti;
	for ( eti=ADDING; eti<=REMOVING; ++eti )
	{
		if ( _EntityTrackers[eti].empty() )
		{
			log.displayNL( " \tNo %s tracker", (eti==ADDING)?"adding":"removing" );
		}
		else
		{
			log.displayNL( " \tList of %d %s trackers", _EntityTrackers[eti].size(), (eti==ADDING)?"adding":"removing" );
			TTrackerListForEnt::const_iterator itl;
			for ( itl=_EntityTrackers[eti].begin(); itl!=_EntityTrackers[eti].end(); ++itl )
			{
				(*itl).displayTrackerInfo( " \t\t", " (ABNORMAL)", &log );
			}
		}
	}

	log.displayNL( "List of %u self prop trackers:", _SelfPropTrackers.size() );
	TSelfPropTrackers::const_iterator ispt;
	for ( ispt=_SelfPropTrackers.begin(); ispt!=_SelfPropTrackers.end(); ++ispt )
	{
		(*ispt).displayTrackerInfo( toString(" \tProperty %hd: ", (*ispt).propIndex() ).c_str(), " (ABNORMAL)", &log );
	}
	log.displayNL( "Other prop trackers:" );
	uint32 propIndex;
	for ( propIndex=0; propIndex!=_PropTrackers.size(); ++propIndex )
	{
		if ( ! _PropTrackers[propIndex].empty() )
		{
			log.displayNL( " \tProperty %hd: %d trackers", (TPropertyIndex)propIndex, _PropTrackers[propIndex].size() );
			TTrackerListForProp::const_iterator itl;
			for ( itl=_PropTrackers[propIndex].begin(); itl!=_PropTrackers[propIndex].end(); ++itl )
			{
				(*itl).displayTrackerInfo( " \t\t", " (ABNORMAL)", &log );
			}
		}
	}
	log.displayNL( "End of trackers" );
}


/*
 * Return the specified self prop tracker, or NULL
 */
CChangeTrackerClientProp *CMirroredDataSet::getSelfPropTracker( TPropertyIndex propIndex )
{
	uint32 i;
	for ( i=0; i!=_SelfPropTrackers.size(); ++i )
	{
		if ( _SelfPropTrackers[i].propIndex() == propIndex )
			break;
	}
	if ( i == _SelfPropTrackers.size() )
		return NULL;
	else
		return &(_SelfPropTrackers[i]);
}


/*
 * Return the local mirror service id (written here for the SEND_ macros to work)
 */
inline NLNET::TServiceId		CMirroredDataSet::localMSId() const
{
	return _PropAllocator->mirrorServiceId();
}


/*
 * Add tracker or add self trackers for the specified property
 */
void				CMirroredDataSet::accessNewPropTracker( bool self, TPropertyIndex propIndex, sint32 smid, sint32 mutid )
{
	if ( self )
	{
		// Search smid in the existing self prop trackers (may already exist for group notification)
		TSelfPropTrackers::const_iterator ipt;
		for( ipt=_SelfPropTrackers.begin(); ipt!=_SelfPropTrackers.end(); ++ipt )
		{
			if ( (*ipt).smid() == smid )
				break;
		}
		if ( ipt == _SelfPropTrackers.end() )
		{
			// Add new tracker only if not found
			_SelfPropTrackers.push_back( CChangeTrackerClientProp( propIndex ) );
			_SelfPropTrackers.back().access( smid );
			_SelfPropTrackers.back().createMutex( mutid, false );
		}
		else
		{
			MIRROR_DEBUG( "MIRROR: Accessing self group tracker for prop %hd (already created)", propIndex );
		}

		// Obsolete: now the ack is forced to arrive after the tracker (sent via the MS)
		/*if ( _SelfPropTrackers.back().RescanEntities )
		{
			uint nbChanges = scanAndResyncProp( propIndex, &(_SelfPropTrackers.back()) );
			nlinfo( "PROPMGT: Tracker (smid %d): filled %u changes for %s/P%hd (after receiving tracker smid)", _SelfPropTrackers.back().smid(), nbChanges, name().c_str(), propIndex );
			// Note: this will lead to unjustified changes! More are better than not enough!

			_SelfPropTrackers.back().RescanEntities = false;
		}*/
	}
	else
	{
		// Search smid in the existing prop trackers for all property names (may already exist if another local service has group notification)
		const CChangeTrackerClient *groupTracker = NULL;
		TTrackersLists::const_iterator itl;
		for ( itl=_PropTrackers.begin(); itl!=_PropTrackers.end(); ++itl )
		{
			TTrackerListForProp::const_iterator ipt;
			for( ipt=(*itl).begin(); ipt!=(*itl).end(); ++ipt )
			{
				if ( (*ipt).smid() == smid )
				{
					groupTracker = &(*ipt);
					break;
				}
			}
			if ( groupTracker )
				break;
		}
		if ( groupTracker )
		{
			// Test if the tracker is already known for this property
			for ( TTrackerListForProp::const_iterator itp=_PropTrackers[propIndex].begin(); itp!=_PropTrackers[propIndex].end(); ++itp )
			{
				if ( &(*itp) == groupTracker )
				{
					MIRROR_DEBUG( "MIRROR: Ignoring tracker, already known for P%hd smid %u", propIndex, smid );
					return;
				}
			}

			// Add a surface-copy of the group tracker for the current property index (using the same data)
			_PropTrackers[propIndex].push_back( *groupTracker );
			_PropTrackers[propIndex].back().setPointingToGroupTracker();
			MIRROR_DEBUG( "MIRROR: Accessing group tracker for prop %hd (already created)", propIndex );
		}
		else
		{
			// Add and access new tracker only if not found
			_PropTrackers[propIndex].push_back( CChangeTrackerClient() );
			_PropTrackers[propIndex].back().access( smid );
			_PropTrackers[propIndex].back().createMutex( mutid, false );
		}
	}
}


/*
 * Add trackers or set the self trackers for adding/removing entities
 */
void				CMirroredDataSet::accessEntityTrackers( bool self, sint32 smidAdd, sint32 smidRemove, sint32 mutidAdd, sint32 mutidRemove )
{
	if ( self )
	{
		_SelfEntityTrackers[ADDING].access( smidAdd );
		_SelfEntityTrackers[ADDING].createMutex( mutidAdd, false );
		_SelfEntityTrackers[REMOVING].access( smidRemove );
		_SelfEntityTrackers[REMOVING].createMutex( mutidRemove, false );

		// Obsolete: now the ack is forced to arrive after the tracker (sent via the MS)
		/*
		if ( _SelfEntityTrackers[ADDING].RescanEntities )
		{
			uint nbAdded = scanAndResyncExistingEntities( 0, maxNbRows() );
			nlinfo( "ROWMGT: AddingTracker (smid %d): filled %u entities in %s (after receiving tracker smid)", _SelfEntityTrackers[ADDING].smid(), nbAdded, name().c_str() );

			_SelfEntityTrackers[ADDING].RescanEntities = false;
		}*/
	}
	else
	{
		// Discard if received twice
		for ( TTrackerListForEnt::const_iterator ite=_EntityTrackers[ADDING].begin(); ite!=_EntityTrackers[ADDING].end(); ++ite )
		{
			if ( (*ite).smid() == smidAdd )
			{
				MIRROR_DEBUG( "MIRROR: Ignoring tracker, already known for entities adding smid %u", smidAdd );
				return;
			}
		}
		_EntityTrackers[ADDING].push_back( CChangeTrackerClient() );
		_EntityTrackers[ADDING].back().access( smidAdd );
		_EntityTrackers[ADDING].back().createMutex( mutidAdd, false );
		_EntityTrackers[REMOVING].push_back( CChangeTrackerClient() );
		_EntityTrackers[REMOVING].back().access( smidRemove );
		_EntityTrackers[REMOVING].back().createMutex( mutidRemove, false );
	}
}


/*
 *
 */
void				CMirroredDataSet::addDeclaredEntityRange( uint8 entityType, const TEntityRange& src, TServiceId declaratorService )
{
	TDeclaredEntityRange declEntityRange( src, NLNET::TServiceId8(declaratorService) );
	_DeclaredEntityRanges.insert( make_pair( entityType, declEntityRange ) );
}


/*
 *
 */
void				CMirroredDataSet::addDeclaredEntityRanges( const TEntityRangeOfType& ranges, NLNET::TServiceId declaratorService )
{
	TEntityRangeOfType::const_iterator it;
	for ( it=ranges.begin(); it!=ranges.end(); ++it )
	{
		addDeclaredEntityRange( (*it).first, (*it).second, declaratorService );
	}
}


/*
 * Set a display callback for the specified property. It will be used by displayPropValue() and CMirror::displayRows() (command "displayMirrorRow")
 */
void				CMirroredDataSet::setDisplayCallback( TPropertyIndex propIndex, TValueToStringFunc cb )
{
	nlassert( (uint)propIndex < _ValueToStringCb.size() );
	_ValueToStringCb[propIndex] = cb;
}


/*
 *
 */
void storeDatasetPtToQuickArray( uint16 bitIndex, CMirroredDataSet *dataSet )
{
	if ( bitIndex < MAX_NB_DATASETS-1 ) // exclude MAX_NB_DATASETS-1 for 'NULL'
		DataSetQuickArray[bitIndex] = dataSet;
	else
		nlerror( "Invalid DataSetQuickArray range (index=%hu, MAX_NB_DATASETS=%u)", bitIndex, MAX_NB_DATASETS );
}



const char *getBlankChars( const string& leftstr )
{
	static string blank;
	blank = "";
	for ( sint i=0; i!=max((sint32)(19-leftstr.size()),(sint32)0)+1; ++i )
	{
		blank += " ";
	}
	return blank.c_str();
}


inline string entityIdToStringCb( void *value )
{
	return ((CEntityId*)value)->toString();
}


string boolToStringCb( void *value )
{
	return (*(bool*)value)?"TRUE":"FALSE";
}



template <class T>
void	displayValueLine( const std::string& propName, const char *valueFormat, const char *typeString, T*, const CMirroredDataSet *dataset, const TDataSetRow& entityIndex, TPropertyIndex propIndex, const char *flagsString, NLMISC::CLog& log, bool typeIsBool=false, bool typeIsEntityId=false )
{
	CMirrorPropValue<T> value( const_cast<CMirroredDataSet&>(*dataset), entityIndex, propIndex );
	TValueToStringFunc valueToString;
	if ( typeIsBool )
		valueToString = boolToStringCb;
	else if ( typeIsEntityId )
		valueToString = entityIdToStringCb;
	else
		valueToString = dataset->valueToStringCb( propIndex );

#ifdef STORE_CHANGE_SERVICEIDS
	log.displayNL( "%s:%s%24s %8u %-10s (%s E%u_%hu_%s_%u P%-3hd %s %s)",
#else
	log.displayNL( "%s:%s%24s %8u (%s E%u_%hu_%s_%u P%-3hd %s %s)",
#endif
		propName.c_str(),
		getBlankChars(propName),
		valueToString ? ( valueToString((void*)&(value())).c_str() ) : ( toString( valueFormat, value() ).c_str() ),
		value.getTimestamp(),
#ifdef STORE_CHANGE_SERVICEIDS
		CUnifiedNetwork::getInstance()->getServiceUnifiedName(value.getWriterServiceId()).c_str(),
#endif
		dataset->name().c_str(),
		entityIndex.getIndex(),
		(uint16)dataset->getBindingCounter(entityIndex.getIndex()),
		dataset->isOnline(entityIndex) ? "ONL" : "OFFL",
		dataset->getOnlineTimestamp(entityIndex),
		propIndex,
		typeString,
		flagsString );
}


// This specialisation is necessary to compile it with gcc because of toString( valueFormat, value() )
template <>
void displayValueLine<NLMISC::CEntityId>( const std::string& propName, const char * /* valueFormat */, const char *typeString, CEntityId*, const CMirroredDataSet *dataset, const TDataSetRow& entityIndex, TPropertyIndex propIndex, const char *flagsString, NLMISC::CLog& log, bool /* typeIsBool */, bool typeIsEntityId )
{
	CMirrorPropValue<CEntityId> value( const_cast<CMirroredDataSet&>(*dataset), entityIndex, propIndex );
	TValueToStringFunc valueToString;
	nlassert( typeIsEntityId );
	valueToString = entityIdToStringCb;

#ifdef STORE_CHANGE_SERVICEIDS
	log.displayNL( "%s:%s%24s %8u %-10s (%s E%u_%hu_%s_%u P%-3hd %s %s)",
#else
	log.displayNL( "%s:%s%24s %8u (%s E%u_%hu_%s_%u P%-3hd %s %s)",
#endif
		propName.c_str(),
		getBlankChars(propName),
		valueToString((void*)&(value())).c_str(),
		value.getTimestamp(),
#ifdef STORE_CHANGE_SERVICEIDS
		CUnifiedNetwork::getInstance()->getServiceUnifiedName(value.getWriterServiceId()).c_str(),
#endif
		dataset->name().c_str(),
		entityIndex.getIndex(),
		(uint16)dataset->getBindingCounter(entityIndex.getIndex()),
		dataset->isOnline(entityIndex) ? "ONL" : "OFFL",
		dataset->getOnlineTimestamp(entityIndex),
		propIndex,
		typeString,
		flagsString );
}


/*
 * Display a value
 */
void				CMirroredDataSet::displayPropValue( const std::string& propName, const TDataSetRow& entityIndex, TPropertyIndex propIndex, const char *flagsStr, NLMISC::CLog& log ) const
{
	if ( ! _PropertyContainer.PropertyValueArrays[propIndex].Values )
		return;

	TTypeOfProp propType = getPropType( propIndex );
	switch( propType )
	{
		case TypeUint8:
			{
				displayValueLine( propName, "%hu", "U8", (uint8*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeSint8:
			{
				displayValueLine( propName, "%hd", "S8", (sint8*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeUint16:
			{
				displayValueLine( propName, "%hu", "U16", (uint16*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeSint16:
			{
				displayValueLine( propName, "%hd", "S16", (sint16*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeUint32:
			{
				displayValueLine( propName, "%u", "U32", (uint32*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeSint32:
			{
				displayValueLine( propName, "%d", "S32", (sint32*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeUint64:
			{
				displayValueLine( propName, "%"NL_I64"u", "U64", (uint64*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeSint64:
			{
				displayValueLine( propName, "%"NL_I64"d", "S64", (sint64*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeFloat:
			{
				displayValueLine( propName, "%g", "F", (float*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeDouble:
			{
				displayValueLine( propName, "%g", "D", (double*)NULL, this, entityIndex, propIndex, flagsStr, log );
				break;
			}
		case TypeBool:
			{
				displayValueLine( propName, "", "B", (bool*)NULL, this, entityIndex, propIndex, flagsStr, log, true );
				break;
			}
		case TypeCEntityId:
			{
				displayValueLine( propName, "", "EId", (CEntityId*)NULL, this, entityIndex, propIndex, flagsStr, log, false, true );
				break;
			}
		default:
			log.displayNL( "%s\t <Invalid type>\t\t\t(%s/E%u%s_%hu/P%hd %s)", propName.c_str(), name().c_str(), (uint32)entityIndex.getIndex(), isOnline(entityIndex)?"":"@@@", (uint16)getBindingCounter(entityIndex.getIndex()), propIndex, flagsStr ); // should not be offline!
	}

}


template <class T>
void	assignValue( CMirroredDataSet *dataset, const TDataSetRow& entityIndex, TPropertyIndex propIndex, const char *valueStr, const char *formatStr, T* )
{
	T tmp;
	sscanf( valueStr, formatStr, &tmp );
	CMirrorPropValue<T> value( *dataset, entityIndex, propIndex );
	value = tmp;
}


/*
 * Change a value from a string
 */
void				CMirroredDataSet::setValueFromString( const TDataSetRow& entityIndex, TPropertyIndex propIndex, const std::string& valueStr )
{
	TTypeOfProp propType = getPropType( propIndex );
	switch( propType )
	{
		case TypeUint8:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%hu", (uint8*)NULL );
				break;
			}
		case TypeSint8:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%hd", (sint8*)NULL );
				break;
			}
		case TypeUint16:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%hu", (uint16*)NULL );
				break;
			}
		case TypeSint16:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%hd", (sint16*)NULL );
				break;
			}
		case TypeUint32:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%u", (uint32*)NULL );
				break;
			}
		case TypeSint32:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%d", (sint32*)NULL );
				break;
			}
		case TypeUint64:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%"NL_I64"u", (uint64*)NULL );
				break;
			}
		case TypeSint64:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%"NL_I64"d", (sint64*)NULL );
				break;
			}
		case TypeFloat:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%g", (float*)NULL );
				break;
			}
		case TypeDouble:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%g", (double*)NULL );
				break;
			}
		case TypeCEntityId:
			{
				CEntityId tmp;
				tmp.fromString( valueStr.c_str() );
				CMirrorPropValue<CEntityId> value( *this, entityIndex, propIndex );
				value = tmp;
				break;
			}
		case TypeBool:
			{
				assignValue( this, entityIndex, propIndex, valueStr.c_str(), "%hu", (bool*)NULL );
				break;
			}
		default:
			nlwarning( "<Invalid type>\t\t(%s/E%d/P%hd)", name().c_str(), entityIndex.getIndex(), propIndex );
	}
}


template <class T>
void	displayValue( const CMirroredDataSet *dataset, const TDataSetRow& entityIndex, TPropertyIndex propIndex, char *destStr, const char *formatStr, T* )
{
	TValueToStringFunc valueToString = dataset->valueToStringCb( propIndex );
	CMirrorPropValueRO<T> value( *dataset, entityIndex, propIndex );
	if ( valueToString )
		smprintf( destStr, 31, "%s", valueToString((void*)&(value())).c_str() );
	else
		smprintf( destStr, 31, formatStr, value() );
}



/*
 * Return a property value as string
 */
void				CMirroredDataSet::getValueToString( const TDataSetRow& entityIndex, TPropertyIndex propIndex, std::string& result ) const
{
	char tmpStr [32];
	TTypeOfProp propType = getPropType( propIndex );
	switch( propType )
	{
		case TypeUint8:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%hu", (uint8*)NULL );
				break;
			}
		case TypeSint8:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%hd", (sint8*)NULL );
				break;
			}
		case TypeUint16:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%hu", (uint16*)NULL );
				break;
			}
		case TypeSint16:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%hd", (sint16*)NULL );
				break;
			}
		case TypeUint32:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%u", (uint32*)NULL );
				break;
			}
		case TypeSint32:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%d", (sint32*)NULL );
				break;
			}
		case TypeUint64:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%"NL_I64"u", (uint64*)NULL );
				break;
			}
		case TypeSint64:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%"NL_I64"d", (sint64*)NULL );
				break;
			}
		case TypeFloat:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%g", (float*)NULL );
				break;
			}
		case TypeDouble:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%g", (double*)NULL );
				break;
			}
		case TypeCEntityId:
			{
				CMirrorPropValueRO<CEntityId> value( *this, entityIndex, propIndex );
				result = value().toString();
				return; // different
			}
		case TypeBool:
			{
				displayValue( this, entityIndex, propIndex, tmpStr, "%hu", (bool*)NULL );
				break;
			}
		default:
			result = toString( "<Invalid type> (%s/E%d/P%hd)", name().c_str(), entityIndex.getIndex(), propIndex );
	}
	result = string(tmpStr);
}


extern sint32 NbAllocdListCells;

/*
 * Report a number of list cells from uncleared previous session (call before clearing them)
 */
void	CMirroredDataSet::reportOrphanSharedListCells( sint32 nb )
{
	NbAllocdListCells += nb;
}


/*
 * Return the number of known shared list cells
 */
sint32	CMirroredDataSet::getNbKnownSharedListCells()
{
	return NbAllocdListCells;
}


/*
 * Return a string showing the values tested by isAccessible()
 */
std::string	CMirroredDataSet::explainIsAccessible( const TDataSetRow& row ) const
{
	return NLMISC::toString("Row %s is %saccessible because: maxNbRows=%u, isOnline=%s, counter=%u",
		row.toString().c_str(), isAccessible(row) ? "" : "not ", maxNbRows(),
		_PropertyContainer.EntityIdArray.isOnline(row.getIndex()) ? "YES" : "NO",
		(uint)(_PropertyContainer.EntityIdArray.Counts[row.getIndex()]) );
}
