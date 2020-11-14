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

#include "stdpacs.h"

#include "nel/pacs/edge_collide.h"

using namespace NLMISC;
using namespace std;



namespace NLPACS
{


static	const	float	EdgeCollideEpsilon= 1e-5f;


// ***************************************************************************
void		CEdgeCollide::make(const CVector2f &p0, const CVector2f &p1)
{
	P0= p0;
	P1= p1;
	// translation axis of the edge.
	Dir= P1-P0;
	Dir.normalize();
	A0= P0*Dir;
	A1= P1*Dir;
	// line equation.
	Norm.x=  Dir.y;
	Norm.y= -Dir.x;
	C= - P0*Norm;
}


// ***************************************************************************
CRational64	CEdgeCollide::testPointMove(const CVector2f &start, const CVector2f &end, TPointMoveProblem &moveBug)
{
	/*
		To have a correct test (with no float precision problem):
			- test first if there is collision beetween the 2 edges:
				- test if first edge collide the other line.
				- test if second edge collide the other line.
				- if both true, yes, there is a collision.
			- compute time of collision.
	*/


	// *this must be a correct edge.
	if(P0==P1)
	{
		moveBug= EdgeNull;
		return -1;
	}

	// if no movement, no collision.
	if(start==end)
		return 1;

	// NB those edges are snapped (1/256 for edgeCollide, and 1/1024 for start/end), so no float problem here.
	// precision is 20 bits.
	CVector2f	normEdge;
	CVector2f	normMove;
	CVector2f	deltaEdge;
	CVector2f	deltaMove;

	// compute normal of the edge (not normalized, because no need, and for precision problem).
	deltaEdge= P1-P0;
	normEdge.x= -deltaEdge.y;
	normEdge.y= deltaEdge.x;

	// compute normal of the movement (not normalized, because no need, and for precision problem).
	deltaMove= end-start;
	normMove.x= -deltaMove.y;
	normMove.y= deltaMove.x;

	// distance from points of movment against edge line. Use double, because of multiplication.
	// precision is now 43 bits.
	double	moveD0= (double)normEdge.x*(double)(start.x-P0.x) + (double)normEdge.y*(double)(start.y-P0.y);
	double	moveD1= (double)normEdge.x*(double)(end.x  -P0.x) + (double)normEdge.y*(double)(end.y  -P0.y);

	// distance from points of edge against movement line. Use double, because of multiplication.
	// precision is now 43 bits.
	double	edgeD0= (double)normMove.x*(double)(P0.x-start.x) + (double)normMove.y*(double)(P0.y-start.y);
	double	edgeD1= (double)normMove.x*(double)(P1.x-start.x) + (double)normMove.y*(double)(P1.y-start.y);


	// If both edges intersect lines (including endpoints), there is a collision, else none.
	sint	sgnMove0, sgnMove1;
	sgnMove0= fsgn(moveD0);
	sgnMove1= fsgn(moveD1);

	// special case if the 2 edges lies on the same line.
	if(sgnMove0==0 && sgnMove1==0)
	{
		// must test if there is a collision. if yes, problem.
		// project all the points on the line of the edge.
		// Use double because of multiplication. precision is now 43 bits.
		double	moveA0= (double)deltaEdge.x*(double)(start.x-P0.x) + (double)deltaEdge.y*(double)(start.y-P0.y);
		double	moveA1= (double)deltaEdge.x*(double)(end.x  -P0.x) + (double)deltaEdge.y*(double)(end.y  -P0.y);
		double	edgeA0= 0;
		double	edgeA1= (double)deltaEdge.x*(double)deltaEdge.x + (double)deltaEdge.y*(double)deltaEdge.y;

		// Test is there is intersection (endpoints included). if yes, return -1. else return 1 (no collision at all).
		if(moveA1>=edgeA0 && edgeA1>=moveA0)
		{
			moveBug= ParallelEdges;
			return -1;
		}
		else
			return 1;
	}

	// if on same side of the line=> there is no collision.
	if( sgnMove0==sgnMove1)
		return 1;

	// test edge against move line.
	sint	sgnEdge0, sgnEdge1;
	sgnEdge0= fsgn(edgeD0);
	sgnEdge1= fsgn(edgeD1);

	// should not have this case, because tested before with (sgnMove==0 && sgnMove1==0).
	nlassert(sgnEdge0!=0 || sgnEdge1!=0);


	// if on same side of the line, no collision against this edge.
	if( sgnEdge0==sgnEdge1 )
		return 1;

	// Here the edges intersect, but ensure that there is no limit problem.
	if(sgnEdge0==0 || sgnEdge1==0)
	{
		moveBug= TraverseEndPoint;
		return -1;
	}
	else if(sgnMove1==0)
	{
		moveBug= StopOnEdge;
		return -1;
	}
	else if(sgnMove0==0)
	{
		// this should not arrive.
		moveBug= StartOnEdge;
		return -1;
	}


	// Here, there is a normal collision, just compute it.
	// Because of Division, there is a precision lost in double. So compute a CRational64.
	// First, compute numerator and denominator in the highest precision. this is 1024*1024 because of prec multiplication.
	double		numerator= (0-moveD0)*1024*1024;
	double		denominator= (moveD1-moveD0)*1024*1024;
	sint64		numeratorInt= (sint64)numerator;
	sint64		denominatorInt= (sint64)denominator;
/*
	nlassert(numerator == numeratorInt);
	nlassert(denominator == denominatorInt);
*/
/*
	if (numerator != numeratorInt)
		nlwarning("numerator(%f) != numeratorInt(%" NL_I64 "d)", numerator, numeratorInt);
	if (denominator != denominatorInt)
		nlwarning("denominator(%f) != denominatorInt(%" NL_I64 "d)", denominator, denominatorInt);
*/
	return CRational64(numeratorInt, denominatorInt);
}


// ***************************************************************************
static	inline float		testCirclePoint(const CVector2f &start, const CVector2f &delta, float radius, const CVector2f &point)
{
	// factors of the qaudratic: at^2 + bt + c=0
	float		a,b,c;
	float		dta;
	float		r0, r1, res;
	CVector2f	relC, relV;

	// As long as delta is not NULL (ensured in testCircleMove() ), this code should not generate Divide by 0.

	// compute quadratic..
	relC= start-point;
	relV= delta;
	a= relV.x*relV.x + relV.y*relV.y;
	// a should be >0. BUT BECAUSE OF PRECISION PROBLEM, it may be ==0, and then cause
	// divide by zero (because b may be near 0, but not 0)
	if(a==0)
	{
		// in this case the move is very small. return 0 if the point is in the circle and if we go toward the point
		if(relC.norm()<radius && relC*delta<0)
			return 0;
		else
			return 1;
	}
	else
	{
		b= 2* (relC.x*relV.x + relC.y*relV.y);
		c= relC.x*relC.x + relC.y*relC.y - radius*radius;
		// compute delta of the quadratic.
		dta= b*b - 4*a*c;	// b^2-4ac
		if(dta>=0)
		{
			dta= (float)sqrt(dta);
			r0= (-b -dta)/(2*a);
			r1= (-b +dta)/(2*a);
			// since a>0, r0<=r1.
			if(r0>r1)
				swap(r0,r1);
			// if r1 is negative, then we are out and go away from this point. OK.
			if(r1<=0)
			{
				res= 1;
			}
			// if r0 is positive, then we may collide this point.
			else if(r0>=0)
			{
				res= min(1.f, r0);
			}
			else	// r0<0 && r1>0. the point is already in the sphere!!
			{
				//nlinfo("COL: Point problem: %.2f, %.2f.  b=%.2f", r0, r1, b);
				// we allow the movement only if we go away from this point.
				// this is true if the derivative at t=0 is >=0 (because a>0).
				if(b>0)
					res= 1;	// go out.
				else
					res=0;
			}
		}
		else
		{
			// never hit this point along this movement.
			res= 1;
		}
	}

	return res;
}


// ***************************************************************************
float		CEdgeCollide::testCircleMove(const CVector2f &start, const CVector2f &delta, float radius, CVector2f &normal)
{
	// If the movement is NULL, return 1 (no collision!)
	if(	delta.isNull() )
		return 1;

	// distance from point to line.
	double	dist= start*Norm + C;
	// projection of speed on normal.
	double	speed= delta*Norm;

	// test if the movement is against the line or not.
	bool	sensPos= dist>0;
	bool	sensSpeed= speed>0;

	// Does the point intersect the line?
	dist= fabs(dist) - radius;
	speed= fabs(speed);
	if( dist > speed )
		return 1;

	// if not already in collision with the line, test when it collides.
	// ===============================
	if(dist>=0)
	{
		// if signs are equals, same side of the line, so we allow the circle to leave the line.
		if(sensPos==sensSpeed )
			return 1;

		// if speed is 0, it means that movement is parralel to the line => never collide.
		if(speed==0)
			return 1;

		// collide the line, at what time.
		double	t= dist/speed;


		// compute the collision position of the Circle on the edge.
		// this gives the center of the sphere at the collision point.
		CVector2d	proj= CVector2d(start) + CVector2d(delta)*t;
		// must add radius vector.
		proj+= Norm * (sensSpeed?radius:-radius);
		// compute projection on edge.
		double		aProj= proj*Dir;

		// if on the interval of the edge.
		if( aProj>=A0 && aProj<=A1)
		{
			// collision occurs on interior of the edge. the normal to return is +- Norm.
			if(sensPos)	// if algebric distance of start position was >0.
				normal= Norm;
			else
				normal= -Norm;

			// return time of collision.
			return (float)t;
		}
	}
	// else, must test if circle collide segment at t=0. if yes, return 0.
	// ===============================
	else
	{
		// There is just need to test if projection of circle's center onto the line hit the segment.
		// This is not a good test to know if a circle intersect a segment, but other cases are
		// managed with test of endPoints of the segment after.
		float		aProj= start*Dir;

		// if on the interval of the edge.
		if( aProj>=A0 && aProj<=A1)
		{
			// if signs are equals, same side of the line, so we allow the circle to leave the edge.
			/* Special case: do not allow to leave the edge if we are too much in the edge.
			 It is important for CGlobalRetriever::testCollisionWithCollisionChains() because of the
 			 "SURFACEMOVE NOT DETECTED" Problem.
			 Suppose we can walk on this chain SA/SB (separate Surface A/SurfaceB). Suppose we are near this edge,
			 and on Surface SA, and suppose there is another chain SB/SC the circle collide with. If we
			 return 1 (no collision), SB/SC won't be detected (because only SA/?? chains will be tested) and
			 so the cylinder will penetrate SB/SC...
			 This case arise at best if chains SA/SB and chain SB/SC do an angle of 45deg
			*/
			if(sensPos==sensSpeed && (-dist)<0.5*radius)
			{
				return 1;
			}
			else
			{
				// hit the interior of the edge, and sensPos!=sensSpeed. So must stop now!!
				// collision occurs on interior of the edge. the normal to return is +- Norm.
				if(sensPos)	// if algebric distance of start position was >0.
					normal= Norm;
				else
					normal= -Norm;

				return 0;
			}
		}
	}

	// In this case, the Circle do not hit the edge on the interior, but may hit on borders.
	// ===============================
	// Then, we must compute collision sphere-points.
	float		tmin, ttmp;
	// first point.
	tmin= testCirclePoint(start, delta, radius, P0);
	// second point.
	ttmp= testCirclePoint(start, delta, radius, P1);
	tmin= min(tmin, ttmp);

	// if collision occurs, compute normal of collision.
	if(tmin<1)
	{
		// to which point we collide?
		CVector2f	colPoint= tmin==ttmp? P1 : P0;
		// compute position of the entity at collision.
		CVector2f	colPos= start + delta*tmin;

		// and so we have this normal (the perpendicular of the tangent at this point).
		normal= colPos - colPoint;
		normal.normalize();
	}

	return tmin;
}



// ***************************************************************************
bool		CEdgeCollide::testEdgeMove(const CVector2f &q0, const CVector2f &q1, const CVector2f &delta, float &tMin, float &tMax, bool &normalOnBox)
{
	double	a,b,cv,cc,  d,e,f;
	CVector2d	tmp;

	// compute D1 line equation of q0q1. bx - ay + c(t)=0, where c is function of time [0,1].
	// ===========================
	tmp= q1 - q0;		// NB: along time, the direction doesn't change.
	// Divide by norm()^2, so that  a projection on this edge is true if the proj is in interval [0,1].
	tmp/= tmp.sqrnorm();
	a= tmp.x;
	b= tmp.y;
	// c= - q0(t)*CVector2d(b,-a).  but since q0(t) is a function of time t (q0+delta*t), compute cv, and cc.
	// so c= cv*t + cc.
	cv= - CVector2d(b,-a)*delta;
	cc= - CVector2d(b,-a)*q0;

	// compute D2 line equation of P0P1. ex - dy + f=0.
	// ===========================
	tmp= P1 - P0;
	// Divide by norm()^2, so that  a projection on this edge is true if the proj is in interval [0,1].
	tmp/= tmp.sqrnorm();
	d= tmp.x;
	e= tmp.y;
	f= - CVector2d(e,-d)*P0;


	// Solve system.
	// ===========================
	/*
		Compute the intersection I of 2 lines across time.
		We have the system:
			bx - ay + c(t)=0
			ex - dy + f=0

		which solve for:
			det= ae-bd	(0 <=> // lines)
			x(t)= (d*c(t) - fa) / det
			y(t)= (e*c(t) - fb) / det
	*/

	// determinant of matrix2x2.
	double	det= a*e - b*d;
	// if to near of 0. (take delta for reference of test).
	if(det==0 || fabs(det)<delta.norm()*EdgeCollideEpsilon)
		return false;

	// intersection I(t)= pInt + vInt*t.
	CVector2d		pInt, vInt;
	pInt.x= ( d*cc - f*a ) / det;
	pInt.y= ( e*cc - f*b ) / det;
	vInt.x= ( d*cv ) / det;
	vInt.y= ( e*cv ) / det;


	// Project Intersection.
	// ===========================
	/*
		Now, we project x,y onto each line D1 and D2, which gives  u(t) and v(t), each one giving the parameter of
		the parametric line function. When it is in [0,1], we are on the edge.

		u(t)= (I(t)-q0(t)) * CVector2d(a,b)	= uc + uv*t
		v(t)= (I(t)-P0) * CVector2d(d,e)	= vc + vv*t
	*/
	double	uc, uv;
	double	vc, vv;
	// NB: q0(t)= q0+delta*t
	uc= (pInt-q0) * CVector2d(a,b);
	uv= (vInt-delta) * CVector2d(a,b);
	vc= (pInt-P0) * CVector2d(d,e);
	vv= (vInt) * CVector2d(d,e);


	// Compute intervals.
	// ===========================
	/*
		Now, for each edge, compute time interval where parameter is in [0,1]. If intervals overlap, there is a collision.
	*/
	double	tu0, tu1, tv0, tv1;
	// infinite interval.
	bool	allU=false, allV=false;

	// compute time interval for u(t).
	if(uv==0 || fabs(uv)<EdgeCollideEpsilon)
	{
		// The intersection does not move along D1. Always projected on u(t)=uc. so if in [0,1], OK, else never collide.
		if(uc<0 || uc>1)
			return false;
		// else suppose "always valid".
		tu0 =tu1= 0;
		allU= true;
	}
	else
	{
		tu0= (0-uc)/uv;	// t for u(t)=0
		tu1= (1-uc)/uv;	// t for u(t)=1
	}

	// compute time interval for v(t).
	if(vv==0 || fabs(vv)<EdgeCollideEpsilon)
	{
		// The intersection does not move along D2. Always projected on v(t)=vc. so if in [0,1], OK, else never collide.
		if(vc<0 || vc>1)
			return false;
		// else suppose "always valid".
		tv0 =tv1= 0;
		allV= true;
	}
	else
	{
		tv0= (0-vc)/vv;	// t for v(t)=0
		tv1= (1-vc)/vv;	// t for v(t)=1
	}


	// clip intervals.
	// ===========================
	// order time interval.
	if(tu0>tu1)
		swap(tu0, tu1);		// now, [tu0, tu1] represent the time interval where line D2 hit the edge D1.
	if(tv0>tv1)
		swap(tv0, tv1);		// now, [tv0, tv1] represent the time interval where line D1 hit the edge D2.

	normalOnBox= false;
	if(!allU && !allV)
	{
		// if intervals do not overlap, no collision.
		if(tu0>tv1 || tv0>tu1)
			return false;
		else
		{
			// compute intersection of intervals.
			tMin= (float)max(tu0, tv0);
			tMax= (float)min(tu1, tv1);
			// if collision of edgeCollide against the bbox.
			if(tv0>tu0)
				normalOnBox= true;
		}
	}
	else if(allU)
	{
		// intersection of Infinite and V interval.
		tMin= (float)tv0;
		tMax= (float)tv1;
		// if collision of edgeCollide against the bbox.
		normalOnBox= true;
	}
	else if(allV)
	{
		// intersection of Infinite and U interval.
		tMin= (float)tu0;
		tMax= (float)tu1;
	}
	else
	{
		// if allU && allV, this means delta is near 0, and so there is always collision.
		tMin= -1000;
		tMax= 1000;
	}

	return true;
}


// ***************************************************************************
float		CEdgeCollide::testBBoxMove(const CVector2f &start, const CVector2f &delta, const CVector2f bbox[4], CVector2f &normal)
{
	// distance from center to line.
	float	dist= start*Norm + C;

	// test if the movement is against the line or not.
	bool	sensPos= dist>0;
	// if signs are equals, same side of the line, so we allow the circle to leave the line.
	/*if(sensPos==sensSpeed)
		return 1;*/


	// Else, do 4 test edge/edge, and return Tmin.
	float	tMin = 0.f, tMax = 0.f;
	bool	edgeCollided= false;
	bool	normalOnBox= false;
	CVector2f	boxNormal(0.f, 0.f);
	for(sint i=0;i<4;i++)
	{
		float	t0, t1;
		bool	nob;
		CVector2f	a= bbox[i];
		CVector2f	b= bbox[(i+1)&3];

		// test move against this edge.
		if(testEdgeMove(a, b, delta, t0, t1, nob))
		{
			if(edgeCollided)
			{
				tMin= min(t0, tMin);
				tMax= max(t1, tMax);
			}
			else
			{
				edgeCollided= true;
				tMin= t0;
				tMax= t1;
			}

			// get normal of box against we collide.
			if(tMin==t0)
			{
				normalOnBox= nob;
				if(nob)
				{
					CVector2f	dab;
					// bbox must be CCW.
					dab= b-a;
					// the normal is computed so that the vector goes In the bbox.
					boxNormal.x= -dab.y;
					boxNormal.y= dab.x;
				}
			}
		}
	}

	// if collision occurs,and int the [0,1] interval...
	if(edgeCollided && tMin<1 && tMax>0)
	{
		// compute normal of collision.
		if(normalOnBox)
		{
			// assume collsion is an endpoint of the edge against the bbox.
			normal= boxNormal;
		}
		else
		{
			// assume collision occurs on interior of the edge. the normal to return is +- Norm.
			if(sensPos)	// if algebric distance of start position was >0.
				normal= Norm;
			else
				normal= -Norm;
		}

		// compute time of collison.
		if(tMin>0)
			// return time of collision.
			return tMin;
		else
		{
			// The bbox is inside the edge, at t==0. test if it goes out or not.
			// accept only if we are much near the exit than the enter.
			/* NB: 0.2 is an empirical value "which works well". Normally, 1 is the good value, but because of the
				"SURFACEMOVE NOT DETECTED" Problem (see testCircleMove()), we must be more restrictive.
			*/
			if( tMax<0.2*(-tMin) )
				return 1;
			else
				return 0;
		}
	}
	else
		return 1;

}


// ***************************************************************************
bool		CEdgeCollide::testBBoxCollide(const CVector2f bbox[4])
{
	// clip the edge against the edge of the bbox.
	CVector2f		p0= P0, p1= P1;

	for(sint i=0; i<4; i++)
	{
		CVector2f	a= bbox[i];
		CVector2f	b= bbox[(i+1)&3];
		CVector2f	delta= b-a, norm;
		// sign is important. bbox is CCW. normal goes OUT the bbox.
		norm.x= delta.y;
		norm.y= -delta.x;

		float	d0= (p0-a)*norm;
		float	d1= (p1-a)*norm;

		// if boths points are out this plane, no collision.
		if( d0>0 && d1>0)
			return false;
		// if difference, must clip.
		if( d0>0 || d1>0)
		{
			CVector2f	intersect= p0 + (p1-p0)* ((0-d0)/(d1-d0));
			if(d1>0)
				p1= intersect;
			else
				p0= intersect;
		}
	}

	// if a segment is still in the bbox, collision occurs.
	return true;
}



} // NLPACS
