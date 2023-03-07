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



class CAIPos;

#ifndef RYAI_POS_H
#define RYAI_POS_H

#include "ai_share/ai_vector.h"
#include "ai_vector_mirror.h"

class CAIPosMirror;


//----------------------------------------------------------------------------
// The class
//----------------------------------------------------------------------------

/** This class is an extension of CAIVector with an added height and
orientation information.

This class as an additionnal sint32 h field and a CAngle theta field. The
height unit is 1 meter length (and thus there is no precision smaller than 1
meter), and the orientation (see CAngle) is on the horizontal XY plane.

*/
class CAIPos
: public CAIVector  
{
public:
	CAIPos() : CAIVector(0., 0.), _h(0), _theta(0.f) { }
	CAIPos(CAIPos const& pos) : CAIVector(pos), _h(pos._h), _theta(pos._theta) { }
	
	explicit CAIPos(CAIVector const& xy, sint32 h, float theta) : CAIVector(xy), _h(h), _theta(theta) { }
	explicit CAIPos(CAIVector const& xy, sint32 h, CAngle const& theta) : CAIVector(xy), _h(h), _theta(theta) { }
	
	CAIPos(CAIPosMirror const& pos);
	explicit CAIPos(CAIVectorMirror const& xy, sint32 h, float theta);
	explicit CAIPos(CAIVectorMirror const& xy, sint32 h, CAngle const& theta);
	template <class C>
	explicit CAIPos(C x, C y, sint32 h, float theta) : CAIVector(x, y), _h(h), _theta(theta) { }
	template <class C>
	explicit CAIPos(C x, C y, sint32 h, CAngle theta) :CAIVector(x, y), _h(h), _theta(theta) { }
	
	std::string toString() const 
	{
		return NLMISC::toString("(%9s,%9s,%d) %3d\"",x().toString().c_str(),y().toString().c_str(),h(),theta().asDegrees());
	}
	
	inline bool operator==(const CAIPos &other)				const	{ return	CAIVector::operator==(other) && other.h()==h(); }
	inline bool operator!=(const CAIPos &other)				const	{ return	CAIVector::operator!=(other) || other.h()!=h(); }
	inline bool operator==(const CAIPosMirror &other)		const;
	inline bool operator!=(const CAIPosMirror &other)		const;	
	
	inline bool operator==(const CAIVectorMirror &other)	const	{	return CAIVector::operator==(other);	}
	inline bool operator!=(const CAIVectorMirror &other)	const	{	return CAIVector::operator!=(other);	}
	
	
	inline const CAIPos &operator= (const CAIPos &other)		{ setX(other.x()); setY(other.y()); _h=other._h; _theta=other._theta; return *this; }
	inline const CAIPos &operator= (const CAIPosMirror &other); /*	{ setX(other.x()); setY(other.y()); _h=other.h(); _theta=other.theta(); return *this; }*/
	
	template <class V> inline const CAIPos &operator+=(const V &v) { CAIVector::operator +=(v); return *this; }
	template <class V> inline const CAIPos &operator-=(const V &v) { CAIVector::operator -=(v); return *this; }
	
	template <class V> inline const CAIPos operator+(const V &v) const { CAIPos p(*this); return p+=v; }
	template <class V> inline const CAIPos operator-(const V &v) const { CAIPos p(*this); return p-=v; }
	
	inline const sint32	 &h()		const	{ return _h; }
	inline const CAngle	 &theta()	const	{ return _theta; }
	
	template <class C> inline void setH(const C &h)					{ _h=h; }
	template <class C> inline void setTheta(const C &theta)			{ _theta=theta; }
	
private:
	sint32	_h;
	CAngle	_theta;
};

//--------------------------------------------------------------------------
// The inlines
//--------------------------------------------------------------------------

#include "ai_pos_mirror.h"


inline const CAIPos &CAIPos::operator= (const CAIPosMirror &other)	
{ 
	setX(other.x()); 
	setY(other.y()); 
	_h=other.h(); 
	_theta=other.theta(); 
	return *this; 
}


inline CAIPos::CAIPos(const CAIPosMirror &pos):
CAIVector(pos.x(),pos.y()), _h(pos.h()), _theta(pos.theta())
{
}

inline CAIPos::CAIPos(const CAIVectorMirror &xy, sint32 h, float theta):
	CAIVector(xy.x(),xy.y()), _h(h), _theta(theta)		
{
}

inline CAIPos::CAIPos(const CAIVectorMirror &xy, sint32 h, const CAngle &theta):
	CAIVector(xy.x(),xy.y()), _h(h), _theta(theta)		
{
}

inline bool CAIPos::operator==(const CAIPosMirror &other) const
{
	return	(other.x()==x()) && (other.y()==y()) && other.h()==h();
}

inline bool CAIPos::operator!=(const CAIPosMirror &other) const
{
	return	!(*this == other); 
}
#endif
