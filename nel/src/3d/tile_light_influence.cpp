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

#include "nel/3d/tile_light_influence.h"
#include "nel/misc/debug.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


void			CTileLightInfluence::setDiffuseLightFactor(uint i, uint8 factor)
{
	nlassert(CTileLightInfluence::NumLightPerCorner == 2);
	nlassert(i == 0 || i == 1);

	// Divide by 16.
	factor>>=4;

	// set to the ith light.
	uint8	mask= 0x0F << (4*i);
	// clear.
	PackedLightFactor&= ~mask;
	// set.
	PackedLightFactor|= factor << (4*i);
}


uint8			CTileLightInfluence::getDiffuseLightFactor(uint i) const
{
	nlassert(CTileLightInfluence::NumLightPerCorner == 2);
	nlassert(i == 0 || i == 1);

	// Choose what factor
	uint8	ret= PackedLightFactor >> (4*i);
	ret&= 0x0F;

	// expand to 0..255
	return ret + (ret<<4);
}


void			CTileLightInfluence::serial(NLMISC::IStream &f)
{
	nlassert(CTileLightInfluence::NumLightPerCorner == 2);
	// No version for smaller size on disk !!

	f.serial(Light[0], Light[1], PackedLightFactor);
}



} // NL3D
