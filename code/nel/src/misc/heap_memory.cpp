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

#include "nel/misc/heap_memory.h"
#include "nel/misc/debug.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{


// ***************************************************************************
CHeapMemory::CHeapMemory()
{
	reset();
	// For allocate to work even if the heap is not initialized.
	_Alignment= 4;
}
// ***************************************************************************
CHeapMemory::~CHeapMemory()
{
	reset();
}


// ***************************************************************************
void			CHeapMemory::reset()
{
	_EmptySpaces.clear();
	_EmptySpaceMap.clear();
	_AllocatedSpaceMap.clear();
	_HeapPtr= NULL;
	_HeapSize= 0;
	_HeapSizeUsed= 0;
}


// ***************************************************************************
void			CHeapMemory::initHeap(void *heap, uint size, uint align)
{
	// setup alignement.
	if(align!=4 && align!=8 && align!=16 && align!=32)
	{
		nlstop;
		align= 4;
	}
	_Alignment= align;

	// Manage alignement.
	size= (size) & (~(_Alignment-1));

	// clear container.
	reset();
	if(heap==0 || size==0)
		return;

	_HeapPtr= (uint8*)heap;
	_HeapSize= size;
	_HeapSizeUsed= 0;

	// Add the only one empty space.
	CEmptySpace		space;
	space.Ptr= _HeapPtr;
	space.Size= _HeapSize;

	addEmptySpace(space);
}


// ***************************************************************************
void		CHeapMemory::removeEmptySpace(CEmptySpace &space)
{
	// remove the iterator on the spaceMap.
	_EmptySpaceMap.erase( space.SizeIt );

	// remove from the list of EmptySpaces. NB: must fo it after all, because "space" may be deleted.
	_EmptySpaces.erase( space.Ptr );
}


// ***************************************************************************
void		CHeapMemory::addEmptySpace(CEmptySpace &space)
{
	// insert and get the iterator on the spaceMap.
	space.SizeIt= _EmptySpaceMap.insert( make_pair(space.Size, space.Ptr));

	// insert into the list of EmptySpaces.
	_EmptySpaces.insert( make_pair(space.Ptr, space) );
}


// ***************************************************************************
void			*CHeapMemory::allocate(uint size)
{
	if(size==0)
		return NULL;

	// Manage alignement.
	size= (size + (_Alignment-1)) & (~(_Alignment-1));


	// retrieve the best block.
	//=========================
	CEmptySpace		bestSpace;
	// NB: do a copy, because of removeEmptySpace() which delete the space.

	// Find the smaller space which is >= than size.
	ItEmptySpaceSizeMap		it;
	it= _EmptySpaceMap.lower_bound(size);

	// if not found, alloc fails.
	if(it == _EmptySpaceMap.end())
		return NULL;
	else
	{
		// NB: this space must exist in the "array".
		bestSpace= _EmptySpaces[it->second];
	}


	// remove this empty space from list.
	//=========================
	removeEmptySpace(bestSpace);


	// if any, add the space unused to the list.
	//=========================
	if(bestSpace.Size > size)
	{
		CEmptySpace		space;
		space.Ptr= bestSpace.Ptr + size;
		space.Size= bestSpace.Size - size;

		addEmptySpace(space);
	}


	// return / insert the allocated space.
	//=========================
	_AllocatedSpaceMap.insert(make_pair(bestSpace.Ptr, size));
	_HeapSizeUsed+= size;

	// return the ptr of start of this empty space.
	return bestSpace.Ptr;
}

// ***************************************************************************
void			CHeapMemory::free(void *ptr)
{
	if(ptr==NULL)
		return;

	// Must find the array in allocated spaces.
	//==========================
	ItAllocatedSpaceMap		itAlloc= _AllocatedSpaceMap.find((uint8*)ptr);
	if(itAlloc == _AllocatedSpaceMap.end())
	{
		nlstop;
		return;
	}
	uint	size= itAlloc->second;

	// free this space from allocated Spaces.
	_AllocatedSpaceMap.erase(itAlloc);
	_HeapSizeUsed-= size;


	// Must find previous or/and next empty space, if any.
	//==========================
	ItEmptySpacePtrMap		itPrevious, itNext;

	// find the empty space which is immediately >= than ptr.
	itNext= _EmptySpaces.lower_bound((uint8*)ptr);
	// NB: it may be end(), if it is the last block (very rare).

	// some check. next empty space ptr must be after ptr.
	if(itNext!=_EmptySpaces.end())
	{
		nlassert(itNext->second.Ptr >= (uint8*)ptr + size);
	}

	// if itNext is not the first empty space, there is an empty space before us.
	if( itNext!= _EmptySpaces.begin() )
	{
		// NB: work even if itNext==end().
		itPrevious= itNext;
		itPrevious--;
		// some check. previous empty space ptr must be before ptr.
		nlassert(itPrevious!=_EmptySpaces.end());
		nlassert(itPrevious->second.Ptr + itPrevious->second.Size <= (uint8*)ptr );
	}
	else
		itPrevious= _EmptySpaces.end();


	// if next exist.
	if(itNext!=_EmptySpaces.end())
	{
		// If Previous is not just after allocated ptr, it means that there is some allocated blocks beetween,
		// so it is not a valid empty space to concat.
		if(itNext->second.Ptr != (uint8*)ptr + size)
			itNext= _EmptySpaces.end();
	}
	// if previous exist.
	if(itPrevious!=_EmptySpaces.end())
	{
		// If Previous is not just before allocated ptr, it means that there is some allocated blocks beetween,
		// so it is not a valid empty space to concat.
		if(itPrevious->second.Ptr + itPrevious->second.Size != (uint8*)ptr )
			itPrevious=_EmptySpaces.end();
	}



	// According to configuration, build the new empty space, mreging previous and next, and remove old ones.
	//==========================
	CEmptySpace		newSpace;

	// if no previous empty space, then newSpace start at ptr.
	if(itPrevious == _EmptySpaces.end())
	{
		// Start with old allocated block.
		newSpace.Ptr= (uint8*)ptr;
		newSpace.Size= size;
	}
	// else, start at previous Ptr.
	else
	{
		// Start with previous block. size is previous size + allocated block size.
		newSpace.Ptr= itPrevious->second.Ptr;
		newSpace.Size= itPrevious->second.Size + size;
	}

	// if next empty space, must inc size.
	if(itNext != _EmptySpaces.end())
	{
		newSpace.Size+= itNext->second.Size;
	}


	// remove old empty space, and add new one.
	//==========================

	// remove old empty spaces.
	if(itPrevious != _EmptySpaces.end())
		removeEmptySpace(itPrevious->second);
	if(itNext != _EmptySpaces.end())
		removeEmptySpace(itNext->second);


	// Add the new concatenated empty space.
	addEmptySpace(newSpace);
}



} // NLMISC
