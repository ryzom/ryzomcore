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



#ifndef CL_INTERF_SCRIPT_H
#define CL_INTERF_SCRIPT_H


/////////////
// Include //
/////////////
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
// Client.
#include "control.h"
#include "osd.h"
#include "button_base.h"
// std
#include <vector>

#define _MAX_LINE_SIZE 512

///////////
// Using //
///////////
using NLMISC::CRGBA;


//////////////
// Function //
//////////////
/// Get a "float" in the script.
float getFloat();
/// Get an "int" in the script.
sint getInt();
/// Get all value for a RGBA in the script.
CRGBA getRGBA();
/// Get a vector of float of size (nbCol) from the script
std::vector<float> getVectorOfFloat(uint8 nbCol);

/// Get the Hot Spot of a control.
CControl::THotSpot getHotSpot();
/// Get the display mode for the background.
COSD::TBG getBGMode();
/// Get the display mode for the background.
CButtonBase::TBG getBGMode2();
/// Get the display mode for the Title Bar.
COSD::TTB getTBMode();

#endif // CL_INTERF_SCRIPT_H

/* End of interf_script.h */
