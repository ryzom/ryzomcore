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

#ifndef NL_VECTOR_2D_H
#define NL_VECTOR_2D_H

#include "types_nl.h"

#include <cmath>
#include <string>

#include "stream.h"
#include "vector_2f.h"

namespace NLMISC
{


// ***************************************************************************
/**
 * A 2D vector of double.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVector2d
{
public:

public:		// Attributes.
	double	x,y;

public:		// Methods.
	/// @name Object.
	//@{
	/// Constructor which does nothing.
	CVector2d() {}
	/// Constructor .
	CVector2d(double	_x, double _y) : x(_x), y(_y) {}
	/// Copy Constructor.
	CVector2d(const CVector2d &v) : x(v.x), y(v.y) {}
	/// Constructor with a CVector2f.
	CVector2d(const CVector2f &v) : x(v.x), y(v.y) {}

	//@}

	/// @name Base Maths.
	//@{
	CVector2d	&operator+=(const CVector2d &v)		{x+=v.x; y+=v.y; return *this;}
	CVector2d	&operator-=(const CVector2d &v)		{x-=v.x; y-=v.y; return *this;}
	CVector2d	&operator*=(double f)				{x*=f; y*=f; return *this;}
	CVector2d	&operator/=(double f)				{x/=f; y/=f; return *this;}
	CVector2d	operator+(const CVector2d &v) const	{return CVector2d(x+v.x, y+v.y);}
	CVector2d	operator-(const CVector2d &v) const	{return CVector2d(x-v.x, y-v.y);}
	CVector2d	operator*(double f) const			{return CVector2d(x*f, y*f);}
	CVector2d	operator/(double f) const			{return CVector2d(x/f, y/f);}
	CVector2d	operator-() const					{return CVector2d(-x, -y);}
	//@}

	/// @name Advanced Maths.
	//@{
	/// Dot product.
	double	operator*(const CVector2d &v) const		{return x*v.x + y*v.y;}
	/// Return the norm of the vector.
	double	norm() const							{return (double)sqrt(sqrnorm());}
	/// Return the square of the norm of the vector.
	double	sqrnorm() const							{return x*x + y*y;}
	/// Normalize the vector.
	void	normalize()
	{
		double	f= norm();
		if(f>0)
			*this/=f;
	}
	/// Return the vector normalized.
	CVector2d	normed() const
	{
		CVector2d	v= *this;
		v.normalize();
		return v;
	}
	//@}

	/// @name Misc.
	//@{
	void	set(double _x, double _y)					{x= _x; y=_y;}
	bool	operator==(const CVector2d &v) const	{return x==v.x && y==v.y;}
	bool	operator!=(const CVector2d &v) const	{return !(*this==v);}
	bool	isNull() const							{return x==0.0f && y==0.0f;}
	/// Set all vector x/y/z as minimum of a/b x/y/z  (respectively).
	void	minof(const CVector2d &a, const CVector2d &b)
	{
		x= std::min(a.x, b.x);
		y= std::min(a.y, b.y);
	}
	/// Set all vector x/y/z as maximum of a/b x/y/z  (respectively).
	void	maxof(const CVector2d &a, const CVector2d &b)
	{
		x= std::max(a.x, b.x);
		y= std::max(a.y, b.y);
	}
	/// serial.
	void	serial(NLMISC::IStream &f)				{f.serial(x,y);}
	//@}

	// friends.
	friend	CVector2d	operator*(double f, const CVector2d &v0);

};


inline	CVector2d	operator*(double f, const CVector2d &v)
{
	return v*f;
}


} // NLMISC


#endif // NL_VECTOR_2D_H

/* End of vector_2d.h */
