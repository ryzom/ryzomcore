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
class	CMatrix3x4
{
public:
	// Order them in memory line first, for faster memory access.
	float	a11, a12, a13, a14;
	float	a21, a22, a23, a24;
	float	a31, a32, a33, a34;

	// Copy from a matrix.
	void	set(const CMatrix &mat)
	{
		const float	*m =mat.get();
		a11= m[0]; a12= m[4]; a13= m[8] ; a14= m[12];
		a21= m[1]; a22= m[5]; a23= m[9] ; a24= m[13];
		a31= m[2]; a32= m[6]; a33= m[10]; a34= m[14];
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &in, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z);
		out.y= (a21*in.x + a22*in.y + a23*in.z);
		out.z= (a31*in.x + a32*in.y + a33*in.z);
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &in, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z + a14);
		out.y= (a21*in.x + a22*in.y + a23*in.z + a24);
		out.z= (a31*in.x + a32*in.y + a33*in.z + a34);
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &in, float scale, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z) * scale;
		out.y= (a21*in.x + a22*in.y + a23*in.z) * scale;
		out.z= (a31*in.x + a32*in.y + a33*in.z) * scale;
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &in, float scale, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
		out.y= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
		out.z= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
	}


	// mulAddvector. NB: in should be different as v!! (else don't work).
	void	mulAddVector(const CVector &in, float scale, CVector &out)
	{
		out.x+= (a11*in.x + a12*in.y + a13*in.z) * scale;
		out.y+= (a21*in.x + a22*in.y + a23*in.z) * scale;
		out.z+= (a31*in.x + a32*in.y + a33*in.z) * scale;
	}
	// mulAddpoint. NB: in should be different as v!! (else don't work).
	void	mulAddPoint(const CVector &in, float scale, CVector &out)
	{
		out.x+= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
		out.y+= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
		out.z+= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
	}



};


// ***************************************************************************
// ***************************************************************************
// SSE Matrix
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)


/** For fast vector/point multiplication. Special usage for Skinning.
 *	NB: SSE is no more used (no speed gain, some memory problem), but keep it for possible future usage.
 */
class	CMatrix3x4SSE
{
public:
	// Order them in memory column first, for SSE column multiplication.
	float	a11, a21, a31, a41;
	float	a12, a22, a32, a42;
	float	a13, a23, a33, a43;
	float	a14, a24, a34, a44;

	// Copy from a matrix.
	void	set(const CMatrix &mat)
	{
		const float	*m =mat.get();
		a11= m[0]; a12= m[4]; a13= m[8] ; a14= m[12];
		a21= m[1]; a22= m[5]; a23= m[9] ; a24= m[13];
		a31= m[2]; a32= m[6]; a33= m[10]; a34= m[14];
		// not used.
		a41= 0   ; a42= 0   ; a43= 0    ; a44= 1;
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &vin, CVector &vout)
	{
		__asm
		{
			mov		eax, vin
			mov		ebx, this
			mov		edi, vout
			// Load in vector in op[0]
			movss	xmm0, [eax]vin.x
			movss	xmm1, [eax]vin.y
			movss	xmm2, [eax]vin.z
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			shufps	xmm2, xmm2, 0
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2

			// write the result.
			movss	[edi]vout.x, xmm0
			shufps	xmm0, xmm0, 33
			movss	[edi]vout.y, xmm0
			movhlps	xmm0, xmm0
			movss	[edi]vout.z, xmm0
		}
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &vin, CVector &vout)
	{
		__asm
		{
			mov		eax, vin
			mov		ebx, this
			mov		edi, vout
			// Load in vector in op[0]
			movss	xmm0, [eax]vin.x
			movss	xmm1, [eax]vin.y
			movss	xmm2, [eax]vin.z
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			shufps	xmm2, xmm2, 0
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2
			// Add Matrix translate column vector
			addps	xmm0, [ebx]this.a14

			// write the result.
			movss	[edi]vout.x, xmm0
			shufps	xmm0, xmm0, 33
			movss	[edi]vout.y, xmm0
			movhlps	xmm0, xmm0
			movss	[edi]vout.z, xmm0
		}
	}


	// mulSetvector. NB: vin should be different as v!! (else don't work).
	void	mulSetVector(const CVector &vin, float scale, CVector &vout)
	{
		__asm
		{
			mov		eax, vin
			mov		ebx, this
			mov		edi, vout
			// Load in vector in op[0]
			movss	xmm0, [eax]vin.x
			movss	xmm1, [eax]vin.y
			movss	xmm2, [eax]vin.z
			// Load scale in op[0]
			movss	xmm3, scale
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			shufps	xmm2, xmm2, 0
			shufps	xmm3, xmm3, 0
			// Store vertex column in other regs.
			movaps	xmm5, xmm0
			movaps	xmm6, xmm1
			movaps	xmm7, xmm2
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2

			// mul final result with scale
			mulps	xmm0, xmm3

			// store it in xmm4 for future use.
			movaps	xmm4, xmm0
		}
	}
	// mulSetpoint. NB: vin should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &vin, float scale, CVector &vout)
	{
		__asm
		{
			mov		eax, vin
			mov		ebx, this
			mov		edi, vout
			// Load in vector in op[0]
			movss	xmm0, [eax]vin.x
			movss	xmm1, [eax]vin.y
			movss	xmm2, [eax]vin.z
			// Load scale in op[0]
			movss	xmm3, scale
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			shufps	xmm2, xmm2, 0
			shufps	xmm3, xmm3, 0
			// Store vertex column in other regs.
			movaps	xmm5, xmm0
			movaps	xmm6, xmm1
			movaps	xmm7, xmm2
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2
			// Add Matrix translate column vector
			addps	xmm0, [ebx]this.a14

			// mul final result with scale
			mulps	xmm0, xmm3

			// store it in xmm4 for future use.
			movaps	xmm4, xmm0
		}
	}


	// mulAddvector. NB: vin should be different as v!! (else don't work).
	void	mulAddVector(const CVector &/* vin */, float scale, CVector &vout)
	{
		__asm
		{
			mov		ebx, this
			mov		edi, vout
			// Load vin vector loaded in mulSetVector
			movaps	xmm0, xmm5
			movaps	xmm1, xmm6
			movaps	xmm2, xmm7
			// Load scale in op[0]
			movss	xmm3, scale
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm3, xmm3, 0
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2

			// mul final result with scale
			mulps	xmm0, xmm3

			// Add result, with prec sum.
			addps	xmm0, xmm4

			// store it in xmm4 for future use.
			movaps	xmm4, xmm0

			// write the result.
			movss	[edi]vout.x, xmm0
			shufps	xmm0, xmm0, 33
			movss	[edi]vout.y, xmm0
			movhlps	xmm0, xmm0
			movss	[edi]vout.z, xmm0
		}
	}
	// mulAddpoint. NB: vin should be different as v!! (else don't work).
	void	mulAddPoint(const CVector &/* vin */, float scale, CVector &vout)
	{
		__asm
		{
			mov		ebx, this
			mov		edi, vout
			// Load vin vector loaded in mulSetPoint
			movaps	xmm0, xmm5
			movaps	xmm1, xmm6
			movaps	xmm2, xmm7
			// Load scale in op[0]
			movss	xmm3, scale
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm3, xmm3, 0
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2
			// Add Matrix translate column vector
			addps	xmm0, [ebx]this.a14

			// mul final result with scale
			mulps	xmm0, xmm3

			// Add result, with prec sum.
			addps	xmm0, xmm4

			// store it in xmm4 for future use.
			movaps	xmm4, xmm0

			// write the result.
			movss	[edi]vout.x, xmm0
			shufps	xmm0, xmm0, 33
			movss	[edi]vout.y, xmm0
			movhlps	xmm0, xmm0
			movss	[edi]vout.z, xmm0
		}
	}

};

#else // NL_OS_WINDOWS
/// dummy CMatrix3x4SSE for non windows platform
class CMatrix3x4SSE : public  CMatrix3x4 { };
#endif



} // NL3D


#endif // NL_MATRIX_3X4_H

/* End of matrix_3x4.h */
