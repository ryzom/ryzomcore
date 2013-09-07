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



#ifndef RYAI_VECTOR_H
#define RYAI_VECTOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2d.h"
#include "angle.h"
#include "ai_coord.h"

class	CAIVectorMirror;

// ***************************************************************************
/** A 2D vector of CAICoord.

This class is a simple 2D vector based on CAICoord components. It can be used
in most computations involving entity positions that do not need height
information.

CAIVectorMirror is an extension of this class which is linked to a mirror
field to automatically update an entity position.

*/
class CAIVector
{
public:		// Methods.
	/// Constructors
	template <class V> CAIVector(const V &v) : _x(v.x()), _y(v.y()) {}
	inline explicit CAIVector(const	CAIVectorMirror	&vectorMirror);
	
	CAIVector() {}
	explicit CAIVector(const NLMISC::CVector2d &v) : _x(v.x), _y(v.y) {}
	explicit CAIVector(const	CAICoord& x, const	CAICoord& y) : _x(x), _y(y) {}
	explicit CAIVector(double x, double y) : _x(x), _y(y)
	{}	

	// assignment operators
	CAIVector& operator=(const NLMISC::CVector2d &v) { _x=v.x; _y=v.y; return *this; }

	// basic accessors x(), y(), setX(), setY(), setXY
	const CAICoord &x() const				{ return _x; }
	const CAICoord &y()	const				{ return _y; }
	template <class C> void setX(const C &x)		{ _x=x; }
	template <class C> void setY(const C &y)		{ _y=y; }
	template <class C> void setXY(const C &x, const C &y)	{ _x=x; _y=y; }
	template <class C> void setXY(const C &xy)		{ *this=xy; }

	// Base Maths: +=, +, -=, - vs CVector2d, CAIVector, CAIVectorMirror
	template <class V> CAIVector &operator+=(const V &v)		{ _x+=v.x(); _y+=v.y(); return *this; }
	template <class V> CAIVector &operator-=(const V &v)		{ _x-=v.x(); _y-=v.y(); return *this; }
	template <class V> CAIVector operator+(const V &v) const	{ CAIVector rv(*this); return (rv+=v); }
	template <class V> CAIVector operator-(const V &v) const	{ CAIVector rv(*this); return (rv-=v); }

	CAIVector &operator+=(const NLMISC::CVector2d &v)			{ _x+=v.x; _y+=v.y; return *this; }
	CAIVector &operator-=(const NLMISC::CVector2d &v)			{ _x-=v.x; _y-=v.y; return *this; }
	
	// Base Maths: *=, *, /=, / vs double, int, etc
	template <class C> CAIVector &operator*=(C c)				{ _x*=c; _y*=c; return *this; }
	template <class C> CAIVector &operator/=(C c)				{ _x/=c; _y/=c; return *this; }

//	template <class C> CAIVector operator*(C c) const			{ return CAIVector(_x*c, _y*c); }
	CAIVector	operator*(double	c)	const					{	return CAIVector(_x*c, _y*c);	}
		
	template <class C> CAIVector operator/(C c) const			{ return CAIVector(_x/c, _y/c); }
	
	// Base Maths: unary -
	CAIVector operator-() const									{ return CAIVector(-_x, -_y); }

	// Advanced Maths.
	// Dot product.
	template <class V> double dot(const V &v) const				{ return	_x.asDouble()*v.x().asDouble() + 
																			_y.asDouble()*v.y().asDouble(); }
	template <class V> double vectorDot(const V &v) const		{ return	_x.asDouble()*v.y().asDouble() - 
																			_y.asDouble()*v.x().asDouble(); }
	
	// Return the norm of the vector.
	double norm() const											{ return sqrt(sqrnorm()); }
	// Return the square of the norm of the vector.
	double sqrnorm() const										{ return _x.asDouble()*_x.asDouble() + 
																		 _y.asDouble()*_y.asDouble(); }

	double quickNorm() const									{	double	dx=fabs(x().asDouble()),
																			dy=fabs(y().asDouble());
																	return (dx>dy)? (dx+dy/2): (dy+dx/2);	}
	
	// Normalize the vector.
	CAIVector &normalize(float newNorm=1.f)
	{
		double	d= norm();
		if(d>0.0)
			operator*=((newNorm/1000)/d);
		return *this;
	}

	/// Return the vector normalized.
/*	CAIVector normed() const
	{
		CAIVector	v= *this;
		v.normalize();
		return v;
	}
*/
	// return the orientation of the vector as a CAngle
	CAngle asAngle() const
	{
		return CAngle(atan2(_y.asDouble(),_x.asDouble()));
	}

	template <class V> bool operator==(const V	&v)	const		{ return _x==v.x() && _y==v.y(); }
	template <class V> bool operator!=(const V	&v)	const		{ return _x!=v.x() || _y!=v.y(); }

	bool	isNull() const										{ return _x.asInt()==0 && _y.asInt()==0; }

	template <class V> CAngle angleTo(const V &v)	const		{ return CAngle(atan2((v.y()-y()).asDouble(),
																					  (v.x()-x()).asDouble())); }
	template <class V> double distTo(const V &v)	const		{ return (*this-v).norm(); }
	template <class V> double distSqTo(const V &v)	const		{ return (*this-v).sqrnorm(); }
	template <class V> double quickDistTo(const V &v) const		{ double dx=fabs((v.x()-x()).asDouble()),
																		 dy=fabs((v.y()-y()).asDouble());
																  return (dx>dy)? (dx+dy/2): (dy+dx/2);	}

	// serial.
	void	serial(NLMISC::IStream &i)				{ i.serial(_x,_y); }

	std::string toString() const { return NLMISC::toString("(%.3f,%.3f)",_x.asDouble(),_y.asDouble() ); }


private:		// Attributes.
	CAICoord	_x, _y;
};

#endif

