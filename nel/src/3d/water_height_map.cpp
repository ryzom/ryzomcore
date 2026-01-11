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

#include "nel/3d/water_height_map.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/vector_2f.h"
#include <algorithm>
#include <cmath>


#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

//===========================================================================================

CWaterHeightMap::CWaterHeightMap() : Date(-1),
									 _WavesEnabled(false),
									 _Damping(0.97f),
									 _FilterWeight(4),
									 _UnitSize(0.6f),
									 _WaveIntensity(0),
									 _WavePeriod(0),
									 _WaveImpulsionRadius(3),
									 _BorderWaves(true),
									 _EmitEllapsedTime(0),
									 _PropagateEllapsedTime(0),
									 _PropagationTime(0.10f),
									 _X(0),
									 _Y(0),
									 _NewX(0),
									 _NewY(0),
									 _CurrMap(0),
									 _Size(0)
{
}


//===========================================================================================

void	CWaterHeightMap::setPropagationTime(float time)
{
	_PropagationTime = time;
	_PropagateEllapsedTime = 0;
	for (uint k = 0; k < NumWaterMap; ++k)
	{
		clearArea(k, 0, 0, _Size << 1, _Size << 1);
	}
}

//===========================================================================================

void CWaterHeightMap::updateUserPos()
{
	const sint x = _NewX;
	const sint y = _NewY;

	nlassert(_Size != 0);
	if ((uint) x == _X && (uint) y == _Y) return;
	if ((uint) std::abs((sint)(x - _X)) < _Size && (uint) std::abs((sint)(y - _Y)) < _Size) // are there common pixels with the previous location?
	{
		// compute zone

		sint XDivSize;
		if ((sint) _X >= 0) XDivSize = (sint) _X / (sint) _Size;
		else XDivSize = ((sint) (_X + 1) / (sint) _Size) - 1;

		sint YDivSize;
		if ((sint) _Y >= 0) YDivSize = (sint) _Y / (sint) _Size;
		else YDivSize = ((sint) (_Y + 1) / (sint) _Size) - 1;

		sint xDivSize;
		if (x >= 0) xDivSize = (sint) x / (sint) _Size;
		else xDivSize = ((sint) (x + 1) / (sint) _Size) - 1;

		sint yDivSize;
		if (y >= 0) yDivSize = (sint) y / (sint) _Size;
		else yDivSize = ((sint) (y + 1) / (sint) _Size) - 1;

		// different zone -> must decal datas
		if (xDivSize != XDivSize || yDivSize != YDivSize)
		{
			sint left   = std::max(x, (sint) _X);
			sint top    = std::max(y, (sint) _Y);
			sint right  = std::min(x + (sint) _Size, (sint) (_X + _Size));
			sint bottom = std::min(y + (sint) _Size, (sint) (_Y + _Size));
			sint offsetX, offsetY;
			if (xDivSize != XDivSize)
			{
				offsetX = xDivSize < XDivSize ?  _Size : -(sint)_Size;
			}
			else
			{
				offsetX = 0;
			}

			if (yDivSize != YDivSize)
			{
				offsetY = yDivSize < YDivSize ?  _Size : -(sint)_Size;
			}
			else
			{
				offsetY = 0;
			}

			sint orgX = _Size * XDivSize;
			sint orgY = _Size * YDivSize;
			for (uint k = 0; k < NumWaterMap; ++k)
			{
				makeCpy(k, (uint) (left - orgX + offsetX) , (uint) (top - orgY + offsetY),
						(uint) (left - orgX), (uint) (top - orgY),
						(uint) (right - left), (uint) (bottom - top));
			}
		}

		sint newOrgX = _Size * xDivSize;
		sint newOrgY = _Size * yDivSize;
		// clear new area
		if (x == (sint) _X)
		{
			if (y < (sint) _Y)
			{
				// x, y, width, height
				clearZone(x - newOrgX, y - newOrgY, _Size, _Y - y);
			}
			else
			{
				clearZone(x - newOrgX, y + _Size - newOrgY, _Size, y - _Y);
			}
		}
		else
		{
			if (x > (sint) _X)
			{
				if (y == (sint) _Y)
				{
					clearZone(_X + _Size - newOrgX, y - newOrgY, x - _X, _Size);
				}
				else if (y < (sint) _Y)
				{
					clearZone(_X + _Size - newOrgX, _Y - newOrgY, x - _X, _Size - (_Y - y));
					clearZone(x - newOrgX, y - newOrgY, _Size, _Y - y);
				}
				else
				{
					clearZone(_X + _Size - newOrgX, y - newOrgY, x - _X, _Size - (y - _Y));
					clearZone(x - newOrgX, _Y + _Size - newOrgY, _Size, y - _Y);
				}
			}
			else
			{
				if (y == (sint) _Y)
				{
					clearZone(x - newOrgX, y - newOrgY, _X - x, _Size);
				}
				else if (y < (sint) _Y)
				{
					clearZone(x - newOrgX, y - newOrgY, _Size, _Y - y);
					clearZone(x - newOrgX, _Y - newOrgY, _X - x, _Size - (_Y - y));
				}
				else
				{
					clearZone(x - newOrgX, y - newOrgY, _X - x, _Size - (y -_Y));
					clearZone(x - newOrgX, _Y + _Size - newOrgY, _Size, y - _Y);
				}
			}
		}

	}
	else
	{
		// the new area has no common pixel's with the previous one
		// clear the whole new area
		uint px = _X % _Size;
		uint py = _Y % _Size;
		clearZone(px, py, _Size, _Size);
	}

	_X = (uint) x;
	_Y = (uint) y;
}



//===========================================================================================

void CWaterHeightMap::animatePart(float startTime, float endTime)
{
	if (endTime < 0.5f * _PropagationTime)
	{
		// perform propagation
		propagate((uint) (_Size * 2.f * startTime / _PropagationTime), (uint) (_Size * 2.f * endTime / _PropagationTime));
	}
	else
	{
		//  end propagation and start filter
		if (startTime < 0.5f * _PropagationTime)
		{
			propagate((uint) (_Size * 2.f * startTime / _PropagationTime), _Size);
			filter(0, (uint) (_Size * 2.f * (endTime / _PropagationTime - 0.5f)));
		}
		else
		{
			filter((uint) (_Size * 2.f * (startTime  / _PropagationTime - 0.5f)), (uint) (_Size * 2.f * (endTime  / _PropagationTime - 0.5f)));
		}
	}
}

//===========================================================================================

void CWaterHeightMap::animate(float deltaT)
{
	if (deltaT < 0) deltaT = 0;
	if (deltaT > _PropagationTime)
	{
		animatePart(0, _PropagationTime);
		swapBuffers(deltaT);
		_PropagateEllapsedTime = 0;
	}
	else
	{
		const float endTime   = _PropagateEllapsedTime + deltaT;
		const float startTime = _PropagateEllapsedTime;

		if (endTime < _PropagationTime)
		{
			animatePart(startTime, endTime);
			_PropagateEllapsedTime = endTime;
		}
		else
		{
			animatePart(startTime, _PropagationTime);
			swapBuffers(deltaT);
			//animatePart(0, endTime - _PropagationTime);

			_PropagateEllapsedTime = 0 /*endTime - _PropagationTime*/;
		}
	}
	animateWaves(deltaT);
}

//===========================================================================================

void		CWaterHeightMap::setSize(uint32 size)
{
	nlassert(size > 4);
	_Size  = size;
	for (uint k = 0; k < NumWaterMap; ++k)
	{
		_Map[k].resize(4 * _Size * _Size);
		clearArea(k, 0, 0, _Size << 1, _Size << 1);
	}
	//_Grad.resize(4 * _Size * _Size);
}

//===========================================================================================

void		CWaterHeightMap::makeCpy(uint buffer, uint dX, uint dY, uint sX, uint sY, uint width, uint height)
{
	if (width == 0 || height == 0) return;
	nlassert(dX <= (2 * _Size));
	nlassert(dY <= (2 * _Size));
	nlassert(sX <= (2 * _Size));
	nlassert(sY <= (2 * _Size));
	nlassert(dX + width <= 2 * _Size);
	nlassert(sX + width <= 2 * _Size);
	nlassert(dY + height <= 2 * _Size);
	nlassert(sY + height <= 2 * _Size);

	sint stepY;
	float *src, *dest;

	const sint stride = _Size << 1;
	if (dY  <= sY)
	{
		stepY = stride;
		src   =  &_Map[buffer][sX + sY * stride];
		dest  =  &_Map[buffer][dX + dY * stride];
	}
	else
	{
		stepY = -stride;
		src   =  &_Map[buffer][sX + (sY + height - 1) * stride];
		dest  =  &_Map[buffer][dX + (dY + height - 1) * stride];
	}

	sint k = height;
	do
	{
		if (dest < src)
		{
#ifdef NL_COMP_VC14
			std::copy(src, src + width, stdext::make_unchecked_array_iterator(dest));
#else
			std::copy(src, src + width, dest);
#endif
		}
		else
		{
			float *rSrc  = src  + width;
			float *rDest = dest + width;
			do
			{
				--rSrc;
				--rDest;
				*rDest = *rSrc;
			}
			while (rSrc != src);
		}
		src  += stepY;
		dest += stepY;
	}
	while (--k);
}

//===========================================================================================

void		CWaterHeightMap::setUserPos(sint x, sint y)
{
	_NewX = x;
	_NewY = y;
}

//===========================================================================================

void		CWaterHeightMap::getUserPos(sint &x, sint &y) const
{
	x = (sint) _X; y = (sint) _Y;
}



//===========================================================================================

void		CWaterHeightMap::propagate(uint start, uint end)
{
	start = std::max(1u, start);
	end   = std::min((uint) (_Size - 1), end);
	const float damping = _Damping;
	clearBorder(0);
	clearBorder(1);
	nlassert(_Size != 0);
	sint x, y;
	uint px = _X % _Size;
	uint py = _Y % _Size;
	sint offset = px + 1 + ((py + start) * (_Size << 1));
	//nlinfo("%d, %d, %d",  (_CurrMap + (NumWaterMap - 1)) % NumWaterMap,  _CurrMap,
	float *buf2 = &_Map[ (_CurrMap + (NumWaterMap - 1)) % NumWaterMap][offset];
	float *buf1 = &_Map[_CurrMap][offset];
	float *dest = &_Map[(_CurrMap + 1) % NumWaterMap][offset];

	const sint  sizeX2 = _Size << 1;
	y = end - start;
	if (y <= 0) return;
	do
	{
		x = _Size - 2;
		do
		{
			*dest	= damping * ( 0.5f * (buf1[1] + buf1[-1] + buf1[sizeX2] + buf1[- sizeX2]) - *buf2);
			++buf1;
			++buf2;
			++dest;
		}
		while (--x);
		buf1 = buf1 + _Size + 2;
		buf2 = buf2 + _Size + 2;
		dest = dest + _Size + 2;
	}
	while (--y);

}



//===========================================================================================

void	CWaterHeightMap::filter(uint start, uint end)
{
	start = std::max(1u, start);
	end   = std::min((uint) (_Size - 1), end);
	const float blurCoeff = _FilterWeight;
	nlassert(_Size != 0);
	sint x, y;
	uint px = _X % _Size;
	uint py = _Y % _Size;
	sint offset = px + 1 + ((py + start) * (_Size << 1));
	float *buf = &_Map[ (_CurrMap + 1) % NumWaterMap ][offset];
	//NLMISC::CVector2f *ptGrad = &_Grad[offset];
	y = end - start;
	if (y <= 0) return;
	const float totalBlurCoeff = (1.f / (4.f + blurCoeff));
	const sint  sizeX2 = _Size << 1;
	do
	{
		x = _Size - 2;
		do
		{
			*buf = totalBlurCoeff * (*buf * blurCoeff
										 + buf[1]
										 + buf[-1]
										 + buf[sizeX2]
										 + buf[- sizeX2]
									 );
			// compute gradient
			/*ptGrad->x = buf[1]		    - buf[- 1];
			ptGrad->y = buf[sizeX2]     - buf[- sizeX2];*/

			++buf;
			//++ptGrad;
		}
		while (--x);
		buf    += _Size + 2;
		//ptGrad += _Size + 2;
	}
	while (--y);
}

//===========================================================================================

void CWaterHeightMap::animateWaves(float deltaT)
{
	if (_WavesEnabled)
	{
		uint numWaves;
		if (_WavePeriod == 0)
		{
			numWaves = 1;
		}
		else
		{
			_EmitEllapsedTime += deltaT;
			if (_EmitEllapsedTime > _WavePeriod)
			{
				numWaves = (uint) (_EmitEllapsedTime / _WavePeriod);
				_EmitEllapsedTime -= numWaves * _WavePeriod;
				if (numWaves > 10) numWaves = 10;
			}
			else
			{
				numWaves = 0;
			}
		}

		uint k;
		// generate automatic waves
		if (!_BorderWaves)
		{
			if (_WaveIntensity != 0)
			{
				for (k = 0; k < numWaves; ++k)
				{
					perturbate(_NewX + rand() % _Size, _NewY + rand() % _Size, _WaveImpulsionRadius, _WaveIntensity);
				}
			}
		}
		else
		{
			switch(rand() & 3) // choose a random border
			{
				case 0: // top border
					for (k = 0; k < numWaves; ++k)
					{
						perturbate(_NewX + (uint) rand() % _Size, _NewY, _WaveImpulsionRadius, _WaveIntensity);
					}
				break;
				case 1: // bottom border
					for (k = 0; k < numWaves; ++k)
					{
						perturbate(_NewX + (uint) rand() % _Size, _NewY + _Size - 1, _WaveImpulsionRadius, _WaveIntensity);
					}
				break;
				case 2: // right border
					for (k = 0; k < numWaves; ++k)
					{
						perturbate(_NewX + _Size - 1, _NewY + (uint) rand() % _Size, _WaveImpulsionRadius, _WaveIntensity);
					}
				break;
				case 3: // left border
					for (k = 0; k < numWaves; ++k)
					{
						perturbate(_NewX, _NewY + (uint) rand() % _Size, _WaveImpulsionRadius, _WaveIntensity);
					}
				break;
			}

		}
	}
}

//===========================================================================================

void CWaterHeightMap::swapBuffers(float deltaT)
{
	updateUserPos();
	_CurrMap = (_CurrMap + 1) % NumWaterMap;
}


//===========================================================================================

void CWaterHeightMap::clearZone(sint x, sint y, sint width, sint height)
{
	for (uint k = 0; k < NumWaterMap; ++k)
	{
		clearArea(k, x, y, width, height);
	}
}

//===========================================================================================

void CWaterHeightMap::clearArea(uint8 currMap, sint x, sint y, sint width, sint height)
{
	nlassert(_Size > 1);
	nlassert(width >= 0);
	nlassert(height >= 0);
	uint sizex2 = _Size << 1;

	if (x < 0)
	{
		width += x;
		x = 0;
		if (width <= 0) return;
	}
	if (y < 0)
	{
		height += y;
		y = 0;
		if (height <= 0) return;
	}
	if (x + width > (sint) sizex2)
	{
		width = width - (x + width - sizex2);
	}

	if (y + height > (sint) sizex2)
	{
		height = height - (y + height - sizex2);
	}

	float *dest = &*(_Map[  currMap	 ].begin() + x + (_Size << 1) * y);
	do
	{
		std::fill(dest, dest + width, 0.f);
		dest  += (_Size << 1);
	}
	while (-- height);
}



//===========================================================================================

void	CWaterHeightMap::perturbate(sint x, sint y, sint radius, float intensity)
{
		nlassert(_Size != 0);
		nlassert(radius > 0);
		sint orgX = _X - _X % _Size;
		sint orgY = _Y - _Y % _Size;
		TFloatVect &map = _Map[(_CurrMap + 1) % NumWaterMap];
		const uint sizeX2 = _Size << 1;
		for (sint px = -radius + 1; px < radius; ++px)
		{
			for (sint py = -radius + 1; py < radius; ++py)
			{
				if ((uint) (x + px - orgX) < sizeX2
					&& (uint) (y + py - orgY) < sizeX2)
				{

					float dist = ((float) radius - sqrtf((float) (px * px + py * py ))) / (float) radius;
					float v = dist < radius ? intensity * cosf(dist * (float) NLMISC::Pi * 0.5f) : 0.f;
					map[x + px - orgX + sizeX2 * (y + py - orgY)] = v;
				}
			}
		}
}

//===========================================================================================

void	CWaterHeightMap::perturbate(const NLMISC::CVector2f &pos, float strenght, float radius)
{
	const float invUnitSize = 1.f / _UnitSize;
	perturbate((sint) (pos.x * invUnitSize), (sint) (pos.y * invUnitSize), (sint) radius, strenght);
}

//===========================================================================================

void CWaterHeightMap::perturbatePoint(sint x, sint y, float intensity)
{
	sint orgX = _X - _X % _Size;
	sint orgY = _Y - _Y % _Size;
	uint X = (uint) (x - orgX);
	uint Y = (uint) (y - orgY);
	if (X < (_Size << 1)
		&& Y < (_Size << 1)
		)
	{
		const uint sizex2 = _Size << 1;
		TFloatVect &map = _Map[(_CurrMap + 1) % NumWaterMap];
		map[X + sizex2 * Y] = intensity;
	}
}

//===========================================================================================

void	CWaterHeightMap::perturbatePoint(const NLMISC::CVector2f &pos, float strenght)
{
	const float invUnitSize = 1.f / _UnitSize;
	perturbatePoint((sint) (pos.x * invUnitSize), (sint) (pos.y * invUnitSize), strenght);
}

//===========================================================================================

void	CWaterHeightMap::clearBorder(uint currMap)
{
	float *map  = &_Map[currMap][0];
	uint sizex2 = _Size << 1;

	// top and bottom

	float *up    = &map[(_X % _Size) + sizex2 * (_Y % _Size)];
	float *curr = up;
	const float *endUp = up + _Size;
	const uint  downOff  = (_Size - 1) * sizex2;
	do
	{
		*curr = curr[downOff] = 0.f;
		++curr;
	}
	while (curr != endUp);

	// right and left
	curr  = up;
	const float *endLeft = up + downOff;
	const uint  rightOff = _Size - 1;
	do
	{
		*curr = curr[rightOff] = 0.f;
		curr += sizex2;
	}
	while (curr != endLeft);
}

//===========================================================================================

void CWaterHeightMap::setWaves(float intensity, float period, uint radius, bool border)
{
	_WaveIntensity		  = intensity;
	_WavePeriod			  = period;
	_WaveImpulsionRadius  = radius;
	_BorderWaves		  = border;

}


//===========================================================================================

void CWaterHeightMap::serial(NLMISC::IStream &f)
{
	f.xmlPushBegin("WaterHeightMap");
		f.xmlSetAttrib ("NAME");
		f.serial (_Name);
	f.xmlPushEnd();
	(void)f.serialVersion(0);
	f.xmlSerial(_Size, "SIZE");
	if (f.isReading())
	{
		setSize(_Size);
	}
	f.xmlSerial(_Damping, "DAMPING");
	f.xmlSerial(_FilterWeight, "FILTER_WEIGHT");
	f.xmlSerial(_UnitSize, "WATER_UNIT_SIZE");
	f.xmlSerial(_WavesEnabled, "WavesEnabled");
	if (_WavesEnabled)
	{
		f.xmlPush("WavesParams");
			f.xmlSerial(_WaveIntensity, "WAVE_INTENSITY");
			f.xmlSerial(_WavePeriod, "WAVE_PERIOD");
			f.xmlSerial(_WaveImpulsionRadius, "WAVE_IMPULSION_RADIUS");
			f.xmlSerial(_BorderWaves, "BORDER_WAVES");
			f.xmlSerial(_PropagationTime, "PROPAGATION_TIME");
		f.xmlPop();
	}
	f.xmlPop();
}




// *** perform a bilinear on 4 values
//   0---1
//   |   |
//   3---2
static float inline BilinFilter(float v0, float v1, float v2, float v3, float u, float v)
{
	const float g = v * v3 + (1.f - v) * v0;
	const float h = v * v2 + (1.f - v) * v1;
	return u * h + (1.f - u) * g;
}



//===========================================================================================

float	CWaterHeightMap::getHeight(const NLMISC::CVector2f &pos)
{
	const float invUnitSize = 1.f / _UnitSize;

	const float xPos = invUnitSize * pos.x; // position in map space
	const float yPos = invUnitSize * pos.y; // position in map space


	if ((uint) xPos - _X < _Size - 1
		&& (uint) yPos - _Y < _Size - 1
		)

	{

		const sint orgX = _X - _X % _Size;
		const sint orgY = _Y - _Y % _Size;
		const uint sizeX2 = _Size << 1;


		const sint  fxPos = (sint) floorf(xPos);
		const sint  fyPos = (sint) floorf(yPos);



			const float deltaU	  = xPos - fxPos;
			const float deltaV	  = yPos - fyPos;
			const uint  offset	  = (uint) fxPos - orgX + sizeX2 * ( (uint) fyPos - orgY);
			const float lambda	  = getBufferRatio();
			const float *map1     = getPrevPointer();
			const float *map2     = getPointer();

			return BilinFilter(lambda * map2[offset]			  + (1.f - lambda) * map1[offset	],			  // top left
							   lambda * map2[offset + 1]		  + (1.f - lambda) * map1[offset + 1],		  // top right
							   lambda * map2[offset + sizeX2 + 1] + (1.f - lambda) * map1[offset + sizeX2 + 1], // bottom right
							   lambda * map2[offset + sizeX2 ]    + (1.f - lambda) * map1[offset + sizeX2 ],	  // bottom left
							   deltaU,
							   deltaV
							   );
	}
	else return 0;

}

} // NL3D
