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
#include <vector>
#include <signal.h>
#include <setjmp.h>
#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/variable.h"
#include "stl_allocator_checker.h"

bool EnableStlAllocatorChecker= true;
NLMISC_VARIABLE(bool,EnableStlAllocatorChecker,"Enable stl allocator tests");

uint32 StlAllocatorMaxFree= 0;
NLMISC_VARIABLE(uint32,StlAllocatorMaxFree,"When EnableStlAllocatorChecker is true, this value gives the largest number of free blocks encountered");

// setup a 'max iterations' value of 3GBytes/ sizeof(uint32*)
// => this is equivalent to the total addressable memory space under linux
static const uint32 MaxIterations= 768*1024*1024;

// the following static vector exists only for the use of the testStlMemoryAllocator() routine
// - it is required to allow us to get hold of the stl small block memory allocator
static std::vector<int> StaticIntVector;

static bool IsCrashed= false;
static jmp_buf Context;

const char* StlMemoryAllocatorCrashPoint= NULL;

static void sigHandler(int sig)
{
    longjmp(Context, sig);
}

void testStlMemoryAllocator(const char* state)
{
	signal(SIGSEGV, sigHandler);
	if (IsCrashed) return;

	// setup a pointer 'p' to the first block in the allocator's linked list of free blocks
	std::vector<uint32>::allocator_type allocator= StaticIntVector.get_allocator();
	uint32 *p;
	p= allocator.allocate(1);
	allocator.deallocate(p,1);

	// setup a counter to 3GBytes/ sizeof(uint32*) => equivalent to the total addressable memory space under linux
	uint32 counter= MaxIterations;

    if (setjmp(Context) == 0)
    {
		do 
		{
			// step forwards allong the linked list
			p= (uint32*)*p;

			// if the counter hits zero then we can assume that we're in an infinite loop
			if (--counter==0)
				break;
		}
		while (p!=NULL);

		// if we hit a NULL end of list terminator then return happily
		if (p==NULL)
		{
			uint32 numIterations= MaxIterations- counter;
			StlAllocatorMaxFree= std::max(numIterations,StlAllocatorMaxFree);
			signal(SIGSEGV, NULL);
			return;
		}

		// if we're here it's because there's been a problem
		// note that our memory allocators contain invalid data so any call to 'nlassert' etc may modify
		// data thta they shouldn't and make our debugging task harder
		// ... so just provoke an access violation
		*(uint32**)(0) = p;
	}

	// we just hit a crash case so setup flags / globals accordingly
	IsCrashed= true;
	StlMemoryAllocatorCrashPoint= state;
	signal(SIGSEGV, NULL);
}

