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

#include "nel/3d/ps_emitter.h"
#include "nel/3d/material.h"
#include "nel/misc/line.h"
#include "nel/3d/dru.h"
#include "nel/3d/particle_system.h"

namespace NL3D {




static const uint  EMITTER_BUFF_SIZE = 512;			   // number of emitter to be processed at once
static const float EMIT_PERIOD_THRESHOLD = 1.f / 75.f; // assuming the same behaviour than with a 75 hz rendering
bool CPSEmitter::_BypassEmitOnDeath = false;


//////////////////////
// STATIC FUNCTIONS //
//////////////////////
/** In an arrey of float, all value that are 0.f are replaced by EMIT_PERIOD_THRESHOLD
  * A period of 0 is allowed for emitter and means "emit at each frame"
  * This is deprecated now, and this helps to avoid that behaviour
  */
static void replaceNullPeriodsByThreshold(float *tab, uint numElem)
{
	NL_PS_FUNC(replaceNullPeriodsByThreshold)
	const float *endTab = tab + numElem;
	while (tab != endTab)
	{
		if (*tab == 0.f) *tab = EMIT_PERIOD_THRESHOLD;
		++ tab;
	}
}

///////////////////////////////
// CPSEmitter implementation //
///////////////////////////////
CPSEmitter::CPSEmitter() : _EmittedType(NULL),
						   _SpeedInheritanceFactor(0.f),
						   _EmissionType(regular),
						   _Period(0.02f),
						   _PeriodScheme(NULL),
						   _GenNb(1),
						   _GenNbScheme(NULL),
						   _EmitDelay(0),
						   _MaxEmissionCount(0),
						   _SpeedBasisEmission(false),
						   _ConsistentEmission(true),
						   _BypassAutoLOD(false),
						   _UserMatrixModeForEmissionDirection(false),
						   _EmitTrigger(false),
						   _UserDirectionMatrixMode(PSFXWorldMatrix)
{
	NL_PS_FUNC(CPSEmitter_CPSEmitter)
}


///==========================================================================
CPSEmitter::~CPSEmitter()
{
	NL_PS_FUNC(CPSEmitter_CPSEmitterDtor)
	delete _PeriodScheme;
	delete _GenNbScheme;
	// if a located is emitted, unregister us as an observer
	if (_EmittedType)
	{
		_EmittedType->unregisterDtorObserver(this);
	}
}

///==========================================================================
void CPSEmitter::releaseRefTo(const CParticleSystemProcess *other)
{
	NL_PS_FUNC(CPSEmitter_releaseRefTo)
	if (_EmittedType == other)
	{
		setEmittedType(NULL);
	}
}

void CPSEmitter::releaseAllRef()
{
	NL_PS_FUNC(CPSEmitter_releaseAllRef)
	setEmittedType(NULL);
}


///==========================================================================
void CPSEmitter::setOwner(CPSLocated *psl)
{
	NL_PS_FUNC(CPSEmitter_setOwner)
	CPSLocatedBindable::setOwner(psl);
	updateMaxCountVect();
}


///==========================================================================
inline void CPSEmitter::processEmit(uint32 index, sint nbToGenerate)
{
	NL_PS_FUNC(CPSEmitter_processEmit)
	NLMISC::CVector speed, pos;
	nlassert(_Owner);
	if (!_SpeedBasisEmission)
	{
		if (_SpeedInheritanceFactor == 0.f)
		{
			if (!_UserMatrixModeForEmissionDirection)
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(_Owner->getPos()[index], index, pos, speed);
					_EmittedType->postNewElement(pos, speed, *this->_Owner, index, _Owner->getMatrixMode(), 0.f);
				}
			}
			else
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(_Owner->getPos()[index], index, pos, speed);
					_EmittedType->postNewElement(pos, speed, *this->_Owner, index, _UserDirectionMatrixMode, 0.f);
				}
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->postNewElement(pos, speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], *this->_Owner, 0, _Owner->getMatrixMode(), 0.f);
			}
		}
	}
	else
	{
		NLMISC::CMatrix m;
		CPSUtil::buildSchmidtBasis(_Owner->getSpeed()[index], m);
		if (_SpeedInheritanceFactor == 0.f)
		{
			while (nbToGenerate > 0)
			{
				nbToGenerate --;
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->postNewElement(pos, m * speed, *this->_Owner, index, _Owner->getMatrixMode(), 0.f);
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->postNewElement(pos, m * speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], *this->_Owner, index, _Owner->getMatrixMode(), 0.f);
			}
		}
	}
}


///==========================================================================
void CPSEmitter::processEmitOutsideSimLoop(uint32 index,sint nbToGenerate)
{
	NL_PS_FUNC(CPSEmitter_processEmitOutsideSimLoop)
	NLMISC::CVector speed, pos;
	nlassert(_Owner);
	if (!_SpeedBasisEmission)
	{
		if (_SpeedInheritanceFactor == 0.f)
		{
			if (!_UserMatrixModeForEmissionDirection)
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(_Owner->getPos()[index], index, pos, speed);
					_EmittedType->newElement(pos, speed, this->_Owner, index, _Owner->getMatrixMode(), true);
				}
			}
			else
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(_Owner->getPos()[index], index, pos, speed);
					_EmittedType->newElement(pos, speed, this->_Owner, index, _UserDirectionMatrixMode);
				}
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->newElement(pos, speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this->_Owner, 0, _Owner->getMatrixMode(), true);
			}
		}
	}
	else
	{
		NLMISC::CMatrix m;
		CPSUtil::buildSchmidtBasis(_Owner->getSpeed()[index], m);
		if (_SpeedInheritanceFactor == 0.f)
		{
			while (nbToGenerate > 0)
			{
				nbToGenerate --;
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->newElement(pos, m * speed, this->_Owner, index, _Owner->getMatrixMode(), true);
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->newElement(pos, m * speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this->_Owner, index, _Owner->getMatrixMode(), true);
			}
		}
	}
}

///==========================================================================
inline void CPSEmitter::processEmitConsistent(const NLMISC::CVector &emitterPos,
											  uint32 index,
											  sint nbToGenerate,
											  TAnimationTime deltaT)
{
	NL_PS_FUNC(CPSEmitter_processEmitConsistent)
	static NLMISC::CVector speed, pos; /// speed and pos of emittee
	nlassert(_Owner);
	if (!_SpeedBasisEmission)
	{
		if (_SpeedInheritanceFactor == 0.f)
		{
			if (!_UserMatrixModeForEmissionDirection)
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(emitterPos, index, pos, speed);
					_EmittedType->postNewElement(pos, speed, *this->_Owner, index, _Owner->getMatrixMode(), deltaT);
				}
			}
			else
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(emitterPos, index, pos, speed);
					_EmittedType->postNewElement(pos, speed, *this->_Owner, index, _UserDirectionMatrixMode, deltaT);
				}
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(emitterPos, index, pos, speed);
				_EmittedType->postNewElement(pos, speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], *this->_Owner, index, _Owner->getMatrixMode(), deltaT);
			}
		}
	}
	else
	{
		NLMISC::CMatrix m;
		CPSUtil::buildSchmidtBasis(_Owner->getSpeed()[index], m);
		if (_SpeedInheritanceFactor == 0.f)
		{
			while (nbToGenerate > 0)
			{
				nbToGenerate --;
				emit(emitterPos, index, pos, speed);
				_EmittedType->postNewElement(pos, m * speed, *this->_Owner, index, _Owner->getMatrixMode(), deltaT);
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(emitterPos, index, pos, speed);
				_EmittedType->postNewElement(pos, m * speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], *this->_Owner, index, _Owner->getMatrixMode(), deltaT);
			}
		}
	}
}


///==========================================================================
bool	CPSEmitter::setEmissionType(TEmissionType freqType)
{
	NL_PS_FUNC(CPSEmitter_setEmissionType)
	if (_Owner && _Owner->getOwner())
	{
		CParticleSystem *ps = _Owner->getOwner();
		if (ps->getBypassMaxNumIntegrationSteps())
		{
			if (!_Owner)
			{
				nlwarning("<CPSEmitter::setEmissionType> The emitter should be inserted in a CPSLocated instance");
				nlassert(0);
			}
			// check if the new value is valid
			TEmissionType oldType = _EmissionType;
			_EmissionType = freqType;
			if (testEmitForever() == true)
			{
				_EmissionType = oldType;
				std::string mess = "<CPSEmitter::setEmissionType> can't set emission type to '" +
					               NLMISC::toString(freqType) +
								  "' with the current configuration : the system has been flagged with \
								   'BypassMaxNumIntegrationSteps', and should have a finite duration.  \
								   The flag is not set";
				nlwarning(mess.c_str());
				return false;

			}
		}
		ps->systemDurationChanged();
	}
	_EmissionType = freqType;
	return true;
}

///==========================================================================
bool CPSEmitter::setEmittedType(CPSLocated *et)
{
	NL_PS_FUNC(CPSEmitter_setEmittedType)
	if (_EmittedType)
	{
		_EmittedType->unregisterDtorObserver(this);
	}
	if (et)
	{
		et->registerDtorObserver(this);
	}
	CPSLocated *oldType = _EmittedType;
	_EmittedType = et;
	if (_Owner && _Owner->getOwner())
	{
		CParticleSystem *ps = _Owner->getOwner();
		if (_EmittedType)
		{
			bool ok = true;
			if (ps->getBypassMaxNumIntegrationSteps())
			{
				ok =  ps->canFinish();
			}
			else
			{
				ok = !ps->hasLoop();
			}
			if (!ok)
			{
				setEmittedType(oldType);
				nlwarning("<CPSLocated::setEmittedType> Can't set new emitted type : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. New emitted type is not set");
				return false;
			}
		}
		ps->systemDurationChanged();
	}
	return true;
}

///==========================================================================
void CPSEmitter::notifyTargetRemoved(CPSLocated *ptr)
{
	NL_PS_FUNC(CPSEmitter_notifyTargetRemoved)
	nlassert(ptr == _EmittedType && _EmittedType);
	setEmittedType(NULL);
}

///==========================================================================
void CPSEmitter::setPeriod(float period)
{
	NL_PS_FUNC(CPSEmitter_setPeriod)
	if (_PeriodScheme)
	{
		delete _PeriodScheme;
		_PeriodScheme = NULL;
	}
	_Period = period;
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->systemDurationChanged();
	}
}

///==========================================================================
void CPSEmitter::setPeriodScheme(CPSAttribMaker<float> *scheme)
{
	NL_PS_FUNC(CPSEmitter_setPeriodScheme)
	delete _PeriodScheme;
	_PeriodScheme = scheme;
	if (_Owner && scheme->hasMemory()) scheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->systemDurationChanged();
	}
}

///==========================================================================
void CPSEmitter::setGenNb(uint32 genNb)
{
	NL_PS_FUNC(CPSEmitter_setGenNb)
	if (_GenNbScheme)
	{
		delete _GenNbScheme;
		_GenNbScheme = NULL;
	}
	_GenNb = genNb;
}

///==========================================================================
void CPSEmitter::setGenNbScheme(CPSAttribMaker<uint32> *scheme)
{
	NL_PS_FUNC(CPSEmitter_setGenNbScheme)
	delete _GenNbScheme;
	_GenNbScheme = scheme;
	if (_Owner && scheme->hasMemory()) scheme->resize(_Owner->getMaxSize(), _Owner->getSize());
}

///==========================================================================
void CPSEmitter::showTool(void)
{
	NL_PS_FUNC(CPSEmitter_showTool)
	uint32 size = _Owner->getSize();
	if (!size) return;
	setupDriverModelMatrix();

	const CVector I = computeI();
	const CVector K = computeK();

	// ugly slow code, but not for runtime
	for (uint  k = 0; k < size; ++k)
	{
		// center of the current particle
		const CVector p = _Owner->getPos()[k];
		const float sSize =0.1f;
		std::vector<NLMISC::CLine> lines;
		NLMISC::CLine l;
		l.V0 = p - sSize * I; l.V1 =  p + sSize * I; lines.push_back(l);
		l.V0 = p - sSize * K; l.V1 =  p + sSize * K; lines.push_back(l);
		l.V0 = p - sSize * (I + K); l.V1 = p + sSize * (I + K); lines.push_back(l);
		l.V0 = p - sSize * (I - K); l.V1 = p + sSize * (I - K); lines.push_back(l);

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

///==========================================================================
void CPSEmitter::singleEmit(uint32 index, uint quantity)
{
	NL_PS_FUNC(CPSEmitter_singleEmit)
	nlassert(_Owner);
	const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner,0) : _GenNb;
	processEmitOutsideSimLoop(index, quantity * nbToGenerate);
}



///==========================================================================
void CPSEmitter::processRegularEmissionWithNoLOD(uint firstInstanceIndex)
{
	NL_PS_FUNC(CPSEmitter_processRegularEmissionWithNoLOD)
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	//
	const bool emitThreshold = _Owner->getOwner()->isEmitThresholdEnabled();
	//
	const uint size = _Owner->getSize();
	nlassert(firstInstanceIndex < size);
	uint leftToDo = size - firstInstanceIndex, toProcess;
	float emitPeriod[EMITTER_BUFF_SIZE];
	const float *currEmitPeriod;
	uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0;
	sint32 nbToGenerate;

	TPSAttribTime::iterator phaseIt = _Phase.begin() + firstInstanceIndex, endPhaseIt;
	TPSAttribUInt8::iterator numEmitIt = _NumEmission.begin() + firstInstanceIndex;

	// we don't use an iterator here
	// because it could be invalidated if size change (a located could generate itself)
	do
	{
		toProcess = leftToDo < EMITTER_BUFF_SIZE ? leftToDo : EMITTER_BUFF_SIZE;


		if (_PeriodScheme)
		{
			// compute period
			// NB : we ask to clamp entry because life counter of emitter are incremented, then spawn is called, and only after that dead emitters are removed
			//      so we may have a life counter that is > to 1
			currEmitPeriod = (float *) (_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess, true, 1 << 16, true));
			if (emitThreshold)
			{

				/** Test if 'make' filled our buffer. If this is not the case, we assume that values where precomputed, and that
				  * all null period have already been replaced by the threshold
				  */
				if (currEmitPeriod == emitPeriod)
				{
					// if there possibility to have 0 in the scheme ?
					if (_PeriodScheme->getMinValue() <= 0.f && _PeriodScheme->getMaxValue() >= 0.f)
					{
						replaceNullPeriodsByThreshold(emitPeriod, toProcess);
					}
				}
			}
		}
		else
		{
			if (_Period != 0.f || !emitThreshold)
			{
				currEmitPeriod = &_Period;
			}
			else
			{
				currEmitPeriod = &EMIT_PERIOD_THRESHOLD;
			}
		}

		endPhaseIt = phaseIt + toProcess;

		if (_MaxEmissionCount == 0) // no emission count limit
		{
			/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do
				{
					*phaseIt += CParticleSystem::EllapsedTime;
					if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							*phaseIt -= ::floorf(*phaseIt / *currEmitPeriod) * *currEmitPeriod;
						}
						const uint32 k = (uint32)(phaseIt - _Phase.begin());
						nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
						processEmit(k, nbToGenerate);
					}

					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do
				{
					*phaseIt += CParticleSystem::EllapsedTime;
					if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							*phaseIt -= ::floorf((*phaseIt - _EmitDelay)  / *currEmitPeriod) * *currEmitPeriod;
						}
						const uint32 k = (uint32)(phaseIt - _Phase.begin());
						nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
						processEmit(k, nbToGenerate);
					}

					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
		}
		else // there's an emission count limit
		{
				/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do
				{
					if (*numEmitIt < _MaxEmissionCount)
					{
						*phaseIt += CParticleSystem::EllapsedTime;
						if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								*phaseIt -= ::floorf(*phaseIt / *currEmitPeriod) * *currEmitPeriod;
							}
							const uint32 k = (uint32)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
							processEmit(k, nbToGenerate);
							++*numEmitIt;
						}
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++ numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do
				{
					if (*numEmitIt < _MaxEmissionCount)
					{
						*phaseIt += CParticleSystem::EllapsedTime;
						if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								*phaseIt -= ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod) * *currEmitPeriod;
							}
							const uint32 k = (uint32)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
							processEmit(k, nbToGenerate);
							++*numEmitIt;
						}
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
		}

		leftToDo -= toProcess;
	}
	while (leftToDo);
}


///==========================================================================
void CPSEmitter::processRegularEmission(uint firstInstanceIndex, float emitLOD)
{
	NL_PS_FUNC(CPSEmitter_processRegularEmission)
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	//
	const bool emitThreshold = _Owner->getOwner()->isEmitThresholdEnabled();
	//
	const uint size = _Owner->getSize();
	nlassert(firstInstanceIndex < size);
	uint leftToDo = size - firstInstanceIndex, toProcess;
	float emitPeriod[EMITTER_BUFF_SIZE];
	const float *currEmitPeriod;
	uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0;
	sint32 nbToGenerate;

	TPSAttribTime::iterator phaseIt = _Phase.begin() + firstInstanceIndex, endPhaseIt;
	TPSAttribUInt8::iterator numEmitIt = _NumEmission.begin() + firstInstanceIndex;

	float ellapsedTimeLOD = emitLOD * CParticleSystem::EllapsedTime;
	uint maxEmissionCountLOD = (uint8) (_MaxEmissionCount * emitLOD);
	maxEmissionCountLOD = std::max(1u, maxEmissionCountLOD);

	// we don't use an iterator here
	// because it could be invalidated if size change (a located could generate itself)
	do
	{
		toProcess = leftToDo < EMITTER_BUFF_SIZE ? leftToDo : EMITTER_BUFF_SIZE;


		if (_PeriodScheme)
		{
			// compute period
			// NB : we ask to clamp entry because life counter of emitter are incremented, then spawn is called, and only after that dead emitters are removed
			//      so we may have a life counter that is > to 1
			currEmitPeriod = (float *) (_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess, true, 1 << 16, true));
			if (emitThreshold)
			{

				/** Test if 'make' filled our buffer. If this is not the case, we assume that values where precomputed, and that
				  * all null period have already been replaced by the threshold
				  */
				if (currEmitPeriod == emitPeriod)
				{
					// if there possibility to have 0 in the scheme ?
					if (_PeriodScheme->getMinValue() <= 0.f && _PeriodScheme->getMaxValue() >= 0.f)
					{
						replaceNullPeriodsByThreshold(emitPeriod, toProcess);
					}
				}
			}
		}
		else
		{
			if (_Period != 0.f || !emitThreshold)
			{
				currEmitPeriod = &_Period;
			}
			else
			{
				currEmitPeriod = &EMIT_PERIOD_THRESHOLD;
			}
		}

		endPhaseIt = phaseIt + toProcess;

		if (_MaxEmissionCount == 0) // no emission count limit
		{
			/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do
				{
					*phaseIt += ellapsedTimeLOD;
					if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							*phaseIt -= ::floorf(*phaseIt / *currEmitPeriod) * *currEmitPeriod;
						}
						const uint32 k = (uint32)(phaseIt - _Phase.begin());
						nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
						if (nbToGenerate)
						{
							nbToGenerate = (sint32) (emitLOD * nbToGenerate);
							if (!nbToGenerate) nbToGenerate = 1;
							processEmit(k, nbToGenerate);
						}
					}

					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do
				{
					if (*phaseIt < _EmitDelay)
					{
						*phaseIt += CParticleSystem::EllapsedTime;
						if (*phaseIt < _EmitDelay)
						{
							++phaseIt;
							currEmitPeriod += currEmitPeriodPtrInc;
							continue;
						}
						else
						{
							*phaseIt = 	(*phaseIt - _EmitDelay) * emitLOD + _EmitDelay;
						}
					}
					else
					{
						*phaseIt += ellapsedTimeLOD;
					}

					if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							*phaseIt -= ::floorf((*phaseIt - _EmitDelay)  / *currEmitPeriod) * *currEmitPeriod;
						}
						const uint32 k = (uint32)(phaseIt - _Phase.begin());
						nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
						if (nbToGenerate)
						{
							nbToGenerate = (sint32) (emitLOD * nbToGenerate);
							if (!nbToGenerate) nbToGenerate = 1;
							processEmit(k, nbToGenerate);
						}
					}

					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
		}
		else // there's an emission count limit
		{
				/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do
				{
					if (*numEmitIt < maxEmissionCountLOD)
					{
						*phaseIt += ellapsedTimeLOD;
						if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								*phaseIt -= ::floorf(*phaseIt / *currEmitPeriod) * *currEmitPeriod;
							}
							const uint32 k = (uint32)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
							if (nbToGenerate)
							{
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								processEmit(k, nbToGenerate);
							}
							++*numEmitIt;
						}
					}
					else
					{
						*numEmitIt = _MaxEmissionCount;
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++ numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do
				{
					if (*numEmitIt < maxEmissionCountLOD)
					{
						if (*phaseIt < _EmitDelay)
						{
							*phaseIt += CParticleSystem::EllapsedTime;
							if (*phaseIt < _EmitDelay)
							{
								++phaseIt;
								currEmitPeriod += currEmitPeriodPtrInc;
								++numEmitIt;
								currEmitPeriod += currEmitPeriodPtrInc;
								continue;
							}
							else
							{
								*phaseIt = 	(*phaseIt - _EmitDelay) * emitLOD + _EmitDelay;
							}
						}
						else
						{
							*phaseIt += ellapsedTimeLOD;
						}

						if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								*phaseIt -= ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod) * *currEmitPeriod;
							}
							const uint32 k = (uint32)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
							if (nbToGenerate)
							{
								nbToGenerate = (sint32) (nbToGenerate * emitLOD);
								if (!nbToGenerate) nbToGenerate = 1;
								processEmit(k, nbToGenerate);
							}
							++*numEmitIt;
						}
					}
					else
					{
						*numEmitIt = _MaxEmissionCount;
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
		}

		leftToDo -= toProcess;
	}
	while (leftToDo);
}

/// private : generate the various position of an emitter in the given tab for the given slice of time,
//  depending on whether its motion is parametric or incremental. This is used to create emittees at the right position

static
#ifndef NL_DEBUG
	inline
#endif
uint GenEmitterPositions(CPSLocated *emitter,
									   CPSLocated *emittee,
									   uint emitterIndex,
									   uint numStep,
									   TAnimationTime deltaT, /* fraction of time needed to reach the first emission */
									   TAnimationTime step,
									   std::vector<NLMISC::CVector> &dest
									  )
{
	NL_PS_FUNC(GenEmitterPositions)
	const uint toProcess = std::max(1U, std::min(numStep, (uint) emittee->getMaxSize()));
	dest.resize(toProcess);


	if (!emitter->isParametricMotionEnabled()) // standard case : take current pos and integrate
	{
		if (toProcess == 1) // only one emission -> takes current pos
		{
			dest[0] = emitter->getPos()[emitterIndex] - deltaT * emitter->getSpeed()[emitterIndex];
		}
		else
		{
			std::vector<NLMISC::CVector>::iterator outIt = dest.end();
			std::vector<NLMISC::CVector>::iterator endIt = dest.begin();
			NLMISC::CVector pos = emitter->getPos()[emitterIndex] - deltaT * emitter->getSpeed()[emitterIndex];
			NLMISC::CVector speed = step * emitter->getSpeed()[emitterIndex];
			do
			{
				-- outIt;
				*outIt = pos;
				pos -= speed;
			}
			while (outIt != endIt);
		}
	}
	else // compute parametric trajectory
	{
		emitter->integrateSingle(emitter->getOwner()->getSystemDate() + CParticleSystem::RealEllapsedTime - deltaT,
								 -step,
								 toProcess,
								 emitterIndex,
								 &dest[0]
								);
	}

	return toProcess;
}


/** The same as GenEmitterPositions, but with LOD taken in account.
  */
static inline uint GenEmitterPositionsWithLOD(CPSLocated *emitter,
									   CPSLocated *emittee,
									   uint emitterIndex,
									   uint numStep,
									   TAnimationTime deltaT, /* fraction of time needed to reach the first emission */
									   TAnimationTime step,
									   float invLODRatio,
									   std::vector<NLMISC::CVector> &dest
									  )
{
	NL_PS_FUNC(GenEmitterPositionsWithLOD)
	const uint toProcess = std::max(1U, std::min(numStep, (uint) emittee->getMaxSize()));
	dest.resize(toProcess);


	if (!emitter->isParametricMotionEnabled()) // standard case : take current pos and integrate
	{
		if (toProcess == 1) // only one emission -> takes current pos
		{
			dest[0] = emitter->getPos()[emitterIndex] - deltaT * emitter->getSpeed()[emitterIndex];
		}
		else
		{
			std::vector<NLMISC::CVector>::iterator outIt = dest.end();
			std::vector<NLMISC::CVector>::iterator endIt = dest.begin();
			NLMISC::CVector pos = emitter->getPos()[emitterIndex] - deltaT * emitter->getSpeed()[emitterIndex];
			NLMISC::CVector speed = step * invLODRatio * emitter->getSpeed()[emitterIndex];
			do
			{
				-- outIt;
				*outIt = pos;
				pos -= speed;
			}
			while (outIt != endIt);
		}
	}
	else // compute parametric trajectory
	{
		emitter->integrateSingle(emitter->getOwner()->getSystemDate() - deltaT - step * toProcess,
								 step,
								 toProcess,
								 emitterIndex,
								 &dest[0]
								);
	}

	return toProcess;
}

///==========================================================================
void CPSEmitter::processRegularEmissionConsistent(uint firstInstanceIndex, float emitLOD, float inverseEmitLOD)
{
	NL_PS_FUNC(CPSEmitter_processRegularEmissionConsistent)
	/// hmm some code factorisation would do no harm, but we want to keep tests outside the loops as much as possible...

	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	//
	const bool emitThreshold = _Owner->getOwner()->isEmitThresholdEnabled();
	//


	static std::vector<NLMISC::CVector> emitterPositions;
	// Positions for the emitter. They are computed by using a parametric trajectory or by using integration

	const uint size = _Owner->getSize();
	nlassert(firstInstanceIndex < size);
	uint leftToDo = size - firstInstanceIndex, toProcess;
	float emitPeriod[EMITTER_BUFF_SIZE];
	const float *currEmitPeriod;
	uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0;
	sint32 nbToGenerate;


	TPSAttribTime::iterator phaseIt = _Phase.begin() + firstInstanceIndex, endPhaseIt;
	TPSAttribUInt8::iterator numEmitIt;
	if(firstInstanceIndex < _NumEmission.getSize())
	{
		numEmitIt = _NumEmission.begin() + firstInstanceIndex;
	}
	else
	{
		numEmitIt = _NumEmission.end();
	}


	float ellapsedTimeLOD = CParticleSystem::EllapsedTime * emitLOD;
	uint maxEmissionCountLOD = (uint8) (_MaxEmissionCount * emitLOD);
	maxEmissionCountLOD = std::max(1u, maxEmissionCountLOD);
	// we don't use an iterator here
	// because it could be invalidated if size change (a located could generate itself)
	do
	{
		toProcess = leftToDo < EMITTER_BUFF_SIZE ? leftToDo : EMITTER_BUFF_SIZE;


		if (_PeriodScheme)
		{
			// compute period
			// NB : we ask to clamp entry because life counter of emitter are incremented, then spawn is called, and only after that dead emitters are removed
			//      so we may have a life counter that is > to 1
			currEmitPeriod = (float *) (_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess, true, 1 << 16, true));
			if (emitThreshold)
			{

				/** Test if 'make' filled our buffer. If this is not the case, we assume that values where precomputed, and that
				  * all null period have already been replaced by the threshold
				  */
				if (currEmitPeriod == emitPeriod)
				{
					// if there possibility to have 0 in the scheme ?
					if (_PeriodScheme->getMinValue() <= 0.f && _PeriodScheme->getMaxValue() >= 0.f)
					{
						replaceNullPeriodsByThreshold(emitPeriod, toProcess);
					}
				}
			}
		}
		else
		{
			if (_Period != 0.f || !emitThreshold)
			{
				currEmitPeriod = &_Period;
			}
			else
			{
				currEmitPeriod = &EMIT_PERIOD_THRESHOLD;
			}
		}

		endPhaseIt = phaseIt + toProcess;

		if (_MaxEmissionCount == 0) // no emission count limit
		{
			/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do
				{
					*phaseIt += ellapsedTimeLOD;
					if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							/** Must ensure phase is valid if period decrease over time
							  */
							*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTimeLOD);
							//
							/// compute the number of emissions
							uint numEmissions = (uint) ::floorf(*phaseIt / *currEmitPeriod);
							*phaseIt -= *currEmitPeriod * numEmissions;

							uint emitterIndex = (uint)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							if (nbToGenerate)
							{
								float deltaT = std::max(0.f, *phaseIt);

								/// compute the position of the emitter for the needed dates
								numEmissions = GenEmitterPositionsWithLOD(_Owner,
																		  _EmittedType,
																		  emitterIndex,
																		  numEmissions,
																		  deltaT,
																		  *currEmitPeriod,
																		  inverseEmitLOD,
																		  emitterPositions
																		 );

								/// process each emission at the right pos at the right date
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								uint k = numEmissions;
								float deltaTInc = *currEmitPeriod * inverseEmitLOD;
								do
								{
									--k;
									processEmitConsistent(emitterPositions[k],
														  emitterIndex,
														  nbToGenerate,
														  deltaT
														 );
									deltaT += deltaTInc;
								}
								while (k);
							}
						}
						else
						{
							const uint32 emitterIndex = (uint32)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							if (nbToGenerate)
							{
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								processEmit(emitterIndex, nbToGenerate);
							}
						}
					}

					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
			else // thhere's an emission delay
			{
				do
				{
					if (*phaseIt < _EmitDelay)
					{
						*phaseIt += CParticleSystem::EllapsedTime;
						if (*phaseIt < _EmitDelay)
						{
							++phaseIt;
							currEmitPeriod += currEmitPeriodPtrInc;
							continue;
						}
						else
						{
							*phaseIt = 	(*phaseIt - _EmitDelay) * emitLOD + _EmitDelay;
						}
					}
					else
					{
						*phaseIt += ellapsedTimeLOD;
					}

					if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							/** Must ensure phase is valid if period decrease over time
							  */
							*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTimeLOD + _EmitDelay);
							//
							uint numEmissions = (uint) ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod);
							*phaseIt -= *currEmitPeriod * numEmissions;
							float deltaT = std::max(*phaseIt - _EmitDelay, 0.f);
							//nlassert(deltaT >= 0.f);
							/// process each emission at the right pos at the right date
							uint emitterIndex = (uint)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							if (nbToGenerate)
							{

								/// compute the position of the emitter for the needed date
								numEmissions = GenEmitterPositionsWithLOD(  _Owner,
																			_EmittedType,
																			emitterIndex,
																			numEmissions,
																			deltaT,
																			*currEmitPeriod,
																			inverseEmitLOD,
																			emitterPositions
																		   );

								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								uint k = numEmissions;
								float deltaTInc = *currEmitPeriod * inverseEmitLOD;
								do
								{
									--k;
									processEmitConsistent(emitterPositions[k],
														  emitterIndex,
														  nbToGenerate,
														  deltaT);
									deltaT += deltaTInc;
								}
								while (k);
							}
						}
						else
						{
							const uint32 emitterIndex = (uint32)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							if (nbToGenerate)
							{
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								processEmit(emitterIndex, nbToGenerate);
							}
						}
					}

					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
		}
		else // there's an emission count limit
		{
				/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do
				{
					if (*numEmitIt < maxEmissionCountLOD)
					{
						*phaseIt += ellapsedTimeLOD;
						if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								/** Must ensure phase is valid if period decrease over time
								 */
								*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTimeLOD);
								//
								uint numEmissions = (uint) ::floorf(*phaseIt / *currEmitPeriod);
								*numEmitIt +=  numEmissions;
								*phaseIt -= *currEmitPeriod * numEmissions;
								float deltaT = std::max(*phaseIt, 0.f);
								//nlassert(deltaT >= 0.f);
								uint emitterIndex = (uint)(phaseIt - _Phase.begin());
								if (*numEmitIt > _MaxEmissionCount) // make sure we don't go over the emission limit
								{
									numEmissions -= *numEmitIt - _MaxEmissionCount;
									*numEmitIt = _MaxEmissionCount;
								}
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								if (nbToGenerate)
								{
									/// compute the position of the emitter for the needed date
									numEmissions = GenEmitterPositionsWithLOD(_Owner,
																				_EmittedType,
																				emitterIndex,
																				numEmissions,
																				deltaT,
																				*currEmitPeriod,
																				inverseEmitLOD,
																				emitterPositions
																			 );
									uint k = numEmissions;
									/// process each emission at the right pos at the right date
									nbToGenerate = (sint32) (emitLOD * nbToGenerate);
									if (!nbToGenerate) nbToGenerate = 1;
									float deltaTInc = *currEmitPeriod * inverseEmitLOD;
									do
									{
										--k;
										processEmitConsistent(emitterPositions[k],
															  emitterIndex,
															  nbToGenerate,
															  deltaT
															 );
										deltaT += deltaTInc;
									}
									while (k);
								}
							}
							else
							{
								const uint32 emitterIndex = (uint32)(phaseIt - _Phase.begin());
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								if (nbToGenerate)
								{
									nbToGenerate = (sint32) (emitLOD * nbToGenerate);
									if (!nbToGenerate) nbToGenerate = 1;
									processEmit(emitterIndex, nbToGenerate);
									++*numEmitIt;
								}
							}
						}
					}
					else
					{
						*numEmitIt = _MaxEmissionCount; // if the lod change, must ensure that the
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++ numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do
				{
					if (*numEmitIt < maxEmissionCountLOD)
					{
						if (*phaseIt < _EmitDelay)
						{
							*phaseIt += CParticleSystem::EllapsedTime;
							if (*phaseIt < _EmitDelay)
							{
								++phaseIt;
								currEmitPeriod += currEmitPeriodPtrInc;
								++numEmitIt;
								continue;
							}
							else
							{
								*phaseIt = 	(*phaseIt - _EmitDelay) * emitLOD + _EmitDelay;
							}
						}
						else
						{
							*phaseIt += ellapsedTimeLOD;
						}
						//
						if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								/** Must ensure phase is valid if period decrease over time
								 */
								*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTimeLOD + _EmitDelay);
								//
								uint numEmissions = (uint) ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod);
								*numEmitIt +=  numEmissions;
								*phaseIt -= *currEmitPeriod * numEmissions;
								float deltaT = std::max(*phaseIt - _EmitDelay, 0.f);
								//nlassert(deltaT >= 0.f);
								uint emitterIndex = (uint)(phaseIt - _Phase.begin());
								if (*numEmitIt > _MaxEmissionCount) // make sure we don't go over the emission limit
								{
									numEmissions -= *numEmitIt - _MaxEmissionCount;
									*numEmitIt = _MaxEmissionCount;
								}
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								if (nbToGenerate)
								{
									/// compute the position of the emitter for the needed date
									numEmissions = GenEmitterPositionsWithLOD(_Owner,
																			  _EmittedType,
																			  emitterIndex,
																			  numEmissions,
																			  deltaT,
																			  *currEmitPeriod,
																			  inverseEmitLOD,
																			  emitterPositions
																			 );
									uint k = numEmissions;
									/// process each emission at the right pos at the right date
									nbToGenerate = (sint32) (emitLOD * nbToGenerate);
									if (!nbToGenerate) nbToGenerate = 1;
									float deltaTInc = *currEmitPeriod * inverseEmitLOD;
									do
									{
										--k;
										processEmitConsistent(emitterPositions[k],
															  emitterIndex,
															  nbToGenerate,
															  deltaT);
										deltaT += deltaTInc;
									}
									while (k);
								}
							}
							else
							{
								const uint32 emitterIndex = (uint32)(phaseIt - _Phase.begin());
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								if (nbToGenerate)
								{
									nbToGenerate = (sint32) (emitLOD * nbToGenerate);
									if (!nbToGenerate) nbToGenerate = 1;
									processEmit(emitterIndex, nbToGenerate);
									++*numEmitIt;
								}
							}
						}
					}
					else
					{
						*numEmitIt = _MaxEmissionCount; // if the lod change, must ensure that the
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
		}

		leftToDo -= toProcess;
	}
	while (leftToDo);
}

///==========================================================================
void CPSEmitter::processRegularEmissionConsistentWithNoLOD(uint firstInstanceIndex)
{
	NL_PS_FUNC(CPSEmitter_processRegularEmissionConsistentWithNoLOD)
	/// hum, some code factorization would do no harm, but we want to keep tests outside the loops as much as possible...

	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	//
	const bool emitThreshold = _Owner->getOwner()->isEmitThresholdEnabled();
	//


	static std::vector<NLMISC::CVector> emitterPositions;
	// Positions for the emitter. They are computed by using a parametric trajectory or by using integration

	const uint size = _Owner->getSize();
	nlassert(firstInstanceIndex < size);
	uint leftToDo = size - firstInstanceIndex, toProcess;
	float emitPeriod[EMITTER_BUFF_SIZE];
	const float *currEmitPeriod;
	uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0;
	sint32 nbToGenerate;


	TPSAttribTime::iterator phaseIt = _Phase.begin() + firstInstanceIndex, endPhaseIt;
	TPSAttribUInt8::iterator numEmitIt;
	if (firstInstanceIndex < _NumEmission.getSize())
		numEmitIt = _NumEmission.begin() + firstInstanceIndex;
	else
		numEmitIt = _NumEmission.end();
	do
	{
		toProcess = leftToDo < EMITTER_BUFF_SIZE ? leftToDo : EMITTER_BUFF_SIZE;


		if (_PeriodScheme)
		{
			// compute period
			// NB : we ask to clamp entry because life counter of emitter are incremented, then spawn is called, and only after that dead emitters are removed
			//      so we may have a life counter that is > to 1
			currEmitPeriod = (float *) (_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess, true, 1 << 16, true));
			if (emitThreshold)
			{

				/** Test if 'make' filled our buffer. If this is not the case, we assume that values where precomputed, and that
				  * all null period have already been replaced by the threshold
				  */
				if (currEmitPeriod == emitPeriod)
				{
					// if there possibility to have 0 in the scheme ?
					if (_PeriodScheme->getMinValue() <= 0.f && _PeriodScheme->getMaxValue() >= 0.f)
					{
						replaceNullPeriodsByThreshold(emitPeriod, toProcess);
					}
				}
			}
		}
		else
		{
			if (_Period != 0.f || !emitThreshold)
			{
				currEmitPeriod = &_Period;
			}
			else
			{
				currEmitPeriod = &EMIT_PERIOD_THRESHOLD;
			}
		}

		endPhaseIt = phaseIt + toProcess;

		if (_MaxEmissionCount == 0) // no emission count limit
		{
			/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do
				{
					*phaseIt += CParticleSystem::EllapsedTime;
					if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							/** Must ensure phase is valid if period decrease over time
							  */
							*phaseIt = std::min(*phaseIt, *currEmitPeriod + CParticleSystem::EllapsedTime);
							//
							/// compute the number of emissions
							uint numEmissions = (uint) ::floorf(*phaseIt / *currEmitPeriod);
							*phaseIt -= *currEmitPeriod * numEmissions;
							float deltaT = std::max(0.f, *phaseIt);
							//nlassert(deltaT >= 0.f);
							uint emitterIndex = (uint)(phaseIt - _Phase.begin());

							/// compute the position of the emitter for the needed dates
							numEmissions = GenEmitterPositions(_Owner,
																_EmittedType,
																emitterIndex,
																numEmissions,
																deltaT,
																*currEmitPeriod,
																emitterPositions
															   );

							/// process each emission at the right pos at the right date
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							uint k = numEmissions;
							do
							{
								--k;
								processEmitConsistent(emitterPositions[k],
													  emitterIndex,
													  nbToGenerate,
													  deltaT);
								deltaT += *currEmitPeriod;
							}
							while (k);
						}
						else
						{
							const uint32 emitterIndex = (uint32)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							processEmit(emitterIndex, nbToGenerate);
						}
					}

					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
			else // thhere's an emission delay
			{
				do
				{
					*phaseIt += CParticleSystem::EllapsedTime;
					if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							/** Must ensure phase is valid if period decrease over time
							  */
							*phaseIt = std::min(*phaseIt, *currEmitPeriod + CParticleSystem::EllapsedTime + _EmitDelay);
							//
							uint numEmissions = (uint) ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod);
							*phaseIt -= *currEmitPeriod * numEmissions;
							float deltaT = std::max(*phaseIt - _EmitDelay, 0.f);
							//nlassert(deltaT >= 0.f);

							uint emitterIndex = (uint)(phaseIt - _Phase.begin());
							/// compute the position of the emitter for the needed date
							numEmissions = GenEmitterPositions(_Owner,
										        _EmittedType,
												emitterIndex,
								                numEmissions,
												deltaT,
											    *currEmitPeriod,
											    emitterPositions
											   );
							/// process each emission at the right pos at the right date
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							uint k = numEmissions;
							do
							{
								--k;
								processEmitConsistent(emitterPositions[k],
													  emitterIndex,
													  nbToGenerate,
													  deltaT);
								deltaT += *currEmitPeriod;
							}
							while (k);
						}
						else
						{
							const uint32 emitterIndex = (uint32)(phaseIt - _Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							processEmit(emitterIndex, nbToGenerate);
						}
					}

					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
		}
		else // there's an emission count limit
		{
				/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do
				{
					if (*numEmitIt < _MaxEmissionCount)
					{
						*phaseIt += CParticleSystem::EllapsedTime;
						if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								/** Must ensure phase is valid if period decrease over time
								 */
								*phaseIt = std::min(*phaseIt, *currEmitPeriod + CParticleSystem::EllapsedTime);
								//
								uint numEmissions = (uint) ::floorf(*phaseIt / *currEmitPeriod);
								*numEmitIt +=  numEmissions;
								*phaseIt -= *currEmitPeriod * numEmissions;
								float deltaT = std::max(*phaseIt, 0.f);
								//nlassert(deltaT >= 0.f);
								uint emitterIndex = (uint)(phaseIt - _Phase.begin());
								if (*numEmitIt > _MaxEmissionCount) // make sure we don't go over the emission limit
								{
									numEmissions -= *numEmitIt - _MaxEmissionCount;
									*numEmitIt = _MaxEmissionCount;
								}
								/// compute the position of the emitter for the needed date
								numEmissions = GenEmitterPositions(_Owner,
													_EmittedType,
													emitterIndex,
													numEmissions,
													deltaT,
													*currEmitPeriod,
													emitterPositions
												   );
								uint k = numEmissions;
								/// process each emission at the right pos at the right date
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								do
								{
									--k;
									processEmitConsistent(emitterPositions[k],
														  emitterIndex,
														  nbToGenerate,
														  deltaT);
									deltaT += *currEmitPeriod;
								}
								while (k);
							}
							else
							{
								const uint32 emitterIndex = (uint32)(phaseIt - _Phase.begin());
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								processEmit(emitterIndex, nbToGenerate);
								++*numEmitIt;
							}
						}
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++ numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do
				{
					if (*numEmitIt < _MaxEmissionCount)
					{
						*phaseIt += CParticleSystem::EllapsedTime;
						if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								/** Must ensure phase is valid if period decrease over time
								 */
								*phaseIt = std::min(*phaseIt, *currEmitPeriod + CParticleSystem::EllapsedTime + _EmitDelay);
								//
								uint numEmissions = (uint) ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod);
								*numEmitIt +=  numEmissions;
								*phaseIt -= *currEmitPeriod * numEmissions;
								float deltaT = std::max(*phaseIt - _EmitDelay, 0.f);
								//nlassert(deltaT >= 0.f);
								uint emitterIndex = (uint)(phaseIt - _Phase.begin());
								if (*numEmitIt > _MaxEmissionCount) // make sure we don't go over the emission limit
								{
									numEmissions -= *numEmitIt - _MaxEmissionCount;
									*numEmitIt = _MaxEmissionCount;
								}
								/// compute the position of the emitter for the needed date
								numEmissions = GenEmitterPositions(_Owner,
																	_EmittedType,
																	emitterIndex,
																	numEmissions,
																	deltaT,
																	*currEmitPeriod,
																	emitterPositions
																   );
								uint k = numEmissions;
								/// process each emission at the right pos at the right date
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								do
								{
									--k;
									processEmitConsistent(emitterPositions[k],
														  emitterIndex,
														  nbToGenerate,
														  deltaT);
									deltaT += *currEmitPeriod;
								}
								while (k);
							}
							else
							{
								const uint32 emitterIndex = (uint32)(phaseIt - _Phase.begin());
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								processEmit(emitterIndex, nbToGenerate);
								++*numEmitIt;
							}
						}
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
		}

		leftToDo -= toProcess;
	}
	while (leftToDo);
}


///==========================================================================
void CPSEmitter::step(TPSProcessPass pass)
{
	NL_PS_FUNC(CPSEmitter_step)
	if (pass == PSToolRender)
	{
		showTool();
	}
}

///==========================================================================
void CPSEmitter::computeSpawns(uint firstInstanceIndex)
{
	NL_PS_FUNC(CPSEmitter_computeSpawns)
	if (!_EmittedType) return;
	nlassert(CParticleSystem::InsideSimLoop);
	const uint32 size = _Owner->getSize();
	if (!size) return;
	if (CParticleSystem::EllapsedTime == 0.f) return; // do nothing when paused
	CParticleSystem *ps = _Owner->getOwner();
	nlassert(ps);
	float emitLOD;
	if (ps->isAutoLODEnabled() && !ps->isSharingEnabled() && !_BypassAutoLOD)
	{
		// temp test for auto lod
		emitLOD = ps->getAutoLODEmitRatio();
	}
	else
	{
		emitLOD = 1.f;
	}
	nlassert(_EmissionType == CPSEmitter::regular);
	if (!_ConsistentEmission)
	{
		if (emitLOD != 1.f)
		{
			processRegularEmission(firstInstanceIndex, emitLOD);
		}
		else
		{
			processRegularEmissionWithNoLOD(firstInstanceIndex);
		}
	}
	else
	{
		if (emitLOD != 1.f)
		{
			if (emitLOD != 0.f)
			{
				processRegularEmissionConsistent(firstInstanceIndex, emitLOD, 1.f / emitLOD);
			}
		}
		else
		{
			processRegularEmissionConsistentWithNoLOD(firstInstanceIndex);
		}
	}
}


///==========================================================================
void CPSEmitter::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSEmitter_newElement)
	nlassert(_Phase.getSize() != _Phase.getMaxSize());

	_Phase.insert(0.f);
	if (_MaxEmissionCount != 0)
	{
		_NumEmission.insert(0);
	}
	if (_PeriodScheme && _PeriodScheme->hasMemory()) _PeriodScheme->newElement(info);
	if (_GenNbScheme && _GenNbScheme->hasMemory()) _GenNbScheme->newElement(info);

}

///==========================================================================
inline void CPSEmitter::deleteElementBase(uint32 index)
{
	NL_PS_FUNC(CPSEmitter_deleteElementBase)
	if (_PeriodScheme && _PeriodScheme->hasMemory()) _PeriodScheme->deleteElement(index);
	if (_GenNbScheme && _GenNbScheme->hasMemory()) _GenNbScheme->deleteElement(index);
	_Phase.remove(index);
	if (_MaxEmissionCount != 0)
	{
		_NumEmission.remove(index);
	}
}

///==========================================================================
void CPSEmitter::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSEmitter_deleteElement)

	if (_EmissionType == CPSEmitter::onDeath && _EmittedType && _Active)
	{
		if (!_BypassEmitOnDeath)
		{
			const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, index) : _GenNb;
			processEmitOutsideSimLoop(index, nbToGenerate);
		}
	}
	deleteElementBase(index);
}

///==========================================================================
void CPSEmitter::deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep)
{
	NL_PS_FUNC(CPSEmitter_deleteElement)
	if (_EmissionType == CPSEmitter::onDeath && _EmittedType && _Active)
	{
		const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, index) : _GenNb;
		processEmitConsistent(_Owner->getPos()[index], index, nbToGenerate, timeUntilNextSimStep);
	}
	deleteElementBase(index);
}


///==========================================================================
void CPSEmitter::resize(uint32 size)
{
	NL_PS_FUNC(CPSEmitter_resize)
	nlassert(size < (1 << 16));
	if (_PeriodScheme && _PeriodScheme->hasMemory()) _PeriodScheme->resize(size, _Owner->getSize());
	if (_GenNbScheme && _GenNbScheme->hasMemory()) _GenNbScheme->resize(size, _Owner->getSize());
	_Phase.resize(size);
	if (_MaxEmissionCount != 0)
	{
		_NumEmission.resize(size);
	}
}

///==========================================================================
void CPSEmitter::bounceOccured(uint32 index, TAnimationTime timeToNextSimStep)
{
	NL_PS_FUNC(CPSEmitter_bounceOccured)
	// TODO : avoid duplication with deleteElement
	if (_EmittedType && _EmissionType == CPSEmitter::onBounce)
	{
		const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, index) : _GenNb;
		processEmitConsistent(_Owner->getPos()[index], index, nbToGenerate, timeToNextSimStep);
	}
}

///==========================================================================
void CPSEmitter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSEmitter_serial)
	/// version 6  : the flag _EmitDirBasis no longer exist, it has been replaced by _UserMatrixModeForEmissionDirection
	//
	/// version 5  : added _BypassAutoLOD
	/// version 4  : added consistent emissions
	sint ver = f.serialVersion(6);
	CPSLocatedBindable::serial(f);

	f.serialPolyPtr(_EmittedType);
	f.serial(_Phase);
	f.serial(_SpeedInheritanceFactor);

	bool speedBasisEmission = _SpeedBasisEmission; // tmp copy because of bitfield serialization scheme
	f.serial(speedBasisEmission);
	_SpeedBasisEmission = speedBasisEmission;

	f.serialEnum(_EmissionType);

	// this is for use with serial
	bool trueB = true, falseB = false;

	if (!f.isReading())
	{
		switch (_EmissionType)
		{
			case CPSEmitter::regular:
				if (_PeriodScheme)
				{
					f.serial(trueB);
					f.serialPolyPtr(_PeriodScheme);
				}
				else
				{
					 f.serial(falseB);
					 f.serial(_Period);
				}
				if (ver >= 3)
				{
					f.serial(_EmitDelay, _MaxEmissionCount);
				}
			break;
			default:
			break;
		}
		if (_GenNbScheme)
		{
			f.serial(trueB);
			f.serialPolyPtr(_GenNbScheme);
		}
		else
		{
			 f.serial(falseB);
			 f.serial(_GenNb);
		}
	}
	else
	{
		bool useScheme;
		switch (_EmissionType)
		{
			case CPSEmitter::regular:
			{
				f.serial(useScheme);
				if (useScheme)
				{
					delete _PeriodScheme;
					f.serialPolyPtr(_PeriodScheme);
				}
				else
				{
					 f.serial(_Period);
				}
				if (ver >= 3)
				{
					f.serial(_EmitDelay, _MaxEmissionCount);
					updateMaxCountVect();
				}
			}
			break;
			default:
			break;
		}

		f.serial(useScheme);
		if (useScheme)
		{
			delete _GenNbScheme;
			f.serialPolyPtr(_GenNbScheme);
		}
		else
		{
			 f.serial(_GenNb);
		}
	}
	if (ver > 1 && ver < 6)
	{
		nlassert(f.isReading());
		bool emitDirBasis;
		f.serial(emitDirBasis);
		if (emitDirBasis)
		{
			_UserMatrixModeForEmissionDirection = false;
			_UserDirectionMatrixMode = PSFXWorldMatrix;
		}
		else
		{
			_UserMatrixModeForEmissionDirection = true;
			if (_Owner)
			{
				_UserDirectionMatrixMode = _Owner->getMatrixMode() == PSFXWorldMatrix ? PSIdentityMatrix : PSFXWorldMatrix;
			}
			else
			{
				_UserDirectionMatrixMode = PSFXWorldMatrix;
			}
		}
	}
	if (ver >= 4)
	{
		bool consistentEmission = _ConsistentEmission; // tmp copy because of bitfield serialization scheme
		f.serial(consistentEmission);
		_ConsistentEmission = consistentEmission;
	}
	if (ver >= 5)
	{
		bool byassAutoLOD = _BypassAutoLOD; // tmp copy because of bitfield serialization scheme
		f.serial(byassAutoLOD);
		_BypassAutoLOD = byassAutoLOD;
	}
	if (ver >= 6)
	{
		bool userMatrixModeForEmissionDirection = _UserMatrixModeForEmissionDirection; // tmp copy because of bitfield serialization scheme
		f.serial(userMatrixModeForEmissionDirection);
		_UserMatrixModeForEmissionDirection = userMatrixModeForEmissionDirection;
		f.serialEnum(_UserDirectionMatrixMode);
	}
}

///==========================================================================
void	CPSEmitter::updateMaxCountVect()
{
	NL_PS_FUNC(CPSEmitter_updateMaxCountVect)
	if (!_MaxEmissionCount || !_Owner)
	{
		_NumEmission.resize(0);
	}
	else
	{
		_NumEmission.resize(_Owner->getMaxSize());
		while (_NumEmission.getSize() != 0)
		{
			_NumEmission.remove(0);
		}
		while (_NumEmission.getSize() != _Owner->getSize())
		{
			_NumEmission.insert(0);
		}
	}
}

///==========================================================================
void CPSEmitter::setEmitDelay(float delay)
{
	NL_PS_FUNC(CPSEmitter_setEmitDelay)
	_EmitDelay = delay;
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->systemDurationChanged();
	}
}

///==========================================================================
bool	CPSEmitter::setMaxEmissionCount(uint8 count)
{
	NL_PS_FUNC(CPSEmitter_setMaxEmissionCount)
	if (count == _MaxEmissionCount) return true;
	nlassert(_Owner && _Owner->getOwner());
	CParticleSystem *ps = _Owner->getOwner();
	if (ps->getBypassMaxNumIntegrationSteps())
	{
		uint8 oldEmissiontCount = _MaxEmissionCount;
		// should check that the new value is valid
		_MaxEmissionCount = count;
		if (testEmitForever())
		{
			_MaxEmissionCount = oldEmissiontCount;
			nlwarning("<CPSEmitter::setMaxEmissionCount> can't set max emission count to %d  \
					   with the current configuration : the system has been flagged with     \
					   'BypassMaxNumIntegrationSteps', and should have a finite duration.    \
					   The new value is not set", (int) count);
			return false;
		}
	}
	ps->systemDurationChanged();
	_MaxEmissionCount = count;
	updateMaxCountVect();
	return true;
}

///==========================================================================
bool CPSEmitter::checkLoop() const
{
	NL_PS_FUNC(CPSEmitter_checkLoop)
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	if (!_EmittedType) return false;
	std::set<const CPSLocated *> seenLocated; // the located we've already seen
	std::vector<const CPSLocated *> leftLoc(1);  // the located that are left to see
	leftLoc[0] = _EmittedType;
	do
	{
		const CPSLocated *curr = leftLoc.back();
		if (curr == this->_Owner) return true;
		leftLoc.pop_back();
		seenLocated.insert(curr);
		for(uint32 k = 0; k < curr->getNbBoundObjects(); ++k)
		{
			const CPSEmitter *emitter = dynamic_cast<const CPSEmitter *>(curr->getBoundObject(k));
			if (emitter && emitter->_EmittedType)
			{
				if (seenLocated.find(emitter->_EmittedType) == seenLocated.end()) // not already seen this one ?
				{
					leftLoc.push_back(emitter->_EmittedType);
				}
			}
		}
	}
	while (!leftLoc.empty());
	return false;
}

///==========================================================================
bool CPSEmitter::testEmitForever() const
{
	NL_PS_FUNC(CPSEmitter_testEmitForever)
	if (!_Owner)
	{
		nlwarning("<CPSEmitter::testEmitForever> The emitter should be inserted in a CPSLocated instance for this call to work.");
		nlassert(0);
		return true;
	}
	if (!_Owner->getLastForever()) return false;
	switch(getEmissionType())
	{
		case CPSEmitter::onBounce:
		case CPSEmitter::externEmit:
		case CPSEmitter::regular:
			// it is ok only if a limited number of located is emitted
			if (getMaxEmissionCount() == 0) return true;
		break;
		case CPSEmitter::onDeath: return true; // the emitter never dies, so ..
		case CPSEmitter::once: return false;
		break;
		default:
			nlassert(0); // not a known type
		break;
	}
	return false;
}


////////////////////////////////////////////
// implementation of CPSModulatedEmitter  //
////////////////////////////////////////////

void CPSModulatedEmitter::serialEmitteeSpeedScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSModulatedEmitter_IStream )
	bool useScheme;
	if (!f.isReading())
	{
		useScheme = useEmitteeSpeedScheme();
	}
	f.serial(useScheme);
	if (useScheme)
	{
		f.serialPolyPtr(_EmitteeSpeedScheme);
	}
	else
	{
		f.serial(_EmitteeSpeed);
	}
}



////////////////////////////////////////////
// implementation of CPSEmitterOmni		  //
////////////////////////////////////////////

///==========================================================================
void CPSEmitterOmni::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	NL_PS_FUNC(CPSEmitterOmni_emit)
	// TODO : verifier que ca marche si une particule s'emet elle-mem
	nlassert(_EmittedType);

	CVector v( ((rand() % 1000) - 500) / 500.0f
				   , ((rand() % 1000) - 500) / 500.0f
				   , ((rand() % 1000) - 500) / 500.0f);
	v.normalize();
	v *= _EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed;

	pos = srcPos;
	speed = v;

}

///==========================================================================
void CPSEmitterOmni::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSEmitterOmni_serial)
	f.serialVersion(1);
	CPSEmitter::serial(f);
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f);
}

///==========================================================================
void CPSEmitterOmni::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSEmitterOmni_newElement)
	CPSEmitter::newElement(info);
	newEmitteeSpeedElement(info);
}

///==========================================================================
inline void CPSEmitterOmni::deleteElementBase(uint32 index)
{
	NL_PS_FUNC(CPSEmitterOmni_deleteElementBase)
	deleteEmitteeSpeedElement(index);
}

///==========================================================================
void CPSEmitterOmni::deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep)
{
	NL_PS_FUNC(CPSEmitterOmni_deleteElement)
	CPSEmitter::deleteElement(index, timeUntilNextSimStep);
	deleteElementBase(index);
}

///==========================================================================
void CPSEmitterOmni::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSEmitterOmni_deleteElement)
	CPSEmitter::deleteElement(index);
	deleteElementBase(index);
}

///==========================================================================
void CPSEmitterOmni::resize(uint32 capacity)
{
	NL_PS_FUNC(CPSEmitterOmni_resize)
	nlassert(capacity < (1 << 16));
	CPSEmitter::resize(capacity);
	resizeEmitteeSpeed(capacity);
}

///==========================================================================
void CPSEmitterDirectionnal::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	NL_PS_FUNC(CPSEmitterDirectionnal_emit)
	// TODO : verifier que ca marche si une particule s'emet elle-mem
	nlassert(_EmittedType);


	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) * _Dir;
	pos = srcPos;
}

///==========================================================================
void CPSEmitterDirectionnal::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSEmitterDirectionnal_newElement)
	CPSEmitter::newElement(info);
	newEmitteeSpeedElement(info);
}


///==========================================================================
inline void CPSEmitterDirectionnal::deleteElementBase(uint32 index)
{
	NL_PS_FUNC(CPSEmitterDirectionnal_deleteElementBase)
	deleteEmitteeSpeedElement(index);
}

///==========================================================================
void CPSEmitterDirectionnal::deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep)
{
	NL_PS_FUNC(CPSEmitterDirectionnal_deleteElement)
	CPSEmitter::deleteElement(index, timeUntilNextSimStep);
	deleteElementBase(index);
}

///==========================================================================
void CPSEmitterDirectionnal::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSEmitterDirectionnal_deleteElement)
	CPSEmitter::deleteElement(index);
	deleteElementBase(index);
}



///==========================================================================
void CPSEmitterDirectionnal::resize(uint32 capacity)
{
	NL_PS_FUNC(CPSEmitterDirectionnal_resize)
	nlassert(capacity < (1 << 16));
	CPSEmitter::resize(capacity);
	resizeEmitteeSpeed(capacity);
}

///==========================================================================
void CPSEmitterDirectionnal::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSEmitterDirectionnal_IStream )
	f.serialVersion(1);
	CPSEmitter::serial(f);
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f);
	f.serial(_Dir);
}

////////////////////////////////////////////
// implementation of CPSEmitterRectangle  //
////////////////////////////////////////////

///==========================================================================
void CPSEmitterRectangle::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSEmitterRectangle_IStream )
	f.serialVersion(1);
	CPSEmitter::serial(f);
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f);
	f.serial(_Basis);
	f.serial(_Width);
	f.serial(_Height);
	f.serial(_Dir);

}

///==========================================================================
void CPSEmitterRectangle::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	NL_PS_FUNC(CPSEmitterRectangle_emit)
	CVector N = _Basis[index].X ^ _Basis[index].Y;
	pos = srcPos + ((rand() % 32000) * (1.f / 16000) - 1.f) *  _Width[index] *  _Basis[index].X
				 + ((rand() % 32000) * (1.f / 16000) - 1.f) *  _Height[index] * _Basis[index].Y;
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed)
					* (_Dir.x * _Basis[index].X+ _Dir.y * _Basis[index].Y + _Dir.z *  N);
}

///==========================================================================
void CPSEmitterRectangle::setMatrix(uint32 index, const CMatrix &m)
{
	NL_PS_FUNC(CPSEmitterRectangle_setMatrix)
	_Owner->getPos()[index] = m.getPos();


	 _Basis[index].X = m.getI();
	 _Basis[index].Y = m.getJ();
}

///==========================================================================
CMatrix CPSEmitterRectangle::getMatrix(uint32 index) const
{
	NL_PS_FUNC(CPSEmitterRectangle_getMatrix)
	CMatrix m;
	m.setPos(_Owner->getPos()[index]);
	m.setRot(_Basis[index].X, _Basis[index].Y, _Basis[index].X ^ _Basis[index].Y, true);
	return m;
}

///==========================================================================
void CPSEmitterRectangle::setScale(uint32 index, float scale)
{
	NL_PS_FUNC(CPSEmitterRectangle_setScale)
	_Width[index] = scale;
	_Height[index] = scale;
}

///==========================================================================
void CPSEmitterRectangle::setScale(uint32 index, const CVector &s)
{
	NL_PS_FUNC(CPSEmitterRectangle_setScale)
	_Width[index] = s.x;
	_Height[index] = s.y;
}

///==========================================================================
CVector CPSEmitterRectangle::getScale(uint32 index) const
{
	NL_PS_FUNC(CPSEmitterRectangle_getScale)
	return CVector(_Width[index], _Height[index], 1.f);
}

///==========================================================================
void CPSEmitterRectangle::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(		CPSEmitterRectangle_newElement)
	CPSEmitter::newElement(info);
	newEmitteeSpeedElement(info);
	_Basis.insert(CPlaneBasis(CVector::K));
	_Width.insert(1.f);
	_Height.insert(1.f);
}

///==========================================================================
inline void CPSEmitterRectangle::deleteElementBase(uint32 index)
{
	NL_PS_FUNC(CPSEmitterRectangle_deleteElementBase)
	deleteEmitteeSpeedElement(index);
	_Basis.remove(index);
	_Width.remove(index);
	_Height.remove(index);
}

///==========================================================================
void CPSEmitterRectangle::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSEmitterRectangle_deleteElement)
	CPSEmitter::deleteElement(index);
	deleteElementBase(index);
}

///==========================================================================
void CPSEmitterRectangle::deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep)
{
	NL_PS_FUNC(CPSEmitterRectangle_deleteElement)
	CPSEmitter::deleteElement(index, timeUntilNextSimStep);
	deleteElementBase(index);
}

///==========================================================================
void CPSEmitterRectangle::resize(uint32 size)
{
	NL_PS_FUNC(CPSEmitterRectangle_resize)
	nlassert(size < (1 << 16));
	CPSEmitter::resize(size);
	resizeEmitteeSpeed(size);
	_Basis.resize(size);
	_Width.resize(size);
	_Height.resize(size);
}

///==========================================================================
void CPSEmitterRectangle::showTool(void)
{
	NL_PS_FUNC(CPSEmitterRectangle_showTool)
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
		CPSUtil::displayBasis(getDriver() ,getLocalToWorldMatrix(), mat, 1.f, *getFontGenerator(), *getFontManager());
		setupDriverModelMatrix();

		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));



		const CVector &pos = _Owner->getPos()[k];
		CPSUtil::display3DQuad(*getDriver(), pos + I * _Width[k] + J * _Height[k]
										   , pos + I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] + J * _Height[k], col);
	}
}



////////////////////////////////////
// CPSEmitterconic implementation //
////////////////////////////////////

///==========================================================================
void CPSEmitterConic::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSEmitterConic_serial)
	f.serialVersion(1);
	CPSEmitterDirectionnal::serial(f);
	f.serial(_Radius);
}

///==========================================================================
void CPSEmitterConic::setDir(const CVector &v)
{
	NL_PS_FUNC(CPSEmitterConic_setDir)
	CPSEmitterDirectionnal::setDir(v);

}

///==========================================================================
void CPSEmitterConic::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	NL_PS_FUNC(CPSEmitterConic_emit)
	// TODO : optimize that
	nlassert(_EmittedType);

	// we choose a custom direction like with omnidirectionnal emitter
	// then we force the direction vect to have the unit size

	static const double divRand = (2.0 / RAND_MAX);

	CVector dir((float) (rand() * divRand - 1)
				, (float) (rand() * divRand - 1)
				, (float) (rand() * divRand - 1) );

	const float n =dir.norm();

	dir *= _Radius / n;

	dir -= (_Dir * dir) * _Dir;
	dir += _Dir;
	dir.normalize();


	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed)
		    * dir;
	pos = srcPos;
}

////////////////////////////////////////
// CPSSphericalEmitter implementation //
////////////////////////////////////////

///==========================================================================
void CPSSphericalEmitter::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	NL_PS_FUNC(CPSSphericalEmitter_emit)
	static const double divRand = (2.0 / RAND_MAX);
	CVector dir((float) (rand() * divRand - 1), (float) (rand() * divRand - 1) , (float) (rand() * divRand - 1) );
	dir.normalize();
	pos = srcPos + _Radius[index] * dir;
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed)  * dir;
}


///==========================================================================
void CPSSphericalEmitter::showTool(void)
{
	NL_PS_FUNC(CPSSphericalEmitter_showTool)
	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);


	TPSAttribFloat::const_iterator radiusIt = _Radius.begin();
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt = _Owner->getPos().end();
	setupDriverModelMatrix();
	for (uint k = 0; posIt != endPosIt; ++posIt, ++radiusIt, ++k)
	{
		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));
		CPSUtil::displaySphere(*getDriver(), *radiusIt, *posIt, 5, col);
	}
}


///==========================================================================
void CPSSphericalEmitter::setMatrix(uint32 index, const CMatrix &m)
{
	NL_PS_FUNC(CPSSphericalEmitter_setMatrix)
	nlassert(index < _Radius.getSize());
	// compute new pos
	_Owner->getPos()[index] = m.getPos();

}

///==========================================================================
CMatrix CPSSphericalEmitter::getMatrix(uint32 index) const
{
	NL_PS_FUNC(CPSSphericalEmitter_getMatrix)
	nlassert(index < _Radius.getSize());
	CMatrix m;
	m.identity();
	m.translate(_Owner->getPos()[index]);
	return m;
}

///==========================================================================
void CPSSphericalEmitter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSSphericalEmitter_serial)
	f.serialVersion(1);
	CPSEmitter::serial(f);
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f);
	f.serial(_Radius);
}

///==========================================================================
void CPSSphericalEmitter::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSSphericalEmitter_newElement)
	CPSEmitter::newElement(info);
	newEmitteeSpeedElement(info);
	_Radius.insert(1.f);
}

///==========================================================================
inline void CPSSphericalEmitter::deleteElementBase(uint32 index)
{
	NL_PS_FUNC(CPSSphericalEmitter_deleteElementBase)
	deleteEmitteeSpeedElement(index);
	_Radius.remove(index);
}

///==========================================================================
void CPSSphericalEmitter::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSSphericalEmitter_deleteElement)
	CPSEmitter::deleteElement(index);
	deleteElementBase(index);
}

///==========================================================================
void CPSSphericalEmitter::deleteElement(uint32 index, TAnimationTime timeUntilNextSimStep)
{
	NL_PS_FUNC(CPSSphericalEmitter_deleteElement)
	CPSEmitter::deleteElement(index, timeUntilNextSimStep);
	deleteElementBase(index);
}

///==========================================================================
void CPSSphericalEmitter::resize(uint32 size)
{
	NL_PS_FUNC(CPSSphericalEmitter_resize)
	nlassert(size < (1 << 16));
	CPSEmitter::resize(size);
	resizeEmitteeSpeed(size);
	_Radius.resize(size);
}

/////////////////////////////////////
// CPSRadialEmitter implementation //
/////////////////////////////////////

///==========================================================================
void CPSRadialEmitter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	NL_PS_FUNC(CPSRadialEmitter_serial)
	f.serialVersion(1);
	CPSEmitterDirectionnal::serial(f);
}

///==========================================================================
void CPSRadialEmitter::emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed)
{
	NL_PS_FUNC(CPSRadialEmitter_emit)
	// TODO : verify if it works when a particle emits itself
	nlassert(_EmittedType);

	static const double divRand = (2.0 / RAND_MAX);
	CVector dir((float) (rand() * divRand - 1),
		        (float) (rand() * divRand - 1),
				(float) (rand() * divRand - 1) );
	dir -= (dir * _Dir) * _Dir; //keep tangential direction
	dir.normalize();
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) * dir;
	pos = srcPos;
}


///===============================================================================
void CPSEmitter::enableSpeedBasisEmission(bool enabled /*=true*/)
{
	NL_PS_FUNC(CPSEmitter_enableSpeedBasisEmission)
	bool wasUserMatNeeded = isUserMatrixUsed();
	_SpeedBasisEmission  = enabled;
	updatePSRefCountForUserMatrixUsage(isUserMatrixUsed(), wasUserMatNeeded);
}

///===============================================================================
void CPSEmitter::enableUserMatrixModeForEmissionDirection(bool enable /*=true*/)
{
	NL_PS_FUNC(CPSEmitter_enableUserMatrixModeForEmissionDirection)
	bool wasUserMatNeeded = isUserMatrixUsed();
	_UserMatrixModeForEmissionDirection = enable;
	updatePSRefCountForUserMatrixUsage(isUserMatrixUsed(), wasUserMatNeeded);
}

///===============================================================================
void CPSEmitter::setUserMatrixModeForEmissionDirection(TPSMatrixMode matrixMode)
{
	NL_PS_FUNC(CPSEmitter_setUserMatrixModeForEmissionDirection)
	bool wasUserMatNeeded = isUserMatrixUsed();
	_UserDirectionMatrixMode = matrixMode;
	updatePSRefCountForUserMatrixUsage(isUserMatrixUsed(), wasUserMatNeeded);
}


///==========================================================================
void CPSEmitter::updatePSRefCountForUserMatrixUsage(bool matrixIsNeededNow, bool matrixWasNeededBefore)
{
	NL_PS_FUNC(CPSEmitter_updatePSRefCountForUserMatrixUsage)
	if (_Owner && _Owner->getOwner())
	{
		if (matrixIsNeededNow && !matrixWasNeededBefore)
		{
			_Owner->getOwner()->addRefForUserSysCoordInfo();
		}
		else if (!matrixIsNeededNow && matrixWasNeededBefore)
		{
			_Owner->getOwner()->releaseRefForUserSysCoordInfo();
		}
	}
}

///==========================================================================
bool CPSEmitter::isUserMatrixUsed() const
{
	NL_PS_FUNC(CPSEmitter_isUserMatrixUsed)
	return !_SpeedBasisEmission && _UserMatrixModeForEmissionDirection && _UserDirectionMatrixMode == PSUserMatrix;
}

///==========================================================================
bool CPSEmitter::getUserMatrixUsageCount() const
{
	NL_PS_FUNC(CPSEmitter_getUserMatrixUsageCount)
	return isUserMatrixUsed() ? 1 : 0;
}



///==========================================================================
void CPSEmitter::doEmitOnce(uint firstInstanceIndex)
{
	NL_PS_FUNC(CPSEmitter_doEmitOnce)
	if (!_EmittedType) return;
	if (!_GenNbScheme && _GenNb == 0) return;
	nlassert(_Owner);
	nlassert(CParticleSystem::InsideSimLoop); // should only be called by the sim loop
	float emitLOD;
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	const CParticleSystem *ps = _Owner->getOwner();
	if (ps->isAutoLODEnabled() && !ps->isSharingEnabled() && !_BypassAutoLOD)
	{
		// temp test for auto lod
		emitLOD = ps->getAutoLODEmitRatio();
	}
	else
	{
		emitLOD = 1.f;
	}
	nlassert(emitLOD >= 0.f);
	if (_GenNbScheme)
	{
		const uint BATCH_SIZE = 1024;
		uint32 numToEmit[BATCH_SIZE];
		uint k = firstInstanceIndex;
		nlassert(firstInstanceIndex < _Owner->getSize());
		uint leftToDo = _Owner->getSize() - firstInstanceIndex;

		while (leftToDo)
		{
			uint toProcess = std::min((uint) BATCH_SIZE, leftToDo);
			uint32 *numToEmitPtr = (uint32 *) _GenNbScheme->make(_Owner, k, numToEmit, sizeof(uint32), true);
			leftToDo -= toProcess;
			while (toProcess)
			{
				CVector startPos;
				if (!_Owner->isParametricMotionEnabled())
				{
					startPos = _Owner->getPos()[k] - _Owner->getSpeed()[k] * CParticleSystem::EllapsedTime;
				}
				else
				{
					startPos = _Owner->getParametricInfos()[k].Pos;
				}
				float currTime = _Owner->getTime()[k];
				_Owner->getTime()[k] = 0.f; // when emit occured, time was 0
				sint32 nbToGenerate = (sint32) (emitLOD * *numToEmitPtr);
				if (nbToGenerate > 0)
				{
					nbToGenerate = std::min(nbToGenerate, (sint32) _EmittedType->getMaxSize());
					processEmitConsistent(startPos, k, nbToGenerate, _Owner->getAgeInSeconds(k) / CParticleSystem::RealEllapsedTimeRatio);
				}
				// restore time & pos
				_Owner->getTime()[k] = currTime;
				++ k;
				++ numToEmitPtr;
				-- toProcess;
			}
		}
	}
	else
	{
		sint nbToGenerate = (sint) (emitLOD * _GenNb);
		if (nbToGenerate <= 0)  nbToGenerate = 1;
		nbToGenerate = std::min(nbToGenerate, (sint) _EmittedType->getMaxSize());
		for(uint k = firstInstanceIndex; k < _Owner->getSize(); ++k)
		{
			// retrieve previous position (because motion step is done before spawn step)
			CVector startPos;
			if (!_Owner->isParametricMotionEnabled())
			{
				startPos = _Owner->getPos()[k] - _Owner->getSpeed()[k] * CParticleSystem::EllapsedTime;
			}
			else
			{
				startPos = _Owner->getParametricInfos()[k].Pos;
			}
			float currTime = _Owner->getTime()[k];
			_Owner->getTime()[k] = 0.f; // when emit occured, time was 0
			processEmitConsistent(startPos, k, nbToGenerate, _Owner->getAgeInSeconds(k) / CParticleSystem::RealEllapsedTimeRatio);
			// restore time & pos
			_Owner->getTime()[k] = currTime;
		}
	}
}

///==========================================================================
void CPSEmitter::updateEmitTrigger()
{
	NL_PS_FUNC(CPSEmitter_updateEmitTrigger)
	if (!_EmitTrigger) return;
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	const CParticleSystem *ps = _Owner->getOwner();
	float emitLOD;
	if (ps->isAutoLODEnabled() && !ps->isSharingEnabled() && !_BypassAutoLOD)
	{
		// temp test for auto lod
		emitLOD = ps->getAutoLODEmitRatio();
	}
	else
	{
		emitLOD = 1.f;
	}
	if (_GenNbScheme)
	{
		const uint BATCH_SIZE = 1024;
		uint32 numToEmit[BATCH_SIZE];
		uint k = 0;
		uint leftToDo = _Owner->getSize();
		while (leftToDo)
		{
			uint toProcess = std::min(BATCH_SIZE, leftToDo);
			uint32 *numToEmitPtr = (uint32 *) _GenNbScheme->make(_Owner, k, numToEmit, sizeof(uint32), true);
			while (toProcess)
			{
				uint32 nbToGenerate = (sint32) (emitLOD * *numToEmitPtr);
				if (!nbToGenerate) nbToGenerate = 1;
				processEmit(k, nbToGenerate);
				++ k;
				++ numToEmitPtr;
			}

			leftToDo -= toProcess;
		}
	}
	else
	{
		uint nbToGenerate = (sint32) (emitLOD * _GenNb);
		if (!nbToGenerate) nbToGenerate = 1;
		for(uint k = 0; k < _Owner->getSize(); ++k)
		{
			processEmit(k, nbToGenerate);
		}
	}
	_EmitTrigger = false;
}



} // NL3D

namespace NLMISC
{

std::string toString(NL3D::CPSEmitter::TEmissionType type)
{
	NL_PS_FUNC(toString_CPSEmitter_TEmissionType)
	nlctassert(NL3D::CPSEmitter::numEmissionType == 5); // If this ct assertion is raised, the content of TEmissionType has changed, so should change this function !
	switch (type)
	{
		case NL3D::CPSEmitter::regular: return "regular";
		case NL3D::CPSEmitter::onDeath: return "onDeath";
		case NL3D::CPSEmitter::once: return "once";
		case NL3D::CPSEmitter::onBounce: return "onBounce";
		case NL3D::CPSEmitter::externEmit: return "externEmit";
		default:
			nlassert(0);
			return "";
		break;
	}
}


} // NLMISC
