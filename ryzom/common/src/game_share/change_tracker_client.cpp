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
#include "change_tracker_client.h"

using namespace NLMISC;


/*
 * Access the tracker header and item array (shared memory)
 */
void	CChangeTrackerClient::access( sint32 shmid )
{
	_SMId = shmid;

	_Header = (TChangeTrackerHeader*)CSharedMemory::accessSharedMemory( toSharedMemId(_SMId) );
	if ( ! _Header )
	{
		// This can occur when a service disconnects from the MS when between the moment when
		// the MS sent the "add trackers" message and the moment when we received and applied it:
		// the MS deletes the tracker, thus the shared memory is immediatel destroyed as we did not
		// accessed it yet.
		nlwarning( "MIRROR: Cannot access shared memory for tracker" );
		return;
	}

	_Array = (TChangeTrackerItem*)(((uint8*)_Header) + sizeof( TChangeTrackerHeader ));
	//nlinfo( "Setting _Array = %p + d% = %p", _Header, sizeof( TChangeTrackerHeader ), _Array );
}



#if 0
/*
 * Set all entities as unchanged
 */
void		CChangeTrackerClient::clean()
{
	// Clear the changed rows until the last one that was notified in the current cycle
	// (do not clear the ones that could have been added (by a parallel recordChange())
	// since the latest call to notifyNext()

	trackerMutex().enter();

	TDataSetRow entityIndex = getFirstChanged(), entityIndex2;
#ifdef NL_DEBUG
	//nlassert( entityIndex.isValid() );
#endif
	/*if ( entityIndex == LAST_CHANGED )
		return;*/
	/////////nldebug( "Cleaning tracker (smid %u): first changed=E%d lastNotified=%d", smid(), entityIndex, lastNotifiedRow() );
	while ( entityIndex != lastNotifiedRow() )
	{
		// Clear a row and get next one
		entityIndex2 = entityIndex;
		entityIndex = getNextChanged( entityIndex );
#ifdef NL_DEBUG
	//nlassert( entityIndex.isValid() );
#endif
		_Array[entityIndex2].NextChanged = TDataSetRow();
#ifdef COUNT_MIRROR_PROP_CHANGES
		--_Header->NbDistinctChanges;
#endif
		/////////nldebug( "Tracker (smid %u): E%d cleaned, next is %d", smid(), entityIndex2, entityIndex );
	}

	// Protect the consistency between clean() and recordChange()

	// Set the new entry point of the queue
	if ( entityIndex != LAST_CHANGED )
	{
		_Header->First = _Array[entityIndex].NextChanged;
		_Array[entityIndex].NextChanged = TDataSetRow();
#ifdef COUNT_MIRROR_PROP_CHANGES
		--_Header->NbDistinctChanges;
#endif
	}
	else
	{
		_Header->First = LAST_CHANGED;
	}

	// If there are no more changed rows, clear the exit point of the queue
	if ( _Header->First == LAST_CHANGED )
	{
		_Header->Last = TDataSetRow();
	}

	trackerMutex().leave();
}
#endif


/*
 * Release the tracker (unaccess shared memory + release mutex)
 */
void		CChangeTrackerClient::release()
{
	if ( (_Header != NULL) && (! isPointingToGroupTracker()) )
	{
		NLMISC::CSharedMemory::closeSharedMemory( _Header );
		_Header = NULL;
		_Array = NULL;
		_SMId = -1;

#ifndef USE_FAST_MUTEX
#ifdef NL_OS_WINDOWS
		trackerMutex().destroy();
#endif
#endif
	}
}

