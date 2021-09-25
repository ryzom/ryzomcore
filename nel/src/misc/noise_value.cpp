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

#include "nel/misc/noise_value.h"
#include "nel/misc/fast_floor.h"


#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{


// 3 level: best quality/speed ratio.
#define	NL3D_NOISE_LEVEL			3
#define	NL3D_NOISE_GRID_SIZE_SHIFT	5
#define	NL3D_NOISE_GRID_SIZE		(1<<NL3D_NOISE_GRID_SIZE_SHIFT)
static	const float NL3D_OO255= 1.0f / 255;

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
/// A static 3D array of random value + other infos for noise
class	CRandomGrid3D
{
public:

	// generate a random grid, with same seed.
	CRandomGrid3D()
	{
		//seed
		srand(0);

		// init the grid
		for(uint z=0; z<NL3D_NOISE_GRID_SIZE; z++)
		{
			for(uint y=0; y<NL3D_NOISE_GRID_SIZE; y++)
			{
				for(uint x=0; x<NL3D_NOISE_GRID_SIZE; x++)
				{
					uint	id= x + (y<<NL3D_NOISE_GRID_SIZE_SHIFT) + (z<<(NL3D_NOISE_GRID_SIZE_SHIFT*2));
					// take higher bits of rand gives better result.
					uint	v= rand() >> 5;
					_Texture3d[id]= v&255;
				}
			}
		}

		// init sizes.
		uint	i;
		// sum of sizes must be 1, and each level must be /2.
		float	sizeSum=0;
		for(i=0; i<NL3D_NOISE_LEVEL; i++)
		{
			_Sizes[i]= 1.0f / (1<<i);
			sizeSum+= _Sizes[i];
		}
		// normalize
		for(i=0; i<NL3D_NOISE_LEVEL; i++)
		{
			_Sizes[i]/= sizeSum;
		}

		// init LevelPhases.
		for(i=0; i<NL3D_NOISE_LEVEL; i++)
		{
			_LevelPhase[i].x= frand(NL3D_NOISE_GRID_SIZE);
			_LevelPhase[i].y= frand(NL3D_NOISE_GRID_SIZE);
			_LevelPhase[i].z= frand(NL3D_NOISE_GRID_SIZE);
		}
		// not for level 0.
		_LevelPhase[0]= CVector::Null;
	}

	// x/y/z are use to lookup directly in the grid 3D.
	static inline float	evalNearest(const CVector &pos)
	{
		// compute integer part.
		sint	x= OptFastFloor(pos.x);
		sint	y= OptFastFloor(pos.y);
		sint	z= OptFastFloor(pos.z);
		// index in texture.
		uint	ux= x& (NL3D_NOISE_GRID_SIZE-1);
		uint	uy= y& (NL3D_NOISE_GRID_SIZE-1);
		uint	uz= z& (NL3D_NOISE_GRID_SIZE-1);

		// read the texture.
		float	turb= lookup(ux,uy,uz);

		return turb*NL3D_OO255;
	}

	// x/y/z are use to lookup directly in the grid 3D.
	static inline float	evalBiLinear(const CVector &pos)
	{
		// compute integer part.
		sint	x= OptFastFloor(pos.x);
		sint	y= OptFastFloor(pos.y);
		sint	z= OptFastFloor(pos.z);
		// index in texture.
		uint	ux= x& (NL3D_NOISE_GRID_SIZE-1);
		uint	uy= y& (NL3D_NOISE_GRID_SIZE-1);
		uint	uz= z& (NL3D_NOISE_GRID_SIZE-1);
		uint	ux2= (x+1)& (NL3D_NOISE_GRID_SIZE-1);
		uint	uy2= (y+1)& (NL3D_NOISE_GRID_SIZE-1);
		uint	uz2= (z+1)& (NL3D_NOISE_GRID_SIZE-1);
		// delta.
		float	dx2;
		float	dy2;
		float	dz2;
		easeInEaseOut(dx2, pos.x-x);
		easeInEaseOut(dy2, pos.y-y);
		easeInEaseOut(dz2, pos.z-z);
		float	dx= 1-dx2;
		float	dy= 1-dy2;
		float	dz= 1-dz2;
		// TriLinear in texture3D.
		float	turb=0;
		float	dxdy= dx*dy;
		turb+= lookup(ux,uy,uz)* dxdy*dz;
		turb+= lookup(ux,uy,uz2)* dxdy*dz2;
		float	dxdy2= dx*dy2;
		turb+= lookup(ux,uy2,uz)* dxdy2*dz;
		turb+= lookup(ux,uy2,uz2)* dxdy2*dz2;
		float	dx2dy= dx2*dy;
		turb+= lookup(ux2,uy,uz)* dx2dy*dz;
		turb+= lookup(ux2,uy,uz2)* dx2dy*dz2;
		float	dx2dy2= dx2*dy2;
		turb+= lookup(ux2,uy2,uz)* dx2dy2*dz;
		turb+= lookup(ux2,uy2,uz2)* dx2dy2*dz2;

		// End!
		return turb*NL3D_OO255;
	}


	// get size according to level
	static inline float	getLevelSize(uint level)
	{
		return _Sizes[level];
	}

	// get an additional level phase.
	static inline const CVector	&getLevelPhase(uint level)
	{
		return _LevelPhase[level];
	}


// **************
private:

	static	uint8		_Texture3d[NL3D_NOISE_GRID_SIZE*NL3D_NOISE_GRID_SIZE*NL3D_NOISE_GRID_SIZE];
	static	float		_Sizes[NL3D_NOISE_LEVEL];
	static	CVector		_LevelPhase[NL3D_NOISE_LEVEL];


	// lookup with no mod.
	static inline float	lookup(uint ux, uint uy, uint uz)
	{
		uint	id= ux + (uy<<NL3D_NOISE_GRID_SIZE_SHIFT) + (uz<<(NL3D_NOISE_GRID_SIZE_SHIFT*2));
		return	_Texture3d[id];
	}

	// easineasout
	static inline void	easeInEaseOut(float &y, float x)
	{
		// cubic such that f(0)=0, f'(0)=0, f(1)=1, f'(1)=0.
		float	x2=x*x;
		float	x3=x2*x;
		y= -2*x3 + 3*x2;
	}

};


uint8		CRandomGrid3D::_Texture3d[NL3D_NOISE_GRID_SIZE*NL3D_NOISE_GRID_SIZE*NL3D_NOISE_GRID_SIZE];
float		CRandomGrid3D::_Sizes[NL3D_NOISE_LEVEL];
CVector		CRandomGrid3D::_LevelPhase[NL3D_NOISE_LEVEL];

// just to init the static arrays.
static	CRandomGrid3D	NL3D_RandomGrid3D;


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
float	CNoiseValue::evalRandom(const CVector &pos) const
{
	return CRandomGrid3D::evalNearest(pos);
}


// ***************************************************************************
float	CNoiseValue::noise(const CVector &pos) const
{
	// eval "fractaly".
	float		turb;

#if (NL3D_NOISE_LEVEL != 3)
	CVector		vd= pos;
	turb=0;
	for(uint level=0;level<NL3D_NOISE_LEVEL;level++)
	{
		// Add the influence of the ith level.
		turb+= CRandomGrid3D::getLevelSize(level) *
			CRandomGrid3D::evalBiLinear(vd + CRandomGrid3D::getLevelPhase(level) );
		// Next level at higher frequency
		vd*= 2;
	}
#else
	// special case. unrolled loop.
	// level 0 has no phase.
	turb= CRandomGrid3D::getLevelSize(0) *
		CRandomGrid3D::evalBiLinear(pos);
	// level 1
	turb+= CRandomGrid3D::getLevelSize(1) *
		CRandomGrid3D::evalBiLinear(pos*2 + CRandomGrid3D::getLevelPhase(1) );
	// level 2
	turb+= CRandomGrid3D::getLevelSize(2) *
		CRandomGrid3D::evalBiLinear(pos*4 + CRandomGrid3D::getLevelPhase(2) );
#endif

	return turb;
}



// ***************************************************************************
CNoiseValue::CNoiseValue()
{
	Abs= 0;
	Rand= 1;
	Frequency= 1;
}


// ***************************************************************************
CNoiseValue::CNoiseValue(float abs, float rand, float freq)
{
	Abs= abs;
	Rand= rand;
	Frequency= freq;
}


// ***************************************************************************
float	CNoiseValue::eval(const CVector &posInWorld) const
{
	// A single cube in the Grid3d correspond to Frequency==1.
	// So enlarging size of the grid3d do not affect the frequency aspect.
	return Abs + Rand * noise(posInWorld*Frequency);
}


// ***************************************************************************
float	CNoiseValue::evalOneLevelRandom(const CVector &posInWorld) const
{
	// A single cube in the Grid3d correspond to Frequency==1.
	// So enlarging size of the grid3d do not affect the frequency aspect.
	return Abs + Rand * evalRandom(posInWorld*Frequency);
}


// ***************************************************************************
void	CNoiseValue::serial(IStream &f)
{
	(void)f.serialVersion(0);
	f.serial(Abs);
	f.serial(Rand);
	f.serial(Frequency);
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CNoiseColorGradient::eval(const CVector &posInWorld, CRGBAF &result) const
{
	// test if not null grads.
	uint	nGrads= (uint)Gradients.size();
	if(nGrads==0)
		return;
	// if only one color, easy
	if(nGrads==1)
	{
		result= Gradients[0];
	}
	else
	{
		// eval noise
		float	f= NoiseValue.eval(posInWorld) * (nGrads-1);
		clamp(f, 0.f, (float)(nGrads-1));
		// look up in table of gradients.
		uint	id= OptFastFloor(f);
		clamp(id, 0U, nGrads-2);
		// fractionnal part.
		f= f-id;
		clamp(f, 0, 1);
		// interpolate the gradient.
		result= Gradients[id]*(1-f) + Gradients[id+1]*f;
	}
}

// ***************************************************************************
void	CNoiseColorGradient::serial(IStream &f)
{
	(void)f.serialVersion(0);
	f.serial(NoiseValue);
	f.serialCont(Gradients);
}



} // NLMISC
