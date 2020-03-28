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

#include "stdpacs.h"

#include "nel/pacs/vector_2s.h"

namespace NLPACS {


// Test if 2 segments intersects
bool	CVector2s::intersect(const CVector2s& a0, const CVector2s& a1, const CVector2s& b0, const CVector2s& b1, double* pa, double* pb)
{

	double		a0x = CVector2s::unpack(a0.x);
	double		a0y = CVector2s::unpack(a0.y);
	double		a1x = CVector2s::unpack(a1.x);
	double		a1y = CVector2s::unpack(a1.y);
	double		b0x = CVector2s::unpack(b0.x);
	double		b0y = CVector2s::unpack(b0.y);
	double		b1x = CVector2s::unpack(b1.x);
	double		b1y = CVector2s::unpack(b1.y);

	double		ax = +(a1x-a0x);
	double		ay = +(a1y-a0y);
	double		bx = +(b1x-b0x);
	double		by = +(b1y-b0y);
	double		cx = +(b0x-a0x);
	double		cy = +(b0y-a0y);

	double		d = -ax*by + ay*bx;

	if (d == 0.0)
		return false;

	double		a = (bx*cy - by*cx) / d;
	double		b = (ax*cy - ay*cx) / d;

	if (pa != NULL)
		*pa = a;
	if (pb != NULL)
		*pb = b;

	return d != 0.0 && a >= 0.0 && a <= 1.0 && b >= 0.0 && b <= 1.0;
}


} // NLPACS
