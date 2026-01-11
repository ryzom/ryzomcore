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

#include "nel/misc/contiguous_block_allocator.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// *********************************************************************************************************
CContiguousBlockAllocator::CContiguousBlockAllocator()
{
	_BlockStart = NULL;
	_NextAvailablePos = NULL;
	_BlockEnd = 0;
	_NumAllocatedBytes = 0;
	#ifdef NL_DEBUG
		_NumAlloc = 0;
		_NumFree = 0;
	#endif
}

// *********************************************************************************************************
CContiguousBlockAllocator::~CContiguousBlockAllocator()
{
	init(0);
}

// *********************************************************************************************************
void CContiguousBlockAllocator::init(uint numBytes /*=0*/)
{
	if (_BlockStart) _DefaultAlloc.deallocate(_BlockStart, _BlockEnd - _BlockStart);
	_BlockEnd =  NULL;
	_BlockStart = NULL;
	_NumAllocatedBytes = 0;
	_NextAvailablePos = NULL;
	if (numBytes != 0)
	{
		_BlockStart = _DefaultAlloc.allocate(numBytes);
		_NextAvailablePos = _BlockStart;
		_BlockEnd = _BlockStart + numBytes;
		_NumAllocatedBytes = 0;
	}
	#ifdef NL_DEBUG
		_NumAlloc = 0;
		_NumFree = 0;
	#endif
}

// *********************************************************************************************************
void *CContiguousBlockAllocator::alloc(uint numBytes)
{
	if (numBytes == 0) return NULL;
	_NumAllocatedBytes += numBytes;
	if (_BlockStart)
	{
		if (_NextAvailablePos + numBytes <= _BlockEnd)
		{
			uint8 *block = _NextAvailablePos;
			_NextAvailablePos += numBytes;
			#ifdef NL_DEBUG
				++ _NumAlloc;
			#endif
			return block;
		}
	}
	// just uses standard new
	#ifdef NL_DEBUG
		++ _NumAlloc;
	#endif
	return _DefaultAlloc.allocate(numBytes);
}

// *********************************************************************************************************
void CContiguousBlockAllocator::freeBlock(void *block, uint numBytes)
{
	if (!block) return;
	#ifdef NL_DEBUG
		++ _NumFree;
	#endif
	// no-op if block not inside the big block (sub-block are never deallocated until init(0) is encountered)
	if (block < _BlockStart || block >= _BlockEnd)
	{
		// the block was allocated with std allocator
		_DefaultAlloc.deallocate((uint8 *) block, numBytes);
	}
}

} // NLMISC
