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

#include "nel/3d/tile_lumel.h"

using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
void CTileLumel::createUncompressed (uint8 interpolated, uint8 shaded)
{
	// Shading
	Shaded=shaded;

	// Same color ?
	if (interpolated!=shaded)
	{
		// Compute compressed value
		sint temp=((shaded<<3)/(sint)(interpolated?interpolated:1));
		clamp (temp, 0, 15);

		// After decompression
		uint decompressed=(((uint)interpolated*((uint)temp))>>3)+4;
		if (decompressed>255)
			decompressed=255;

		// Need to compress ?
		if (abs((sint)shaded-(sint)decompressed)>=abs((sint)shaded-(sint)interpolated))
		{
			// Shadow not present
			_ShadowValue=0xff;
		}
		else
		{
			// Shadow
			_ShadowValue=temp;
		}
	}
	else
	{
		// Shadow not present
		_ShadowValue=0xff;
	}
}

// ***************************************************************************
void CTileLumel::pack (CStreamBit& stream) const
{
	// There is shadow here ?
	if (isShadowed ())
	{
		// Put a true
		stream.pushBackBool(true);

		// Read the shadow value
		stream.pushBack4bits(_ShadowValue);
	}
	else
		// Put a false
		stream.pushBackBool(false);
}


} // NL3D


/* End of tile_lumel.h */
