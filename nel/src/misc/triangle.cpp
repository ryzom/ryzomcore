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

#include "stdmisc.h"

#include "nel/misc/triangle.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

#define EPSILON 0.0001f
// ***************************************************************************
bool CTriangle::intersect (const CVector& p0, const CVector& p1, CVector& hit, const CPlane& plane) const
{
	CVector	normal = plane.getNormal();

	float	np1 = normal*p1;
	float	np2 = np1-normal*p0;

	if (np2 == 0.0f)
		return false;

	float	lambda = (plane.d+np1)/np2;

	// Checks the intersection belongs to the segment
	if (lambda < -EPSILON || lambda > 1.0f+EPSILON)
		return false;

	// The intersection on the plane
	hit = p0*lambda+p1*(1.0f-lambda);

	float	d0 = ((V1-V0)^normal)*(hit-V0);
	float	d1 = ((V2-V1)^normal)*(hit-V1);
	float	d2 = ((V0-V2)^normal)*(hit-V2);

	return (d0 < +EPSILON && d1 < +EPSILON && d2 < +EPSILON) ||
		   (d0 > -EPSILON && d1 > -EPSILON && d2 > -EPSILON);
}



// ***************************************************************************
void	CTriangle::computeGradient(float c0, float c1, float c2, CVector &grad) const
{
	// Compute basis for 2D triangle.
	CVector		locI, locJ, locK;
	locI= V1-V0;
	locJ= V2-V0;
	locK= locI^locJ;
	locK.normalize();
	locI.normalize();
	locJ= locK^locI;

	// compute triangle in 2D.
	CTriangle	tri2D;
	tri2D.V0.set(0,0,0);
	tri2D.V1.x= (V1-V0)*locI;
	tri2D.V1.y= (V1-V0)*locJ;
	tri2D.V1.z= 0;
	tri2D.V2.x= (V2-V0)*locI;
	tri2D.V2.y= (V2-V0)*locJ;
	tri2D.V2.z= 0;

	// Compute 2 2D Gradients.
	float	dx01= tri2D.V0.x - tri2D.V2.x;
	float	dx02= tri2D.V1.x - tri2D.V2.x;
	float	dy01= tri2D.V0.y - tri2D.V2.y;
	float	dy02= tri2D.V1.y - tri2D.V2.y;
	float	dc01= c0 - c2;
	float	dc02= c1 - c2;
	float	gd= dx02*dy01 - dx01*dy02;

	float	OOgd;
	if(gd!=0)
		OOgd= 1.0f/gd;
	else
		OOgd= 1;	// for now, do not manage correctly this case.
	float	gx, gy;
	gx= (dc02*dy01 - dc01*dy02) * OOgd;
	gy= (dc01*dx02 - dc02*dx01) * OOgd;

	// transform in 3D.
	grad= locI*gx + locJ*gy;
}

// ***************************************************************************
void CTriangle::applyMatrix(const CMatrix &m, CTriangle &dest) const
{
	dest.V0 = m * V0;
	dest.V1 = m * V1;
	dest.V2 = m * V2;
}



} // NLMISC

