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

#ifndef NL_TILE_COLOR_H
#define NL_TILE_COLOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"



namespace NL3D {


// ***************************************************************************
/**
 * A basic color Element for CPatchTexture.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTileColor
{
public:
	/// The RGB user color.
	uint16			Color565;

public:

	/// Constructor
	CTileColor() {}

	// For patch with version >= 7, just read color.
	void	serial(NLMISC::IStream &f)
	{
		// NB: still don't use version number, for file size optimisation
		f.xmlSerial (Color565, "COLOR");
	}

};


/**
 * Old version of TileColors for correct serialising.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTileColorOldPatchVersion6
{
public:
	/// The RGB user color.
	uint16			Color565;

public:

	// For patch with version <= 6, read dummy LightVector
	void	serial(NLMISC::IStream &f)
	{
		f.xmlSerial (Color565, "COLOR");
		// read dummy bump light info
		uint8	lightX, lightY, lightZ;
		f.xmlSerial (lightX,lightY,lightZ, "LIGHT_VECTOR");
	}

};


} // NL3D


#endif // NL_TILE_COLOR_H

/* End of tile_color.h */
