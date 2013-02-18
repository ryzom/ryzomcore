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

#include "nel/misc/rgba.h"
#include "nel/misc/stream.h"
#include "nel/misc/system_info.h"
#include "nel/misc/common.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

#ifdef NL_NO_ASM
	#define DISABLE_MMX_OPTIM
#endif

// ***************************************************************************
// ***************************************************************************
// CRGBA
// ***************************************************************************
// ***************************************************************************

// predifined colors

/// some colors
const CRGBA CRGBA::Black(0, 0, 0) ;
const CRGBA CRGBA::Red(255, 0, 0) ;
const CRGBA CRGBA::Green(0, 255, 0) ;
const CRGBA CRGBA::Yellow(255, 255, 0) ;
const CRGBA CRGBA::Blue(0, 0, 255) ;
const CRGBA CRGBA::Magenta(255, 0, 255) ;
const CRGBA CRGBA::Cyan(0, 255, 255) ;
const CRGBA CRGBA::White(255, 255, 255) ;

// ***************************************************************************
void CRGBA::serial(class NLMISC::IStream &f)
{
	f.serial (R);
	f.serial (G);
	f.serial (B);
	f.serial (A);
}
// ***************************************************************************
void CRGBA::set(uint8 r, uint8 g, uint8 b, uint8 a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}
// ***************************************************************************
uint8 CRGBA::toGray() const
{
	return (R*11+G*16+B*5)/32;
}

#ifdef 	NL_OS_WINDOWS
	#pragma warning(disable : 4731) /* frame pointer register 'ebp' modified by inline assembly code */
#endif

// ***************************************************************************
void CRGBA::addColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride, uint destStride, uint dup)
{
	if (numColors == 0) return;
#if defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	if (!CSystemInfo::hasMMX())
#endif
	{   // unoptimized version
		if (dup == 1)
		{
			while (numColors--)
			{
				dest->add(*src1, *src2);
				dest = (CRGBA *) ((uint8 *) dest + destStride);
				src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
				src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
			}
		}
		else
		{
			if (dup == 4) // optimisation for the 4 case
			{
				while (numColors--)
				{
					dest->add(*src1, *src2);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + (destStride << 1));

					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
			else
			{
				while (numColors--)
				{
					dest->add(*src1, *src2);

					uint k = dup - 1;
					do
					{
						* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
						dest = (CRGBA *) ((uint8 *) dest + destStride);
					}
					while (--k);

					dest = (CRGBA *) ((uint8 *) dest + destStride);
					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
		}
	}
#if defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	else // optimized mmx version
	{
		/// well, this could be further optimized when stride is 4 (2 at once)
		if (dup == 1)
		{
			__asm
			{
						push        ebp
						mov			edi, dest
						mov			esi, src1
						mov			ebx, src2
						sub			ebx, esi  ; offset to source 2
						mov			ecx, numColors
						mov         edx, destStride
						mov         ebp, srcStride
						sub         edi, edx
				myLoop:
						movd        mm0, [esi]
						add         edi, edx
						movd        mm1, [esi + ebx]
						paddusb     mm0, mm1
						movd        [edi], mm0
						add         esi, ebp
						dec			ecx
						jne         myLoop
						pop			ebp
						emms
			}
		}
		else
		{
			if (dup == 4)
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         ebp, srcStride
					myLoop4:
							movd        mm0, [esi]
							movd        mm1, [esi + ebx]
							paddusb     mm0, mm1
							movd        eax, mm0

							mov         [edi], eax
							mov         [edi + edx], eax
							mov         [edi + 2 * edx], eax
							lea         edi, [edi + edx * 2]
							mov         [edi + edx], eax
							lea         edi, [edi + edx * 2]
							add         esi, ebp

							dec         ecx
							jne         myLoop4
							pop			ebp
							emms
				}
			}
			else
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         eax, dup
							mov         ebp, srcStride
							push        eax
					myLoopN:
							movd        mm0, [esi]
							movd        mm1, [esi + ebx]
							push        ecx
							paddusb     mm0, mm1
							mov         ecx, 4[esp]
							movd        eax, mm0
					dupLoopN:
							mov         [edi], eax
							dec         ecx
							lea         edi, [edi + edx]
							jne		    dupLoopN
							pop         ecx			; get back the loop counter
							add         esi, ebp
							dec         ecx
							jne         myLoopN
							pop eax
							pop	ebp
							emms
				}
			}
		}
	}
#endif
}

// ***************************************************************************
void CRGBA::modulateColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride, uint destStride, uint dup)
{
#if 	defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	if (!CSystemInfo::hasMMX())
#endif
	{   // unoptimized version
		if (dup == 1)
		{
			while (numColors--)
			{
				dest->modulateFromColor(*src1, *src2);
				dest = (CRGBA *) ((uint8 *) dest + destStride);
				src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
				src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
			}
		}
		else
		{
			if (dup == 4) // optimisation for the 4 case
			{
				while (numColors--)
				{
					dest->modulateFromColor(*src1, *src2);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + (destStride << 1));

					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
			else
			{
				while (numColors--)
				{
					dest->modulateFromColor(*src1, *src2);

					uint k = dup - 1;
					do
					{
						* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
						dest = (CRGBA *) ((uint8 *) dest + destStride);
					}
					while (--k);

					dest = (CRGBA *) ((uint8 *) dest + destStride);
					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
		}
	}
#if 	defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	else // optimized mmx version
	{
		uint64 blank = 0;
		/// well, this could be further optimized when stride is 4
		if (dup == 1)
		{	__asm
			{
						push        ebp
						movq        mm2, blank
						mov			edi, dest
						mov			esi, src1
						mov			ebx, src2
						sub			ebx, esi  ; offset to source 2
						mov			ecx, numColors
						mov         edx, destStride
						mov         ebp, srcStride
				myLoop:
						movd			mm0, [esi]
						movd			mm1, [esi + ebx]
						punpcklbw	mm0, mm2
						punpcklbw	mm1, mm2
						pmullw		mm0, mm1
						psrlw       mm0, 8
						packuswb    mm0, mm0
						movd        [edi], mm0
						add         edi, edx
						add         esi, ebp

						dec         ecx
						jne         myLoop
						pop			ebp
						emms
			}
		}
		else
		{
			if (dup == 4)
			{

				__asm
				{
							push        ebp
							movq        mm2, blank
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         ebp, srcStride
					myLoop4:
							movd		mm0, [esi]
							movd		mm1, [esi + ebx]
							punpcklbw	mm0, mm2
							punpcklbw	mm1, mm2
							pmullw		mm0, mm1
							psrlw       mm0, 8
							packuswb    mm0, mm0
							movd        eax, mm0
							; duplicate the result 4 times
							mov         [edi], eax
							mov         [edi + edx], eax
							mov         [edi + 2 * edx], eax
							lea         edi, [edi +  2 * edx]
							mov         [edi + edx], eax
							lea         edi, [edi + 2 * edx]
							add         esi, ebp
							dec         ecx
							jne         myLoop4
							pop			ebp
							emms
				}
			}
			else
			{
				__asm
				{
							push        ebp
							movq        mm2, blank
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         eax, dup
							mov         ebp, srcStride
							push        eax
					myLoopN:
							movd		mm0, [esi]
							movd		mm1, [esi + ebx]
							punpcklbw	mm0, mm2
							punpcklbw	mm1, mm2
							pmullw		mm0, mm1
							push        ecx
							psrlw       mm0, 8
							mov         ecx, 4[esp]
							packuswb    mm0, mm0
							movd        eax, mm0
					dupLoopN:
							mov         [edi], eax
							dec         ecx
							lea         edi, [edi + edx]
							jne		    dupLoopN
							pop         ecx			; get back the loop counter
							add         esi, ebp
							dec         ecx
							jne         myLoopN
							pop eax
							pop			ebp
							emms
				}
			}
		}
	}
#endif
}

// ***************************************************************************
void CRGBA::subtractColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride, uint destStride, uint dup)
{
	if (numColors == 0) return;
#if defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	if (!CSystemInfo::hasMMX())
#endif
	{   // unoptimized version
		if (dup == 1)
		{
			while (numColors--)
			{
				dest->sub(*src1, *src2);
				dest = (CRGBA *) ((uint8 *) dest + destStride);
				src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
				src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
			}
		}
		else
		{
			if (dup == 4) // optimisation for the 4 case
			{
				while (numColors--)
				{
					dest->sub(*src1, *src2);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + (destStride << 1));

					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
			else
			{
				while (numColors--)
				{
					dest->sub(*src1, *src2);

					uint k = dup - 1;
					do
					{
						* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
						dest = (CRGBA *) ((uint8 *) dest + destStride);
					}
					while (--k);

					dest = (CRGBA *) ((uint8 *) dest + destStride);
					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
		}
	}
#if defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	else // optimized mmx version
	{
		/// well, this could be further optimized when stride is 4 (2 at once)
		if (dup == 1)
		{
			__asm
			{
						push        ebp
						mov			edi, dest
						mov			esi, src1
						mov			ebx, src2
						sub			ebx, esi  ; offset to source 2
						mov			ecx, numColors
						mov         edx, destStride
						mov         ebp, srcStride
						sub         edi, edx
				myLoop:
						movd        mm0, [esi]
						add         edi, edx
						movd        mm1, [esi + ebx]
						psubusb     mm0, mm1
						movd        [edi], mm0
						add         esi, ebp
						dec			ecx
						jne         myLoop
						pop			ebp
						emms
			}
		}
		else
		{
			if (dup == 4)
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         ebp, srcStride
					myLoop4:
							movd        mm0, [esi]
							movd        mm1, [esi + ebx]
							psubusb     mm0, mm1
							movd        eax, mm0

							mov         [edi], eax
							mov         [edi + edx], eax
							mov         [edi + 2 * edx], eax
							lea         edi, [edi + edx * 2]
							mov         [edi + edx], eax
							lea         edi, [edi + edx * 2]
							add         esi, ebp

							dec         ecx
							jne         myLoop4
							pop			ebp
							emms
				}
			}
			else
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         eax, dup
							mov         ebp, srcStride
							push        eax
					myLoopN:
							movd        mm0, [esi]
							movd        mm1, [esi + ebx]
							push        ecx
							psubusb     mm0, mm1
							mov         ecx, 4[esp]
							movd        eax, mm0
					dupLoopN:
							mov         [edi], eax
							dec         ecx
							lea         edi, [edi + edx]
							jne		    dupLoopN
							pop         ecx			; get back the loop counter
							add         esi, ebp
							dec         ecx
							jne         myLoopN
							pop eax
							pop	ebp
							emms
				}
			}
		}
	}
#endif
}

// ***************************************************************************
// ***************************************************************************
// CBGRA
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CBGRA::serial(class NLMISC::IStream &f)
{
	f.serial (B);
	f.serial (G);
	f.serial (R);
	f.serial (A);
}

// ***************************************************************************
void CBGRA::set(uint8 r, uint8 g, uint8 b, uint8 a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}

// ***************************************************************************
void CBGRA::blendFromui(CBGRA &c0, CBGRA &c1, uint coef) // coef must be in [0,256]
{
	sint	a1 = coef;
	sint	a2 = 256-a1;
	R = (c0.R*a2 + c1.R*a1) >>8;
	G = (c0.G*a2 + c1.G*a1) >>8;
	B = (c0.B*a2 + c1.B*a1) >>8;
	A = (c0.A*a2 + c1.A*a1) >>8;
}

// ***************************************************************************
bool CRGBA::convertToHLS(float &h, float &l, float &s) const
{
	float r = R / 255.f;
	float g = G / 255.f;
	float b = B / 255.f;

	float maxV = NLMISC::maxof(r, g, b);
	float minV = NLMISC::minof(r, g, b);

	// all composants are equals -> achromatique
	if (minV == maxV)
	{
		h = 0.f;
		l = minV;
		s = 0.f;
		return true;
	}

	// get lightness
	l = 0.5f * (maxV + minV);

	float diff = maxV - minV;

	// get saturation
	s  = l > 0.5f ?   /*are we in the top of the double-hexcone ? */
		diff / (2.f - maxV - minV)  :
		diff / (maxV + minV);

	// get hue
	if (maxV == r)
	{
		h = (g - b) / diff;
	}
	else if (maxV == g)
	{
		h = 2.f + (b - r) / diff;
	}
#if defined(GCC_VERSION) && (GCC_VERSION == 40204)
	// use the fix only if using the specific GCC version
	else if (maxV == b)
	{
		h = 4.f + (r - g) / diff;
	}
	else
	{
		// this case is to fix a compiler bug
		h = (g - b) / diff;
	}
#else
	else
	{
		h = 4.f + (r - g) / diff;
	}
#endif

	h *= 60.f; // scale to [0..360]

	if (h < 0.f) h += 360.f;

	return false;
}

// ***************************************************************************
/// Used by buildFromHLS
static float HLSValue(float h, float v1, float v2)
{
	/* get hue in the [0, 360] interval */
	// h -= 360.f * ::floorf(h / 360.f);

	if (h > 360.f) h -= 360.f;
	else if (h < 0) h += 360.f;

	if (h < 60.f)
	{
		return v1 + (v2 - v1) * h / 60.f;
	}
	else if (h < 180.f)
	{
		return v2;
	}
	else if (h < 240.f)
	{
		return v1 + (v2 - v1) * (240.f - h) / 60.f;
	}
	else
	{
		return v1;
	}
}


// ***************************************************************************
void CRGBA::buildFromHLS(float h, float l, float s)
{
	clamp(l, 0.f, 1.f);
	clamp(s, 0.f, 1.f);

	float v2 = (l <= 0.5f) ? (l * (1.f + s)) : (l + s - l * s);
	float v1 = 2.f * l - v2;

	if (s == 0) // achromatic ?
	{
		R = G = B = (uint8) (255.f * l);
	}
	else // chromatic case
	{
		float v;
		//
		v = HLSValue(h + 120.f, v1, v2);
		clamp(v, 0.f, 1.f);
		R = (uint8) (255.f * v);
		//
		v = HLSValue(h, v1, v2);
		clamp(v, 0.f, 1.f);
		G = (uint8) (255.f * v);
		//
		v = HLSValue(h - 120.f, v1, v2);
		clamp(v, 0.f, 1.f);
		B = (uint8) (255.f * v);
	}
}

CRGBA CRGBA::stringToRGBA( const char *ptr )
{
	if (!ptr)
		return NLMISC::CRGBA::White;
	
	int r = 255, g = 255, b = 255, a = 255;
	sscanf( ptr, "%d %d %d %d", &r, &g, &b, &a );
	clamp( r, 0, 255 );
	clamp( g, 0, 255 );
	clamp( b, 0, 255 );
	clamp( a, 0, 255 );
	
	return CRGBA( r,g,b,a );
}

std::string CRGBA::toString() const
{
	std::string s;
	s =  NLMISC::toString( R );
	s += " ";
	s += NLMISC::toString( G );
	s += " ";
	s += NLMISC::toString( B );
	s += " ";
	s += NLMISC::toString( A );
	return s;
}

bool CRGBA::fromString( const std::string &s )
{
	*this = stringToRGBA( s.c_str() );
	return true;
}


// ***************************************************************************
// ***************************************************************************
// CRGBAF
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CRGBAF::serial(class NLMISC::IStream &f)
{
	f.serial (R);
	f.serial (G);
	f.serial (B);
	f.serial (A);
}
// ***************************************************************************
void CRGBAF::set(float r, float g, float b, float a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}

} // NLMISC
