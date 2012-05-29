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

#ifndef NL_ASYNC_TEXTURE_BLOCK_H
#define NL_ASYNC_TEXTURE_BLOCK_H

#include "nel/misc/types_nl.h"
#include "nel/3d/material.h"

#include <limits>

namespace NL3D
{


class CMeshBaseInstance;


// ***************************************************************************
/**
 * Descriptor of TextureFile Names.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CAsyncTextureBlock
{
public:
	// List of texture file names for a material
	std::string			TextureNames[IDRV_MAT_MAXTEXTURES];

	CAsyncTextureBlock()
	{
		for(uint i=0;i<IDRV_MAT_MAXTEXTURES;i++)
		{
			TextIds[i]= std::numeric_limits<uint>::max();
			IsTextureFile[i]= false;
		}
	}
	bool				isTextureFile(uint stage) const {return IsTextureFile[stage];}

private:
	friend	class CMeshBaseInstance;
	// Ids in the CAsyncTextureManager
	uint				TextIds[IDRV_MAT_MAXTEXTURES];
	// true if the texture in the stage is really a texture file.
	bool				IsTextureFile[IDRV_MAT_MAXTEXTURES];

};


} // NL3D


#endif // NL_ASYNC_TEXTURE_BLOCK_H

/* End of async_texture_block.h */
