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

#ifndef NL_VECTOR_H
#define NL_VECTOR_H

#include "types_nl.h"

#include <cmath>
#include <string>

#include "stream.h"

namespace	NLMISC
{

class IStream;

// ======================================================================================================
/**
 * A 3D vector of float.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CVector
{
public:		// Attributes.
	float	x,y,z;

public:		// const.
	/// Null vector (0,0,0).
	static const	CVector		Null;
	/// I vector (1,0,0).
	static const	CVector		I;
	/// J vector (0,1,0).
	static const	CVector		J;
	/// K vector (0,0,1).
	static const	CVector		K;

public:		// Methods.
	/// @name Object.
	//@{
	/// Constructor which does nothing.
	CVector() {}
	/// Constructor .
	CVector(float	_x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	/// Copy Constructor.
	CVector(const CVector &v) : x(v.x), y(v.y), z(v.z) {}
	//@}

	/// @name Base Maths.
	//@{
	CVector	&operator+=(const CVector &v);
	CVector	&operator-=(const CVector &v);
	CVector	&operator*=(float f);
	CVector	&operator/=(float f);
	CVector	operator+(const CVector &v) const;
	CVector	operator-(const CVector &v) const;
	CVector	operator*(float f) const;
	CVector	operator/(float f) const;
	CVector	operator-() const;
	//@}

	/// @name Advanced Maths.
	//@{
	/// Dot product.
	float	operator*(const CVector &v) const;
	/** Cross product.
	 * compute the cross product *this ^ v.
	 */
	CVector	operator^(const CVector &v) const;
	/// Return the norm of the vector.
	float	norm() const;
	/// Return the square of the norm of the vector.
	float	sqrnorm() const;
	/// Normalize the vector.
	void	normalize();
	/// Return the vector normalized.
	CVector	normed() const;
	//@}

	/// @name Misc.
	//@{
	void	set(float _x, float _y, float _z);
	bool	operator==(const CVector &v) const;
	bool	operator!=(const CVector &v) const;
	bool	isNull() const;
	/// This operator is here just for map/set insertion (no meaning). comparison order is x,y,z.
	bool	operator<(const CVector &v) const;
	/**
	 * Setup the vector with spheric coordinates.
	 * sphericToCartesian(1,0,0) build the I vector  ((1,0,0)).
	 * the formula is:  \n
	 * x= r*cos(theta)*cos(phi) \n
	 * y= r*sin(theta)*cos(phi) \n
	 * z= r*sin(phi) \n
	 * \sa cartesianToSpheric()
	 */
	void	sphericToCartesian(float r, float theta,float phi);
	/**
	 * Get the spheric coordinates of the vector.
	 * See sphericToCartesian() to know coordinates conventions.
	 * \sa sphericToCartesian()
	 */
	void	cartesianToSpheric(float &r, float &theta,float &phi) const;
	/// Set all vector x/y/z as minimum of a/b x/y/z  (respectively).
	void	minof(const CVector &a, const CVector &b);
	/// Set all vector x/y/z as maximum of a/b x/y/z  (respectively).
	void	maxof(const CVector &a, const CVector &b);
	/// serial.
	void	serial(IStream &f);
	//@}

	/// Returns the contents as a printable string "x y z"
	/// undeprecated, use the generic function toString()
	std::string	asString() const { return toString(); }

	/// Returns the contents as a printable string "x y z"
	std::string	toString() const;

	// friends.
	friend	CVector	operator*(float f, const CVector &v0);
};

// blend (faster version than the generic version found in algo.h)
inline CVector blend(const CVector &v0, const CVector &v1, float lambda)
{
	float invLambda = 1.f - lambda;
	return CVector(invLambda * v0.x + lambda * v1.x,
		           invLambda * v0.y + lambda * v1.y,
				   invLambda * v0.z + lambda * v1.z);
}


}


#include "vector_inline.h"


#endif // NL_VECTOR_H

/* End of vector.h */
