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

#ifndef NL_HEIGHT_MAP_H
#define NL_HEIGHT_MAP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bitmap.h"
#include <vector>


namespace NL3D
{


// ***************************************************************************
/**
 * A HeightMap. The heightMap is oriented left-right(X+) / bottom-top(Y+).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CHeightMap
{
public:
	std::vector<uint8>		Heights;

	/// The origin of the bottom-left corner of this heightmap.
	float					OriginX, OriginY;
	/// The size of one Element ot this HeightMap (eg: 160x160 for a zone).
	float					SizeX, SizeY;
	/// The range of heights: 0 to MaxZ.
	float					MaxZ;

public:
	/// Constructor
	CHeightMap()
	{
		SizeX= SizeY= 160;
		OriginX= OriginY= 0;
		MaxZ= 100;
	}


	/// resize the heightmap. resize Heights too.
	void			resize(uint w, uint h);
	uint			getWidth() const {return _Width;}
	uint			getHeight() const {return _Height;}


	/// return the ZValue, interval is: 0-MaxZ. nlassert(x,y).
	float			getZ(uint x, uint y) const;


	/** This tool method build a HeightMap, from a bitmap, resize(), and fill Heights.
	 * value are read from bitmap luminance value. NB: Lines are inverted, because bitmap definition is left-right / top-bottom.
	 */
	void			buildFromBitmap(const NLMISC::CBitmap &bitmap);

private:
	/// The size of this array. Heights.size
	uint			_Width, _Height;
};


} // NL3D


#endif // NL_HEIGHT_MAP_H

/* End of height_map.h */
