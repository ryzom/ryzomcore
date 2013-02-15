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

#ifndef NL_TEXTURE_H
#define NL_TEXTURE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/stream.h"
#include "nel/misc/rect.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/object_arena_allocator.h"
#include <string>
#include <list>
#include <map>


namespace NL3D
{


using NLMISC::CBitmap;


class	IDriver;


// ****************************************************************************

// List typedef.
class	ITextureDrvInfos;
class	CTextureDrvShare;
class TTexDrvInfoPtrMap : public std::map< std::string, ITextureDrvInfos*> {};
typedef	std::list<CTextureDrvShare*>		TTexDrvSharePtrList;
typedef	TTexDrvInfoPtrMap::iterator			ItTexDrvInfoPtrMap;
typedef	TTexDrvSharePtrList::iterator		ItTexDrvSharePtrList;


// Class for interaction of textures with Driver.
// ITextureDrvInfos represent the real data of the texture, stored into the driver (eg: just a GLint for opengl).
class ITextureDrvInfos : public NLMISC::CRefCount
{
private:
	// _Driver==NULL if ITexture::supportSharing()==false
	IDriver					*_Driver;
	ItTexDrvInfoPtrMap		_DriverIterator;

public:
	ITextureDrvInfos(IDriver *drv, ItTexDrvInfoPtrMap it) {_Driver= drv; _DriverIterator= it;}
	ITextureDrvInfos(class IDriver& driver);
	virtual ~ITextureDrvInfos(void);

	// For Debug info. return the memory cost of this texture
	virtual uint	getTextureMemoryUsed() const =0;
};

// Many ITexture may point to the same ITextureDrvInfos, through CTextureDrvShare.
class CTextureDrvShare : public NLMISC::CRefCount
{
private:
	IDriver					*_Driver;
	ItTexDrvSharePtrList	_DriverIterator;
	// The ITexture associated (for debug purpose)
	class ITexture			*_OwnerTexture;

public:
	NLMISC::CSmartPtr<ITextureDrvInfos>		DrvTexture;

public:
	CTextureDrvShare(IDriver *drv, ItTexDrvSharePtrList it, ITexture *owner) {_Driver= drv; _DriverIterator= it; _OwnerTexture= owner;}
	~CTextureDrvShare();

	// get the ITexture that owns this DrvShare. for Dbg purpose
	class ITexture			*getOwnerTexture() const {return _OwnerTexture;}
};


// ****************************************************************************
/**
 * Interface for textures
 *
 * Sharing System note: The deriver may implement sharing system by implement supportSharing() and getShareName().
 * Such a texture may return a Unique Name for sharing. If the driver already has this texture, it will reuse it.
 * As a direct impact, you cannot invalidate part of the textures with shared texture. This is logic, since the Unique
 * sharname of the texture must represent all of it.
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class ITexture : public CBitmap, public NLMISC::CRefCount, public NLMISC::IStreamable
{
public:
	NL_USES_DEFAULT_ARENA_OBJECT_ALLOCATOR // for fast alloc
	// Those enums MUST be the same than in UTexture!!
	enum	TWrapMode
	{
		Repeat= 0,
		Clamp,

		WrapModeCount
	};

	enum	TUploadFormat
	{
		Auto= 0,
		RGBA8888,
		RGBA4444,
		RGBA5551,
		RGB888,
		RGB565,
		DXTC1,
		DXTC1Alpha,
		DXTC3,
		DXTC5,
		Luminance,
		Alpha,
		AlphaLuminance,
		DsDt,
		UploadFormatCount
	};


	/** Magnification mode.
	 * Same behavior as OpenGL.
	 */
	enum	TMagFilter
	{
		Nearest=0,
		Linear,

		MagFilterCount
	};

	/** Minifying mode.
	 * Same behavior as OpenGL. If the bitmap has no mipmap, and mipmap is required, then mipmaps are computed.
	 */
	enum	TMinFilter
	{
		NearestMipMapOff=0,
		NearestMipMapNearest,
		NearestMipMapLinear,
		LinearMipMapOff,
		LinearMipMapNearest,
		LinearMipMapLinear,

		MinFilterCount
	};

	/** Category String
	 */
	class CTextureCategory : public NLMISC::CRefCount
	{
	public:
		std::string		Name;

		CTextureCategory() {}
		CTextureCategory(const std::string &name) : Name(name) {}
	};

public:

	/// Object.
	// @{
	/// By default, a texture is releasable.
	ITexture();
	/// see operator=.
	ITexture(const ITexture &tex) : CBitmap(), CRefCount(), IStreamable() {operator=(tex);}
	/// Need a virtual dtor.
	virtual ~ITexture();
	/// The operator= do not copy drv info, and set touched=true. _Releasable, WrapMode and UploadFormat are copied.
	ITexture &operator=(const ITexture &tex);
	// @}


	/// \name Texture parameters.
	/** By default, parameters are:
		- WrapS==Repeat
		- WrapT==Repeat
		- UploadFormat== Auto
		- MagFilter== Linear.
		- MinFilter= LinearMipMapLinear.

		NB: if multiple ITexture acces the same data via the sharing system (such as a CTextureFile), then:
			- WrapS/WrapT is LOCAL for each ITexture (ie each ITexture will have his own Wrap mode) => no duplication
				is made.
			- UploadFormat may duplicate the texture in video memory. There is one texture per different UploadFormat.
			- MinFilter may duplicate the texture in video memory in the same way, whether the texture has mipmap or not.
	 */
	// @{
	void			setWrapS(TWrapMode mode);
	void			setWrapT(TWrapMode mode);
	TWrapMode		getWrapS() const {return _WrapS;}
	TWrapMode		getWrapT() const {return _WrapT;}
	/** Replace the uploaded format of the texture.
	 * If "Auto", the driver use CBitmap::getPixelFormat() to find the best associated pixelFormat.
	 * When no alpha is wanted (RGB, Luminance....), texture default output is 1.0.
	 * For "Alpha" mode, RGB output is (0,0,0).
	 */
	void			setUploadFormat(TUploadFormat pf);
	TUploadFormat	getUploadFormat() const {return _UploadFormat;}
	virtual         void setFilterMode(TMagFilter magf, TMinFilter minf);
	TMagFilter		getMagFilter() const {return _MagFilter;}
	TMinFilter		getMinFilter() const {return _MinFilter;}
	bool			mipMapOff() const {return _MinFilter==NearestMipMapOff || _MinFilter==LinearMipMapOff;}
	bool			mipMapOn() const {return !mipMapOff();}
	// @}


	/**
	 *  This method invalidates all the texture surface. When the driver calls generate, the
	 *  texture will rebuild all the texture and the driver will update it.
     *
	 * \see isAllInvalidated(), generate(), touchRect(), touched(), _ListInvalidRect
	 */
	void	touch()
	{
		_ListInvalidRect.clear ();
		_Touched=true;
		_GoodGenerate= false;
	}

	/**
	 *  This method invalidates a rectangle of the texture surface. When the driver calls generate, the
	 *  texture could rebuild only this part of texture and the driver will update only those rectangles.
	 *
	 *  This method is incompatible with textures which support sharing (see class description).
	 *  This method is incompatible with compressed textures.
	 *  This method is incompatible with cube textures.
     *
	 * \see isAllInvalidated(), generate(), touch(), touched(), _ListInvalidRect
	 */
	void	touchRect(const NLMISC::CRect& rect)
	{
		// The texture must not support sharing....
		nlassert(!supportSharing());
		nlassert(!isTextureCube());
		// Don't invalidate the rectangle if the full texture is already invalidated.
		if (!isAllInvalidated ())
		{
			// Add the region to invalidate list
			_ListInvalidRect.push_back (rect);
			// Touch flag
			_Touched=true;
		}

		_GoodGenerate= false;
	}


	/**
	 * Return whether texture can be released. If it returns true, the driver will release the texture
	 * after generate it and upload it into the videomemory by calling release(). If it returns false,
	 * the driver won't release the texture.
	 *
	 * \return true if texture can be released, false else
	 * \see setReleasable(), generate()
	 */
	bool getReleasable() const { return _Releasable; }


	/**
	 * Set if texture can be released
	 * If it is true, the driver will release the texture after generating it and upload it into the
	 * videomemory by calling release(). If it is false, the driver won't release the texture.
     *
	 * \see getReleasable(), generate()
	 * \param true if texture can be released, false else
	 */
	void setReleasable(bool r) { _Releasable = r; }

	/**
	 * Generate the texture pixels.
	 *
	 * This method is called by the driver when it needs to generate pixels of the texture. If the
	 * texture is used for the first time or if it is touched, the driver will call this method.
	 * For exemple, a texture file will load the bitmap in this method.
	 *
	 * If the invalidate rect list is empty, generate() rebuild all the texture.
	 * If the invalidate rect list is not empty, generate() rebuilds only the invalidate rectangles
	 * in the list.
	 *
	 * Don't clear the touch flag or the invalid rectangle list until updating the texture in generate().
	 * It's the generate()'s caller jobs.
	 *
	 * After generation, if the texture is releasable, the driver will release the texture by calling
	 * release().
	 *
	 * NB: a flag is maintained to see if the generated bitmap is coherent with texture description (see touch*()).
	 * So if you do {generate(); generate();}, you only get 1 real bitmap generation...
	 *
	 * If, after the doGenerate, the bitmap format is compressed (DXTC) and no mipmaps have been generated, the
	 * mipmap are disabled beacause the user probably don't want the driver to unpacks the texture, generates the mipmaps
	 * and repacks the dxtc texture (that takes a lot of CPU time).
	 *
	 * \param async tells the texture if the call is made asynchronously or not.
	 *
	 * \see isAllInvalidated(), touch(), touched(), touchRect(), clearTouched(), _ListInvalidRect
	 * \see getReleasable(), setReleasable()
	 */
	void generate(bool async = false)
	{
		if(!_GoodGenerate)
		{
			doGenerate(async);
			_GoodGenerate=true;
		}
	}

	/**	Advanced. erase the _GoodGenerate=true. Special if you want to setup directly the bitmap without
	 *	using generate().
	 *	USE IT WITH CARE!! (used by the CAsyncTextureManager)
	 */
	void validateGenerateFlag() {_GoodGenerate=true;}

	/**
	 * Release the texure (free memory)
	 */
	virtual void release() { reset(); _GoodGenerate= false; }

	/**
	 * Does this texture support sharing system.
	 */
	virtual bool			supportSharing() const {return false;}

	/**
	 * Return the Unique ident/name of the texture, used for Driver sharing caps.
	 * Deriver should add a prefix for their texture type. eg "file::pipoland", "noise::4-4-2" etc....
	 */
	virtual std::string		getShareName() const {return std::string();}

	/**
	 * Tells if the texture has been setuped by the driver.
	 */
	bool	setupedIntoDriver() const
	{
		return TextureDrvShare!=NULL;
	}

	/// Release the Driver info for this texture (if any). Call it with care.
	void	releaseDriverSetup();

	/// Does this texture allow the driver to degrade
	virtual bool allowDegradation() const { return false; }

	/// serial ITexture basic infos (clamp ...).
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/** Select a texture among several other (if this texture is a set of texture such as CTextureMultiFile)
	  * The default does nothing
	  */
	virtual void selectTexture(uint /* index */) {}

	/// Test whether this texture is selectable
	virtual bool isSelectable() const { return false; }

	/** If this texture is selectable, build a non selectable version of this texture that is setupped with the given slot.
	  * NB : If this texture is selectable, you are ensured that the return pointer is not 'this'
	  */
	virtual ITexture *buildNonSelectableVersion(uint /* index */) { return this; }




	/// Cubic textures.
	// @{
		/// Does this texture is a cube texture
		virtual bool isTextureCube() const { return false; }
	// @}

	// is this texture a bumpmap ?
	virtual bool isBumpMap() const { return false; }

	// is this texture a bloom texture ?
	virtual bool isBloomTexture() const { return false; }

	// For Texture profiling. The smartPtr is kept (NULL default)
	void	setTextureCategory(NLMISC::CSmartPtr<CTextureCategory> &textCat) {_TextureCategory= textCat;}

	/** Render target texture
	 *  Active / disable render target abilities for this texture.
	 */
	void	setRenderTarget (bool enable);

	/** Render target texture
	 *  Get render target abilities for this texture.
	 */
	bool	getRenderTarget () const { return _RenderTarget; }

	// get the texture category
	CTextureCategory	*getTextureCategory() const {return _TextureCategory;}

// ****************************
// Private part.
protected:
	// Derived texture should set it to true when they are updated.
	bool		_Touched : 1;
	bool		_FilterOrWrapModeTouched : 1;


	/**
	 * Generate the texture pixels.
	 *
	 * If the invalidate rect list is empty, generate() must rebuild all the texture.
	 * If the invalidate rect list is not empty, generate() rebuilds only the invalidate rectangles
	 * in the list.
	 *
	 * \see isAllInvalidated(), touch(), touched(), touchRect(), clearTouched(), _ListInvalidRect, generate()
	 * \see getReleasable(), setReleasable()
	 */
	virtual void doGenerate(bool async = false) = 0;


private:
	bool			_GoodGenerate;
	bool			_Releasable;
	bool			_RenderTarget;
	TUploadFormat	_UploadFormat;
	TWrapMode		_WrapS;
	TWrapMode		_WrapT;
	TMinFilter		_MinFilter;
	TMagFilter		_MagFilter;
	NLMISC::CSmartPtr<CTextureCategory>		_TextureCategory;

public:
	// Private Part!!!. For Driver Only.
	//==================================

	NLMISC::CRefPtr<CTextureDrvShare>	TextureDrvShare;
	/**
	 *  List of invalided rectangle. If the list is empty, generate() will rebuild all the texture.
     *
	 * \see isAllInvalidated(), generate(), touch(), touchRect(), touched()
	 */
	std::list<NLMISC::CRect>	_ListInvalidRect;


	/**
	 * Return true if ALL the texture is invalidate, else return false.
	 */
	bool					isAllInvalidated () const
	{
		return  _Touched&&(_ListInvalidRect.begin()==_ListInvalidRect.end());
	}

	/**
	 *  This method return the touched flag. If it is true, the driver will call generate to rebuild the texture.
     *
	 * \see isAllInvalidated(), generate(), touch(), touchRect(), _ListInvalidRect
	 */
	bool	touched (void)
	{
		return _Touched;
	}

	/** See if filter mode or wrap mode have been touched.
	  * If this is the case, the driver should resetup them for that texture (If driver stores filter & wrap mode
	  * per texture (OpenGL) rather than globally (D3D))
	  */
	bool filterOrWrapModeTouched() const { return _FilterOrWrapModeTouched; }

	/*
	 * Clear the touched flag and the invalid rectangle list
	 *
	 * \see isAllInvalidated(), generate(), touch(), touched(), touchRect(), _ListInvalidRect
	 */
	void	clearTouched(void)
	{
		_Touched=false;
		_ListInvalidRect.clear();
	}

	void clearFilterOrWrapModeTouched()
	{
		_FilterOrWrapModeTouched = false;
	}

};

// inlines
inline void	ITexture::setWrapS(TWrapMode mode)
{
	if (_WrapS == mode) return;
	_WrapS = mode;
	_FilterOrWrapModeTouched = true;
}

inline void	ITexture::setWrapT(TWrapMode mode)
{
	if (_WrapT == mode) return;
	_WrapT = mode;
	_FilterOrWrapModeTouched = true;
}

} // NL3D


#endif // NL_TEXTURE_H

/* End of texture.h */
