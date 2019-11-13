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

#ifndef NL_RAY_MESH_H
#define NL_RAY_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"


namespace NL3D {


// ***************************************************************************
/**
 * A tool class. Provides fast methods to compute intersection of ray agst mesh
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CRayMesh
{
public:

	/** ray intersection with vertices in ray space
	 *	\param vertices vertices that must be transformed in ray space ( space where the ray is (0,K) )
	 *	\param tris list of triplets defining triangles
	 *	if intersect, dist2D=0, and distZ= Depth Distance
	 *	if don't intersect, dist2D="nearest distance to the ray", and distZ=0
	 *	\param computeDist2D if false and don't intersect, then return dist2D=FLT_MAX, and distZ=0
	 *	return false only if tris.size()<3
	 *	NB: don't test if tris indices are good agst vertices array!
	 */
	static bool		getRayIntersection(std::vector<NLMISC::CVector> &vertices, const std::vector<uint32> &tris,
		float &dist2D, float &distZ, bool computeDist2D);

	/// same, for 16 bit indices
	static bool		getRayIntersection(std::vector<NLMISC::CVector> &vertices, const std::vector<uint16> &tris,
		float &dist2D, float &distZ, bool computeDist2D);

public:
	// Simple Definition of a mesh used to test against Ray
	std::vector<NLMISC::CVector>		Vertices;
	std::vector<uint32>			Triangles;

	/// Empty?
	bool	empty() const
	{
		return Vertices.empty() || Triangles.size()<3;
	}

	/// clear
	void	clear()
	{
		Vertices.clear();
		Triangles.clear();
	}

	/** Fast intersect the mesh with the ray. return false if empty, else return true and:
	 *	\param worldMatrix: the matrix to apply to the mesh to make it worldSpace
	 *	\param p0/dir: the ray, in world space
	 *	if intersect, dist2D=0, and distZ= Depth Distance
	 *	if don't intersect, dist2D="nearest distance to the ray", and distZ=0
	 *	\param computeDist2D if false and don't intersect, then return dist2D=FLT_MAX, and distZ=0
	 */
	bool	fastIntersect(const NLMISC::CMatrix &worldMatrix, const NLMISC::CVector &p0, const NLMISC::CVector &dir, float &dist2D, float &distZ, bool computeDist2D) const;


private:
	static  uint	NumCacheVertex;
};


} // NL3D


#endif // NL_RAY_MESH_H

/* End of ray_mesh.h */
