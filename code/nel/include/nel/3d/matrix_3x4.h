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

#ifndef NL_MATRIX_3X4_H
#define NL_MATRIX_3X4_H

#include "nel/misc/types_nl.h"


namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// STD Matrix
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/**
 *	For fast vector/point multiplication. Special usage for Skinning.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class NL_ALIGN_SSE2 CMatrix3x4
{
public:
#ifdef NL_HAS_SSE2
	union { struct { float a11, a21, a31; }; __m128 mm1; };
	union { struct { float a12, a22, a32; }; __m128 mm2; };
	union { struct { float a13, a23, a33; }; __m128 mm3; };
	union { struct { float a14, a24, a34; }; __m128 mm4; };
#else
	// Order them in memory line first, for faster memory access.
	float	a11, a12, a13, a14;
	float	a21, a22, a23, a24;
	float	a31, a32, a33, a34;
#endif

	// Copy from a matrix.
	void	set(const CMatrix &mat)
	{
		const float	*m = mat.get();
#ifdef NL_HAS_SSE2
		mm1 = _mm_load_ps(&m[0]);
		mm2 = _mm_load_ps(&m[4]);
		mm3 = _mm_load_ps(&m[8]);
		mm4 = _mm_load_ps(&m[12]);
#else
		a11= m[0]; a12= m[4]; a13= m[8] ; a14= m[12];
		a21= m[1]; a22= m[5]; a23= m[9] ; a24= m[13];
		a31= m[2]; a32= m[6]; a33= m[10]; a34= m[14];
#endif
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &in, CVector &out)
	{
#ifdef NL_HAS_SSE2
		__m128 xxx = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 yyy = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 zzz = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(2, 2, 2, 2));
		out.mm = _mm_mul_ps(mm1, xxx);
		out.mm = _mm_add_ps(out.mm, _mm_mul_ps(mm2, yyy));
		out.mm = _mm_add_ps(out.mm, _mm_mul_ps(mm3, zzz));
#else
		out.x= (a11*in.x + a12*in.y + a13*in.z);
		out.y= (a21*in.x + a22*in.y + a23*in.z);
		out.z= (a31*in.x + a32*in.y + a33*in.z);
#endif
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &in, CVector &out)
	{
#ifdef NL_HAS_SSE2
		__m128 xxx = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 yyy = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 zzz = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(2, 2, 2, 2));
		out.mm = _mm_mul_ps(mm1, xxx);
		out.mm = _mm_add_ps(out.mm, _mm_mul_ps(mm2, yyy));
		out.mm = _mm_add_ps(out.mm, _mm_mul_ps(mm3, zzz));
		out.mm = _mm_add_ps(out.mm, mm4);
#else
		out.x= (a11*in.x + a12*in.y + a13*in.z + a14);
		out.y= (a21*in.x + a22*in.y + a23*in.z + a24);
		out.z= (a31*in.x + a32*in.y + a33*in.z + a34);
#endif
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &in, float scale, CVector &out)
	{
#ifdef NL_HAS_SSE2
		__m128 xxx = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 yyy = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 zzz = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(2, 2, 2, 2));
		out.mm = _mm_mul_ps(mm1, xxx);
		out.mm = _mm_add_ps(out.mm, _mm_mul_ps(mm2, yyy));
		out.mm = _mm_add_ps(out.mm, _mm_mul_ps(mm3, zzz));
		out.mm = _mm_mul_ps(out.mm, _mm_set1_ps(scale));
#else
		out.x= (a11*in.x + a12*in.y + a13*in.z) * scale;
		out.y= (a21*in.x + a22*in.y + a23*in.z) * scale;
		out.z= (a31*in.x + a32*in.y + a33*in.z) * scale;
#endif
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &in, float scale, CVector &out)
	{
#ifdef NL_HAS_SSE2
		__m128 xxx = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 yyy = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 zzz = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(2, 2, 2, 2));
		out.mm = _mm_mul_ps(mm1, xxx);
		out.mm = _mm_add_ps(out.mm, _mm_mul_ps(mm2, yyy));
		out.mm = _mm_add_ps(out.mm, _mm_mul_ps(mm3, zzz));
		out.mm = _mm_add_ps(out.mm, mm4);
		out.mm = _mm_mul_ps(out.mm, _mm_set1_ps(scale));
#else
		out.x= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
		out.y= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
		out.z= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
#endif
	}


	// mulAddvector. NB: in should be different as v!! (else don't work).
	void	mulAddVector(const CVector &in, float scale, CVector &out)
	{
#ifdef NL_HAS_SSE2
		__m128 xxx = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 yyy = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 zzz = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 temp = _mm_mul_ps(mm1, xxx);
		temp = _mm_add_ps(temp, _mm_mul_ps(mm2, yyy));
		temp = _mm_add_ps(temp, _mm_mul_ps(mm3, zzz));
		temp = _mm_mul_ps(temp, _mm_set1_ps(scale));
		out.mm = _mm_add_ps(out.mm, temp);
#else
		out.x+= (a11*in.x + a12*in.y + a13*in.z) * scale;
		out.y+= (a21*in.x + a22*in.y + a23*in.z) * scale;
		out.z+= (a31*in.x + a32*in.y + a33*in.z) * scale;
#endif
	}
	// mulAddpoint. NB: in should be different as v!! (else don't work).
	void	mulAddPoint(const CVector &in, float scale, CVector &out)
	{
#ifdef NL_HAS_SSE2
		__m128 xxx = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 yyy = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 zzz = _mm_shuffle_ps(in.mm, in.mm, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 temp = _mm_mul_ps(mm1, xxx);
		temp = _mm_add_ps(temp, _mm_mul_ps(mm2, yyy));
		temp = _mm_add_ps(temp, _mm_mul_ps(mm3, zzz));
		temp = _mm_add_ps(temp, mm4);
		temp = _mm_mul_ps(temp, _mm_set1_ps(scale));
		out.mm = _mm_add_ps(out.mm, temp);
#else
		out.x+= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
		out.y+= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
		out.z+= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
#endif
	}



};


} // NL3D


#endif // NL_MATRIX_3X4_H

/* End of matrix_3x4.h */
