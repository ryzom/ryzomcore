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

#include "stdmisc.h"

#include "nel/misc/rect.h"
#include "nel/misc/vector_2f.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

// *********************************************************************
void CRect::extend (sint32 x, sint32 y)
{
	if (x<X)
	{
		Width+=(uint32)(X-x);
		x=X;
	}
	else if (x>=(X+(sint32)Width))
		Width=(uint32)(x-X+1);
	if (y<Y)
	{
		Height+=(uint32)(Y-y);
		y=Y;
	}
	else if (y>=(Y+(sint32)Height))
		Height=(uint32)(y-Y+1);
}


// *********************************************************************
void CRect::setWH(sint32 x, sint32 y, uint32 width, uint32 height)
{
	X=x;
	Y=y;
	Width=width;
	Height=height;
}


// *********************************************************************
void CRect::set(sint32 x0, sint32 y0, sint32 x1, sint32 y1)
{
	if (x0 < x1)
	{
		X = x0;
		Width = x1 - x0;
	}
	else
	{
		X = x1;
		Width = y0 - y1;
	}

	if (y0 < y1)
	{
		Y = y0;
		Height = y1 - y0;
	}
	else
	{
		Y = y1;
		Height = y0 - y1;
	}
}

// *********************************************************************
CRect::CRect (sint32 x, sint32 y, uint32 width, uint32 height)
{
	setWH(x, y, width, height);
}




} // NLMISC
