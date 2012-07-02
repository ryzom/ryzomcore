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



#ifndef NL_CHANGE_TRACKER_BASE_H
#define NL_CHANGE_TRACKER_BASE_H

#include "nel/misc/types_nl.h"
#include "base_types.h"
#include <nel/misc/hierarchical_timer.h> // TEMP


//#ifdef NL_OS_WINDOWS
#define USE_FAST_MUTEX
//#endif

/*
 * Set this define for stats about the property changes (slower)
 * Important note: all the user services and the mirror service should have the same value!
 */
#define COUNT_MIRROR_PROP_CHANGES


/**
 * Header of a tracker
 */
struct TChangeTrackerHeader
{
	/** First changed item (useful to read the changed from the beginning).
	 * When there is no change to read, First equals LAST_CHANGED.
	 */
	volatile TDataSetIndex		First;

	/** Last changed item (useful to link the last changed to a newly changed item).
	 * When there is no change yet, Last equals INVALID_DATASET_ROW.
	 */
	volatile TDataSetIndex		Last;

#ifdef USE_FAST_MUTEX
	/// Fast mutex (TODO: use multi-processor version)
  volatile NLMISC::CFastMutex	FastMutex;
#endif
	/*
	 * Number of values set (used in COUNT_MIRROR_CHANGES mode only, always allocated for mode interoperability)
	 * Currently, not implemented.
	 */
	//volatile sint32				NbValuesSet;

	/**
	 * Number of changes really recorded (used in COUNT_MIRROR_CHANGES mode only, always allocated for mode interoperability).
	 * NbValuesSet-NbDistinctChanges is the number of skipped changes.
	 */
	volatile sint32					NbDistinctChanges;
};


const uint16 LOCAL_TRACKER_SERVICE_ID = (uint16)~0;

/**
 * Item in a tracker
 */
struct TChangeTrackerItem
{
	/**
	 * If the item state is 'unchanged', NextChanged is INVALID_DATASET_ROW.
	 * If it is 'changed', NextChanged is either the index of the next changed
	 * item or the value LAST_CHANGED.
	 */
	TDataSetIndex				NextChanged;
};



/**
 * Base class for change tracker
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CChangeTrackerBase
{
public:

	/// Constructor
	CChangeTrackerBase() : _SMId(-1), _MutId(-1), _Header(NULL), _Array(NULL) {}

	/// Assignment operator
	CChangeTrackerBase&		operator = ( const CChangeTrackerBase& src )
	{
		if ( &src == this )
			return *this;

		_SMId = src._SMId;
#ifndef USE_FAST_MUTEX
		_TrackerMutex = src._TrackerMutex; // contains only the mutex handle
#endif
		_MutId = src._MutId;
		_Header = src._Header; // leave the pointed data where they are (in shared memory)
		_Array = src._Array; // same
		return *this;
	}

	/// Return the shared memory id
	const sint32&			smid() const { return _SMId; }

	/// Return true if the tracker header and item array are already allocated
	bool					isAllocated() const
							{ return _Array != NULL; }

	/// Return the pointer to the header (root of the shared memory segment)
	TChangeTrackerHeader	*header() { return _Header; }

	/// Record a change (push) (assumes isAllocated())
	void					recordChange( TDataSetIndex entityIndex );

	/// Remove a change if found in the tracker (slow) (assumes isAllocated())
	void					cancelChange( TDataSetIndex entityIndex );

	/// Get the entity index of the first changed (assumes isAllocated()). Returns LAST_CHANGED if there is no change.
	uint32					getFirstChanged() const { /*nlinfo( "Array = %p, First = %d, _Array[First].NextChanged = %d, _Array[0].NextChanged = %d", _Array, _Header->First, _Array[_Header->First].NextChanged, _Array[0].NextChanged );*/ return _Header->First; }

	/// Pop the first change out of the tracker. Do not call if getFirstChanged() returned LAST_CHANGED.
	void					popFirstChanged()
	{
		// Protect consistency of popFirstChanged() in parallel with recordChange()
		// (there can't be two parallels calls to popFirstChanged()
		trackerMutex().enter();
#ifdef NL_DEBUG
		nlassert( _Header->First != LAST_CHANGED );
#endif
		TChangeTrackerItem *queueFront = &(_Array[_Header->First]);
		_Header->First = queueFront->NextChanged;
		queueFront->NextChanged = INVALID_DATASET_INDEX;
		if ( _Header->First == LAST_CHANGED )
		{
			_Header->Last = INVALID_DATASET_INDEX;
		}
#ifdef COUNT_MIRROR_PROP_CHANGES
		--_Header->NbDistinctChanges;
#endif
		trackerMutex().leave();
	}

	/// Return the number of changes (assumes isAllocated()) (slow)
	sint32					nbChanges() const;

#ifdef USE_FAST_MUTEX
	/// Return the mutex
	volatile NLMISC::CFastMutex&	trackerMutex() { return _Header->FastMutex; }
#else
	/// Return the mutex
	NLMISC::CSharedMutex&	trackerMutex() { return _TrackerMutex; }
#endif

	/// Return the mutex id
	const sint32&			mutid() const { return _MutId; }

	/// Create the mutex (either create or use existing)
	bool					createMutex( sint32 mutid, bool createNew );

	/// Display debug info (1 line)
	void					displayTrackerInfo( const char *headerStr="", const char *footerStrNotAllocd="", NLMISC::CLog *log=NLMISC::InfoLog ) const;

	/// Serial ids
	void					serial( NLMISC::IStream& s )
	{
		s.serial( _SMId );
		s.serial( _MutId );
	}

protected:

	/// Get the entity index of the next changed (assumes isAllocated() and entityIndex is valid). Returns LAST_CHANGED if there is no more change.
	TDataSetRow				getNextChanged( const TDataSetRow& entityIndex ) const { /*nlinfo( "_Array[%d].NextChanged = %d", entityIndex, _Array[entityIndex].NextChanged );*/ return TDataSetRow(_Array[entityIndex.getIndex()].NextChanged); }

	/// Shared memory numeric id
	sint32					_SMId;

#ifndef USE_FAST_MUTEX
	/// Mutex
	NLMISC::CSharedMutex	_TrackerMutex;
#endif

	/// Mutex id
	sint32					_MutId;

	/// Pointer to the handling variables of the tracker (may be pointing to shared memory)
	TChangeTrackerHeader	*_Header;

	/// Pointer to the array of entities, indexed by TDataSetRow (may be pointing to shared memory)
	TChangeTrackerItem		*_Array;
};


#endif // NL_CHANGE_TRACKER_BASE_H

/* End of change_tracker_base.h */





















