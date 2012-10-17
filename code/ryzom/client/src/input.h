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




#ifndef CL_INPUT_H
#define CL_INPUT_H


namespace NLMISC
{
	class CEvent;
}

/////////////
// INCLUDE //
/////////////
// Misc


const double FREE_LOOK_ANGLE_TO_MICKEYS = 5000;

//////////////
// FUNCTION //
//////////////

// Initialize the mouse
bool	InitMouseWithCursor (bool hardware);

// Is mouse cursor hardware ?
bool	IsMouseCursorHardware ();

// Set the mouse mode. Call this method once per frame to update window size
void	UpdateMouse ();

// Use this method to toggle the mouse (freelook <- cursor)
void	SetMouseFreeLook ();

// test if free look mode
bool	IsMouseFreeLook();

// Use this method to toggle the mouse (freelook -> cursor)
void	SetMouseCursor (bool updatePos = true);

// Use this method to set the cursor speed
void	SetMouseSpeed (float speed);

// Use this method to set the cursor acceleration
void	SetMouseAcceleration (uint accel);

// handle capturing of mouse on button up / button down
void HandleSystemCursorCapture(const NLMISC::CEvent &event);

// get state of mouse button, as a bitfield formatted like NLMISC::TMouseButton (modifier keys are not included)
uint GetMouseButtonsState();

// 'nice' the inputs : disable direct input in ctor, then re-enable if needed in dtor
// useful for non-blocking loading.
// NB : CNiceInputAuto maintains an internal counter, so sevral nested CNiceInputAuto's will remain nice
class CNiceInputAuto
{
public:
	CNiceInputAuto();
	~CNiceInputAuto();
private:
	static sint _Count;
};

#endif // CL_INPUT_H

/* End of misc.h */





















