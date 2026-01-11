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

#ifndef NL_VECTOR_H_H
#define NL_VECTOR_H_H

#include "types_nl.h"
#include "vector.h"


namespace NLMISC {


/**
 * Homogeneus vector.
 * A CVectorH has a fourth value, w.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CVectorH
{
public:

	/// Attributes
	float	x, y, z, w;

	/// Constructor
	CVectorH() {}
	/// Constructor .
	CVectorH(float	_x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	/// Copy Constructor.
	CVectorH(const CVectorH &v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	/// Constructor using a vector
	CVectorH(const CVector &v) : x(v.x), y(v.y), z(v.z), w(1.0f) {}

	/// Set
	void	set(float _x, float _y, float _z, float _w)
	{
		x=_x; y=_y; z=_z; w=_w;
	}

	/// Cast operator to a vector (ignoring w)
	operator CVector() const
	{
		return CVector( x, y, z );
	}


	/// @name Misc.
	//@{
	bool	operator==(const CVectorH &v) const
	{
		return x==v.x && y==v.y && z==v.z && w==v.w;
	}
	bool	operator!=(const CVectorH &v) const
	{
		return !(*this==v);
	}
	/// This operator is here just for map/set insertion (no meaning). comparison order is x,y,z,w.
	bool	operator<(const CVectorH &v) const
	{
		if(x!=v.x)
			return x<v.x;
		if(y!=v.y)
			return y<v.y;
		if(z!=v.z)
			return z<v.z;
		return w<v.w;
	}

	//@}

};


} // NLMISC


#endif // NL_VECTOR_H_H

/* End of vector_h.h */
