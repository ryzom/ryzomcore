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

#ifndef NL_CONTIGUOUS_BLOCK_ALLOCATOR_H
#define NL_CONTIGUOUS_BLOCK_ALLOCATOR_H

#include "types_nl.h"

namespace NLMISC
{

/** One of the simplest scheme of allocation around, but very useful in some situations.
  * This allocator is just provided with a single block of memory at start (possibly of size 0). Each alloc get a new block in that big block, by
  * simply advancing a pointer. When not enough space is available, the default stl allocator is used.
  * When a block is deallocated, nothing happens, unless the block was allocated using the default stl allocator is used, in which case deallocate() is called.
  *
  * The typical use is when an object makes a lot of allocations at init, but in a predictable way, and if it doesn't make alloc / realloc
  * later. In this case the caller can measure the amount of memory needed to create the object, and can create this allocator with the good amount of
  * memory. Subsequent allocations will then be very fast even for very differently sized blocks, with no fragmentation inside the allocated block
  * and no memory overhead per allocated bloc.
  *
  * Obviously, if the quantity of memory to be allocated can't be predicted (or if no max bytes can be forseen), then other allocators may
  * be best suited.
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  *
  */
class CContiguousBlockAllocator
{
public:
	// ctor
	CContiguousBlockAllocator();
	// dtor
	~CContiguousBlockAllocator();
	// Init the allocator with the given size. Previous allocations become invalid, so when calling init again, the user must have
    // freed all memory he allocated
	void init(uint numBytes = 0);
	// synonymous to init(0)
	void release() { init(0); }
	// allocated a block of n bytes
	void *alloc(uint numBytes);
	// deallocate a block
	void freeBlock(void *block, uint numBytes);
	// compute the total number of bytes allocated since init
	// NB : freed block are not subtracted from that total !!
	uint getNumAllocatedBytes() const { return _NumAllocatedBytes; }
	#ifdef NL_DEBUG
	    // get number of calls to alloc since last init
		uint getNumAlloc() const { return _NumAlloc; }
		// get number of calls to free since last init
		uint getNumFree() const { return _NumFree; }
	#endif
private:
	uint8					*_BlockStart;
	uint8					*_BlockEnd;
	uint8					*_NextAvailablePos;
	uint					 _NumAllocatedBytes;
	std::allocator<uint8>    _DefaultAlloc;

	#ifdef NL_DEBUG
		uint _NumAlloc;
		uint _NumFree;
	#endif
};

}

#endif
