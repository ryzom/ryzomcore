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

#include "nel/3d/texture.h"
#include "nel/3d/driver.h"
#include <vector>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


/*==================================================================*\
							ITEXTURE
\*==================================================================*/


// ***************************************************************************
ITexture::ITexture()
{
	_Touched= false;
	_FilterOrWrapModeTouched = false;
	_GoodGenerate= false;
	_Releasable= true;
	_RenderTarget= false;
	_WrapS= _WrapT= Repeat;
	_UploadFormat= Auto;
	_MagFilter= Linear;
	_MinFilter= LinearMipMapLinear;
}


// ***************************************************************************
ITexture::~ITexture()
{
	// Must kill the drv mirror of this texture.
	releaseDriverSetup();
}


// ***************************************************************************
void	ITexture::releaseDriverSetup()
{
	// Must kill the drv mirror of this texture.
	if (TextureDrvShare) TextureDrvShare.kill();
}


// ***************************************************************************
ITexture &ITexture::operator=(const ITexture &tex)
{
	// The operator= do not copy drv info
	// set touched=true. _Releasable is copied.
	_UploadFormat= tex._UploadFormat;
	_Releasable= tex._Releasable;
	_WrapS= tex._WrapS;
	_WrapT= tex._WrapT;
	_MagFilter= tex._MagFilter;
	_MinFilter= tex._MinFilter;
	_RenderTarget= tex._RenderTarget;
	_FilterOrWrapModeTouched = tex._FilterOrWrapModeTouched;
	touch();
	return *this;
}


// ***************************************************************************
void		ITexture::setUploadFormat(TUploadFormat pf)
{
	if(pf!=_UploadFormat)
	{
		_UploadFormat= pf;
		// All the texture may be reloaded...
		touch();
	}
}


// ***************************************************************************
void		ITexture::setFilterMode(TMagFilter magf, TMinFilter minf)
{
	if (_MagFilter != magf)
	{
		_MagFilter= magf;
		_FilterOrWrapModeTouched = true;
	}
	// If the MipMap mode has switched Off/On, then must recompute...
	bool	precOff= mipMapOff();
	if (_MinFilter != minf)
	{
		_MinFilter= minf;
		_FilterOrWrapModeTouched = true;
	}
	bool	nowOff= mipMapOff();

	if(precOff!=nowOff)
	{
		// Must recompute mipmaps!!
		touch();
	}
}



// ***************************************************************************
CTextureDrvShare::~CTextureDrvShare()
{
	_Driver->removeTextureDrvSharePtr(_DriverIterator);
}
// ***************************************************************************
ITextureDrvInfos::~ITextureDrvInfos()
{
	// NB: _Driver may be NULL because texture may not be stored in the share texture map.
	// so there is no need to remove it from this map!!
	if(_Driver)
		_Driver->removeTextureDrvInfoPtr(_DriverIterator);
}


// ***************************************************************************
void	ITexture::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- _LoadGrayscaleAsAlpha
	Version 0:
		- base version.
	*/

	sint	ver= f.serialVersion(1);

	f.serialEnum(_UploadFormat);
	f.serialEnum(_WrapS);
	f.serialEnum(_WrapT);
	f.serialEnum(_MinFilter);
	f.serialEnum(_MagFilter);
	if(ver>=1)
		f.serial(_LoadGrayscaleAsAlpha);
}

// ***************************************************************************

void ITexture::setRenderTarget (bool enable)
{
	_RenderTarget = enable;
	touch ();
}

// ***************************************************************************


} // NL3D
