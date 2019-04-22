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

#ifdef NL_OS_WINDOWS

#include "nel/misc/check_fpu.h"
#include "nel/misc/fast_floor.h"


#pragma optimize("", off )

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC
{


int		CFpuChecker::_RefFpuCtrl= 0x00090013;


CFpuChecker::CFpuChecker()
{
	check();
}
CFpuChecker::~CFpuChecker()
{
	check();
}

void CFpuChecker::dumpFpu(int value)
{
	nlwarning("_MCW_DN = %s", value & _MCW_DN ? "_DN_FLUSH" : "_DN_SAVE");
	nlwarning("_MCW_EM = %x", _MCW_EM);
	switch(value & _MCW_EM)
	{
		case _EM_INVALID: nlwarning("_EM_INVALID"); break;
		case _EM_DENORMAL: nlwarning("_EM_DENORMAL"); break;
		case _EM_ZERODIVIDE: nlwarning("_EM_ZERODIVIDE"); break;
		case _EM_OVERFLOW: nlwarning("_EM_OVERFLOW"); break;
		case _EM_UNDERFLOW: nlwarning("_EM_UNDERFLOW"); break;
		case _EM_INEXACT: nlwarning("_EM_INEXACT"); break;
		default:
			nlwarning("_MCW_EM = %x", value & _MCW_EM);
		break;
	}
	nlwarning("_MCW_IC = %s", value & _MCW_IC ? "_IC_AFFINE" : "_IC_PROJECTIVE");
	switch(value & _MCW_RC)
	{
		case _RC_CHOP: nlwarning("_RC_CHOP"); break;
		case _RC_UP: nlwarning("_RC_UP"); ; break;
		case _RC_DOWN: nlwarning("_RC_DOWN"); break;
		case _RC_NEAR: nlwarning("_RC_NEAR"); break;
	}
	switch(value & _MCW_PC)
	{
		case _PC_24: nlwarning("_PC_24"); break;
		case _PC_53: nlwarning("_PC_53"); ; break;
		case _PC_64: nlwarning("_PC_64"); break;
	}


}

void CFpuChecker::check()
{
#if defined(NL_OS_WINDOWS) && defined(NL_COMP_VC6)
	// don't Check if in a user control state
	if (NLMISC::OptFastFloorCWStackPtr != NLMISC::OptFastFloorCWStack) return;

	// check standard control state
	volatile int cfp = _controlfp(0, 0);
	if ((cfp & ~_MCW_EM) != (_RefFpuCtrl & ~_MCW_EM))
	{
		nlwarning("Ref = ");
		nlwarning("=========================");
		dumpFpu(_RefFpuCtrl);
		nlwarning("Current = ");
		nlwarning("=========================");
		dumpFpu(cfp);
		nlassert(0);
	}
#endif
}

}

#endif // NL_OS_WINDOWS
