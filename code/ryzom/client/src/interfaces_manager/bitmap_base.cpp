// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#include "stdpch.h"


/////////////
// Include //
/////////////
// Misc
#include "nel/misc/bitmap.h"
#include "nel/misc/path.h"
// Client
#include "bitmap_base.h"
#include "interfaces_manager.h"


//-----------------------------------------------
// CBitmapBase :
// Constructor.
//-----------------------------------------------
CBitmapBase::CBitmapBase()
{
	init(0, CRGBA(255,255,255,255));
}// CBitmapBase //

//-----------------------------------------------
// CBitmapBase :
// Constructor.
//-----------------------------------------------
CBitmapBase::CBitmapBase(uint texture, const CRGBA &rgba)
{
	init(texture, rgba);
}// CBitmapBase //

//-----------------------------------------------
// init :
// Initialize the class(only 1 function for all constructor -> easier).
//-----------------------------------------------
void CBitmapBase::init(uint texture, const CRGBA &rgba)
{
	_RGBA		= rgba;
	_Tiled		= false;
	CBitmapBase::texture( texture );
}// init //


//-----------------------------------------------
// texture :
// Set the texture.
//-----------------------------------------------
void CBitmapBase::texture(uint texture)
{
	_TextureId	= texture;
	_Texture = CInterfMngr::getTexture(_TextureId);

	_TexturePath.clear();
	_TextureWidth = 0;
	_TextureHeight = 0;

	if ( _Texture != NULL)
	{
		_TexturePath = NLMISC::CPath::lookup( _Texture->getFileName(), false);

		if ( ! _TexturePath.empty() )
		{
			NLMISC::CBitmap bitmap;
			bitmap.loadSize( _TexturePath , _TextureWidth, _TextureHeight);
		}
	}
}// texture //


//-----------------------------------------------
// textureId:
// get the textureId.
//-----------------------------------------------
uint CBitmapBase::textureId() const
{
	return _TextureId;
}// rgba //


//-----------------------------------------------
// rgba :
// Set the rgba.
//-----------------------------------------------
void CBitmapBase::rgba(const CRGBA &rgba)
{
	_RGBA		= rgba;
}// rgba //


//-----------------------------------------------
// rgba :
// get the rgba.
//-----------------------------------------------
const CRGBA &CBitmapBase::rgba() const
{
	return _RGBA;
}// rgba //
