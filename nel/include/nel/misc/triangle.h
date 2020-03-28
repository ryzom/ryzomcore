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

#ifndef NL_TRIANGLE_H
#define NL_TRIANGLE_H

#include "types_nl.h"
#include "vector.h"

namespace NLMISC
{
	class CPlane;
	class CMatrix;
}

namespace NLMISC
{


// ***************************************************************************
/**
 * A simple triangles of 3 points.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTriangle
{
public:
	CVector		V0,V1,V2;

public:
	/// Constructor
	CTriangle() {}
	/// Constructor
	CTriangle(const CVector &a, const CVector &b, const CVector &c) : V0(a), V1(b), V2(c) {}

	/**
	  *  Intersection detection with a segment. You must pass the normalized plane of the triangle as parameter.
	  *
	  *  \param p0 is the first point of the segment.
	  *  \param p1 is the second point of the segment.
	  *  \param hit will receive the coordinate of the intersection if the method returns true.
	  *  \param plane is the plane of the triangle. Build it like this:
	  *  \code
	  *  CPlane plane;
	  *  plane.make (triangle.V0, triangle.V1, triangle.V2);
	  *  \endcode
	  *  \return true if the segement [p0,p1] intersects the triangle else false.
	  */
	bool intersect (const CVector& p0, const CVector& p1, CVector& hit, const class NLMISC::CPlane& plane) const;

	/** 3D Gradient computation.
	 * Given 3 values at the 3 corners 'ci' (gouraud, uv....), this method compute the gradients Grad.
	 * The formula to compute the interpolated value according to a 3d position 'v' in space is then simply: \n
	 *	c(v)= c0 + grad*(v-V0)
	 */
	void	computeGradient(float c0, float c1, float c2, CVector &grad) const;

	// transform triangle
	void	applyMatrix(const CMatrix &m, CTriangle &dest) const;
	// compute the minimal corner of this triangle
	inline void getMinCorner(NLMISC::CVector &dest) const;
	// compute the minimal corner of this triangle
	inline void getMaxCorner(NLMISC::CVector &dest) const;
};



// inlines
inline void	CTriangle::getMinCorner(NLMISC::CVector &dest) const
{
	dest.set(minof(V0.x, V1.x, V2.x),
			 minof(V0.y, V1.y, V2.y),
			 minof(V0.z, V1.z, V2.z));
}

inline void	CTriangle::getMaxCorner(NLMISC::CVector &dest) const
{
	dest.set(maxof(V0.x, V1.x, V2.x),
			 maxof(V0.y, V1.y, V2.y),
			 maxof(V0.z, V1.z, V2.z));
}



} // NLMISC


#endif // NL_TRIANGLE_H

/* End of triangle.h */
