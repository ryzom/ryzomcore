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

#include "nel/3d/texture_bump.h"



#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {


CTextureBump::TNameToNI CTextureBump::_NameToNF;


#define GET_HGT(x, y) ((sint) ((src[(uint) (x) % width + ((uint) (y) % height) * width] & 0x00ff00) >> 8))
/// create a DsDt texture from a height map (red component of a rgba bitmap)
static void BuildDsDt(uint32 *src, sint width, sint height, uint16 *dest)
{
	#define GET_HGT(x, y) ((sint) ((src[(uint) (x) % width + ((uint) (y) % height) * width] & 0x00ff00) >> 8))
	sint x, y;
	for (x = 0; x < width; ++x)
	{
		for (y = 0; y < height; ++y)
		{
			sint off = x + y * width;
			sint16 ds = (sint16) (GET_HGT(x + 1, y) - GET_HGT(x - 1, y));
			sint16 dt = (sint16) (GET_HGT(x, y + 1) - GET_HGT(x, y - 1));
			dest[off] = (uint16) ((ds & 0xff)  | ((dt & 0xff) << 8));
		}
	}
}


/// create a rgba gradient texture from a height map (red component of a rgba bitmap)
static void BuildDsDtAsRGBA(uint32 *src, sint width, sint height, uint32 *dest)
{
	#define GET_HGT(x, y) ((sint) ((src[(uint) (x) % width + ((uint) (y) % height) * width] & 0x00ff00) >> 8))
	sint x, y;
	for (x = 0; x < width; ++x)
	{
		for (y = 0; y < height; ++y)
		{
			sint off = x + y * width;
			sint16 ds = (sint16) (GET_HGT(x + 1, y) - GET_HGT(x - 1, y));
			sint16 dt = (sint16) (GET_HGT(x, y + 1) - GET_HGT(x, y - 1));
			dest[off] = 0xff000000 | (uint32) ((ds + 0x80) & 0xff) | (uint32) ((dt + 0x80) << 8);
		}
	}
}

/// Normalize a DsDt texture after it has been built, and return the normalization factor
static float NormalizeDsDt(uint16 *src, sint width, sint height)
{
	const uint size = width * height;
	uint highestDelta = 0;
	uint k;

	for (k = 0; k < size; ++k)
	{
		highestDelta = std::max(highestDelta, (uint) ::abs((sint) (sint8) (src[k] & 255)));
		highestDelta = std::max(highestDelta, (uint) ::abs((sint) (sint8) (src[k] >> 8)));
	}

	if (highestDelta == 0)
	{
		return 1.f;
	}
	float normalizationFactor = 127.f / highestDelta;
	for (k = 0; k < size; ++k)
	{
		float fdu = (sint8) (src[k] & 255) * normalizationFactor;
		float fdv = (sint8) (src[k] >> 8) * normalizationFactor;
		NLMISC::clamp(fdu, -128, 127);
		NLMISC::clamp(fdv, -128, 127);
		uint8 du = (uint8) (sint8) fdu;
		uint8 dv = (uint8) (sint8) fdv;
		src[k] = (uint16) du | (((uint16) dv) << 8);
	}
	return 1.f / normalizationFactor;
}

static float NormalizeDsDtAsRGBA(uint32 *src, sint width, sint height)
{
	uint k;
	const uint size = width * height;
	uint highestDelta = 0;

	/// first, get the highest delta
	for (k = 0; k < size; ++k)
	{
		highestDelta = std::max(highestDelta, (uint) abs((sint8) ((src[k] & 0xff) - 0x80)));
		highestDelta = std::max(highestDelta, (uint) abs((sint8) (((src[k] >> 8) & 0xff) - 0x80)));
	}

	if (highestDelta == 0)
	{
		return 1.f;
	}
	float normalizationFactor = 127.f / highestDelta;
	for (k = 0; k < size; ++k)
	{
		float fdu = ((sint8) ((src[k] & 255) - 0x80)) * normalizationFactor;
		float fdv = ((sint8) (((src[k] >> 8) & 0xff) - 0x80)) * normalizationFactor;
		NLMISC::clamp(fdu, -128, 127);
		NLMISC::clamp(fdv, -128, 127);
		uint8 du = (uint8) ((sint8) fdu + 0x80);
		uint8 dv = (uint8) ((sint8) fdv + 0x80);
		src[k] = (src[k] & 0xffff0000) | (uint32) du | (uint32) (((uint16) dv) << 8);
	}
	return 1.f / normalizationFactor;

}





/*
 * Constructor
 */
CTextureBump::CTextureBump() : _NormalizationFactor(NULL),
							   _DisableSharing(false),
							   _ForceNormalize(true)
{
	// mipmapping not supported for now, disable it
	ITexture::setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
}


///==============================================================================================
void CTextureBump::setFilterMode(TMagFilter magf, TMinFilter minf)
{
	nlstop; // set filter mode not allowed with bump textures (not supported by some GPUs)
}

void CTextureBump::setHeightMap(ITexture *heightMap)
{
	if (heightMap != _HeightMap)
	{
		_HeightMap = heightMap;
		touch();
	}
}


///==============================================================================================
void CTextureBump::serial(NLMISC::IStream &f)
{
	/// version 2 : normalization flag
	sint ver = f.serialVersion(3);
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
	f.serial(_DisableSharing);
	if (ver >= 1)
	{
		bool oldFlag = false;
		f.serial(oldFlag); // backaward compatibility : serial the old "forceAbsoluteOffset" flag
	}
	if (ver >= 2)
	{
		f.serial(_ForceNormalize);
	}
}

///==============================================================================================
void CTextureBump::doGenerate(bool async)
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
	releaseMipMaps();
	uint width = _HeightMap->getWidth();
	uint height = _HeightMap->getHeight();
	if (getUploadFormat() == RGBA8888)
	{
		CBitmap::resize(_HeightMap->getWidth(), _HeightMap->getHeight(), CBitmap::RGBA);
	}
	else
	{
		CBitmap::resize(_HeightMap->getWidth(), _HeightMap->getHeight(), CBitmap::DsDt);
	}
	// build the DsDt map
	if (getUploadFormat() == RGBA8888)
	{
		BuildDsDtAsRGBA((uint32 *) &(_HeightMap->getPixels()[0]), width, height, (uint32 *) &(getPixels()[0]));
	}
	else
	{
		BuildDsDt((uint32 *) &(_HeightMap->getPixels()[0]), width, height, (uint16 *) &(getPixels()[0]));
	}

	float normalizationFactor = 1.0f;
	// Normalize the map if needed
	if (_ForceNormalize)
	{
		if (getUploadFormat() == RGBA8888)
		{
			normalizationFactor = NormalizeDsDtAsRGBA((uint32 *) &(getPixels()[0]), width, height);
		}
		else
		{
			normalizationFactor = NormalizeDsDt((uint16 *) &(getPixels()[0]), width, height);
		}
	}
	// create entry in the map for the normalization factor
	std::string shareName = getShareName();
	TNameToNI::iterator it = _NameToNF.find(shareName);
	if (it == _NameToNF.end())
	{
		// create a new entry
		CNormalizationInfo ni;
		ni.NumRefs = 1;
		ni.NormalizationFactor = normalizationFactor;
		std::pair<TNameToNI::iterator, bool> pb = _NameToNF.insert(TNameToNI::value_type(shareName, ni));
		_NormalizationFactor = &(pb.first->second.NormalizationFactor);
	}
	else
	{
		// another map has computed the factor
		_NormalizationFactor = &(it->second.NormalizationFactor);
		++(it->second.NumRefs);
	}

	if (_HeightMap->getReleasable())
	{
		_HeightMap->release();
	}
}

///==============================================================================================
void CTextureBump::release()
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


///==============================================================================================
bool	CTextureBump::supportSharing() const
{
	return !_DisableSharing && _HeightMap && _HeightMap->supportSharing();
}


///==============================================================================================
std::string	CTextureBump::getShareName() const
{
	nlassert(supportSharing());
	return "BumpDsDt:" + _HeightMap->getShareName();
}

///==============================================================================================
float CTextureBump::getNormalizationFactor()
{
	if (!_Touched)
	{
		if (_NormalizationFactor) return *_NormalizationFactor;
	}
	if (!_HeightMap) return 1.f;
	// not computed yet, see if another map has computed it
	std::string shareName = getShareName();
	TNameToNI::iterator it = _NameToNF.find(shareName);
	if (it != _NameToNF.end())
	{
		_NormalizationFactor = &(it->second.NormalizationFactor);
		++(it->second.NumRefs);
	}
	else
	{
		doGenerate();
		if (this->getReleasable()) this->release();
	}
	return _NormalizationFactor ? *_NormalizationFactor : 1.f;
}

///==============================================================================================
CTextureBump::~CTextureBump()
{
	if (_NormalizationFactor && !_NameToNF.empty())
	{
		// find normalization factor from its name
		TNameToNI::iterator it = _NameToNF.find(getShareName());

		// if found
		if (it != _NameToNF.end())
		{
			// we can delete it only if it's not used anymore
			if (--(it->second.NumRefs) == 0) _NameToNF.erase(it);
		}
	}
}




} // NL3D
