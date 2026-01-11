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
#include "nel/misc/mouse_smoother.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// *******************************************************************************************
/// build some hermite spline value, with the given points and tangents
static inline void BuildHermiteVector(const NLMISC::CVector2f &P0,
									  const NLMISC::CVector2f &P1,
									  const NLMISC::CVector2f &T0,
									  const NLMISC::CVector2f &T1,
									  NLMISC::CVector2f &dest,
									  float lambda
									  )
{
	const float lambda2 = lambda * lambda;
	const float lambda3 = lambda2 * lambda;
	const float h1 = 2 * lambda3 - 3 * lambda2 + 1;
	const float h2 = - 2 * lambda3 + 3 * lambda2;
	const float h3 = lambda3 - 2 * lambda2 + lambda;
	const float h4 = lambda3 - lambda2;
	/// just avoid some ctor calls here...
	dest.set(h1 * P0.x + h2 * P1.x + h3 * T0.x + h4 * T1.x,
		h1 * P0.y + h2 * P1.y + h3 * T0.y + h4 * T1.y);
}

// *******************************************************************************************
CMouseSmoother::CMouseSmoother(double samplingPeriod /*=0.2f*/)
{
	nlassert(samplingPeriod > 0);
	_SamplingPeriod = samplingPeriod;
	_Init = false;
}


// *******************************************************************************************
void CMouseSmoother::setSamplingPeriod(double period)
{
	if (period == _SamplingPeriod) return;
	reset();
	nlassert(_SamplingPeriod > 0);
	_SamplingPeriod = period;
}


// *******************************************************************************************
NLMISC::CVector2f CMouseSmoother::samplePos(const CVector2f &wantedPos, double date)
{
	if (!_Init)
	{
		_Sample[0] = _Sample[1] = _Sample[2] = _Sample[3] = CSample(date, wantedPos);
		_Init = true;
	}
	else
	{
		// see if enough time has elapsed since last sample
		if (date - _Sample[3].Date >= _SamplingPeriod)
		{
			uint numSamples = (uint) floor((date - _Sample[3].Date) / _SamplingPeriod);
			numSamples = std::min(numSamples, (uint) 4);
			for(uint k = 0; k < numSamples; ++k)
			{
				// add a new sample
				_Sample[0] = _Sample[1];
				_Sample[1] = _Sample[2];
				_Sample[2] = _Sample[3];
				_Sample[3] = CSample(date, wantedPos);
			}
		}
		else if (date == _Sample[3].Date)
		{
			// update cur pos
			_Sample[3] = CSample(date, wantedPos);
		}
	}
	if (_Sample[1].Pos.x == _Sample[2].Pos.x &&
		_Sample[1].Pos.y == _Sample[2].Pos.y
	   )
	{
		// special case : if pointer hasn't moved, allow a discontinuity of speed
		return _Sample[2].Pos;
	}
	double evalDate = date - 2 * _SamplingPeriod;
	clamp(evalDate, _Sample[1].Date, _Sample[2].Date);
	CVector2f t0;
	double dt = _Sample[2].Date - _Sample[1].Date;
	if (_Sample[2].Date != _Sample[0].Date)
	{
		t0 = (float) dt * (_Sample[2].Pos - _Sample[0].Pos) / (float) (_Sample[2].Date - _Sample[0].Date);
	}
	else
	{
		t0= NLMISC::CVector::Null;
	}
	CVector2f t1;
	if (_Sample[3].Date != _Sample[1].Date)
	{
		t1 = (float) dt * (_Sample[3].Pos - _Sample[1].Pos) / (float) (_Sample[3].Date - _Sample[1].Date);
	}
	else
	{
		t1= NLMISC::CVector::Null;
	}
	NLMISC::CVector2f result;
	if (dt == 0) return _Sample[2].Pos;
	BuildHermiteVector(_Sample[1].Pos, _Sample[2].Pos, t0, t1, result, (float) ((evalDate - _Sample[1].Date) / dt));
	return result;
}

// *******************************************************************************************
void CMouseSmoother::reset()
{
	_Init = false;
}

} // NLMISC
