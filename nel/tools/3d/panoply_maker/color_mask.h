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

#ifndef _COLOR_MASK_H
#define _COLOR_MASK_H

#include "color_modifier.h"
#include <string>

struct CColorMask
{
	// the extension used for this color mask.
	// Example : if this is named 'mask1', a base texture 
	// 'tex.tga' will use this mask if there's a tex_mask1.tga file
	std::string			MaskExt;

	/// the various color modifiers that must be applied on textures
	TColorModifierVect	CMs;
};

typedef std::vector<CColorMask> TColorMaskVect;



#endif

