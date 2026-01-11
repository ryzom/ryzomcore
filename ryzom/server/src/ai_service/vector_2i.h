// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef RY_VECTOR2I_H
#define RY_VECTOR2I_H

#include "nel/misc/types_nl.h"


// ***************************************************************************
/**
 * A 2D vector of sint32.
 */
class CVector2i
{
public:

public:		// Attributes.
	sint32	x,y;

public:		// Methods.
	// Object.
	/// Constructor which do nothing.
	CVector2i() {}
	/// Constructor .
	CVector2i(sint32 _x, sint32 _y) : x(_x), y(_y) {}
	/// Copy Constructor.
	CVector2i(const CVector2i &v) : x(v.x), y(v.y) {}

	// Base Maths.
	CVector2i	&operator+=(const CVector2i &v)		{x+=v.x; y+=v.y; return *this;}
	CVector2i	&operator-=(const CVector2i &v)		{x-=v.x; y-=v.y; return *this;}
	CVector2i	&operator*=(sint32 i)				{x*=i; y*=i; return *this;}
	CVector2i	&operator/=(sint32 i)				{x/=i; y/=i; return *this;}
	CVector2i	operator+(const CVector2i &v) const	{return CVector2i(x+v.x, y+v.y);}
	CVector2i	operator-(const CVector2i &v) const	{return CVector2i(x-v.x, y-v.y);}
	CVector2i	operator*(sint32 i) const			{return CVector2i(x*i, y*i);}
	CVector2i	operator/(sint32 i) const			{return CVector2i(x/i, y/i);}
	CVector2i	operator-() const					{return CVector2i(-x, -y);}

	// Advanced Maths.
	// Dot product.
	sint32	operator*(const CVector2i &v) const		{return x*v.x + y*v.y;}
	// Return the norm of the vector.
	sint32	norm() const							{return (sint32)sqrt((double)sqrnorm());}
	// Return the square of the norm of the vector.
	sint32	sqrnorm() const							{return x*x + y*y;}
	// Normalize the vector.
	void	normalize()
	{
		sint32	i= norm();
		if(i>0)
			*this/=i;
	}
	/// Return the vector normalized.
	CVector2i	normed() const
	{
		CVector2i	v= *this;
		v.normalize();
		return v;
	}

	// Misc.
	void	set(sint32 _x, sint32 _y)				{x= _x; y=_y;}
	bool	operator==(const CVector2i &v) const	{return x==v.x && y==v.y;}
	bool	operator!=(const CVector2i &v) const	{return !(*this==v);}
	bool	isNull() const							{return x==0.0f && y==0.0f;}

	// serial.
	void	serial(NLMISC::IStream &i)				{i.serial(x,y);}
};

#endif

