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

#include "nel/3d/texture_grouped.h"
#include "nel/3d/texture_file.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"

#include <algorithm>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

std::map<std::string, uint> CTextureGrouped::_NameToSize;


///=====================================================================================================
/// This is used to get the size of a texture, with an optimisation in the case of texture files
static inline void GetTextureSize(ITexture *tex, uint &width, uint &height)
{
	if (tex->getClassName() == "CTextureFile")
	{
		CTextureFile *tf = static_cast<CTextureFile *>(tex);
		uint32 srcWidth, srcHeight;
		if (!tf->getFileName().empty())
		{
			try
			{
				CBitmap::loadSize(NLMISC::CPath::lookup(tf->getFileName()), srcWidth, srcHeight);
				if (srcWidth == 0 || srcHeight == 0)
				{
					nlinfo("Unable to get size of texture : %s", tf->getFileName().c_str());
					width = height = 0;
					return;
				}
				width = srcWidth;
				height = srcHeight;
			}
			catch (const NLMISC::EPathNotFound &e)
			{
				nlinfo("%s", e.what());
				width = height = 0;
			}
			catch (const NLMISC::EStream &e)
			{
				nlinfo("unable to load size from a bitmap ! name = %s", tf->getFileName().c_str());
				nlinfo("reason = %s", e.what());
				width = height = 0;
			}
		}
		else
		{
			width = height = 0;
		}
	}
	else // we must generate the texture to get its size
	{
		tex->generate();
		width = tex->getWidth();
		height = tex->getHeight();
		if (tex->getReleasable())
		{
			tex->release();
		}
	}
}

///=====================================================================================================
CTextureGrouped::CTextureGrouped() : _NbTex(0)
{
	setFilterMode(Linear, LinearMipMapOff);
}

///=====================================================================================================
CTextureGrouped::CTextureGrouped(const CTextureGrouped &src) : ITexture(src)
{
	// copy the part associated with us;
	duplicate(src);
}

///=====================================================================================================
void CTextureGrouped::duplicate(const CTextureGrouped &src)
{
	_NbTex = src._NbTex;
	TTexList texCopy(src._Textures.begin(), src._Textures.end());
	_Textures.swap(texCopy);
	TFourUVList	uvCopy(_TexUVs.begin(), _TexUVs.end());
	_TexUVs.swap(uvCopy);
}

///=====================================================================================================
CTextureGrouped &CTextureGrouped::operator=(const CTextureGrouped &src)
{
	ITexture::operator=(src); // copy parent part
	duplicate(src);
	return *this;
}

///=====================================================================================================
bool CTextureGrouped::areValid(CSmartPtr<ITexture> *textureTab, uint nbTex)
{
	bool result = true;
	uint k;
	for(k = 0; k < nbTex; ++k)
	{
		textureTab[k]->generate();
		if (textureTab[k]->getWidth() != textureTab[0]->getWidth()
			|| textureTab[k]->getHeight() != textureTab[0]->getHeight()
			|| textureTab[k]->getPixelFormat() != textureTab[0]->getPixelFormat()
		   )
		{
			result = false;
			break;
		}
	}


	for (k = 0; k < nbTex; ++k)
	{
		if (textureTab[k]->getReleasable()) textureTab[k]->release();
	}
	return result;
}

///=====================================================================================================
void CTextureGrouped::setTextures(CSmartPtr<ITexture> *textureTab, uint nbTex, bool checkValid)
{
	nlassert(nbTex > 0);


	if (checkValid)
	{
		if (!areValid(textureTab, nbTex))
		{
			displayIncompatibleTextureWarning(textureTab, nbTex);
			makeDummies(textureTab, nbTex);
		}
	}
	_Textures.resize(nbTex);
	std::copy(textureTab, textureTab + nbTex, _Textures.begin());
	_TexUVs.resize(nbTex);
	for(uint k = 0; k < nbTex; ++k)
	{
		// real uvs are generated during doGenerate
		_TexUVs[k].uv0.set(0.f, 0.f);
		_TexUVs[k].uv1.set(0.f, 0.f);
		_TexUVs[k].uv2.set(0.f, 0.f);
		_TexUVs[k].uv3.set(0.f, 0.f);
	}
	_DeltaUV.set(0.f, 0.f);
	_NbTex = nbTex;
	touch(); // the texture need regeneration

	nlassert(_NbTex == _Textures.size());
}

///=====================================================================================================
void CTextureGrouped::doGenerate(bool async)
{
	nlassert(_NbTex == _Textures.size());
	if (_NbTex == 0)
	{
		makeDummy();
	}
	else
	{
		// Generate the first texture to get the size
		_Textures[0]->generate();
		const uint width = _Textures[0]->getWidth(), height = _Textures[0]->getHeight();
		const uint totalHeight = height * _NbTex;
		const uint realHeight  = NLMISC::raiseToNextPowerOf2(totalHeight);
		resize(width, realHeight, _Textures[0]->getPixelFormat());
		uint k;
		sint32 currY =  0;
		for(k = 0; k < _NbTex; ++k)
		{
			_Textures[k]->generate();
			if (_Textures[k]->getWidth() != width
				|| _Textures[k]->getHeight() != height
				|| _Textures[k]->getPixelFormat() != _Textures[0]->getPixelFormat())
			{
				makeDummy();
				break;
			}
			this->blit(_Textures[k], 0, currY);
			currY += height;
		}

		for(k = 0; k < _NbTex; ++k)
		{
			if ( _Textures[k]->getReleasable())
			{
				_Textures[k]->release();
			}
		}

		// save sub bitmap size so that bitmaps won't be reloaded to get their size the next time
		_NameToSize[getShareName()] = height;
		// compute the uvs
		genUVs(height);
	}
}

///=====================================================================================================
void CTextureGrouped::forceGenUVs()
{
	// retrieve size of sub-bitmap
	std::string shareName = getShareName();
	std::map<std::string, uint>::const_iterator it = _NameToSize.find(shareName);
	if (it == _NameToSize.end())
	{
		doGenerate(); // this will retrieve size of sub-bitmaps
		              // TODO : only load the size... not a problem for now because textures can be cached at startup
		release();
		it = _NameToSize.find(getShareName());
		if (it == _NameToSize.end())
		{
			_TexUVs.resize(_NbTex);
			// generate default uvs
			for(uint k = 0; k < _NbTex; ++k)
			{
				TFourUV &uvs = _TexUVs[k];
				uvs.uv0.set(0.f, 0.f);
				uvs.uv1.set(1.f, 0.f);
				uvs.uv2.set(1.f, 1.f);
				uvs.uv3.set(0.f, 1.f);
			}
			return;
		}
	}
	genUVs(it->second);
}

///=====================================================================================================
void CTextureGrouped::genUVs(uint subBitmapHeight)
{
	_TexUVs.resize(_NbTex);
	//
	const uint totalHeight = subBitmapHeight * _NbTex; // *it contains the height of bitmap
	const uint realHeight  = NLMISC::raiseToNextPowerOf2(totalHeight);
	// TODO : better ordering of sub-bitmaps
	const float deltaV = realHeight ? (float(totalHeight) / float(realHeight)) * (1.0f / _NbTex)
		: 0.f;
	_DeltaUV = CUV(1,  deltaV);
	CUV currentUV(0, 0);
	for(uint k = 0; k < _NbTex; ++k)
	{
		TFourUV &uvs = _TexUVs[k];
		uvs.uv0 = currentUV;
		uvs.uv1 = currentUV + CUV(1, 0);
		uvs.uv2 = currentUV + _DeltaUV;
		uvs.uv3 = currentUV + CUV(0, deltaV);
		currentUV.V += deltaV;
	}
}

///=====================================================================================================
void CTextureGrouped::getTextures(CSmartPtr<ITexture> *textureTab) const
{
	CSmartPtr<ITexture> *dest = textureTab;

	for(TTexList::const_iterator it = _Textures.begin(); it != _Textures.end(); ++it, ++dest)
	{
		*dest = *it;
	}
}

///=====================================================================================================
bool	CTextureGrouped::supportSharing() const
{
	// all textures in this group must support sharing for this one to support it
	for(TTexList::const_iterator it = _Textures.begin(); it != _Textures.end(); ++it)
	{
		if (!(*it)->supportSharing()) return false;
	}

	return true;
}

///=====================================================================================================
std::string		CTextureGrouped::getShareName() const
{
	nlassert(supportSharing());

	std::string shareName("groupedTex:");
	for(TTexList::const_iterator it = _Textures.begin(); it != _Textures.end(); ++it)
	{
		shareName += (*it)->getShareName() + std::string("|");
	}
	return shareName;
}

///=====================================================================================================
void CTextureGrouped::serial(NLMISC::IStream &f)
{
	f.serialVersion(1);

	if (f.isReading())
	{
		TTexList texList;

		/// read the number of textures
		uint32 nbTex;
		f.serial(nbTex);

		/// cerate a vector of textures
		ITexture *ptTex = NULL;
		texList.reserve(nbTex);
		for (uint k = 0; k < nbTex; ++k)
		{
			f.serialPolyPtr(ptTex);
			texList.push_back(ptTex);
		}

		// setup the textures
		setTextures(&texList[0], nbTex, false);
		forceGenUVs();
		touch();
	}
	else
	{
		f.serial(_NbTex);
		ITexture *ptTex;
		for (TTexList::iterator it = _Textures.begin(); it != _Textures.end(); ++it)
		{
			ptTex = *it;
			f.serialPolyPtr(ptTex);
		}
	}
}

///=====================================================================================================
void CTextureGrouped::release()
{
	ITexture::release();
	for(uint k = 0; k < _NbTex; ++k)
	{
		if ( _Textures[k]->getReleasable())
		{
			_Textures[k]->release();
		}
	}
}

///=====================================================================================================
void CTextureGrouped::makeDummies(CSmartPtr<ITexture> *textureTab,uint nbTex)
{
 	for(uint k = 0; k < nbTex; ++k)
	{
		textureTab[k] = new CTextureFile("DummyTex"); // well this shouldn't exist..
	}
}

///=====================================================================================================
void CTextureGrouped::displayIncompatibleTextureWarning(CSmartPtr<ITexture> *textureTab,uint nbTex)
{
	nlwarning("=======================================================");
	nlwarning("CTextureGrouped : found incompatible textures, that differs by size or format :"	);
	for(uint k = 0; k < nbTex; ++k)
	{
		if (textureTab[k])
		{
			nlwarning((textureTab[k]->getShareName()).c_str());
		}
		else
		{
			nlwarning("NULL Texture found");
		}
	}
	nlwarning("=======================================================");
}


} // NL3D
