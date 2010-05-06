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

#include "nel/misc/cpu_info.h"


namespace NLMISC
{

static bool DetectMMX(void)
{
#ifdef NL_OS_WINDOWS
	if (!CCpuInfo___::hasCPUID()) return false; // cpuid not supported ...

	uint32 result = 0;
	__asm
	{
		 mov  eax,1
		 cpuid
		 test edx,0x800000  // bit 23 = MMX instruction set
		 je   noMMX
		 mov result, 1
		noMMX:
	}

	return result == 1;

	// printf("mmx detected\n");

#else
	return false;
#endif
}


static bool DetectSSE()
{
	#ifdef NL_OS_WINDOWS
		if (!CCpuInfo___::hasCPUID()) return false; // cpuid not supported ...

		uint32 result = 0;
		__asm
		{
			mov eax, 1   // request for feature flags
			cpuid
			test EDX, 002000000h   // bit 25 in feature flags equal to 1
			je noSSE
			mov result, 1  // sse detected
		noSSE:
		}


		if (result)
		{
			// check OS support for SSE
			try
			{
				__asm
				{
					xorps xmm0, xmm0  // Streaming SIMD Extension
				}
			}
			catch(...)
			{
				return false;
			}

			// printf("sse detected\n");

			return true;
		}
		else
		{
			return false;
		}
	#else
		return false;
	#endif
}

bool HasMMX = DetectMMX();
bool HasSSE = DetectSSE();

bool CCpuInfo___::hasCPUID()
{
#ifdef NL_OS_WINDOWS
	 uint32 result;
	 __asm
	 {
		 pushad
		 pushfd
		 //	 If ID bit of EFLAGS can change, then cpuid is available
		 pushfd
		 pop  eax					// Get EFLAG
		 mov  ecx,eax
		 xor  eax,0x200000			// Flip ID bit
		 push eax
		 popfd						// Write EFLAGS
		 pushfd
		 pop  eax					// read back EFLAG
		 xor  eax,ecx
		 je   noCpuid				// no flip -> no CPUID instr.

		 popfd						// restore state
		 popad
		 mov  result, 1
		 jmp  CPUIDPresent

		noCpuid:
		 popfd					    // restore state
		 popad
		 mov result, 0
		CPUIDPresent:
	 }
	 return result == 1;
#else
	 return false;
#endif
}
bool CCpuInfo___::hasMMX() { return HasMMX; }
bool CCpuInfo___::hasSSE() { return HasSSE; }

} // NLMISC
