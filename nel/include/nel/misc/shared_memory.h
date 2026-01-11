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

#ifndef NL_SHARED_MEMORY_H
#define NL_SHARED_MEMORY_H

#include "types_nl.h"

#ifdef NL_OS_WINDOWS
#include <map>
#endif

namespace NLMISC {


#ifdef NL_OS_WINDOWS
typedef const char *TSharedMemId;
#else
typedef key_t TSharedMemId;
#endif


/* Helpers:
 * toSharedMemId: converts an integer to TSharedMemId
 * NL_SMID: format type used for printf syntax. Ex: nldebug( "Segment %"NL_SMID" was created", sharedMemId );
 */
#ifdef NL_OS_WINDOWS
#define toSharedMemId( id ) toString( "NeLSM_%d", id ).c_str()
#define NL_SMID "s"
#else
#define toSharedMemId( id ) (id)
#define NL_SMID "d"
#endif


/**
 * Encapsulation of shared memory APIs.
 * Using file mapping under Windows, System V shared memory (shm) under Linux.
 *
 * Note: under Linux, an option could be added to prevent a segment to be swapped out.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CSharedMemory
{
public:

	// Constructor
	//CSharedMemory();

	/**
	 * Create a shared memory segment and get access to it. The id must not be used.
	 * The id 0x3a732235 is reserved by the NeL memory manager.
	 * \return Access address of the segment of the choosen size
	 */
	static void *		createSharedMemory( TSharedMemId sharedMemId, uint32 size );

	/**
	 * Get access to an existing shared memory segment. The id must be used.
	 * \return Access address of the segment. Its size was determined at the creation.
	 */
	static void *		accessSharedMemory( TSharedMemId sharedMemId );

	/**
	 * Close (detach) a shared memory segment, given the address returned by createSharedMemory()
	 * or accessSharedMemory(). Must be called by each process that called createSharedMemory()
	 * or accessSharedMemory().
	 */
	static bool			closeSharedMemory( void * accessAddress );

  /**
   * Destroy a shared memory segment (must be called by the process that created the segment,
   * not by the accessors).
   *
   * "Rescue feature": set "force" to true if a segment was created and left out of
   * control (meaning a new createSharedMemory() with the same sharedMemId fails), but
   * before, make sure the segment really belongs to you!
   *
   * Note: this method does nothing under Windows, destroying is automatic.
   * Under Unix, the segment will actually be destroyed after the last detach
   * (quoting shmctl man page). It means after calling destroySharedMemory(), the
   * segment is still accessible by another process until it calls closeSharedMemory().
   */
  static void destroySharedMemory( TSharedMemId sharedMemId, bool force=false );
};


} // NLMISC


#endif // NL_SHARED_MEMORY_H

/* End of shared_memory.h */
