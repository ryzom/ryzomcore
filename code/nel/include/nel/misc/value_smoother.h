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

#ifndef NL_VALUE_SMOOTHER_H
#define NL_VALUE_SMOOTHER_H

#include "types_nl.h"

#include <vector>


namespace NLMISC {


// ***************************************************************************
/**
 * A smoother of values.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
template <class T>
class CValueSmootherTemplate
{
public:

	/// Constructor
	explicit CValueSmootherTemplate(uint n=16)
	{
		init(n);
	}

	/// reset the ValueSmoother, and set the number of frame to smooth.
	void		init(uint n)
	{
		// reset all the array to 0.
		_LastFrames.clear();

		if (n > 0)
			_LastFrames.resize(n, 0);

		_CurFrame = 0;
		_NumFrame = 0;
		_FrameSum = 0;
		_CurrentValue = 0;
	}

	/// reset only the ValueSmoother
	void		reset()
	{
		std::fill(_LastFrames.begin(), _LastFrames.end(), T(0));

		_CurFrame = 0;
		_NumFrame = 0;
		_FrameSum = 0;
		_CurrentValue = 0;
	}

	/// add a new value to be smoothed.
	void		addValue(T dt)
	{
		if (_LastFrames.empty())
			return;

		// update the frame sum. NB: see init(), at start, array is full of 0. so it works even for un-inited values.
		_FrameSum-= _LastFrames[_CurFrame];
		_FrameSum+= dt;

		// backup this value in the array.
		_LastFrames[_CurFrame]= dt;
		_CurrentValue = dt;

		// next frame.
		_CurFrame++;
//		_CurFrame%=_LastFrames.size();
		if (_CurFrame >= _LastFrames.size())
			_CurFrame -= (uint)_LastFrames.size();

		// update the number of frames added.
		_NumFrame++;
		_NumFrame= std::min(_NumFrame, (uint)_LastFrames.size());
	}

	/// get the smoothed value.
	T		getSmoothValue() const
	{
		if(_NumFrame>0)
			return T(_FrameSum) / T(_NumFrame);
		else
			return T(0);
	}

	T		getCurrentValue() const
	{
		if (_NumFrame>0)
			return T(_CurrentValue);
		else
			return T(0);
	}

	uint getNumFrame() const
	{
		return _NumFrame;
	}

	uint getCurrentFrame() const
	{
		return _CurFrame;
	}

	const std::vector<T> &getLastFrames() const
	{
		return _LastFrames;
	}

private:
	std::vector<T>			_LastFrames;
	uint					_CurFrame;
	uint					_NumFrame;
	T						_FrameSum;
	T						_CurrentValue;
};

// ***************************************************************************
/**
 * A smoother replacement for boolean.
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2003
 */
template <>
class CValueSmootherTemplate<bool>
{
public:

	/// Constructor
	explicit CValueSmootherTemplate(uint n=1)
	{
		init(n);
	}

	/// reset the ValueSmoother, and set the number of frame to smooth.
	void		init(uint n)
	{
		// reset all the array to 0.
		_LastFrames.clear();

		if (n>0)
			_LastFrames.resize(n, false);

		_NumFrame = 0;
	}

	/// reset only the ValueSmoother
	void		reset()
	{
		std::fill(_LastFrames.begin(), _LastFrames.end(), false);
 		_NumFrame = 0;
	}

	/// add a new value to be smoothed.
	void		addValue(bool dt)
	{
		if(_NumFrame>0)
			_LastFrames[0] = dt;
	}

	/// get the smoothed value.
	bool		getSmoothValue() const
	{
		if(_NumFrame>0)
			return _LastFrames[0];
		else
			return false;
	}

	uint getNumFrame() const
	{
		return _NumFrame;
	}

	uint getCurrentFrame() const
	{
		return 0;
	}

	const std::vector<bool> &getLastFrames() const
	{
		return _LastFrames;
	}

private:
	std::vector<bool>       _LastFrames;
	uint            		_NumFrame;
};

class CValueSmoother : public CValueSmootherTemplate<float>
{
public:
	/// Constructor
	explicit CValueSmoother(uint n=16) : CValueSmootherTemplate<float>(n)
	{
	}
};

} // NLMISC


#endif // NL_VALUE_SMOOTHER_H

/* End of value_smoother.h */
