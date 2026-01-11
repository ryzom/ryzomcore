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
#include "nel/3d/texture_emboss.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

// ***********************************************************************************************************
CTextureEmboss::CTextureEmboss() : _Ambient(CRGBA::Black),
								   _Diffuse(CRGBA::White),
								   _DisableSharing(false),
								   _SlopeFactor(1.f)
{
	_LightDir.set(1.f, 1.f, 1.f);
	_LightDir.normalize();
}


// ***********************************************************************************************************
void CTextureEmboss::setHeightMap(ITexture *heightMap)
{
	if (heightMap != _HeightMap)
	{
		_HeightMap = heightMap;
		touch();
	}
}

// ***********************************************************************************************************
void CTextureEmboss::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);
	ITexture::serial(f);
	ITexture *tex = NULL;
	if (f.isReading())
	{
		f.serialPolyPtr(tex);
		_HeightMap = tex;
		touch();
	}
	else
	{
		tex = _HeightMap;
		f.serialPolyPtr(tex);
	}
	f.serial(_Ambient);
	f.serial(_Diffuse);
	f.serial(_LightDir);
	f.serial(_DisableSharing);
}

// ***********************************************************************************************************
bool CTextureEmboss::supportSharing() const
{
	return !_DisableSharing && _HeightMap && _HeightMap->supportSharing();
}

// ***********************************************************************************************************
std::string CTextureEmboss::getShareName() const
{
	nlassert(supportSharing());
	return "Emboss:" + _HeightMap->getShareName();
}

// ***********************************************************************************************************
void CTextureEmboss::release()
{
	ITexture::release();
	if (_HeightMap != NULL)
	{
		if (_HeightMap->getReleasable())
		{
			_HeightMap->release();
		}
	}
}

// ***********************************************************************************************************
void CTextureEmboss::doGenerate(bool /* async */)
{
	if (!_HeightMap)
	{
		makeDummy();
		return;
	}
	// generate the height map
	_HeightMap->generate();
	if (!_HeightMap->convertToType(CBitmap::RGBA))
	{
		makeDummy();
		return;
	}
	CBitmap::resize(_HeightMap->getWidth(), _HeightMap->getHeight(), CBitmap::RGBA);
	releaseMipMaps();
	uint width = _HeightMap->getWidth();
	uint height = _HeightMap->getHeight();

	const CRGBA *src = (CRGBA *) &(_HeightMap->getPixels(0)[0]);
	CRGBA *dest = (CRGBA *) &(getPixels(0)[0]);
	CVector normal;
	for (uint y = 0; y < height; ++y)
	{
		for (uint x = 0; x < width; ++x)
		{
			// get position of each adajacent pixel (sure, it can be optimized)
			const CRGBA *pixelRight = (x != width - 1) ? (src + 1) : (src + 1 - width );
			const CRGBA *pixelLeft  = (x != 0) ? (src - 1) : (src + width - 1);
			const CRGBA *pixelTop   = (y != 0) ? (src - width) : (src + width * (height - 1));
			const CRGBA *pixelBottom = (y != (height - 1)) ? (src + width) : (src - width * (height - 1));

			normal.x = ((sint16) pixelRight->R - (sint16) pixelLeft->R) / 256.f;
			normal.y = ((sint16) pixelTop->R - (sint16) pixelBottom->R) / 256.f;
			normal.x *= _SlopeFactor;
			NLMISC::clamp(normal.x, -1.f, 1.f);
			normal.y *= _SlopeFactor;
			NLMISC::clamp(normal.y, -1.f, 1.f);
			normal.z = 1.f;
			normal.normalize();

			float colorValue = _LightDir * normal;
			if (colorValue <= 0.f) colorValue = 0.f;

			CRGBA diffuse;
			*dest = _Ambient;
			diffuse.modulateFromui(_Diffuse, (uint) (255.f * colorValue));
			dest->add(*dest, diffuse);
			dest->A = 255;

			++ src;
			++ dest;
		}
	}

	if (_HeightMap->getReleasable())
	{
		_HeightMap->release();
	}
}

}
