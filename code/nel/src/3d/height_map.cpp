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

#include "nel/3d/height_map.h"


using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
void			CHeightMap::resize(uint w, uint h)
{
	_Width= w;
	_Height= h;
	Heights.resize(w*h);
}


// ***************************************************************************
void			CHeightMap::buildFromBitmap(const NLMISC::CBitmap &bitmap0)
{
	// copy bitmap.
	CBitmap		bitmap= bitmap0;
	// convert to luminance.
	bitmap.convertToType(CBitmap::Luminance);

	// resize array.
	uint	w, h;
	w= bitmap.getWidth();
	h= bitmap.getHeight();
	resize(w,h);

	// get luminance image.
	CObjectVector<uint8>	&array= bitmap.getPixels();
	// invert the image in Y.
	for(uint y=0;y<h;y++)
	{
		for(uint x=0;x<w;x++)
		{
			uint8	v= array[(h-1-y)*w+x];
			Heights[y*w+x]= v;
		}
	}
}


// ***************************************************************************
float			CHeightMap::getZ(uint x, uint y) const
{
	nlassert(x<_Width && y<_Height);
	return Heights[y*_Width+x]*MaxZ/255;
}



} // NL3D
