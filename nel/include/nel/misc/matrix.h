// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_MATRIX_H
#define NL_MATRIX_H

#include "vector.h"
#include "vector_h.h"
#include "quat.h"


namespace	NLMISC
{

class	CPlane;


// ======================================================================================================
/**
 * A 4*4 Homogeneous Matrix.
 * This is a column matrix, so operations like: \c v1=A*B*C*v0; applies C first , then B, then A to vector v0. \n
 * Since it is a column matrix, the first column is the I vector of the base, 2nd is J, 3th is K. \n
 * 4th column vector is T, the translation vector.
 *
 * Angle orientation are: Xaxis: YtoZ. Yaxis: ZtoX. Zaxis: XtoY.
 *
 * This matrix keep a matrix state to improve Matrix, vector and plane computing (matrix inversion, vector multiplication...).
 * The internal matrix know if:
 * - matrix is identity
 * - matrix has a translation component
 * - matrix has a rotation component
 * - matrix has a uniform scale component (scale which is the same along the 3 axis)
 * - matrix has a non-uniform scale component
 * - matrix has a projection component (4th line of the matrix is not 0 0 0 1).
 *
 * An example of improvement is that CMatrix::operator*(const CVector &v) return v if the matrix is identity.
 *
 * By default, a matrix is identity. But for a performance view, this is just a StateBit=0...
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */

class NL_ALIGN_SSE2 CMatrix
{
public:
	/// Rotation Order.
	enum	TRotOrder
	{
		XYZ,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};

	/// The identity matrix. Same as CMatrix().
	static	const CMatrix	Identity;

public:

	/// \name Object
	//@{
	/// Constructor which init to identity().
	CMatrix()
	{
		StateBit= 0;
		// Init just Pos because must always be valid for faster getPos()
		M[12]= M[13]= M[14]= 0;
	}
	/// Copy Constructor.
	CMatrix(const CMatrix &);
	/// operator=.
	CMatrix	&operator=(const CMatrix &);
	//@}



	/// \name Sets
	//@{
	/// Reset the matrix to identity.
	void		identity();
	/** Explicit setup the Rotation/Scale matrix (base).
	 * Avoid it. It implies low compute since no check is done on base to see what type of matrix it is
	 * (identity, rotation, scale, uniform scale...)
	 * \param i The I vector of the Cartesian base.
	 * \param j The J vector of the Cartesian base.
	 * \param k The K vector of the Cartesian base.
	 * \param hintNoScale set it to true if you are sure that your rot matrix is a pure rot matrix with no scale.
	 * If set to true and your rotation is not an orthonormal basis, unpredictable result are excepted.
	 */
	void		setRot(const CVector &i, const CVector &j, const CVector &k, bool hintNoScale=false);
	/** Explicit setup the Rotation/Scale matrix (base).
	 * Avoid it. It implies low compute since no check is done on m33 to see what type of matrix it is
	 * (identity, rotation, scale, uniform scale)
	 * \param m33 the 3*3 column rotation matrix. (3x3 matrix stored in column-major order as 9 consecutive values)
	 * \param hintNoScale set it to true if you are sure that your rot matrix is a pure rot matrix with no scale.
	 * If set to true and your rotation is not an orthonormal basis, unpredictable result are excepted.
	 */
	void		setRot(const float m33[9], bool hintNoScale=false);
	/** Explicit setup the Rotation matrix (base) as a Euler rotation matrix.
	 * \param v a vector of 3 angle (in radian), giving rotation around each axis (x,y,z)
	 * \param ro the order of transformation applied. if ro==XYZ, then the transform is M=M*Rx*Ry*Rz
	 */
	void		setRot(const CVector &v, TRotOrder ro);
	/** Explicit setup the Rotation matrix (base) as a Quaternion rotation matrix.
	 * \param quat a UNIT quaternion
	 */
	void		setRot(const CQuat &quat);
	/** Explicit setup the Rotation/Scale matrix (base) with the rotation part of another matrix.
	 * \param matrix the matrix to copy rot part.
	 */
	void		setRot(const CMatrix &matrix);
	/** Explicit setup the Rotation/Scale matrix (base) with a scale (=> matrix has no Rotation).
	 *	1 is tested to update bits accordingly
	 * \param scale the scale to set
	 */
	void		setScale(float scale);
	/** Explicit setup the Rotation/Scale matrix (base) with a scale (=> matrix has no Rotation).
	 *	case where v.x==v.y==v.z is tested to set a uniform scale
	 * \param scale the scale to set
	 */
	void		setScale(const CVector &v);
	/** Explicit setup the Translation component.
	 * v==Null is tested to see if the matrix now have a translation component.
	 * \param v the translation vector.
	 */
	void		setPos(const CVector &v);
	/** Explicit move the Translation component.
	 * \param v a vector to move the translation vector.
	 */
	void		movePos(const CVector &v);
	/** Explicit setup the Projection component.
	 * Proj is tested to see if the matrix now have a projection component.
	 * \param proj the 4th line of the matrix. Set it to 0 0 0 1 to reset it to default.
	 */
	void		setProj(const float proj[4]);
	/** Reset the Projection component to 0 0 0 1.
	 */
	void		resetProj();
	/** Explicit setup the 4*4 matrix.
	 * Avoid it. It implies low compute since no check is done on rotation matrix to see what type of matrix it is
	 * (identity, rotation, scale, uniform scale).
	 * BUT check are made to see if it has translation or projection components.
	 * \param m44 the 4*4 column matrix (4x4 matrix stored in column-major order as 16 consecutive values)
	 */
	void		set(const float m44[16]);
	/** Setup the (i, j) matrix coefficient
	 *	\param coeff: coefficient value.
	 *	\param i : column index.
	 *	\param j : line index.
	 */
	void		setCoefficient(float coeff, sint i, sint j)
	{
		M[ (j<<2) + i] = coeff;
	}
	//@}
	/** Choose an arbitrary rotation matrix for the given direction. The matrix will have I==idir
	 *	\param idir: vector direction. MUST  be normalized.
	 */

	void		setArbitraryRotI(const CVector &idir);
	/** Choose an arbitrary rotation matrix for the given direction. The matrix will have J==jdir
	 *	\param jdir: vector direction. MUST  be normalized.
	 */
	void		setArbitraryRotJ(const CVector &jdir);
	/** Choose an arbitrary rotation matrix for the given direction. The matrix will have K==kdir
	 *	\param kdir: vector direction. MUST  be normalized.
	 */
	void		setArbitraryRotK(const CVector &kdir);
	//@}




	/// \name Gets.
	//@{
	/** Get the Rotation/Scale matrix (base).
	 * \param i The matrix's I vector of the Cartesian base.
	 * \param j The matrix's J vector of the Cartesian base.
	 * \param k The matrix's K vector of the Cartesian base.
	 */
	void		getRot(CVector &i, CVector &j, CVector &k) const;
	/** Get the Rotation/Scale matrix (base).
	 * \param m33 the matrix's 3*3 column rotation matrix. (3x3 matrix stored in column-major order as 9 consecutive values)
	 */
	void		getRot(float m33[9]) const;
	/** Get the Rotation matrix (base).
	 * \param quat the return quaternion.
	 */
	void		getRot(CQuat &quat) const;
	/** Get the Rotation matrix (base).
	 * \param quat the return quaternion.
	 */
	CQuat		getRot() const {CQuat	ret; getRot(ret); return ret;}
	/** Get the Translation component.
	 * \param v the matrix's translation vector.
	 */
	void			getPos(CVector &v) const	{v.x= M[12]; v.y= M[13]; v.z= M[14];}
	/** Get the Translation component.
	 *	NB: a const & works because it is a column vector
	 * \return the matrix's translation vector.
	 */
	const CVector	&getPos() const				{return *(CVector*)(M+12);}
	/** Get the Projection component.
	 * \param proj the matrix's projection vector.
	 */
	void		getProj(float proj[4]) const;
	/// Get the I vector of the Rotation/Scale matrix (base).
	CVector		getI() const;
	/// Get the J vector of the Rotation/Scale matrix (base).
	CVector		getJ() const;
	/// Get the K vector of the Rotation/Scale matrix (base).
	CVector		getK() const;
	/** Get 4*4 matrix.
	 * \param m44 the matrix's 4*4 column matrix (4x4 matrix stored in column-major order as 16 consecutive values)
	 */
	void		get(float m44[16]) const;
	/** Get 4*4 matrix.
	 * \return the matrix's 4*4 column matrix (4x4 matrix stored in column-major order as 16 consecutive values)
	 */
	const float *get() const;
	//@}



	/// \name 3D Operations.
	//@{
	/// Apply a translation to the matrix. same as M=M*T
	void		translate(const CVector &v);
	/** Apply a rotation on axis X to the matrix. same as M=M*Rx
	 * \param a angle (in radian).
	 */
	void		rotateX(float a);
	/** Apply a rotation on axis Y to the matrix. same as M=M*Ry
	 * \param a angle (in radian).
	 */
	void		rotateY(float a);
	/** Apply a rotation on axis Z to the matrix. same as M=M*Rz
	 * \param a angle (in radian).
	 */
	void		rotateZ(float a);
	/** Apply a Euler rotation.
	 * \param v a vector of 3 angle (in radian), giving rotation around each axis (x,y,z)
	 * \param ro the order of transformation applied. if ro==XYZ, then the transform is M=M*Rx*Ry*Rz
	 */
	void		rotate(const CVector &v, TRotOrder ro);
	/** Apply a quaternion rotation.
	 */
	void		rotate(const CQuat &quat);
	/// Apply a uniform scale to the matrix.
	void		scale(float f);
	/// Apply a non-uniform scale to the matrix.
	void		scale(const CVector &scale);
	//@}



	/// \name Matrix Operations.
	//@{
	/** Matrix multiplication. Because of copy avoidance, this is the fastest method
	 *	Equivalent to *this= m1 * m2
	 *	\warning *this MUST NOT be the same as m1 or m2, else it doesn't work (not checked/nlasserted)
	 */
	void		setMulMatrix(const CMatrix &m1, const CMatrix &m2);
	/// Matrix multiplication
	CMatrix		operator*(const CMatrix &in) const
	{
		CMatrix		ret;
		ret.setMulMatrix(*this, in);
		return ret;
	}
	/// equivalent to M=M*in
	CMatrix		&operator*=(const CMatrix &in)
	{
		CMatrix		ret;
		ret.setMulMatrix(*this, in);
		*this= ret;
		return *this;
	}
	/** Matrix multiplication assuming no projection at all in m1/m2 and Hence this. Even Faster than setMulMatrix()
	 *	Equivalent to *this= m1 * m2
	 *	NB: Also always suppose m1 has a translation, for optimization consideration
	 *	\warning *this MUST NOT be the same as m1 or m2, else it doesn't work (not checked/nlasserted)
	 */
	void		setMulMatrixNoProj(const CMatrix &m1, const CMatrix &m2);
	/** transpose the rotation part only of the matrix (swap columns/lines).
	 */
	void		transpose3x3();
	/** transpose the matrix (swap columns/lines).
	 * NB: this transpose the 4*4 matrix entirely (even proj/translate part).
	 */
	void		transpose();
	/** Invert the matrix.
	 * if the matrix can't be inverted, it is set to identity.
	 */
	void		invert();
	/** Return the matrix inverted.
	 * if the matrix can't be inverted, identity is returned.
	 */
	CMatrix		inverted() const;
	/** Normalize the matrix so that the rotation part is now an orthonormal basis, ie a rotation with no scale.
	 * NB: projection part and translation part are not modified.
	 * \param pref the preference axis order to normalize. ZYX means that K direction will be kept, and the plane JK
	 * will be used to lead the I vector.
	 * \return false if One of the vector basis is null. true otherwise.
	 */
	bool		normalize(TRotOrder pref);
	//@}



	/// \ Vector Operations.
	//@{
	/// Multiply a normal. ie v.w=0 so the Translation component doesn't affect result. Projection doesn't affect result.
	CVector		mulVector(const CVector &v) const;
	/// Multiply a point. ie v.w=1 so the Translation component do affect result. Projection doesn't affect result.
	CVector		mulPoint(const CVector &v) const;
	/** Multiply a point. \sa mulPoint
	  */
	CVector		operator*(const CVector &v) const
	{
		return mulPoint(v);
	}

	/// Multiply with an homogeneous vector
	CVectorH	operator*(const CVectorH& v) const;
	//@}

	/// \name Misc
	//@{
	void		serial(IStream &f);
	/// return true if the matrix has a scale part (by scale(), by multiplication etc...)
	bool		hasScalePart() const;
	/// return true if hasScalePart() and if if this scale is uniform.
	bool		hasScaleUniform() const;
	/// return true the uniform scale. valid only if hasScaleUniform() is true, else 1 is returned.
	float		getScaleUniform() const;
	/// return true if the matrix has a projection part (by setProj(), by multiplication etc...)
	bool		hasProjectionPart() const;
	//@}

	// Friend.
	/// Plane (line vector) multiplication.
	friend CPlane		operator*(const CPlane &p, const CMatrix &m);


private:
	float	M[16];
	float	Scale33;
	uint32	StateBit;	// BitVector. 0<=>identity.

	// Methods For inversion.
	bool	fastInvert33(CMatrix &ret) const;
	bool	slowInvert33(CMatrix &ret) const;
	bool	slowInvert44(CMatrix &ret) const;
	// access to M, in math conventions (mat(1,1) ... mat(4,4)). Indices from 0 to 3.
	float	&mat(sint i, sint j)
	{
		return M[ (j<<2) + i];
	}
	// access to M, in math conventions (mat(1,1) ... mat(4,4)). Indices from 0 to 3.
	const float	&mat(sint i, sint j) const
	{
		return M[ (j<<2) + i];
	}
	// return the good 3x3 Id to compute the minor of (i,j);
	void	getCofactIndex(sint i, sint &l1, sint &l2, sint &l3) const
	{
		switch(i)
		{
			case 0: l1=1; l2=2; l3=3; break;
			case 1: l1=0; l2=2; l3=3; break;
			case 2: l1=0; l2=1; l3=3; break;
			case 3: l1=0; l2=1; l3=2; break;
			default: l1=0; l2=0; l3=0; break;
		}
	}

	// true if MAT_TRANS.
	// trans part is true means the right 3x1 translation part matrix is relevant.
	// Else it IS initialized to (0,0,0) (exception!!!)
	bool	hasTrans() const;
	// true if MAT_ROT | MAT_SCALEUNI | MAT_SCALEANY.
	// rot part is true means the 3x3 rot matrix AND Scale33 are relevant.
	// Else they are not initialized but are supposed to represent identity and Scale33==1.
	bool	hasRot() const;
	// true if MAT_PROJ.
	// proj part is true means the bottom 1x4 projection part matrix is relevant.
	// Else it is not initialized but is supposed to represent the line vector (0,0,0,1).
	bool	hasProj() const;
	bool	hasAll() const;

	void	testExpandRot() const;
	void	testExpandProj() const;

	// inline
	void	setScaleUni(float scale);
};

}

#endif // NL_MATRIX_H

/* End of matrix.h */
