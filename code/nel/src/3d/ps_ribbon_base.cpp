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

#include "nel/misc/common.h"
#include "nel/3d/ps_ribbon_base.h"
#include "nel/3d/particle_system.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

////////////////////////////////////
//  CPSRibbonBase implementation  //
////////////////////////////////////


/// build some hermite spline value, with the given points and tangents
static inline void BuildHermiteVector(const NLMISC::CVector &P0,
							   const NLMISC::CVector &P1,
							   const NLMISC::CVector &T0,
							   const NLMISC::CVector &T1,
									 NLMISC::CVector &dest,
							   float lambda
							   )
{
	NL_PS_FUNC(BuildHermiteVector)
	const float lambda2 = lambda * lambda;
	const float lambda3 = lambda2 * lambda;
	const float h1 = 2 * lambda3 - 3 * lambda2 + 1;
	const float h2 = - 2 * lambda3 + 3 * lambda2;
	const float h3 = lambda3 - 2 * lambda2 + lambda;
	const float h4 = lambda3 - lambda2;
	/// just avoid some ctor calls here...
	dest.set (h1 * P0.x + h2 * P1.x + h3 * T0.x + h4 * T1.x,
			  h1 * P0.y + h2 * P1.y + h3 * T0.y + h4 * T1.y,
			  h1 * P0.z + h2 * P1.z + h3 * T0.z + h4 * T1.z);

}

/// for test
static inline void BuildLinearVector(const NLMISC::CVector &P0,
									 const NLMISC::CVector &P1,
									 NLMISC::CVector &dest,
									 float lambda,
									 float oneMinusLambda
							        )
{
	NL_PS_FUNC(BuildLinearVector)
	dest.set (lambda * P1.x + oneMinusLambda * P0.x,
			  lambda * P1.y + oneMinusLambda * P0.y,
			  lambda * P1.z + oneMinusLambda * P0.z);
}


const uint EndRibbonStorage = 1;


//=======================================================
CPSRibbonBase::CPSRibbonBase() : _NbSegs(8),
								 _SegDuration(0.02f),
								 _Parametric(false),
								 _RibbonIndex(0),
								 _MatrixMode(FatherMatrix),
								 _LastUpdateDate(0),
								 _RibbonMode(VariableSize),
								 _InterpolationMode(Hermitte),
								 _RibbonLength(1),
								 _SegLength(_RibbonLength / _NbSegs),
								 _LODDegradation(1)

{
	NL_PS_FUNC(CPSRibbonBase_CPSRibbonBase)
	initDateVect();
}

//=======================================================
void CPSRibbonBase::setMatrixMode(TMatrixMode matrixMode)
{
	NL_PS_FUNC(CPSRibbonBase_setMatrixMode)
	if (matrixMode == _MatrixMode) return;
	if (_Owner) resetFromOwner();
	_MatrixMode = matrixMode;
}

//=======================================================
void	CPSRibbonBase::setRibbonLength(float length)
{
	NL_PS_FUNC(CPSRibbonBase_setRibbonLength)
	nlassert(length > 0.f);
	_RibbonLength = length;
	_SegLength = length / _NbSegs;
}

//=======================================================
void	CPSRibbonBase::setRibbonMode(TRibbonMode mode)
{
	NL_PS_FUNC(CPSRibbonBase_setRibbonMode)
	nlassert(mode < RibbonModeLast);
	_RibbonMode = mode;
}


//=======================================================
void	CPSRibbonBase::setInterpolationMode(TInterpolationMode mode)
{
	NL_PS_FUNC(CPSRibbonBase_setInterpolationMode)
	nlassert(mode < InterpModeLast);
	_InterpolationMode = mode;
}

//=======================================================
void	CPSRibbonBase::setTailNbSeg(uint32 nbSegs)
{
	NL_PS_FUNC(CPSRibbonBase_setTailNbSeg)
	nlassert(nbSegs >= 1);
	_NbSegs = nbSegs;
	_RibbonIndex = 0;
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}
	initDateVect();
}


//=======================================================
void	CPSRibbonBase::setSegDuration(TAnimationTime ellapsedTime)
{
	NL_PS_FUNC(CPSRibbonBase_setSegDuration)
	_SegDuration = ellapsedTime;

}

//=======================================================
void	CPSRibbonBase::updateGlobals()
{
	NL_PS_FUNC(CPSRibbonBase_updateGlobals)
	nlassert(!_Parametric);
	nlassert(_Owner);
	const uint size = _Owner->getSize();
	if (!size) return;
	const TAnimationTime currDate = _Owner->getOwner()->getSystemDate() + CParticleSystem::RealEllapsedTime;
	if (currDate  - _LastUpdateDate >= _SegDuration)
	{
		if (_RibbonIndex == 0) _RibbonIndex = _NbSegs + EndRibbonStorage;
		else --_RibbonIndex;

		/// decal date
		::memmove(&_SamplingDate[1], &_SamplingDate[0], sizeof(float) * (_NbSegs + EndRibbonStorage));
		_LastUpdateDate = currDate;
	}

	/// save current date
	_SamplingDate[0] = currDate;

	/// updating ribbons positions
	TPSMatrixMode mm = convertMatrixMode();
	if (mm == _Owner->getMatrixMode())
	{
		// trail reside in the same coord system -> no conversion needed
		TPSAttribVector::iterator posIt = _Owner->getPos().begin();
		NLMISC::CVector *currIt = &_Ribbons[_RibbonIndex];
		uint k = size;
		for (;;)
		{
			*currIt = *posIt;
			--k;
			if (!k) break;
			++posIt;
			currIt += (_NbSegs + 1 + EndRibbonStorage);
		}
	}
	else
	{
		nlassert(_Owner->getOwner());
		const CMatrix &mat = CPSLocated::getConversionMatrix(*_Owner->getOwner(), mm, _Owner->getMatrixMode());
		TPSAttribVector::iterator posIt = _Owner->getPos().begin();
		NLMISC::CVector *currIt = &_Ribbons[_RibbonIndex];
		uint k = size;
		for (;;)
		{
			*currIt = mat * *posIt;
			--k;
			if (!k) break;
			++posIt;
			currIt += (_NbSegs + 1 + EndRibbonStorage);
		}
	}
}


//=======================================================
void	CPSRibbonBase::computeHermitteRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{
	NL_PS_FUNC(CPSRibbonBase_CVector )
	nlassert(!_Parametric);
	NLMISC::CVector *startIt = &_Ribbons[(_NbSegs + 1 + EndRibbonStorage) * index];
	NLMISC::CVector *endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	NLMISC::CVector *currIt  = startIt + _RibbonIndex;
	const NLMISC::CVector *firstIt = currIt;
	NLMISC::CVector *nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	NLMISC::CVector *nextNextIt = nextIt + 1;
	if (nextNextIt == endIt) nextNextIt = startIt;
	float *date = &_SamplingDate[0];

	NLMISC::CVector t0 = (*nextIt - *currIt);
	NLMISC::CVector t1 = 0.5f * (*nextNextIt - *currIt);

	uint leftToDo = _UsedNbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;


	for (;;)
	{
		float dt = date[0] - date[1];

		if (dt < 10E-6f) // we reached the start of ribbon
		{

			do
			{
				*dest = *currIt;
				#ifdef NL_DEBUG
					nlassert(NLMISC::isValidDouble(dest->x));
					nlassert(NLMISC::isValidDouble(dest->y));
					nlassert(NLMISC::isValidDouble(dest->z));
				#endif
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			}
			while (--leftToDo);
			return;
		}

		float newLambdaStep = _UsedSegDuration / dt;
		// readapt lambda
		lambda *= newLambdaStep / lambdaStep;
		lambdaStep = newLambdaStep;
		for(;;)
		{
			if (lambda >= 1.f) break;
			/// compute a location
			BuildHermiteVector(*currIt, *nextIt, t0, t1, *dest, lambda);
			#ifdef NL_DEBUG
				nlassert(NLMISC::isValidDouble(dest->x));
				nlassert(NLMISC::isValidDouble(dest->y));
				nlassert(NLMISC::isValidDouble(dest->z));
			#endif
			dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			-- leftToDo;
			if (!leftToDo) return;
			lambda += lambdaStep;
		}

		++date;
		lambda -= 1.f;

		// Start new segment and compute new tangents
		t0 = t1;
		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;
		if (nextNextIt == firstIt)
		{
			t1 = *nextIt - *currIt;
		}
		else
		{
			t1 = 0.5f * (*nextNextIt - *currIt);
		}
	}
}

//=======================================================
void CPSRibbonBase::computeLinearRibbon(uint index, NLMISC::CVector *dest, uint stride)
{
	NL_PS_FUNC(CPSRibbonBase_computeLinearRibbon)
	nlassert(!_Parametric);
	NLMISC::CVector *startIt = &_Ribbons[(_NbSegs + 1 + EndRibbonStorage) * index];
	NLMISC::CVector *endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	NLMISC::CVector *currIt  = startIt + _RibbonIndex;
	NLMISC::CVector *nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	NLMISC::CVector *nextNextIt = nextIt + 1;
	if (nextNextIt == endIt) nextNextIt = startIt;
	float *date = &_SamplingDate[0];

	uint leftToDo = _UsedNbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;

	for (;;)
	{
		float dt = date[0] - date[1];

		if (dt < 10E-6f) // we reached the start of ribbon
		{
			do
			{
				*dest = *currIt;
				#ifdef NL_DEBUG
					nlassert(NLMISC::isValidDouble(dest->x));
					nlassert(NLMISC::isValidDouble(dest->y));
					nlassert(NLMISC::isValidDouble(dest->z));
				#endif
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);

			}
			while (--leftToDo);
			return;
		}


		float newLambdaStep = _UsedSegDuration / dt;
		// readapt lambda
		lambda *= newLambdaStep / lambdaStep;
		lambdaStep = newLambdaStep;

		float oneMinusLambda = 1.f - lambda;
		for(;;)
		{
			if (lambda >= 1.f) break;
			/// compute a location
			BuildLinearVector(*currIt, *nextIt, *dest, lambda, oneMinusLambda);
			#ifdef NL_DEBUG
				nlassert(NLMISC::isValidDouble(dest->x));
				nlassert(NLMISC::isValidDouble(dest->y));
				nlassert(NLMISC::isValidDouble(dest->z));
			#endif
			dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			-- leftToDo;
			if (!leftToDo) return;
			lambda += lambdaStep;
			oneMinusLambda -= lambdaStep;
		}

		++date;
		lambda -= 1.f;

		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;

	}
}

/*
void CPSRibbonBase::computeLinearRibbon(uint index, NLMISC::CVector *dest, uint stride)
{
	nlassert(!_Parametric);
	NLMISC::CVector *startIt = &_Ribbons[(_NbSegs + 1 + EndRibbonStorage) * index];
	NLMISC::CVector *endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	NLMISC::CVector *currIt  = startIt + _RibbonIndex;
	NLMISC::CVector *nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	NLMISC::CVector *nextNextIt = nextIt + 1;
	if (nextNextIt == endIt) nextNextIt = startIt;
	float *date = &_SamplingDate[0];

	uint leftToDo = _UsedNbSegs + 1;

	float lambda = 0.f;


	float dt = date[0] - date[1];
	if (dt < 10E-6f) // we reached the start of ribbon
	{
		do
		{
			*dest = *currIt;
			dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
		}
		while (--leftToDo);
		return;
	}
	float lambdaStep = _UsedSegDuration / dt;
	BuildLinearVector(*currIt, *nextIt, *dest, 0.f, 1.f);
	dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
	-- leftToDo;
	// snap lambda to nearest time step
	lambda = lambdaStep * fmodf(date[0], _UsedSegDuration) / _UsedSegDuration;
	for (;;)
	{
		float oneMinusLambda = 1.f - lambda;
		for(;;)
		{
			if (lambda >= 1.f) break;
			/// compute a location
			BuildLinearVector(*currIt, *nextIt, *dest, lambda, oneMinusLambda);
			dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			-- leftToDo;
			if (!leftToDo) return;
			lambda += lambdaStep;
			oneMinusLambda -= lambdaStep;
		}

		++date;
		lambda -= 1.f;

		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;
		float dt = date[0] - date[1];
		if (dt < 10E-6f) // we reached the start of ribbon
		{
			do
			{
				*dest = *currIt;
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			}
			while (--leftToDo);
			return;
		}
		float newLambdaStep = _UsedSegDuration / dt;
		// readapt lambda
		lambda *= newLambdaStep / lambdaStep;
		lambdaStep = newLambdaStep;
	}

}
*/


//=======================================================
void CPSRibbonBase::computeLinearCstSizeRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{
	NL_PS_FUNC(CPSRibbonBase_CVector )
	nlassert(!_Parametric);
	CVector *startIt = &_Ribbons[(_NbSegs + 1 + EndRibbonStorage) * index];
	NLMISC::CVector *endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	NLMISC::CVector *currIt  = startIt + _RibbonIndex;
	NLMISC::CVector *firstIt = currIt;
	NLMISC::CVector *nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	NLMISC::CVector *nextNextIt = nextIt + 1;
	if (nextNextIt == endIt) nextNextIt = startIt;

	uint leftToDo = _UsedNbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;


	/// Our goal here is to match the length of the ribbon, But if it isn't moving fast enough, we must truncate it
	for (;;)
	{
		/// compute length between the 2 sampling points
		const float sampleLength = (*nextIt - *currIt).norm();
		if (sampleLength > 10E-6f)
		{
			/// compute lambda so that it match the length needed for each segment
			float newLambdaStep = _UsedSegLength / sampleLength;
			// readapt lambda
			lambda *= newLambdaStep / lambdaStep;
			lambdaStep = newLambdaStep;

			float oneMinusLambda = 1.f - lambda;
			for(;;)
			{
				if (lambda >= 1.f) break;
				/// compute a location
				BuildLinearVector(*currIt, *nextIt, *dest, lambda, oneMinusLambda);
				#ifdef NL_DEBUG
					nlassert(NLMISC::isValidDouble(dest->x));
					nlassert(NLMISC::isValidDouble(dest->y));
					nlassert(NLMISC::isValidDouble(dest->z));
				#endif
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
				-- leftToDo;
				if (!leftToDo) return;
				lambda += lambdaStep;
				oneMinusLambda -= lambdaStep;
			}
			lambda -= 1.f;
		}

		/// go to next sampling pos
		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;
		if (nextNextIt == firstIt)
		{
			// The length of the sampling curve is too short
			// must truncate the ribbon.
			NLMISC::CVector &toDup = *nextIt;
			while (leftToDo --)
			{
				*dest = toDup;
				#ifdef NL_DEBUG
					nlassert(NLMISC::isValidDouble(dest->x));
					nlassert(NLMISC::isValidDouble(dest->y));
					nlassert(NLMISC::isValidDouble(dest->z));
				#endif
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			}
			return;
		}
	}
}

//=======================================================
void CPSRibbonBase::computeHermitteCstSizeRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{
	NL_PS_FUNC(CPSRibbonBase_CVector )
	nlassert(!_Parametric);
	NLMISC::CVector *startIt = &_Ribbons[(_NbSegs + 1 + EndRibbonStorage) * index];
	NLMISC::CVector *endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	NLMISC::CVector *currIt  = startIt + _RibbonIndex;
	NLMISC::CVector *firstIt = currIt;
	NLMISC::CVector *nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	NLMISC::CVector *nextNextIt = nextIt + 1;
	if (nextNextIt == endIt) nextNextIt = startIt;

	NLMISC::CVector t0 = (*nextIt - *currIt);
	NLMISC::CVector t1 = 0.5f * (*nextNextIt - *currIt);

	uint leftToDo = _UsedNbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;


	/// Our goal here is to match the length of the ribbon, But if it isn't moving fast enough, we must truncate it
	/// Having a constant speed over a hermite curve is expensive, so we make a (very) rough approximation...
	for (;;)
	{
		/// compute length between the 2 sampling points
		const float sampleLength = (*nextIt - *currIt).norm();
		if (sampleLength > 10E-6f)
		{
			/// compute lambda so that it match the length needed for each segment
			float newLambdaStep = _UsedSegLength / sampleLength;
			// readapt lambda
			lambda *= newLambdaStep / lambdaStep;
			lambdaStep = newLambdaStep;

			for(;;)
			{
				if (lambda >= 1.f) break;
				/// compute a location
				BuildHermiteVector(*currIt, *nextIt, t0, t1, *dest, lambda);
				#ifdef NL_DEBUG
					nlassert(NLMISC::isValidDouble(dest->x));
					nlassert(NLMISC::isValidDouble(dest->y));
					nlassert(NLMISC::isValidDouble(dest->z));
				#endif

				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
				-- leftToDo;
				if (!leftToDo) return;
				lambda += lambdaStep;
			}
			lambda -= 1.f;
		}

		/// go to next sampling pos
		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;
		if (nextNextIt == firstIt)
		{
			// The length of the sampling curve is too short
			// must truncate the ribbon.
			NLMISC::CVector &toDup = *nextIt;
			while (leftToDo --)
			{
				*dest = toDup;
				#ifdef NL_DEBUG
					nlassert(NLMISC::isValidDouble(dest->x));
					nlassert(NLMISC::isValidDouble(dest->y));
					nlassert(NLMISC::isValidDouble(dest->z));
				#endif
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			}
			return;
		}
		/// update tangents
		t0 = t1;
		t1 = 0.5f * (*nextNextIt - *currIt);
	}
}


//=======================================================
void CPSRibbonBase::computeRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{
	NL_PS_FUNC(CPSRibbonBase_CVector )
	switch (_InterpolationMode)
	{
		case Linear:
			if (_RibbonMode == VariableSize)
			{
				computeLinearRibbon(index, dest, stride);
			}
			else
			{
				computeLinearCstSizeRibbon(index, dest, stride);
			}
		break;
		case Hermitte:
			if (_RibbonMode == VariableSize)
			{
				computeHermitteRibbon(index, dest, stride);

			}
			else
			{
				computeHermitteCstSizeRibbon(index, dest, stride);
			}
		break;
		default:
			nlassert(0);
		break;
	}
}


//=======================================================
void	CPSRibbonBase::dupRibbon(uint dest, uint src)
{
	NL_PS_FUNC(CPSRibbonBase_dupRibbon)
	nlassert(!_Parametric);
	nlassert(_Owner);
	const uint size = _Owner->getSize();
	nlassert(dest < size && src < size);
	::memcpy(&_Ribbons[dest * (_NbSegs + EndRibbonStorage + 1)], &_Ribbons[src * (_NbSegs + EndRibbonStorage + 1)], sizeof(NLMISC::CVector) * (_NbSegs + 1 + EndRibbonStorage));
}

//=======================================================
void	CPSRibbonBase::newElement(const CPSEmitterInfo &info)
{
	NL_PS_FUNC(CPSRibbonBase_newElement)
	if (_Parametric) return;
	/// dump the same pos for all pos of the ribbon
	const uint index = _Owner->getNewElementIndex();
	const NLMISC::CVector &pos = _Owner->getPos()[index]; // get the pos of the new element;
	resetSingleRibbon(index, pos);
}

//=======================================================
void	CPSRibbonBase::deleteElement(uint32 index)
{
	NL_PS_FUNC(CPSRibbonBase_deleteElement)
	if (_Parametric) return;
	const uint32 size = _Owner->getSize();
	if(index == (size - 1)) return; // was the last element, no permutation needed.
	dupRibbon(index, size - 1);
}

//=======================================================
void	CPSRibbonBase::resize(uint32 size)
{
	NL_PS_FUNC(CPSRibbonBase_resize)
	nlassert(size < (1 << 16));
	if (_Parametric) return;
	_Ribbons.resize(size * (_NbSegs + 1 + EndRibbonStorage));
	resetFromOwner();
}


//=======================================================
void CPSRibbonBase::resetSingleRibbon(uint index, const NLMISC::CVector &pos)
{
	NL_PS_FUNC(CPSRibbonBase_resetSingleRibbon)
	nlassert(!_Parametric);
	TPSMatrixMode mm = convertMatrixMode();
	NLMISC::CVector *it = &_Ribbons[(index * (_NbSegs + 1 + EndRibbonStorage))];
	if (mm == _Owner->getMatrixMode())
	{
		std::fill(it, it + (_NbSegs + 1 + EndRibbonStorage), pos);
	}
	else
	{
		nlassert(_Owner->getOwner());
		const CMatrix &mat = CPSLocated::getConversionMatrix(*_Owner->getOwner(), mm, _Owner->getMatrixMode());
		std::fill(it, it + (_NbSegs + 1 + EndRibbonStorage), mat * pos);
	}
}



//=======================================================
void CPSRibbonBase::resetFromOwner()
{
	NL_PS_FUNC(CPSRibbonBase_resetFromOwner)
	nlassert(!_Parametric);
	TPSAttribVector::iterator posIt = _Owner->getPos().begin();
	TPSAttribVector::iterator endPosIt = _Owner->getPos().end();
	for (uint k = 0; posIt != endPosIt; ++posIt, ++k)
	{
		resetSingleRibbon(k, *posIt);
	}
}

//=======================================================
void CPSRibbonBase::motionTypeChanged(bool parametric)
{
	NL_PS_FUNC(CPSRibbonBase_motionTypeChanged)
	_Parametric = parametric;
	if (parametric)
	{
		NLMISC::contReset(_Ribbons); // kill the vector
	}
	else
	{
		nlassert(_Owner);
		resize(_Owner->getMaxSize());
		initDateVect();
		resetFromOwner();
	}
}


//=======================================================
void CPSRibbonBase::initDateVect()
{
	NL_PS_FUNC(CPSRibbonBase_initDateVect)
	_SamplingDate.resize( _NbSegs + 1 + EndRibbonStorage);
	std::fill(_SamplingDate.begin(), _SamplingDate.begin() + (_NbSegs + 1 + EndRibbonStorage), 0.f);
}


//=======================================================
void CPSRibbonBase::serial(NLMISC::IStream &f)
{
	NL_PS_FUNC(CPSRibbonBase_serial)
	CPSParticle::serial(f);
	// version 2 : added matrix mode
	sint ver = f.serialVersion(2);
	f.serialEnum(_RibbonMode);
	f.serialEnum(_InterpolationMode);
	f.serial(_NbSegs, _SegDuration);
	if (_RibbonMode == FixedSize)
	{
		f.serial(_RibbonLength);
		if (f.isReading())
		{
			_SegLength = _RibbonLength / _NbSegs;
		}
	}
	if (f.isReading())
	{
		if (_Owner)
		{
			resize(_Owner->getMaxSize());
			initDateVect();
			resetFromOwner();
		}
	}
	if (ver >= 1)
	{
		f.serial(_LODDegradation);
	}
	if (ver >= 2)
	{
		f.serialEnum(_MatrixMode);
	}
}


//=======================================================
void CPSRibbonBase::updateLOD()
{
	NL_PS_FUNC(CPSRibbonBase_updateLOD)
	nlassert(_Owner);
	float ratio = _Owner->getOwner()->getOneMinusCurrentLODRatio();
	float squaredRatio = ratio * ratio;
	float lodRatio = _LODDegradation + (1.f - _LODDegradation ) * squaredRatio * squaredRatio * squaredRatio;

	_UsedNbSegs = (uint) (_NbSegs * lodRatio);
	NLMISC::clamp(_UsedNbSegs, 0u, _NbSegs);
	const float epsilon = 10E-4f;
	_UsedSegDuration =  _SegDuration / std::max(epsilon, lodRatio);
	_UsedSegLength   =  _SegLength / std::max(epsilon, lodRatio);

}

//=======================================================
void CPSRibbonBase::systemDateChanged()
{
	NL_PS_FUNC(CPSRibbonBase_systemDateChanged)
	nlassert(_Owner->getOwner());
	_Owner->getOwner()->getSystemDate();
	float date = _Owner->getOwner()->getSystemDate();
	std::fill(_SamplingDate.begin(), _SamplingDate.end(), date);
	_LastUpdateDate = date;
}



} // NL3D

