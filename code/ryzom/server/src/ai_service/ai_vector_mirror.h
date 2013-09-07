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



#ifndef RYAI_VECTOR_MIRROR_H
#define RYAI_VECTOR_MIRROR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2d.h"
#include "mirrors.h"
#include "ai_share/ai_vector.h"


class CAngle;
class CAIPos;
class CAIVector;

// ***************************************************************************
/** A 2D vector of CAICoord.

This class is a 2D vector based on CAICoord components placed in mirror shared
memory. This class is designed to receive computed CAIVector vectors, but may
be used directly in computations although being slightly less efficient.

@see @ref CAIVector

*/
class CAIVectorMirror
{
public:		// Methods.
	/// Constructors
	CAIVectorMirror(const TDataSetRow& entityIndex)
	: _x(*CMirrors::DataSet, entityIndex, DSPropertyPOSX)
	, _y(*CMirrors::DataSet, entityIndex, DSPropertyPOSY)
	{
	}
	
	// Base Maths.
	CAIVectorMirror& operator+=(const NLMISC::CVector2d &v)	{ _x=x()+v.x; _y=y()+v.y; return *this; }
	CAIVectorMirror& operator-=(const NLMISC::CVector2d &v)	{ _x=x()-v.x; _y=y()-v.y; return *this; }
	CAIVectorMirror& operator+=(const CAIVector &v)			{ _x=x()+v.x(); _y=y()+v.y(); return *this; }
	CAIVectorMirror& operator-=(const CAIVector &v)			{ _x=x()-v.x(); _y=y()-v.y(); return *this; }
	template<class V> CAIVector operator+(const V &v) const	{ CAIVector rv(*this); return (rv+=v); }
	template<class V> CAIVector operator-(const V &v) const	{ CAIVector rv(*this); return (rv-=v); }
	
	template <class C> CAIVector operator*(C c) const	{ return CAIVector(x()*c, y()*c);	}
	template <class C> CAIVector operator/(C c) const	{ return CAIVector(x()/c, y()/c);	}
	
	CAIVector operator-() const						{ return CAIVector(-x(), -y()); }
	
	// Misc.
	template<class T>
	bool	operator==(const T &v) const		{ return x()==v.x() && y()==v.y(); }
	
	template<class T>
	bool	operator!=(const T &v) const		{ return x()!=v.x() || y()!=v.y(); }
	
	// toString() for debug
	std::string			toString	()	const	{	return NLMISC::toString("(%.3f,%.3f)",x().asDouble(),y().asDouble() );}
	
	// Basic read/ write accessors
	const	CAICoord &x() const	{ return	_x();	}
	const	CAICoord &y() const	{ return	_y();	}
	
	inline	operator CAIVector() const;
		
	inline	void	setXY(const CAIVector &xy)	{ setX(xy.x()); setY(xy.y());	}
	
	// a few handy utility methods
	inline	CAngle	angleTo(const CAIPos &dest)			const;
	inline	double	distTo(const CAIPos &dest)			const;
	inline	double	distSqTo(const CAIPos &dest)		const;
	inline	double	quickDistTo(const CAIPos &dest)		const;
	
	inline	CAngle	angleTo(const CAIVector &dest)		const;
	inline	double	distTo(const CAIVector &dest)		const;
	inline	double	distSqTo(const CAIVector &dest)		const;
	inline	double	quickDistTo(const CAIVector &dest)	const;
	
	inline	CAngle	angleTo(const CAIVectorMirror &dest) const;
	inline	double	distTo(const CAIVectorMirror &dest)  const;
	inline	double	distSqTo(const CAIVectorMirror &dest)  const;
	inline	double	quickDistTo(const CAIVectorMirror &dest) const;
	
protected:
	inline void	setX(const CAICoord &x)	{ _x=x;	}
	inline void	setY(const CAICoord &y)	{ _y=y;	}

	inline void	setXY(const CAICoord &x, const CAICoord &y)	{ setX(x); setY(y);	}
	
private:		// Attributes
	CMirrorPropValue<CAICoord> _x;
	CMirrorPropValue<CAICoord> _y;
// make sure our coordinate class is same size as mirror pos class
};

inline	CAIVector::CAIVector(const	CAIVectorMirror	&vectorMirror)	:	_x(vectorMirror.x()), _y(vectorMirror.y())
{
}

#endif

