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

#ifndef NL_NOISE_VALUE_H
#define NL_NOISE_VALUE_H

#include "types_nl.h"
#include "vector.h"
#include "stream.h"
#include "rgba.h"


namespace NLMISC
{





// ***************************************************************************
/**
 * A class which generate noisy value, according to a position
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CNoiseValue
{
public:
	float	Abs;
	float	Rand;
	float	Frequency;

public:
	/// Default to 0, 1, 1.
	CNoiseValue();
	CNoiseValue(float abs, float rand, float freq);

	/** return Abs + Rand* noise(Pos*Frequency).  with noise() E [0..1].
	 *	Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 */
	float	eval(const CVector &posInWorld) const;

	/** same as eval, but eval just one random level for noise() => act much more like a random.
	 *	Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 */
	float	evalOneLevelRandom(const CVector &posInWorld) const;


	void	serial(IStream &f);

// *******************
private:
	/// pos scale is in [0..1]
	float	noise(const CVector &pos) const;
	float	evalRandom(const CVector &pos) const;
};


// ***************************************************************************
/**
 * A noisy color generator
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CNoiseColorGradient
{
public:
	/// Abs and Rand should be 0 and 1 here. If not, some colors may not be generated...
	CNoiseValue					NoiseValue;
	std::vector<CRGBAF>	Gradients;


public:
	/** Use NoiseValue to compute a PerlinNoise E [0..1], and peek in Gradients, with linear interpolation.
	 *	result unmodified if no colors. If only one color, copied into result.
	 *	Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 */
	void	eval(const CVector &posInWorld, CRGBAF &result) const;

	void	serial(IStream &f);
};



} // NL3D


#endif // NL_NOISE_VALUE_H

/* End of noise_value.h */
