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



#ifndef NL_BITMAP_BASE_H
#define NL_BITMAP_BASE_H


/////////////
// Include //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
// 3D Interface.
#include "nel/3d/u_driver.h"

///////////
// Using //
///////////
using NLMISC::CRGBA;


/**
 * <Class description>
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CBitmapBase
{
protected:
	uint					_TextureId;
	NL3D::UTextureFile *	_Texture;
	CRGBA					_RGBA;

public:
	/// Constructor
	CBitmapBase();
	CBitmapBase(uint texture, const CRGBA &rgba);

	/// Set the texture.
	void texture(uint texture);

	/// get the texture.
	uint textureId() const;

	/// Set the rgba.
	void rgba(const CRGBA &rgba);

	/// get the rgba.
	const CRGBA &rgba() const;

	/// set the mode of the bitmap, either tiled (true) or stretched (false)
	inline void tiled(bool tiled = true) { _Tiled = tiled; }

private:
	inline void init(uint texture, const CRGBA &rgba);

// variables used to store internal datas
protected:
	/// bool indicating if the bitmap is to be displayed tiled or stretched (default = false = stretched)
	bool				_Tiled;

	mutable std::string	_TexturePath;
	mutable uint32		_TextureWidth;
	mutable uint32		_TextureHeight;
};


#endif // NL_BITMAP_BASE_H

/* End of bitmap_base.h */
