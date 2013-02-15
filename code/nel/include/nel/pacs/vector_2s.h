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

#ifndef NL_VECTOR_2S_H
#define NL_VECTOR_2S_H

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/vector.h"


namespace NLPACS {


const float		Vector2sAccuracy = 128.0f;

/**
 * TODO Class description
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CVector2s
{
private:
	// safely cast a fixed64 into a fixed16
	static sint16	safeCastSint16(sint64 s)
	{
#ifdef NL_DEBUG
		if (s>32767 || s<-32768)
			nlerror("in CVector2s::safeCastSint16(sint64): value doesn't fit sint16 (value=%" NL_I64 "d)", s);
#endif
		return (sint16)s;
	}

	// safely cast a premuled float into a fixed16
	static sint16	safeCastSint16(float f)
	{
#ifdef NL_DEBUG
		sint64	s = (sint64)f;
		if (s>32767 || s<-32768)
			nlerror("in CVector2s::safeCastSint16(float): value doesn't fit sint16 (value=%f)", f);
		return (sint16)s;
#else
		return (sint16)f;
#endif
	}

	// check the cast into fixed16
	static bool		checkCastSint16(sint64 s)
	{
		return (s<=32767 && s>=-32768);
	}

	// check the cast into fixed16
	static bool		checkCastSint16(float f)
	{
		sint64	s = (sint64)f;
		return (s<=32767 && s>=-32768);
	}


	// pack a float into a fixed16
	static sint16	pack(float f)
	{
		return safeCastSint16(f*Vector2sAccuracy);
	}

	// unpack a fixed16 into a float
	static float	unpack(sint16 s)
	{
		return (float)s/Vector2sAccuracy;
	}

public:		// Attributes.
	sint16	x, y;

public:		// Methods.
	/// @name Object.
	//@{
	/// Constructor which do nothing.
	CVector2s() {}
	/// Constructor .
	CVector2s(sint16 _x, sint16 _y) : x(_x), y(_y) {}
	/// Copy Constructor.
	CVector2s(const CVector2s &v) : x(v.x), y(v.y) {}
	CVector2s(const NLMISC::CVector &v) { pack(v); }
	//@}


	/// @name Base Maths.
	//@{
	CVector2s	&operator+=(const CVector2s &v)		{ *this = *this+v; return *this; }
	CVector2s	&operator-=(const CVector2s &v)		{ *this = *this-v; return *this; }
	CVector2s	operator+(const CVector2s &v) const	{return CVector2s(safeCastSint16((sint64)x+(sint64)v.x), safeCastSint16((sint64)y+(sint64)v.y));}
	CVector2s	operator-(const CVector2s &v) const	{return CVector2s(safeCastSint16((sint64)x-(sint64)v.x), safeCastSint16((sint64)y-(sint64)v.y));}
	CVector2s	operator-() const					{return CVector2s(safeCastSint16(-(sint64)x), safeCastSint16(-(sint64)y));}

	CVector2s	&operator*=(float f)				{ x = safeCastSint16(f*x); y = safeCastSint16(f*y); return *this; }
	CVector2s	&operator/=(float f)				{ x = safeCastSint16(f/x); y = safeCastSint16(f/y); return *this; }
	CVector2s	operator*(float f) const			{return CVector2s(safeCastSint16(x*f), safeCastSint16(y*f));}
	CVector2s	operator/(float f) const			{return CVector2s(safeCastSint16(x/f), safeCastSint16(y/f));}
	//@}

	/// @name Advanced Maths.
	//@{
	/// Dot product.
	float	operator*(const CVector2s &v) const		{return ((float)x*(float)v.x + (float)y*(float)v.y)/(Vector2sAccuracy*Vector2sAccuracy);}
	/// Return the norm of the vector.
	float	norm() const							{return (float)sqrt(sqrnorm());}
	/// Return the square of the norm of the vector.
	float	sqrnorm() const							{return ((float)x*(float)x + (float)y*(float)y)/(Vector2sAccuracy*Vector2sAccuracy);}
	/// Normalize the vector.
	void	normalize()
	{
		float	f= norm();
		if(f>0)
			*this/=f;
	}
	/// Return the vector normalized.
	CVector2s	normed() const
	{
		CVector2s	v= *this;
		v.normalize();
		return v;
	}
	//@}

	/// @name Misc.
	//@{
	void	set(sint16 _x, sint16 _y)				{x= _x; y=_y;}
	bool	operator==(const CVector2s &v) const	{return x==v.x && y==v.y;}
	bool	operator!=(const CVector2s &v) const	{return !(*this==v);}
	bool	isNull() const							{return x==0.0f && y==0.0f;}
	/// Set all vector x/y/z as minimum of a/b x/y/z  (respectively).
	void	minof(const CVector2s &a, const CVector2s &b)
	{
		x= std::min(a.x, b.x);
		y= std::min(a.y, b.y);
	}
	/// Set all vector x/y/z as maximum of a/b x/y/z  (respectively).
	void	maxof(const CVector2s &a, const CVector2s &b)
	{
		x= std::max(a.x, b.x);
		y= std::max(a.y, b.y);
	}
	/// serial.
	void	serial(NLMISC::IStream &f)				{f.serial(x,y);}
	//@}

	void				pack(const NLMISC::CVector &v)		{ x = pack(v.x); y = pack(v.y); }
	void				pack(const NLMISC::CVector2f &v)	{ x = pack(v.x); y = pack(v.y); }
	NLMISC::CVector2f	unpack() const						{ return NLMISC::CVector2f(unpack(x), unpack(y)); }
	NLMISC::CVector		unpack3f(float hintz=0.0f) const	{ return NLMISC::CVector(unpack(x), unpack(y), hintz); }

	/// Test if 2 segments intersects
	static bool			intersect(const CVector2s& a0, const CVector2s& a1, const CVector2s& b0, const CVector2s& b1, double* pa = NULL, double* pb = NULL);
};

} // NLPACS


#endif // NL_VECTOR_2S_H

/* End of vector_2s.h */
