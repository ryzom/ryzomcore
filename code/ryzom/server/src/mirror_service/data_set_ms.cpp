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



#include "data_set_ms.h"

#include <nel/misc/shared_memory.h>
#include <nel/misc/sheet_id.h>
#include <nel/georges/u_form_elm.h>
#include "mirror_service.h"

using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;
using namespace std;


extern bool DestroyGhostSharedMemSegments;

/*
 * Intialize
 */
void	CDataSetMS::init( const NLMISC::CSheetId& sheetId, const TDataSetSheet& properties, TPropertiesInMirrorMS& propertyMap )
{
	CDataSetBase::init( sheetId, properties );

	//_Weights.resize( properties.Weights.size() ); // cannot assign directly because of different element type
	//copy( properties.Weights.begin(), properties.Weights.end(), _Weights.begin() );
	_Weights.resize( nbProperties(), 1 ); // feature currently not used
	_ChangesBudgets.resize( nbProperties() );

	// Fill map of properties
	for ( sint p=0; p!=properties.NbProperties; ++p )
	{
		propertyMap.insert( make_pair( properties.PropertyNames[p], TPropertyInfoMS( this, p ) ) );
		nldebug( "\t Dataset %s: %4d Prop %s", name().c_str(), p, properties.PropertyNames[p].c_str() );
	}
	propertyMap.insert( make_pair( string("_EId_")+properties.DataSetName, TPropertyInfoMS( this, INVALID_PROPERTY_INDEX ) ) );

	_PropertyNames = properties.PropertyNames;
	_SubscribersByProperty.resize( properties.NbProperties );
}


/*
 * Add the specified property
 */
CChangeTrackerMS&	CDataSetMS::addPropTracker( TPropertyIndex propIndex, TServiceId destId, bool local, bool readOnly, bool allocate, bool writeOnly )
{
	nldebug( "TRACKER/%s: Adding %s %s property tracker for propindex %hd, %s service %s", name().c_str(), readOnly?"RO":"RW", allocate?"allocated":"notallocd", propIndex, local?"local":"mirror", servStr(destId).c_str() );
	nlassert( propIndex < (TPropertyIndex)_SubscribersByProperty.size() );
	_SubscribersByProperty[propIndex].push_back( CChangeTrackerMS( destId, local, readOnly, writeOnly ) );
	return _SubscribersByProperty[propIndex].back();
}


/*
 * Add a tracker for adding/removing entities
 */
CChangeTrackerMS&	CDataSetMS::addEntityTracker( TEntityTrackerIndex addOrRemove, TServiceId destId, bool local, sint32 smid )
{
	nldebug( "TRACKER/%s: Adding entity tracker (%u) for %s service %s (smid %d)", name().c_str(), (uint32)addOrRemove, local?"local":"mirror", servStr(destId).c_str(), smid );
	_EntityTrackers[addOrRemove].push_back( CChangeTrackerMS( destId, local ) );
	return _EntityTrackers[addOrRemove].back();
}


/*
 *
 */
void				CDataSetMS::readdAllocatedTrackers( NLNET::CMessage& msgin, TServiceId serviceId, CSMIdPool& smidpool, CSMIdPool& mutidpool )
{
	// Prop trackers
	TSelfPropTrackers selfPropTrackers;
	msgin.serialCont( selfPropTrackers );

	TSelfPropTrackers::const_iterator ipt;
	for ( ipt=selfPropTrackers.begin(); ipt!=selfPropTrackers.end(); ++ipt )
	{
		smidpool.reacquireId( (*ipt).smid() );
		mutidpool.reacquireId( (*ipt).mutid() );

		_SubscribersByProperty[(*ipt).propIndex()].push_back(
			CChangeTrackerMS( serviceId, true,
							  false, false, // at this time, we don't know the RO/WO state yet (set later in resetTrackerFlags())
							  (*ipt).isPointingToGroupTracker()?PointingToGroupTracker:Normal ) ); // and GroupTracker??

		_SubscribersByProperty[(*ipt).propIndex()].back().reaccess( (*ipt).smid() );
		_SubscribersByProperty[(*ipt).propIndex()].back().createMutex( (*ipt).mutid(), false );
	}
	nlinfo( "TRACKER: Reaccessed %u allocated prop trackers for service %hu", selfPropTrackers.size(), serviceId.get() );

	// Entity trackers
	CChangeTrackerClient selfAddingTracker;
	msgin.serial( selfAddingTracker );
	smidpool.reacquireId( selfAddingTracker.smid() );
	mutidpool.reacquireId( selfAddingTracker.mutid() );
	_EntityTrackers[ADDING].push_back( CChangeTrackerMS( serviceId, true ) );
	_EntityTrackers[ADDING].back().reaccess( selfAddingTracker.smid() );
	_EntityTrackers[ADDING].back().createMutex( selfAddingTracker.mutid(), false );
	CChangeTrackerClient selfRemovingTracker;
	msgin.serial( selfRemovingTracker );
	smidpool.reacquireId( selfRemovingTracker.smid() );
	mutidpool.reacquireId( selfRemovingTracker.mutid() );
	_EntityTrackers[REMOVING].push_back( CChangeTrackerMS( serviceId, true ) );
	_EntityTrackers[REMOVING].back().reaccess( selfRemovingTracker.smid() );
	_EntityTrackers[REMOVING].back().createMutex( selfRemovingTracker.mutid(), false );
	nlinfo( "TRACKER: Reeaccessed entity trackers for service %hu", serviceId.get() );
}


/*
 *
 */
void				CDataSetMS::readdTracker( TServiceId serviceId, TPropertyIndex propIndex, bool readOnly, bool writeOnly, bool pointingToGroup, TPropertyIndex propNotifyingTheGroup )
{
	// The tracker is not here yet, add it as 'not allocated' and set its flags
	_SubscribersByProperty[propIndex].push_back( CChangeTrackerMS( serviceId, true, readOnly, writeOnly, pointingToGroup?PointingToGroupTracker:Normal ) );

	if ( pointingToGroup )
	{
		// Find pointed tracker
		TSubscriberListForProp::iterator isl;
		for ( isl=_SubscribersByProperty[propNotifyingTheGroup].begin(); isl!=_SubscribersByProperty[propNotifyingTheGroup].end(); ++isl )
		{
			if ( (*isl).destServiceId() == serviceId )
			{
				_SubscribersByProperty[propIndex].back().useGroupTracker( &(*isl) ); // set as pointing
				(*isl).useGroupTracker( NULL ); // set as group tracker
				nlinfo( "TRACKER: Tracker for service %hu, prop %hd pointing to prop %hd (smid %d)", serviceId.get(), propIndex, propNotifyingTheGroup, (*isl).smid() );
				break;
			}
		}
	}
}


/*
 * Clear all the property values of the row
 */
void				CDataSetMS::clearValuesOfRow( TDataSetIndex entityIndex )
{
	//nldebug( "Clearing values of E%u", entityIndex );
	uint propIndex;
	for ( propIndex=0; propIndex!=_PropertyContainer.PropertyValueArrays.size(); ++propIndex )
	{
		if ( _PropertyContainer.PropertyValueArrays[propIndex].Values == NULL )
			continue;

		// Do not clear if an initial property value was received before the entity addition
		// It can occur if the service spawns the entity outside the tick update callback or outside
		// any synchronised message callback. V1.5: now property values are transfered up to 10 ticks
		// before initial entity => keep any value with non-null timestamp
		const NLMISC::TGameCycle& propTimestamp = _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex];
		if ( propTimestamp != 0 )
		{
			//nldebug( "ROWMGT: Keeping property E%hu P%hd because timestamp=%u", entityIndex, propIndex, propTimestamp );
			continue;
		}
		//nldebug( "ROWMGT: Resetting property E%hu P%hd", entityIndex, propIndex );

		// Empty list container if it's a list property
		if ( _PropIsList[ propIndex ] )
		{
			//nldebug( "KillList E%u", entityIndex );
			killList( forceCurrentDataSetRow( entityIndex ), propIndex );
			TSharedListRow *listHeaders = (TSharedListRow*)(_PropertyContainer.PropertyValueArrays[propIndex].Values);
			listHeaders[entityIndex] = INVALID_SHAREDLIST_ROW;  // TODO: remove (already done by killList())
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
		_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex].set(0);
#endif
	}
}


/*
 * Tolerance: 10 increments
 */
inline bool			isCircularCounterGreaterThan( uint8 counter1, uint8 counter2 )
{
	return (counter1 > counter2) || ((counter1 < 6) && (counter2 > 250)); // (0 is always skipped)
}


/*
 *
 */
bool				CDataSetMS::transmitAllBindingCounters( NLMISC::CMemStream& deltaBuffer, const TClientServices& clientServices, bool forceRemovals, bool inDelta ) const
{
	if ( ! _PropertyContainer.EntityIdArray.Counts )
		return false;

	sint32 sizeBefore = deltaBuffer.getPos();
	TDataSetIndex first = 0, end = maxNbRows();
	deltaBuffer.serial( first, end );
	TDataSetIndex entityIndex;
	for ( entityIndex=first; entityIndex!=end; ++entityIndex )
	{
		uint8 bindingCounter = getBindingCounter( entityIndex );
		deltaBuffer.serial( bindingCounter );
		// We don't send the online timestamps, their time is local to one machine
	}

	// Remove the entities that correspond to locally managed offline entities.
	// because *if the MS has already sent row management to the remote MS*, the remote MS would know some
	// previous entities and could have not received some recent removals.
	uint32 nbRanges = 0;
	uint32 nbChanges = 0;
	if ( forceRemovals )
	{
#ifdef DISPLAY_DELTAS
		nldebug( "ROWMGT: Forcing removals:" );
#endif
		sint32 posForNb = deltaBuffer.reserve( sizeof(nbChanges) );
		for ( TClientServices::const_iterator ics=MSInstance->clientServices().begin(); ics!=MSInstance->clientServices().end(); ++ics )
		{
			const TClientServiceInfo& cs = GET_CLIENT_SERVICE_INFO(ics);
			for ( std::vector<TClientServiceInfoOwnedRange>::const_iterator ior=cs.OwnedRanges.begin(); ior!=cs.OwnedRanges.end(); ++ior )
			{
				const TClientServiceInfoOwnedRange& sior = (*ior);
				if ( sior.DataSet != this )
					continue;
				for ( entityIndex=sior.First; entityIndex<=sior.Last; ++entityIndex )
				{
					if ( entityIndex == INVALID_DATASET_INDEX )
					{
						// Workaround
						nlwarning( "Invalid datasetrow in owned range [%u,%u] of %s", sior.First, sior.Last, CUnifiedNetwork::getInstance()->getServiceUnifiedName( (*ics).first ).c_str() );
						break;
					}
					if ( ! _PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
					{
						TServiceId8 spawnerId = getSpawnerServiceId( entityIndex );

						// If an old offline entity has the same creator id as this service, it will be included
						// in this message. However, here we process only the ranges owned by current local services, i.e.
						// *no entity* in these ranges can be used (e.g. created but undeclared) by another service.
						// Note: the recipient MS may produce warnings "already offline".
						if ( clientServices.find( spawnerId ) != clientServices.end() )
						{
							deltaBuffer.serial( entityIndex );
							++nbChanges;
#ifdef DISPLAY_DELTAS
							DebugLog->displayRawNL( " %u_", entityIndex );
#endif
						}
					}
				}
				++nbRanges;
			}
		}
		deltaBuffer.poke( nbChanges, posForNb );
	}
	else
	{
		deltaBuffer.serial( nbChanges );
	}

	sint sizeOfSync = deltaBuffer.getPos() - sizeBefore;
	nlinfo( "ROWMGT: %s to remote MS with binding counters and %u removals in %u ranges (%.3f KB)", inDelta ? "Delta" : "Message", nbChanges, nbRanges, ((float)sizeOfSync) / 1024.0f );
	return true;
}

/*
 *
 */
void				CDataSetMS::transmitAllBindingCounters( CDeltaToMS *delta, const TClientServices& clientServices ) const
{
	if ( _PropertyContainer.EntityIdArray.Counts )
	{
		delta->beginTransmitAllBindingCounters();
		transmitAllBindingCounters( delta->getDeltaBuffer(), clientServices, true, true );
		delta->endTransmitAllBindingCounters();
	}
	else
	{
		// Skip if no service ever connected to this MS (for this dataset)
		delta->cancelTransmitAllBindingCounters();
	}
}


/*
 * The range must belong to only one writer service
 */
void				CDataSetMS::transmitBindingCountersOfRange( NLMISC::CMemStream& deltaBuffer, TDataSetIndex first, TDataSetIndex end, TServiceId serviceId ) const
{
	nlassert ( _PropertyContainer.EntityIdArray.Counts );

	sint32 sizeBefore = deltaBuffer.getPos();
	deltaBuffer.serial( first, end );
	TDataSetIndex entityIndex;
	for ( entityIndex=first; entityIndex!=end; ++entityIndex )
	{
		uint8 bindingCounter = getBindingCounter( entityIndex );
		deltaBuffer.serial( bindingCounter );
		// We don't send the online timestamps, their time is local to one machine
	}

	// Remove the entities that correspond to the service owning the range
	// because *if the MS has already sent row management to the remote MS*, the remote MS would know some
	// previous entities and could have not received some recent removals.
	uint32 nbChanges = 0;
#ifdef DISPLAY_DELTAS
	nldebug( "ROWMGT: Forcing removals:" );
#endif
	sint32 posForNb = deltaBuffer.reserve( sizeof(nbChanges) );
	for ( entityIndex=first; entityIndex!=end; ++entityIndex )
	{
		if ( ! _PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
		{
			// If an old offline entity has the same creator id as this service, it will be included
			// in this message. However, here we process only the ranges owned by this service, i.e.
			// *no entity* in these ranges can be used (e.g. created but undeclared) by another service.
			// Note: the recipient MS may produce warnings "already offline".
			TServiceId8 spawnerId = getSpawnerServiceId( entityIndex );
			if ( spawnerId == serviceId ) // avoid sending useless empty rows
			{
				deltaBuffer.serial( entityIndex );
				++nbChanges;
#ifdef DISPLAY_DELTAS
				DebugLog->displayRawNL( " %u_", entityIndex );
#endif
			}
		}
	}
	deltaBuffer.poke( nbChanges, posForNb );

	sint sizeOfSync = deltaBuffer.getPos() - sizeBefore;
	nlinfo( "ROWMGT: Message to remote MS with range binding counters and %u removals (%.3f KB)", nbChanges, ((float)sizeOfSync) / 1024.0f );
}


/*
 * See applyAllBindingCounters()
 */
void				CDataSetMS::receiveAllBindingCounters( NLNET::CMessage& msgin )
{
	TBindingCountersToApply bc;

	// Counters
	TDataSetIndex first, end;
	msgin.serial( first, end );
	bc.RangeFirst = first;
	TDataSetIndex entityIndex;
	for ( entityIndex=first; entityIndex!=end; ++entityIndex )
	{
		uint8 counter;
		msgin.serial( counter );
		bc.Counters.push_back( counter );
	}
	// Obsolete entities
	uint32 nbToRemove;
	msgin.serial( nbToRemove );
	for ( uint i=0; i!=nbToRemove; ++i )
	{
		msgin.serial( entityIndex );
		bc.EntitiesToRemove.push_back( entityIndex );
	}

	_ReceivedBindingCounters.push_back( bc );
}


/*
 * See applyAllBindingCounters()
 */
void				CDataSetMS::processReceivedBindingCounters()
{
	for ( vector<TBindingCountersToApply>::iterator itm=_ReceivedBindingCounters.begin(); itm!=_ReceivedBindingCounters.end(); ++itm )
	{
		applyAllBindingCounters( *itm );
	}
	NLMISC::contReset( _ReceivedBindingCounters );
}


/*
 * This must be done in a synchronized state, because this method can clear some timestamps.
 * If a concurrent service was writing a property simultaneously, the timestamps could be
 * overwritten, then tests on null timestamps (e.g. in clearValuesOfRow()) would fail.
 * Added: V1.8
 * See also alternate version below
 */
void				CDataSetMS::applyAllBindingCounters( NLNET::CMessage& msgin )
{
	TDataSetIndex first, end;
	msgin.serial( first, end );

	if ( _PropertyContainer.EntityIdArray.Counts )
	{
		// The entity ids in the dataset are already initialised => set the counters
		TDataSetIndex entityIndex;
		for ( entityIndex=first; entityIndex!=end; ++entityIndex )
		{
			uint8 counter;
			msgin.serial( counter );

			// Apply only the greatest counter, to avoid conflicts between syncs from different MSes
			if ( isCircularCounterGreaterThan( counter, _PropertyContainer.EntityIdArray.Counts[entityIndex] ) )
				_PropertyContainer.EntityIdArray.Counts[entityIndex] = counter;

			//if ( _PropertyContainer.EntityIdArray.Counts[entityIndex] != 1 ) //TEMP
			//	nldebug( "< E%u_%hu", entityIndex, (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex] );
		}
		// Erase obsolete entities
		uint32 nbToRemove;
		msgin.serial( nbToRemove );
		for ( uint i=0; i!=nbToRemove; ++i )
		{
			msgin.serial( entityIndex );

			// Set offline (will be useful if it wasn't, but not harmful if it is already)
			_PropertyContainer.EntityIdArray.setOnline( entityIndex, false );

			// Clear the timestamp of all properties for the entity
			// to prevent a connecting service to receive values corresponding to no longer entity
			for ( uint propIndex=0; propIndex!=_PropertyContainer.PropertyValueArrays.size(); ++propIndex )
			{
				if ( _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps )
					_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex] = 0;
			}

			// Don't notify anyone, a new service will rescan the online entities
			// BUG: if the entity was just removed in the same tick, local services won't know the removal
		}

		nlinfo( "Received binding counters (range [%u..%u[) and %u offline entities", first, end, nbToRemove );
	}
	else
	{
		// Binding counters: we store them in a vector because as this time we don't know
		// if the shared memory will be ever allocated.
		// TODO: If all services (including remote MS) that use the dataset leave and the
		// vector is still unused, delete it.
		if ( (! _BindingCountsToSet) && (first != INVALID_DATASET_INDEX) )
		{
			_BindingCountsToSet = new vector<uint8>;
			_BindingCountsToSet->resize( maxNbRows(), 0 );
		}
		TDataSetIndex entityIndex;
		for ( entityIndex=first; entityIndex!=end; ++entityIndex )
		{
			uint8 counter;
			msgin.serial( counter );

			// Apply only the greatest counter, to avoid conflicts between syncs from different MSes
			if ( isCircularCounterGreaterThan( counter, (*_BindingCountsToSet)[entityIndex] ) )
				(*_BindingCountsToSet)[entityIndex] = counter;

			//if ( (*_BindingCountsToSet)[entityIndex] != 1 ) //TEMP
			//	nldebug( "< E%u_%hu", entityIndex, (uint16)(*_BindingCountsToSet)[entityIndex] );
		}

		uint32 nbToRemove;
		msgin.serial( nbToRemove );
		for ( uint i=0; i!=nbToRemove; ++i )
		{
			msgin.serial( entityIndex ); // nothing to do with it
		}

		nlinfo( "Received binding counters (range [%u..%u[) for later application", first, end );
	}
}


/*
 * Same as above but alternate version with stored data from a previous message
 */
void				CDataSetMS::applyAllBindingCounters( const TBindingCountersToApply& bc )
{
	if ( _PropertyContainer.EntityIdArray.Counts )
	{
		// The entity ids in the dataset are already initialised => set the counters
		TDataSetIndex entityIndex = bc.RangeFirst;
		for ( vector<uint8>::const_iterator ic=bc.Counters.begin(); ic!=bc.Counters.end(); ++ic )
		{
			const uint8& counter = (*ic);

			// Apply only the greatest counter, to avoid conflicts between syncs from different MSes
			if ( isCircularCounterGreaterThan( counter, _PropertyContainer.EntityIdArray.Counts[entityIndex] ) )
				_PropertyContainer.EntityIdArray.Counts[entityIndex] = counter;

			//if ( _PropertyContainer.EntityIdArray.Counts[entityIndex] != 1 ) //TEMP
			//	nldebug( "< E%u_%hu", entityIndex, (uint16)_PropertyContainer.EntityIdArray.Counts[entityIndex] );

			++entityIndex;
		}
		// Erase obsolete entities
		for ( vector<TDataSetIndex>::const_iterator ie=bc.EntitiesToRemove.begin(); ie!=bc.EntitiesToRemove.end(); ++ie )
		{
			const TDataSetIndex& entityIndex = (*ie);

			// Set offline (will be useful if it wasn't, but not harmful if it is already)
			_PropertyContainer.EntityIdArray.setOnline( entityIndex, false );

			// Clear the timestamp of all properties for the entity
			// to prevent a connecting service to receive values corresponding to no longer entity
			for ( uint propIndex=0; propIndex!=_PropertyContainer.PropertyValueArrays.size(); ++propIndex )
			{
				if ( _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps )
					_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex] = 0;
			}

			// Don't notify anyone, a new service will rescan the online entities
			// BUG: if the entity was just removed in the same tick, local services won't know the removal
		}

		nlinfo( "Received binding counters (range [%u..%u[) and %u offline entities", bc.RangeFirst, bc.RangeFirst+bc.Counters.size(), bc.EntitiesToRemove.size() );
	}
	else
	{
		// Binding counters: we store them in a vector because as this time we don't know
		// if the shared memory will be ever allocated.
		// TODO: If all services (including remote MS) that use the dataset leave and the
		// vector is still unused, delete it.
		if ( (! _BindingCountsToSet) && (bc.RangeFirst != INVALID_DATASET_INDEX) )
		{
			_BindingCountsToSet = new vector<uint8>;
			_BindingCountsToSet->resize( maxNbRows(), 0 );
		}
		TDataSetIndex entityIndex = bc.RangeFirst;
		for ( vector<uint8>::const_iterator ic=bc.Counters.begin(); ic!=bc.Counters.end(); ++ic )
		{
			const uint8& counter = (*ic);

			// Apply only the greatest counter, to avoid conflicts between syncs from different MSes
			if ( isCircularCounterGreaterThan( counter, (*_BindingCountsToSet)[entityIndex] ) )
				(*_BindingCountsToSet)[entityIndex] = counter;

			//if ( (*_BindingCountsToSet)[entityIndex] != 1 ) //TEMP
			//	nldebug( "< E%u_%hu", entityIndex, (uint16)(*_BindingCountsToSet)[entityIndex] );

			++entityIndex;
		}

		// Nothing to do with EntitiesToRemove

		nlinfo( "Received binding counters (range [%u..%u[) for later application", bc.RangeFirst, bc.RangeFirst+bc.Counters.size() );
	}
}


/*
 * Allocate the tracker header and item array (shared memory)
 */
bool	CChangeTrackerMS::allocate( sint32 shmid, sint32 nbRowsToAllocate )
{
	_SMId = shmid;

	uint32 segmentSize = sizeof(TChangeTrackerHeader) + (nbRowsToAllocate*sizeof(TChangeTrackerItem));
	_Header = (TChangeTrackerHeader*)CSharedMemory::createSharedMemory( toSharedMemId(_SMId), segmentSize );
	if ( (_Header == NULL) && DestroyGhostSharedMemSegments )
	{
		// Under Linux, segment may have not been destroyed if the MS crashed: destroy (if asked in the config file, and retry)
		CSharedMemory::destroySharedMemory( toSharedMemId(_SMId), true );
		nlinfo( "SHDMEM: Destroyed shared memory segment, smid %d", _SMId );
		_Header = (TChangeTrackerHeader*)CSharedMemory::createSharedMemory( toSharedMemId(_SMId), segmentSize );
	}
	if ( ! _Header )
	{
		nlwarning( "SHDMEM: Cannot allocate shared memory for tracker (smid %d)", _SMId );
		beep( 280, 150 );
		return false;
	}

	_Array = (TChangeTrackerItem*)(((uint8*)_Header) + sizeof( TChangeTrackerHeader ));

	// Reset header and array
	_Header->First	=	LAST_CHANGED;
	_Header->Last	=	INVALID_DATASET_INDEX;
#ifdef COUNT_MIRROR_PROP_CHANGES
	//_Header->NbValuesSet = 0;
	_Header->NbDistinctChanges = 0;
#endif

	for ( uint32	i=0; i!=(uint32)nbRowsToAllocate; ++i )
	{
		_Array[i].NextChanged = INVALID_DATASET_INDEX;
	}
	return true;
}


/*
 * Reaccess a previously allocated tracker
 */
void	CChangeTrackerMS::reaccess( sint32 shmid )
{
	_SMId = shmid;

	_Header = (TChangeTrackerHeader*)CSharedMemory::accessSharedMemory( toSharedMemId(_SMId) );
	if ( ! _Header )
	{
		nlwarning( "SHDMEM: Cannot reaccess shared memory for tracker (smid %d)", _SMId );
		beep( 280, 150 );
		return;
	}

	_Array = (TChangeTrackerItem*)(((uint8*)_Header) + sizeof( TChangeTrackerHeader ));
}


/*
 * If param not null: instead of allocating, use another tracker's data. If null: set as group tracker
 */
void	CChangeTrackerMS::useGroupTracker( const CChangeTrackerMS *groupTracker )
{
	if ( groupTracker )
	{
		operator=( *groupTracker ); // copies the information in the tracker object, does not duplicate the data in shared memory
		_GroupTrackerState = PointingToGroupTracker;
	}
	else
		_GroupTrackerState = IsGroupTracker;
}


/*
 * Allow/block sending to remote MS.
 * Precondition: isLocal()
 * Postcondition: b == (_BlockingInfo != NULL)
 */
void	CChangeTrackerMS::allowSendingToRemoteMS( bool b )
{
	nlassert( ! isLocal() );
	if ( ! b )
	{
		// Block
		if ( ! _BlockingInfo )
		{
			_BlockingInfo = new CTrackerBlockingInfo();
			nldebug( "Blocking tracker (smid %d) to MS-%hu", smid(), destServiceId().get() );
		}
	}
	else
	{
		// Unblock
		if ( _BlockingInfo )
		{
			delete _BlockingInfo;
			_BlockingInfo = NULL;
			nldebug( "Unblocking tracker (smid %d) to MS-%hu", smid(), destServiceId().get() );
		}
	}
}


/*
 * [Remote ADDING entity tracker only]
 * Set the counter of acks to receive before unblocking the tracker
 */
void	CChangeTrackerMS::pushATEAcknowledgesExpectedBeforeUnblocking( TServiceId remoteClientServiceId, const vector<TServiceId>& entityTrackerPairsSentTo )
{
	// Push
	allowSendingToRemoteMS( false );
	bool tagGetsWider = (MSInstance->hasTagOfServiceChanged( (TServiceId)destServiceId()) &&
					    (MSInstance->getPreviousTagOfService( destServiceId() ).rawTag() != AllTag));
	_BlockingInfo->pushAckExpected( remoteClientServiceId, entityTrackerPairsSentTo, tagGetsWider );
	nldebug( "pushATEAcknowledgesExpectedBeforeUnblocking[MS-%hu]: %hu", destServiceId().get(), _BlockingInfo->nbAcksExpected() );

	// If no actual ack is expected, pop at once
	if ( entityTrackerPairsSentTo.empty() )
		popATEAcknowledgesExpected( TServiceId(255), false );
}


/*
 * [Blocked remote ADDING entity tracker only]
 * (255 is used for "no serviceIdFrom")
 * Called when an ack is received
 */
void	CChangeTrackerMS::popATEAcknowledgesExpected( TServiceId serviceIdFrom, bool wasTrackerAdded )
{
	// Pop
	nlassert( (! _Local) && _BlockingInfo && _LinkToDeltaBuffer );
	_BlockingInfo->popAckExpected( serviceIdFrom, wasTrackerAdded );
	nldebug( "popATEAcknowledgesExpected[MS-%hu]: %hu", destServiceId().get(), _BlockingInfo->nbAcksExpected() );
	if ( _BlockingInfo->canSyncNow() )
	{
//
		//if ( _BlockingInfo->hasRemoteTagGotWider() )
		{
			if ( _BlockingInfo->weHaveWriters() )
			{
				_LinkToDeltaBuffer->setMustTransmitAllBindingCounters();

				if ( _LinkToDeltaBuffer->dataSet()->nbLocalWriterProps() == 0 )
				{
					// The delta would not be sent; we should forced it once
					nlerror( "Dataset %s has writers for entities but not for properties: not supported at the moment" );
				}
			}
			else
			{
				// No sync is needed (for example, if the local service does not own any range, or there is no local service).
				// => Cancel sync of entities, and transmit the binding counters at once
				// because maybe we don't send deltas to this MS. As the new service needing the binding counters
				// will not use them immediately, it's ok to send by a separate message.
				// See more explanation at end of CMirrorService::allocateEntityTrackers().
				// We don't unblock the tracker here, it will be done only when a writer connects.
				// (note: the corresponding EntityTrackerRemoving may not been blocked)
				_BlockingInfo->resetInfo();
				nldebug( "Tracker to MS-%hu: no sync of entities but of binding counters needed", destServiceId().get() );
				CMessage msgout( "BIDG_CNTR" );
				msgout.serial( const_cast<string&>(_LinkToDeltaBuffer->dataSet()->name()) );
				if ( _LinkToDeltaBuffer->dataSet()->transmitAllBindingCounters( msgout, MSInstance->clientServices(), true, false ) )
					CUnifiedNetwork::getInstance()->send( destServiceId(), msgout );
			}
		}
		/*else
		{
			// No sync is needed because the new tag of the remote MS is not "wider" as the previous one
			if ( _BlockingInfo->weHaveWriters() )
			{
				allowSendingToRemoteMS( true );
				CChangeTrackerMS *remTracker = _LinkToDeltaBuffer->dataSet()->findEntityTracker( REMOVING, destServiceId() );
				if ( remTracker )
					remTracker->allowSendingToRemoteMS( true );
				nldebug( "Tracker to MS-%hu: no sync of entities needed, entity trackers reopened", destServiceId() );
			}
			else
			{
				_BlockingInfo->resetInfo();
				nldebug( "Tracker to MS-%hu: no sync of entities needed, tracker left closed", destServiceId() );
			}
		}*/
	}
}


/*
 * [Blocked remote ADDING entity tracker only]
 * Return true if all acks have been received and a sync is needed. If true, you should unblock the tracker.
 */
bool	CChangeTrackerMS::needsSyncAfterAllATEAcknowledgesReceived()
{
	nlassert( _BlockingInfo );
	return _BlockingInfo->canSyncNow() && _BlockingInfo->weHaveWriters();
}


/*
 * Unallocate
 */
bool	CChangeTrackerMS::destroy()
{
	if ( isAllocated() && (_GroupTrackerState!=PointingToGroupTracker) )
	{
		// Destroy the shared memory used by the tracker.
		// Client services can still access the segment until they destroy their tracker
		// (see destroySharedMemory() doc)
		if ( ! CSharedMemory::closeSharedMemory( header() ) )
			nlwarning( "Closing shared memory (smid %d) failed", smid() );
		CSharedMemory::destroySharedMemory( toSharedMemId(smid()) );

#ifndef USE_FAST_MUTEX
		// Destroy mutex
		trackerMutex().destroy();
#endif

		nldebug( "Destroyed tracker (smid %d) and mutex", smid() );
		return true;
	}
	else
	{
		if ( isAllocated() )
			nldebug( "Not destroying tracker (smid %d) because %s", smid(), isAllocated()?"not allocated":"pointing to group" );
		return false;
	}
}


/*
 * Display debug info (1 line)
 */
void	CChangeTrackerMS::displayTrackerInfo( const char *headerStr, const char *footerStrNotAllocd, NLMISC::CLog *log ) const
{
	// Cannot reuse CChangeTrackerBase::displayTrackerInfo() because we want the info on one line
	if ( isAllocated() )
		log->displayNL( "%s++Allocated, %d changes, %s %s, smid %d mutid %d%s", headerStr, nbChanges(), isLocal()?"local":(isAllowedToSendToRemoteMS()?"open remote":"pending remote"), servStr(destServiceId()).c_str(), smid(), mutid(), isGroupTracker()?", group tracker":"" );
	else
		log->displayNL( "%s--Not allocated, %s %s%s%s%s", headerStr, isLocal()?"local":"remote", servStr(destServiceId()).c_str(), footerStrNotAllocd, isReadOnly()?", read-only":"", isWriteOnly()?", write-only":"" );

}


/*
 * Display the trackers
 */
void				CDataSetMS::displayTrackers( NLMISC::CLog& log ) const
{
	log.displayNL( "*** Dataset %s ***", name().c_str() );

	log.displayNL( "Pending binding counters: %d", _BindingCountsToSet ? (sint)_BindingCountsToSet->size() : -1 );

	log.displayNL( "Entity trackers:" );
	sint eti;
	for ( eti=ADDING; eti<=REMOVING; ++eti )
	{
		if ( _EntityTrackers[eti].empty() )
		{
			log.displayNL( "\tNo %s tracker", (eti==ADDING)?"adding":"removing" );
		}
		else
		{
			log.displayNL( "\tList of %d %s trackers", _EntityTrackers[eti].size(), (eti==ADDING)?"adding":"removing" );
			TSubscriberListForEnt::const_iterator itl;
			for ( itl=_EntityTrackers[eti].begin(); itl!=_EntityTrackers[eti].end(); ++itl )
			{
				(*itl).displayTrackerInfo( " \t\t", "", &log );
			}
		}
	}

	log.displayNL( "Local writer properties: %u", _NbLocalWriterProps );
	log.displayNL( "Property trackers:" );
	uint32 propIndex;
	for ( propIndex=0; propIndex!=_SubscribersByProperty.size(); ++propIndex )
	{
		if ( ! _SubscribersByProperty[propIndex].empty() )
		{
			log.displayNL( "\tProperty %s (%hd): %d trackers", getPropertyName((TPropertyIndex)propIndex).c_str(), (TPropertyIndex)propIndex, _SubscribersByProperty[propIndex].size() );
			TSubscriberListForProp::const_iterator itl;
			for ( itl=_SubscribersByProperty[propIndex].begin(); itl!=_SubscribersByProperty[propIndex].end(); ++itl )
			{
				(*itl).displayTrackerInfo( " \t\t", "", &log );
			}
		}
	}
	log.displayNL( "End of trackers" );
}


/*
 * Calculate the changes budgets with the current bandwidth limit and property weights
 */
void			CDataSetMS::calculateChangesBudgets()
{
	// Calculate the sum of (weight*size) (TODO: only properties used by this mirror)
	sint sum = 0;
	sint propIndex;
	for ( propIndex=0; propIndex!=(sint)nbProperties(); ++propIndex )
	{
		sum += (_Weights[propIndex] * getDataSizeOfProp( propIndex ));
	}
	if ( sum == 0 )
	{
		nlwarning( "MIRROR: Missing weights in dataset file %s", name().c_str() );
		return;
	}

	// Calculate each budget
	for ( propIndex=0; propIndex!=(sint)nbProperties(); ++propIndex )
	{
		_ChangesBudgets[propIndex] = _Weights[propIndex] * _MaxOutBandwidth / sum;
	}
}


/*
 * Display weights
 */
void			CDataSetMS::displayWeights( NLMISC::CLog& log ) const
{
	log.displayNL( "* Weights for props of dataset %s (max bandwidth %u) *", name().c_str(), maxOutBandwidth() );
	sint propIndex;
	//stringstream s1, s2;
	string str1, str2;
	for ( propIndex=0; propIndex!=(sint)nbProperties(); ++propIndex )
	{
		//s1 << _Weights[propIndex] << " ";
		str1 += NLMISC::toString(_Weights[propIndex]) + " ";
		//s2 << _ChangesBudgets[propIndex] << " ";
		str2 += NLMISC::toString(_ChangesBudgets[propIndex]) + " ";
		if ( propIndex % 3 == 0 )
		{
			str1 += "- ";
//			s1 << "- ";
			str2 += "- ";
//			s2 << "- ";
		}
	}
	log.displayNL( "Weights: %s", str1.c_str() );
	log.displayNL( "Budgets: %s", str2.c_str() );
}


#ifdef COUNT_MIRROR_PROP_CHANGES

/*
 * Return the number of non-local changes for add/remove
 */
sint32			CDataSetMS::getNbChangesForRowManagement() const
{
	sint32 sum = 0;
	sint32 eti;
	for ( eti=REMOVING; eti>=ADDING; --eti )
	{
		TSubscriberListForEnt::const_iterator isl;
		for ( isl=_EntityTrackers[eti].begin(); isl!=_EntityTrackers[eti].end(); ++isl )
		{
			if ( ! (*isl).isLocal() )
				sum += (*isl).getStoredNbChanges();
		}
	}
	return sum;
}


/*
 * Return the number of non-local changes for props
 */
sint32			CDataSetMS::getNbChangesForPropChanges() const
{
	sint32 sum = 0;
	TPropertyIndex propIndex;
	for ( propIndex=0; propIndex!= nbProperties(); ++propIndex )
	{
		// TODO: Skip properties not declared
		TSubscriberListForProp::const_iterator isl;
		for ( isl=_SubscribersByProperty[propIndex].begin(); isl!=_SubscribersByProperty[propIndex].end(); ++isl )
		{
			if ( ! (*isl).isLocal() )
				sum += (*isl).getStoredNbChanges();
		}
	}
	return sum;
}

#endif

/*
 * Empty a list without knowing the type of elements
 */

#define KILL_LIST_TYPE(type_name, type)	case type_name : { CMirrorPropValueListMS<type> list(*this, entityIndex, propIndex ); list.clear(); break; }

void	CDataSetMS::killList( TDataSetRow entityIndex, TPropertyIndex propIndex )
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



#if NL_ISO_SYNTAX

/*
 * Constructor
 */
CMirrorPropValueListMS<NLMISC::CEntityId>::CMirrorPropValueListMS( CDataSetMS& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex )
{
	dataSet.getPropPointerForList( &_PtFront, propIndex, entityIndex, (NLMISC::CEntityId*)NULL );
	_Container = (TSharedListCell*)(dataSet.getListCellContainer( propIndex ));
}


void						CMirrorPropValueListMS<NLMISC::CEntityId>::push_front( const NLMISC::CEntityId& value )
{
  // Ensure the TSharedListCell struct has 1-byte packing
  nlctassert( sizeof(TSharedListRow) + sizeof(uint64) == sizeof(TSharedListCell) );

  TSharedListRow oldfront = *_PtFront;
  TSharedListRow newfront = allocateNewCell();
  if ( newfront != INVALID_SHAREDLIST_ROW )
    {
      _Container[newfront].Next = oldfront;
      _Container[newfront].Value = value.getRawId();
      *_PtFront = newfront;
    }
  else
    nlwarning( "No shared list storage left" );
}

CMirrorPropValueListMS<NLMISC::CEntityId>::size_type				CMirrorPropValueListMS<NLMISC::CEntityId>::size()
{
	size_type s = 0;
	for ( iterator it=begin(); it!=end(); ++it )
	{
		++s;
		if ( s > 100 ) // TEMP workaround
			return ~0;
	}
	return s;
}

void												CMirrorPropValueListMS<NLMISC::CEntityId>::clear()
{
	TSharedListRow row = *_PtFront;
	*_PtFront = INVALID_SHAREDLIST_ROW;
	while ( row != INVALID_SHAREDLIST_ROW )
	{
		TSharedListRow oldrow = row;
		row = _Container[oldrow].Next;
		(*((NLMISC::CEntityId*)(&_Container[oldrow].Value))).~CEntityId();
		releaseCell( oldrow );
		//nldebug( "Release cell" );
	}
}

CMirrorPropValueListMS<NLMISC::CEntityId>::iterator					CMirrorPropValueListMS<NLMISC::CEntityId>::begin()
{
	iterator it;
	it._ParentList = this;
	it._Index = *_PtFront;
	return it;
}

CMirrorPropValueListMS<NLMISC::CEntityId>::iterator					CMirrorPropValueListMS<NLMISC::CEntityId>::end()
{
	iterator it;
	it._ParentList = this;
	it._Index = INVALID_SHAREDLIST_ROW;
	return it;
}


TSharedListRow										CMirrorPropValueListMS<NLMISC::CEntityId>::allocateNewCell()
{
	/*// Old trivial linear browsing code
	TSharedListRow row = 0;
	while ( (_Container[row].Next != UNUSED_SHAREDLIST_CELL) && (row < INVALID_SHAREDLIST_ROW) )
	{
		++row;
	ListCellAllocRow = row;
	}*/

	// New linked-list code
	TSharedListRow *firstFreeCell = TPropertyValueArrayHeader::getFreeCellsFront( (void*)_Container );
	TSharedListRow cellToAllocate = *firstFreeCell;
	if ( cellToAllocate != INVALID_SHAREDLIST_ROW )
		*firstFreeCell = _Container[cellToAllocate].Next;
	return cellToAllocate;
}

void												CMirrorPropValueListMS<NLMISC::CEntityId>::releaseCell( TSharedListRow row )
{
	/*// Old trivial linear browsing code
	_Container[row].Next = UNUSED_SHAREDLIST_CELL;
	*/

	// New linked-list code
	TSharedListRow *firstFreeCell = TPropertyValueArrayHeader::getFreeCellsFront( (void*)_Container );
	_Container[row].Next = *firstFreeCell;
	*firstFreeCell = row;
}

/// Get the value (ugly workaround to the packing limitation of GCC 3.4)
const NLMISC::CEntityId&											CMirrorPropValueItemMS<NLMISC::CEntityId>::operator() () const
{
  uint64 rawId = *_Pt;
  nlctassert( sizeof(NLMISC::CEntityId) == sizeof(uint64) );
  const NLMISC::CEntityId *placementEid = new (_Pt) NLMISC::CEntityId(rawId);
  return *placementEid;
}

/// Set the value
CMirrorPropValueItemMS<NLMISC::CEntityId>&							CMirrorPropValueItemMS<NLMISC::CEntityId>::operator= ( const NLMISC::CEntityId& srcValue )
{
	*_Pt = srcValue.getRawId();
	return *this;
}

#endif
