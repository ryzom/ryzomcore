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



class CAIPosMirror;

#ifndef RYAI_POS_MIRROR_INLINE_H
#define RYAI_POS_MIRROR_INLINE_H

/*
#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2d.h"
#include "ai_share/ai_vector.h"
#include "ai_share/angle.h"
#include "ai_vector_mirror.h"

#include <map>
#include <vector>
#include <string>

class CAIPos;
*/
//--------------------------------------------------------------------------
// The inlines
//--------------------------------------------------------------------------

#include "ai_pos.h"

// ctor
inline CAIPosMirror::CAIPosMirror(const TDataSetRow& entityIndex):
	CAIVectorMirror(entityIndex), 
	_h( *CMirrors::DataSet, entityIndex, DSPropertyPOSZ ),
	_theta( *CMirrors::DataSet, entityIndex, DSPropertyORIENTATION )
	{}

// toString for debugging...
inline std::string CAIPosMirror::toString() const 
{
	return NLMISC::toString("(%9s,%9s,%d) %3d\"",x().toString().c_str(),(y()).toString().c_str(),h(),theta().asDegrees());
}


// == and != operators
inline bool CAIPosMirror::operator==(const CAIPos &other) const	
{
	return other.x()==x() && other.y()==y() && other.h()==h(); 
}

inline bool CAIPosMirror::operator!=(const CAIPos &other) const	
{
	return !(*this == other); 
}


inline bool CAIPosMirror::operator==(const CAIPosMirror &other) const	
{
	return other.x()==x() && other.y()==y() && other.h()==h(); 
}

inline bool CAIPosMirror::operator!=(const CAIPosMirror &other) const	
{
	return !(*this == other);
}


// = operator
inline const CAIPosMirror &CAIPosMirror::operator= (const CAIPos &other)	
{
	setX(other.x()); setY(other.y()); setH(other.h()); setTheta(other.theta()); return *this; 
}

// += and -= operators
template <class C> 
inline const CAIPosMirror &CAIPosMirror::operator+=(const C &v)		
{
	setX(x()+v.x()); setY(y()+v.y()); return *this;
}

template <class C> 
inline const CAIPosMirror &CAIPosMirror::operator-=(const C &v)		
{
	setX(x()-v.x()); setY(y()-v.y()); return *this; 
}


// * and / operators
inline const CAIPos  CAIPosMirror::operator* (double d) const
{
	return CAIPos(CAIVectorMirror::operator*(d),h(),theta()); 
}

inline const CAIPos  CAIPosMirror::operator/ (double d) const	
{
	return CAIPos(CAIVectorMirror::operator/(d),h(),theta()); 
}


// + and - operators
template <class C> 
inline CAIPos  CAIPosMirror::operator+ (const C &v) const	
{
	return CAIPos(*this)+v; 
}

template <class C>
inline CAIPos  CAIPosMirror::operator- (const C &v) const	
{
	return CAIPos(*this)-v; 
}

inline const sint32	&CAIPosMirror::h() const	
{
	return _h(); 
}

inline CAngle CAIPosMirror::theta() const	
{
	return (CAngle)_theta(); 
}


template <class C> inline void CAIPosMirror::setH(C h)					
{
	_h=(TYPE_POSZ)h; 
}

template <class C> inline void CAIPosMirror::setTheta(C theta)			
{
	_theta=((CAngle)theta).asRadians(); 
}


// a few handy utility methods SLOW!!!!!!!!!
inline CAngle	CAIVectorMirror::angleTo(const CAIPos &dest) const	
{
	return (dest-CAIVector(*this)).asAngle();
}

inline double	CAIVectorMirror::distTo(const CAIPos &dest) const	
{
	return (dest-CAIVector(*this)).norm(); 
}

inline double	CAIVectorMirror::distSqTo(const CAIPos &dest) const	
{
	return (dest-CAIVector(*this)).sqrnorm(); 
}

inline double	CAIVectorMirror::quickDistTo(const CAIPos &dest) const	
{
	double dx=fabs((dest.x()-x()).asDouble()), dy=fabs((dest.y()-y()).asDouble());
	return (dx>dy)? (dx+dy/2): (dy+dx/2); 
}


inline CAngle	CAIVectorMirror::angleTo(const CAIVector &dest) const	
{
	return (dest-CAIVector(*this)).asAngle();
}

inline double	CAIVectorMirror::distTo(const CAIVector &dest) const	
{
	return (dest-CAIVector(*this)).norm(); 
}

inline double	CAIVectorMirror::distSqTo(const CAIVector &dest) const	
{
	return (dest-CAIVector(*this)).sqrnorm(); 
}

inline double	CAIVectorMirror::quickDistTo(const CAIVector &dest) const	
{
	double dx=fabs((dest.x()-x()).asDouble()), dy=fabs((dest.y()-y()).asDouble());
	return (dx>dy)? (dx+dy/2): (dy+dx/2); 
}


inline CAngle	CAIVectorMirror::angleTo(const CAIVectorMirror &dest) const	
{
	return CAngle(atan2((dest.y()-y()).asDouble(),(dest.x()-x()).asDouble())); 
}

inline double	CAIVectorMirror::distTo(const CAIVectorMirror &dest)  const	
{
	return (dest-*this).norm(); 
}

inline double	CAIVectorMirror::distSqTo(const CAIVectorMirror &dest)  const	
{
	return (dest-*this).sqrnorm(); 
}

inline double	CAIVectorMirror::quickDistTo(const CAIVectorMirror &dest) const 
{
	double dx=fabs((dest.x()-x()).asDouble()), dy=fabs((dest.y()-y()).asDouble()); return (dx>dy)? (dx+dy/2): (dy+dx/2); 
}

inline CAIVectorMirror::operator CAIVector() const
{
	return CAIVector(*this);
}

#endif

