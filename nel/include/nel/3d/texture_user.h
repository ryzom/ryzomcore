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

#ifndef NL_TEXTURE_USER_H
#define NL_TEXTURE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_texture.h"
#include "nel/3d/texture.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/texture_mem.h"


namespace NL3D
{


using	NLMISC::CSmartPtr;


// ***************************************************************************
/**
 * UTexture implementation. This Texture must keep a SmartPtr on the real ITexture. Because CMaterial attach texture
 * with SmartPtr too!!
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTextureUser : virtual public UTexture
{
protected:
	CSmartPtr<ITexture>		_Texture;

public:

	CTextureUser()
	{
	}

	/// \name Object
	// @{
	/// This ctpor takes a freshly created texture!! UTexture will owns this texture (via smartptr).
	CTextureUser(ITexture	*text)
	{
		nlassert(text);
		_Texture=text;

		// UTexture and ITexture MUST be the same!
		nlassert((uint)UTexture::WrapModeCount==(uint)ITexture::WrapModeCount);
		nlassert((uint)UTexture::UploadFormatCount==(uint)ITexture::UploadFormatCount);
		nlassert((uint)UTexture::MagFilterCount==(uint)ITexture::MagFilterCount);
		nlassert((uint)UTexture::MinFilterCount==(uint)ITexture::MinFilterCount);
	}
	virtual ~CTextureUser()
	{
		// texture auto deleted.
	}
	// @}

	virtual	void			setWrapS(TWrapMode mode)
	{
		_Texture->setWrapS((ITexture::TWrapMode)(uint32)mode);
	}
	virtual	void			setWrapT(TWrapMode mode)
	{
		_Texture->setWrapT((ITexture::TWrapMode)(uint32)mode);
	}
	virtual	TWrapMode		getWrapS() const
	{
		return (UTexture::TWrapMode)(uint32)_Texture->getWrapS();
	}
	virtual	TWrapMode		getWrapT() const
	{
		return (UTexture::TWrapMode)(uint32)_Texture->getWrapT();
	}
	virtual	void			setUploadFormat(TUploadFormat pf)
	{
		_Texture->setUploadFormat((ITexture::TUploadFormat)(uint32)pf);
	}
	virtual	TUploadFormat	getUploadFormat() const
	{
		return (UTexture::TUploadFormat)(uint32)_Texture->getUploadFormat();
	}
	virtual	void			setFilterMode(TMagFilter magf, TMinFilter minf)
	{
		_Texture->setFilterMode((ITexture::TMagFilter)(uint32)magf, (ITexture::TMinFilter)(uint32)minf);
	}
	virtual	TMagFilter		getMagFilter() const
	{
		return (UTexture::TMagFilter)(uint32)_Texture->getMagFilter();
	}
	virtual	TMinFilter		getMinFilter() const
	{
		return (UTexture::TMinFilter)(uint32)_Texture->getMinFilter();
	}
	virtual	bool			mipMapOff() const
	{
		return _Texture->mipMapOff();
	}
	virtual	bool			mipMapOn() const
	{
		return _Texture->mipMapOn();
	}
	virtual	NLMISC::CRGBA	getPixelColor(sint32 x, sint32 y) const
	{
		return _Texture->getPixelColor(x,y);
	}
	virtual	void setReleasable(bool bReleasable)
	{
		_Texture->setReleasable(bReleasable);
	}

	/// Accessor for UMaterial imp.
	ITexture*				getITexture()
	{
		return _Texture;
	}

	// generate this texture datas
	virtual CBitmap			*generateDatas()
	{
		if (_Texture->isTextureCube()) return NULL;
		_Texture->generate();
		return _Texture;
	}

	// release this texture datas
	virtual void			releaseDatas()
	{
		_Texture->release();
	}
};


// ***************************************************************************
/**
 * UTextureFile implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CTextureFileUser : virtual public UTextureFile, public CTextureUser
{
public:


	/// \name Object
	// @{
	CTextureFileUser() : CTextureUser(new CTextureFile)
	{
		((CTextureFile*)(ITexture*)_Texture)->setAllowDegradation(false);
	}
	CTextureFileUser(std::string s) : CTextureUser(new CTextureFile(s))
	{
		((CTextureFile*)(ITexture*)_Texture)->setAllowDegradation(false);
	}
	virtual ~CTextureFileUser()
	{
	}
	// @}


	virtual	void setFileName(std::string s)
	{
		((CTextureFile*)(ITexture*)_Texture)->setFileName(s);
	}

	virtual	std::string getFileName() const
	{
		return ((CTextureFile*)(ITexture*)_Texture)->getFileName();
	}

	virtual bool	allowDegradation() const
	{
		return _Texture->allowDegradation();
	}
	virtual void	setAllowDegradation(bool allow)
	{
		((CTextureFile*)(ITexture*)_Texture)->setAllowDegradation(allow);
	}
	virtual void	setEnlargeCanvasNonPOW2Tex(bool dontStretch)
	{
		((CTextureFile*)(ITexture*)_Texture)->setEnlargeCanvasNonPOW2Tex(dontStretch);
	}
	virtual bool	getEnlargeCanvasNonPOW2Tex() const
	{
		return ((CTextureFile*)(ITexture*)_Texture)->getEnlargeCanvasNonPOW2Tex();
	}
};


// ***************************************************************************
/**
 * UTextureMem implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CTextureMemUser : virtual public UTextureMem, public CTextureUser
{
public:

	CTextureMemUser(uint w, uint h, CBitmap::TType t)
	{
		uint bpp = 0;
		if (t == CBitmap::RGBA)					bpp = 32;
		else if (t == CBitmap::Luminance)		bpp = 8;
		else if (t == CBitmap::Alpha)			bpp = 8;
		else if (t == CBitmap::AlphaLuminance)	bpp = 16;
		else if (t == CBitmap::DXTC1)			bpp = 4;
		else if (t == CBitmap::DXTC1Alpha)		bpp = 4;
		else if (t == CBitmap::DXTC3)			bpp = 8;
		else if (t == CBitmap::DXTC5)			bpp = 8;
		else if (t == CBitmap::DsDt)			bpp = 16;

		uint size = w*h*bpp/8;
		uint8 *data = new uint8[size];

		CTextureMem *pTxMem = new CTextureMem(data, size, true, false, w, h, t);
		_Texture = pTxMem;
	}

	uint8* getPointer() const
	{
		return ((CTextureMem*)(ITexture*)_Texture)->getPointer();
	}

	void touch()
	{
		((CTextureMem*)(ITexture*)_Texture)->touch();
	}

	void touchRect(const NLMISC::CRect& rect)
	{
		((CTextureMem*)(ITexture*)_Texture)->touchRect(rect);
	}

	uint32 getImageWidth() const
	{
		return ((CTextureMem*)(ITexture*)_Texture)->getImageWidth();
	}

	uint32 getImageHeight() const
	{
		return ((CTextureMem*)(ITexture*)_Texture)->getImageHeight();
	}

};


} // NL3D


#endif // NL_TEXTURE_USER_H

/* End of texture_user.h */
