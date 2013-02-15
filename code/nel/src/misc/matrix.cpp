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

#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "nel/misc/debug.h"

using namespace std;


#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace	NLMISC
{


// ======================================================================================================
const CMatrix	CMatrix::Identity;


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// State Bits.
#define	MAT_TRANS		1
#define	MAT_ROT			2
#define	MAT_SCALEUNI	4
#define	MAT_SCALEANY	8
#define	MAT_PROJ		16
// Validity bits. These means that the part may be yet identity, but is valid in the floats.
// NB: MAT_VALIDTRANS no more used for faster Pos access
#define	MAT_VALIDROT	64
#define	MAT_VALIDPROJ	128
#define	MAT_VALIDALL	(MAT_VALIDROT | MAT_VALIDPROJ)
// The identity is nothing.
#define	MAT_IDENTITY	0



// Matrix elements.
#define	a11		M[0]
#define	a21		M[1]
#define	a31		M[2]
#define	a41		M[3]
#define	a12		M[4]
#define	a22		M[5]
#define	a32		M[6]
#define	a42		M[7]
#define	a13		M[8]
#define	a23		M[9]
#define	a33		M[10]
#define	a43		M[11]
#define	a14		M[12]
#define	a24		M[13]
#define	a34		M[14]
#define	a44		M[15]



// ======================================================================================================
// ======================================================================================================
// ======================================================================================================



// ======================================================================================================
bool		CMatrix::hasScalePart() const
{
	return (StateBit&(MAT_SCALEUNI|MAT_SCALEANY))!=0;
}
bool		CMatrix::hasProjectionPart() const
{
	return (StateBit&MAT_PROJ)!=0;
}


bool		CMatrix::hasScaleUniform() const
{
	return (StateBit & (MAT_SCALEUNI|MAT_SCALEANY))== MAT_SCALEUNI;
}
float		CMatrix::getScaleUniform() const
{
	if(hasScaleUniform())
		return Scale33;
	else
		return 1;
}



// ======================================================================================================
inline bool	CMatrix::hasRot() const
{
	return (StateBit&(MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY))!=0;
}
inline bool	CMatrix::hasTrans() const
{
	return (StateBit&MAT_TRANS)!=0;
}
inline bool	CMatrix::hasProj() const
{
	return (StateBit&MAT_PROJ)!=0;
}
inline bool	CMatrix::hasAll() const
{
	return (hasRot() && hasTrans() && hasProj());
}


inline void CMatrix::testExpandRot() const
{
	if(hasRot())
		return;
	if(!(StateBit&MAT_VALIDROT))
	{
		CMatrix	*self= const_cast<CMatrix*>(this);
		self->StateBit|=MAT_VALIDROT;
		self->a11= 1; self->a12=0; self->a13=0;
		self->a21= 0; self->a22=1; self->a23=0;
		self->a31= 0; self->a32=0; self->a33=1;
		self->Scale33= 1;
	}
}
inline void CMatrix::testExpandProj() const
{
	if(hasProj())
		return;
	if(!(StateBit&MAT_VALIDPROJ))
	{
		CMatrix	*self= const_cast<CMatrix*>(this);
		self->StateBit|=MAT_VALIDPROJ;
		self->a41=0; self->a42=0; self->a43=0; self->a44=1;
	}
}


// ======================================================================================================
CMatrix::CMatrix(const CMatrix &m)
{
	(*this)= m;
}
// ======================================================================================================
CMatrix		&CMatrix::operator=(const CMatrix &m)
{
	StateBit= m.StateBit & ~MAT_VALIDALL;
	if(hasAll())
	{
		memcpy(M, m.M, 16*sizeof(float));
		Scale33= m.Scale33;
	}
	else
	{
		if(hasRot())
		{
			memcpy(&a11, &m.a11, 3*sizeof(float));
			memcpy(&a12, &m.a12, 3*sizeof(float));
			memcpy(&a13, &m.a13, 3*sizeof(float));
			Scale33= m.Scale33;
		}
		if(hasProj())
		{
			a41= m.a41;
			a42= m.a42;
			a43= m.a43;
			a44= m.a44;
		}
		// Must always copy Trans part.
		memcpy(&a14, &m.a14, 3*sizeof(float));
	}
	return *this;
}


// ======================================================================================================
void		CMatrix::identity()
{
	StateBit= MAT_IDENTITY;
	// Reset just Pos because must always be valid for faster getPos()
	a14= a24= a34= 0;
	// For optimisation it would be useful to keep MAT_VALID states.
	// But this slows identity(), and this may not be interesting...
}
// ======================================================================================================
void		CMatrix::setRot(const CVector &i, const CVector &j, const CVector &k, bool hintNoScale)
{
	StateBit|= MAT_ROT | MAT_SCALEANY;
	if(hintNoScale)
		StateBit&= ~(MAT_SCALEANY|MAT_SCALEUNI);
	a11= i.x; a12= j.x; a13= k.x;
	a21= i.y; a22= j.y; a23= k.y;
	a31= i.z; a32= j.z; a33= k.z;
	Scale33= 1.0f;
}
// ======================================================================================================
void		CMatrix::setRot(const float m33[9], bool hintNoScale)
{
	StateBit|= MAT_ROT | MAT_SCALEANY;
	if(hintNoScale)
		StateBit&= ~(MAT_SCALEANY|MAT_SCALEUNI);
	a11= m33[0]; a12= m33[3]; a13= m33[6];
	a21= m33[1]; a22= m33[4]; a23= m33[7];
	a31= m33[2]; a32= m33[5]; a33= m33[8];
	Scale33= 1.0f;
}
// ======================================================================================================
void		CMatrix::setRot(const CVector &v, TRotOrder ro)
{
	CMatrix		rot;
	rot.identity();
	rot.rotate(v, ro);
	float	m33[9];
	rot.getRot(m33);
	setRot(m33, true);
}


// ======================================================================================================
void		CMatrix::setRot(const CMatrix &matrix)
{
	// copy rotpart statebit from other.
	StateBit&= ~(MAT_ROT | MAT_SCALEUNI | MAT_SCALEANY);
	StateBit|= matrix.StateBit & (MAT_ROT | MAT_SCALEUNI | MAT_SCALEANY);
	// copy values.
	if(hasRot())
	{
		a11= matrix.a11; a12= matrix.a12; a13= matrix.a13;
		a21= matrix.a21; a22= matrix.a22; a23= matrix.a23;
		a31= matrix.a31; a32= matrix.a32; a33= matrix.a33;
		// if has scale uniform, copy from matrix.
		if(hasScaleUniform())
			Scale33= matrix.Scale33;
	}
	else
	{
		// we are rot identity, with undefined values.
		StateBit&= ~MAT_VALIDROT;
	}
}


// ======================================================================================================
void		CMatrix::setPos(const CVector &v)
{
	a14= v.x;
	a24= v.y;
	a34= v.z;
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
		// The trans is identity
		StateBit&= ~MAT_TRANS;
}
// ======================================================================================================
void		CMatrix::movePos(const CVector &v)
{
	a14+= v.x;
	a24+= v.y;
	a34+= v.z;
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
		// The trans is identity
		StateBit&= ~MAT_TRANS;
}
// ======================================================================================================
void		CMatrix::setProj(const float proj[4])
{
	a41= proj[0];
	a42= proj[1];
	a43= proj[2];
	a44= proj[3];

	// Check Proj state.
	if(a41!=0 || a42!=0 || a43!=0 || a44!=1)
		StateBit|= MAT_PROJ;
	else
	{
		// The proj is identity, and is correcly setup!
		StateBit&= ~MAT_PROJ;
		StateBit|= MAT_VALIDPROJ;
	}
}
// ======================================================================================================
void		CMatrix::resetProj()
{
	a41= 0;
	a42= 0;
	a43= 0;
	a44= 1;
	// The proj is identity, and is correcly setup!
	StateBit&= ~MAT_PROJ;
	StateBit|= MAT_VALIDPROJ;
}
// ======================================================================================================
void		CMatrix::set(const float m44[16])
{
	StateBit= MAT_IDENTITY;

	StateBit|= MAT_ROT | MAT_SCALEANY;
	memcpy(M, m44, 16*sizeof(float));
	Scale33= 1.0f;

	// Check Trans state.
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
		// The trans is identity
		StateBit&= ~MAT_TRANS;

	// Check Proj state.
	if(a41!=0 || a42!=0 || a43!=0 || a44!=1)
		StateBit|= MAT_PROJ;
	else
	{
		// The proj is identity, and is correcly setup!
		StateBit&= ~MAT_PROJ;
		StateBit|= MAT_VALIDPROJ;
	}
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::getRot(CVector &i, CVector &j, CVector &k) const
{
	if(hasRot())
	{
		i.set(a11, a21, a31);
		j.set(a12, a22, a32);
		k.set(a13, a23, a33);
	}
	else
	{
		i.set(1, 0, 0);
		j.set(0, 1, 0);
		k.set(0, 0, 1);
	}
}
// ======================================================================================================
void		CMatrix::getRot(float m33[9]) const
{
	if(hasRot())
	{
		m33[0]= a11;
		m33[1]= a21;
		m33[2]= a31;

		m33[3]= a12;
		m33[4]= a22;
		m33[5]= a32;

		m33[6]= a13;
		m33[7]= a23;
		m33[8]= a33;
	}
	else
	{
		m33[0]= 1;
		m33[1]= 0;
		m33[2]= 0;

		m33[3]= 0;
		m33[4]= 1;
		m33[5]= 0;

		m33[6]= 0;
		m33[7]= 0;
		m33[8]= 1;
	}
}
// ======================================================================================================
void		CMatrix::getProj(float proj[4]) const
{
	if(hasProj())
	{
		proj[0]= a41;
		proj[1]= a42;
		proj[2]= a43;
		proj[3]= a44;
	}
	else
	{
		proj[0]= 0;
		proj[1]= 0;
		proj[2]= 0;
		proj[3]= 1;
	}
}
// ======================================================================================================
CVector		CMatrix::getI() const
{
	if(hasRot())
		return CVector(a11, a21, a31);
	else
		return CVector(1, 0, 0);
}
// ======================================================================================================
CVector		CMatrix::getJ() const
{
	if(hasRot())
		return CVector(a12, a22, a32);
	else
		return CVector(0, 1, 0);
}
// ======================================================================================================
CVector		CMatrix::getK() const
{
	if(hasRot())
		return CVector(a13, a23, a33);
	else
		return CVector(0, 0, 1);
}
// ======================================================================================================
void		CMatrix::get(float m44[16]) const
{
	testExpandRot();
	testExpandProj();
	memcpy(m44, M, 16*sizeof(float));
}
// ======================================================================================================
const float *CMatrix::get() const
{
	testExpandRot();
	testExpandProj();
	return M;
}
/*// ======================================================================================================
CVector		CMatrix::toEuler(TRotOrder ro) const
{

}*/


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::translate(const CVector &v)
{
	// SetTrans.
	if( hasRot() )
	{
		a14+= a11*v.x + a12*v.y + a13*v.z;
		a24+= a21*v.x + a22*v.y + a23*v.z;
		a34+= a31*v.x + a32*v.y + a33*v.z;
	}
	else
	{
		a14+= v.x;
		a24+= v.y;
		a34+= v.z;
	}

	// SetProj.
	if( hasProj() )
		a44+= a41*v.x + a42*v.y + a43*v.z;

	// Check Trans.
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
		// The trans is identity, and is correcly setup!
		StateBit&= ~MAT_TRANS;
}
// ======================================================================================================
void		CMatrix::rotateX(float a)
{

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( hasRot() )
	{
		float	b12=a12, b22=a22, b32=a32;
		float	b13=a13, b23=a23, b33=a33;
		a12= (float)(b12*ca + b13*sa);
		a22= (float)(b22*ca + b23*sa);
		a32= (float)(b32*ca + b33*sa);
		a13= (float)(b13*ca - b12*sa);
		a23= (float)(b23*ca - b22*sa);
		a33= (float)(b33*ca - b32*sa);
	}
	else
	{
		testExpandRot();
		a12= 0.0f; a22= (float)ca; a32= (float)sa;
		a13= 0.0f; a23= (float)-sa; a33= (float)ca;
	}

	// SetProj.
	if( hasProj() )
	{
		float	b42=a42, b43=a43;
		a42= (float)(b42*ca + b43*sa);
		a43= (float)(b43*ca - b42*sa);
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotateY(float a)
{

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( hasRot() )
	{
		float	b11=a11, b21=a21, b31=a31;
		float	b13=a13, b23=a23, b33=a33;
		a11= (float)(b11*ca - b13*sa);
		a21= (float)(b21*ca - b23*sa);
		a31= (float)(b31*ca - b33*sa);
		a13= (float)(b13*ca + b11*sa);
		a23= (float)(b23*ca + b21*sa);
		a33= (float)(b33*ca + b31*sa);
	}
	else
	{
		testExpandRot();
		a11= (float)ca; a21=0.0f; a31= (float)-sa;
		a13= (float)sa; a23=0.0f; a33= (float)ca;
	}

	// SetProj.
	if( hasProj() )
	{
		float	b41=a41, b43=a43;
		a41= (float)(b41*ca - b43*sa);
		a43= (float)(b43*ca + b41*sa);
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotateZ(float a)
{

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		float	b11=a11, b21=a21, b31=a31;
		float	b12=a12, b22=a22, b32=a32;
		a11= (float)(b11*ca + b12*sa);
		a21= (float)(b21*ca + b22*sa);
		a31= (float)(b31*ca + b32*sa);
		a12= (float)(b12*ca - b11*sa);
		a22= (float)(b22*ca - b21*sa);
		a32= (float)(b32*ca - b31*sa);
	}
	else
	{
		testExpandRot();
		a11= (float)ca; a21= (float)sa; a31=0.0f;
		a12= (float)-sa; a22= (float)ca; a32=0.0f;
	}

	// SetProj.
	if( hasProj() )
	{
		float	b41=a41, b42=a42;
		a41= (float)(b41*ca + b42*sa);
		a42= (float)(b42*ca - b41*sa);
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotate(const CVector &v, TRotOrder ro)
{
	CMatrix		rot;
	rot.identity();
	switch(ro)
	{
		case XYZ: rot.rotateX(v.x); rot.rotateY(v.y); rot.rotateZ(v.z); break;
		case XZY: rot.rotateX(v.x); rot.rotateZ(v.z); rot.rotateY(v.y); break;
		case YXZ: rot.rotateY(v.y); rot.rotateX(v.x); rot.rotateZ(v.z); break;
		case YZX: rot.rotateY(v.y); rot.rotateZ(v.z); rot.rotateX(v.x); break;
		case ZXY: rot.rotateZ(v.z); rot.rotateX(v.x); rot.rotateY(v.y); break;
		case ZYX: rot.rotateZ(v.z); rot.rotateY(v.y); rot.rotateX(v.x); break;
	}

	(*this)*= rot;
}

// ======================================================================================================
void		CMatrix::rotate(const CQuat &quat)
{
	CMatrix		rot;
	rot.setRot(quat);
	(*this)*= rot;
}

// ======================================================================================================
void		CMatrix::scale(float f)
{

	if(f==1.0f) return;
	if(StateBit & MAT_SCALEANY)
	{
		scale(CVector(f,f,f));
	}
	else
	{
		testExpandRot();
		StateBit|= MAT_SCALEUNI;
		Scale33*=f;
		a11*= f; a12*=f; a13*=f;
		a21*= f; a22*=f; a23*=f;
		a31*= f; a32*=f; a33*=f;

		// SetProj.
		if( hasProj() )
		{
			a41*=f; a42*=f; a43*=f;
		}
	}
}
// ======================================================================================================
void		CMatrix::scale(const CVector &v)
{

	if( v==CVector(1,1,1) ) return;
	if( !(StateBit & MAT_SCALEANY) && v.x==v.y && v.x==v.z)
	{
		scale(v.x);
	}
	else
	{
		testExpandRot();
		StateBit|=MAT_SCALEANY;
		a11*= v.x; a12*=v.y; a13*=v.z;
		a21*= v.x; a22*=v.y; a23*=v.z;
		a31*= v.x; a32*=v.y; a33*=v.z;

		// SetProj.
		if( hasProj() )
		{
			a41*=v.x;
			a42*=v.y;
			a43*=v.z;
		}
	}
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ***************************************************************************
void		CMatrix::setMulMatrixNoProj(const CMatrix &m1, const CMatrix &m2)
{
	/*
	For a fast MulMatrix, it appears to be better to not take State bits into account (no test/if() overhead)
	Just do heavy mul all the time (common case, and not so slow)
	*/

	// Ensure the src matrix have correct values in rot part
	m1.testExpandRot();
	m2.testExpandRot();

	// Rot Mul
	a11= m1.a11*m2.a11 + m1.a12*m2.a21 + m1.a13*m2.a31;
	a12= m1.a11*m2.a12 + m1.a12*m2.a22 + m1.a13*m2.a32;
	a13= m1.a11*m2.a13 + m1.a12*m2.a23 + m1.a13*m2.a33;

	a21= m1.a21*m2.a11 + m1.a22*m2.a21 + m1.a23*m2.a31;
	a22= m1.a21*m2.a12 + m1.a22*m2.a22 + m1.a23*m2.a32;
	a23= m1.a21*m2.a13 + m1.a22*m2.a23 + m1.a23*m2.a33;

	a31= m1.a31*m2.a11 + m1.a32*m2.a21 + m1.a33*m2.a31;
	a32= m1.a31*m2.a12 + m1.a32*m2.a22 + m1.a33*m2.a32;
	a33= m1.a31*m2.a13 + m1.a32*m2.a23 + m1.a33*m2.a33;

	// Trans mul
	a14= m1.a11*m2.a14 + m1.a12*m2.a24 + m1.a13*m2.a34 + m1.a14;
	a24= m1.a21*m2.a14 + m1.a22*m2.a24 + m1.a23*m2.a34 + m1.a24;
	a34= m1.a31*m2.a14 + m1.a32*m2.a24 + m1.a33*m2.a34 + m1.a34;

	// Setup no proj at all, and force valid rot (still may be identity, but 0/1 are filled)
	StateBit= (m1.StateBit | m2.StateBit | MAT_VALIDROT) & ~(MAT_PROJ|MAT_VALIDPROJ);

	// Modify Scale. This test is important because Scale33 may be a #NAN if SCALEANY => avoid very slow mul.
	if( hasScaleUniform() )
		Scale33= m1.Scale33*m2.Scale33;
	else
		Scale33=1;

}


// ***************************************************************************
void		CMatrix::setMulMatrix(const CMatrix &m1, const CMatrix &m2)
{
	// Do *this= m1*m2
	identity();
	StateBit= m1.StateBit | m2.StateBit;
	StateBit&= ~MAT_VALIDALL;

	// Build Rot part.
	//===============
	bool	M1Identity= ! m1.hasRot();
	bool	M2Identity= ! m2.hasRot();
	bool	M1ScaleOnly= ! (m1.StateBit & MAT_ROT);
	bool	M2ScaleOnly= ! (m2.StateBit & MAT_ROT);
	bool	MGeneralCase= !M1Identity && !M2Identity && !M1ScaleOnly && !M2ScaleOnly;


	// Manage the most common general case first (optim the if ): blending of two rotations.
	if( MGeneralCase )
	{
		a11= m1.a11*m2.a11 + m1.a12*m2.a21 + m1.a13*m2.a31;
		a12= m1.a11*m2.a12 + m1.a12*m2.a22 + m1.a13*m2.a32;
		a13= m1.a11*m2.a13 + m1.a12*m2.a23 + m1.a13*m2.a33;

		a21= m1.a21*m2.a11 + m1.a22*m2.a21 + m1.a23*m2.a31;
		a22= m1.a21*m2.a12 + m1.a22*m2.a22 + m1.a23*m2.a32;
		a23= m1.a21*m2.a13 + m1.a22*m2.a23 + m1.a23*m2.a33;

		a31= m1.a31*m2.a11 + m1.a32*m2.a21 + m1.a33*m2.a31;
		a32= m1.a31*m2.a12 + m1.a32*m2.a22 + m1.a33*m2.a32;
		a33= m1.a31*m2.a13 + m1.a32*m2.a23 + m1.a33*m2.a33;
	}
	// If one of the 3x3 matrix is an identity, just do a copy
	else if( M1Identity || M2Identity )
	{
		// If both identity, then me too.
		if( M1Identity && M2Identity )
		{
			// just expand me (important because validated below)
			testExpandRot();
		}
		else
		{
			// Copy the non identity matrix.
			const CMatrix	*c= M2Identity? &m1 : &m2;
			a11= c->a11; a12= c->a12; a13= c->a13;
			a21= c->a21; a22= c->a22; a23= c->a23;
			a31= c->a31; a32= c->a32; a33= c->a33;
		}
	}
	// If two 3x3 matrix are just scaleOnly matrix, do a scaleFact.
	else if( M1ScaleOnly && M2ScaleOnly )
	{
		// same process for scaleUni or scaleAny.
		a11= m1.a11*m2.a11; a12= 0; a13= 0;
		a21= 0; a22= m1.a22*m2.a22; a23= 0;
		a31= 0; a32= 0; a33= m1.a33*m2.a33;
	}
	// If one of the matrix is a scaleOnly matrix, do a scale*Rot.
	else if( M1ScaleOnly && !M2ScaleOnly )
	{
		a11= m1.a11*m2.a11; a12= m1.a11*m2.a12; a13= m1.a11*m2.a13;
		a21= m1.a22*m2.a21; a22= m1.a22*m2.a22; a23= m1.a22*m2.a23;
		a31= m1.a33*m2.a31; a32= m1.a33*m2.a32; a33= m1.a33*m2.a33;
	}
	else
	{
		// This must be this case
		nlassert(!M1ScaleOnly && M2ScaleOnly);
		a11= m1.a11*m2.a11; a12= m1.a12*m2.a22; a13= m1.a13*m2.a33;
		a21= m1.a21*m2.a11; a22= m1.a22*m2.a22; a23= m1.a23*m2.a33;
		a31= m1.a31*m2.a11; a32= m1.a32*m2.a22; a33= m1.a33*m2.a33;
	}

	// If M1 has translate and M2 has projective, rotation is modified.
	if( m1.hasTrans() && m2.hasProj())
	{
		StateBit|= MAT_ROT|MAT_SCALEANY;

		a11+= m1.a14*m2.a41;
		a12+= m1.a14*m2.a42;
		a13+= m1.a14*m2.a43;

		a21+= m1.a24*m2.a41;
		a22+= m1.a24*m2.a42;
		a23+= m1.a24*m2.a43;

		a31+= m1.a34*m2.a41;
		a32+= m1.a34*m2.a42;
		a33+= m1.a34*m2.a43;
	}

	// Modify Scale.
	if( (StateBit & MAT_SCALEUNI) && !(StateBit & MAT_SCALEANY) )
	{
		// Must have correct Scale33
		m1.testExpandRot();
		m2.testExpandRot();
		Scale33= m1.Scale33*m2.Scale33;
	}
	else
		Scale33=1;

	// In every case, I am valid now!
	StateBit|=MAT_VALIDROT;


	// Build Trans part.
	//=================
	if( StateBit & MAT_TRANS )
	{
		// Compose M2 part.
		if( M1Identity )
		{
			a14= m2.a14;
			a24= m2.a24;
			a34= m2.a34;
		}
		else if (M1ScaleOnly )
		{
			a14= m1.a11*m2.a14;
			a24= m1.a22*m2.a24;
			a34= m1.a33*m2.a34;
		}
		else
		{
			a14= m1.a11*m2.a14 + m1.a12*m2.a24 + m1.a13*m2.a34;
			a24= m1.a21*m2.a14 + m1.a22*m2.a24 + m1.a23*m2.a34;
			a34= m1.a31*m2.a14 + m1.a32*m2.a24 + m1.a33*m2.a34;
		}
		// Compose M1 part.
		if(m1.StateBit & MAT_TRANS)
		{
			if(m2.StateBit & MAT_PROJ)
			{
				a14+= m1.a14*m2.a44;
				a24+= m1.a24*m2.a44;
				a34+= m1.a34*m2.a44;
			}
			else
			{
				a14+= m1.a14;
				a24+= m1.a24;
				a34+= m1.a34;
			}
		}
	}


	// Build Proj part.
	//=================
	if( StateBit & MAT_PROJ )
	{
		// optimize nothing... (projection matrix are rare).
		m1.testExpandRot();
		m1.testExpandProj();
		m2.testExpandRot();
		m2.testExpandProj();
		a41= m1.a41*m2.a11 + m1.a42*m2.a21 + m1.a43*m2.a31 + m1.a44*m2.a41;
		a42= m1.a41*m2.a12 + m1.a42*m2.a22 + m1.a43*m2.a32 + m1.a44*m2.a42;
		a43= m1.a41*m2.a13 + m1.a42*m2.a23 + m1.a43*m2.a33 + m1.a44*m2.a43;
		a44= m1.a41*m2.a14 + m1.a42*m2.a24 + m1.a43*m2.a34 + m1.a44*m2.a44;
		// The proj is valid now
		StateBit|= MAT_VALIDPROJ;
	}
	else
	{
		// Don't copy proj part, and leave MAT_VALIDPROJ not set
	}
}
// ======================================================================================================
void		CMatrix::invert()
{

	*this= inverted();
}


// ======================================================================================================
void		CMatrix::transpose3x3()
{
	if(hasRot())
	{
		// swap values.
		swap(a12, a21);
		swap(a13, a31);
		swap(a32, a23);
		// Scale mode (none, uni, or any) is conserved. Scale33 too...
	}
}

// ======================================================================================================
void		CMatrix::transpose()
{
	transpose3x3();
	if(hasTrans() || hasProj())
	{
		// if necessary, Get valid 0 on proj part.
		testExpandProj();
		// swap values
		swap(a41, a14);
		swap(a42, a24);
		swap(a43, a34);
		// swap StateBit flags, if not both were sets...
		if(!hasTrans() || !hasProj())
		{
			// swap StateBit flags (swap trans with proj).
			if(hasTrans())
			{
				StateBit&= ~MAT_TRANS;
				StateBit|= MAT_PROJ;
			}
			else
			{
				StateBit&= ~MAT_PROJ;
				StateBit|= MAT_TRANS;
			}
		}
		// reset validity. NB, maybe not useful, but simpler, and bugfree.
		StateBit&= ~(MAT_VALIDPROJ);
	}
	// NB: if no Trans or no Proj, do nothing, so don't need to modify VALIDTRANS and VALIDPROJ too.
}


// ======================================================================================================
bool	CMatrix::fastInvert33(CMatrix &ret) const
{
	// Fast invert of 3x3 rot matrix.
	// Work if no scale and if MAT_SCALEUNI. doesn't work if MAT_SCALEANY.

	if(StateBit & MAT_SCALEUNI)
	{
		if (Scale33 == 0.f) return false;
		double	s,S;	// important for precision.
		// Must divide the matrix by 1/Scale 2 times, to set unit, and to have a Scale=1/Scale.
		S=1.0/Scale33;
		ret.Scale33= (float)S;
		s=S*S;
		// The matrix is a base, so just transpose it.
		ret.a11= (float)(a11*s); ret.a12= (float)(a21*s); ret.a13= (float)(a31*s);
		ret.a21= (float)(a12*s); ret.a22= (float)(a22*s); ret.a23= (float)(a32*s);
		ret.a31= (float)(a13*s); ret.a32= (float)(a23*s); ret.a33= (float)(a33*s);
	}
	else
	{
		ret.Scale33=1;
		// The matrix is a base, so just transpose it.
		ret.a11= a11; ret.a12= a21; ret.a13=a31;
		ret.a21= a12; ret.a22= a22; ret.a23=a32;
		ret.a31= a13; ret.a32= a23; ret.a33=a33;
	}
	return true;
	// 15 cycles if no scale.
	// 35 cycles if scale.
}
// ======================================================================================================
bool	CMatrix::slowInvert33(CMatrix &ret) const
{
	CVector	invi,invj,invk;
	CVector	i,j,k;
	double	s;

	i= getI();
	j= getJ();
	k= getK();
	// Compute cofactors (minors *(-1)^(i+j)).
	invi.x= j.y*k.z - k.y*j.z;
	invi.y= j.z*k.x - k.z*j.x;
	invi.z= j.x*k.y - k.x*j.y;
	invj.x= k.y*i.z - i.y*k.z;
	invj.y= k.z*i.x - i.z*k.x;
	invj.z= k.x*i.y - i.x*k.y;
	invk.x= i.y*j.z - j.y*i.z;
	invk.y= i.z*j.x - j.z*i.x;
	invk.z= i.x*j.y - j.x*i.y;
	// compute determinant.
	s= invi.x*i.x + invj.x*j.x + invk.x*k.x;
	if(s==0)
		return false;
	// Transpose the Comatrice, and divide by determinant.
	s=1.0/s;
	ret.a11= (float)(invi.x*s); ret.a12= (float)(invi.y*s); ret.a13= (float)(invi.z*s);
	ret.a21= (float)(invj.x*s); ret.a22= (float)(invj.y*s); ret.a23= (float)(invj.z*s);
	ret.a31= (float)(invk.x*s); ret.a32= (float)(invk.y*s); ret.a33= (float)(invk.z*s);

	return true;
	// Roundly 82 cycles. (1Div=10 cycles).
}
// ======================================================================================================
bool	CMatrix::slowInvert44(CMatrix &ret) const
{
	sint	i,j;
	double	s;

	// Compute Cofactors
	//==================
	for(i=0;i<=3;i++)
	{
		for(j=0;j<=3;j++)
		{
			sint	l1=0,l2=0,l3=0;
			sint	c1,c2,c3;
			getCofactIndex(i,l1,l2,l3);
			getCofactIndex(j,c1,c2,c3);

			ret.mat(i,j)= 0;
			ret.mat(i,j)+= mat(l1,c1) * mat(l2,c2) * mat(l3,c3);
			ret.mat(i,j)+= mat(l1,c2) * mat(l2,c3) * mat(l3,c1);
			ret.mat(i,j)+= mat(l1,c3) * mat(l2,c1) * mat(l3,c2);

			ret.mat(i,j)-= mat(l1,c1) * mat(l2,c3) * mat(l3,c2);
			ret.mat(i,j)-= mat(l1,c2) * mat(l2,c1) * mat(l3,c3);
			ret.mat(i,j)-= mat(l1,c3) * mat(l2,c2) * mat(l3,c1);

			if( (i+j)&1 )
				ret.mat(i,j)=-ret.mat(i,j);
		}
	}

	// Compute determinant.
	//=====================
	s= ret.mat(0,0) * mat(0,0) + ret.mat(0,1) * mat(0,1) + ret.mat(0,2) * mat(0,2) + ret.mat(0,3) * mat(0,3);
	if(s==0)
		return false;

	// Divide by determinant.
	//=======================
	s=1.0/s;
	for(i=0;i<=3;i++)
	{
		for(j=0;j<=3;j++)
			ret.mat(i,j)= (float)(ret.mat(i,j)*s);
	}

	// Transpose the comatrice.
	//=========================
	for(i=0;i<=3;i++)
	{
		for(j=i+1;j<=3;j++)
		{
			swap(ret.mat(i,j), ret.mat(j,i));
		}
	}

	return true;
}
// ======================================================================================================
CMatrix		CMatrix::inverted() const
{

	CMatrix	ret;

	testExpandRot();
	testExpandProj();

	// Do a conventionnal 44 inversion.
	//=================================
	if(StateBit & MAT_PROJ)
	{
		if(!slowInvert44(ret))
		{
			ret.identity();
			return ret;
		}

		// Well, don't know what happens to matrix, so set all StateBit :).
		ret.StateBit= MAT_TRANS|MAT_ROT|MAT_SCALEANY|MAT_PROJ;

		// Check Trans state.
		if(ret.a14!=0 || ret.a24!=0 || ret.a34!=0)
			ret.StateBit|= MAT_TRANS;
		else
			ret.StateBit&= ~MAT_TRANS;

		// Check Proj state.
		if(ret.a41!=0 || ret.a42!=0 || ret.a43!=0 || ret.a44!=1)
			ret.StateBit|= MAT_PROJ;
		else
			ret.StateBit&= ~MAT_PROJ;
	}

	// Do a speed 34 inversion.
	//=========================
	else
	{
		// Invert the rotation part.
		if(StateBit & MAT_SCALEANY)
		{
			if(!slowInvert33(ret))
			{
				ret.identity();
				return ret;
			}
		}
		else
		{
			if (!fastInvert33(ret))
			{
				ret.identity();
				return ret;
			}
		}
		// Scale33 is updated in fastInvert33().

		// Invert the translation part.
		if(StateBit & MAT_TRANS)
		{
			// Invert the translation part.
			// This can only work if 4th line is 0 0 0 1.
			// formula: InvVp= InvVi*(-Vp.x) + InvVj*(-Vp.y) + InvVk*(-Vp.z)
			ret.a14= ret.a11*(-a14) + ret.a12*(-a24) + ret.a13*(-a34);
			ret.a24= ret.a21*(-a14) + ret.a22*(-a24) + ret.a23*(-a34);
			ret.a34= ret.a31*(-a14) + ret.a32*(-a24) + ret.a33*(-a34);
		}
		else
		{
			ret.a14= 0;
			ret.a24= 0;
			ret.a34= 0;
		}

		// The projection part is unmodified.
		ret.a41= 0; ret.a42= 0; ret.a43= 0; ret.a44= 1;

		// The matrix inverted keep the same state bits.
		ret.StateBit= StateBit;
	}


	return ret;
}
// ======================================================================================================
bool		CMatrix::normalize(TRotOrder ro)
{

	CVector	ti,tj,tk;
	ti= getI();
	tj= getJ();
	tk= getK();

	testExpandRot();

	// Normalize with help of ro
	switch(ro)
	{
		case XYZ:
			ti.normalize();
			tk= ti^tj;
			tk.normalize();
			tj= tk^ti;
			break;
		case XZY:
			ti.normalize();
			tj= tk^ti;
			tj.normalize();
			tk= ti^tj;
			break;
		case YXZ:
			tj.normalize();
			tk= ti^tj;
			tk.normalize();
			ti= tj^tk;
			break;
		case YZX:
			tj.normalize();
			ti= tj^tk;
			ti.normalize();
			tk= ti^tj;
			break;
		case ZXY:
			tk.normalize();
			tj= tk^ti;
			tj.normalize();
			ti= tj^tk;
			break;
		case ZYX:
			tk.normalize();
			ti= tj^tk;
			ti.normalize();
			tj= tk^ti;
			break;
	}

	// Check, and set result.
	if( ti.isNull() || tj.isNull() || tk.isNull() )
		return false;
	a11= ti.x; a12= tj.x; a13= tk.x;
	a21= ti.y; a22= tj.y; a23= tk.y;
	a31= ti.z; a32= tj.z; a33= tk.z;
	// Scale is reseted.
	StateBit&= ~(MAT_SCALEUNI|MAT_SCALEANY);
	// Rot is setup...
	StateBit|= MAT_ROT;
	Scale33=1;

	return true;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
CVector		CMatrix::mulVector(const CVector &v) const
{

	CVector	ret;

	if( hasRot() )
	{
		ret.x= a11*v.x + a12*v.y + a13*v.z;
		ret.y= a21*v.x + a22*v.y + a23*v.z;
		ret.z= a31*v.x + a32*v.y + a33*v.z;
		return ret;
	}
	else
		return v;
}

// ======================================================================================================
CVector		CMatrix::mulPoint(const CVector &v) const
{

	CVector	ret;

	if( hasRot() )
	{
		ret.x= a11*v.x + a12*v.y + a13*v.z;
		ret.y= a21*v.x + a22*v.y + a23*v.z;
		ret.z= a31*v.x + a32*v.y + a33*v.z;
	}
	else
	{
		ret= v;
	}
	if( hasTrans() )
	{
		ret.x+= a14;
		ret.y+= a24;
		ret.z+= a34;
	}

	return ret;
}


/*
 * Multiply
 */
CVectorH	CMatrix::operator*(const CVectorH& v) const
{

	CVectorH ret;

	testExpandRot();
	testExpandProj();

	ret.x= a11*v.x + a12*v.y + a13*v.z + a14*v.w;
	ret.y= a21*v.x + a22*v.y + a23*v.z + a24*v.w;
	ret.z= a31*v.x + a32*v.y + a33*v.z + a34*v.w;
	ret.w= a41*v.x + a42*v.y + a43*v.z + a44*v.w;
	return ret;
}


// ======================================================================================================
CPlane		operator*(const CPlane &p, const CMatrix &m)
{
	m.testExpandRot();
	m.testExpandProj();

	CPlane	ret;

	if( m.StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY|MAT_PROJ) )
	{
		// Compose with translation too.
		ret.a= p.a*m.a11 + p.b*m.a21 + p.c*m.a31 + p.d*m.a41;
		ret.b= p.a*m.a12 + p.b*m.a22 + p.c*m.a32 + p.d*m.a42;
		ret.c= p.a*m.a13 + p.b*m.a23 + p.c*m.a33 + p.d*m.a43;
		ret.d= p.a*m.a14 + p.b*m.a24 + p.c*m.a34 + p.d*m.a44;
		return ret;
	}
	else if( m.StateBit & MAT_TRANS )
	{

		// Compose just with a translation.
		ret.a= p.a;
		ret.b= p.b;
		ret.c= p.c;
		ret.d= p.a*m.a14 + p.b*m.a24 + p.c*m.a34 + p.d*m.a44;
		return ret;
	}
	else	// Identity!!
		return p;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::setRot(const CQuat &quat)
{
	// A quaternion do not have scale.
	StateBit&= ~(MAT_ROT | MAT_SCALEANY|MAT_SCALEUNI);
	Scale33= 1.0f;
	if(quat.isIdentity())
	{
		a11= 1; a12= 0; a13= 0;
		a21= 0; a22= 1; a23= 0;
		a31= 0; a32= 0; a33= 1;
	}
	else
	{
		StateBit|= MAT_ROT;
		float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

		// calculate coefficients
		x2 = quat.x + quat.x; y2 = quat.y + quat.y;
		z2 = quat.z + quat.z;
		xx = quat.x * x2;   xy = quat.x * y2;   xz = quat.x * z2;
		yy = quat.y * y2;   yz = quat.y * z2;   zz = quat.z * z2;
		wx = quat.w * x2;   wy = quat.w * y2;   wz = quat.w * z2;

		a11 = 1.0f - (yy + zz);
		a12 = xy - wz;
		a13 = xz + wy;

		a21 = xy + wz;
		a22 = 1.0f - (xx + zz);
		a23 = yz - wx;

		a31 = xz - wy;
		a32 = yz + wx;
		a33 = 1.0f - (xx + yy);
	}
}


// ======================================================================================================
void		CMatrix::getRot(CQuat &quat) const
{
	const CMatrix	*pmat= this;
	CMatrix			MatNormed;


	// Rot Indentity?
	if(! (StateBit & MAT_ROT))
	{
		quat= CQuat::Identity;
		return;
	}

	// Must normalize the matrix??
	if(StateBit & (MAT_SCALEUNI | MAT_SCALEANY) )
	{
		MatNormed= *this;
		MatNormed.normalize(ZYX);
		pmat= &MatNormed;
	}

	// Compute quaternion.
	float  tr, s, q[4];

	tr = pmat->a11 + pmat->a22 + pmat->a33;

	// check the diagonal
	if (tr > 0.0)
	{
		s = (float)sqrt (tr + 1.0f);
		quat.w = s / 2.0f;
		s = 0.5f / s;
		quat.x = (pmat->a32 - pmat->a23) * s;
		quat.y = (pmat->a13 - pmat->a31) * s;
		quat.z = (pmat->a21 - pmat->a12) * s;
	}
	else
	{
		sint    i, j, k;
		sint	nxt[3] = {1, 2, 0};

		// diagonal is negative
		i = 0;
		if (pmat->a22 > pmat->a11) i = 1;
		if (pmat->a33 > pmat->mat(i,i) ) i = 2;
		j = nxt[i];
		k = nxt[j];

		s = (float) sqrt (  (pmat->mat(i,i) - (pmat->mat(j,j) + pmat->mat(k,k)) )   + 1.0);

		q[i] = s * 0.5f;

		if (s != 0.0f) s = 0.5f / s;

		q[j] = (pmat->mat(j,i) + pmat->mat(i,j)) * s;
		q[k] = (pmat->mat(k,i) + pmat->mat(i,k)) * s;
		q[3] =	 (pmat->mat(k,j) - pmat->mat(j,k)) * s;

		quat.x = q[0];
		quat.y = q[1];
		quat.z = q[2];
		quat.w = q[3];
	}

}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
inline	void	CMatrix::setScaleUni(float scale)
{
	// A Scale matrix do not have rotation.
	StateBit&= ~(MAT_ROT | MAT_SCALEANY | MAT_SCALEUNI);
	StateBit|= MAT_SCALEUNI | MAT_VALIDROT;
	Scale33= scale;
	a11= scale; a12= 0; a13= 0;
	a21= 0; a22= scale; a23= 0;
	a31= 0; a32= 0; a33= scale;
}

// ======================================================================================================
void		CMatrix::setScale(float scale)
{
	setScaleUni(scale);
}


// ======================================================================================================
void		CMatrix::setScale(const CVector &v)
{
	// actually a scale uniform?
	if(v.x==v.y && v.x==v.z)
		setScaleUni(v.x);

	// A Scale matrix do not have rotation.
	StateBit&= ~(MAT_ROT | MAT_SCALEANY | MAT_SCALEUNI);
	StateBit|= MAT_SCALEANY | MAT_VALIDROT;
	a11= v.x; a12= 0; a13= 0;
	a21= 0; a22= v.y; a23= 0;
	a31= 0; a32= 0; a33= v.z;

}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::serial(IStream &f)
{
	// Use versionning, maybe for futur improvement.
	(void)f.serialVersion(0);

	if(f.isReading())
		identity();
	f.serial(StateBit);
	// avoid serial of random data
	if(!f.isReading() && !hasScaleUniform())
	{
		float	fs= 1.f;
		f.serial(fs);
	}
	else
		f.serial(Scale33);
	if( hasRot() )
	{
		f.serial(a11, a12, a13);
		f.serial(a21, a22, a23);
		f.serial(a31, a32, a33);
	}
	if( hasTrans() )
	{
		f.serial(a14, a24, a34);
	}
	else if(f.isReading())
	{
		// must reset because Pos must always be valid
		a14= a24= a34= 0;
	}
	if( hasProj() )
	{
		f.serial(a41, a42, a43, a44);
	}
}


// ======================================================================================================
void		CMatrix::setArbitraryRotI(const CVector &idir)
{
	// avoid gimbal lock. if idir == nearly K, use another second lead vector
	if( fabs(idir.z)<0.9f )
		setRot(idir, CVector::J, CVector::K);
	else
		setRot(idir, CVector::J, CVector::I);
	normalize(CMatrix::XZY);
}

void		CMatrix::setArbitraryRotJ(const CVector &jdir)
{
	// avoid gimbal lock. if jdir == nearly K, use another second lead vector
	if(fabs(jdir.z)<0.9f)
		setRot(CVector::I, jdir, CVector::K);
	else
		setRot(CVector::I, jdir, CVector::J);
	normalize(CMatrix::YZX);
}

void		CMatrix::setArbitraryRotK(const CVector &kdir)
{
	// avoid gimbal lock. if kdir == nearly I, use another second lead vector
	if( fabs(kdir.y)<0.9f )
		setRot(CVector::I, CVector::J, kdir);
	else
		setRot(CVector::I, CVector::K, kdir);
	normalize(CMatrix::ZYX);
}




}

