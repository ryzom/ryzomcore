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
#include "nel/misc/shared_memory.h"
#include "change_tracker_base.h"


/*
 * Create the mutex (either create or use existing)
 */
bool		CChangeTrackerBase::createMutex( sint32 mutid, bool createNew )
{
#ifdef USE_FAST_MUTEX

	bool res = true;
	if ( createNew )
		trackerMutex().init();

#else

#ifdef NL_OS_WINDOWS
	bool res = _TrackerMutex.createByName( NLMISC::toString( "%dM", mutid ).c_str() );
#else
	bool res = _TrackerMutex.createByKey( mutid, createNew );
#endif

#endif
	MIRROR_DEBUG( "MIRROR: TRACKER: Mutex %s (id %d) ", createNew?"created":"accessed", mutid );
	_MutId = mutid;
	return res;
}


/*
 * Record a change
 */
void		CChangeTrackerBase::recordChange( TDataSetIndex entityIndex )
{
	// Note: could be shorter to link backwards but we need to link forward
	// to read the changes in order (for partial sending)
#ifdef NL_DEBUG
	nlassertex( (entityIndex != INVALID_DATASET_INDEX) && (entityIndex != LAST_CHANGED), ("E%d", entityIndex) );
#endif

	// Protect consistency of two parallel calls to recordChange() for the same tracker
	// and between recordChange() and popFirstChanged()
	trackerMutex().enter();

	// Test if the entity is already or not flagged as changed
	if ( _Array[entityIndex].NextChanged==INVALID_DATASET_INDEX )
	{
		//nldebug( "Tracker (smid %u): adding change E%d", smid(), entityIndex );
		// Set the entityIndex as changed, and last one
		_Array[entityIndex].NextChanged = LAST_CHANGED;

		if ( _Header->Last!=INVALID_DATASET_INDEX )
		{
			// Link the previous last one to the new last one
			//nlassertex( _Header->Last != entityIndex, ( "Looping at %u", entityIndex ) );
			_Array[_Header->Last].NextChanged = entityIndex;
			//nldebug( "Tracker %p: E%d -> %d", _Array, _Header->Last, entityIndex );
		}
		else
		{
			// Set the first one if not set
			_Header->First = entityIndex;
			//nldebug( "Tracker %p: FIRST = %d", _Array, entityIndex );
			//nldebug( "_Array = %p, _Array[%d].NextChanged = %d", _Array, entityIndex, _Array[entityIndex].NextChanged );
		}

		// Set the new last one
		_Header->Last = entityIndex;

#ifdef COUNT_MIRROR_PROP_CHANGES
		++_Header->NbDistinctChanges;
#endif
	}
	//else
	//	nldebug( "Tracker %p: E%d already in list (%d)", _Array, entityIndex, _Array[entityIndex].NextChanged );

#ifdef COUNT_MIRROR_PROP_CHANGES
	//++_Header->NbValuesSet; // should be atomic!
#endif
		//nldebug( "Tracker (smid %u): E%d already in list (pointing to %d)", smid(), entityIndex, _Array[entityIndex].NextChanged );
	trackerMutex().leave();

}


/*
 * Remove a change (slow) (assumes isAllocated())
 */
void		CChangeTrackerBase::cancelChange( TDataSetIndex entityIndex )
{
#ifdef NL_DEBUG
	nlassertex( (entityIndex != INVALID_DATASET_INDEX) && (entityIndex != LAST_CHANGED), ("E%d", entityIndex) );
#endif

	trackerMutex().enter();

	// Find the change before the specified one, to make the link skip it
	TDataSetIndex row = _Header->First;
	if ( row != LAST_CHANGED )
	{
		if ( row == entityIndex )
		{
			// It was the first one
			_Header->First = _Array[entityIndex].NextChanged;
			_Array[entityIndex].NextChanged = INVALID_DATASET_INDEX;
			if ( _Header->First == LAST_CHANGED ) // and it was the only one
				_Header->Last = INVALID_DATASET_INDEX;
			nldebug( "Cancelling change of E%u", entityIndex );
		}
		else
		{
			// It wasn't the first one
			while ( (_Array[row].NextChanged != entityIndex) && (_Array[row].NextChanged != LAST_CHANGED) )
			{
				row = _Array[row].NextChanged;
			}
			if ( _Array[row].NextChanged == entityIndex )
			{
				_Array[row].NextChanged = _Array[entityIndex].NextChanged;
				_Array[entityIndex].NextChanged = INVALID_DATASET_INDEX;
				if ( _Header->Last == entityIndex ) // it was the last one
					_Header->Last = row;
				nldebug( "Cancelling change of E%u", entityIndex );
			}
		}
	}
	trackerMutex().leave();
}


/*
 * Return the number of changes (slow)
 */
sint32		CChangeTrackerBase::nbChanges() const
{
	sint32 nb = 0;
	TDataSetRow	entityIndex	(getFirstChanged());
	while ( entityIndex.getIndex() != LAST_CHANGED )
	{
		++nb;
		entityIndex = getNextChanged( entityIndex );
	}
	return nb;
}


/*
 * Display debug info (1 line)
 */
void		CChangeTrackerBase::displayTrackerInfo( const char *headerStr, const char *footerStrNotAllocd, NLMISC::CLog *log ) const
{
	if ( isAllocated() )
		log->displayNL( "%sAllocated, %d changes, smid %d mutid %d", headerStr, nbChanges(), smid(), mutid() );
	else
		log->displayNL( "%sNot allocated%s", headerStr, footerStrNotAllocd );
}
