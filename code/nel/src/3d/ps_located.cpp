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



#include <algorithm>
#include "nel/misc/aabbox.h"
#include "nel/misc/matrix.h"
#include "nel/misc/common.h"
#include "nel/3d/ps_util.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_zone.h"
#include "nel/3d/driver.h"
#include "nel/3d/material.h"
#include "nel/3d/dru.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_emitter.h"
#include "nel/3d/ps_misc.h"

#include "nel/misc/line.h"
#include "nel/misc/system_info.h"
#include "nel/misc/common.h"

//
#include "nel/3d/particle_system_model.h"


#ifdef NL_DEBUG
	#define CHECK_PS_INTEGRITY checkIntegrity();
#else
	#define CHECK_PS_INTEGRITY
#endif

namespace NL3D {


std::vector<CPSCollisionInfo> CPSLocated::_Collisions;
CPSCollisionInfo *CPSLocated::_FirstCollision = NULL;



/// ***************************************************************************************
/**
 * Constructor
 */
CPSLocated::CPSLocated() : /*_MaxNumFaces(0),*/
						   _Size(0),
						   _MaxSize(DefaultMaxLocatedInstance),
						   _CollisionInfoNbRef(0),
						   _CollisionNextPos(NULL),
						   _InitialLife(1.f),
						   _LifeScheme(NULL),
						   _InitialMass(1.f),
						   _MassScheme(NULL),
						   _LODDegradation(false),
						   _ParametricMotion(false),
						   _TriggerOnDeath(false),
						   _LastForever(true),
						   _TriggerID(NELID("NONE")),
						   _NonIntegrableForceNbRefs(0),
						   _NumIntegrableForceWithDifferentBasis(0)
{
	NL_PS_FUNC(CPSLocated_CPSLocated)
}


// *****************************************************************************************************
const NLMISC::CMatrix &CPSLocated::getLocalToWorldMatrix() const
{
	NL_PS_FUNC(CPSLocated_getLocalToWorldMatrix)
	nlassert(_Owner);
	switch(getMatrixMode())
	{
		case PSFXWorldMatrix:				return _Owner->getSysMat();
		case PSIdentityMatrix:				return NLMISC::CMatrix::Identity;
		case PSUserMatrix:	return _Owner->getUserMatrix();
		default:
			nlassert(0);
	}
	nlassert(0);
	return NLMISC::CMatrix::Identity;
}

// *****************************************************************************************************
const NLMISC::CMatrix &CPSLocated::getWorldToLocalMatrix() const
{
	NL_PS_FUNC(CPSLocated_getWorldToLocalMatrix)
	nlassert(_Owner);
	switch(getMatrixMode())
	{
		case PSFXWorldMatrix:				return _Owner->getInvertedSysMat();
		case PSIdentityMatrix:				return NLMISC::CMatrix::Identity;
		case PSUserMatrix:	return _Owner->getInvertedUserMatrix();
		default:
			nlassert(0);
	}
	nlassert(0);
	return NLMISC::CMatrix::Identity;
}


/// ***************************************************************************************
float CPSLocated::evalMaxDuration() const
{
	NL_PS_FUNC(CPSLocated_evalMaxDuration)
	if (_LastForever) return -1.f;
	return _LifeScheme ? _LifeScheme->getMaxValue() : _InitialLife;
}


/// ***************************************************************************************
void CPSLocated::checkIntegrity() const
{
	NL_PS_FUNC(CPSLocated_checkIntegrity)
	nlassert(_InvMass.getMaxSize() == _Pos.getMaxSize());
	nlassert(_Pos.getMaxSize() == _Speed.getMaxSize());
	nlassert(_Speed.getMaxSize() == _Time.getMaxSize());
	nlassert(_Time.getMaxSize() == _TimeIncrement.getMaxSize());
	//
	nlassert(_InvMass.getSize() == _Pos.getSize());
	nlassert(_Pos.getSize() == _Speed.getSize());
	nlassert(_Speed.getSize() == _Time.getSize());
	nlassert(_Time.getSize() == _TimeIncrement.getSize());
	//
	if (hasCollisionInfos())
	{
		nlassert(_CollisionNextPos->getSize() == _Pos.getSize());
		nlassert(_CollisionNextPos->getMaxSize() == _Pos.getMaxSize());
	}
	//
}

/// ***************************************************************************************
bool CPSLocated::setLastForever()
{
	NL_PS_FUNC(CPSLocated_setLastForever)
	CHECK_PS_INTEGRITY
	_LastForever = true;
	if (_Owner && _Owner->getBypassMaxNumIntegrationSteps())
	{
		// Should test that the system is still valid.
		if (!_Owner->canFinish())
		{
			_LastForever = false;
			nlwarning("<CPSLocated::setLastForever> Can't set flag : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. Flag is not set");
			return false;
		}
	}
	CHECK_PS_INTEGRITY
	return true;
}


/// ***************************************************************************************
void CPSLocated::systemDateChanged()
{
	NL_PS_FUNC(CPSLocated_systemDateChanged)
	CHECK_PS_INTEGRITY
	for(TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->systemDateChanged();
	}
	CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
void CPSLocated::releaseRefTo(const CParticleSystemProcess *other)
{
	NL_PS_FUNC(CPSLocated_releaseRefTo)
	CHECK_PS_INTEGRITY
	// located bindables
	{
		for(TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			(*it)->releaseRefTo(other);
		}
	}
	// dtor observers
	{

		for(TDtorObserversVect::iterator it = _DtorObserversVect.begin(); it != _DtorObserversVect.end(); ++it)
		{
			if ((*it)->getOwner() == other)
			{
				CPSLocatedBindable *refMaker = *it;
				refMaker->notifyTargetRemoved(this);
				break;
			}
		}
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::releaseAllRef()
{
	NL_PS_FUNC(CPSLocated_releaseAllRef)
	CHECK_PS_INTEGRITY
	 // located bindables
	{
		for(TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			(*it)->releaseAllRef();
		}
	}

	// we must do a copy, because the subsequent call can modify this vector
	TDtorObserversVect copyVect(_DtorObserversVect.begin(), _DtorObserversVect.end());
	// call all the dtor observers
	for (TDtorObserversVect::iterator it = copyVect.begin(); it != copyVect.end(); ++it)
	{
		(*it)->notifyTargetRemoved(this);
	}
	_DtorObserversVect.clear();

	nlassert(_CollisionInfoNbRef == 0); //If this is not = 0, then someone didnt call releaseCollisionInfo
										 // If this happen, you can register with the registerDTorObserver
										 // (observer pattern)
										 // and override notifyTargetRemove to call releaseCollisionInfo
	nlassert(_IntegrableForces.size() == 0);
	nlassert(_NonIntegrableForceNbRefs == 0);
	CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
void CPSLocated::notifyMotionTypeChanged(void)
{
	NL_PS_FUNC(CPSLocated_notifyMotionTypeChanged)
	CHECK_PS_INTEGRITY
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->motionTypeChanged(_ParametricMotion);
	}
	CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
void CPSLocated::integrateSingle(float startDate, float deltaT, uint numStep,
								uint32 indexInLocated,
								NLMISC::CVector *destPos,
								uint stride) const
{
	NL_PS_FUNC(CPSLocated_integrateSingle)
	CHECK_PS_INTEGRITY
	nlassert(supportParametricMotion() && _ParametricMotion);
	if (_IntegrableForces.size() != 0)
	{
		bool accumulate = false;
		for (TForceVect::const_iterator it = _IntegrableForces.begin(); it != _IntegrableForces.end(); ++it)
		{
			nlassert((*it)->isIntegrable());
			(*it)->integrateSingle(startDate, deltaT, numStep, this, indexInLocated, destPos, accumulate, stride);
			accumulate = true;
		}
	}
	else // no forces applied, just deduce position from date, initial pos and speed
	{
			#ifdef NL_DEBUG
				NLMISC::CVector *endPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride * numStep);
			#endif
			const CPSLocated::CParametricInfo &pi = _PInfo[indexInLocated];
			destPos = FillBufUsingSubdiv(pi.Pos, pi.Date, startDate, deltaT, numStep, destPos, stride);
			if (numStep != 0)
			{
				float currDate = startDate - pi.Date;
				nlassert(currDate >= 0);
				do
				{
					#ifdef NL_DEBUG
						nlassert(destPos < endPos);
					#endif
					destPos->x = pi.Pos.x + currDate * pi.Speed.x;
					destPos->y = pi.Pos.y + currDate * pi.Speed.y;
					destPos->z = pi.Pos.z + currDate * pi.Speed.z;
					currDate += deltaT;
					destPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride);
				}
				while (--numStep);
			}
	}
	CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
void CPSLocated::performParametricMotion(TAnimationTime date)
{
	NL_PS_FUNC(CPSLocated_performParametricMotion)
	CHECK_PS_INTEGRITY
	if (!_Size) return;
	nlassert(supportParametricMotion() && _ParametricMotion);

	if (_IntegrableForces.size() != 0)
	{
		bool accumulate = false;
		for (TForceVect::iterator it = _IntegrableForces.begin(); it != _IntegrableForces.end(); ++it)
		{
			nlassert((*it)->isIntegrable());
			(*it)->integrate(date, this, 0, _Size, &_Pos[0], &_Speed[0], accumulate);
			accumulate = true;
		}
	}
	else
	{
		CPSLocated::TPSAttribParametricInfo::const_iterator it = _PInfo.begin(),
											endIt = _PInfo.end();
		TPSAttribVector::iterator posIt = _Pos.begin();
		float deltaT;
		do
		{
			deltaT = date - it->Date;
			posIt->x = it->Pos.x + deltaT * it->Speed.x;
			posIt->y = it->Pos.y + deltaT * it->Speed.y;
			posIt->z = it->Pos.z + deltaT * it->Speed.z;
			++posIt;
			++it;
		}
		while (it != endIt);
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
/// allocate parametric infos
void  CPSLocated::allocateParametricInfos(void)
{
	NL_PS_FUNC(CPSLocated_allocateParametricInfos)
	CHECK_PS_INTEGRITY
	if (_ParametricMotion) return;
	nlassert(supportParametricMotion());
	nlassert(_Owner);
	const float date = _Owner->getSystemDate();
	_PInfo.resize(_MaxSize);
	// copy back infos from current position and speeds
	TPSAttribVector::const_iterator posIt = _Pos.begin(), endPosIt = _Pos.end();
	TPSAttribVector::const_iterator speedIt = _Speed.begin();
	while (posIt != endPosIt)
	{
		_PInfo.insert( CParametricInfo(*posIt, *speedIt, date) );
		++posIt;
	}
	_ParametricMotion = true;
	notifyMotionTypeChanged();
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
/// release parametric infos
void  CPSLocated::releaseParametricInfos(void)
{
	NL_PS_FUNC(CPSLocated_releaseParametricInfos)
	CHECK_PS_INTEGRITY
	if (!_ParametricMotion) return;
	NLMISC::contReset(_PInfo);
	_ParametricMotion = false;
	notifyMotionTypeChanged();
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
/// Test whether this located support parametric motion
bool      CPSLocated::supportParametricMotion(void) const
{
	NL_PS_FUNC(CPSLocated_supportParametricMotion)
	return _NonIntegrableForceNbRefs == 0 && _NumIntegrableForceWithDifferentBasis == 0;
}

/// ***************************************************************************************
/** When set to true, this tells the system to use parametric motion. This is needed in a few case only,
  * and can only work if all the forces that apply to the system are integrable
  */
void	CPSLocated::enableParametricMotion(bool enable /*= true*/)
{
	NL_PS_FUNC(CPSLocated_enableParametricMotion)
	CHECK_PS_INTEGRITY
	nlassert(supportParametricMotion());
	if (enable)
	{
		allocateParametricInfos();
	}
	else
	{
		releaseParametricInfos();
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::setMatrixMode(TPSMatrixMode matrixMode)
{
	NL_PS_FUNC(CPSLocated_setMatrixMode)
	CHECK_PS_INTEGRITY
	if (matrixMode != getMatrixMode())
	{
		for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			(*it)->basisChanged(matrixMode);
		}

		CParticleSystemProcess::setMatrixMode(matrixMode);
		for (TForceVect::iterator fIt = _IntegrableForces.begin(); fIt != _IntegrableForces.end(); ++fIt)
		{
			integrableForceBasisChanged( (*fIt)->getOwner()->getMatrixMode() );
		}
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
/*
void CPSLocated::notifyMaxNumFacesChanged(void)
{
	CHECK_PS_INTEGRITY
	if (!_Owner) return;

	// we examine whether we have particle attached to us, and ask for the max number of faces they may want
	_MaxNumFaces  = 0;
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSParticle)
		{
			uint maxNumFaces = ((CPSParticle *) (*it))->getMaxNumFaces();
			///nlassertex(maxNumFaces < ((1 << 16) - 1), ("%s", (*it)->getClassName().c_str()));
			_MaxNumFaces += maxNumFaces;
		}
	}
	CHECK_PS_INTEGRITY
}
*/

/// ***************************************************************************************
uint CPSLocated::getNumWantedTris() const
{
	NL_PS_FUNC(CPSLocated_getNumWantedTris)
	CHECK_PS_INTEGRITY
	if (!_Owner) return 0;
	uint numWantedTris = 0;
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSParticle)
		{
			numWantedTris += NLMISC::safe_cast<CPSParticle *>(*it)->getNumWantedTris();
		}
	}
	CHECK_PS_INTEGRITY
	return numWantedTris;
}

/*
/// ***************************************************************************************
uint CPSLocated::querryMaxWantedNumFaces(void)
{
	return _MaxNumFaces;
}
*/

/// ***************************************************************************************
/// tells whether there are alive entities / particles in the system
bool CPSLocated::hasParticles(void) const
{
	NL_PS_FUNC(CPSLocated_hasParticles)
	CHECK_PS_INTEGRITY
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSParticle && (*it)->hasParticles()) return true;
	}
	CHECK_PS_INTEGRITY
	return false;
}

/// ***************************************************************************************
/// tells whether there are alive emitters
bool CPSLocated::hasEmitters(void) const
{
	NL_PS_FUNC(CPSLocated_hasEmitters)
	CHECK_PS_INTEGRITY
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSEmitter && (*it)->hasEmitters()) return true;
	}
	CHECK_PS_INTEGRITY
	return false;
}

/// ***************************************************************************************
void CPSLocated::getLODVect(NLMISC::CVector &v, float &offset, TPSMatrixMode matrixMode)
{
	NL_PS_FUNC(CPSLocated_getLODVect)
	nlassert(_Owner);
	CHECK_PS_INTEGRITY
	_Owner->getLODVect(v, offset, matrixMode);
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
float CPSLocated::getUserParam(uint numParam) const
{
	NL_PS_FUNC(CPSLocated_getUserParam)
	nlassert(_Owner);
	CHECK_PS_INTEGRITY
	return _Owner->getUserParam(numParam);
}

/// ***************************************************************************************
CScene *CPSLocated::getScene(void)
{
	NL_PS_FUNC(CPSLocated_getScene)
	nlassert(_Owner);
	CHECK_PS_INTEGRITY
	return _Owner->getScene();
}

/// ***************************************************************************************
void CPSLocated::incrementNbDrawnParticles(uint num)
{
	NL_PS_FUNC(CPSLocated_incrementNbDrawnParticles)
	CHECK_PS_INTEGRITY
	CParticleSystem::NbParticlesDrawn += num; // for benchmark purpose
}

/// ***************************************************************************************
void CPSLocated::setInitialLife(TAnimationTime lifeTime)
{
	NL_PS_FUNC(CPSLocated_setInitialLife)
	CHECK_PS_INTEGRITY
	_LastForever = false;
	_InitialLife = lifeTime;
	delete _LifeScheme;
	_LifeScheme = NULL;

	/** Reset all particles current date to 0. This is needed because we do not check
	  * if particle life is over when the date of the system has not gone beyond the life duration of particles
	  */
	for (uint k = 0; k < _Size; ++k)
	{
		_Time[k] = 0.f;
	}
	//
	if (_Owner)
	{
		_Owner->systemDurationChanged();
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::setLifeScheme(CPSAttribMaker<float> *scheme)
{
	NL_PS_FUNC(CPSLocated_setLifeScheme)
	CHECK_PS_INTEGRITY
	nlassert(scheme);
	nlassert(!scheme->hasMemory()); // scheme with memory is invalid there !!
	_LastForever = false;
	delete _LifeScheme;
	_LifeScheme = scheme;
	//
	if (_Owner)
	{
		_Owner->systemDurationChanged();
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::setInitialMass(float mass)
{
	NL_PS_FUNC(CPSLocated_setInitialMass)
	CHECK_PS_INTEGRITY
	_InitialMass = mass;
	delete _MassScheme;
	_MassScheme = NULL;
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::setMassScheme(CPSAttribMaker<float> *scheme)
{
	NL_PS_FUNC(CPSLocated_setMassScheme)
	CHECK_PS_INTEGRITY
	nlassert(scheme);
	nlassert(!scheme->hasMemory()); // scheme with memory is invalid there !!
	delete _MassScheme;
	_MassScheme = scheme;
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
/// get a matrix that helps to express located B coordinate in located A basis
const NLMISC::CMatrix &CPSLocated::getConversionMatrix(const CParticleSystem &ps, TPSMatrixMode destMode, TPSMatrixMode srcMode)
{
	NL_PS_FUNC(CPSLocated_getConversionMatrix)
	switch(destMode)
	{
		case PSFXWorldMatrix:
			switch(srcMode)
			{
				case PSFXWorldMatrix:				return NLMISC::CMatrix::Identity;
				case PSIdentityMatrix:				return ps.getInvertedSysMat();
				case PSUserMatrix:	return ps.getUserToFXMatrix();
				default:
				nlassert(0);
			}
		break;
		case PSIdentityMatrix:
			switch(srcMode)
			{
				case PSFXWorldMatrix:				return ps.getSysMat();
				case PSIdentityMatrix:				return NLMISC::CMatrix::Identity;
				case PSUserMatrix:	return ps.getUserMatrix();
				default:
				nlassert(0);
			}
		break;
		case PSUserMatrix:
			switch(srcMode)
			{
				case PSFXWorldMatrix:				return ps.getFXToUserMatrix();
				case PSIdentityMatrix:				return ps.getInvertedUserMatrix();
				case PSUserMatrix:	return NLMISC::CMatrix::Identity;
				default:
				nlassert(0);
			}
		break;
		default:
			nlassert(0);
	}
	nlassert(0);
	return NLMISC::CMatrix::Identity;
}

/// ***************************************************************************************
NLMISC::CVector CPSLocated::computeI(void) const
{
	NL_PS_FUNC(CPSLocated_computeI)
	CHECK_PS_INTEGRITY
	const NLMISC::CMatrix &sysMat = _Owner->getSysMat();
	if (getMatrixMode() == PSIdentityMatrix)
	{
		if (!sysMat.hasScalePart())
		{
			return _Owner->getInvertedViewMat().getI();
		}
		else
		{
			return sysMat.getScaleUniform() * _Owner->getInvertedViewMat().getI();
		}
	}
	else
	{
		if (!sysMat.hasScalePart())
		{
			// we must express the I vector in the system basis, so we need to multiply it by the inverted matrix of the system
			return getWorldToLocalMatrix().mulVector(_Owner->getInvertedViewMat().getI());
		}
		else
		{
			return sysMat.getScaleUniform() * getWorldToLocalMatrix().mulVector(_Owner->getInvertedViewMat().getI());
		}
	}
}

/// ***************************************************************************************
NLMISC::CVector CPSLocated::computeIWithZAxisAligned(void) const
{
	NL_PS_FUNC(CPSLocated_computeIWithZAxisAligned)
	CHECK_PS_INTEGRITY
	const NLMISC::CMatrix &sysMat = _Owner->getSysMat();
	const CVector &camI = _Owner->getInvertedViewMat().getI();
	CVector I(camI.x, camI.y, 0.f);
	I.normalize();
	if (getMatrixMode() == PSIdentityMatrix)
	{
		if (!sysMat.hasScalePart())
		{
			return I;
		}
		else
		{
			return sysMat.getScaleUniform() * I;
		}
	}
	else
	{
		if (!sysMat.hasScalePart())
		{
			// we must express the I vector in the system basis, so we need to multiply it by the inverted matrix of the system
			return getWorldToLocalMatrix().mulVector(I);
		}
		else
		{
			return sysMat.getScaleUniform() * getWorldToLocalMatrix().mulVector(I);
		}
	}
}

/// ***************************************************************************************
NLMISC::CVector CPSLocated::computeJ(void) const
{
	NL_PS_FUNC(CPSLocated_computeJ)
	CHECK_PS_INTEGRITY
	const NLMISC::CMatrix &sysMat = _Owner->getSysMat();
	if (getMatrixMode() == PSIdentityMatrix)
	{
		if (!sysMat.hasScalePart())
		{
			return _Owner->getInvertedViewMat().getJ();
		}
		else
		{
			return sysMat.getScaleUniform() * _Owner->getInvertedViewMat().getJ();
		}
	}
	else
	{
		if (!sysMat.hasScalePart())
		{
			// we must express the J vector in the system basis, so we need to multiply it by the inverted matrix of the system
			return getWorldToLocalMatrix().mulVector(_Owner->getInvertedViewMat().getJ());
		}
		else
		{
			return sysMat.getScaleUniform() * getWorldToLocalMatrix().mulVector(_Owner->getInvertedViewMat().getJ());
		}
	}
}

/// ***************************************************************************************
NLMISC::CVector CPSLocated::computeK(void) const
{
	NL_PS_FUNC(CPSLocated_computeK)
	CHECK_PS_INTEGRITY
	const NLMISC::CMatrix &sysMat = _Owner->getSysMat();
	if (getMatrixMode() == PSIdentityMatrix)
	{

		if (!sysMat.hasScalePart())
		{
			return _Owner->getInvertedViewMat().getK();
		}
		else
		{
			return sysMat.getScaleUniform() * _Owner->getInvertedViewMat().getK();
		}
	}
	else
	{
		if (!sysMat.hasScalePart())
		{
			// we must express the K vector in the system basis, so we need to multiply it by the inverted matrix of the system
			return getWorldToLocalMatrix().mulVector(_Owner->getInvertedViewMat().getK());
		}
		else
		{
			return sysMat.getScaleUniform() * getWorldToLocalMatrix().mulVector(_Owner->getInvertedViewMat().getK());
		}
	}
}

/// ***************************************************************************************
NLMISC::CVector CPSLocated::computeKWithZAxisAligned(void) const
{
	NL_PS_FUNC(CPSLocated_computeKWithZAxisAligned)
	CHECK_PS_INTEGRITY
		const NLMISC::CMatrix &sysMat = _Owner->getSysMat();
	if (getMatrixMode() == PSIdentityMatrix)
	{
		if (!sysMat.hasScalePart())
		{
			return CVector::K;
		}
		else
		{
			return CVector(0.f, 0.f, sysMat.getScaleUniform());
		}
	}
	else
	{
		if (!sysMat.hasScalePart())
		{
			// we must express the K vector in the system basis, so we need to multiply it by the inverted matrix of the system
			return getWorldToLocalMatrix().mulVector(CVector::K);
		}
		else
		{
			return getWorldToLocalMatrix().mulVector(CVector(0.f, 0.f, sysMat.getScaleUniform()));
		}
	}
}

/// ***************************************************************************************
IDriver *CPSLocated::getDriver() const
{
	NL_PS_FUNC(CPSLocated_getDriver)
	CHECK_PS_INTEGRITY
	nlassert(_Owner);
	nlassert (_Owner->getDriver() ); // you haven't called setDriver on the system
	return _Owner->getDriver();
}

/// ***************************************************************************************
/// dtor
CPSLocated::~CPSLocated()
{
	NL_PS_FUNC(CPSLocated_CPSLocated)
	CHECK_PS_INTEGRITY
	// we must do a copy, because the subsequent call can modify this vector
	TDtorObserversVect copyVect(_DtorObserversVect.begin(), _DtorObserversVect.end());
	// call all the dtor observers
	for (TDtorObserversVect::iterator it = copyVect.begin(); it != copyVect.end(); ++it)
	{
		(*it)->notifyTargetRemoved(this);
	}

	nlassert(_CollisionInfoNbRef == 0); //If this is not = 0, then someone didnt call releaseCollisionInfo
										 // If this happen, you can register with the registerDTorObserver
										 // (observer pattern)
										 // and override notifyTargetRemove to call releaseCollisionInfo
	nlassert(_IntegrableForces.size() == 0);
	nlassert(_NonIntegrableForceNbRefs == 0);

	// delete all bindable
	for (TLocatedBoundCont::iterator it2 = _LocatedBoundCont.begin(); it2 != _LocatedBoundCont.end(); ++it2)
	{
		(*it2)->finalize();
		delete *it2;
	}
	_LocatedBoundCont.clear();

	delete _LifeScheme;
	delete _MassScheme;
	delete _CollisionNextPos;
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
/**
* sorted insertion  (by decreasing priority order) of a bindable (particle e.g an aspect, emitter) in a located
*/
bool CPSLocated::bind(CPSLocatedBindable *lb)
{
	NL_PS_FUNC(CPSLocated_bind)
	CHECK_PS_INTEGRITY
	nlassert(std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), lb) == _LocatedBoundCont.end());
	TLocatedBoundCont::iterator it = _LocatedBoundCont.begin();
	while (it != _LocatedBoundCont.end() && **it < *lb) // the "<" operator sort them correctly
	{
		++it;
	}

	_LocatedBoundCont.insert(it, lb);
	lb->setOwner(this);
	lb->resize(_MaxSize);

	// any located bindable that is bound to us should have no element in it for now !!
	// we resize it anyway...

	uint32 initialSize  = _Size;
	CPSEmitterInfo ei;
	ei.setDefaults();
	for (uint k = 0; k < initialSize; ++k)
	{
		_Size = k;
		lb->newElement(ei);
	}
	_Size = initialSize;


	if (_ParametricMotion) lb->motionTypeChanged(true);

	/// the max number of shapes may have changed
	//notifyMaxNumFacesChanged();

	if (_Owner)
	{
		CParticleSystem *ps = _Owner;
		if (ps->getBypassMaxNumIntegrationSteps())
		{
			if (!ps->canFinish())
			{
				unbind(getIndexOf(lb));
				nlwarning("<CPSLocated::bind> Can't bind the located : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. Located is not bound.");
				return false;
			}
		}
		// if there's an extern id, register in lb list
		if (lb->getExternID() != 0)
		{
			// register in ID list
			ps->registerLocatedBindableExternID(lb->getExternID(), lb);
		}
		_Owner->systemDurationChanged();
	}

	CHECK_PS_INTEGRITY
	return true;
}

/// ***************************************************************************************
void CPSLocated::remove(const CPSLocatedBindable *p)
{
	NL_PS_FUNC(CPSLocated_remove)
	CHECK_PS_INTEGRITY
	TLocatedBoundCont::iterator it = std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), p);
	nlassert(it != _LocatedBoundCont.end());
	(*it)->finalize();
	delete *it;
	_LocatedBoundCont.erase(it);
	if (_Owner)
	{
		_Owner->systemDurationChanged();
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::registerDtorObserver(CPSLocatedBindable *anObserver)
{
	NL_PS_FUNC(CPSLocated_registerDtorObserver)
	CHECK_PS_INTEGRITY
	// check whether the observer wasn't registered twice
	nlassert(std::find(_DtorObserversVect.begin(), _DtorObserversVect.end(), anObserver) == _DtorObserversVect.end());
	_DtorObserversVect.push_back(anObserver);
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::unregisterDtorObserver(CPSLocatedBindable *anObserver)
{
	NL_PS_FUNC(CPSLocated_unregisterDtorObserver)
	CHECK_PS_INTEGRITY
	// check that it was registered
	TDtorObserversVect::iterator it = std::find(_DtorObserversVect.begin(), _DtorObserversVect.end(), anObserver);
	nlassert(it != _DtorObserversVect.end());
	_DtorObserversVect.erase(it);
	CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
void CPSLocated::postNewElement(const NLMISC::CVector &pos,
							    const NLMISC::CVector &speed,
							    CPSLocated &emitterLocated,
							    uint32 indexInEmitter,
							    TPSMatrixMode speedCoordSystem,
							    TAnimationTime lifeTime)
{
	NL_PS_FUNC(CPSLocated_postNewElement)
	nlassert(CParticleSystem::InsideSimLoop); // should be called only inside the sim loop!
	// In the event loop life of emitter is updated just after particles are spawned, so we must check there if the particle wasn't emitted when the
	// emitter was already destroyed
	// When postNewElement is called, the particle and the emitter that created it live at the same date, so EmitterLife - ParticleLife should be > 1.f
	float emitterLife;
	if (!emitterLocated.getLastForever())
	{
		if (emitterLocated._LifeScheme)
		{
			emitterLife = emitterLocated._Time[indexInEmitter] - lifeTime * CParticleSystem::RealEllapsedTimeRatio * emitterLocated._TimeIncrement[indexInEmitter];
			if (emitterLife >= 1.f)
			{
				return; // emitter had finished its life
			}
		}
		else
		{
			emitterLife = emitterLocated._Time[indexInEmitter] * emitterLocated._InitialLife - lifeTime * CParticleSystem::RealEllapsedTimeRatio;
			if (emitterLife >= emitterLocated._InitialLife)
			{
				return; // emitter had finished its life
			}
			if (emitterLocated._InitialLife != 0.f)
			{
				emitterLife /= emitterLocated._InitialLife;
			}
		}
	}
	else
	{
		emitterLife = emitterLocated.getTime()[indexInEmitter];
	}

	// now check that the emitter didn't collide before it spawned a particle
	if (emitterLocated.hasCollisionInfos())
	{
		const CPSCollisionInfo &ci = _Collisions[indexInEmitter];
		if (ci.Dist != -1.f)
		{
			// a collision occured, check time from collision to next time step
			if ((emitterLocated.getPos()[indexInEmitter] - ci.NewPos) * (pos - ci.NewPos) > 0.f) return; // discard emit that are farther than the collision
		}
	}


	// create a request to create a new element
	CParticleSystem::CSpawnVect &sp = *CParticleSystem::_Spawns[getIndex()];
	if (!_Owner->getAutoCountFlag() && sp.MaxNumSpawns == sp.SpawnInfos.size()) return; // no more place to spawn
	if (getMaxSize() >= ((1 << 16) - 1)) return;
	sp.SpawnInfos.resize(sp.SpawnInfos.size() + 1);
	CPSSpawnInfo &si = sp.SpawnInfos.back();
	si.EmitterInfo.Pos = emitterLocated.getPos()[indexInEmitter];
	si.EmitterInfo.Speed = emitterLocated.getSpeed()[indexInEmitter];
	si.EmitterInfo.InvMass = emitterLocated.getInvMass()[indexInEmitter];
	si.EmitterInfo.Life = emitterLife;
	si.EmitterInfo.Loc = &emitterLocated;
	si.SpawnPos = pos;
	si.Speed = speed;
	si.SpeedCoordSystem = speedCoordSystem;
	si.LifeTime = lifeTime;
}


/// ***************************************************************************************
sint32 CPSLocated::newElement(const CPSSpawnInfo &si, bool doEmitOnce /* = false */, TAnimationTime ellapsedTime)
{
	NL_PS_FUNC(CPSLocated_newElement)
	CHECK_PS_INTEGRITY
	sint32 creationIndex;

	// get the convertion matrix  from the emitter basis to the emittee basis
	// if the emitter is null, we assume that the coordinate are given in the chosen basis for this particle type
	if (_MaxSize == _Size)
	{
		if (_Owner && _Owner->getAutoCountFlag() && getMaxSize() < ((1 << 16) - 1) )
		{
			// we are probably in edition mode -> auto-count mode helps to compute ideal particle array size
			// but at the expense of costly allocations
			uint maxSize = getMaxSize();
			resize((uint32) std::min((uint) NLMISC::raiseToNextPowerOf2(maxSize + 1), (uint) ((1 << 16) - 1))); // force a reserve with next power of 2 (no important in edition mode)
			resize(maxSize + 1);
			CParticleSystem::_SpawnPos.resize(maxSize + 1);
		}
		else
		{
			return -1;
		}
	}

	// During creation, we interpolate the position of the system (by using the ellapsed time) if particle are created in world basis and if the emitter is in local basis.
	// Example a fireball FX let particles in world basis, but the fireball is moving. If we dont interpolate position between 2 frames, emission will appear to be "sporadic".
	// For now, we manage the local to world case. The world to local is possible, but not very useful
	switch(si.EmitterInfo.Loc ? si.EmitterInfo.Loc->getMatrixMode() : this->getMatrixMode())
	{
		case PSFXWorldMatrix:
			switch(this->getMatrixMode())
			{
				case PSFXWorldMatrix:
				{
					creationIndex  =_Pos.insert(si.SpawnPos);
				}
				break;
				case PSIdentityMatrix:
				{
					CVector fxPosDelta;
					_Owner->interpolateFXPosDelta(fxPosDelta, si.LifeTime);
					creationIndex  =_Pos.insert(_Owner->getSysMat() * si.SpawnPos + fxPosDelta);
				}
				break;
				case PSUserMatrix:
				{
					CVector fxPosDelta;
					_Owner->interpolateFXPosDelta(fxPosDelta, si.LifeTime);
					CVector userMatrixPosDelta;
					_Owner->interpolateUserPosDelta(userMatrixPosDelta, si.LifeTime);
					creationIndex  =_Pos.insert(_Owner->getInvertedUserMatrix() * (_Owner->getSysMat() * si.SpawnPos + fxPosDelta - userMatrixPosDelta));
				}
				break;
				default:
				nlassert(0);
			}
		break;
		case PSIdentityMatrix:
			switch(this->getMatrixMode())
			{
				case PSFXWorldMatrix:
				{
					CVector fxPosDelta;
					_Owner->interpolateFXPosDelta(fxPosDelta, si.LifeTime);
					creationIndex  =_Pos.insert(_Owner->getInvertedSysMat() * (si.SpawnPos - fxPosDelta));
				}
				break;
				case PSIdentityMatrix:
				{
					creationIndex  =_Pos.insert(si.SpawnPos);
				}
				break;
				case PSUserMatrix:
				{
					CVector userMatrixPosDelta;
					_Owner->interpolateUserPosDelta(userMatrixPosDelta, si.LifeTime);
					creationIndex  =_Pos.insert(_Owner->getInvertedUserMatrix() * (si.SpawnPos - userMatrixPosDelta));
				}
				break;
				default:
				nlassert(0);
			}
		break;
		case PSUserMatrix:
			switch(this->getMatrixMode())
			{
				case PSFXWorldMatrix:
				{
					CVector fxPosDelta;
					_Owner->interpolateFXPosDelta(fxPosDelta, si.LifeTime);
					CVector userMatrixPosDelta;
					_Owner->interpolateUserPosDelta(userMatrixPosDelta, si.LifeTime);
					creationIndex  =_Pos.insert(_Owner->getInvertedSysMat() * (_Owner->getUserMatrix() * si.SpawnPos + userMatrixPosDelta- fxPosDelta));
				}
				break;
				case PSIdentityMatrix:
				{
					CVector userMatrixPosDelta;
					_Owner->interpolateUserPosDelta(userMatrixPosDelta, si.LifeTime);
					creationIndex  =_Pos.insert(_Owner->getUserMatrix() * si.SpawnPos + userMatrixPosDelta);
				}
				break;
				case PSUserMatrix:
				{
					creationIndex  =_Pos.insert(si.SpawnPos);
				}
				break;
				default:
				nlassert(0);
			}
		break;
		default:
			nlassert(0);
	}


	nlassert(creationIndex != -1); // all attributs must contains the same number of elements

	if (si.SpeedCoordSystem == this->getMatrixMode()) // is speed vector expressed in the good basis ?
	{
		_Speed.insert(si.Speed);
	}
	else
	{
		// must do conversion of speed
		nlassert(_Owner);
		const NLMISC::CMatrix &convMat = getConversionMatrix(*_Owner, this->getMatrixMode(), si.SpeedCoordSystem);
		_Speed.insert(convMat.mulVector(si.Speed));
	}

	_InvMass.insert(1.f / ((_MassScheme && si.EmitterInfo.Loc) ? _MassScheme->get(si.EmitterInfo) : _InitialMass ) );
	if (CParticleSystem::InsideSimLoop)
	{
		CParticleSystem::_SpawnPos[creationIndex] = _Pos[creationIndex];
	}
	// compute age of particle when it has been created
	if (getLastForever())
	{
		// age of particle is in seconds
		_Time.insert(CParticleSystem::RealEllapsedTimeRatio * si.LifeTime);
		_TimeIncrement.insert(_InitialLife != 0.f ? 1.f / _InitialLife : 1.f);
	}
	else
	{
		const float totalLifeTime = (_LifeScheme && si.EmitterInfo.Loc) ?  _LifeScheme->get(si.EmitterInfo) : _InitialLife ;
		float timeIncrement = totalLifeTime ? 1.f / totalLifeTime : 10E6f;
		_TimeIncrement.insert(timeIncrement);
		_Time.insert(CParticleSystem::RealEllapsedTimeRatio * si.LifeTime * timeIncrement);
	}


	// test whether parametric motion is used, and generate the infos that are needed then
	if (_ParametricMotion)
	{
		_PInfo.insert( CParametricInfo(_Pos[creationIndex], _Speed[creationIndex], _Owner->getSystemDate() + CParticleSystem::RealEllapsedTime - CParticleSystem::RealEllapsedTimeRatio * si.LifeTime) );
	}
	else
	{
		_Pos[creationIndex] += si.LifeTime * _Speed[creationIndex];
	}

	///////////////////////////////////////////
	// generate datas for all bound objects  //
	///////////////////////////////////////////
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->newElement(si.EmitterInfo);
		// if element is an emitter, then must bias the emission time counter because it will be updated of frameDT, but the particle has been alive for (frameDT - deltaT)
		if ((*it)->getType() == PSEmitter)
		{
			CPSEmitter *pEmit = NLMISC::safe_cast<CPSEmitter *>(*it);
			pEmit->_Phase[creationIndex] -= std::max(0.f, (ellapsedTime - si.LifeTime));
		}
	}
	if (doEmitOnce && !CPSEmitter::getBypassEmitOnDeath())
	{
		// can be called only outside the sim loop (when the user triggers an emitters for example)
		for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			if ((*it)->getType() == PSEmitter)
			{
				CPSEmitter *pEmit = NLMISC::safe_cast<CPSEmitter *>(*it);
				if (pEmit->getEmissionType() == CPSEmitter::once)
				{
					for(uint k = 0; k < getSize(); ++k)
					{
						pEmit->singleEmit(k, 1);
					}
				}
			}
		}
	}
	if (_CollisionNextPos)
	{
		_CollisionNextPos->insert();
	}
	++_Size;	// if this is modified, you must also modify the getNewElementIndex in this class
				// because that method give the index of the element being created for overrider of the newElement method
				// of the CPSLocatedClass (which is called just above)


	CHECK_PS_INTEGRITY
	return creationIndex;
}


/// ***************************************************************************************
sint32 CPSLocated::newElement(const CVector &pos, const CVector &speed, CPSLocated *emitter, uint32 indexInEmitter,
							  TPSMatrixMode speedCoordSystem, bool doEmitOnce /* = false */)
{
	NL_PS_FUNC(CPSLocated_newElement)
	CPSSpawnInfo si;
	si.EmitterInfo.Loc = emitter;
	if (emitter)
	{
		si.EmitterInfo.Pos = emitter->getPos()[indexInEmitter];
		si.EmitterInfo.Speed = emitter->getSpeed()[indexInEmitter];
		si.EmitterInfo.InvMass = emitter->getInvMass()[indexInEmitter];
		si.EmitterInfo.Life = emitter->getTime()[indexInEmitter];
	}
	else
	{
		si.EmitterInfo.Pos = NLMISC::CVector::Null;
		si.EmitterInfo.Speed = NLMISC::CVector::Null;
		si.EmitterInfo.InvMass = 1.f;
		si.EmitterInfo.Life = 0.f;
	}
	si.SpawnPos = pos;
	si.Speed = speed;
	si.SpeedCoordSystem = speedCoordSystem;
	si.LifeTime = 0.f;
	return newElement(si, doEmitOnce, 0.f);
}


/// ***************************************************************************************
static inline uint32 IDToLittleEndian(uint32 input)
{
	NL_PS_FUNC(IDToLittleEndian)
	#ifdef NL_LITTLE_ENDIAN
		return input;
	#else
		return ((input & (0xff<<24))>>24)
				|| ((input & (0xff<<16))>>8)
				|| ((input & (0xff<<8))<<8)
				|| ((input & 0xff)<<24);
	#endif
}

/// ***************************************************************************************
inline void CPSLocated::deleteElementBase(uint32 index)
{
	NL_PS_FUNC(CPSLocated_deleteElementBase)
	// remove common located's attributes
	_InvMass.remove(index);
	_Pos.remove(index);
	_Speed.remove(index);
	_Time.remove(index);
	_TimeIncrement.remove(index);
	if (_CollisionNextPos)
	{
		_CollisionNextPos->remove(index);
	}
	if (_ParametricMotion)
	{
		_PInfo.remove(index);
	}
	--_Size;
	if (_TriggerOnDeath)
	{
		const uint32 id = IDToLittleEndian(_TriggerID);
		nlassert(_Owner);
		uint numLb  = _Owner->getNumLocatedBindableByExternID(id);
		for (uint k = 0; k < numLb; ++k)
		{
			CPSLocatedBindable *lb = _Owner->getLocatedBindableByExternID(id, k);
			if (lb->getType() == PSEmitter)
			{
				CPSEmitter *e = NLMISC::safe_cast<CPSEmitter *>(lb);
				e->setEmitTrigger();
			}
		}
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSLocated_deleteElement)
	#ifdef NL_DEBUG
		if (CParticleSystem::InsideSimLoop)
		{
			nlassert(CParticleSystem::InsideRemoveLoop);
		}
	#endif
	CHECK_PS_INTEGRITY
	nlassert(index < _Size);
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->deleteElement(index);
	}
	deleteElementBase(index);
}


/// ***************************************************************************************
void CPSLocated::deleteElement(uint32 index, TAnimationTime timeToNextSimStep)
{
	NL_PS_FUNC(CPSLocated_deleteElement)
	#ifdef NL_DEBUG
		if (CParticleSystem::InsideSimLoop)
		{
			nlassert(CParticleSystem::InsideRemoveLoop);
		}
	#endif
	nlassert(index < _Size);
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->deleteElement(index, timeToNextSimStep);
	}
	deleteElementBase(index);
}

/// Resize the located container
/// ***************************************************************************************
void CPSLocated::resize(uint32 newSize)
{
	NL_PS_FUNC(CPSLocated_resize)
	CHECK_PS_INTEGRITY
	nlassert(newSize < (1 << 16));
	if (newSize < _Size)
	{
		for (uint32 k = _Size - 1; k >= newSize; --k)
		{
			deleteElement(k);

			if (k == 0) break; // we're dealing with unsigned quantities
		}
		_Size = newSize;
	}


	_MaxSize = newSize;
	_InvMass.resize(newSize);
	_Pos.resize(newSize);
	_Speed.resize(newSize);
	_Time.resize(newSize);
	_TimeIncrement.resize(newSize);

	if (_ParametricMotion)
	{
		_PInfo.resize(newSize);
	}

	if (_CollisionNextPos)
	{
		_CollisionNextPos->resize(newSize);
	}


	// resize attributes for all bound objects
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->resize(newSize);
	}


	/// compute the new max number of faces
	//notifyMaxNumFacesChanged();
	CHECK_PS_INTEGRITY
}

// dummy struct for serial of a field that has been removed
class CDummyCollision
{
public:
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		NL_PS_FUNC(CDummyCollision_serial)
		f.serialVersion(1);
		float dummyDist = 0.f;
		NLMISC::CVector dummyNewPos, dummyNewSpeed;
		f.serial(dummyDist, dummyNewPos, dummyNewSpeed);
	};
};

/// ***************************************************************************************
void CPSLocated::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSLocated_serial)

	// version 7 : - removed the field _NbFramesToSkip to get some space (it is never used)
	//			   - removed the requestStack (system graph can't contains loops now)
	//			   - removed _CollisonInfos because they are now static

	// version 4 to version 5 : bugfix with reading of collisions
	sint ver = f.serialVersion(7);
	CParticleSystemProcess::serial(f);

	if (f.isReading() && !CParticleSystem::getSerializeIdentifierFlag())
	{
		// just skip the name
		sint32 len;
		f.serial(len);
		f.seek(len, NLMISC::IStream::current);
	}
	else
	{
		f.serial(_Name);
	}

	f.serial(_InvMass);
	f.serial(_Pos);
	f.serial(_Speed);
	f.serial(_Time);
	if (f.isReading())
	{
		// tmp fix : some fx were saved with a life that is != to 0
		// this cause an assertion in CPSLocated::updateLife, because all particle are assumed to have an age of 0 when the system is started
		// TODO : saving _Time is maybe unecessary... or find something better for CPSLocated::updateLife
		uint timeSize = _Time.getSize();
		if (timeSize != 0)
		{
			std::fill(&_Time[0], &_Time[0] + timeSize, 0.f);
		}
	}
	f.serial(_TimeIncrement);
	f.serial(_Size);
	f.serial(_MaxSize);

	bool lastForever = _LastForever;
	f.serial(lastForever);
	_LastForever = lastForever;

	if (ver < 7)
	{
		nlassert(f.isReading());
		// serial a dummy ptr (previous code did a serial ptr)
		uint64 dummyPtr;
		f.serial(dummyPtr);
		if (dummyPtr)
		{
			#ifdef PS_FAST_ALLOC
				extern NLMISC::CContiguousBlockAllocator *PSBlockAllocator;
				NLMISC::CContiguousBlockAllocator *oldAlloc = PSBlockAllocator;
				PSBlockAllocator = NULL;
			#endif
			static CPSAttrib<CDummyCollision> col;
			col.clear();
			f.serial(col);
			#ifdef PS_FAST_ALLOC
				PSBlockAllocator = oldAlloc;
			#endif
		}
	}
	f.serial(_CollisionInfoNbRef); // TODO avoid to serialize this ?
	//
	if (f.isReading())
	{
		if (_CollisionInfoNbRef)
		{
			_CollisionNextPos = new TPSAttribVector;
			_CollisionNextPos->resize(_Pos.getMaxSize());
			for(uint k = 0; k < _Size; ++k)
			{
				_CollisionNextPos->insert();
			}
		}
	}
	//CHECK_PS_INTEGRITY
	if (f.isReading())
	{
		delete _LifeScheme;
		delete _MassScheme;

		bool useScheme;
		f.serial(useScheme);
		if (useScheme)
		{
			f.serialPolyPtr(_LifeScheme);
		}
		else
		{
			f.serial(_InitialLife);
			_LifeScheme = NULL;
		}

		f.serial(useScheme);
		if (useScheme)
		{
			f.serialPolyPtr(_MassScheme);
		}
		else
		{
			f.serial(_InitialMass);
			nlassert(_InitialMass > 0);
			_MassScheme = NULL;
		}
	}
	else
	{
		bool bFalse = false, bTrue = true;
		if (_LifeScheme)
		{
			f.serial(bTrue);
			f.serialPolyPtr(_LifeScheme);
		}
		else
		{
			f.serial(bFalse);
			f.serial(_InitialLife);
		}
		if (_MassScheme)
		{
			f.serial(bTrue);
			f.serialPolyPtr(_MassScheme);
		}
		else
		{
			f.serial(bFalse);
			nlassert(_InitialMass > 0);
			f.serial(_InitialMass);
		}
	}

	if (ver < 7)
	{
		uint32 dummy = 0; // was previously the field "_NbFramesToSkip"
		f.serial(dummy);
	}

	f.serialContPolyPtr(_DtorObserversVect);

	if (ver < 7)
	{
		nlassert(f.isReading());
		// previously, there was a request stack serialized (because system permitted loops)
		uint32 size;
		f.serial(size);
		nlassert(size == 0);
		/*
		for (uint32 k = 0; k < size; ++k)
		{
			TNewElementRequestStack::value_type t;
			f.serial(t);
			_RequestStack.push(t);
		}
		*/
	}

	if (ver < 7)
	{
		nlassert(f.isReading());
		bool dummy;
		f.serial(dummy); // was previously the flag "_UpdateLock"
	}

	f.serialContPolyPtr(_LocatedBoundCont);


	// check that owners are good
	#ifdef NL_DEBUG
		for(uint k = 0; k < _LocatedBoundCont.size(); ++k)
		{
			nlassert(_LocatedBoundCont[k]->getOwner() == this);
		}
	#endif

	if (ver > 1)
	{
		bool lodDegradation = _LODDegradation;
		f.serial(lodDegradation);
		_LODDegradation = lodDegradation;
	}

	if (ver > 2)
	{
		bool parametricMotion = _ParametricMotion;
		f.serial(parametricMotion);
		_ParametricMotion = parametricMotion;
	}

	if (f.isReading())
	{
		// evaluate our max number of faces
		//notifyMaxNumFacesChanged();

		if (_ParametricMotion)
		{
			_ParametricMotion = false;
			allocateParametricInfos();
			_ParametricMotion = true;
		}
	}

	if (ver > 3)
	{
		bool  triggerOnDeath = _TriggerOnDeath;
		f.serial(triggerOnDeath);
		_TriggerOnDeath = triggerOnDeath;
		f.serial(_TriggerID);
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
// integrate speed of particles. Makes eventually use of SSE instructions when present
static void IntegrateSpeed(uint count, float *src1, const float *src2, float *dest, float ellapsedTime)
{
	NL_PS_FUNC(IntegrateSpeed)
	#if 0 // this works, but is not enabled for now. The precision is not that good...
	/*
		#ifdef NL_OS_WINDOWS



		if (NLMISC::CCpuInfo::hasSSE()
			&& ((uint) src1 & 15) == ((uint) src2 & 15)
			&& ! ((uint) src1 & 3)
			&& ! ((uint) src2 & 3)
		   )   // must must be sure that memory alignment is identical
		{

			// compute first datas in order to align to 16 byte boudary

			uint alignCount =  ((uint) src1 >> 2) & 3; // number of float to process

			while (alignCount --)
			{
				*src1++ += ellapsedTime * *src2 ++;
			}



			count -= alignCount;
			if (count > 3)
			{
				float et[4] = { ellapsedTime, ellapsedTime, ellapsedTime, ellapsedTime};
				// sse part of computation
				__asm
				{
						mov  ecx, count
						shr  ecx, 2


						xor   edx, edx

						mov    eax, src1
						mov    ebx, src2
						movups  xmm0, et[0]
					myLoop:
						movaps xmm2, [ebx + 8 * edx]
						movaps xmm1, [eax + 8 * edx]
						mulps  xmm2, xmm0
						addps  xmm1, xmm2
						movaps [eax + 8 * edx], xmm1
						add edx, 2
						dec ecx
						jne myLoop
				}
			}
			// proceed with left float
			count &= 3;

			if (count)
			{
				src1 += alignCount;
				src2 += alignCount;
				do
				{
					*src1 += ellapsedTime * *src2;

					++src1;
					++src2;
				}
				while (--count);
			}

		}
		else
		#endif
		*/
	#endif
	{
		// standard version

		// standard version
		uint countDiv8 = count>>3;
		count &= 7; // takes count % 8

		if (dest == src1)
		{
			while (countDiv8 --)
			{
				src1[0] += ellapsedTime * src2[0];
				src1[1] += ellapsedTime * src2[1];
				src1[2] += ellapsedTime * src2[2];
				src1[3] += ellapsedTime * src2[3];

				src1[4] += ellapsedTime * src2[4];
				src1[5] += ellapsedTime * src2[5];
				src1[6] += ellapsedTime * src2[6];
				src1[7] += ellapsedTime * src2[7];

				src2 += 8;
				src1 += 8;
			}
			while (count--)
			{
				*src1++ += ellapsedTime * *src2++;
			}
		}
		else
		{
			while (countDiv8 --)
			{
				dest[0] = src1[0] + ellapsedTime * src2[0];
				dest[1] = src1[1] + ellapsedTime * src2[1];
				dest[2] = src1[2] + ellapsedTime * src2[2];
				dest[3] = src1[3] + ellapsedTime * src2[3];
				dest[4] = src1[4] + ellapsedTime * src2[4];
				dest[5] = src1[5] + ellapsedTime * src2[5];
				dest[6] = src1[6] + ellapsedTime * src2[6];
				dest[7] = src1[7] + ellapsedTime * src2[7];
				src2 += 8;
				src1 += 8;
				dest += 8;
			}
			while (count--)
			{
				*dest++ = *src1++ + ellapsedTime * *src2++;
			}
		}
	}
}

/// ***************************************************************************************
void CPSLocated::computeMotion()
{
	NL_PS_FUNC(CPSLocated_computeMotion)
	nlassert(_Size);
	// there are 2 integration steps : with and without collisions
	if (!_CollisionNextPos) // no collisionner are used
	{
		{
			MINI_TIMER(PSMotion3)
			if (_Size != 0) // avoid referencing _Pos[0] if there's no size, causes STL vectors to assert...
				IntegrateSpeed(_Size * 3, &_Pos[0].x, &_Speed[0].x, &_Pos[0].x, CParticleSystem::EllapsedTime);
		}
	}
	else
	{
		{
			MINI_TIMER(PSMotion4)
			// compute new position after the timeStep
			IntegrateSpeed(_Size * 3, &_Pos[0].x, &_Speed[0].x, &(*_CollisionNextPos)[0].x, CParticleSystem::EllapsedTime);
			nlassert(CPSLocated::_Collisions.size() >= _Size);
			computeCollisions(0, &_Pos[0], &(*_CollisionNextPos)[0]);
			// update new poositions by just swapping the 2 vectors
			_CollisionNextPos->swap(_Pos);
		}
	}
}



/// ***************************************************************************************
void CPSLocated::computeNewParticleMotion(uint firstInstanceIndex)
{
	NL_PS_FUNC(CPSLocated_computeNewParticleMotion)
	nlassert(_CollisionNextPos);
	resetCollisions(_Size);
	computeCollisions(firstInstanceIndex, &CParticleSystem::_SpawnPos[0], &_Pos[0]);
}

/// ***************************************************************************************
void CPSLocated::resetCollisions(uint numInstances)
{
	NL_PS_FUNC(CPSLocated_resetCollisions)
	CPSCollisionInfo *currCollision = _FirstCollision;
	while (currCollision)
	{
		currCollision->Dist = -1.f;
		currCollision = currCollision->Next;
	}
	_FirstCollision = NULL;
	if (numInstances > _Collisions.size())
	{
		uint oldSize = (uint) _Collisions.size();
		_Collisions.resize(numInstances);
		for(uint k = oldSize; k < numInstances; ++k)
		{
			_Collisions[k].Index = k;
		}
	}
}

/// ***************************************************************************************
void CPSLocated::updateCollisions()
{
	NL_PS_FUNC(CPSLocated_updateCollisions)
	CPSCollisionInfo *currCollision = _FirstCollision;
	if (getLastForever())
	{
		while (currCollision)
		{
			_Pos[currCollision->Index] = currCollision->NewPos;
			std::swap(_Speed[currCollision->Index], currCollision->NewSpeed); // keep speed because may be needed when removing particles
			// notify each located bindable that a bounce occured ...
			for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
			{
				(*it)->bounceOccured(currCollision->Index, computeDateFromCollisionToNextSimStep(currCollision->Index, getAgeInSeconds(currCollision->Index)));
			}
			if (currCollision->CollisionZone->getCollisionBehaviour() == CPSZone::destroy)
			{
				#ifdef NL_DEBUG
					nlassert(CParticleSystem::_ParticleRemoveListIndex[currCollision->Index] == -1);
				#endif
				CParticleSystem::_ParticleToRemove.push_back(currCollision->Index);
				#ifdef NL_DEBUG
					nlassert(CParticleSystem::_ParticleToRemove.size() <= _Size);
				#endif
				CParticleSystem::_ParticleRemoveListIndex[currCollision->Index] = (sint)CParticleSystem::_ParticleToRemove.size() - 1;

			}
			currCollision = currCollision->Next;
		}
	}
	else
	{
		while (currCollision)
		{
			if (_Time[currCollision->Index] >= 1.f)
			{
				// check whether particles died before the collision. If so, just continue (particle has already been inserted in the remove list), and cancel the collision
				float timeToCollision = currCollision->Dist / _Speed[currCollision->Index].norm();
				if (_Time[currCollision->Index] / _TimeIncrement[currCollision->Index] - timeToCollision * CParticleSystem::RealEllapsedTimeRatio >= 1.f)
				{
					// says that collision did not occurs
					currCollision->Dist = -1.f;
					currCollision = currCollision->Next;
					continue;
				}
			}
			// if particle is too old, check whether it died before the collision
			_Pos[currCollision->Index] = currCollision->NewPos;
			std::swap(_Speed[currCollision->Index], currCollision->NewSpeed);
			// notify each located bindable that a bounce occured ...
			if (!_LocatedBoundCont.empty())
			{
				TAnimationTime timeFromcollisionToNextSimStep = computeDateFromCollisionToNextSimStep(currCollision->Index, getAgeInSeconds(currCollision->Index));
				for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
				{
					(*it)->bounceOccured(currCollision->Index, timeFromcollisionToNextSimStep);
				}
			}
			if (currCollision->CollisionZone->getCollisionBehaviour() == CPSZone::destroy)
			{
				if (_Time[currCollision->Index] < 1.f)
				{
					// insert particle only if not already dead
					#ifdef NL_DEBUG
						nlassert(CParticleSystem::_ParticleRemoveListIndex[currCollision->Index] == -1);
					#endif
					CParticleSystem::_ParticleToRemove.push_back(currCollision->Index);
					#ifdef NL_DEBUG
						nlassert(CParticleSystem::_ParticleToRemove.size() <= _Size);
					#endif
					CParticleSystem::_ParticleRemoveListIndex[currCollision->Index] = (sint)CParticleSystem::_ParticleToRemove.size() - 1;
				}
			}
			currCollision = currCollision->Next;
		}
	}

}

/// ***************************************************************************************
void CPSLocated::doLODDegradation()
{
	NL_PS_FUNC(CPSLocated_doLODDegradation)
	nlassert(CParticleSystem::InsideSimLoop);
	nlassert(!CParticleSystem::InsideRemoveLoop);
	CParticleSystem::InsideRemoveLoop = true;
	if (CParticleSystem::EllapsedTime > 0)
	{
		nlassert(_Owner);
		// compute the number of particles to show
		const uint maxToHave = (uint) (_MaxSize * _Owner->getOneMinusCurrentLODRatio());
		if (_Size > maxToHave) // too much instances ?
		{
			// choose a random element to start at, and a random step
			// this will avoid a pulse effect when the system is far away

			uint pos = maxToHave ? rand() % maxToHave : 0;
			uint step  = maxToHave ? rand() % maxToHave : 0;

			do
			{
				deleteElement(pos);
				pos += step;
				if (pos >= maxToHave) pos -= maxToHave;
			}
			while (_Size !=maxToHave);
		}
	}
	CParticleSystem::InsideRemoveLoop = false;
}

/// ***************************************************************************************
void CPSLocated::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSLocated_step)
	CHECK_PS_INTEGRITY
	if (!_Size) return;

	if (pass != PSMotion)
	{
		{
			/*
			uint64 *target;
			switch(pass)
			{
				case PSEmit: target = &PSStatEmit; break;
				case PSCollision: target = &PSStatCollision; break;
				default:
					target = &PSStatRender;
				break;
			}
			MINI_TIMER(*target)
			*/
			// apply the pass to all bound objects
			for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
			{
				if ((*it)->isActive())
				{
					if ((*it)->getLOD() == PSLod1n2 || _Owner->getLOD() == (*it)->getLOD()) // has this object the right LOD ?
					{
						(*it)->step(pass);
					}
				}
			}
		}
	}
	else
	{
		for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			if ((*it)->isActive())
			{
				(*it)->step(pass);
			}
		}

	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::updateLife()
{
	NL_PS_FUNC(CPSLocated_updateLife)
	CHECK_PS_INTEGRITY
	if (!_Size) return;
	if (! _LastForever)
	{
		if (_LifeScheme != NULL)
		{
			TPSAttribTime::iterator itTime = _Time.begin(), itTimeInc = _TimeIncrement.begin();
			for (uint32 k = 0; k < _Size; ++k)
			{
				*itTime += CParticleSystem::RealEllapsedTime * *itTimeInc;
				if (*itTime >= 1.0f)
				{
					#ifdef NL_DEBUG
						nlassert(CParticleSystem::_ParticleRemoveListIndex[k] == -1);
					#endif
					CParticleSystem::_ParticleToRemove.push_back(k);
					#ifdef NL_DEBUG
						nlassert(CParticleSystem::_ParticleToRemove.size() <= _Size);
					#endif
					CParticleSystem::_ParticleRemoveListIndex[k] = (sint)CParticleSystem::_ParticleToRemove.size() - 1;
				}
				++itTime;
				++itTimeInc;
			}
		}
		else /// all particles have the same lifetime
		{
			if (_InitialLife != 0)
			{
				nlassert(_Owner);
				float timeInc = CParticleSystem::RealEllapsedTime / _InitialLife;
				if (_Owner->getSystemDate() + 0.1f + 2.f * timeInc >= (_InitialLife - CParticleSystem::RealEllapsedTime))
				{
					// NB : 0.1f + 2.f * timeInc added to avoid case were time of particle is slighty greater than 1.f after life update because that test failed
					TPSAttribTime::iterator itTime = _Time.begin();
					for (uint32 k = 0; k < _Size; ++k)
					{
						*itTime += timeInc;
						if (*itTime >= 1.0f)
						{
							#ifdef NL_DEBUG
								nlassert(CParticleSystem::_ParticleRemoveListIndex[k] == -1);
							#endif
							CParticleSystem::_ParticleToRemove.push_back(k);
							#ifdef NL_DEBUG
								nlassert(CParticleSystem::_ParticleToRemove.size() <= _Size);
							#endif
							CParticleSystem::_ParticleRemoveListIndex[k] = (sint)CParticleSystem::_ParticleToRemove.size() - 1;
						}
						++ itTime;
					}
				}
				else
				{
					// system has not lasted enough for any particle to die
					TPSAttribTime::iterator itTime = _Time.begin(), itEndTime = _Time.end();
					do
					{
						*itTime += timeInc;
						++itTime;
					}
					while (itTime != itEndTime);
				}
			}
			else
			{
				for(uint k = 0; k < _Size; ++k)
				{
					#ifdef NL_DEBUG
						nlassert(CParticleSystem::_ParticleRemoveListIndex[k] == -1);
					#endif
					CParticleSystem::_ParticleToRemove.push_back(k);
					#ifdef NL_DEBUG
						nlassert(CParticleSystem::_ParticleToRemove.size() <= _Size);
					#endif
					CParticleSystem::_ParticleRemoveListIndex[k] = (sint)CParticleSystem::_ParticleToRemove.size() - 1;
				}
			}
		}
	}
	else
	{
		// the time attribute gives the life in seconds
		TPSAttribTime::iterator itTime = _Time.begin(), endItTime = _Time.end();
		for (; itTime != endItTime; ++itTime)
		{
			*itTime += CParticleSystem::RealEllapsedTime;
		}
	}
		CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
// When a particle is deleted, it is replaced by the last particle in the array
// if this particle is to be deleted to, must update its new index
static inline void removeParticleFromRemoveList(uint indexToRemove, uint arraySize)
{
	NL_PS_FUNC(removeParticleFromRemoveList)
	if (indexToRemove != arraySize)
	{
		if (CParticleSystem::_ParticleRemoveListIndex[arraySize] != -1)
		{
			// when a particle is deleted, it is replaced by the last particle in the array
			// if this particle is to be deleted too, must update its new index (becomes the index of the particle that has just been deleted)
			CParticleSystem::_ParticleToRemove[CParticleSystem::_ParticleRemoveListIndex[arraySize]] = indexToRemove;
			CParticleSystem::_ParticleRemoveListIndex[indexToRemove] = CParticleSystem::_ParticleRemoveListIndex[arraySize];
			CParticleSystem::_ParticleRemoveListIndex[arraySize] = -1; // not to remove any more
		}
		else
		{
			CParticleSystem::_ParticleRemoveListIndex[indexToRemove] = -1;
		}
	}
	else
	{
		CParticleSystem::_ParticleRemoveListIndex[arraySize] = -1;
	}
}

void checkRemoveArray(uint size)
{
	NL_PS_FUNC(checkRemoveArray)
	for(uint k = 0; k < size; ++k)
	{
		if (CParticleSystem::_ParticleRemoveListIndex[k] != -1)
		{
			nlassert(std::find(CParticleSystem::_ParticleRemoveListIndex.begin(), CParticleSystem::_ParticleRemoveListIndex.end(), CParticleSystem::_ParticleRemoveListIndex[k]) != CParticleSystem::_ParticleRemoveListIndex.end());
		}
	}
	for(uint k = 0; k < CParticleSystem::_ParticleToRemove.size(); ++k)
	{
		nlassert(CParticleSystem::_ParticleRemoveListIndex[CParticleSystem::_ParticleToRemove[k]] == (sint) k);
	}

}


/// ***************************************************************************************
#ifndef NL_DEBUG
	inline
#endif
TAnimationTime CPSLocated::computeDateFromCollisionToNextSimStep(uint particleIndex, float particleAgeInSeconds)
{
	NL_PS_FUNC(	CPSLocated_computeDateFromCollisionToNextSimStep)
	// compute time from the start of the sim step to the birth of the particle (or 0 if already born)
	float ageAtStart = CParticleSystem::RealEllapsedTime > particleAgeInSeconds ? CParticleSystem::RealEllapsedTime - particleAgeInSeconds : 0.f;
	ageAtStart /= CParticleSystem::RealEllapsedTimeRatio;
	// compute time to collision. The 'NewSpeed' field is swapped with speed of particle at the sim step start when 'updateCollision' is called, and thus contains the old speed.
	float norm = _Collisions[particleIndex].NewSpeed.norm();
	if (norm == 0.f) return 0.f;
	float timeToCollision = _Collisions[particleIndex].Dist / norm;
	// So time from collision to end of sim step is :
	TAnimationTime result = CParticleSystem::EllapsedTime - ageAtStart - timeToCollision;
	return std::max(0.f, result);
}

/// ***************************************************************************************
void CPSLocated::removeOldParticles()
{
	NL_PS_FUNC(CPSLocated_removeOldParticles)
	nlassert(CParticleSystem::RealEllapsedTime > 0.f);
	#ifdef NL_DEBUG
		CParticleSystem::InsideRemoveLoop = true;
		checkRemoveArray(_Size);
	#endif
	// remove all elements that were marked as too old
	// if there are emitters marked as 'on' death, should correct position by moving backward (because motion is done on a whole time step, so particle is further than it should be)
	if (getLastForever())
	{
		// if the particle lasts for ever it can be destroyed only if it touch a collision zone flaged as 'destroy'
		// during the call to 'updateCollisions', the list of particles to remove will be updated so just test it
		if (hasCollisionInfos())
		{
			for(std::vector<uint>::iterator it = CParticleSystem::_ParticleToRemove.begin(); it != CParticleSystem::_ParticleToRemove.end(); ++it)
			{
				if (_Collisions[*it].Dist != -1.f)
				{
					deleteElement(*it, computeDateFromCollisionToNextSimStep(*it, _Time[*it]));
				}
				else
				{
					deleteElement(*it);
				}
				removeParticleFromRemoveList(*it, _Size);
			}
		}
	}
	else
	if (hasCollisionInfos()) // particle has collision, and limited lifetime
	{
		float ellapsedTimeRatio = CParticleSystem::EllapsedTime / CParticleSystem::RealEllapsedTime;
		for(std::vector<uint>::iterator it = CParticleSystem::_ParticleToRemove.begin(); it != CParticleSystem::_ParticleToRemove.end(); ++it)
		{
			TAnimationTime timeUntilNextSimStep;
			if (_Collisions[*it].Dist == -1.f)
			{
				// no collision occured
				if (_Time[*it] > 1.f)
				{

					if (_LifeScheme)
					{
						_Pos[*it] -= _Speed[*it] * ((_Time[*it] - 1.f) / _TimeIncrement[*it]) * ellapsedTimeRatio;
						timeUntilNextSimStep = (_Time[*it] - 1.f) / _TimeIncrement[*it];
					}
					else
					{
						_Pos[*it] -= _Speed[*it] * ((_Time[*it] - 1.f) * _InitialLife) * ellapsedTimeRatio;
						timeUntilNextSimStep = (_Time[*it] - 1.f) * _InitialLife;
					}
					_Time[*it] = 0.9999f;
				}
				else
				{
					timeUntilNextSimStep = 0.f;
				}
			}
			else
			{
				// a collision occured before particle died, so pos is already good
				if (_LifeScheme)
				{
					timeUntilNextSimStep = computeDateFromCollisionToNextSimStep(*it, _Time[*it] / _TimeIncrement[*it]);
					// compute age of particle when collision occured
					_Time[*it] -= timeUntilNextSimStep * _TimeIncrement[*it];
					NLMISC::clamp(_Time[*it], 0.f, 1.f); // avoid imprecisions
				}
				else
				{
					timeUntilNextSimStep = computeDateFromCollisionToNextSimStep(*it, _Time[*it] * _InitialLife);
					// compute age of particle when collision occured
					_Time[*it] -= timeUntilNextSimStep / (_InitialLife == 0.f ? 1.f : _InitialLife);
					NLMISC::clamp(_Time[*it], 0.f, 1.f); // avoid imprecisions
				}


			}
			deleteElement(*it, timeUntilNextSimStep);
			removeParticleFromRemoveList(*it, _Size);
		}
	}
	else // particle has no collisions, and limited lifetime
	{
		float ellapsedTimeRatio = CParticleSystem::EllapsedTime / CParticleSystem::RealEllapsedTime;
		if (!isParametricMotionEnabled())
		{
			if (_LifeScheme)
			{
				for(std::vector<uint>::iterator it = CParticleSystem::_ParticleToRemove.begin(); it != CParticleSystem::_ParticleToRemove.end(); ++it)
				{
					#ifdef NL_DEBUG
						for(std::vector<uint>::iterator it2 = it; it2 != CParticleSystem::_ParticleToRemove.end(); ++it2)
						{
							nlassert(*it2 < _Size);
						}
					#endif
					TAnimationTime timeUntilNextSimStep;
					if (_Time[*it] > 1.f)
					{
						// move position backward (compute its position at death)
						timeUntilNextSimStep = ((_Time[*it] - 1.f) / _TimeIncrement[*it]) * ellapsedTimeRatio;
						_Pos[*it] -= _Speed[*it] * timeUntilNextSimStep;

						// force time to 1 because emitter 'on death' may rely on the date of emitter to compute its attributes
						_Time[*it] = 0.9999f;
					}
					else
					{
						timeUntilNextSimStep = 0.f;
					}
					deleteElement(*it, timeUntilNextSimStep);
					removeParticleFromRemoveList(*it, _Size);
					#ifdef NL_DEBUG
						for(std::vector<uint>::iterator it2 = it + 1; it2 != CParticleSystem::_ParticleToRemove.end(); ++it2)
						{
							nlassert(*it2 < _Size);
						}
					#endif
				}
			}
			else
			{
				for(std::vector<uint>::iterator it = CParticleSystem::_ParticleToRemove.begin(); it != CParticleSystem::_ParticleToRemove.end(); ++it)
				{
					TAnimationTime timeUntilNextSimStep;
					if (_Time[*it] > 1.f)
					{
						// move position backward
						timeUntilNextSimStep = (_Time[*it] - 1.f) * _InitialLife * ellapsedTimeRatio;
						_Pos[*it] -= _Speed[*it] * timeUntilNextSimStep;
						// force time to 1 because emitter 'on death' may rely on the date of emitter to compute its attributes
						_Time[*it] = 0.9999f;
					}
					else
					{
						timeUntilNextSimStep = 0.f;
					}
					deleteElement(*it, timeUntilNextSimStep);
					removeParticleFromRemoveList(*it, _Size);
				}
			}
		}
		else
		{
			// parametric case
			if (_LifeScheme)
			{
				for(std::vector<uint>::iterator it = CParticleSystem::_ParticleToRemove.begin(); it != CParticleSystem::_ParticleToRemove.end(); ++it)
				{
					TAnimationTime timeUntilNextSimStep;
					if (_Time[*it] > 1.f)
					{
						// move position backward (compute its position at death)
						timeUntilNextSimStep = (_Time[*it] - 1.f) / _TimeIncrement[*it];
						computeParametricPos(_Owner->getSystemDate() + CParticleSystem::RealEllapsedTime - timeUntilNextSimStep, *it, _Pos[*it]);
						timeUntilNextSimStep *= ellapsedTimeRatio;
						// force time to 1 because emitter 'on death' may rely on the date of emitter to compute its attributes
						_Time[*it] = 0.9999f;
					}
					else
					{
						timeUntilNextSimStep = 0.f;
					}
					deleteElement(*it, timeUntilNextSimStep);
					removeParticleFromRemoveList(*it, _Size);
				}
			}
			else
			{
				for(std::vector<uint>::iterator it = CParticleSystem::_ParticleToRemove.begin(); it != CParticleSystem::_ParticleToRemove.end(); ++it)
				{
					TAnimationTime timeUntilNextSimStep;
					if (_Time[*it] > 1.f)
					{
						// move position backward
						timeUntilNextSimStep = (_Time[*it] - 1.f) * _InitialLife;
						computeParametricPos(_Owner->getSystemDate() + CParticleSystem::RealEllapsedTime - timeUntilNextSimStep, *it, _Pos[*it]);
						timeUntilNextSimStep *= ellapsedTimeRatio;
						// force time to 1 because emitter 'on death' may rely on the date of emitter to compute its attributes
						_Time[*it] = 0.9999f;
					}
					else
					{
						timeUntilNextSimStep = 0.f;
					}
					deleteElement(*it, timeUntilNextSimStep);
					removeParticleFromRemoveList(*it, _Size);
				}
			}
		}
	}
	#ifdef NL_DEBUG
		CParticleSystem::InsideRemoveLoop = false;
	#endif
	CParticleSystem::_ParticleToRemove.clear();
	#ifdef NL_DEBUG
		if (!_LastForever)
		{
			for(uint k = 0; k < _Size; ++k)
			{
				nlassert(_Time[k] >= 0.f && _Time[k] <= 1.f);
			}
		}
	#endif
}

/// ***************************************************************************************
void CPSLocated::addNewlySpawnedParticles()
{
	NL_PS_FUNC(CPSLocated_addNewlySpawnedParticles)
	#ifdef NL_DEBUG
		CParticleSystem::InsideNewElementsLoop = true;
	#endif
		CParticleSystem::CSpawnVect &spawns = *CParticleSystem::_Spawns[getIndex()];
		if (spawns.SpawnInfos.empty()) return;
		uint numSpawns = 0;
		if (!_Owner->getAutoCountFlag())
		{
			CParticleSystem::_SpawnPos.resize(getMaxSize());
			numSpawns = std::min((uint) (_MaxSize - _Size), (uint) spawns.SpawnInfos.size());
		}
		else
		{
			numSpawns = (uint) spawns.SpawnInfos.size();
		}
		CParticleSystem::TSpawnInfoVect::const_iterator endIt = spawns.SpawnInfos.begin() + numSpawns;
		if (_LastForever)
		{
			for (CParticleSystem::TSpawnInfoVect::const_iterator it = spawns.SpawnInfos.begin(); it !=endIt; ++it)
			{
//			sint32 insertionIndex = 
				newElement(*it, false, CParticleSystem::EllapsedTime);
			}
		}
		else
		{
			// to avoid warning in autocount mode
			//CParticleSystem::InsideSimLoop = false;
			for (CParticleSystem::TSpawnInfoVect::const_iterator it = spawns.SpawnInfos.begin(); it !=endIt; ++it)
			{
				sint32 insertionIndex = newElement(*it, false, CParticleSystem::EllapsedTime);
				#ifdef NL_DEBUG
					nlassert(insertionIndex != -1);
				#endif
				if (_Time[insertionIndex] >= 1.f)
				{
					#ifdef NL_DEBUG
						nlassert(CParticleSystem::_ParticleRemoveListIndex[insertionIndex] == -1);
					#endif
					CParticleSystem::_ParticleToRemove.push_back(insertionIndex);
					#ifdef NL_DEBUG
						nlassert(CParticleSystem::_ParticleToRemove.size() <= _Size);
					#endif
					CParticleSystem::_ParticleRemoveListIndex[insertionIndex] = (sint)CParticleSystem::_ParticleToRemove.size() - 1;
				}
			}
			//CParticleSystem::InsideSimLoop = true;
		}
		spawns.SpawnInfos.clear();
	#ifdef NL_DEBUG
		CParticleSystem::InsideNewElementsLoop = false;
	#endif
}

/// ***************************************************************************************
bool CPSLocated::computeBBox(NLMISC::CAABBox &box) const
{
	NL_PS_FUNC(CPSLocated_computeBBox)
	CHECK_PS_INTEGRITY
	if (!_Size) return false; // something to compute ?


	TLocatedBoundCont::const_iterator it;
	TPSAttribVector::const_iterator it2;

	// check whether any object bound to us need a bbox

	for (it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->doesProduceBBox())
		{
			break;
		}
	}

	if (it == _LocatedBoundCont.end())
	{
		return false;
	}

	CVector min = _Pos[0], max = _Pos[0];

	for (it2 = _Pos.begin(); it2 != _Pos.end(); ++ it2)
	{
		const CVector &v = (*it2);
		min.minof(min, v);
		max.maxof(max, v);
	}

	box.setMinMax(min, max);

	// we've considered that located had no extent in space
	// now, we must call each objects that are bound to the located in order
	// to complete the bbox if they have no null extent

	NLMISC::CAABBox tmpBox, startBox = box;

	for (it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->doesProduceBBox())
		{
			tmpBox = startBox;
			if ((*it)->completeBBox(tmpBox))
			{
				box = NLMISC::CAABBox::computeAABBoxUnion(tmpBox, box);
			}
		}
	}
	CHECK_PS_INTEGRITY
	return true;
}


/// Setup the driver model matrix. It is set accordingly to the basis used for rendering
/// ***************************************************************************************
void CPSLocated::setupDriverModelMatrix(void)
{
	NL_PS_FUNC(CPSLocated_setupDriverModelMatrix)
	CHECK_PS_INTEGRITY
	getDriver()->setupModelMatrix(getLocalToWorldMatrix());
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::queryCollisionInfo(void)
{
	NL_PS_FUNC(CPSLocated_queryCollisionInfo)
	CHECK_PS_INTEGRITY
	if (_CollisionInfoNbRef)
	{
		++ _CollisionInfoNbRef;
	}
	else
	{
		_CollisionNextPos = new TPSAttribVector;
		_CollisionInfoNbRef = 1;
		_CollisionNextPos ->resize(_MaxSize);
		for(uint k = 0; k < _Size; ++k)
		{
			_CollisionNextPos->insert();
		}
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::releaseCollisionInfo(void)
{
	NL_PS_FUNC(CPSLocated_releaseCollisionInfo)
	CHECK_PS_INTEGRITY
	nlassert(_CollisionInfoNbRef); // check whether queryCollisionInfo was called
									// so the number of refs must not = 0
    --_CollisionInfoNbRef;
	if (_CollisionInfoNbRef == 0)
	{
		delete _CollisionNextPos;
		_CollisionNextPos = NULL;
	}
	CHECK_PS_INTEGRITY
}



/// ***************************************************************************************
void CPSLocated::registerIntegrableForce(CPSForce *f)
{
	NL_PS_FUNC(CPSLocated_registerIntegrableForce)
	CHECK_PS_INTEGRITY
	nlassert(std::find(_IntegrableForces.begin(), _IntegrableForces.end(), f) == _IntegrableForces.end()); // force registered twice
	_IntegrableForces.push_back(f);
	if (getMatrixMode() != f->getOwner()->getMatrixMode())
	{
		++_NumIntegrableForceWithDifferentBasis;
		releaseParametricInfos();
	}
	CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
void CPSLocated::unregisterIntegrableForce(CPSForce *f)
{
	NL_PS_FUNC(CPSLocated_unregisterIntegrableForce)
	CHECK_PS_INTEGRITY
	nlassert(f->getOwner()); // f must be attached to a located
	CPSVector<CPSForce *>::V::iterator it = std::find(_IntegrableForces.begin(), _IntegrableForces.end(), f);
	nlassert(it != _IntegrableForces.end() );
	_IntegrableForces.erase(it);
	if (getMatrixMode() != f->getOwner()->getMatrixMode())
	{
		--_NumIntegrableForceWithDifferentBasis;
	}
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::addNonIntegrableForceRef(void)
{
	NL_PS_FUNC(CPSLocated_addNonIntegrableForceRef)
	CHECK_PS_INTEGRITY
	++_NonIntegrableForceNbRefs;
	releaseParametricInfos();
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
void CPSLocated::releaseNonIntegrableForceRef(void)
{
	NL_PS_FUNC(CPSLocated_releaseNonIntegrableForceRef)
	CHECK_PS_INTEGRITY
	nlassert(_NonIntegrableForceNbRefs != 0);
	--_NonIntegrableForceNbRefs;
	CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
void CPSLocated::integrableForceBasisChanged(TPSMatrixMode matrixMode)
{
	NL_PS_FUNC(CPSLocated_integrableForceBasisChanged)
	CHECK_PS_INTEGRITY
	if (getMatrixMode() != matrixMode)
	{
		++_NumIntegrableForceWithDifferentBasis;
		releaseParametricInfos();
	}
	else
	{
		--_NumIntegrableForceWithDifferentBasis;
	}
	CHECK_PS_INTEGRITY
}


/// ***************************************************************************************
CPSLocatedBindable *CPSLocated::unbind(uint index)
{
	NL_PS_FUNC(CPSLocated_unbind)
	CHECK_PS_INTEGRITY
	nlassert(index < _LocatedBoundCont.size());
	CPSLocatedBindable *lb = _LocatedBoundCont[index];
	lb->setOwner(NULL);
	_LocatedBoundCont.erase(_LocatedBoundCont.begin() + index);
	return lb;
	CHECK_PS_INTEGRITY
}

/// ***************************************************************************************
bool CPSLocated::isBound(const CPSLocatedBindable *lb) const
{
	NL_PS_FUNC(CPSLocated_isBound)
	CHECK_PS_INTEGRITY
	TLocatedBoundCont::const_iterator it = std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), lb);
	return it != _LocatedBoundCont.end();
}

/// ***************************************************************************************
uint CPSLocated::getIndexOf(const CPSLocatedBindable *lb) const
{
	NL_PS_FUNC(CPSLocated_getIndexOf)
	CHECK_PS_INTEGRITY
	for(uint k = 0; k < _LocatedBoundCont.size(); ++k)
	{
		if (_LocatedBoundCont[k] == lb) return k;
	}
	nlassert(0);
	return 0;
}



///////////////////////////////////////
// CPSLocatedBindable implementation //
///////////////////////////////////////


/// ***************************************************************************************
CPSLocatedBindable::CPSLocatedBindable() : _Owner(NULL), _ExternID(0), _LOD(PSLod1n2), _Active(true)
{
	NL_PS_FUNC(CPSLocatedBindable_CPSLocatedBindable)
	_Owner = NULL;
}

/// ***************************************************************************************
void CPSLocatedBindable::setOwner(CPSLocated *psl)
{
	NL_PS_FUNC(CPSLocatedBindable_setOwner)
	if (psl == _Owner) return;
	if (psl == NULL)
	{
		releaseAllRef();
		if (_Owner)
		{
			// empty this located bindable. Need to be empty if it must be rebound to another located.
			for (uint k = 0; k < _Owner->getSize(); ++k)
			{
				deleteElement(0);
			}
		}
	}
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->releaseRefForUserSysCoordInfo(getUserMatrixUsageCount());
	}
	_Owner = psl;
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->addRefForUserSysCoordInfo(getUserMatrixUsageCount());
	}
}

/// ***************************************************************************************
void CPSLocatedBindable::finalize(void)
{
	NL_PS_FUNC(CPSLocatedBindable_finalize)
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->releaseRefForUserSysCoordInfo(getUserMatrixUsageCount());
	}
}

/// ***************************************************************************************
CPSLocatedBindable::~CPSLocatedBindable()
{
	NL_PS_FUNC(CPSLocatedBindable_CPSLocatedBindableDtor)
	if (_ExternID)
	{
		if (_Owner && _Owner->getOwner())
		{
			_Owner->getOwner()->unregisterLocatedBindableExternID(this);
		}
	}
}

/// ***************************************************************************************
void CPSLocatedBindable::notifyTargetRemoved(CPSLocated *ptr)
{
	NL_PS_FUNC(CPSLocatedBindable_notifyTargetRemoved)
	ptr->unregisterDtorObserver(this);
}

/// ***************************************************************************************
void CPSLocatedBindable::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSLocatedBindable_IStream )
	sint ver = f.serialVersion(4);
	f.serialPtr(_Owner);
	if (ver > 1) f.serialEnum(_LOD);
	if (ver > 2)
	{
		if (f.isReading() && !CParticleSystem::getSerializeIdentifierFlag())
		{
			// just skip the name
			sint32 len;
			f.serial(len);
			f.seek(len, NLMISC::IStream::current);
		}
		else
		{
			f.serial(_Name);
		}
	}
	if (ver > 3)
	{
		if (f.isReading())
		{
			uint32 id;
			f.serial(id);
			setExternID(id);
		}
		else
		{
			f.serial(_ExternID);
		}
	}

}

/// ***************************************************************************************
void CPSLocatedBindable::displayIcon2d(const CVector tab[], uint nbSegs, float scale)
{
	NL_PS_FUNC(CPSLocatedBindable_displayIcon2d)
	uint32 size = _Owner->getSize();
	if (!size) return;
	setupDriverModelMatrix();

	const CVector I = computeI();
	const CVector K = computeK();

	static std::vector<NLMISC::CLine> lines;

	lines.clear();

	// ugly slow code, but not for runtime
	for (uint  k = 0; k < size; ++k)
	{
		// center of the current particle
		const CVector p = _Owner->getPos()[k];



		for (uint l = 0; l < nbSegs; ++l)
		{
			NLMISC::CLine li;
			li.V0 = p + scale * (tab[l << 1].x * I + tab[l << 1].y * K);
			li.V1 = p + scale * (tab[(l << 1) + 1].x * I + tab[(l << 1) + 1].y * K);
			lines.push_back(li);
		}

		CMaterial mat;

		mat.setBlendFunc(CMaterial::one, CMaterial::one);
		mat.setZWrite(false);
		mat.setLighting(false);
		mat.setBlend(true);
		mat.setZFunc(CMaterial::less);



		CPSLocated *loc;
		uint32 index;
		CPSLocatedBindable *lb;
		_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);

		mat.setColor((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));


		CDRU::drawLinesUnlit(lines, mat, *getDriver() );
	}

}

/// ***************************************************************************************
CFontManager *CPSLocatedBindable::getFontManager(void)
{
	NL_PS_FUNC(CPSLocatedBindable_getFontManager)
	nlassert(_Owner);
	return _Owner->getFontManager();
}

/// ***************************************************************************************
 /// Shortcut to get the font manager if one was set (const version)
const CFontManager *CPSLocatedBindable::getFontManager(void) const
{
	NL_PS_FUNC(CPSLocatedBindable_getFontManager)
	nlassert(_Owner);
	return _Owner->getFontManager();
}


/// ***************************************************************************************
// Shortcut to get the matrix of the system
 const NLMISC::CMatrix &CPSLocatedBindable::getSysMat(void) const
{
	NL_PS_FUNC( CPSLocatedBindable_getSysMat)
	nlassert(_Owner);
	return _Owner->getOwner()->getSysMat();
}

 /// ***************************************************************************************
/// shortcut to get the inverted matrix of the system
const NLMISC::CMatrix &CPSLocatedBindable::getInvertedSysMat(void) const
{
	NL_PS_FUNC(CPSLocatedBindable_getInvertedSysMat)
	nlassert(_Owner);
		return _Owner->getOwner()->getInvertedSysMat();

}

/// ***************************************************************************************
/// shortcut to get the view matrix
const NLMISC::CMatrix &CPSLocatedBindable::getViewMat(void) const
{
	NL_PS_FUNC(CPSLocatedBindable_getViewMat)
	nlassert(_Owner);
	return _Owner->getOwner()->getViewMat();
}


/// ***************************************************************************************
/// shortcut to get the inverted view matrix
const NLMISC::CMatrix &CPSLocatedBindable::getInvertedViewMat(void) const
{
	NL_PS_FUNC(CPSLocatedBindable_getInvertedViewMat)
	nlassert(_Owner);
	return _Owner->getOwner()->getInvertedViewMat();
}

/// ***************************************************************************************
/// shortcut to setup the model matrix (system basis or world basis)
void CPSLocatedBindable::setupDriverModelMatrix(void)
{
	NL_PS_FUNC(CPSLocatedBindable_setupDriverModelMatrix)
	nlassert(_Owner);
	_Owner->setupDriverModelMatrix();
}

/// ***************************************************************************************
void	CPSLocatedBindable::setExternID(uint32 id)
{
	NL_PS_FUNC(CPSLocatedBindable_setExternID)
	if (id == _ExternID) return;
	CParticleSystem *ps = NULL;
	if (_Owner && _Owner->getOwner())
	{
		ps = _Owner->getOwner();
	}
	if (ps)
	{
		ps->unregisterLocatedBindableExternID(this);
		_ExternID = 0;
	}
	if (id != 0)
	{
		if (ps) ps->registerLocatedBindableExternID(id, this);
		_ExternID = id;
	}
}

/// ***************************************************************************************
void CPSLocatedBindable::releaseAllRef()
{
	NL_PS_FUNC(CPSLocatedBindable_releaseAllRef)
}





/////////////////////////////////////////////
// CPSTargetLocatedBindable implementation //
/////////////////////////////////////////////

/// ***************************************************************************************
void CPSTargetLocatedBindable::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSTargetLocatedBindable_serial)
	(void)f.serialVersion(1);
	f.serialPtr(_Owner);
	f.serial(_Name);
	if (f.isReading())
	{
		// delete previous attached bindables...
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{
			delete (*it);
		}
		_Targets.clear();
	}
	f.serialContPolyPtr(_Targets);
}


/// ***************************************************************************************
void CPSTargetLocatedBindable::attachTarget(CPSLocated *ptr)
{
	NL_PS_FUNC(CPSTargetLocatedBindable_attachTarget)

	// a target can't be shared between different particle systems
	#ifdef NL_DEBUG
	if (_Owner)
	{
		nlassert(_Owner->getOwner() == ptr->getOwner());
	}
	#endif

	// see whether this target has not been registered before
	nlassert(std::find(_Targets.begin(), _Targets.end(), ptr) == _Targets.end());
	_Targets.push_back(ptr);

	// we register us to be notified when the target disappear
	ptr->registerDtorObserver(this);
}


/// ***************************************************************************************
void CPSTargetLocatedBindable::notifyTargetRemoved(CPSLocated *ptr)
{
	NL_PS_FUNC(CPSTargetLocatedBindable_notifyTargetRemoved)
	TTargetCont::iterator it = std::find(_Targets.begin(), _Targets.end(), ptr);
	nlassert(it != _Targets.end());
	releaseTargetRsc(*it);
	_Targets.erase(it);

	CPSLocatedBindable::notifyTargetRemoved(ptr);
}



// dtor

/// ***************************************************************************************
void CPSTargetLocatedBindable::finalize(void)
{
	NL_PS_FUNC(CPSTargetLocatedBindable_finalize)
	/** Release the collisionInfos we've querried. We can't do it in the dtor, as calls to releaseTargetRsc wouldn't be polymorphics for derived class!
	  * And the behaviour of releaseTergetRsc is implemented in derived class
	  */
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{
		releaseTargetRsc(*it);
	}
	CPSLocatedBindable::finalize();
}

/// ***************************************************************************************
CPSTargetLocatedBindable::~CPSTargetLocatedBindable()
{
	NL_PS_FUNC(CPSTargetLocatedBindable_CPSTargetLocatedBindable)
	// we unregister to all the targets
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{
		(*it)->unregisterDtorObserver(this);
	}
}

/// ***************************************************************************************
void CPSTargetLocatedBindable::releaseRefTo(const CParticleSystemProcess *other)
{
	NL_PS_FUNC(CPSTargetLocatedBindable_releaseRefTo)
	TTargetCont::iterator it = std::find(_Targets.begin(), _Targets.end(), other);
	if (it == _Targets.end()) return;
	releaseTargetRsc(*it);
	(*it)->unregisterDtorObserver(this);
	_Targets.erase(it);
	nlassert(std::find(_Targets.begin(), _Targets.end(), other) == _Targets.end());
}

/// ***************************************************************************************
void CPSTargetLocatedBindable::releaseAllRef()
{
	NL_PS_FUNC(CPSTargetLocatedBindable_releaseAllRef)
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{
		releaseTargetRsc(*it);
		(*it)->unregisterDtorObserver(this);
	}
	_Targets.clear();
	CPSLocatedBindable::releaseAllRef();
}

/// ***************************************************************************************
uint CPSLocated::getUserMatrixUsageCount() const
{
	NL_PS_FUNC(CPSLocated_getUserMatrixUsageCount)
	uint count = 0;
	for(TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		count += (*it)->getUserMatrixUsageCount();
	}
	return count + CParticleSystemProcess::getUserMatrixUsageCount();
}

/// ***************************************************************************************
void CPSLocated::enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv)
{
	NL_PS_FUNC(CPSLocated_enumTexs)
	for(TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->enumTexs(dest, drv);
	}
}

/// ***************************************************************************************
void CPSLocated::setZBias(float value)
{
	NL_PS_FUNC(CPSLocated_setZBias)
	for(TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->setZBias(value);
	}
}

/// ***************************************************************************************
void CPSLocated::computeCollisions(uint firstInstanceIndex, const NLMISC::CVector *posBefore, const NLMISC::CVector *posAfter)
{
	NL_PS_FUNC(CPSLocated_computeCollisions)
	for(TDtorObserversVect::iterator it = _DtorObserversVect.begin(); it != _DtorObserversVect.end(); ++it)
	{
		if ((*it)->getType() == PSZone)
		{
			static_cast<CPSZone *>(*it)->computeCollisions(*this, firstInstanceIndex, posBefore, posAfter);
		}
	}
}

/// ***************************************************************************************
void CPSLocated::computeSpawns(uint firstInstanceIndex, bool includeEmitOnce)
{
	NL_PS_FUNC(CPSLocated_computeSpawns)
	nlassert(CParticleSystem::InsideSimLoop);
	for(TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if (!(*it)->isActive()) continue;
		if ((*it)->getType() == PSEmitter)
		{
			CPSEmitter *emit = static_cast<CPSEmitter *>(*it);
			emit->updateEmitTrigger();
			switch(emit->getEmissionType())
			{
				case CPSEmitter::regular:
					emit->computeSpawns(firstInstanceIndex);
				break;
				case CPSEmitter::once:
					// if we're at first frame, then do emit for each emitter
					nlassert(_Owner);
					if (_Owner->getSystemDate() == 0.f || includeEmitOnce)
					{
						// if first pass, then do the emit a single time
						// if firstInstanceIndex != 0 then we're dealing with newly created particles, so do the spawn too
						emit->doEmitOnce(firstInstanceIndex);
					}
				break;
				default:
					break;
			}
		}
	}
}

/// ***************************************************************************************
void CPSLocated::computeForces()
{
	NL_PS_FUNC(CPSLocated_computeForces)
	nlassert(CParticleSystem::InsideSimLoop);
	for(TDtorObserversVect::iterator it = _DtorObserversVect.begin(); it != _DtorObserversVect.end(); ++it)
	{
		if ((*it)->getType() == PSForce)
		{
			CPSForce *force = static_cast<CPSForce *>(*it);
			force->computeForces(*this);
		}
	}
}

/// ***************************************************************************************
void CPSCollisionInfo::update(const CPSCollisionInfo &other)
{
	NL_PS_FUNC(CPSCollisionInfo_update)
	if (Dist == -1)
	{
		// link collision in the global list of active collisions
		Next = CPSLocated::_FirstCollision;
		CPSLocated::_FirstCollision = this;
		Dist = other.Dist;
		NewPos = other.NewPos;
		NewSpeed = other.NewSpeed;
		CollisionZone = other.CollisionZone;
	}
	else if (other.Dist < Dist) // is the new collision better (e.g it happens sooner) ?
	{
		Dist = other.Dist;
		NewPos = other.NewPos;
		NewSpeed = other.NewSpeed;
		CollisionZone = other.CollisionZone;
	}
}

/// ***************************************************************************************
void CPSLocated::checkLife() const
{
	NL_PS_FUNC(CPSLocated_checkLife)
	if (!getLastForever())
	{
		for(uint k = 0; k < getSize(); ++k)
		{
			nlassert(getTime()[k] >= 0.f);
			nlassert(getTime()[k] <= 1.f);
		}
	}
}

/// ***************************************************************************************
void CPSLocated::onShow(bool shown)
{
	for(TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->onShow(shown);
	}
}


} // NL3D
