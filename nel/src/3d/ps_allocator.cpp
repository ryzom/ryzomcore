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

#include "std3d.h"
#include "nel/3d/particle_system_process.h"
#include "nel/3d/ps_allocator.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{
	#ifdef PS_FAST_ALLOC
		uint NumPSAlloc = 0;
		uint NumDealloc = 0;
		NLMISC::CContiguousBlockAllocator *PSBlockAllocator= NULL;
		static std::allocator<uint8> PSStdAllocator;
		//
		typedef NLMISC::CContiguousBlockAllocator *TBlocAllocPtr;
		//
		struct CPSAllocInfo
		{
			size_t			NumAllocatedBytes;
			TBlocAllocPtr   BlocAllocator;  // may be NULL if was allocated from stl allocator
		};
		//
		void *PSFastMemAlloc(uint numBytes)
		{
			NL_PS_FUNC(PSFastMemAlloc)
			CPSAllocInfo *result;
			// if a block allocator is available, use it
			if (PSBlockAllocator)
			{
				result = (CPSAllocInfo *) PSBlockAllocator->alloc(numBytes + sizeof(CPSAllocInfo));
				result->BlocAllocator = PSBlockAllocator; // mark as a block from block allocator
			}
			else
			{
				result = (CPSAllocInfo *) PSStdAllocator.allocate(numBytes + sizeof(CPSAllocInfo));
				result->BlocAllocator = NULL;
			}
			result->NumAllocatedBytes = numBytes;
			return (void *) (result + 1); // usable space starts after header
		}

		void PSFastMemFree(void *block)
		{
			NL_PS_FUNC(PSFastMemFree)
			uint8 *realAddress = (uint8 *) ((uint8 *) block - sizeof(CPSAllocInfo));
			CPSAllocInfo *ai = (CPSAllocInfo *) realAddress;
			if (ai->BlocAllocator)
			{
				// block comes from a block allocator
				ai->BlocAllocator->free((void *) realAddress, ai->NumAllocatedBytes + sizeof(CPSAllocInfo));
			}
			else
			{
				// block comes from the stl allocator
				PSStdAllocator.deallocate((uint8 *) realAddress, ai->NumAllocatedBytes + sizeof(CPSAllocInfo));
			}
		}
	#endif
}
