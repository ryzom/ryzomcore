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

#ifndef NL_CPU_INFO_H
#define NL_CPU_INFO_H

#include "types_nl.h"


namespace NLMISC {


/**
 * This helps to know whether cpu has some features such as mmx, sse ...
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
struct CCpuInfo___
{
	/** test whether the cpuid instruction is supported
	  * (always false on non intel architectures)
	  */
	static bool hasCPUID(void);

	/** helps to know whether the processor features mmx instruction set
	  * This is initialized at startup, so it's fast
	  * (always false on not 0x86 architecture ...)
	  */
	static bool hasMMX(void);

	/** helps to know whether the processor has streaming SIMD instructions (the OS must support it)
	  * This is initialized at startup, so it's fast
	  * (always false on not 0x86 architecture ...)
	  */
	static bool hasSSE(void);
};


} // NLMISC


#endif // NL_CPU_INFO_H

/* End of cpu_info.h */
