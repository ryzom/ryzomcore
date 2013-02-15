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

#ifndef NL_HEAT_HAZE_H
#define NL_HEAT_HAZE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/vertex_buffer.h"

namespace NL3D {

class IDriver ;
class CScene ;

/**
 * This perform  a heat haze effect at the horizon of the scene.
 * This make use of 2d deformation of the frame buffer
 * \param width  : viewport width
 * \param height : viewport height
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CHeatHaze
{
public:
	// you must have setup effect2d before calling this
	static void performHeatHaze(uint width, uint height, CScene &s, IDriver *drv);
};


} // NL3D


#endif // NL_HEAT_HAZE_H

/* End of heat_haze.h */
