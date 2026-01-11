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

#ifndef NL_PRIMITIVE_PROFILE_H
#define NL_PRIMITIVE_PROFILE_H

#include "nel/misc/types_nl.h"


namespace NL3D {


// ****************************************************************************
/**
 * A Primitive counter.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CPrimitiveProfile
{
public:
	uint	NPoints;
	uint	NLines;
	uint	NTriangles;
	uint	NQuads;
	/// The number of triangles strip rendered (if you just draw 1 strip of 400 triangles, NTriangleStrips==400).
	uint	NTriangleStrips;

	CPrimitiveProfile()
	{
		reset();
	}

	void	reset();
};




} // NL3D


#endif // NL_PRIMITIVE_PROFILE_H

/* End of primitive_profile.h */
