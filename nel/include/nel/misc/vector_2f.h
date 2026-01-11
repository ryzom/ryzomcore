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

#ifndef NL_VECTOR_2F_H
#define NL_VECTOR_2F_H

#include "types_nl.h"
#include "vector.h"
#include <cmath>
#include "stream.h"
#include <string>


namespace NLMISC
{


// ***************************************************************************
/**
 * A 2D vector of float.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVector2f
{
public:

public:		// Attributes.
	float	x,y;

public:		// Methods.
	/// @name Object.
	//@{
	/// Constructor which do nothing.
	CVector2f() {}
	/// Constructor.
	CVector2f(float	_x, float _y) : x(_x), y(_y) {}
	/// Copy Constructor.
	CVector2f(const CVector2f &v) : x(v.x), y(v.y) {}
	/// Constructor that uses the (x,y) coordinates of a CVector.
	CVector2f(const CVector &v) : x(v.x), y(v.y) {}
	// conversion operator
	operator CVector() const { return this->asVector(); }
	// convert to a CVector by setting z to 0
	CVector  asVector() const { return CVector(x, y, 0); }
	//@}

	/// @name Base Maths.
	//@{
	CVector2f	&operator+=(const CVector2f &v)		{x+=v.x; y+=v.y; return *this;}
	CVector2f	&operator-=(const CVector2f &v)		{x-=v.x; y-=v.y; return *this;}
	CVector2f	&operator*=(float f)				{x*=f; y*=f; return *this;}
	CVector2f	&operator/=(float f)				{x/=f; y/=f; return *this;}
	CVector2f	operator+(const CVector2f &v) const	{return CVector2f(x+v.x, y+v.y);}
	CVector2f	operator-(const CVector2f &v) const	{return CVector2f(x-v.x, y-v.y);}
	CVector2f	operator*(float f) const			{return CVector2f(x*f, y*f);}
	CVector2f	operator/(float f) const			{return CVector2f(x/f, y/f);}
	CVector2f	operator-() const					{return CVector2f(-x, -y);}
	//@}

	/// @name Advanced Maths.
	//@{
	/// Dot product.
	float	operator*(const CVector2f &v) const		{return x*v.x + y*v.y;}
	/// Return the norm of the vector.
	float	norm() const							{return (float)sqrt(sqrnorm());}
	/// Return the square of the norm of the vector.
	float	sqrnorm() const							{return x*x + y*y;}
	/// Normalize the vector.
	void	normalize()
	{
		float	f= norm();
		if(f>0)
			*this/=f;
	}
	/// Return the vector normalized.
	CVector2f	normed() const
	{
		CVector2f	v= *this;
		v.normalize();
		return v;
	}
	//@}

	/// @name Misc.
	//@{
	void	set(float _x, float _y)					{x= _x; y=_y;}
	bool	operator==(const CVector2f &v) const	{return x==v.x && y==v.y;}
	bool	operator!=(const CVector2f &v) const	{return !(*this==v);}
	bool	isNull() const							{return x==0.0f && y==0.0f;}
	/// Set all vector x/y/z as minimum of a/b x/y/z  (respectively).
	void	minof(const CVector2f &a, const CVector2f &b)
	{
		x= std::min(a.x, b.x);
		y= std::min(a.y, b.y);
	}
	/// Set all vector x/y/z as maximum of a/b x/y/z  (respectively).
	void	maxof(const CVector2f &a, const CVector2f &b)
	{
		x= std::max(a.x, b.x);
		y= std::max(a.y, b.y);
	}
	/// serial.
	void	serial(NLMISC::IStream &f)				{f.serial(x,y);}
	//@}

	// friends.
	friend	CVector2f	operator*(float f, const CVector2f &v0);

	/// @name Constants
	//@{
	static const CVector2f Null;
	//@}
};


inline	CVector2f	operator*(float f, const CVector2f &v)
{
	return v*f;
}

// for map/set insertion
inline bool operator < (const CVector2f &lhs, const CVector2f &rhs)
{
	return (lhs.x != rhs.x) ? lhs.x < rhs.x : lhs.y < rhs.y;
}

} // NLMISC


#endif // NL_VECTOR_2F_H

/* End of vector_2f.h */
