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

#include "nel/misc/hierarchical_timer.h"

#include "nel/pacs/primitive_world_image.h"
#include "nel/pacs/move_primitive.h"
#include "nel/pacs/move_element.h"

using namespace NLMISC;


namespace NLPACS
{

// ***************************************************************************

CPrimitiveWorldImage::CPrimitiveWorldImage()
{
	// Set to NULL
	for (uint i=0; i<4; i++)
		_MoveElement[i]=NULL;

	_DynamicFlags=0;
	_BBXMin=-FLT_MAX;
	_BBXMax=-FLT_MAX;
	_BBYMin=-FLT_MAX;
	_BBYMax=-FLT_MAX;
}

// ***************************************************************************

void CPrimitiveWorldImage::deleteIt (CMoveContainer &container, uint8 worldImage)
{
	// Free the move elements
	for (uint i=0; i<4; i++)
		if (_MoveElement[i])
			removeMoveElement (i, container, worldImage);
}
// ***************************************************************************

void CPrimitiveWorldImage::copy (const CPrimitiveWorldImage& source)
{
	// Copy
	this->operator=(source);

	// Reset some flags
	_DynamicFlags&=~InModifiedListFlag;

	// Pointer into the 4 possibles sorted lists of movable primitives. Must be NULL
	for (uint i=0; i<4; i++)
		_MoveElement[i]=NULL;
}

// ***************************************************************************

bool CPrimitiveWorldImage::evalCollision (CPrimitiveWorldImage& other, CCollisionDesc& desc, double timeMin, double timeMax, uint32 testTime,
									uint32 maxTestIteration, double &firstContactTime, double &lastContactTime, CMovePrimitive& primitive,
									CMovePrimitive& otherPrimitive)
{
//	H_AUTO(PACS_PWI_evalCollision_long);

	// Mask test
	if (( (primitive.getCollisionMaskInternal() & otherPrimitive.getOcclusionMaskInternal())  == 0) &&
		( (primitive.getOcclusionMaskInternal() & otherPrimitive.getCollisionMaskInternal())  == 0))
		return false;

	// Test time
	if ( (!primitive.checkTestTime (testTime, maxTestIteration)) || (!otherPrimitive.checkTestTime (testTime, maxTestIteration)) )
		return false;

	// Clear time min time max
	firstContactTime=FLT_MAX;
	lastContactTime=-FLT_MAX;

	// Switch the good test
	switch (primitive.getPrimitiveTypeInternal())
	{

	// Static box over...
	case UMovePrimitive::_2DOrientedBox:
		{
			// Switch second type
			switch (otherPrimitive.getPrimitiveTypeInternal())
			{

			// Static box over movable box
			case UMovePrimitive::_2DOrientedBox:
				// Make the test
				return evalCollisionOBoverOB (other, desc, timeMin, timeMax, firstContactTime, lastContactTime, primitive, otherPrimitive);

			// Static box over movable cylinder
			case UMovePrimitive::_2DOrientedCylinder:
				// Make the test
				return evalCollisionOBoverOC (other, desc, timeMin, timeMax, firstContactTime, lastContactTime, primitive, otherPrimitive);

			default:
			// Should not go here
			nlstop;
			}
		}

	// Static box over...
	case UMovePrimitive::_2DOrientedCylinder:
		{
			// Switch second type
			switch (otherPrimitive.getPrimitiveTypeInternal())
			{

			// Static box over movable box
			case UMovePrimitive::_2DOrientedBox:
				{
					// Make the test
					bool collid=other.evalCollisionOBoverOC (*this, desc, timeMin, timeMax, firstContactTime, lastContactTime, otherPrimitive,
						primitive);
					if (collid)
						desc.XChgContactNormals ();
					return collid;
				}

			// Static box over movable cylinder
			case UMovePrimitive::_2DOrientedCylinder:
				// Make the test
				return evalCollisionOCoverOC (other, desc, timeMin, timeMax, firstContactTime, lastContactTime, primitive, otherPrimitive);

			default:
			// Should not go here
			nlstop;
			}
		}

	default:
		// Should not go here
		nlstop;
	}

	return false;
}

// ***************************************************************************

const TCollisionSurfaceDescVector *CPrimitiveWorldImage::evalCollision (CGlobalRetriever &retriever, CCollisionSurfaceTemp& surfaceTemp,
																  uint32 testTime, uint32 maxTestIteration, CMovePrimitive& primitive)
{
//	H_AUTO(PACS_PWI_evalCollision_short);

	// Test time
	if (!primitive.checkTestTime (testTime, maxTestIteration))
		return NULL;

	// Switch the good test
	if (primitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedBox)
	{
		// Local I
		CVector locI ((float)(_OBData.EdgeDirectionX[0]*primitive.getLength(0)/2.0), (float)(_OBData.EdgeDirectionY[0]*primitive.getLength(1)/2.0), 0);

		// Local J
		CVector locJ ((float)(_OBData.EdgeDirectionX[1]*primitive.getLength(0)/2.0), (float)(_OBData.EdgeDirectionY[1]*primitive.getLength(1)/2.0), 0);

		// Test
		return retriever.testBBoxMove (_Position.getGlobalPos (), _DeltaPosition, locI, locJ, surfaceTemp);
	}
	else
	{
		// Check
		nlassert (primitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedCylinder);

		// Test
		//nlinfo ("1) %f %f %f\n", _DeltaPosition.x, _DeltaPosition.y, _DeltaPosition.z);

		return retriever.testCylinderMove (_Position.getGlobalPos (), _DeltaPosition, primitive.getRadiusInternal(), surfaceTemp);
	}
}

// ***************************************************************************

void CPrimitiveWorldImage::doMove (CGlobalRetriever &retriever, CCollisionSurfaceTemp& surfaceTemp, double originalMax, double finalMax, bool keepZ /*= false*/)
{
	H_AUTO(NLPACS_PWI_Do_Move);


	// Time to avance
	double ratio;
	if (finalMax!=originalMax)
		ratio=(finalMax-_InitTime)/(originalMax-_InitTime);
	else
		ratio=1;

	// Make the move
	if (!keepZ)
	{
		_Position.setGlobalPos (retriever.doMove(_Position.getGlobalPos(), _DeltaPosition, (float)ratio, surfaceTemp, false), retriever);
	}
	else
	{
		_Position.setGlobalPosKeepZ(retriever.doMove(_Position.getGlobalPos(), _DeltaPosition, (float)ratio, surfaceTemp, false), retriever);
	}


	// Final position
	_InitTime=finalMax;
}

// ***************************************************************************

void CPrimitiveWorldImage::doMove (double timeMax)
{
//	H_AUTO(PACS_PWI_doMove_short);

	// Make the move
	_Position.setPos (_Position.getPos ()+_Speed*(timeMax-_InitTime));

	// Final position
	_InitTime=timeMax;
}

// ***************************************************************************

bool CPrimitiveWorldImage::evalCollisionOBoverOB (CPrimitiveWorldImage& other, CCollisionDesc& desc, double timeMin, double timeMax,
											double &firstContactTime, double &lastContactTime, CMovePrimitive& primitive,
										   CMovePrimitive& otherPrimitive)
{
	// Checks
	nlassert (primitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedBox);
	nlassert (otherPrimitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedBox);

	// Find a collision
	bool find=false;

	// Best time
	desc.ContactTime=FLT_MAX;

	// Timemin
	double _timeMax=-FLT_MAX;

	// Check movable points over the edge
	uint pt;
	uint seg;
	for (pt=0; pt<4; pt++)
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if ( evalCollisionPoverS (other, d, pt, seg, primitive, otherPrimitive) )
		{
			// Find
			find=true;

			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
			}

			// Best max time ?
			if (d.ContactTime>_timeMax)
			{
				// This is the new max time
				_timeMax=d.ContactTime;
			}
		}
	}

	// Check static points over the movable box
	for (pt=0; pt<4; pt++)
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if (other.evalCollisionPoverS (*this, d, pt, seg, primitive, otherPrimitive))
		{
			// Find
			find=true;

			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
			}

			// Best max time ?
			if (d.ContactTime>_timeMax)
			{
				// This is the new max time
				_timeMax=d.ContactTime;
			}
		}
	}

	if (find)
	{
		// First last contact time
		firstContactTime=desc.ContactTime;
		lastContactTime=_timeMax;

		// Half time
		//double halfTime = (_timeMax+desc.ContactTime)/2.0;

		// Collision in the past ?
		//if (timeMin > halfTime)
		if (timeMin > _timeMax)
			// yes, abort
			return false;

		// Collision not in the future ?
		if (timeMax>desc.ContactTime)
		{
			// Clamp time
			if (desc.ContactTime<timeMin)
				desc.ContactTime=timeMin;

			// yes, found it
			return true;
		}
	}

	// No collision found
	return false;
}

// ***************************************************************************

bool CPrimitiveWorldImage::evalCollisionOBoverOC (CPrimitiveWorldImage& other, CCollisionDesc& desc, double timeMin, double timeMax,
											double &firstContactTime, double &lastContactTime, CMovePrimitive& primitive,
										   CMovePrimitive& otherPrimitive)
{
	// Checks
	nlassert (primitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedBox);
	nlassert (otherPrimitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedCylinder);

	// Find a collision
	bool find=false;

	// Best time
	desc.ContactTime=FLT_MAX;

	// time min clip
	double _timeMax = -FLT_MAX;

	// Check movable points over the cylinder
	uint pt;
	for (pt=0; pt<4; pt++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		double firstContactTime;
		double lastContactTime;
		if (evalCollisionPoverOC (other, d, pt, firstContactTime, lastContactTime, primitive, otherPrimitive))
		{
			// Found
			find=true;

			// Best time ?
			if (firstContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
			}

			// Best max time ?
			if (lastContactTime>_timeMax)
			{
				// New max time
				_timeMax=lastContactTime;
			}
		}
	}

	// Check static points over the movable box
	uint seg;
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if (evalCollisionSoverOC (other, d, seg, primitive, otherPrimitive))
		{
			// Found
			find=true;

			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
			}

			// Best max time ?
			if (d.ContactTime>_timeMax)
			{
				// New max time
				_timeMax=d.ContactTime;
			}
		}
	}

	if (find)
	{
		// First last contact time
		firstContactTime=desc.ContactTime;
		lastContactTime=_timeMax;

		// Half time
		//double halfTime = (_timeMax+desc.ContactTime)/2.0;

		// Collision in the past ?
		//if (timeMin > halfTime)
		if (timeMin > _timeMax)
			// yes, abort
			return false;

		// Collision not in the future ?
		if (timeMax>desc.ContactTime)
		{
			// Clamp time
			if (desc.ContactTime<timeMin)
				desc.ContactTime=timeMin;

			// yes, found it
			return true;
		}
	}

	// No collision found
	return false;
}

// ***************************************************************************

bool CPrimitiveWorldImage::evalCollisionPoverS (CPrimitiveWorldImage& other, CCollisionDesc& desc, uint numPoint, uint numSeg,
												CMovePrimitive& primitive, CMovePrimitive& otherPrimitive)
{
	// Checks
	nlassert (primitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedBox);
	nlassert (otherPrimitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedBox);

	// Some constants
	const double normalSegX=other._OBData.EdgeDirectionY[numSeg];
	const double normalSegY=-other._OBData.EdgeDirectionX[numSeg];

  	// Relative speed
	const double speedX=other._Speed.x-_Speed.x;
	const double speedY=other._Speed.y-_Speed.y;

	// Dot product with the plan tangeante
	double dotProd= speedX*normalSegX + speedY*normalSegY;
	//if ( dotProd > 0 )
	if ( dotProd != 0 )
	{
		// Time of the collision
		double time= (normalSegX*(_OBData.PointPosX[numPoint] - other._OBData.PointPosX[numSeg]) +
			normalSegY*(_OBData.PointPosY[numPoint] - other._OBData.PointPosY[numSeg])) / dotProd;

		// Position of segment point at collision time
		const double segPosX= other._OBData.PointPosX[numSeg] + other._Speed.x*time;
		const double segPosY= other._OBData.PointPosY[numSeg] + other._Speed.y*time;

		// Position of the point at collision time
		const double ptPosX= _OBData.PointPosX[numPoint] + _Speed.x*time;
		const double ptPosY= _OBData.PointPosY[numPoint] + _Speed.y*time;

		// Direction of the collision on the segment
		const double dirX= ptPosX - segPosX;
		const double dirY= ptPosY - segPosY;

		// Length of this vector
		const double length= dirY*normalSegX - dirX*normalSegY;

		// Included ?
		if ( ( length >= 0 ) && ( length <= otherPrimitive.getLength(numSeg&1) ) )
		{
			// 2d Collid checked... Now check height

			// Pos Z
			const double pointSegZ=other._3dInitPosition.z;
			const double segPosZ= pointSegZ + other._Speed.z*time;

			// Some constants
			const double pointZ=_3dInitPosition.z;
			const double ptPosZ= pointZ + _Speed.z*time;

			// Included ?
			if ( (ptPosZ <= segPosZ + otherPrimitive.getHeightInternal()) && (ptPosZ + primitive.getHeightInternal() >= segPosZ) )
			{
				// Ok Collision, fill the result

				// Time
				desc.ContactTime=time;

				// Position
				desc.ContactPosition.x=ptPosX;
				desc.ContactPosition.y=ptPosY;
				desc.ContactPosition.z=std::max (segPosZ, ptPosZ);

				// Seg box normal
				desc.ContactNormal1.x=normalSegX;
				desc.ContactNormal1.y=normalSegY;
				desc.ContactNormal1.z=0;
				desc.ContactNormal0.x=-desc.ContactNormal1.x;
				desc.ContactNormal0.y=-desc.ContactNormal1.y;
				desc.ContactNormal0.z=0;

				// End
				return true;
			}
		}
	}

	// No collision
	return false;
}

// ***************************************************************************

inline uint secondDegree (double a, double b, double c, double& s0, double& s1)
{
	double d=b*b-4.f*a*c;
	if (d>0)
	{
		// sqrt d
		d=(double)sqrt (d);

		// 1 / 2a
		a=0.5f/a;

		// 2 solutions
		s0 = (-b-d)*a;
		s1 = (-b+d)*a;

		return 2;
	}
	else if (d<0)
	{
		// No solution
		return 0;
	}
	else
	{
		// 1 solution
		s0 = -b/(2.f*a);

		return 1;
	}
}

// ***************************************************************************

bool CPrimitiveWorldImage::evalCollisionPoverOC (CPrimitiveWorldImage& other, CCollisionDesc& desc, uint numPoint,
										   double &firstContactTime, double &lastContactTime, CMovePrimitive& primitive,
										   CMovePrimitive& otherPrimitive)
{
	// Checks
	nlassert (primitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedBox);
	nlassert (otherPrimitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedCylinder);

	/* Point Equ:
	 * p(t) = p0 + v0*(t - t0)
	 *
	 * Cylinder center Equ:
	 * p'(t) = p'0 + v'0*(t - t'0)
	 *
	 * Find t for this equation:
	 * R^2 = Norm^2 (p(t) - p'(t))
	 * R^2 = Norm^2 ( p0 + v0 ( t - t0 ) - p'0 - v'0 ( t - t'0 ) )
	 *
	 * A = p0 - v0*t0 - p'0 + v'0*t'0
	 * B = (v0 - v'0)
	 *
	 * Norm^2 (B)*t^2 + 2*(A.B)*t + Norm^2 (A) - R^2 = 0
	 *
	 * a = Norm^2 (B)
	 * b = 2*(A.B)
	 * c = Norm^2 (A) - R^2
	 *
	 * a*t^2 + b*t + c = 0
	 */

	// Let's go
	const double _Ax = _OBData.PointPosX[numPoint] - other._3dInitPosition.x;
	const double _Ay = _OBData.PointPosY[numPoint] - other._3dInitPosition.y;
	const double _Bx = _Speed.x - other._Speed.x;
	const double _By = _Speed.y - other._Speed.y;

	// Eval system
	double s0, s1;
	double squareRadius=otherPrimitive.getRadiusInternal()*otherPrimitive.getRadiusInternal();
	uint numSolution=secondDegree (_Bx*_Bx+_By*_By, 2.f*(_Ax*_Bx+_Ay*_By), _Ax*_Ax+_Ay*_Ay-squareRadius, s0, s1);
	if (numSolution!=0)
	{
		// time
		double time;

		// Collision time
		if (numSolution==1)
		{
			firstContactTime=s0;
			lastContactTime=s0;
		}
		else
		{
			// First and last time
			if (s0<s1)
			{
				firstContactTime=s0;
				lastContactTime=s1;
			}
			else
			{
				firstContactTime=s1;
				lastContactTime=s0;
			}
		}
		time=firstContactTime;

		// Pos Z
		const double pointCylZ=other._3dInitPosition.z;
		const double cylPosZ= pointCylZ + other._Speed.z*time;

		// Some constants
		const double pointZ=_3dInitPosition.z;
		const double ptPosZ= pointZ + _Speed.z*time;

		// Z Included ?
		if ( (ptPosZ <= cylPosZ + otherPrimitive.getHeightInternal()) && (ptPosZ + primitive.getHeightInternal() >= cylPosZ) )
		{
			// Ok Collision, fill the result

			// Time
			desc.ContactTime=time;

			// Point position
			const double ptPosX= _OBData.PointPosX[numPoint] + _Speed.x*time;
			const double ptPosY= _OBData.PointPosY[numPoint] + _Speed.y*time;

			// Cylinder position
			const double cylPosX= other._3dInitPosition.x + other._Speed.x*time;
			const double cylPosY= other._3dInitPosition.y + other._Speed.y*time;

			// Position
			desc.ContactPosition.x=ptPosX;
			desc.ContactPosition.y=ptPosY;
			desc.ContactPosition.z=std::max (cylPosZ, ptPosZ);

			// Cylinder normal
			desc.ContactNormal1.x=ptPosX-cylPosX;
			desc.ContactNormal1.y=ptPosY-cylPosY;
			desc.ContactNormal1.z=0;
			desc.ContactNormal1.normalize ();
			desc.ContactNormal0.x=-desc.ContactNormal1.x;
			desc.ContactNormal0.y=-desc.ContactNormal1.y;
			desc.ContactNormal0.z=0;

			// End
			return true;
		}
	}

	// No collision
	return false;
}

// ***************************************************************************

bool CPrimitiveWorldImage::evalCollisionSoverOC (CPrimitiveWorldImage& other, CCollisionDesc& desc, uint numSeg, CMovePrimitive& primitive,
										   CMovePrimitive& otherPrimitive)
{
	// Checks
	nlassert (primitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedBox);
	nlassert (otherPrimitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedCylinder);

	// Some constants
	const double normalSegX=_OBData.EdgeDirectionY[numSeg];
	const double normalSegY=-_OBData.EdgeDirectionX[numSeg];

	// Relative speed
	const double speedX=other._Speed.x-_Speed.x;
	const double speedY=other._Speed.y-_Speed.y;

	// Dot product with the plan tangeante
	double dotProd= speedX*normalSegX + speedY*normalSegY;
	//if ( dotProd < 0 )
	if ( dotProd !=0 )
	{
		// Time of the collision
		double time= (otherPrimitive.getRadiusInternal() + normalSegX*(_OBData.PointPosX[numSeg] - other._3dInitPosition.x ) +
			normalSegY*(_OBData.PointPosY[numSeg] - other._3dInitPosition.y ) ) / dotProd;

		// Position of segment point at collision time
		const double segPosX= _OBData.PointPosX[numSeg] + _Speed.x*time;
		const double segPosY= _OBData.PointPosY[numSeg] + _Speed.y*time;

		// Position of the cylinder at collision time
		const double cylPosX= other._3dInitPosition.x + _Speed.x*time;
		const double cylPosY= other._3dInitPosition.y + _Speed.y*time;

		// Position de contact
		const double contactX= cylPosX - normalSegX*otherPrimitive.getRadiusInternal();
		const double contactY= cylPosY - normalSegY*otherPrimitive.getRadiusInternal();

		// Direction of the collision on the segment
		const double dirX= contactX - segPosX;
		const double dirY= contactY - segPosY;

		// Length of this vector
		const double length= dirY*normalSegX - dirX*normalSegY;

		// Included ?
		if ( ( length >= 0 ) && ( length <= primitive.getLength (numSeg&1) ) )
		{
			// 2d Collid checked... Now check height

			// Pos Z
			const double segPosZ= _3dInitPosition.z + _Speed.z*time;

			// Some constants
			const double cylPosZ= other._3dInitPosition.z + other._Speed.z*time;

			// Included ?
			if ( (cylPosZ <= segPosZ + primitive.getHeightInternal() ) && (cylPosZ + otherPrimitive.getHeightInternal() >= segPosZ) )
			{
				// Ok Collision, fill the result

				// Time
				desc.ContactTime=time;

				// Position
				desc.ContactPosition.x=contactX;
				desc.ContactPosition.y=contactY;
				desc.ContactPosition.z=std::max (segPosZ, cylPosZ);

				// Segment normal
				desc.ContactNormal0.x=normalSegX;
				desc.ContactNormal0.y=normalSegY;
				desc.ContactNormal0.z=0;

				// Seg box normal
				desc.ContactNormal1.x=contactX-cylPosX;
				desc.ContactNormal1.y=contactY-cylPosY;
				desc.ContactNormal1.z=0;
				desc.ContactNormal1.normalize ();

				// End
				return true;
			}
		}
	}

	// No collision
	return false;
}


// ***************************************************************************

bool CPrimitiveWorldImage::evalCollisionOCoverOC (CPrimitiveWorldImage& other, CCollisionDesc& desc, double timeMin, double timeMax,
											double &firstContactTime, double &lastContactTime, CMovePrimitive& primitive,
										   CMovePrimitive& otherPrimitive)
{
	// Checks
	nlassert (primitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedCylinder);
	nlassert (otherPrimitive.getPrimitiveTypeInternal()==UMovePrimitive::_2DOrientedCylinder);


	/* Cylinder0 center equ:
	 * p(t) = p0 + v0*(t - t0)
	 *
	 * Cylinder1 center equ:
	 * p'(t) = p'0 + v'0*(t - t'0)
	 *
	 * Find t for this equation:
	 * (R + R')^2 = Norm^2 (p(t) - p'(t))
	 * (R + R')^2 = Norm^2 ( p0 + v0 ( t - t0 ) - p'0 - v'0 ( t - t'0 ) )
	 *
	 * A = p0 - v0*t0 - p'0 + v'0*t'0
	 * B = (v0 - v'0)
	 *
	 * Norm^2 (B)*t^2 + 2*(A.B)*t + Norm^2 (A) - (R + R')^2 = 0
	 *
	 * a = Norm^2 (B)
	 * b = 2*(A.B)
	 * c = Norm^2 (A) - (R + R')^2
	 *
	 * a*t^2 + b*t + c = 0
	 */

	// Let's go
	const double _Ax = _3dInitPosition.x - other._3dInitPosition.x;
	const double _Ay = _3dInitPosition.y - other._3dInitPosition.y;
	const double _Bx = _Speed.x - other._Speed.x;
	const double _By = _Speed.y - other._Speed.y;

	// Eval system
	double s0, s1;
	double radiusSquare=primitive.getRadiusInternal()+otherPrimitive.getRadiusInternal();
	radiusSquare*=radiusSquare;
	uint numSolution=secondDegree (_Bx*_Bx+_By*_By, 2.f*(_Ax*_Bx+_Ay*_By), _Ax*_Ax+_Ay*_Ay-radiusSquare, s0, s1);
	if (numSolution!=0)
	{
		// time
		double _timeMin, _timeMax;

		// Collision time
		if (numSolution==1)
		{
			_timeMin=s0;
			_timeMax=s0;
		}
		else
		{
			// Time min and max
			if (s0>s1)
			{
				_timeMin=s1;
				_timeMax=s0;
			}
			else
			{
				_timeMin=s0;
				_timeMax=s1;
			}
		}

		// half time
		//const double halfTime=(_timeMin+_timeMax)/2.0;

		// Conatct time
		firstContactTime=_timeMin;
		lastContactTime=_timeMax;

		// Clip time
		if ((timeMin<_timeMax)&&(_timeMin<timeMax))
		{
			// Some constants
			const double cyl0Time= _timeMin;
			const double pointCyl0Z=_3dInitPosition.z;
			const double cyl0PosZ= pointCyl0Z + _Speed.z*cyl0Time;

			// Pos Z
			const double cyl1Time= _timeMin;
			const double pointCyl1Z=other._3dInitPosition.z;
			const double cyl1PosZ= pointCyl1Z + other._Speed.z * cyl1Time;

			// Z Included ?
			if ( (cyl0PosZ <= cyl1PosZ + otherPrimitive.getHeightInternal() ) && (cyl0PosZ + primitive.getHeightInternal() >= cyl1PosZ) )
			{
				// Ok Collision, fill the result

				// Time
				desc.ContactTime=std::max (_timeMin, timeMin);

				// Cylinder 0 position
				const double cyl0PosX= _3dInitPosition.x + _Speed.x*cyl0Time;
				const double cyl0PosY= _3dInitPosition.y + _Speed.y*cyl0Time;

				// Cylinder 1 position
				const double cyl1PosX= other._3dInitPosition.x + other._Speed.x*cyl1Time;
				const double cyl1PosY= other._3dInitPosition.y + other._Speed.y*cyl1Time;

				// First cylinder normal
				desc.ContactNormal0.x= cyl1PosX - cyl0PosX;
				desc.ContactNormal0.y= cyl1PosY - cyl0PosY;
				desc.ContactNormal0.z= 0;
				desc.ContactNormal0.normalize ();

				// Contact position
				desc.ContactPosition.x= desc.ContactNormal0.x*primitive.getRadiusInternal() + cyl0PosX;
				desc.ContactPosition.y= desc.ContactNormal0.y*primitive.getRadiusInternal() + cyl0PosY;
				desc.ContactPosition.z= std::max (cyl0PosZ, cyl1PosZ);

				// Second cylinder normal
				desc.ContactNormal1.x= -desc.ContactNormal0.x;
				desc.ContactNormal1.y= -desc.ContactNormal0.y;
				desc.ContactNormal1.z= 0;

				// End
				return true;
			}
		}
	}

	// No collision
	return false;
}

// ***************************************************************************

void CPrimitiveWorldImage::precalcPos (CMovePrimitive &primitive)
{
	// Type of the primitive
	uint type=primitive.getPrimitiveTypeInternal();

	// Box ?
	if (type==UMovePrimitive::_2DOrientedBox)
	{
		// Calc cosinus and sinus
		double cosinus=(double)cos(_OBData.Orientation);
		double sinus=(double)sin(_OBData.Orientation);

		// Size
		double halfWidth=primitive.getLength (0)/2;
		double halfDepth=primitive.getLength (1)/2;

		// First point
		_OBData.PointPosX[0]=cosinus*(-halfWidth)-sinus*(-halfDepth)+_3dInitPosition.x;
		_OBData.PointPosY[0]=sinus*(-halfWidth)+cosinus*(-halfDepth)+_3dInitPosition.y;

		// Second point
		_OBData.PointPosX[1]=cosinus*halfWidth-sinus*(-halfDepth)+_3dInitPosition.x;
		_OBData.PointPosY[1]=sinus*halfWidth+cosinus*(-halfDepth)+_3dInitPosition.y;

		// Third point
		_OBData.PointPosX[2]=cosinus*halfWidth-sinus*halfDepth+_3dInitPosition.x;
		_OBData.PointPosY[2]=sinus*halfWidth+cosinus*halfDepth+_3dInitPosition.y;

		// Fourth point
		_OBData.PointPosX[3]=cosinus*(-halfWidth)-sinus*halfDepth+_3dInitPosition.x;
		_OBData.PointPosY[3]=sinus*(-halfWidth)+cosinus*halfDepth+_3dInitPosition.y;

		// Direction
		double length0 = (primitive.getLength(0)==0)? 0.001 : primitive.getLength(0);
		double length1 = (primitive.getLength(1)==0)? 0.001 : primitive.getLength(1);
		double oneOverLength[2]= { 1 / length0, 1 / length1 };

		// Direction
		uint i;
		for (i=0; i<4; i++)
		{
			// Next index
			uint next=(i+1)&3;
			double oneOver=oneOverLength[i&1];

			// New direction
			_OBData.EdgeDirectionX[i]=(_OBData.PointPosX[next] - _OBData.PointPosX[i])*oneOver;
			_OBData.EdgeDirectionY[i]=(_OBData.PointPosY[next] - _OBData.PointPosY[i])*oneOver;
		}
	}
	else
	{
		// Should be a cylinder
		nlassert (type==UMovePrimitive::_2DOrientedCylinder);
	}
}

// ***************************************************************************

void CPrimitiveWorldImage::precalcBB (double beginTime, double endTime, CMovePrimitive &primitive)
{
	// Type of the primitive
	uint type=primitive.getPrimitiveTypeInternal();

	// Box ?
	if (type==UMovePrimitive::_2DOrientedBox)
	{
		// Orientation index
		sint orient= (sint)(256.f*_OBData.Orientation/(2.f*NLMISC::Pi));
		orient&=0xff;
		orient>>=6;
		nlassert (orient>=0);
		nlassert (orient<4);

		// Compute coordinates
		_BBXMin=FLT_MAX;
		_BBYMin=FLT_MAX;
		_BBXMax=-FLT_MAX;
		_BBYMax=-FLT_MAX;

		for (uint i=0; i<4; i++)
		{
			if (_OBData.PointPosX[i]<_BBXMin)
				_BBXMin=_OBData.PointPosX[i];
			if (_OBData.PointPosX[i]>_BBXMax)
				_BBXMax=_OBData.PointPosX[i];
			if (_OBData.PointPosY[i]<_BBYMin)
				_BBYMin=_OBData.PointPosY[i];
			if (_OBData.PointPosY[i]>_BBYMax)
				_BBYMax=_OBData.PointPosY[i];
		}
		_BBXMin=std::min (std::min (_BBXMin, _BBXMin+endTime*_Speed.x), _BBXMin+beginTime*_Speed.x);
		_BBXMax=std::max (std::max (_BBXMax, _BBXMax+endTime*_Speed.x), _BBXMax+beginTime*_Speed.x);
		_BBYMin=std::min (std::min (_BBYMin, _BBYMin+endTime*_Speed.y), _BBYMin+beginTime*_Speed.y);
		_BBYMax=std::max (std::max (_BBYMax, _BBYMax+endTime*_Speed.y), _BBYMax+beginTime*_Speed.y);

/*
		// This code is faster but buggy..
		_BBXMin= _OBData.PointPosX[minX[orient]] + _Speed.x*beginTime;
		_BBXMin= std::min (_BBXMin, _OBData.PointPosX[minX[orient]] + _Speed.x*endTime);

		_BBYMin= _OBData.PointPosY[minY[orient]] + _Speed.y*beginTime;
		_BBYMin= std::min (_BBYMin, _OBData.PointPosY[minY[orient]] + _Speed.y*endTime);

		_BBXMax= _OBData.PointPosX[maxX[orient]] + _Speed.x*beginTime;
		_BBXMax= std::max (_BBXMax, _OBData.PointPosX[maxX[orient]] + _Speed.x*endTime);

		_BBYMax= _OBData.PointPosY[maxY[orient]] + _Speed.y*beginTime;
		_BBYMax= std::max (_BBYMax, _OBData.PointPosY[maxY[orient]] + _Speed.y*endTime);*/
	}
	else
	{
		// Should be a cylinder
		nlassert (type==UMovePrimitive::_2DOrientedCylinder);

		// Compute X coordinates
		_BBXMin= _3dInitPosition.x + _Speed.x*beginTime;
		_BBXMax= _3dInitPosition.x + _Speed.x*endTime;
		if (_BBXMin>_BBXMax)
		{
			double tmp=_BBXMin;
			_BBXMin=_BBXMax;
			_BBXMax=tmp;
		}
		_BBXMin-=primitive.getRadiusInternal();
		_BBXMax+=primitive.getRadiusInternal();

		// Compute Y coordinates
		_BBYMin= _3dInitPosition.y + _Speed.y*beginTime;
		_BBYMax= _3dInitPosition.y + _Speed.y*endTime;
		if (_BBYMin>_BBYMax)
		{
			double tmp=_BBYMin;
			_BBYMin=_BBYMax;
			_BBYMax=tmp;
		}
		_BBYMin-=primitive.getRadiusInternal();
		_BBYMax+=primitive.getRadiusInternal();
	}

	// Delta position
	_DeltaPosition=_Speed*(endTime-beginTime);
}

// ***************************************************************************

void CPrimitiveWorldImage::addMoveElement (CMoveCell& cell, uint16 x, uint16 y, double centerX, double /* centerY */, CMovePrimitive *primitive,
										   CMoveContainer &container, uint8 worldImage)
{
	// Find a free place
	uint slot;
	for (slot=0; slot<4; slot++)
	{
		// Empty ?
		if (_MoveElement[slot]==NULL)
		{
			// Primitive center
			double cx=(_BBXMin+_BBXMax)/2.f;

			// Allocate move element
			_MoveElement[slot]=container.allocateMoveElement ();
			_MoveElement[slot]->Primitive=primitive;
			_MoveElement[slot]->X=x;
			_MoveElement[slot]->Y=y;

			// Insert in left or right ?
			if (cx<centerX)
				// In the left
				cell.linkFirstX (_MoveElement[slot]);
			else
				// In the right
				cell.linkLastX (_MoveElement[slot]);

			/*// Insert in left or right ?
			if (cy<centerY)
				// In the left
				cell.linkFirstY (_MoveElement[slot]);
			else
				// In the right
				cell.linkLastY (_MoveElement[slot]);*/

			// Move it
			 cell.updateSortedLists (_MoveElement[slot], worldImage);

			// End
			break;
		}
	}
}

// ***************************************************************************

void CPrimitiveWorldImage::addMoveElementendOfList (CMoveCell& cell, uint16 x, uint16 y, CMovePrimitive *primitive,
													CMoveContainer &container)
{
	// Find a free place
	uint slot;
	for (slot=0; slot<4; slot++)
	{
		// Empty ?
		if (_MoveElement[slot]==NULL)
		{
			// Allocate move element
			_MoveElement[slot]=container.allocateMoveElement ();
			_MoveElement[slot]->Primitive=primitive;
			_MoveElement[slot]->X=x;
			_MoveElement[slot]->Y=y;

			// In the right
			cell.linkLastX (_MoveElement[slot]);

			// End
			break;
		}
	}
}

// ***************************************************************************

void CPrimitiveWorldImage::removeMoveElement (uint i, CMoveContainer &container, uint8 worldImage)
{
	// Check
	nlassert (i<4);
	nlassert (_MoveElement[i]!=NULL);

	// Unlink the element
	container.unlinkMoveElement (_MoveElement[i], worldImage);

	// Free the move element
	container.freeMoveElement (_MoveElement[i]);

	// Set to NULL
	_MoveElement[i]=NULL;
}

// ***************************************************************************

void CPrimitiveWorldImage::checkSortedList (uint8 worldImage)
{
	// For the 4 elements
	for (uint i=0; i<4; i++)
	{
		// element here ?
		if (_MoveElement[i])
		{
			if (_MoveElement[i]->PreviousX)
				nlassertonce (_MoveElement[i]->PreviousX->Primitive->getWorldImage(worldImage)->_BBXMin <= _BBXMin);
			if (_MoveElement[i]->NextX)
				nlassertonce (_BBXMin <= _MoveElement[i]->NextX->Primitive->getWorldImage(worldImage)->_BBXMin);
		}
	}
}

// ***************************************************************************

void CPrimitiveWorldImage::reaction (CPrimitiveWorldImage& second, const CCollisionDesc& desc, CGlobalRetriever* retriever,
							   CCollisionSurfaceTemp& surfaceTemp, bool collision, CMovePrimitive &primitive,
							   CMovePrimitive &otherPrimitive, CMoveContainer *container, uint8 worldImage, uint8 secondWorldImage,
							   bool secondConst)
{
//	H_AUTO(PACS_PWI_reaction_long);

	// Get the two reaction codes
	UMovePrimitive::TReaction firstReaction=primitive.getReactionTypeInternal();
	UMovePrimitive::TReaction secondReaction=otherPrimitive.getReactionTypeInternal();

	// Overide collsion
	collision = collision && (primitive.isObstacle ()) && (otherPrimitive.isObstacle ());

	// Get the two mass
	float mass0 = primitive.getMass ();
	float mass1 = otherPrimitive.getMass ();

	// Energy sum
	double projSpeed0 = desc.ContactNormal1 * _Speed;
	double projSpeed1 = desc.ContactNormal0 * second._Speed;
	double energySum = (- mass0 * projSpeed0 - mass1 * projSpeed1 ) / 2.0;

	// Old position
	CVectorD collisionPosition=_3dInitPosition;
	collisionPosition+=_Speed*desc.ContactTime;

	// Calc new speed
	CVectorD newSpeed(0.0, 0.0, 0.0);

	// Obstacle ?
	if (collision)
	{
		switch (firstReaction)
		{
		case UMovePrimitive::Slide:
			// Remove projected speed
			newSpeed=_Speed - projSpeed0 * desc.ContactNormal1;

			// Reflexion speed
			newSpeed+=( primitive.getAttenuation()*energySum / mass0 ) * desc.ContactNormal1;
			break;
		case UMovePrimitive::Reflexion:
			// Remove projected speed
			newSpeed=_Speed - projSpeed0 * desc.ContactNormal1;

			// Reflexion speed
			newSpeed+=( primitive.getAttenuation()*energySum / mass0 ) * desc.ContactNormal1;
			break;
		case UMovePrimitive::Stop:
			newSpeed.set (0,0,0);
			break;
		case UMovePrimitive::DoNothing:
			newSpeed=_Speed;
			break;
		default: break;
		}

		// Set new speed
		setSpeed (newSpeed, container, &primitive, worldImage);

		// New position at t=0
		if (retriever)
		{
			// Make a domove in the Ben data
			double	deltaDist= _DeltaPosition.norm();
			double	deltaTime;
			if(deltaDist<0.000001)
				deltaTime= 0;
			else
				deltaTime=(collisionPosition-_Position.getPos ()).norm()/deltaDist;
			nlassert (deltaTime>=0);
			nlassert (deltaTime<=1);

			UGlobalPosition newPosition = retriever->doMove (_Position.getGlobalPos (), _DeltaPosition,
				(float)deltaTime, surfaceTemp, true);

			// Set the new position
			_Position.setGlobalPos (newPosition, *retriever);

			// Position at t=0
			_3dInitPosition = _Position.getPos() - newSpeed * desc.ContactTime;

			// New init time
			_InitTime = desc.ContactTime;
		}
		else
		{
			// No retriever used
			_Position.setPos (collisionPosition);

			// Position at t=0
			_3dInitPosition = collisionPosition - newSpeed * desc.ContactTime;

			// New init time
			_InitTime = desc.ContactTime;
		}

		// Dirt pos
		dirtPos (container, &primitive, worldImage);

		// ****** Second object

		// Is second object in a static world ?
		if (!secondConst)
		{
			// Old position
			collisionPosition=second._3dInitPosition;
			collisionPosition+=second._Speed * desc.ContactTime;

			// Obstacle ?
			switch (secondReaction)
			{
			case UMovePrimitive::Slide:
				// Remove projected speed
				newSpeed=second._Speed - projSpeed1 * desc.ContactNormal0;

				// Reflexion speed
				newSpeed+=( otherPrimitive.getAttenuation()*energySum / mass1 ) * desc.ContactNormal1;
				break;
			case UMovePrimitive::Reflexion:
				// Remove projected speed
				newSpeed=second._Speed - projSpeed1 * desc.ContactNormal0;

				// Reflexion speed
				newSpeed+=( otherPrimitive.getAttenuation()*energySum / mass1 ) * desc.ContactNormal0;
				break;
			case UMovePrimitive::Stop:
				newSpeed.set (0,0,0);
				break;
			case UMovePrimitive::DoNothing:
				newSpeed=second._Speed;
				break;
			default: break;
			}

			// Set new speed
			second.setSpeed (newSpeed, container, &otherPrimitive, secondWorldImage);

			// New position at t=0
			if (retriever)
			{
				// Make a domove in the Ben data
				double	deltaDist= second._DeltaPosition.norm();
				double	deltaTime;
				if(deltaDist==0)
					deltaTime= 0;
				else
					deltaTime=(collisionPosition-second._Position.getPos ()).norm()/deltaDist;
				clamp (deltaTime, 0.0, 1.0);

				UGlobalPosition newPosition = retriever->doMove (second._Position.getGlobalPos (), second._DeltaPosition,
					(float)deltaTime, surfaceTemp, true);

				// Set the new position
				second._Position.setGlobalPos (newPosition, *retriever);

				// Position at t=0
				second._3dInitPosition = second._Position.getPos() - newSpeed * desc.ContactTime;

				// New init time
				second._InitTime = desc.ContactTime;
			}
			else
			{
				// No retriever used
				second._Position.setPos (collisionPosition);

				// Position at t=0
				second._3dInitPosition = collisionPosition - newSpeed * desc.ContactTime;

				// New init time
				second._InitTime = desc.ContactTime;
			}

			// Dirt pos
			second.dirtPos (container, &otherPrimitive, secondWorldImage);
		}
	}
}

// ***************************************************************************

void CPrimitiveWorldImage::reaction (const CCollisionSurfaceDesc&	surfaceDesc, const UGlobalPosition& globalPosition,
							   CGlobalRetriever& retriever, double /* ratio */, double dt, CMovePrimitive &primitive, CMoveContainer &container,
							   uint8 worldImage)
{
//	H_AUTO(PACS_PWI_reaction_short);

	// Reaction type
	uint32 type=primitive.getReactionTypeInternal();

	// Reaction to the collision: copy the CGlobalRetriever::CGlobalPosition
	_Position.setGlobalPos (globalPosition, retriever);

	// Relfexion or slide ?
	if ((type==UMovePrimitive::Reflexion)||(type==UMovePrimitive::Slide))
	{
		// Slide ?
		if (type==UMovePrimitive::Slide)
		{
			// Project last delta on plane of collision.
			_Speed-= surfaceDesc.ContactNormal*(surfaceDesc.ContactNormal*_Speed-NELPACS_DIST_BACK/(dt-surfaceDesc.ContactTime));
		}

		// Reflexion ?
		if (type==UMovePrimitive::Reflexion)
		{
			// Project last delta on plane of collision.
			double speedProj=surfaceDesc.ContactNormal*_Speed;
			_Speed-=surfaceDesc.ContactNormal*(speedProj+speedProj*primitive.getAttenuation()-NELPACS_DIST_BACK/(dt-surfaceDesc.ContactTime));
		}
	}
	else
	{
		// Stop ?
		if (type==UMovePrimitive::Stop)
		{
			_Speed.set (0,0,0);
		}
	}

	// Contact time
	double contactTime=surfaceDesc.ContactTime;

	// Init position
	_3dInitPosition = _Position.getPos() - _Speed * contactTime;

	// Set contactTime
	_InitTime=contactTime;

	// Dirt pos
	dirtPos (&container, &primitive, worldImage);
}

// ***************************************************************************

void CPrimitiveWorldImage::setGlobalPosition (const UGlobalPosition& pos, CMoveContainer& container, CMovePrimitive &primitive, uint8 worldImage)
{
	// Cast type
	nlassert (dynamic_cast<const CMoveContainer*>(&container));
	const CMoveContainer *cont=(const CMoveContainer*)&container;

	if (!cont->getGlobalRetriever()) return;
	// Use the global retriever ?
	nlassert (cont->getGlobalRetriever());

	// Get the pos
	_Position.setGlobalPos (pos, *cont->getGlobalRetriever());

	// Precalc some values
	_3dInitPosition = _Position.getPos ();
	_InitTime = 0;

	// Speed NULL
	_Speed=CVector::Null;

	// Dirt BB
	dirtPos (&container, &primitive, worldImage);
}

// ***************************************************************************

void CPrimitiveWorldImage::setGlobalPosition (const NLMISC::CVectorD& pos, CMoveContainer& container, CMovePrimitive &primitive, uint8 worldImage, bool keepZ /*= false*/, UGlobalPosition::TType type /* =UGlobalPosition::Unspecified*/)
{
	// Cast type
	nlassert (dynamic_cast<const CMoveContainer*>(&container));
	const CMoveContainer *cont=(const CMoveContainer*)&container;

	// Get the retriever
	CGlobalRetriever *retriever=cont->getGlobalRetriever();

	// Use a global retriever
	if (retriever)
	{
		// Get a cvector
//		CVector vect=pos;		// better with CVectorD

		// Get global position
		UGlobalPosition globalPosition=retriever->retrievePosition (pos, 1.0e10, type);

		if (keepZ)
		{
			// Set the position
			_Position.setPos (pos);

			// Set global position
			_Position.setGlobalPosKeepZ (globalPosition, *retriever);
		}
		else
		{
			// Set global position
			_Position.setGlobalPos (globalPosition, *retriever);
		}
	}
	else
	{
		// Set the position
		_Position.setPos (pos);
	}

	// Precalc some values
	_3dInitPosition = _Position.getPos ();
	_InitTime = 0;

	// Speed NULL
	_Speed=CVector::Null;

	// Dirt BB
	dirtPos (&container, &primitive, worldImage);
}

// ***************************************************************************

void CPrimitiveWorldImage::move (const NLMISC::CVectorD& speed, CMoveContainer& container, CMovePrimitive &primitive, uint8 worldImage)
{
	// New speed
	setSpeed (speed, &container, &primitive, worldImage);

	// Set initial position
	_3dInitPosition = _Position.getPos ();

	// Set initial time
	_InitTime = 0;

	// Dirt BB
	dirtPos (&container, &primitive, worldImage);
}

// ***************************************************************************


} // NLPACS
