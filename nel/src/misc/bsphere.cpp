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

#include "nel/misc/bsphere.h"

using namespace	NLMISC;
using namespace	std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {


bool	CBSphere::clipFront(const CPlane &p) const
{
	// assume normalized planes.

	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d<-Radius)
		return false;

	return true;
}


bool	CBSphere::clipBack(const CPlane &p) const
{
	// assume normalized planes.

	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d>Radius)
		return false;

	return true;
}


bool	CBSphere::include(const CVector &p) const
{
	float	r2= (p-Center).sqrnorm();
	return (r2<=sqr(Radius));
}

bool	CBSphere::include(const CBSphere &s) const
{
	// if smaller than s, how could we include it???
	if(Radius<=s.Radius)
		return false;
	float	r2= (s.Center-Center).sqrnorm();
	// Because of prec test, Radius-s.Radius>0.
	return  r2<=sqr(Radius-s.Radius);
}

bool	CBSphere::intersect(const CBSphere &s) const
{
	float	r2= (s.Center-Center).sqrnorm();

	return r2<=sqr(Radius+s.Radius);

}


void	CBSphere::applyTransform(const CMatrix &mat, CBSphere &res)
{
	res.Center= mat*Center;

	if(!mat.hasScalePart())
		res.Radius= Radius;
	else
	{
		if(mat.hasScaleUniform())
			res.Radius= Radius*mat.getScaleUniform();
		else
		{
			// must compute max of 3 axis.
			float	m, mx;
			CVector	i,j,k;
			i= mat.getI();
			j= mat.getJ();
			k= mat.getK();
			// take the max of the 3 axis.
			m= i.sqrnorm();
			mx= m;
			m= j.sqrnorm();
			mx= max(m, mx);
			m= k.sqrnorm();
			mx= max(m, mx);

			// result.
			res.Radius= Radius * (float)sqrt(mx);
		}
	}
}


// ***************************************************************************
void	CBSphere::setUnion(const CBSphere &sa, const CBSphere &sb)
{
	float	r2= (sb.Center-sa.Center).norm();

	// Name Sphere 0 the biggest one, and Sphere 1 the other
	const CBSphere	*s0;
	const CBSphere	*s1;
	if(sa.Radius>sb.Radius)
	{
		s0= &sa;
		s1= &sb;
	}
	else
	{
		s0= &sb;
		s1= &sa;
	}
	float	r0= s0->Radius;
	float	r1= s1->Radius;

	// If Sphere1 is included into Sphere0, then the union is simply Sphere0
	if(r2<=(r0-r1))
	{
		*this= *s0;
	}
	else
	{
		/* Compute the Union sphere Diameter. It is D= r0 + r2 + r1
			do the draw, works for intersect and don't intersect case,
			acknowledge that Sphere1 not included inton Sphere0
		*/
		float	diameter= r0 + r2 + r1;

		// compute dir from big center to small one.
		CVector	dir= s1->Center - s0->Center;
		dir.normalize();

		// Then finally set Radius and center
		Center= s0->Center + dir * (diameter/2 - r0);
		Radius= diameter/2;
	}
}


} // NLMISC
