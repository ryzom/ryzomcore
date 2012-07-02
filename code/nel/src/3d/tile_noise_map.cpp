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

#include "nel/3d/tile_noise_map.h"


namespace NL3D
{


// ***************************************************************************
void		CTileNoiseMap::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serialBuffer((uint8*)Pixels, NL3D_TILE_NOISE_MAP_SIZE * NL3D_TILE_NOISE_MAP_SIZE);
}


} // NL3D
