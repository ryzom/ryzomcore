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

#include "nel/3d/ps_zone.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/ps_util.h"
#include "nel/3d/dru.h"
#include "nel/3d/particle_system.h"
#include "nel/misc/plane.h"

// tmp

#include "nel/3d/particle_system_model.h"

#include <cmath>
#include <limits>

namespace NL3D {


/*
 * Constructor
 */
CPSZone::CPSZone() : _BounceFactor(1.f), _CollisionBehaviour(bounce)
{
	NL_PS_FUNC(CPSZone_CPSZone)
}

void CPSZone::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSZone_serial)
	f.serialVersion(1);
	CPSTargetLocatedBindable::serial(f);
	f.serialEnum(_CollisionBehaviour);
	f.serial(_BounceFactor);
	if (f.isReading())
	{
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{
			// though this is not a force, this prevent parametric motion
			(*it)->addNonIntegrableForceRef();
		}
	}
}

/** Add a new type of located for this zone to apply on.
* We override this to queery the target to allocate the CollisionInfo attribute
*/
void CPSZone::attachTarget(CPSLocated *ptr)
{
	NL_PS_FUNC(CPSZone_attachTarget)
	CPSTargetLocatedBindable::attachTarget(ptr);
	ptr->queryCollisionInfo();
	ptr->addNonIntegrableForceRef();
}





/// inherit from CPSTargetLocatedBindable. Its called when one of the targets has been detroyed
void CPSZone::releaseTargetRsc(CPSLocated *target)
{
	NL_PS_FUNC(CPSZone_releaseTargetRsc)
	// tell the target that we were using collision infos and that we won't use them anymore
	target->releaseCollisionInfo();
	target->releaseNonIntegrableForceRef();
}



void CPSZone::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSZone_step)
	// for zone, the PSCollision pass and the PSToolRenderPass are processed
	switch(pass)
	{
		case PSToolRender:
			show();
			break;
		default: break;
	}
}


// build a basis with K = the normal of the plane
CMatrix CPSZonePlane::buildBasis(uint32 index) const
{
	NL_PS_FUNC(CPSZonePlane_buildBasis)
	CMatrix m;
	m.setPos(_Owner->getPos()[index]);
	CPSUtil::buildSchmidtBasis(_Normal[index], m);
	return m;
}


/// this compute a new speed vector, so that the located will reach the correct position at the next speed integration
/// this create the illusion of collision
/*void CPSZone::bounce(uint32 locatedIndex, const CVector &bouncePoint, const CVector &surfNormal, float elasticity, TAnimationTime ellapsedTime)
{
	CVector &speed = _Owner->getSpeed()[locatedIndex];
	const CVector &pos   = _Owner->getPos()[locatedIndex];
	CVector &bounceVect = elasticity  * (speed - 2.0f * (speed * surfNormal) * surfNormal); // speed vector after collision
	// now check where the located will be after integration
	CVector d = bouncePoint - pos;
	TAnimationTime collideDelay = speed.norm() / d.norm();
	CVector finalPos = bouncePoint + (ellapsedTime - collideDelay) * bounceVect;
	// now, we must have pos + ellapsedTime * newSpeed = finalPos
	// newSpeed = alpha * (finalPos - pos)
	// so alpha = 1 / ellapsedTime

	speed = (1.0f / ellapsedTime) * (finalPos - pos);
}*/


void CPSZonePlane::show()
{
	NL_PS_FUNC(CPSZonePlane_show)
	const float planeSize = 2.0f;
	setupDriverModelMatrix();
	IDriver *driver = getDriver();
	uint k  = 0;
	setupDriverModelMatrix();
	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);
	for (TPSAttribVector::const_iterator it = _Owner->getPos().begin(); it != _Owner->getPos().end(); ++it, ++k)
	{
		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));
		CMatrix mat = buildBasis(k);
		CPSUtil::displayBasis(getDriver(), getLocalToWorldMatrix(), mat, 1.f, *getFontGenerator(), *getFontManager());
		setupDriverModelMatrix();
		CDRU::drawLine(*it + (planeSize  + 3) * mat.getI() + planeSize * mat.getJ(),
					   *it - (planeSize + 3) * mat.getI() + planeSize * mat.getJ(),
					   col,
					   *driver);

		CDRU::drawLine(*it + (planeSize  + 3) * mat.getI() - planeSize * mat.getJ(),
					   *it - (planeSize + 3) * mat.getI() - planeSize * mat.getJ(),
					   col,
					   *driver);

		CDRU::drawLine(*it + planeSize  * mat.getI() + (planeSize + 3) * mat.getJ(),
					   *it + planeSize * mat.getI() - (planeSize + 3) * mat.getJ(),
					   col,
					   *driver);
		CDRU::drawLine(*it - planeSize  * mat.getI() + (planeSize + 3) * mat.getJ(),
					   *it - planeSize * mat.getI() - (planeSize + 3) * mat.getJ(),
					   col,
					   *driver);
	}
}



void CPSZonePlane::resize(uint32 size)
{
	NL_PS_FUNC(CPSZonePlane_resize)
	nlassert(size < (1 << 16));
	_Normal.resize(size);
}


void CPSZonePlane::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSZonePlane_newElement)
	nlassert(_Normal.getSize() != _Normal.getMaxSize());
	_Normal.insert(CVector(0, 0, 1));
}


void CPSZonePlane::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSZonePlane_deleteElement)
	_Normal.remove(index);
}


void CPSZonePlane::computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter)
{
	NL_PS_FUNC(CPSZonePlane_computeCollisions)
	MINI_TIMER(PSStatsZonePlane)
	// for each target, we must check whether they are going through the plane
	// if so they must bounce
	TPSAttribVector::const_iterator planePosIt, planePosEnd, normalIt;
	CPSCollisionInfo ci;
	// cycle through the planes
	planePosEnd = _Owner->getPos().end();
	for (planePosIt = _Owner->getPos().begin(), normalIt = _Normal.begin(); planePosIt != planePosEnd; ++planePosIt, ++normalIt)
	{

		// we must setup the plane in the good basis
		const CMatrix &m = CPSLocated::getConversionMatrix(&target, this->_Owner);
		const float epsilon = 0.5f * PSCollideEpsilon;
		NLMISC::CPlane p;
		p.make(m.mulVector(*normalIt), m * (*planePosIt));
		// deals with each particle
		const NLMISC::CVector *itPosBefore = posBefore + firstInstanceIndex;
		const NLMISC::CVector *itPosBeforeEnd = posBefore + target.getSize();
		const NLMISC::CVector *itPosAfter = posAfter + firstInstanceIndex;
		while (itPosBefore != itPosBeforeEnd)
		{
			float posSide = p * *itPosBefore;
			float negSide = p * *itPosAfter;
			if (posSide >= - epsilon && negSide <= epsilon)
			{
				float alpha;
				if (fabsf(posSide - negSide) > std::numeric_limits<float>::min())
				{
					alpha = posSide / (posSide - negSide);
				}
				else
				{
					alpha = 0.f;
				}
				CVector startEnd = alpha * (*itPosAfter - *itPosBefore);
				ci.Dist = startEnd.norm();
				// we translate the particle from an epsilon so that it won't get hooked to the plane
				ci.NewPos = *itPosBefore  + startEnd + PSCollideEpsilon * p.getNormal();
				const CVector &speed = target.getSpeed()[(uint32)(itPosBefore - posBefore)];
				ci.NewSpeed = _BounceFactor * (speed - 2.0f * (speed * p.getNormal()) * p.getNormal());
				ci.CollisionZone = this;
				CPSLocated::_Collisions[itPosBefore - posBefore].update(ci);
			}
			++ itPosBefore;
			++ itPosAfter;
		}
	}

}

void CPSZonePlane::setMatrix(uint32 index, const CMatrix &m)
{
	NL_PS_FUNC(CPSZonePlane_setMatrix)
	nlassert(index < _Normal.getSize());
	_Normal[index] = m.getK();
	_Owner->getPos()[index] = m.getPos();
}

CMatrix CPSZonePlane::getMatrix(uint32 index) const
{
	NL_PS_FUNC(CPSZonePlane_getMatrix)
	return buildBasis(index);
}


CVector CPSZonePlane::getNormal(uint32 index)
{
	NL_PS_FUNC(CPSZonePlane_getNormal)
	return _Normal[index];
}
void CPSZonePlane::setNormal(uint32 index, CVector n)
{
	NL_PS_FUNC(CPSZonePlane_setNormal)
	_Normal[index] = n;
}




void CPSZonePlane::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSZonePlane_serial)
	f.serialVersion(1);
	CPSZone::serial(f);
	f.serial(_Normal);
}



///////////////////////////
// sphere implementation //
///////////////////////////

void CPSZoneSphere::computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter)
{
	NL_PS_FUNC(CPSZoneSphere_computeCollisions)
	MINI_TIMER(PSStatsZoneSphere)
	// for each target, we must check whether they are going through the plane
	// if so they must bounce
	TPSAttribRadiusPair::const_iterator radiusIt = _Radius.begin();
	TPSAttribVector::const_iterator spherePosIt, spherePosEnd;
	CPSCollisionInfo ci;
	float rOut, rIn;
	// cycle through the spheres
	spherePosEnd = _Owner->getPos().end();
	for (spherePosIt = _Owner->getPos().begin(), radiusIt = _Radius.begin(); spherePosIt != spherePosEnd; ++spherePosIt, ++radiusIt)
	{
		// we must setup the sphere in the good basis
		const CMatrix &m = CPSLocated::getConversionMatrix(&target, this->_Owner);
		CVector center = m * *spherePosIt;
		// deals with each particle
		const NLMISC::CVector *itPosBefore = posBefore + firstInstanceIndex;
		const NLMISC::CVector *itPosBeforeEnd = posBefore + target.getSize();
		const NLMISC::CVector *itPosAfter = posAfter + firstInstanceIndex;
		while (itPosBefore != itPosBeforeEnd)
		{
			// check whether the located is going through the sphere
			// we don't use raytracing for now because it is too slow ...
			rOut = (*itPosBefore - center) * (*itPosBefore - center);
			// initial position outside the sphere ?
			if (rOut > radiusIt->R2)
			{
				rIn = (*itPosAfter - center) * (*itPosAfter - center);
				const CVector &pos = *itPosBefore;
				const CVector &dest = *itPosAfter;
				const CVector D = dest - pos;
				// final position inside the sphere ?
				if ( rIn <= radiusIt->R2)
				{
					// discriminant of the intersection equation
					const float b = 2.f * (pos * D - D * center), a = D * D
								, c = (pos * pos) + (center * center) - 2.f * (pos * center) - radiusIt->R2;
					float d = b * b - 4 * a * c;
					if (d > 0.f)
					{
						d = sqrtf(d);
						// roots of the equation, we take the smallest
						const float r1 = .5f * (-b + 2.f * d) * a,
									r2 = .5f * (-b - 2.f * d) * a;
						const float  r = std::min(r1, r2);
						// collision point
						const CVector C = pos  + r * D;
						const float alpha = ((C - pos) * D) * a;
						const CVector startEnd = alpha * (dest - pos);
						CVector normal = C - center;
						normal = normal * (1.f / radiusIt->R);
						ci.Dist = startEnd.norm();
						// we translate the particle from an epsilon so that it won't get hooked to the sphere
						ci.NewPos = pos  + startEnd + PSCollideEpsilon * normal;
						const CVector &speed = target.getSpeed()[(uint32)(itPosBefore - posBefore)];
						ci.NewSpeed = _BounceFactor * (speed - 2.0f * (speed * normal) * normal);
						ci.CollisionZone = this;
						CPSLocated::_Collisions[itPosBefore - posBefore].update(ci);
					}
				}
			}
			++ itPosBefore;
			++ itPosAfter;
		}
	}

}



void CPSZoneSphere::show()
{
	NL_PS_FUNC(CPSZoneSphere_show)

	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);


	TPSAttribRadiusPair::const_iterator radiusIt = _Radius.begin();
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt = _Owner->getPos().end();
	setupDriverModelMatrix();
	for (uint k = 0; posIt != endPosIt; ++posIt, ++radiusIt, ++k)
	{
		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));
		CPSUtil::displaySphere(*getDriver(), radiusIt->R, *posIt, 5, col);
	}
}

void CPSZoneSphere::setMatrix(uint32 index, const CMatrix &m)
{
	NL_PS_FUNC(CPSZoneSphere_setMatrix)
	nlassert(index < _Radius.getSize());

	// compute new pos
	_Owner->getPos()[index] = m.getPos();

}


CMatrix CPSZoneSphere::getMatrix(uint32 index) const
{
	NL_PS_FUNC(CPSZoneSphere_getMatrix)
	nlassert(index < _Radius.getSize());
	CMatrix m;
	m.identity();
	m.translate(_Owner->getPos()[index]);
	return m;
}

void CPSZoneSphere::setScale(uint32 k, float scale)
{
	NL_PS_FUNC(CPSZoneSphere_setScale)
	_Radius[k].R = scale;
	_Radius[k].R2 = scale * scale;
}
CVector CPSZoneSphere::getScale(uint32 k) const
{
	NL_PS_FUNC(CPSZoneSphere_getScale)
	return CVector(_Radius[k].R, _Radius[k].R, _Radius[k].R);
}


void CPSZoneSphere::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSZoneSphere_serial)
	f.serialVersion(1);
	CPSZone::serial(f);
	f.serial(_Radius);
}

void CPSZoneSphere::resize(uint32 size)
{
	NL_PS_FUNC(CPSZoneSphere_resize)
	nlassert(size < (1 << 16));
	_Radius.resize(size);
}

void CPSZoneSphere::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSZoneSphere_newElement)
	CRadiusPair rp;
	rp.R = rp.R2 = 1.f;
	nlassert(_Radius.getSize() != _Radius.getMaxSize());
	_Radius.insert(rp);
}

void CPSZoneSphere::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSZoneSphere_deleteElement)
	_Radius.remove(index);
}


////////////////////////////////
// CPSZoneDisc implementation //
////////////////////////////////
void CPSZoneDisc::computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter)
{
	NL_PS_FUNC(CPSZoneDisc_computeCollisions)
	MINI_TIMER(PSStatsZoneDisc)
	// for each target, we must check whether they are going through the disc
	// if so they must bounce
	TPSAttribVector::const_iterator discPosIt, discPosEnd, normalIt;
	TPSAttribRadiusPair::const_iterator radiusIt;
	CPSCollisionInfo ci;
	// the square of radius at the hit point
	float hitRadius2;
	// alpha is the ratio that gives the percent of endPos - startPos that hit the disc
	CVector center;
	// cycle through the disc
	discPosEnd = _Owner->getPos().end();
	for (discPosIt = _Owner->getPos().begin(), radiusIt = _Radius.begin(), normalIt = _Normal.begin(); discPosIt != discPosEnd; ++discPosIt, ++normalIt, ++radiusIt)
	{
		// we must setup the disc in the good basis
		const CMatrix &m = CPSLocated::getConversionMatrix(&target, this->_Owner);
		NLMISC::CPlane p;
		center = m * (*discPosIt);
		p.make(m.mulVector(*normalIt), center);

		// deals with each particle
		const float epsilon = 0.5f * PSCollideEpsilon;

		// deals with each particle
		const NLMISC::CVector *itPosBefore = posBefore + firstInstanceIndex;
		const NLMISC::CVector *itPosBeforeEnd = posBefore + target.getSize();
		const NLMISC::CVector *itPosAfter = posAfter + firstInstanceIndex;
		while (itPosBefore != itPosBeforeEnd)
		{
			float posSide = p * *itPosBefore;
			float negSide = p * *itPosAfter;
			if (posSide >= - epsilon && negSide <= epsilon)
			{
				float alpha;
				if (fabsf(posSide - negSide) > std::numeric_limits<float>::min())
				{
					alpha = posSide / (posSide - negSide);
				}
				else
				{
					alpha = 0.f;
				}
				CVector startEnd = alpha * (*itPosAfter - *itPosBefore);
				ci.Dist = startEnd.norm();
				// we translate the particle from an epsilon so that it won't get hooked to the disc
				ci.NewPos = *itPosBefore  + startEnd + PSCollideEpsilon * p.getNormal();
				// now, check the collision pos against radius
				hitRadius2 = (ci.NewPos - center) * (ci.NewPos - center);
				if (hitRadius2 < radiusIt->R2) // check collision against disc
				{
					const CVector &speed = target.getSpeed()[(uint32)(itPosBefore - posBefore)];
					ci.NewSpeed = _BounceFactor * (speed - 2.0f * (speed * p.getNormal()) * p.getNormal());
					ci.CollisionZone = this;
					CPSLocated::_Collisions[itPosBefore - posBefore].update(ci);
				}

			}
			++ itPosBefore;
			++ itPosAfter;
		}
	}
}

void CPSZoneDisc::show()
{
	NL_PS_FUNC(CPSZoneDisc_show)
	TPSAttribRadiusPair::const_iterator radiusIt = _Radius.begin();
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt = _Owner->getPos().end()
									, normalIt = _Normal.begin();
	setupDriverModelMatrix();
	CMatrix mat;



	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);



	for (uint k = 0; posIt != endPosIt; ++posIt, ++radiusIt, ++normalIt, ++k)
	{
		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));
		CPSUtil::buildSchmidtBasis(*normalIt, mat);
		CPSUtil::displayDisc(*getDriver(), radiusIt->R, *posIt, mat, 32, col);

		mat.setPos(*posIt);
		CPSUtil::displayBasis(getDriver() ,getLocalToWorldMatrix(), mat, 1.f, *getFontGenerator(), *getFontManager());
		setupDriverModelMatrix();
	}
}






void CPSZoneDisc::setMatrix(uint32 index, const CMatrix &m)
{
	NL_PS_FUNC(CPSZoneDisc_setMatrix)
	nlassert(index < _Radius.getSize());
	// compute new pos
	_Owner->getPos()[index] = m.getPos();
	// compute new normal
	_Normal[index] = m.getK();
}

CMatrix CPSZoneDisc::getMatrix(uint32 index) const
{
	NL_PS_FUNC(CPSZoneDisc_getMatrix)
	CMatrix m, b;
	m.translate(_Owner->getPos()[index]);
	CPSUtil::buildSchmidtBasis(_Normal[index], b);
	m = m * b;
	return m;
}

CVector CPSZoneDisc::getNormal(uint32 index)
{
	NL_PS_FUNC(CPSZoneDisc_getNormal)
	return _Normal[index];
}
void CPSZoneDisc::setNormal(uint32 index, CVector n)
{
	NL_PS_FUNC(CPSZoneDisc_setNormal)
	_Normal[index] = n;
}

void CPSZoneDisc::setScale(uint32 k, float scale)
{
	NL_PS_FUNC(CPSZoneDisc_setScale)
	_Radius[k].R = scale;
	_Radius[k].R2 = scale * scale;
}

CVector CPSZoneDisc::getScale(uint32 k) const
{
	NL_PS_FUNC(CPSZoneDisc_getScale)
	return CVector(_Radius[k].R, _Radius[k].R, _Radius[k].R);
}


void CPSZoneDisc::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSZoneDisc_serial)
	f.serialVersion(1);
	CPSZone::serial(f);
	f.serial(_Normal);
	f.serial(_Radius);
}

void CPSZoneDisc::resize(uint32 size)
{
	NL_PS_FUNC(CPSZoneDisc_resize)
	nlassert(size < (1 << 16));
	_Radius.resize(size);
	_Normal.resize(size);
}

void CPSZoneDisc::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSZoneDisc_newElement)
	CRadiusPair rp;
	rp.R = rp.R2 = 1.f;
	nlassert(_Radius.getSize() != _Radius.getMaxSize());
	_Radius.insert(rp);
	_Normal.insert(CVector::K);
}

void CPSZoneDisc::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSZoneDisc_deleteElement)
	_Radius.remove(index);
	_Normal.remove(index);
}


////////////////////////////////////
// CPSZoneCylinder implementation //
////////////////////////////////////


/*
void CPSZoneCylinder::performMotion(TAnimationTime ellapsedTime)
{
	TPSAttribVector::const_iterator dimIt = _Dim.begin();
	CPSAttrib<CPlaneBasis>::const_iterator basisIt = _Basis.begin();
	TPSAttribVector::const_iterator cylinderPosIt, cylinderPosEnd, targetPosIt, targetPosEnd;
	CVector dest;
	CPSCollisionInfo ci;
	CVector startEnd;
	uint32 k;
	const TPSAttribVector *speedAttr;



	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{

		speedAttr = &((*it)->getSpeed());


		// cycle through the cylinders

		cylinderPosEnd = _Owner->getPos().end();
		for (cylinderPosIt = _Owner->getPos().begin(); cylinderPosIt != cylinderPosEnd
				; ++cylinderPosIt, ++dimIt, ++basisIt)
		{

			// we must setup the cylinder in the good basis

			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner);



			// compute the new center pos
			CVector center = m * *cylinderPosIt;

			// compute a basis for the cylinder
			CVector I = m.mulVector(basisIt->X);
			CVector J = m.mulVector(basisIt->Y);
			CVector K = m.mulVector(basisIt->X ^ basisIt->Y);


			// the pos projected (and scale) over the plane basis of the cylinder, the pos minus the center
			CVector projectedPos, tPos;

			// the same, but with the final position
			CVector destProjectedPos, destTPos;


			// deals with each particle
			targetPosEnd = (*it)->getPos().end();
			for (targetPosIt = (*it)->getPos().begin(), k = 0; targetPosIt != targetPosEnd; ++targetPosIt, ++k)
			{
				const CVector &speed = (*speedAttr)[k];
				const CVector &pos = *targetPosIt;



				// check whether current pos was outside the cylinder


				tPos = pos - center;
				projectedPos = (1 / dimIt->x) * (I * tPos) * I + (1 / dimIt->y)  * (J * tPos) * J;

				if (!
					 (
						((tPos * K) < dimIt->z)
						&& ((tPos * K) > -dimIt->z)
						&& (projectedPos * projectedPos < 1.f)
					 )
				   )
				{
					dest = pos + ellapsedTime *  speed;
					destTPos = dest - center;
					destProjectedPos = (1.f / dimIt->x) * (I * tPos) * I + (1.f / dimIt->y)  * (J * tPos) * J;

					// test whether the new position is inside the cylinder


					if (!
						 (
							((destTPos * K) < dimIt->z)
							&& ((destTPos * K) > -dimIt->z)
							&& (destProjectedPos * destProjectedPos < 1.f)
						 )
					   )
					{
						// now, detect the closest hit point (the smallest alpha, with alpha, the percent of the move vector
						// to reach the hit point)

						const float epsilon = 10E-6f;

						float alphaTop, alphaBottom, alphaCyl;

						const float denum = (dest - pos) * K;

						// top plane

						if (fabs(denum) < epsilon)
						{
							alphaTop = (dimIt->z - (tPos * K)) / denum;
							if (alphaTop < 0.f) alphaTop = 1.f;
						}
						else
						{
							alphaTop = 1.f;
						}

						// bottom plane

						if (fabs(denum) < epsilon)
						{
							alphaBottom = (- dimIt->z - (tPos * K)) / denum;
							if (alphaBottom < 0.f) alphaBottom = 1.f;
						}
						else
						{
							alphaBottom = 1.f;
						}

						// cylinder

						//expressed the src and dest positions in the cylinder basis

						const float ox = tPos * I, oy = tPos * J, dx = (destTPos - tPos) * I, dy = (destTPos - tPos) * J;

						// coefficients of the equation : a * alpha ^ 2 + b * alpha + c = 0
						const float a = (dx * dx) / (dimIt->x * dimIt->x)
								  + (dy * dy) / (dimIt->y * dimIt->y);
						const float b = 2.f * (ox * dx) / (dimIt->x * dimIt->x)
								  + (oy * dy) / (dimIt->y * dimIt->y);
						const float c = ox * ox + oy * oy - 1;

						// discriminant
						const float delta = b * b - 4.f * a * c;

						if (delta < epsilon)
						{
							alphaCyl = 1.f;
						}
						else
						{
							const float deltaRoot = sqrtf(delta);
							const float r1 = (- b - deltaRoot) / (2.f / a);
							const float r2 = (- b - deltaRoot) / (2.f / a);

							if (r1 < 0.f) alphaCyl = r2;
								else if (r2 < 0.f) alphaCyl = r1;
									else alphaCyl = r1 < r2 ? r1 : r2;
						}


						// now, choose the minimum positive dist

						if (alphaTop < alphaBottom && alphaTop < alphaCyl)
						{
							// collision with the top plane
							CVector startEnd = alphaTop * (dest - pos);
							ci.newPos = pos + startEnd + PSCollideEpsilon * K;
							ci.dist = startEnd.norm();
							ci.newSpeed = (-2.f * (speed * K)) * K + speed;
							ci.collisionZone = this;

							(*it)->collisionUpdate(ci, k);
						}
						else
							if (alphaBottom < alphaCyl)
							{
								// collision with the bottom plane
								CVector startEnd = alphaBottom * (dest - pos);
								ci.newPos = pos + startEnd - PSCollideEpsilon * K;
								ci.dist = startEnd.norm();
								ci.newSpeed = (-2.f * (speed * K)) * K + speed;
								ci.collisionZone = this;


								(*it)->collisionUpdate(ci, k);
							}
							else
							{
								// collision with the cylinder

								CVector startEnd = alphaCyl * (dest - pos);

								// normal at the hit point. It is the gradient of the implicit equation x^2 / a^2 + y^2 / b^2 - R^ 2=  0
								// so we got unormalized n =  (2 x / a ^ 2, 2 y / b ^ 2, 0) in the basis of the cylinder
								// As we'll normalize it, we don't need  the 2 factor

								float px = ox + alphaCyl * dx;
								float py = oy + alphaCyl * dy;

								CVector normal = px / (dimIt->x * dimIt->x) * I + py / (dimIt->y * dimIt->y) * J;
								normal.normalize();

								ci.newPos = pos + startEnd - PSCollideEpsilon * normal;
								ci.dist = startEnd.norm();
								ci.newSpeed = (-2.f * (speed * normal)) * normal + speed;
								ci.collisionZone = this;

								(*it)->collisionUpdate(ci, k);
							}

					}
				}
			}
		}
	}
}
*/


void CPSZoneCylinder::computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter)
{
	NL_PS_FUNC(CPSZoneCylinder_computeCollisions)
	MINI_TIMER(PSStatsZoneCylinder)
	TPSAttribVector::const_iterator dimIt;
	CPSAttrib<CPlaneBasis>::const_iterator basisIt;
	TPSAttribVector::const_iterator cylinderPosIt, cylinderPosEnd;
	CPSCollisionInfo ci;
	// cycle through the cylinders
	cylinderPosEnd = _Owner->getPos().end();
	for (cylinderPosIt = _Owner->getPos().begin(), basisIt = _Basis.begin(),  dimIt = _Dim.begin(); cylinderPosIt != cylinderPosEnd; ++cylinderPosIt, ++dimIt, ++basisIt)
	{
		// we must setup the cylinder in the good basis
		const CMatrix &m = CPSLocated::getConversionMatrix(&target, this->_Owner);
		// compute the new center pos
		CVector center = m * *cylinderPosIt;
		// compute a basis for the cylinder
		CVector I = m.mulVector(basisIt->X);
		CVector J = m.mulVector(basisIt->Y);
		CVector K = m.mulVector(basisIt->X ^ basisIt->Y);
		// the pos projected (and scale) over the plane basis of the cylinder, the pos minus the center
		CVector projectedPos, tPos;
		// the same, but with the final position
		CVector destProjectedPos, destTPos;
		// deals with each particle
		// deals with each particle
		const NLMISC::CVector *itPosBefore = posBefore + firstInstanceIndex;
		const NLMISC::CVector *itPosBeforeEnd = posBefore + target.getSize();
		const NLMISC::CVector *itPosAfter = posAfter + firstInstanceIndex;
		while (itPosBefore != itPosBeforeEnd)
		{
			const CVector &pos = *itPosBefore;
			// check whether current pos was outside the cylinder
			tPos = pos - center;
			projectedPos = (1 / dimIt->x) * (I * tPos) * I + (1 / dimIt->y)  * (J * tPos) * J;
			if (!
				 (
					((tPos * K) < dimIt->z)
					&& ((tPos * K) > -dimIt->z)
					&& (projectedPos * projectedPos < 1.f)
				 )
			   )
			{
				const CVector &dest = *itPosAfter;
				destTPos = dest - center;
				destProjectedPos = (1.f / dimIt->x) * (I * destTPos) * I + (1.f / dimIt->y)  * (J * destTPos) * J;
				// test whether the new position is inside the cylinder
				if (
					((destTPos * K) < dimIt->z)
					&& ((destTPos * K) > -dimIt->z)
					&& (destProjectedPos * destProjectedPos < 1.f)
				   )
				{
					// now, detect the closest hit point (the smallest alpha, with alpha, the percent of the move vector
					// to reach the hit point)
					const float epsilon = 10E-3f;
					float alphaTop, alphaBottom, alphaCyl;
					const float denum = (dest - pos) * K;
					// top plane
					if (fabs(denum) < epsilon)
					{
						alphaTop = (dimIt->z - (tPos * K)) / denum;
						if (alphaTop < 0.f) alphaTop = 1.f;
					}
					else
					{
						alphaTop = 1.f;
					}
					// bottom plane
					if (fabs(denum) < epsilon)
					{
						alphaBottom = (- dimIt->z - (tPos * K)) / denum;
						if (alphaBottom < 0.f) alphaBottom = 1.f;
					}
					else
					{
						alphaBottom = 1.f;
					}
					// cylinder
					//expressed the src and dest positions in the cylinder basis
					const float ox = tPos * I, oy = tPos * J, dx = (destTPos - tPos) * I, dy = (destTPos - tPos) * J;
					// coefficients of the equation : a * alpha ^ 2 + b * alpha + c = 0
					const float a = (dx * dx) / (dimIt->x * dimIt->x)
							  + (dy * dy) / (dimIt->y * dimIt->y);
					const float b = 2.f * ((ox * dx) / (dimIt->x * dimIt->x)
							  + (oy * dy) / (dimIt->y * dimIt->y));
					const float c = (ox * ox) / (dimIt->x * dimIt->x) + (oy * oy) / (dimIt->y * dimIt->y)  - 1;
					// discriminant
					const float delta = b * b - 4.f * a * c;

					if (delta < epsilon)
					{
						alphaCyl = 1.f;
					}
					else
					{
						const float deltaRoot = sqrtf(delta);
						const float r1 = (- b + 2.f * deltaRoot) / (2.f * a);
						const float r2 = (- b - 2.f * deltaRoot) / (2.f * a);

						if (r1 < 0.f) alphaCyl = r2;
							else if (r2 < 0.f) alphaCyl = r1;
								else alphaCyl = r1 < r2 ? r1 : r2;

								if (alphaCyl < 0.f) alphaCyl = 1.f;
					}
					const CVector &speed = target.getSpeed()[(uint32)(itPosBefore - posBefore)];
					// now, choose the minimum positive dist
					if (alphaTop < alphaBottom && alphaTop < alphaCyl)
					{
						// collision with the top plane
						CVector startEnd = alphaTop * (dest - pos);
						ci.NewPos = pos + startEnd + PSCollideEpsilon * K;
						ci.Dist = startEnd.norm();
						ci.NewSpeed = (-2.f * (speed * K)) * K + speed;
						ci.CollisionZone = this;
						CPSLocated::_Collisions[itPosBefore - posBefore].update(ci);
					}
					else
						if (alphaBottom < alphaCyl)
						{
							// collision with the bottom plane
							CVector startEnd = alphaBottom * (dest - pos);
							ci.NewPos = pos + startEnd - PSCollideEpsilon * K;
							ci.Dist = startEnd.norm();
							ci.NewSpeed = (-2.f * (speed * K)) * K + speed;
							ci.CollisionZone = this;
							CPSLocated::_Collisions[itPosBefore - posBefore].update(ci);
						}
						else
						{
							// collision with the cylinder
							CVector startEnd = alphaCyl * (dest - pos);
							// normal at the hit point. It is the gradient of the implicit equation x^2 / a^2 + y^2 / b^2 - R^ 2=  0
							// so we got unormalized n =  (2 x / a ^ 2, 2 y / b ^ 2, 0) in the basis of the cylinder
							// As we'll normalize it, we don't need  the 2 factor
							float px = ox + alphaCyl * dx;
							float py = oy + alphaCyl * dy;
							CVector normal = px / (dimIt->x * dimIt->x) * I + py / (dimIt->y * dimIt->y) * J;
							normal.normalize();
							ci.NewPos = pos + startEnd + PSCollideEpsilon * normal;
							ci.Dist = startEnd.norm();
							ci.NewSpeed = (-2.f * (speed * normal)) * normal + speed;
							ci.CollisionZone = this;
							CPSLocated::_Collisions[itPosBefore - posBefore].update(ci);
						}

				}
			}
			++ itPosBefore;
			++ itPosAfter;
		}
	}
}

void CPSZoneCylinder::show()
{
	NL_PS_FUNC(CPSZoneCylinder_show)
	TPSAttribVector::const_iterator dimIt = _Dim.begin()
									,posIt = _Owner->getPos().begin()
									, endPosIt = _Owner->getPos().end();

	CPSAttrib<CPlaneBasis>::const_iterator basisIt = _Basis.begin();

	setupDriverModelMatrix();
	CMatrix mat;



	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);


	for (uint32 k = 0; posIt != endPosIt; ++posIt, ++dimIt, ++basisIt, ++k)
	{
		mat.setRot(basisIt->X, basisIt->Y, basisIt->X ^ basisIt->Y);
		mat.setPos(CVector::Null);

		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));


		CPSUtil::displayCylinder(*getDriver(), *posIt, mat, *dimIt, 32, col);

		mat.setPos(*posIt);
		CPSUtil::displayBasis(getDriver() ,getLocalToWorldMatrix(), mat, 1.f, *getFontGenerator(), *getFontManager());
		setupDriverModelMatrix();

	}
}



void CPSZoneCylinder::setMatrix(uint32 index, const CMatrix &m)
{
	NL_PS_FUNC(CPSZoneCylinder_setMatrix)
	// transform the basis
	_Basis[index].X = m.getI();
	_Basis[index].Y = m.getJ();

	// compute new pos
	_Owner->getPos()[index] = m.getPos();


}



CMatrix CPSZoneCylinder::getMatrix(uint32 index) const
{
	NL_PS_FUNC(CPSZoneCylinder_getMatrix)
	CMatrix m;
	m.setRot(_Basis[index].X, _Basis[index].Y, _Basis[index].X ^_Basis[index].Y);
	m.setPos(_Owner->getPos()[index]);
	return m;
}


void CPSZoneCylinder::setScale(uint32 k, float scale)
{
	NL_PS_FUNC(CPSZoneCylinder_setScale)
	_Dim[k] = CVector(scale, scale, scale);
}

CVector CPSZoneCylinder::getScale(uint32 k) const
{
	NL_PS_FUNC(CPSZoneCylinder_getScale)
	return _Dim[k];
}

void CPSZoneCylinder::setScale(uint32 index, const CVector &s)
{
	NL_PS_FUNC(CPSZoneCylinder_setScale)
	_Dim[index] = s;
}


void CPSZoneCylinder::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSZoneCylinder_serial)
	f.serialVersion(1);
	CPSZone::serial(f);
	f.serial(_Basis);
	f.serial(_Dim);
}



void CPSZoneCylinder::resize(uint32 size)
{
	NL_PS_FUNC(CPSZoneCylinder_resize)
	nlassert(size < (1 << 16));
	_Basis.resize(size);
	_Dim.resize(size);
}

void CPSZoneCylinder::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSZoneCylinder_newElement)
	_Basis.insert(CPlaneBasis(CVector::K));
	_Dim.insert(CVector(1, 1, 1));
}

void CPSZoneCylinder::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSZoneCylinder_deleteElement)
	_Basis.remove(index);
	_Dim.remove(index);
}


//////////////////////////////////////////////
//	implementation of CPSZoneRectangle      //
//////////////////////////////////////////////

void CPSZoneRectangle::computeCollisions(CPSLocated &target, uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter)
{
	NL_PS_FUNC(CPSZoneRectangle_computeCollisions)
	MINI_TIMER(PSStatsZoneRectangle)
	// for each target, we must check whether they are going through the rectangle
	// if so they must bounce
	TPSAttribVector::const_iterator rectanglePosIt, rectanglePosEnd;
	CPSAttrib<CPlaneBasis>::const_iterator basisIt;
	TPSAttribFloat::const_iterator widthIt, heightIt;
	CPSCollisionInfo ci;
	// alpha is the ratio that gives the percent of endPos - startPos that hit the rectangle
	basisIt = _Basis.begin();
	heightIt = _Height.begin();
	widthIt = _Width.begin();
	rectanglePosEnd = _Owner->getPos().end();
	for (rectanglePosIt = _Owner->getPos().begin(); rectanglePosIt != rectanglePosEnd; ++rectanglePosIt, ++basisIt, ++widthIt, ++heightIt)
	{
		// we must setup the rectangle in the good basis
		const CMatrix &m = CPSLocated::getConversionMatrix(&target, this->_Owner);
		NLMISC::CPlane p;
		CVector center = m * (*rectanglePosIt);
		const CVector X = m.mulVector(basisIt->X);
		const CVector Y = m.mulVector(basisIt->Y);
		p.make(X ^ Y, center);
		// deals with each particle
		const float epsilon = 0.5f * PSCollideEpsilon;
		const NLMISC::CVector *itPosBefore = posBefore + firstInstanceIndex;
		const NLMISC::CVector *itPosBeforeEnd = posBefore + target.getSize();
		const NLMISC::CVector *itPosAfter = posAfter + firstInstanceIndex;
		while (itPosBefore != itPosBeforeEnd)
		{
			float posSide = p * *itPosBefore;
			float negSide = p * *itPosAfter;
			if (posSide >= - epsilon && negSide <= epsilon)
			{
				float alpha;
				if (fabsf(posSide - negSide) > std::numeric_limits<float>::min())
				{
					alpha = posSide / (posSide - negSide);
				}
				else
				{
					alpha = 0.f;
				}
				CVector startEnd = alpha * (*itPosAfter - *itPosBefore);
				ci.Dist = startEnd.norm();
				// we translate the particle from an epsilon so that it won't get hooked to the rectangle
				ci.NewPos = *itPosBefore + startEnd;
				// tmp
				if ( fabs( (ci.NewPos - center) * X ) < *widthIt && fabs( (ci.NewPos - center) * Y ) < *heightIt) // check collision against rectangle
				{
					ci.NewPos += PSCollideEpsilon * p.getNormal();
					const CVector &speed = target.getSpeed()[(uint32)(itPosBefore - posBefore)];
					ci.NewSpeed = _BounceFactor * (speed - 2.0f * (speed * p.getNormal()) * p.getNormal());
					ci.CollisionZone = this;
					CPSLocated::_Collisions[itPosBefore - posBefore].update(ci);
				}
			}
			++ itPosBefore;
			++ itPosAfter;
		}
	}

}


void CPSZoneRectangle::show()
{
	NL_PS_FUNC(CPSZoneRectangle_show)
	nlassert(_Owner);
	const uint size = _Owner->getSize();
	if (!size) return;
	setupDriverModelMatrix();
	CMatrix mat;

	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);

	for (uint k = 0; k < size; ++k)
	{
		const CVector &I = _Basis[k].X;
		const CVector &J = _Basis[k].Y;
		mat.setRot(I, J , I ^J);
		mat.setPos(_Owner->getPos()[k]);
		CPSUtil::displayBasis(getDriver(), getLocalToWorldMatrix(), mat, 1.f, *getFontGenerator(), *getFontManager());
		setupDriverModelMatrix();

		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));



		const CVector &pos = _Owner->getPos()[k];
		CPSUtil::display3DQuad(*getDriver(), pos + I * _Width[k] + J * _Height[k]
										   , pos + I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] + J * _Height[k], col);
	}
}

void CPSZoneRectangle::setMatrix(uint32 index, const CMatrix &m)
{
	NL_PS_FUNC(CPSZoneRectangle_setMatrix)
	nlassert(_Owner);

	_Owner->getPos()[index] = m.getPos();
	_Basis[index].X = m.getI();
	_Basis[index].Y = m.getJ();
}


CMatrix CPSZoneRectangle::getMatrix(uint32 index) const
{
	NL_PS_FUNC(CPSZoneRectangle_getMatrix)
	nlassert(_Owner);
	CMatrix m;
	m.setRot(_Basis[index].X, _Basis[index].Y, _Basis[index].X ^ _Basis[index].Y);
	m.setPos(_Owner->getPos()[index]);
	return m;
}

void CPSZoneRectangle::setScale(uint32 index, float scale)
{
	NL_PS_FUNC(CPSZoneRectangle_setScale)
	_Width[index] = scale;
	_Height[index] = scale;
}
void CPSZoneRectangle::setScale(uint32 index, const CVector &s)
{
	NL_PS_FUNC(CPSZoneRectangle_setScale)
	_Width[index] = s.x;
	_Height[index] = s.y;
}
CVector CPSZoneRectangle::getScale(uint32 index) const
{
	NL_PS_FUNC(CPSZoneRectangle_getScale)
	return CVector(_Width[index], _Height[index], 1.f);
}



void CPSZoneRectangle::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSZoneRectangle_IStream )
	f.serialVersion(1);
	CPSZone::serial(f);
	f.serial(_Basis);
	f.serial(_Width);
	f.serial(_Height);
}


void CPSZoneRectangle::resize(uint32 size)
{
	NL_PS_FUNC(CPSZoneRectangle_resize)
	nlassert(size < (1 << 16));
	_Basis.resize(size);
	_Width.resize(size);
	_Height.resize(size);
}

void CPSZoneRectangle::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSZoneRectangle_newElement)
	_Basis.insert(CPlaneBasis(CVector::K));
	_Width.insert(1.f);
	_Height.insert(1.f);
}

void CPSZoneRectangle::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSZoneRectangle_deleteElement)
	_Basis.remove(index);
	_Width.remove(index);
	_Height.remove(index);
}



} // NL3D
