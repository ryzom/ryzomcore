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

#include "nel/3d/texture_cube.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/debug.h"
#include <memory>

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************
	CTextureCube::CTextureCube()
{
	for( uint i = 0; i < 6; ++i )
		_Textures[i] = NULL;
}

// ***************************************************************************
void CTextureCube::setTexture(TFace f, ITexture *t)
{
	_Textures[f] = t;
}

// ***************************************************************************
string CTextureCube::getShareName() const
{
	string sTemp;

	for( uint i = 0; i < 6; ++i )
		if( _Textures[i] != NULL )
			sTemp += _Textures[i]->getShareName();
	return sTemp;
}

// ***************************************************************************
void CTextureCube::doGenerate(bool /* async */)
{
	uint i;
	ITexture *pRefTex = NULL;

	for( i = 0; i < 6; ++i )
		if( _Textures[i] != NULL )
		{
			pRefTex = _Textures[i];
			break;
		}

	if( pRefTex == NULL )
		return; // There are NO texture

	pRefTex->generate();
	// The reference texture must be square and power of 2
	// If not power of 2 resize to the power of 2 just below the current size
	/*
	if( ( !isPowerOf2(pRefTex->getWidth()) ) || ( pRefTex->getWidth() != pRefTex->getHeight() ) )
	{
		uint32 nNewSize = pRefTex->getWidth();
		if( !isPowerOf2(nNewSize) )
		{
			nNewSize = raiseToNextPowerOf2(nNewSize);	// 5 -> 8 | 20 -> 32
			nNewSize = getPowerOf2(nNewSize);			// 8 -> 3 | 32 -> 5
			nNewSize -= 1;								// 3 -> 2 | 5 -> 4
			nNewSize = 1<<nNewSize;						// 2 -> 4 | 4 -> 16
		}

		pRefTex->resample( nNewSize, nNewSize );
	}
	*/

	// All textures must be like the reference texture
	for( i = 0; i < 6; ++i )
	{
		if( _Textures[i] != NULL )
			_Textures[i]->generate();
		else
			_Textures[i] = pRefTex;

		// The texture must have the same UpLoadFormat
		if( _Textures[i]->getUploadFormat()!=pRefTex->getUploadFormat() )
		{
			nlwarning("Bad TextureCube: different UpLoad format: %s and %s",
				_Textures[i]->getShareName().c_str(), pRefTex->getShareName().c_str() );
			// => replace the texture with the reference
			_Textures[i] = pRefTex;
		}
		// if Auto format, must have the same PixelFormat
		else if( _Textures[i]->getUploadFormat()==ITexture::Auto && _Textures[i]->getPixelFormat()!=pRefTex->getPixelFormat() )
		{
			nlwarning("Bad TextureCube: different Pixel format: %s and %s",
				_Textures[i]->getShareName().c_str(), pRefTex->getShareName().c_str() );
			// => replace the texture with the reference
			_Textures[i] = pRefTex;
		}

		// The textures must have the same size.
		if( ( _Textures[i]->getWidth()  != pRefTex->getWidth()  ) ||
			( _Textures[i]->getHeight() != pRefTex->getHeight() ) )
		{
			// If can't resample the bitmap
			if( _Textures[i]->getPixelFormat()!=CBitmap::RGBA )
			{
				nlwarning("Bad TextureCube: different Size (not RGBA): %s and %s",
					_Textures[i]->getShareName().c_str(), pRefTex->getShareName().c_str() );
				// => replace the texture with the reference
				_Textures[i] = pRefTex;
			}
			else
			{
				// Ok, can resample the bitmap.
				_Textures[i]->resample( pRefTex->getWidth(), pRefTex->getHeight() );
			}
		}
	}
}

// ***************************************************************************
void	CTextureCube::release()
{
	uint i;
	for( i = 0; i < 6; ++i )
		if( _Textures[i] != NULL )
			_Textures[i]->release();
}

// ***************************************************************************
void	CTextureCube::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(2);

	// serial the base part of ITexture.
	ITexture::serial(f);

	for( uint i = 0; i < 6; ++i )
	{
		ITexture *tex = _Textures[i];
		f.serialPolyPtr( tex );
		_Textures[i] = tex;
	}
	if( f.isReading() )
		touch();

	if (ver == 1)
	{
		bool temp;
		f.serial(temp);
	}
}



// ***************************************************************************
void CTextureCube::selectTexture(uint index)
{
	for( uint i = 0; i < 6; ++i )
	{
		if (_Textures[i]) _Textures[i]->selectTexture(index);
	}
	touch();
}

// ***************************************************************************
bool CTextureCube::isSelectable() const
{
	for( uint i = 0; i < 6; ++i )
	{
		if (_Textures[i] && _Textures[i]->isSelectable()) return true;
	}
	return false;
}

// ***************************************************************************
ITexture *CTextureCube::buildNonSelectableVersion(uint index)
{
	if (!isSelectable()) return this;
	CUniquePtr<CTextureCube> tc(new CTextureCube);

	// copy basic texture parameters
	(ITexture &) *tc.get() = (ITexture &) *this; // invoke ITexture = op for basics parameters

	for( uint i = 0; i < 6; ++i )
	{
		tc->_Textures[i] = _Textures[i]->buildNonSelectableVersion(index);
	}
	return tc.release();
}


} // NL3D
