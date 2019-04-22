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

#include "std3d.h"

#include "nel/3d/ray_mesh.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/plane.h"

#include "nel/3d/matrix_3x4.h"

using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// The number of byte to process per block
const	uint	NL_BlockByteL1= 4096;

// Number of vertices per block to process For vertices mul
uint	CRayMesh::NumCacheVertex= NL_BlockByteL1 / sizeof(CVector);


// ***************************************************************************
template<class TIndex>
static bool	getRayIntersectionT(std::vector<NLMISC::CVector> &vertices, const std::vector<TIndex> &tris,
							   float &dist2D, float &distZ, bool computeDist2D)
{
	uint	numTris= (uint)tris.size()/3;
	if(!numTris)
		return false;

	// test all tris
	const TIndex	*pTri= &tris[0];
	float	minSkinDistZ= FLT_MAX;
	for(uint i=0;i<numTris;i++)
	{
		const CVector &p0= vertices[*(pTri++)];
		const CVector &p1= vertices[*(pTri++)];
		const CVector &p2= vertices[*(pTri++)];

		// 2D tri seg
		CVector2f	p01(p1.x-p0.x, p1.y-p0.y);
		CVector2f	p12(p2.x-p1.x, p2.y-p1.y);
		CVector2f	p20(p0.x-p2.x, p0.y-p2.y);

		// If some vertices are equal (cause of graphists, or cause of projection), then  this triangle cannot
		// include the ray.
		// must do this, else it is bugguy (if one degenerated triangle exist, will return true for all the skin)
		if(p01.isNull() || p12.isNull() || p20.isNull())
			continue;

		/* Since the triangle is "rendered" in the ray "camera", the ray here is (Pos=Null,dir=K)
			Therefore we can do fast triangle intersection test, only testing the 2D intersection in
			the X/Y plane first
		*/
		float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
		bool	allInf= true;
		bool	allSup= true;
		// Line p0-p1.
		a= -p01.y;
		b= p01.x;
		c= (p0.x*a + p0.y*b);
		allInf= allInf && c<=0;
		allSup= allSup && c>=0;
		// Line p1-p2.
		a= -p12.y;
		b= p12.x;
		c= (p1.x*a + p1.y*b);
		allInf= allInf && c<=0;
		allSup= allSup && c>=0;
		// Line p2-p0.
		a= -p20.y;
		b= p20.x;
		c= (p2.x*a + p2.y*b);
		allInf= allInf && c<=0;
		allSup= allSup && c>=0;

		// all on same side (don't bother front or backfaces)?
		if(allInf || allSup)
		{
			// => ray intersect. compute the intersection now.
			// This code is called for a very small subset of faces, hence don't bother optim.
			CPlane	plane;
			plane.make(p0,p1,p2);
			// intersect with the ray. Since vertices are in ray basis, the Ray is (Null, K).
			CVector	hit= plane.intersect(CVector::Null, CVector::K);
			// then dist of Null to the tri is just Z.
			float	distToTri= hit.z;
			// avoid problems if the plane is // to the ray. take maximum proj dist with the 3 points
			float	minVertDist= min(p0.z, p1.z);
			minVertDist= min(minVertDist, p2.z);
			// distToTri cannot be less than minVertDist
			distToTri= max(minVertDist, distToTri);

			// NB: it is possible than distToTri<0 (face behind or clipped by camera). clamp then to 0
			distToTri= max(distToTri, 0.f);
			minSkinDistZ= min(minSkinDistZ, distToTri);
		}
	}

	// don't intersect?
	if(minSkinDistZ==FLT_MAX)
	{
		// get the nearest distance to the ray (do the compute only if no IT found at all => optim)
		const TIndex	*pTri= &tris[0];
		float	minSkinSqrDist2D= FLT_MAX;

		// only if user want this feature
		if(computeDist2D)
		{
			for(uint i=0;i<numTris;i++)
			{
				const CVector &p0= vertices[*(pTri++)];
				const CVector &p1= vertices[*(pTri++)];
				const CVector &p2= vertices[*(pTri++)];

				// 2D tri seg
				CVector2f	p01(p1.x-p0.x, p1.y-p0.y);
				CVector2f	p12(p2.x-p1.x, p2.y-p1.y);
				CVector2f	p20(p0.x-p2.x, p0.y-p2.y);

				// If some vertices are equal (cause of graphists, or cause of projection), then  this triangle cannot
				// include the ray.
				// must do this, else it is bugguy (if one degenerated triangle exist, will return true for all the skin)
				if(p01.isNull() || p12.isNull() || p20.isNull())
					continue;

				// compute the min dist to the ray
				// ** Min 2D vert dist to Center(Null)
				float	p0sqdist= sqr(p0.x) + sqr(p0.y);
				float	p1sqdist= sqr(p1.x) + sqr(p1.y);
				float	p2sqdist= sqr(p2.x) + sqr(p2.y);
				float	sqdTri= min(p0sqdist, p1sqdist);
				sqdTri= min(sqdTri, p2sqdist);
				// ** Minimize with segment dist
				// compute 2D segment sqrdist
				float	p01sqdist= sqr(p01.x) + sqr(p01.y);
				float	p12sqdist= sqr(p12.x) + sqr(p12.y);
				float	p20sqdist= sqr(p20.x) + sqr(p20.y);
				float	fSeg;
				// project Null onto segment (Null-p0)*(p1-p0) => value between 0 and p01sqdist (if in segment)
				fSeg= -p0.x*p01.x -p0.y*p01.y;
				if(fSeg>0 && fSeg<p01sqdist)
				{
					// project on segment
					CVector2f	pProj= CVector2f(p0) + p01*(fSeg/p01sqdist);
					// min sq dist of tri with (Null-pProj)
					sqdTri= min(sqdTri, pProj.sqrnorm());
				}
				// same for p12
				fSeg= -p1.x*p12.x -p1.y*p12.y;
				if(fSeg>0 && fSeg<p12sqdist)
				{
					CVector2f	pProj= CVector2f(p1) + p12*(fSeg/p12sqdist);
					sqdTri= min(sqdTri, pProj.sqrnorm());
				}
				// same for p20
				fSeg= -p2.x*p20.x -p2.y*p20.y;
				if(fSeg>0 && fSeg<p20sqdist)
				{
					CVector2f	pProj= CVector2f(p2) + p20*(fSeg/p20sqdist);
					sqdTri= min(sqdTri, pProj.sqrnorm());
				}

				// ** minimize with the whole 2D skin dist
				minSkinSqrDist2D= min(minSkinSqrDist2D, sqdTri);
			}
		}


		// NB: in case of all degenerated triangles FLT_MAX is return...)
		if(minSkinSqrDist2D==FLT_MAX)
			dist2D= FLT_MAX;
		else
			dist2D= sqrtf(minSkinSqrDist2D);
		distZ= 0.f;
	}
	else
	{
		dist2D= 0.f;
		distZ= minSkinDistZ;
	}

	return true;
}


// ***************************************************************************
bool		CRayMesh::getRayIntersection(std::vector<NLMISC::CVector> &vertices, const std::vector<uint32> &tris,
		float &dist2D, float &distZ, bool computeDist2D)
{
	return getRayIntersectionT(vertices, tris, dist2D, distZ, computeDist2D);
}


// ***************************************************************************
bool		CRayMesh::getRayIntersection(std::vector<NLMISC::CVector> &vertices, const std::vector<uint16> &tris,
		float &dist2D, float &distZ, bool computeDist2D)
{
	return getRayIntersectionT(vertices, tris, dist2D, distZ, computeDist2D);
}


// ***************************************************************************
bool		CRayMesh::fastIntersect(const NLMISC::CMatrix &worldMatrix, const NLMISC::CVector &p0, const NLMISC::CVector &dir, float &dist2D, float &distZ, bool computeDist2D) const
{
	if(empty())
		return false;


	// *** Compute toRaySpace matrix
	// The skinning must be done in final RaySpace.
	CMatrix		toRaySpace;
	// compute the ray matrix
	CVector	dirn= dir;
	if(dirn.isNull())
		dirn= CVector::K;
	dirn.normalize();
	toRaySpace.setArbitraryRotK(dirn);
	toRaySpace.setPos(p0);
	// The skinning must be done in ray space: (RayMat-1)*worldMatrix;
	toRaySpace.invert();
	toRaySpace*= worldMatrix;

	CMatrix3x4		fastMat;
	fastMat.set(toRaySpace);


	// *** Make all points in ray space
	uint	numVerts= (uint)Vertices.size();
	const CVector	*src= &Vertices[0];
	// enlarge temp buffer
	static std::vector<CVector>	meshInRaySpace;
	if(Vertices.size()>meshInRaySpace.size())
		meshInRaySpace.resize(Vertices.size());
	CVector	*dst= &meshInRaySpace[0];

	// Then do the skin
	for(;numVerts>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertex, numVerts);
		// next block.
		numVerts-= nBlockInf;

		// cache the data in L1 cache.
		CFastMem::precache(src, nBlockInf * sizeof(CVector));

		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++, dst++)
		{
			fastMat.mulSetPoint( *src, *dst );
		}
	}


	// *** and get ray intersection
	return getRayIntersection(meshInRaySpace, Triangles, dist2D, distZ, computeDist2D);
}


} // NL3D
