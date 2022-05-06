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

#ifndef NL_EDGE_COLLIDE_H
#define NL_EDGE_COLLIDE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2f.h"


namespace NLPACS
{


using	NLMISC::CVector2f;



// ***************************************************************************
/**
 * A 128 bits integer.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CInt128
{
public:
	sint64		High;
	sint64		Low;

public:

	/// Compute a*b, and store in CInt128.
	void	setMul(sint64 a, sint64 b)
	{
		// reset and backup sign.
		sint	sgn=0;
		if(a<0)
			sgn++, a=-a;
		if(b<0)
			sgn--, b=-b;


		// compute unsigned version.
		uint64	high, low, hl1, hl2, oldLow;
		uint32	mask32= 0xFFFFFFFF;
		high= (a>>32) * (b>>32);
		low=  (a&mask32) * (b&mask32);
		hl1=  (a&mask32) * (b>>32);
		hl2=  (a>>32) * (b&mask32);
		// add/adc hl1.
		oldLow= low;
		low+= (hl1&mask32) << 32;
		if(low<oldLow)
			high++;
		high+= (hl1>>32);
		// add/adc hl2.
		oldLow= low;
		low+= (hl2&mask32) << 32;
		if(low<oldLow)
			high++;
		high+= (hl2>>32);


		// extend with sign.
		High= high;
		Low= low;
		if(sgn!=0)
		{
			High= -High;
			Low= -Low;
		}
	}

	/// compare 2 int128.
	bool	operator<(const CInt128 &o) const
	{
		if(High!=o.High)
			return High<o.High;
		else
			return Low<o.Low;
	}

	/// compare 2 int128.
	bool	operator==(const CInt128 &o) const
	{
		return High==o.High && Low==o.Low;
	}

};


// ***************************************************************************
/**
 * A Rational of 2 64 bits.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CRational64
{
public:
	/// Numerator.
	sint64		Numerator;
	/// Denominator. always >0.
	sint64		Denominator;


public:

	CRational64() {}
	CRational64(sint64 a) : Numerator(a), Denominator(1) {}
	CRational64(sint64 num, sint64 den)
	{
		if(den>0)
		{
			Numerator= num;
			Denominator= den;
		}
		else
		{
			Numerator= -num;
			Denominator= -den;
		}
	}

	/// operator<
	bool	operator<(const CRational64 &o) const
	{
		CInt128		n0, n1;
		// a/b < c/d  <=>  a*d < c*b
		n0.setMul(Numerator, o.Denominator);
		n1.setMul(o.Numerator, Denominator);
		return n0<n1;
	}

	/// operator==
	bool	operator==(const CRational64 &o) const
	{
		CInt128		n0, n1;
		// a/b == c/d  <=>  a*d == c*b
		n0.setMul(Numerator, o.Denominator);
		n1.setMul(o.Numerator, Denominator);
		return n0==n1;
	}

};


// ***************************************************************************
/**
 * Collisions against edge in 2D.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CEdgeCollide
{
public:
	enum	TPointMoveProblem {ParallelEdges=0, StartOnEdge, StopOnEdge, TraverseEndPoint, EdgeNull, PointMoveProblemCount};


public:
	CVector2f		P0;
	CVector2f		P1;
	CVector2f		Dir, Norm;
	float			A0, A1;
	float			C;

public:

	void		make(const CVector2f &p0, const CVector2f &p1);

	/** return 1 either if no collision occurs. Else return a [0,1[ interval. return -1 if precision problem (see below).
	 * This method is used by CGlobalRetriever::doMove(). UnManageables cases arise when:
	 *	- the movement start/stop on a edge (dist==0). In this case, don't know on what surface we arrive (before or after).
	 *	- the movement is // to the edge and collide with it. same problem than stop on an edge.
	 *	- the movement traverse the edge on an endpoint. In this case, there is 2+ edges sharing the point, and result is undefined.
	 * On thoses cases, moveBug is filled, and -1 is returned.
	 */
	CRational64	testPointMove(const CVector2f &start, const CVector2f &end, TPointMoveProblem &moveBug);
	/** return 1 either if the circle moves away from the line, or no collision occurs. Else return a [0,1[ interval.
	 * If collision occurs (ie return<1), return in "normal" the normal of the collision.
	 * It may be normal of edge (+-), or normal against a point of the edge.
	 */
	float		testCircleMove(const CVector2f &start, const CVector2f &delta, float radius, CVector2f &normal);
	/** return 1 either if the bbox moves away from the line, or no collision occurs. Else return a [0,1[ interval.
	 * If collision occurs (ie return<1), return in "normal" the normal of the collision.
	 * It may be normal of edge (+-), or normal against a point of the edge.
	 * \param bbox 4 points of the bbox at start. start must be the barycentre of those points.
	 */
	float		testBBoxMove(const CVector2f &start, const CVector2f &delta, const CVector2f bbox[4], CVector2f &normal);
	/** return true if this oriented bbox collide this edge.
	 * \param bbox 4 points of the bbox, in CCW.
	 * \return true if collision occurs.
	 */
	bool		testBBoxCollide(const CVector2f bbox[4]);


// ****************************
private:

	/** test if edge collide against me. if true, return time intervals of collisions (]-oo, +oo[).
	 * NB: for simplicity, if lines are //, return false.
	 */
	bool		testEdgeMove(const CVector2f &q0, const CVector2f &q1, const CVector2f &delta, float &tMin, float &tMax, bool &normalOnBox);

};



} // NLPACS


#endif // NL_EDGE_COLLIDE_H

/* End of edge_collide.h */
