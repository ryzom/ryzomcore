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

#include "stddirect3d.h"
#include "driver_direct3d.h"

#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

void CDriverD3D::showCursor (bool b)
{
	H_AUTO_D3D(CDriverD3D_showCursor);
	if (b)
	{
		while (ShowCursor(b) < 0) {}
	}
	else
	{
		while (ShowCursor(b) >= 0) {}
	}
};

// ***************************************************************************

void CDriverD3D::setMousePos(float x, float y)
{
	H_AUTO_D3D(CDriver3D_setMousePos);
	if (_HWnd)
	{
		// NeL window coordinate to MSWindows coordinates
		POINT pt;
		pt.x = (int)((float)(_CurrentMode.Width)*x);
		pt.y = (int)((float)(_CurrentMode.Height)*(1.0f-y));
		ClientToScreen (_HWnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
}

// ***************************************************************************

void CDriverD3D::setCapture (bool b)
{
	if (b)
	{
		RECT client;
		GetClientRect (_HWnd, &client);
		POINT pt1,pt2;
		pt1.x = client.left;
		pt1.y = client.top;
		ClientToScreen (_HWnd, &pt1);
		pt2.x = client.right;
		pt2.y = client.bottom;
		ClientToScreen (_HWnd, &pt2);
		client.bottom = pt2.y;
		client.top = pt1.y;
		client.left = pt1.x;
		client.right = pt2.x;
		ClipCursor (&client);
	}
	else
		ClipCursor (NULL);
}

// ***************************************************************************

NLMISC::IMouseDevice	*CDriverD3D::enableLowLevelMouse(bool enable, bool exclusive)
{
	H_AUTO_D3D(CDriverD3D_enableLowLevelMouse);
	if (_EventEmitter.getNumEmitters() < 2)
		return NULL;
	NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<CDIEventEmitter *>(_EventEmitter.getEmitter(1));
	if (enable)
	{
		try
		{
			NLMISC::IMouseDevice *md = diee->getMouseDevice(exclusive);
			return md;
		}
		catch (EDirectInput &)
		{
			return NULL;
		}
	}
	else
	{
		diee->releaseMouse();
		return NULL;
	}
}

// ***************************************************************************

NLMISC::IKeyboardDevice		*CDriverD3D::enableLowLevelKeyboard(bool enable)
{
	H_AUTO_D3D(CDriverD3D_enableLowLevelKeyboard);
	if (_EventEmitter.getNumEmitters() < 2) return NULL;
	NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1));
	if (enable)
	{
		try
		{
			NLMISC::IKeyboardDevice *md = diee->getKeyboardDevice();
			return md;
		}
		catch (EDirectInput &)
		{
			return NULL;
		}
	}
	else
	{
		diee->releaseKeyboard();
		return NULL;
	}
}

// ***************************************************************************

NLMISC::IInputDeviceManager		*CDriverD3D::getLowLevelInputDeviceManager()
{
	H_AUTO_D3D(CDriverD3D_getLowLevelInputDeviceManager);
	if (_EventEmitter.getNumEmitters() < 2) return NULL;
	NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1));
	return diee;
}

// ***************************************************************************

uint CDriverD3D::getDoubleClickDelay(bool hardwareMouse)
{
	H_AUTO_D3D(CDriverD3D_getDoubleClickDelay);
	NLMISC::IMouseDevice *md = NULL;
	if (_EventEmitter.getNumEmitters() >= 2)
	{
		NLMISC::CDIEventEmitter *diee = NLMISC::safe_cast<CDIEventEmitter *>(_EventEmitter.getEmitter(1));
		if (diee->isMouseCreated())
		{
			try
			{
				md = diee->getMouseDevice(hardwareMouse);
			}
			catch (EDirectInput &)
			{
				// could not get device ..
			}
		}
	}
	if (md)
	{
		return md->getDoubleClickDelay();
	}
	// try to read the good value from windows
	return ::GetDoubleClickTime();
}

// ***************************************************************************

} // NL3D
