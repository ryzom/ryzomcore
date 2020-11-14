// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_FAST_FLOOR_H
#define NL_FAST_FLOOR_H

#include "types_nl.h"
#include <cmath>
#include <nel/misc/debug.h>

namespace NLMISC
{

const uint		OptFastFloorCWStackSize = 10;
extern int      OptFastFloorCWStack[OptFastFloorCWStackSize];
extern int      *OptFastFloorCWStackPtr;
extern int      *OptFastFloorCWStackEnd;

// fastFloor function.
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM) && defined(NL_USE_FASTFLOOR)

#include <cfloat>

// The magic constant value. support both positive and negative numbers.
extern double	OptFastFloorMagicConst ;

inline void OptFastFloorPushCW(int ctrl)
{
	nlassert(OptFastFloorCWStackPtr < OptFastFloorCWStackEnd);
	*OptFastFloorCWStackPtr++ = _controlfp(0, 0);
	_controlfp( ctrl, _MCW_RC|_MCW_PC );
}

inline void OptFastFloorPopCW()
{
	nlassert(OptFastFloorCWStackPtr >=  OptFastFloorCWStack);
	_controlfp(*(--OptFastFloorCWStackPtr), _MCW_RC|_MCW_PC);
}


// init float CW.
inline void  OptFastFloorBegin()
{
	OptFastFloorPushCW(_RC_DOWN|_PC_53);
}

// reset float CW.
inline void  OptFastFloorEnd()
{
	OptFastFloorPopCW();
}

// Force __stdcall to not pass parameters in registers.
inline sint32 __stdcall OptFastFloor(float x)
{
	static __int64	res;
	__asm
	{
		fld		x
		fadd	qword ptr OptFastFloorMagicConst
		fstp	qword ptr res
	}

	return (sint32) (res&0xFFFFFFFF);
}


// Force __stdcall to not pass parameters in registers.
// Only used by particles system
inline float __stdcall OptFastFractionnalPart(float x)
{
	static double res;
	__asm
	{
		fld		x
		fld     st(0)
		fadd	qword ptr OptFastFloorMagicConst
		fstp	qword ptr res
		fisub   dword ptr res
		fstp    dword ptr res
	}

	return * (float *) &res;
}


// The magic constant value, for 24 bits precision support positive numbers only
extern float	OptFastFloorMagicConst24 ;
// init float CW. Init with float 24 bits precision, for faster float operation.
inline void  OptFastFloorBegin24()
{
	OptFastFloorPushCW(_RC_DOWN|_PC_24);
}

// reset float CW.
inline void  OptFastFloorEnd24()
{
	OptFastFloorPopCW();
}

// Force __stdcall to not pass parameters in registers.
/// Same method as OptFastFloor, but result are always positive and should never be bigger than 2^23-1
/// Only used for float to byte color attributes conversions
inline uint32 __stdcall OptFastFloor24(float x)
{
	static uint32	res;
	__asm
	{
		fld		x
		fadd	dword ptr OptFastFloorMagicConst24
		fstp	dword ptr res
	}

	return res;
}



#else

#ifdef __SSE__

// SSE intrinsics header
#include <xmmintrin.h>

inline void OptFastFloorPushCW(int ctrl)
{
	nlassert(OptFastFloorCWStackPtr < OptFastFloorCWStackEnd);
	*OptFastFloorCWStackPtr++ = _MM_GET_ROUNDING_MODE();
	_MM_SET_ROUNDING_MODE(ctrl);
}

inline void OptFastFloorPopCW()
{
	nlassert(OptFastFloorCWStackPtr >=  OptFastFloorCWStack);
	_MM_SET_ROUNDING_MODE(*(--OptFastFloorCWStackPtr));
}

#endif

inline void  OptFastFloorBegin()
{
#ifdef __SSE__
	OptFastFloorPushCW(_MM_ROUND_DOWN);
#endif
}

inline void  OptFastFloorEnd()
{
#ifdef __SSE__
	OptFastFloorPopCW();
#endif
}

inline sint  OptFastFloor(float x)
{
#ifdef __SSE__
	return _mm_cvtss_si32(_mm_set_ss(x));
#else
	return (sint)floor(x);
#endif
}

inline float  OptFastFractionnalPart(float x)
{
#ifdef __SSE__
	static __m128 a, b;
	a = _mm_set_ss(x);
	b = _mm_cvtsi32_ss(b, _mm_cvttss_si32(a));
	return _mm_cvtss_f32(_mm_comilt_ss(a, b) ? _mm_sub_ss(b, a):_mm_sub_ss(a, b));
#else
	return x < 0.f ? (sint)x - x:x - (sint)x;
#endif
}


inline void  OptFastFloorBegin24()
{
#ifdef __SSE__
	OptFastFloorPushCW(_MM_ROUND_DOWN);
#endif
}

inline void  OptFastFloorEnd24()
{
#ifdef __SSE__
	OptFastFloorPopCW();
#endif
}

inline uint32 OptFastFloor24(float x)
{
#ifdef __SSE__
	return (uint32)_mm_cvtss_si32(_mm_set_ss(x));
#else
	return (uint32)floor(x);
#endif
}


#endif



} // NLMISC


#endif // NL_FAST_FLOOR_H

/* End of fast_floor.h */
