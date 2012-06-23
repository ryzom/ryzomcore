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

#ifndef NL_SYSTEM_INFO_H
#define NL_SYSTEM_INFO_H

#include "types_nl.h"

#include <string>

namespace NLMISC {


/**
 * This class provides general system-level information about the local machine.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CSystemInfo
{
public:

	static std::string getOS ();
	static std::string getProc ();

	/** Gives an evaluation of the processor frequency, in hertz
	  * \param quick true to do quick frequency evaluation
	  * \warning Supports only Intel architectures for now. Returns 0 if not implemented.
	  */
	static uint64 getProcessorFrequency (bool quick = false);

	/** Tests whether the CPUID instruction is supported
	  * (always false on non Intel architectures)
	  */
	static bool hasCPUID ();

	/** Helps to know whether the processor features MMX instruction set
	  * This is initialized at started, so its fast
	  * (always false on non 0x86 architecture ...)
	  */
	static bool hasMMX () {return _HaveMMX;}

	/** Helps to know whether the processor has streaming SIMD instructions (the OS must supports it)
	  * This is initialized at started, so its fast
	  * (always false on non 0x86 architecture ...)
	  */
	static bool hasSSE () {return _HaveSSE;}

	/** Gets the CPUID (if available). Useful for debug info
	  */
	static uint32 getCPUID();

	/** true if the Processor has HyperThreading
	  */
	static bool hasHyperThreading();

	/** true if running under NT
	  */
	static bool isNT();

	/** Returns the space left on the hard drive that contains the filename
	  */
	static std::string availableHDSpace (const std::string &filename);

	/** Returns all the physical memory available on the computer (in bytes)
	  */
	static uint64 availablePhysicalMemory ();

	/** Returns the physical memory on the computer (in bytes)
	  */
	static uint64 totalPhysicalMemory ();

	/** Returns the amount of allocated system memory (in bytes)
	  */
	static uint64 getAllocatedSystemMemory ();

	/** Returns all the virtual memory used by this process (in bytes)
	  */
	static uint64 virtualMemory ();

	/** Returns the main video card name and the video driver version
	  */
	static bool getVideoInfo (std::string &deviceName, uint64 &driverVersion);

private:
	static bool _HaveMMX;
	static bool _HaveSSE;
};

} // NLMISC

#endif // NL_SYSTEM_INFO_H

/* End of system_info.h */
