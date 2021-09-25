// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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





#ifndef CL_SKY_RENDER_H
#define CL_SKY_RENDER_H

#include "nel/misc/rgba.h"


namespace NL3D
{
	class UCamera;
	class UInstance;
}

class CLightCycleManager;

// create the sky scene
void createSkyScene();

// Render the sky dome
void renderSky(const CLightCycleManager &lcm, NLMISC::CRGBA fogColor);

// delete the sky scene
void deleteSkyScene();


// the scene containing sky
extern NL3D::UScene	    *SkyScene;
// Instance of sky used for 1st pass drawing
extern NL3D::UInstance  Sky;
// Instance of sky used for 2ndt pass drawing. We use a second instance to keep texture pointers.
extern NL3D::UInstance  Sky2ndPass;
// A shape used to draw fog part
extern NL3D::UInstance	SkyFogPart;



#endif

