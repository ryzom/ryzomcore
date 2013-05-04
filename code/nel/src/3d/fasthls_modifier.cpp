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
#include "nel/3d/fasthls_modifier.h"
#include "nel/misc/fast_floor.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/system_info.h"
#include "nel/misc/algo.h"


using	namespace std;
using	namespace NLMISC;


namespace NL3D
{

// ***************************************************************************
CFastHLSModifier	*CFastHLSModifier::_Instance= NULL;

// ***************************************************************************
void CFastHLSModifier::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ***************************************************************************
CFastHLSModifier::CFastHLSModifier()
{
	uint i;
	// build the HueTable.
	for(i=0;i<HueTableSize;i++)
	{
		_HueTable[i].buildFromHLS(360.0f*i/HueTableSize, 0.5f, 1);
	}
	// build conversion from uint16 to HLS.
	for(i=0;i<65536;i++)
	{
		CRGBA	col;
		col.set565(i);
		float	h,l,s;
		col.convertToHLS(h,l,s);
		h= (float)floor(255*(h/360.f)+0.5f);
		l= (float)floor(255*l+0.5f);
		s= (float)floor(255*s+0.5f);
		clamp(h,0,255);
		clamp(l,0,255);
		clamp(s,0,255);
		_Color16ToHLS[i].H= (uint8)h;
		_Color16ToHLS[i].L= (uint8)l;
		_Color16ToHLS[i].S= (uint8)s;
		_Color16ToHLS[i].A= 255;
	}
}

// ***************************************************************************
CFastHLSModifier::~CFastHLSModifier()
{
}

// ***************************************************************************
CFastHLSModifier	&CFastHLSModifier::getInstance()
{
	if(!_Instance)
		_Instance= new CFastHLSModifier;
	return *_Instance;
}


// ***************************************************************************
CRGBA		CFastHLSModifier::convert(uint H, uint L, uint S)
{
	static	CRGBA	gray(128,128,128);
	L+= L>>7;
	S+= S>>7;
	// H.
	CRGBA	col= _HueTable[H];
	// S.
	col.blendFromuiRGBOnly(gray, col, S);
	// L.
	if(L<=128)
	{
		col.modulateFromuiRGBOnly(col, L*2);
	}
	else
	{
		col.blendFromuiRGBOnly(col, CRGBA::White, (L-128)*2 );
	}

	return col;
}

#if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 71
#	pragma warning( push )
#	pragma warning( disable : 4799 )
#endif

#ifdef NL_OS_WINDOWS
#pragma managed(push, off)
#endif

// ***************************************************************************
uint16		CFastHLSModifier::applyHLSMod(uint16 colorIn, uint8 dHue, uint dLum, uint dSat)
{
	static	uint64	mmBlank	= 0;
	static	uint64	mmOne	= INT64_CONSTANT(0x00FF00FF00FF00FF);
	static	uint64	mmGray	= INT64_CONSTANT(0x0080008000800080);
	static	uint64	mmInterpBufer[4]= {0,0,0,INT64_CONSTANT(0x00FF00FF00FF00FF)};

	/*
		dLum is actually 0xFFFFFF00 + realDLum
		dSat is actually 0xFFFFFF00 + realDSat
	*/

	uint16	retVal;

#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	if(CSystemInfo::hasMMX())
	{
		__asm
		{
			mov			edi, offset mmInterpBufer
			mov			ecx, this

			// get HLS in edx.
			mov			eax, 0
			mov			ebx, 0
			lea			esi, [ecx]this._Color16ToHLS
			mov			ax, colorIn
			mov			edx, [esi+ eax*4]

			// apply dh to H (ie dl!). Auto-wrap.
			add			dl, dHue
			// get the color into mm0
			mov			bl, dl
			lea			esi, [ecx]this._HueTable
			movd		mm0, [esi+ ebx*4]
			punpcklbw	mm0, mmBlank

			// get L into eax and S into ebx
			mov			eax, edx
			mov			ebx, edx
			shr			eax, 8
			shr			ebx, 16
			and			eax, 255
			and			ebx, 255
			// add dLum/dSat and clamp to 1.
			add			eax, dLum
			sbb			ecx, ecx	// ecx= FFFFFFFF if carry.
			add			ebx, dSat
			sbb			edx, edx
			or			eax, ecx	// eax= FFFFFFFF if carry was set
			or			ebx, edx
			// add Magic delta, and clamp to 0.
			add			eax, 256
			sbb			ecx, ecx	// ecx= 0 if carry not set => result below 0.
			add			ebx, 256
			sbb			edx, edx
			and			eax, ecx	// eax= 0 if result was below 0
			and			ebx, edx

			// Load Sat/(1-Sat) into MMX
			movd		mm2, ebx
			movq		mm3, mmOne
			punpckldq	mm2, mm2	// mm2= 0000 00AA 0000 00AA
			packssdw	mm2, mm2	// mm2= 00AA 00AA 00AA 00AA
			movq		mm1, mmGray
			psubusw		mm3, mm2		// mm3= 1-sat.
			// combine Color and Sat
			pmullw		mm0, mm2	// mm0= color*sat
			pmullw		mm1, mm3	// mm1= gray*(1-sat)
			paddusw		mm0, mm1	// mm0= color saturated
			// shift and store into the buffer for Luminance interpolation
			psrlw       mm0, 8
			movq		[edi+ 8], mm0
			movq		[edi+ 16], mm0

			// use edx as index for luminance: 0: L=0 to 127. 1: L=128 to 255.
			mov			edx, eax
			shl			eax, 1
			shr			edx, 7
			and			eax, 255		// 0-127 and 128-255 transform auto to 0-254
			// expand 0-254 to 0-255
			mov			ecx, eax
			shl			edx, 4
			shr			ecx, 7
			add			eax, ecx

			// Combine color and Luminance into MMX. interpolate 0->col or col->white according to edx.
			// Load Lum/(1-Lum) into MMX
			movd		mm2, eax
			movq		mm3, mmOne
			punpckldq	mm2, mm2	// mm2= 0000 00AA 0000 00AA
			packssdw	mm2, mm2	// mm2= 00AA 00AA 00AA 00AA
			psubusw		mm3, mm2	// mm3= 1-lum.
			// Combine color and Sat into MMX
			movq		mm0, [edi+ edx]
			movq		mm1, [edi+ edx + 8]
			pmullw		mm0, mm3	// mm0= color0*(1-lum)
			pmullw		mm1, mm2	// mm1= color1*lum
			paddusw		mm0, mm1	// mm0= final color

			// shift and unpack
			psrlw       mm0, 8
			packuswb    mm0, mm0
			movd		eax, mm0

			// pack to 16bits.
			mov			ebx, eax
			mov			ecx, eax
			shl			eax, 8		// Red
			shr			ebx, 5		// Green
			shr			ecx, 19		// Blue
			and			eax, 0xF800
			and			ebx, 0x07E0
			and			ecx, 0x001F
			or			eax, ebx
			or			eax, ecx

			mov			retVal, ax
		}
	}
	else
#endif	// NL_OS_WINDOWS
	{
		CHLSA	hls= _Color16ToHLS[colorIn];
		// apply (C version) Dhue, dLum and dSat
		hls.H= uint8((hls.H + dHue) & 0xFF);
		sint	v= (sint)hls.L + (sint)(dLum-0xFFFFFF00);
		fastClamp8(v);
		hls.L= v;
		v= (sint)hls.S + (sint)(dSat-0xFFFFFF00);
		fastClamp8(v);
		hls.S= v;

		CRGBA	ret= convert(hls.H, hls.L, hls.S);
		retVal= ret.get565();
	}

	return retVal;
}
#ifdef NL_OS_WINDOWS
#pragma managed(pop)
#endif

#if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 71
#	pragma warning( pop )
#endif

// ***************************************************************************
#ifdef NL_OS_WINDOWS
#pragma managed(push, off)
#endif
void		CFastHLSModifier::convertDDSBitmapDXTC1Or1A(CBitmap &dst, const CBitmap &src, uint8 dh, uint dLum, uint dSat)
{
	uint	W= src.getWidth();
	uint	H= src.getHeight();

	const uint8	*srcPix= &(src.getPixels()[0]);
	uint8		*dstPix= &(dst.getPixels()[0]);
	uint	numBlock= (W*H)/16;

	/*
		need to swap color and bits for DXTC1 or DXTC1A.
	*/

	static uint32	bitLUT[8]= {
		1,0,3,2,			// reverse std order
		1,0,2,3,			// reverse order for "special 0/black packing"
	};

	// Do not use alpha mask for now.
	for(;numBlock>0;numBlock--)
	{
		uint16	srcCol0= ((uint16*)srcPix)[0];
		uint16	srcCol1= ((uint16*)srcPix)[1];
		bool	srcSign= srcCol0>srcCol1;
		// apply modifiers for 2 colors.
		uint16	dstCol0= applyHLSMod(srcCol0, dh,dLum,dSat);
		uint16	dstCol1= applyHLSMod(srcCol1, dh,dLum,dSat);
		bool	dstSign= dstCol0>dstCol1;
		if((uint)dstSign!=(uint)srcSign)
		{
			swap(dstCol0,dstCol1);
			// must change bits too!
			uint32	srcBits= ((uint32*)srcPix)[1];
			uint32	dstBits= 0;
			// take correct lut according to original sign
			uint32	*lut;
			if(srcCol0>srcCol1)
				lut= bitLUT;
			else
				lut= bitLUT+4;

			// for all bits, transpose with lut.
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
			__asm
			{
				mov		eax, srcBits
				mov		esi, lut
				mov		edx, 0
				mov		ecx, 16
				// prepare 1st.
				rol		eax, 2
				mov		ebx, eax
				and		ebx, 2
				// do it 16 times.
			myLoop:
				or		edx, [esi+ebx*4]
				rol		eax, 2
				rol		edx, 2
				mov		ebx, eax
				and		ebx, 2
				dec		ecx
				jnz		myLoop

				ror		edx, 2
				mov		dstBits, edx
			}
#else
			for(uint n=16;n>0;n--)
			{
				// transform the id.
				uint	id= srcBits&3;
				id= lut[id];
				// write.
				dstBits|= id<<30;
				// don't decal last
				if(n>1)
					dstBits>>=2;
			}
#endif

			// store
			((uint32*)dstPix)[1]= dstBits;
		}
		else
			// just copy bits
			((uint32*)dstPix)[1]= ((uint32*)srcPix)[3];
		((uint16*)dstPix)[0]= dstCol0;
		((uint16*)dstPix)[1]= dstCol1;
		// skip.
		srcPix+= 8;
		dstPix+= 8;
	}

	// Must end MMX, for applyHLSMod()
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	if(CSystemInfo::hasMMX())
		_asm	emms;
#endif
}
#ifdef NL_OS_WINDOWS
#pragma managed(pop)
#endif

// ***************************************************************************
#ifdef NL_OS_WINDOWS
#pragma managed(push, off)
#endif
void		CFastHLSModifier::convertDDSBitmapDXTC3Or5(CBitmap &dst, const CBitmap &src, uint8 dh, uint dLum, uint dSat)
{
	uint	W= src.getWidth();
	uint	H= src.getHeight();

	const uint8	*srcPix= &(src.getPixels()[0]);
	uint8		*dstPix= &(dst.getPixels()[0]);
	uint	numBlock= (W*H)/16;

	/*
		NB: don't need to swap color and bits for DXTC3 or DXTC5.
	*/

	// Do not use alpha mask for now.
	for(;numBlock>0;numBlock--)
	{
		uint16	srcCol0= ((uint16*)srcPix)[4];
		uint16	srcCol1= ((uint16*)srcPix)[5];
		// apply modifiers for 2 colors.
		((uint16*)dstPix)[4]= applyHLSMod(srcCol0, dh,dLum,dSat);
		((uint16*)dstPix)[5]= applyHLSMod(srcCol1, dh,dLum,dSat);
		// just copy bits
		((uint32*)dstPix)[3]= ((uint32*)srcPix)[3];
		// copy alpha part.
		((uint32*)dstPix)[0]= ((uint32*)srcPix)[0];
		((uint32*)dstPix)[1]= ((uint32*)srcPix)[1];
		// skip bits and alpha part.
		srcPix+= 16;
		dstPix+= 16;
	}

	// Must end MMX, for applyHLSMod()
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	if(CSystemInfo::hasMMX())
		_asm	emms;
#endif
}
#ifdef NL_OS_WINDOWS
#pragma managed(pop)
#endif

// ***************************************************************************
void		CFastHLSModifier::convertDDSBitmap(CBitmap &dst, const CBitmap &src, uint8 dh, sint dl, sint ds)
{
	nlassert(src.getPixelFormat()==dst.getPixelFormat());
	nlassert(src.getWidth()==dst.getWidth() && src.getHeight()==dst.getHeight());

	// Magic add clamp.
	uint	dLum= 0xFFFFFF00 + dl;
	uint	dSat= 0xFFFFFF00 + ds;

	if(src.getPixelFormat()==CBitmap::DXTC1 || src.getPixelFormat()==CBitmap::DXTC1Alpha)
		convertDDSBitmapDXTC1Or1A(dst, src, dh, dLum, dSat);
	else if(src.getPixelFormat()==CBitmap::DXTC3 || src.getPixelFormat()==CBitmap::DXTC5)
		convertDDSBitmapDXTC3Or5(dst, src, dh, dLum, dSat);
	else
	{
		nlstop;
	}
}


// ***************************************************************************
void		CFastHLSModifier::convertRGBABitmap(CBitmap &dst, const CBitmap &src, uint8 dh, sint dl, sint ds)
{
	nlassert(src.getPixelFormat()==dst.getPixelFormat());
	nlassert(src.getPixelFormat()==CBitmap::RGBA);

	uint	W= src.getWidth();
	uint	H= src.getHeight();

	const CRGBA	*srcPix= (const CRGBA*)&(src.getPixels()[0]);
	CRGBA		*dstPix= (CRGBA*)&(dst.getPixels()[0]);
	uint	numPix= W*H;

	// Do not use alpha mask for now.
	for(;numPix>0;numPix--)
	{
		float	H,L,S;
		srcPix->convertToHLS(H,L,S);
		H*= 256.f/360.f;
		L*= 255.f;
		S*= 255.f;
		H+= dh+0.5f;
		L+= dl+0.5f;
		S+= ds+0.5f;
		clamp(H, 0, 255);
		clamp(L, 0, 255);
		clamp(S, 0, 255);
		uint8	H8= (uint8)NLMISC::OptFastFloor(H);
		uint8	L8= (uint8)NLMISC::OptFastFloor(L);
		uint8	S8= (uint8)NLMISC::OptFastFloor(S);
		*dstPix= convert(H8, L8, S8);
		srcPix++;
		dstPix++;
	}
}


} // NL3D
