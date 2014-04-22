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

#ifndef NL_PLANE_H
#define NL_PLANE_H


#include	"stream.h"
#include	"vector.h"


namespace NLMISC
{

class CUV;

// ======================================================================================================
/**
 * Class CPlane. a 3D Plane.
 *
 * A vector v is said "front" of a plane p if p*v>0.
 *
 * A "front segment" or a "front polygon" have all their vertices in front of the plane.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPlane
{
public:
	float	a,b,c,d;

public:

	/// \name Object.
	//@{
	/// Constructor that does nothing.
	CPlane() {}
	/// Constructor .
	CPlane(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) {}
	/// Copy Constructor.
	CPlane(const CPlane &v) : a(v.a), b(v.b), c(v.c), d(v.d) {}
	//@}


	/// \name Construction/Get.
	//@{
	/**
	 * Make a plane with a normal and a vertex.
	 * NB: the plane normal is normalized by make().
	 */
	void	make(const CVector &normal, const CVector &pos);
	/**
	 * Make a plane with 3 vertices.
	 * NB: the plane normal is normalized by make().
	 */
	void	make(const CVector &p0, const CVector &p1, const CVector &p2);
	/**
	 * Return the normal vector of the plane.
	 * Since the normal of the plane may not be normalized (if setuped without make()), the returned normal
	 * may NOT be normalized.
	 */
	CVector	getNormal() const;
	/**
	 * Normalize the plane, such that getNormal() return a normalized vector.
	 */
	void	normalize();
	/**
	 * return the normalized version of a plane. \see normalize()
	 */
	inline CPlane	normed() const;
	//@}


	/// \name Projection / clipping.
	//@{
	/**
	 * Return the distance of p from the plane. Hence, the result is >=0.
	 * Since the plane normal may not be normalized, distance() compute the distance with the normalized normal
	 * of plane. If you are sure that your plane has a normalized normal, it is much faster to do a \c fabs(p*v).
	 */
	float	distance(const CVector &p) const;
	/// Return plane*vector.
	inline float	operator*(const CVector &p) const;
	/// Intersect a line onto a plane. p1 is returned if line // to plane.
	CVector intersect(const CVector &p0,const CVector &p1) const;
	/// Project a point onto a plane.
	CVector project(const CVector &p0) const;

	/**
	 * Clip a segment onto a plane.
	 * The "back segment" is written in (p1,p2). If segment is entirely "front", (p1,p2) is not modified.
	 * \return false if segment entirely front, or true.
	 * \sa clipSegmentFront()
	 */
	bool	clipSegmentBack(CVector &p0, CVector &p1) const;
	/**
	 * Clip a segment onto a plane.
	 * The "front segment" is written in (p1,p2). If segment is entirely "back", (p1,p2) is not modified.
	 * \return false if segment entirely back, or true.
	 * \sa clipSegmentBack()
	 */
	bool	clipSegmentFront(CVector &p0, CVector &p1) const;

	/** Clip a polygon by a plane. The "back polygon" is returned.
	 * Nb: Out must be allocated to nIn+1 (at less).
	 * \param in the input polygon
	 * \param out the clipped back polygon
	 * \param nIn number of vertices of input polygon
	 * \return number of vertices of out. 0 is returned if In polygon entirely front, or if nIn<=2.
	 */
	sint	clipPolygonBack(CVector in[], CVector out[], sint nIn) const;
	// Clip a polygon with uvs by a plane
	sint	clipPolygonBack(const CVector in[], const CUV inUV[], CVector out[], CUV outUV[], sint nIn) const;
	/** Clip a polygon by a plane. The "front polygon" is returned.
	 * Nb: Out must be allocated to nIn+1 (at less).
	 * \param in the input polygon
	 * \param out the clipped front polygon
	 * \param nIn number of vertices of input polygon
	 * \return number of vertices of out. 0 is returned if In polygon entirely back, or if nIn<=2.
	 */
	sint	clipPolygonFront(CVector in[], CVector out[], sint nIn) const;
	//@}

	/// \name normal inversion
	//@{
		/// get the inverted version of this plane (same position, but inverted normal)
		CPlane  inverted() const;

		/// invert this plane (same position, but inverted normal)
		void	invert();
	//@}

	/// \name Misc
	//@{
	void	serial(IStream &f)
	{
		f.serial(a,b,c,d);
	}

	// Strict equality comparator
	bool	operator==(const CPlane &o) const
	{
		return a==o.a && b==o.b && c==o.c && d==o.d;
	}

	//@}

};


}


#include "plane_inline.h"


#endif // NL_PLANE_H

/* End of plane.h */
