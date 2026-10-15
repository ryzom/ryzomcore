// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/misc/atomic.h"

namespace NLMISC {

#if !defined(NL_ATOMIC_CPP14) && defined(NL_ATOMIC_WIN32)

void nlYield()
{
	::SwitchToThread();
}

#endif /* !defined(NL_ATOMIC_CPP14) && defined(NL_ATOMIC_WIN32) */

// Dummy function to keep the compiler quiet on empty CPP and to ensure all types compiled well
void atomic_cpp_dummy__()
{
	// Basic atomic types and operations
	CAtomicFlag flag;
	CAtomicInt num;
	CAtomicBool ok; // is CAtomicEnum<bool>
	flag.testAndSet();
	flag.clear();
#ifdef NL_ATOMIC_FLAG_TEST
	flag.test();
#endif
	num.store(num.load());
	num.fetchAdd(1);
	num.exchange(2);
	ok.store(true);
	ok.load();
	ok.exchange(false);

	// All atomic lock strategies
	{
		CAtomicLockSpin lock(flag);
	}
	{
		CAtomicLockYield lock(flag);
	}
	{
		CAtomicLockFast lock(flag);
	}
	{
		CAtomicLockFastMP lock(flag);
	}

	// Not initialized
	CAtomicFlag flagUninit;
	CAtomicInt numUninit;
	CAtomicBool okUninit;

	// Initialized
	CAtomicFlag flagInit = true;
	CAtomicInt numInit = 0;
	CAtomicBool okInit = false;
}

} /* namespace NLMISC */

/* end of file */
