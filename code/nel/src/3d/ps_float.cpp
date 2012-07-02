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

#include "nel/3d/ps_float.h"
#include "nel/3d/ps_register_float_attribs.h"
#include "nel/misc/fast_floor.h"

namespace NL3D {


/////////////////////////////////////
// CPSFloatGradient implementation //
/////////////////////////////////////

CPSFloatGradient::CPSFloatGradient(const float *floatTab, uint32 nbValues, uint32 nbStages, float nbCycles)
				: CPSValueGradient<float>(nbCycles)
{
	_F.setValues(floatTab, nbValues, nbStages) ;
}


////////////////////////////////////////////
// CPSFloatBezierCurve implementation     //
////////////////////////////////////////////

CPSFloatCurveFunctor::CPSFloatCurveFunctor() : _NumSamples(0), _Smoothing(true)
{
	/*
	_CtrlPoints.push_back(CCtrlPoint(0, 0.5f));
	_CtrlPoints.push_back(CCtrlPoint(1, 0.5f));
	updateTab();
	*/
}

///=======================================================================================
void CPSFloatCurveFunctor::sortPoints(void)
{
	std::sort(_CtrlPoints.begin(), _CtrlPoints.end());
}

///=======================================================================================
void CPSFloatCurveFunctor::addControlPoint(const CCtrlPoint &ctrlPoint)
{
	_CtrlPoints.push_back(ctrlPoint);
	sortPoints();
	updateTab();
}

///=======================================================================================
const CPSFloatCurveFunctor::CCtrlPoint &CPSFloatCurveFunctor::getControlPoint(uint index) const
{
	return _CtrlPoints[index];
}

///=======================================================================================
void CPSFloatCurveFunctor::setCtrlPoint(uint index, const CCtrlPoint &ctrlPoint)
{
	nlassert(ctrlPoint.Date >= 0 && ctrlPoint.Date <= 1);
	_CtrlPoints[index] = ctrlPoint;
	sortPoints();
	updateTab();
}

///=======================================================================================
void CPSFloatCurveFunctor::removeCtrlPoint(uint index)
{
	nlassert(_CtrlPoints.size() > 1);
	_CtrlPoints.erase(_CtrlPoints.begin() + index);
	updateTab();
}

///=======================================================================================
void CPSFloatCurveFunctor::setNumSamples(uint32 numSamples)
{
	nlassert(numSamples > 0);
	_NumSamples = numSamples;
	updateTab();
}

///=======================================================================================
float CPSFloatCurveFunctor::getValue(float date) const
{
	if (_CtrlPoints.empty()) return 0.f;
	NLMISC::clamp(date, 0, 1);
	// find a key that has a higher value
	CPSVector<CCtrlPoint>::V::const_iterator it = _CtrlPoints.begin();
	while ( it != _CtrlPoints.end() && it->Date <= date ) ++it;

	if (it == _CtrlPoints.begin()) return _CtrlPoints[0].Value;
	if (it == _CtrlPoints.end()) return _CtrlPoints[_CtrlPoints.size() - 1].Value;
	CPSVector<CCtrlPoint>::V::const_iterator precIt = it - 1;
	if (precIt->Date == it->Date) return 0.5f * (precIt->Value + it->Value);
	const float lambda = (date - precIt->Date) / (it->Date - precIt->Date);
	if (!_Smoothing) // linear interpolation
	{
		return lambda * it->Value + (1.f - lambda) * precIt->Value;
	}
	else // hermite interpolation
	{
		float width = it->Date - precIt->Date;
		uint index = (uint)(precIt - _CtrlPoints.begin());
		float t1 = getSlope(index) * width, t2 = getSlope(index + 1) * width;
		const float lambda2 = NLMISC::sqr(lambda);
		const float lambda3 = lambda2 * lambda;
		const float h1 = 2 * lambda3 - 3 * lambda2 + 1;
		const float h2 = - 2 * lambda3 + 3 * lambda2;
		const float h3 = lambda3 - 2 * lambda2 + lambda;
		const float h4 = lambda3 - lambda2;

		return h1 * precIt->Value + h2 * it->Value + h3 * t1 + h4 * t2;
	}
}

///=======================================================================================
void CPSFloatCurveFunctor::updateTab(void)
{
	float step  = 1.f / _NumSamples;
	float d = 0.f;
	_Tab.resize(_NumSamples + 1);
	uint k;
	for (k = 0; k <= _NumSamples; ++k)
	{
		_Tab[k] = getValue(d);
		d += step;
	}
	_MinValue = _MaxValue = _Tab[0];
	for (k = 1; k <= _NumSamples; ++k)
	{
		_MinValue = std::min(_MinValue, _Tab[k]);
		_MaxValue = std::max(_MaxValue, _Tab[k]);
	}
}

///=======================================================================================
void CPSFloatCurveFunctor::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	f.serial(_NumSamples, _Smoothing);
	f.serialCont(_CtrlPoints);
	if (f.isReading())
	{
		updateTab();
	}
}

///=======================================================================================
float CPSFloatCurveFunctor::getSlope(uint index) const
{
	// tangent for first point
	if (index == 0)
	{
		return _CtrlPoints[1].Date != _CtrlPoints[0].Date ? (_CtrlPoints[1].Value - _CtrlPoints[0].Value)
															 / (_CtrlPoints[1].Date - _CtrlPoints[0].Date)
														  : 1e6f;
	}

	// tangent for last point
	if (index == _CtrlPoints.size() - 1)
	{
		return _CtrlPoints[index].Date != _CtrlPoints[index - 1].Date ? (_CtrlPoints[index].Value - _CtrlPoints[index - 1].Value)
																		/ (_CtrlPoints[index].Date - _CtrlPoints[index - 1].Date)
																	  : 1e6f;
	}

	// tangent for other points
	return _CtrlPoints[index + 1].Date != _CtrlPoints[index - 1].Date ? (_CtrlPoints[index + 1].Value - _CtrlPoints[index - 1].Value)
																		/ (_CtrlPoints[index + 1].Date - _CtrlPoints[index - 1].Date)
																	  : 1e6f;
}

///=======================================================================================
void CPSFloatCurveFunctor::enableSmoothing(bool enable /* = true*/)
{
	_Smoothing = enable;
	updateTab();
}

///=======================================================================================
void PSRegisterFloatAttribs()
{
	NLMISC_REGISTER_CLASS(CPSFloatBlender);
	NLMISC_REGISTER_CLASS(CPSFloatGradient);
	NLMISC_REGISTER_CLASS(CPSFloatMemory);
	NLMISC_REGISTER_CLASS(CPSFloatBinOp);
	NLMISC_REGISTER_CLASS(CPSFloatCurve);
}

} // NL3D
