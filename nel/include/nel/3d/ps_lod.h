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

#ifndef NL_PS_LOD_H
#define NL_PS_LOD_H

#include "nel/misc/types_nl.h"


namespace NL3D {


/// lod for located bindables. (see ps_located.h)
enum TPSLod
{
	PSLod1n2 = 0, // the object is dealt with for every distance (his is the default)
	PSLod1,  // the object is dealt when the system center is near
	PSLod2,  // the object is dealt when the system center is far
};







} // NL3D


#endif // NL_PS_LOD_H

/* End of ps_lod.h */
