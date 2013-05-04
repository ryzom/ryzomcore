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

#ifndef NL_INPUT_DEVICE_MANAGER_H
#define NL_INPUT_DEVICE_MANAGER_H

#include "types_nl.h"
#include "game_device.h"
#include "common.h"

namespace NLMISC
{


struct IMouseDevice;
struct IKeyboardDevice;



struct EInputDevice : public Exception
{
	EInputDevice(const char *reason) : Exception(reason) {}
};


/** Interface for objects that give low level access to devices (mouse, keyboard, joypads and joysticks).
  * Generally an object implementing this interface will send the appropriate events when a device is 'created'.
  * (Example of implementation : a direct input event emitter)
  */
struct IInputDeviceManager
{
	// Test if a mouse has been created (by a call to getMouseDeivce)
	virtual	bool			isMouseCreated() = 0;
	/// Create the low level mouse device if needed (one active at a time for that object, repeated calls returns the same pointer). An exception if thrown if it couldn't be obtained.
	virtual IMouseDevice	*getMouseDevice(bool hardware) throw(EInputDevice) = 0;
	/// remove the low level mouse
	virtual void			releaseMouse() = 0;
	/// Create the low level keyboard device if needed (one active at a time for that object, repeated calls returns the same pointer). An exception if thrown if it couldn't be obtained.
	virtual IKeyboardDevice	*getKeyboardDevice() throw(EInputDevice) = 0;
	/// remove the low level keyboard
	virtual void			releaseKeyboard() = 0;
	// Enumerates current game devices (gamepads, joystick etc.). The result is stored in the given vector
	virtual void			enumerateGameDevice(TDeviceDescVect &descs) throw(EInputDevice) = 0;
	// Create the given game device interface from its instance name. It also means that it will begin to sends events.
	virtual IGameDevice		*createGameDevice(const std::string &instanceName) throw(EInputDevice) = 0;
	// Release the given game device.
	virtual void			releaseGameDevice(IGameDevice *gd) = 0;
};


} // NLMISC


#endif // NL_INPUT_DEVICE_MANAGER_H

/* End of device_manager.h */
