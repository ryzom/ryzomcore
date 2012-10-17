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
#include "nel/3d/camera_col.h"
#include "nel/misc/matrix.h"
#include "nel/misc/triangle.h"

using namespace std;
using namespace NLMISC;


namespace NL3D {


// ***************************************************************************
const	float	NL3D_CameraSmoothRadiusFactor= 4;
const	float	NL3D_CameraSmoothNumZSample= 20;
const	float	NL3D_CameraSmoothNumAngleSample= 10;

// ***************************************************************************
CCameraCol::CCameraCol()
{
}

// ***************************************************************************
void		CCameraCol::build(const CVector &start, const CVector &end, float radius, bool cone)
{
	// copy
	_Start= start;
	_End= end;
	_Radius= radius;
	_Cone= cone;
	_SimpleRay= false;

	// For camera smoothing
	float	maxRadiusFactor= NL3D_CameraSmoothRadiusFactor;

	// not a Cone? => no smoothing
	if(!_Cone)
		maxRadiusFactor= 1;

	// **** Compute Camera smooth infos
	_MaxRadius= radius * maxRadiusFactor;
	_MinRadiusProj= _Radius / (end-start).norm();
	_MaxRadiusProj= _MaxRadius / (end-start).norm();
	_RayNorm= (end-start).normed();
	_RayLen= (end-start).norm();
	_OODeltaRadiusProj= 0;
	if(_MaxRadiusProj>_MinRadiusProj)
		_OODeltaRadiusProj= 1.f / (_MaxRadiusProj-_MinRadiusProj);

	// **** build the pyramid, with MaxRadius
	// approximate with a box
	CMatrix		mat;
	// Precision note: make the pyramid local to Start
	mat.setRot(CVector::I, (start-end).normed(), CVector::K);
	mat.normalize(CMatrix::YZX);
	// build the start 4 points
	CVector		ps[4];
	// cone or cylinder?
	if(cone)
	{
		_NPlanes= 5;
		// local to start!
		ps[0]= CVector::Null;
		ps[1]= CVector::Null;
		ps[2]= CVector::Null;
		ps[3]= CVector::Null;
	}
	else
	{
		_NPlanes= 6;
		// local to start!
		ps[0]= mat * CVector(_MaxRadius, 0, -_MaxRadius);
		ps[1]= mat * CVector(_MaxRadius, 0, _MaxRadius);
		ps[2]= mat * CVector(-_MaxRadius, 0, _MaxRadius);
		ps[3]= mat * CVector(-_MaxRadius, 0, -_MaxRadius);
	}
	// build the end 4 points
	CVector		pe[4];
	// local to start!
	mat.setPos(end-start);
	pe[0]= mat * CVector(_MaxRadius, 0, -_MaxRadius);
	pe[1]= mat * CVector(_MaxRadius, 0, _MaxRadius);
	pe[2]= mat * CVector(-_MaxRadius, 0, _MaxRadius);
	pe[3]= mat * CVector(-_MaxRadius, 0, -_MaxRadius);
	// try to roder for optimisation
	// left/right
	_Pyramid[0].make(ps[3], pe[3], pe[2]);
	_Pyramid[1].make(ps[1], pe[1], pe[0]);
	// back
	_Pyramid[2].make(pe[0], pe[1], pe[2]);
	// top-bottom
	_Pyramid[3].make(ps[2], pe[2], pe[1]);
	_Pyramid[4].make(ps[0], pe[0], pe[3]);
	// front if not cone
	if(!cone)
		_Pyramid[5].make(ps[0], ps[2], ps[1]);

	// **** build the bbox
	_BBox.setCenter(start);
	_BBox.extend(end);
	// enlarge a bit for radius
	_BBox.setHalfSize(_BBox.getHalfSize()+CVector(_MaxRadius, _MaxRadius, _MaxRadius));

}


// ***************************************************************************
void		CCameraCol::buildRay(const CVector &start, const CVector &end)
{
	// copy
	_Start= start;
	_End= end;
	_Radius= 0.f;
	_Cone= false;
	_SimpleRay= true;

	// compute infos
	_MaxRadius= 0.f;
	_MinRadiusProj= 0.f;
	_MaxRadiusProj= 0.f;
	_RayNorm= (end-start).normed();
	_RayLen= (end-start).norm();
	_OODeltaRadiusProj= 0;

	// Don't need to build the pyramids here

	// **** build the bbox
	_BBox.setCenter(start);
	_BBox.extend(end);
	// enlarge a bit for radius
	_BBox.setHalfSize(_BBox.getHalfSize()+CVector(0.01f, 0.01f, 0.01f));
}


// ***************************************************************************
void		CCameraCol::setApplyMatrix(const CCameraCol &other, const NLMISC::CMatrix &matrix)
{
	// get parameters modified by matrix
	CVector		start= matrix * other._Start;
	CVector		end= matrix * other._End;
	float		radius= other._Radius;

	// scale the radius
	if(matrix.hasScalePart())
	{
		// get the uniform scale
		if(matrix.hasScaleUniform())
			radius*= matrix.getScaleUniform();
		// Tricky code, deduce a uniform scale. Should not arise
		else
		{
			float	meanScale= matrix.getI().norm() + matrix.getJ().norm() + matrix.getK().norm();
			meanScale/= 3;
			radius*= meanScale;
		}
	}

	// rebuild
	build(start, end, radius, other._Cone);
}


// ***************************************************************************
void		CCameraCol::minimizeDistanceAgainstTri(const CVector &p0, const CVector &p1, const CVector &p2, float &sqrMinDist)
{
	// If the camera collision is actually a ray test, use a simpler method
	if(_SimpleRay)
	{
		intersectRay(p0, p1, p2, sqrMinDist);
		return;
	}

	// Else compute the distance with a smoother way.
	CVector		*pIn= _ArrayIn;
	CVector		*pOut= _ArrayOut;

	// **** clip triangle against the pyramid
	// build the triangle, local to start for precision problems
	pIn[0]= p0 - _Start;
	pIn[1]= p1 - _Start;
	pIn[2]= p2 - _Start;
	sint	nVert= 3;
	// clip
	for(uint i=0;i<_NPlanes;i++)
	{
		nVert= _Pyramid[i].clipPolygonBack(pIn, pOut, nVert);
		swap(pIn, pOut);
		if(!nVert)
			break;
	}

	// **** if clipped => collision
	if(nVert)
	{
		/*
			Polygon nearest distance to a point is:
				- the nearest distance of all vertices
				- or the nearest distance to the plane (if the project lies in the polygon)
				- or the nearest distance to each edge

			NB: testing only points works with low radius, but may fails in general case
		*/

		// compute first the poly min distance
		float	sqrPolyMinDist= FLT_MAX;

		// **** get the nearest distance for all points (avoid precision problem if doing only edge ones)
		sint	i;
		for(i=0;i<nVert;i++)
		{
			// NB: pIn[i] is already local to start
			float	sqrDist= pIn[i].sqrnorm();
			if(sqrDist<sqrPolyMinDist)
				sqrPolyMinDist= sqrDist;
		}

		// **** get the nearest distance of the Start against each edge
		for(i=0;i<nVert;i++)
		{
			const	CVector	&v0= pIn[i];
			const	CVector	&v1= pIn[(i+1)%nVert];
			CVector	vDir= v1-v0;
			// project on line
			float	fLen= vDir.sqrnorm();	// same as  vDir * (v1 - v0)
			// NB: Project CVector::Null, since we are local to start here!
			float	fStart= vDir * (-v0);
			// if start projection in the edge
			if(fStart>0 && fStart<fLen)
			{
				// compute distance to line
				CVector	proj= v0 + (fStart / fLen) * vDir;
				// proj is local to Start
				float	sqrDist= proj.sqrnorm();
				if(sqrDist<sqrPolyMinDist)
					sqrPolyMinDist= sqrDist;
			}
		}

		// **** get the nearest distance of the Start against the plane
		// get the plane local to start
		CPlane	plane;
		plane.make(p0-_Start, p1-_Start, p2-_Start);
		// plane * StartLocalToStart == plane * CVector::Null == plane.d !
		float	planeDist= plane.d;
		// need to do the poly inclusion test only if the plane dist is better than the vertices
		float	sqrPlaneDist= sqr(planeDist);
		if(sqrPlaneDist < sqrPolyMinDist)
		{
			CVector	normal= plane.getNormal();

			// the projection of Start on the plane: StartLocalToStart +
			CVector	proj= planeDist * normal;
			// test poly inclusion
			sint	sign= 0;
			for(i=0;i<nVert;i++)
			{
				const	CVector	&v0= pIn[i];
				const	CVector	&v1= pIn[(i+1)%nVert];
				float	d = ((v1-v0)^normal)*(proj-v0);
				if(d<0)
				{
					if(sign==1) break;
					else	sign=-1;
				}
				else if(d>0)
				{
					if(sign==-1) break;
					else	sign=1;
				}
				else
					break;
			}

			// if succeed, minimize
			if(i==nVert)
				sqrPolyMinDist= sqrPlaneDist;
		}

		// **** Camera Smoothing: modulate according to angle of poly against cone
		// Camera smooth not enabled?
		if(_MaxRadiusProj<=_MinRadiusProj)
		{
			// then just take minum
			if(sqrPolyMinDist<sqrMinDist)
				sqrMinDist= sqrPolyMinDist;
		}
		// Camera Smooth mode. if the unmodulated distance is lower than the current minDist,
		// then this poly may be interesting, else don't have a chance
		else if(sqrPolyMinDist<sqrMinDist)
		{
			float	sampleZSize= _RayLen / NL3D_CameraSmoothNumZSample;
			float	sampleProjSize= 2*_MaxRadiusProj / NL3D_CameraSmoothNumAngleSample;

			// **** Compute num Subdivision required
			// To compute the number of subdivision, let's take the max of 2 req:
			// the subdivision in Z (for Distance part of the function)
			// the subdivision in Projection (for angle part of the function)

			// Project all vertices to the plane
			static	CVector	pProj[3+MaxNPlanes];
			float	minZ= _RayLen;
			float	maxZ= 0;
			for(i=0;i<nVert;i++)
			{
				float	z= pIn[i] * _RayNorm;
				minZ= min(minZ, z);
				maxZ= max(maxZ, z);
				// cause of pyramid cliping, z should be >=0
				if(z>0)
					pProj[i]= pIn[i] / z;
				else
					pProj[i]= CVector::Null;

				// make local
				pProj[i]-= _RayNorm;
			}
			// Compute perimeter of projected poly
			float	perimeterProj= 0;
			for(i=0;i<nVert;i++)
			{
				perimeterProj+= (pProj[(i+1)%nVert]-pProj[i]).norm();
			}

			// compute the number of subdivision required on Z
			uint	numSubdivZ= (uint)((maxZ-minZ) / sampleZSize);
			// suppose a full projected quad perimeter will require max samples
			uint	numSubdivAngle= (uint)(perimeterProj / (4*sampleProjSize));
			// the number of subdivision
			uint	numSubdiv= max(numSubdivZ, numSubdivAngle);
			numSubdiv= max(numSubdiv, (uint)1);
			float	ooNumSubdiv= 1.f / (float)numSubdiv;

			// **** Sample the polygon, to compute the minimum of the function
			// for each tri of the polygon
			for(sint tri=0;tri<nVert-2;tri++)
			{
				CVector		lp[3];
				// optim: prediv by numSubdiv
				lp[0]= pIn[0] * ooNumSubdiv;
				lp[1]= pIn[tri+1] * ooNumSubdiv;
				lp[2]= pIn[tri+2] * ooNumSubdiv;

				// sample using barycentric coordinates
				for(uint i=0;i<=numSubdiv;i++)
				{
					for(uint j=0;j<=numSubdiv-i;j++)
					{
						uint	k= numSubdiv - i - j;
						CVector	sample= lp[0] * (float)i + lp[1] * (float)j + lp[2] * (float)k;

						// NB: sample is already local to start
						float	sqrDist= sample.sqrnorm();

						// **** get the point projection
						float		z= sample * _RayNorm;
						CVector		proj;
						// cause of pyramid cliping, z should be >=0
						if(z>0)
							proj= sample / z;
						else
							proj= CVector::Null;
						// make local
						proj-= _RayNorm;

						// **** compute the Cone Linear factor (like a spot light)
						float	rayDist= proj.norm();
						float	angleFactor= (rayDist-_MinRadiusProj) * _OODeltaRadiusProj;
						clamp(angleFactor, 0.f, 1.f);
						// avoid C1 discontinuity when angleFactor==0
						angleFactor= sqr(angleFactor);

						// **** modulate, but to a bigger value! (ie raylen)
						sqrDist= _RayLen * angleFactor + sqrtf(sqrDist) * (1-angleFactor);
						sqrDist= sqr(sqrDist);

						// if distance is lesser, take it
						if(sqrDist<sqrMinDist)
							sqrMinDist= sqrDist;
					}
				}
			}
		}
	}
}


// ***************************************************************************
void		CCameraCol::intersectRay(const CVector &p0, const CVector &p1, const CVector &p2, float &sqrMinDist)
{
	// build the triangle and the plane from p0,p1,p2
	CTriangle	tri;
	tri.V0= p0;
	tri.V1= p1;
	tri.V2= p2;
	CPlane	plane;
	plane.make(p0,p1,p2);

	// If doesn't intersect, quit
	CVector	hit;
	if(!tri.intersect(_Start, _End, hit, plane))
		return;

	// Else compute the intersection distance factor
	float	f= (hit-_Start) * _RayNorm;
	clamp(f, 0.f, _RayLen);
	// set the result if less
	f= sqr(f);
	if(f<sqrMinDist)
		sqrMinDist= f;
}


/*
			// Perspective Project each vertices on the plane normalized to the ray, at Start + rayNormed * 1
			// And then make local to the Point Start + rayNormed * 1 (ie _RayNorm since we are local to Start!).
			for(i=0;i<nVert;i++)
			{
				float	z= pIn[i] * _RayNorm;
				// cause of pyramid cliping, z should be >=0
				if(z>0)
					pIn[i]= pIn[i] / z;
				else
					pIn[i]= CVector::Null;

				// make local
				pIn[i]-= _RayNorm;
			}

			// Compute now poly distance to the ray
			//	If the ray intersect the poly this is 0!!!
			//	else this is the min distance of each edge/vertex against CVector::Null
			// test poly inclusion
			sint	sign= 0;
			for(i=0;i<nVert;i++)
			{
				const	CVector	&v0= pIn[i];
				const	CVector	&v1= pIn[(i+1)%nVert];
				// _RayNorm should be the plane 's normal of the poly
				float	d = ((v1-v0)^_RayNorm)*(-v0);
				if(d<0)
				{
					if(sign==1) break;
					else	sign=-1;
				}
				else if(d>0)
				{
					if(sign==-1) break;
					else	sign=1;
				}
				else
					break;
			}
			// if succeed, then the poly fully intersect the ray => just minimize
			if(i==nVert)
				sqrMinDist= sqrPolyMinDist;
			// else smooth distance!
			else
			{
				// must get min distance of each projected edge/vertex againt origin
				float	sqrMinRayDist= FLT_MAX;

				// get min distance of each vertex against origin
				for(i=0;i<nVert;i++)
				{
					float	sqrDist= pIn[i].sqrnorm();
					if(sqrDist<sqrMinRayDist)
						sqrMinRayDist= sqrDist;
				}

				// get distance of each edge against origin
				for(i=0;i<nVert;i++)
				{
					const	CVector	&v0= pIn[i];
					const	CVector	&v1= pIn[(i+1)%nVert];
					CVector	vDir= v1-v0;
					// project on line
					float	fLen= vDir.sqrnorm();	// same as  vDir * (v1 - v0)
					// NB: Project CVector::Null
					float	fStart= vDir * (-v0);
					// if origin projection in the edge
					if(fStart>0 && fStart<fLen)
					{
						// compute distance to line
						CVector	proj= v0 + (fStart / fLen) * vDir;
						// proj is local to Start
						float	sqrDist= proj.sqrnorm();
						if(sqrDist<sqrMinRayDist)
							sqrMinRayDist= sqrDist;
					}
				}

				float	minRayDist= sqrtf(sqrMinRayDist);

				// OK! now we have the minRayDist
				// compute the Cone Linear factor (like a spot light)
				float	angleFactor= (minRayDist-_MinRadiusProj) / (_MaxRadiusProj-_MinRadiusProj);
				clamp(angleFactor, 0.f, 1.f);
				// avoid C1 discontinuity when angleFactor==0
				angleFactor= sqr(angleFactor);

				// modulate, but to a bigger value! (ie raylen)
				sqrPolyMinDist= _RayLen * angleFactor + sqrtf(sqrPolyMinDist) * (1-angleFactor);
				sqrPolyMinDist= sqr(sqrPolyMinDist);

				// then if the modified distance is still lower than minDist, set
				if(sqrPolyMinDist<sqrMinDist)
					sqrMinDist= sqrPolyMinDist;
			}
*/

/*
const	float	sampleSize= 0.1f;

// Sample the triangle, to compute the minimum of the function
CVector		lp[3];
float		len[3];
lp[0]= p0-Start;
lp[1]= p1-Start;
lp[2]= p2-Start;
// compute max and min edge length
len[0]= (lp[1]-lp[0]).norm();
len[1]= (lp[2]-lp[1]).norm();
len[2]= (lp[0]-lp[2]).norm();
float	meanLen= (len[0] + len[1] + len[2])/3;
// the number of subdivision
uint	numSubdiv= meanLen / sampleSize;
numSubdiv= max(numSubdiv, (uint)1);
// preca
lp[0]/= numSubdiv;
lp[1]/= numSubdiv;
lp[2]/= numSubdiv;

// sample using barycentric coordinates
for(uint i=0;i<=numSubdiv;i++)
{
	for(uint j=0;j<=numSubdiv-i;j++)
	{
		uint	k= numSubdiv - i - j;
		CVector	sample= lp[0] * i + lp[1] * j + lp[2] * k;

		// NB: sample is already local to start
		float	sqrDist= sample.sqrnorm();

		// **** get the point projection
		float		z= sample * _RayNorm;
		CVector		proj;
		// cause of pyramid cliping, z should be >=0
		if(z>0)
			proj= sample / z;
		else
			proj= CVector::Null;
		// make local
		proj-= _RayNorm;

		// **** compute the Cone Linear factor (like a spot light)
		float	rayDist= proj.norm();
		float	angleFactor= (rayDist-_MinRadiusProj) / (_MaxRadiusProj-_MinRadiusProj);
		clamp(angleFactor, 0.f, 1.f);
		// avoid C1 discontinuity when angleFactor==0
		angleFactor= sqr(angleFactor);

		// **** modulate, but to a bigger value! (ie raylen)
		sqrDist= _RayLen * angleFactor + sqrtf(sqrDist) * (1-angleFactor);
		sqrDist= sqr(sqrDist);

		// if distance is lesser, take it
		if(sqrDist<sqrMinDist)
			sqrMinDist= sqrDist;
	}
}
*/

} // NL3D
