// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdmisc.h"

#include "nel/misc/shared_memory.h"
#include "nel/misc/debug.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#else
#	include <sys/types.h>
#	include <sys/ipc.h>
#	include <sys/shm.h>
#endif

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {


#ifdef NL_OS_WINDOWS
	// Storage for file handles, necessary to close the handles
	map<void*,HANDLE>	AccessAddressesToHandles;
#else
	// Storage for shmid, necessary to destroy the segments
	map<TSharedMemId, sint>   SharedMemIdsToShmids;
#endif


/*
 * Create a shared memory segment
 */
void			*CSharedMemory::createSharedMemory( TSharedMemId sharedMemId, uint32 size )
{
#ifdef NL_OS_WINDOWS

	// Create a file mapping backed by the virtual memory swap file (not a data file)
	HANDLE hMapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, sharedMemId );
	if ( (hMapFile == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS) )
	{
		nlwarning( "SHDMEM: Cannot create file mapping for smid %s: error %u%s, mapFile %p", sharedMemId, GetLastError(), (GetLastError()==ERROR_ALREADY_EXISTS) ? " (already exists) ": "", hMapFile );
		return NULL;
	}
	//else
	//	nldebug( "SHDMEM: Creating smid %s --> mapFile %p", sharedMemId, hMapFile );


	// Map the file into memory address space
	void *accessAddress = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	AccessAddressesToHandles.insert( make_pair( accessAddress, hMapFile ) );
	/*if ( accessAddress == NULL )
	{
		nlwarning( "SHDMEM: Cannot map view of file: error %u", GetLastError() );
	}*/
	return accessAddress;

#else

	// Create a shared memory segment
	sint shmid = shmget( sharedMemId, size, IPC_CREAT | IPC_EXCL | 0666 );
	if ( shmid == -1 )
		return NULL;
	SharedMemIdsToShmids.insert( make_pair( sharedMemId, shmid ) );

	// Map the segment into memory address space
	void *accessAddress = (void*)shmat( shmid, 0, 0 );
	if ( accessAddress == (void*)-1 )
		return NULL;
	else
		return accessAddress;

#endif
}


/*
 * Get access to an existing shared memory segment
 */
void			*CSharedMemory::accessSharedMemory( TSharedMemId sharedMemId )
{
#ifdef NL_OS_WINDOWS

	// Open the existing file mapping by name
	HANDLE hMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, false, sharedMemId );
	if ( hMapFile == NULL )
		return NULL;
	//nldebug( "SHDMEM: Opening smid %s --> mapFile %p", sharedMemId, hMapFile );

	// Map the file into memory address space
	void *accessAddress = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	AccessAddressesToHandles.insert( make_pair( accessAddress, hMapFile ) );
	return accessAddress;

#else

	// Open an existing shared memory segment
	int shmid = shmget( sharedMemId, 0, 0666 );
	if ( shmid == -1 )
		return NULL;

	// Map the segment into memory address space
	void *accessAddress = (void*)shmat( shmid, 0, 0 );
	if ( accessAddress == (void*)-1 )
		return NULL;
	else
		return accessAddress;

#endif
}


/*
 * Close (detach) a shared memory segment
 */
bool			CSharedMemory::closeSharedMemory( void *accessAddress )
{
#ifdef NL_OS_WINDOWS

	bool result = true;

	// Unmap the file from memory address space
	if ( UnmapViewOfFile( accessAddress ) == 0 )
	{
		nlwarning( "SHDMEM: UnmapViewOfFile failed: error %u", GetLastError() );
		result = false;
	}

	// Close the corresponding handle
	map<void*,HANDLE>::iterator im = AccessAddressesToHandles.find( accessAddress );
	if ( im != AccessAddressesToHandles.end() )
	{
		//nldebug( "SHDMEM: CloseHandle mapFile %u", (*im).second );
		if ( ! CloseHandle( (*im).second ) )
			nlwarning( "SHDMEM: CloseHandle failed: error %u, mapFile %u", GetLastError(), (*im).second );
		AccessAddressesToHandles.erase( im );
		return result;
	}
	else
	{
		return false;
	}

#else

	// Detach the shared memory segment
	return ( shmdt( accessAddress ) != -1 );

#endif
}


/*
 * Destroy a shared memory segment (to call only by the process that created the segment)
 * Note: does nothing under Windows, it is automatic.
 * "Rescue feature": set "force" to true if a segment was created and left out of
 * control (meaning a new createSharedMemory() with the same sharedMemId fails), but
 * before, make sure the segment really belongs to you!
 *
 * Note: this method does nothing under Windows, destroying is automatic.
 * Under Unix, the segment will actually be destroyed after the last detach
 * (quoting shmctl man page). It means after calling destroySharedMemory(), the
 * segment is still accessible by another process until it calls closeSharedMemory().
 */
void        CSharedMemory::destroySharedMemory( TSharedMemId sharedMemId, bool force )
{
#ifndef NL_OS_WINDOWS
  // Set the segment to auto-destroying (when the last process detaches)
  map<TSharedMemId,int>::iterator im = SharedMemIdsToShmids.find( sharedMemId );
  if ( im != SharedMemIdsToShmids.end() )
	{
	  // Destroy the segment created before
	  shmctl( (*im).second, IPC_RMID, 0 );
	  SharedMemIdsToShmids.erase( im );
	}
  else if ( force )
	{
	  // Open and destroy the segment
	  int shmid = shmget( sharedMemId, 0, 0666 );
	  if ( shmid != -1 )
		{
		  // Destroy the segment
		  shmctl( shmid, IPC_RMID, 0 );
		}
	}
#endif
}


} // NLMISC
