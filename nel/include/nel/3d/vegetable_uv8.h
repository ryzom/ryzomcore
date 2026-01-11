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

#ifndef NL_VEGETABLE_UV8_H
#define NL_VEGETABLE_UV8_H

#include "nel/misc/types_nl.h"


namespace NL3D
{


/**
 * A simple tuple UV in 8 bits, for Dynamic Lightmap encoding in Alpha components of colors
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CVegetableUV8
{
public:

	uint8	U,V;

};


} // NL3D


#endif // NL_VEGETABLE_UV8_H

/* End of vegetable_uv8.h */
