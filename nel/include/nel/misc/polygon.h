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

#ifndef NL_POLYGON_H
#define NL_POLYGON_H

#include "types_nl.h"
#include "matrix.h"
#include "stream.h"
#include "vector_2f.h"

#include <vector>


namespace NLMISC
{


class CTriangle;

// Used by the method toConvexPolygons
class CBSPNode2v;

// ***************************************************************************
/**
 * A polygon, with an unlimited size of vertices.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPolygon
{
public:
	std::vector<CVector>	Vertices;

public:

	/// Constructor
	CPolygon() {}
	/// Constructor. Init with a triangle.
	CPolygon(const CVector &a, const CVector &b, const CVector &c);

	sint			getNumVertices() const {return (sint)Vertices.size();}

	// build a triangle fan from this polygon, appending resulting tris to 'dest'
	void		toTriFan(std::vector<NLMISC::CTriangle> &dest) const;

	/// Clip a polygon with a set of planes. Cohen-sutherland... clipPolygonBack() is used on planes.
	void			clip(const CPlane *planes, uint nPlanes);
	/// Clip a polygon with a set of planes. Cohen-sutherland clipping... clipPolygonBack() is used on planes.
	void			clip(const std::vector<CPlane> &planes);

	float			computeArea() const;

	/// Serial this polygon
	void			serial(NLMISC::IStream &f);

	/**
	  * Convert a concave polygon into a list of convex polygons using a 2d projection.
	  * The polygon mustn't overlap itself in the XY plane of the basis passed in parameter.
	  * The polygon must be direct in the XY plane of the basis passed in parameter. (Counter clock wise)
	  *
	  * The subdivison is in non-constant n*log(n) with n is the number of vertices.
	  *
	  * \param outputPolygons is the list filled with clipped convex polygons. The list is not cleared at the beginning.
	  * New polygons are just appended at the end.
	  * \param basis is the basis of the polygon projection.
	  * \return true if the polygon has been subdivided. false if the polygon overlap itself in the XY plane of the basis
	  * or if the polygon is not direct (clock wise).
	  */
	bool			toConvexPolygons (std::list<CPolygon>& outputPolygons, const CMatrix& basis) const;

	/**
	  * Chain the arg polygons with this polygon testing 2d intersections.
	  * The 2d intersection test has been done in the XY plane of the basis passed at the function.
	  *
	  * The polygon a-b-c-d-e chained with f-g-h-i-j will give the polygon a-b-f-g-h-i-j-f-b-c-d-e
	  * if the edge b-f is not 2d clipped by any edge plane in the XY plane of basis.
	  *
	  * \param basis is the basis of the polygon projection.
	  * \return false if chain failed. else true.
	  */
	bool			chain (const std::vector<CPolygon> &other, const CMatrix& basis);

	/// get the best triplet from this poly (the one that has the highest area)
	void		getBestTriplet(uint &index0, uint &index1, uint &index2);

	/** Takes the best triplet from this poly to build a normal.
	  * From this normal and a points, build a basis (the normal is the K vector of the basis)
	  * This can be used to transform the poly in 2D after it has been inverted
	  */
	void		buildBasis(CMatrix &dest);

	// Used by the method toConvexPolygons and chain
	void			toConvexPolygonsLocalAndBSP (std::vector<CVector> &localVertices, CBSPNode2v &root, const CMatrix &basis) const;
	static bool		toConvexPolygonsEdgeIntersect (const CVector2f& a0, const CVector2f& a1, const CVector2f& b0, const CVector2f& b1);
	static bool		toConvexPolygonsLeft (const std::vector<CVector> &vertex, uint a, uint b, uint c);
	static bool		toConvexPolygonsLeftOn (const std::vector<CVector> &vertex, uint a, uint b, uint c);
	static bool		toConvexPolygonsInCone (const std::vector<CVector> &vertex, uint a, uint b);
	static bool		toConvexPolygonsDiagonal (const std::vector<CVector> &vertex, const CBSPNode2v &bsp, uint a, uint b);
};

/**
  * A 2d convex polygon
  */
class CPolygon2D
{
public:
	typedef std::vector<CVector2f> TVec2fVect;
	TVec2fVect Vertices;
public:
	/// default ctor
	CPolygon2D() {}

	// swap this poly content with another poly content
	void swap(CPolygon2D &other) { Vertices.swap(other.Vertices); }

	/** Build a 2D polygon from this 3D polygon, by using the given projection matrix
	  * The x and y components of projected vertices are used to create the 2D polygon
	  */
	CPolygon2D(const CPolygon &src, const CMatrix &projMat = CMatrix::Identity);

	/** Reinit a 2D polygon from this 3D polygon, by using the given projection matrix
	  * The x and y components of projected vertices are used to create the 2D polygon
	  */
	void fromPolygon(const CPolygon &src, const CMatrix &projMat = CMatrix::Identity);

	/** Build a 2D polygon from the given triangle, by using the given projection matrix
	  * The x and y components of projected vertices are used to create the 2D polygon
	  */
	CPolygon2D(const CTriangle &tri, const CMatrix &projMat = CMatrix::Identity);

	/// Check whether this polygon is convex;
	bool		isConvex();

	/** Build a convex hull from this polygon. The result poly is ordered, so it can also be used to order a convex
	  * poly given its set of vertices.
	  * NB: require this != &dest
	  */
	void		buildConvexHull(CPolygon2D &dest) const;

	/// get the best triplet of vector. e.g the triplet that has the best surface
	void		getBestTriplet(uint &index0, uint &index1, uint &index2);

	/// Serial this polygon
	void		serial(NLMISC::IStream &f);

	typedef std::pair<sint, sint> TRaster;
	typedef std::vector<TRaster>  TRasterVect;

	/** Compute the borders of this poly with sub-pixel accuracy. No clipping is performed.
	  * Only points exactly inside or exactly on the left border of the polygon are kept.
	  * This means that pixels are seen as points, not as surfaces.
	  * The output is in a vector of sint pairs. minimumY is filled with the minimum y value of the poly.
	  * Each pairs gives [xmin, xmax] for the current segment. if xmin > xmax, then no point is valid for this segment.
	  * Otherwise, all points from x = xmin (included)  to x = xmax (included) are valid.
	  * IMPORTANT: coordinates must be in the -32000, 32000 range. This is checked in debug
	  */
	void		computeBorders(TRasterVect &borders, sint &minimumY) const;
	/** The same as compute borders, but pixel are seen as surfaces and not as points.
	   * Any pixel that is touched by the poly will be selected
	   * IMPORTANT: coordinates must be in the -32000, 32000 range. This is checked in debug
	   */
	void		computeOuterBorders(TRasterVect &borders, sint &minimumY) const;
	/** The same as compute borders, but pixel are seen as surfaces and not as points
	  * In this version, only pixels that are entirely INSIDE the poly are kept
	  * IMPORTANT: coordinates must be in the -32000, 32000 range. This is checked in debug
	  */
	void		computeInnerBorders(TRasterVect &borders, sint &minimumY) const;
	/// Test whether this polygon intersect another convex polygon. Currently not optimized.
	bool        intersect(const CPolygon2D &other) const;

	/// Check whether a point is contained by this poly
	bool		contains(const CVector2f &p, bool hintIsConvex = true) const;

	/** Get the index of a segment of this poly that is a non null segment.
	  * \return true if such a segment was found
	  */
	bool  getNonNullSeg(uint &seg) const;

	/// Get a line equation of the seg starting at the given index
	void  getLineEquation(uint index, float &a, float &b, float &c) const;

	// Test if current poly is CCW oriented (in a right handed coord. system)
	bool  isCCWOriented() const;

	// get bounding rect (poly must not be empty)
	void getBoundingRect(CVector2f &minCorner, CVector2f &maxCorner) const;

	// test self intersection
	bool selfIntersect() const;

private:
	/// Sum the dot product of this poly vertices against a line equation a*x + b*y + c
	float sumDPAgainstLine(float a, float b, float c) const;

	/// Get ref to the first vertex that start at index
	const CVector2f &getSegRef0(uint index) const
	{
		nlassert(index < Vertices.size()); return Vertices[index];
	}
	const CVector2f &getSegRef1(uint index) const
	{
		nlassert(index < Vertices.size());
		return index + 1 == Vertices.size() ?
			   Vertices[0]                 :
		       Vertices[index + 1];
	}
	void checkValidBorders() const;
};

// comparison of 2D polygon
bool operator == (const CPolygon2D &lhs, const CPolygon2D &rhs);
bool operator < (const CPolygon2D &lhs, const CPolygon2D &rhs);

} // NLMISC


#endif // NL_POLYGON_H

/* End of polygon.h */
