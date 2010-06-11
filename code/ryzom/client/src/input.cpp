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



/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Client
#include "actions.h"
#include "input.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/input_handler_manager.h"
#include "client_cfg.h"
#include "time_client.h"
#include "interface_v3/custom_mouse.h"
// 3D
#include "nel/3d/u_driver.h"
// Misc
#include "nel/misc/mouse_device.h"
#include "nel/misc/mouse_smoother.h"
// Game Share


///////////
// USING //
///////////
using namespace NL3D;
using namespace NLMISC;


////////////
// EXTERN //
////////////
extern UDriver					*Driver;
extern UMaterial				GenericMat;
extern CActionsManager			Actions;		// Actions Manager.


////////////
// GLOBAL //
////////////
IMouseDevice					*MouseDevice = NULL;
bool							MouseHardware = false;
bool							MouseFreeLook = false;
float							MouseCursorSpeed = 1.f;
uint							MouseCursorAcceleration = 0;
bool							InitMouseFirstTime = true;
bool							SetMousePosFirstTime = true;

// mask for mouse buttons that are known to be down
uint							DownMouseButtons = 0;

#ifdef NL_OS_UNIX
// on X11 and cocoa, store whether the mouse was captured or not
bool							MouseCapture = false;
#endif

//////////////
// FUNCTION //
//////////////

//*********************************************************************************
uint GetMouseButtonsState()
{
	return DownMouseButtons;
}

//*********************************************************************************
// Initialize the mouse
bool	InitMouseWithCursor (bool hardware)
{
	Driver->showCursor(false);

	// First init ?
	if (MouseDevice)
	{
		// No.. change soft to hard or hard to soft ?
		if (hardware ^ MouseHardware)
		{
			// Ok, reinit the mouse
			Driver->enableLowLevelMouse (false, false);
			MouseDevice = NULL;
			MouseHardware = false;
		}
	}

	// Get the new mouse state
	MouseHardware = hardware;

	// Reinit ?
	if (MouseDevice == NULL)
	{
		if (!ClientCfg.DisableDirectInput)
		{
			// mouse capture not taken in account for hardware mouse
			MouseDevice = Driver->enableLowLevelMouse(true, hardware);
			if (!MouseDevice)
				return false;
		}

		// Update mouse informations
		UpdateMouse ();

		if (InitMouseFirstTime)
		{
			InitMouseFirstTime = false;
		}
		else
		{
			if (!MouseFreeLook)
			{
				// Get the current mouse position
				if (hardware)
				{
					if (CInterfaceManager::getInstance()->getPointer())
					{
						float x = (float)CInterfaceManager::getInstance()->getPointer()->getX()/(float)Driver->getWindowWidth();
						float y = (float)CInterfaceManager::getInstance()->getPointer()->getY()/(float)Driver->getWindowHeight();
						CustomMouse.updateCursor(); // update current hardware icon to avoid to have the plain arrow
						Driver->showCursor(true);
						if (SetMousePosFirstTime)
						{
							SetMousePosFirstTime = false;
						}
						else
						{
							Driver->setMousePos(x, y);
						}
					}
					else
					{
						CustomMouse.updateCursor(); // update current hardware icon to avoid to have the plain arrow
						Driver->showCursor(true);
					}
				}
				else
				{
					CInterfaceManager *pIm = CInterfaceManager::getInstance();
					CViewPointer *vp = pIm->getPointer();
					Driver->showCursor(false);
					SetMousePosFirstTime = false;
					if (vp)
					{
						float x = (float) vp->getX();
						float y = (float) vp->getY();
						// First, hide the hardware mouse
						if (MouseDevice)
						{
							MouseDevice->setMousePos(x, y);
						}
						else
						{
							uint width = Driver->getWindowWidth();
							uint height = Driver->getWindowHeight();
							if (width != 0 && height != 0)
							{
								Driver->setMousePos(x / width, y / height);
							}
						}
					}
				}
			}
		}
	}

	return true;
}

//*********************************************************************************
// Is mouse cursor hardware ?
bool	IsMouseCursorHardware ()
{
	return MouseHardware;
}

//*********************************************************************************
// Set the mouse mode. Call this method once per frame to update window size
void	UpdateMouse ()
{
	// Freelook ?
	if (MouseFreeLook)
	{
		// Raw mode
		if (MouseDevice)
		{
			MouseDevice->setMessagesMode(IMouseDevice::RawMode);
			MouseDevice->setMouseAcceleration(ClientCfg.FreeLookAcceleration);
		}
		else
		{
			// no mouse device implementation on X11 and Cocoa, emulate raw mode
			Driver->emulateMouseRawMode(true);
		}
	}
	else
	{
		// Set the mouse properties
		if (MouseDevice)
		{
			// Get the driver size
			uint32 width, height;
			Driver->getWindowSize(width, height);

			MouseDevice->setMessagesMode(IMouseDevice::NormalMode);
			MouseDevice->setMouseMode(IMouseDevice::XAxis, IMouseDevice::Clamped);
			MouseDevice->setMouseMode(IMouseDevice::YAxis, IMouseDevice::Clamped);
			CRect window (0, 0, width, height);
			MouseDevice->setMouseFrame(window);
			MouseDevice->setFactors(1.f/std::max((float)width, 1.0f), 1.f/std::max((float)height, 1.0f));
			MouseDevice->setMouseSpeed(MouseCursorSpeed);
			MouseDevice->setMouseAcceleration(MouseCursorAcceleration);
		}
		else
		{
			// no mouse device implementation on X11 and Cocoa, emulate raw mode
			Driver->emulateMouseRawMode(false);
		}
	}
	if (!IsSystemCursorCaptured())
	{
		DownMouseButtons = 0;
	}
}

//*********************************************************************************
// Use this method to toggle the mouse (freelook <- cursor)
void	SetMouseFreeLook ()
{
	if (!MouseFreeLook)
	{
		MouseFreeLook = true;
		if (MouseHardware)
			Driver->showCursor(false);
		else
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			if (im)
			{
				CViewPointer *pointer = im->getPointer();
				if (pointer)
					pointer->show (false);
			}
		}
		UpdateMouse ();
	}
}

//*********************************************************************************
bool IsMouseFreeLook()
{
	return MouseFreeLook;
}


//*********************************************************************************
// Use this method to toggle the mouse (freelook -> cursor)
void	SetMouseCursor (bool updatePos)
{
	if (MouseFreeLook)
	{
		// Get the last cursor
		float x = 0.5f, y = 0.5f;

		// Window size
		uint width = Driver->getWindowWidth();
		uint height = Driver->getWindowHeight();

		// Update the interface pointer
		CInterfaceManager *instance = CInterfaceManager::getInstance();
		if (instance)
		{
			// Get the cursor instance
			CViewPointer *cursor = instance->getPointer();
			if (cursor)
			{
				sint32 ix, iy;
				cursor->getPointerPos (ix, iy);
				x = (float)ix / (float)width;
				y = (float)iy / (float)height;
			}
		}

		MouseFreeLook = false;
		UpdateMouse ();

		// Integer coordinates
		sint ix = (sint)(x*(float)width+0.5f);
		sint iy = (sint)(y*(float)height+0.5f);

		if (updatePos)
		{
			if (MouseDevice)
				MouseDevice->setMousePos((float)ix, (float)iy);
			else
				Driver->setMousePos(x, y);

			if (MouseHardware)
			{
				Driver->setMousePos(x, y);
			}
		}

		// Update the interface pointer
		if (instance)
		{
			// Get the cursor instance
			CViewPointer *cursor = instance->getPointer();
			if (cursor)
			{
				cursor->setPointerPos(ix, iy);
				cursor->setPointerDispPos(ix, iy);
			}
		}
		CInputHandlerManager *IHM = CInputHandlerManager::getInstance ();
		if (IHM)
		{
			IHM->resetPos (ix, iy);
		}

		// Show the cursor
		if (MouseHardware)
		{
			Driver->showCursor(true);
		}
		else
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			if (im)
			{
				CViewPointer *pointer = im->getPointer();
				if (pointer)
					pointer->show (true);
			}
		}
	}
}

//*********************************************************************************
// Use this method to set the cursor speed
void	SetMouseSpeed (float speed)
{
	MouseCursorSpeed = speed;
	UpdateMouse ();
}

//*********************************************************************************
// Use this method to set the cursor acceleration
void	SetMouseAcceleration (uint accel)
{
	MouseCursorAcceleration = accel;
	UpdateMouse ();
}

//*********************************************************************************
void CaptureSystemCursor()
{
	if (IsSystemCursorCaptured()) return;
#ifdef NL_OS_WINDOWS
	HWND drvWnd = Driver->getDisplay();
	if (!drvWnd) return;
	SetCapture(drvWnd);
#else
	// on X11 and cocoa, set driver mouse capture on and store it locally as well
	Driver->setCapture(MouseCapture = true);
#endif
}

//*********************************************************************************
void ReleaseSystemCursor()
{
	if (!IsSystemCursorCaptured()) return;
#ifdef NL_OS_WINDOWS
	// if hardware mouse and not in client area, then force to update its aspect by updating its pos
	if (!IsSystemCursorInClientArea())
	{
		// force update
		ShowCursor(FALSE);
		ShowCursor(TRUE);
	}
	ReleaseCapture();
#else
	// on X11 and cocoa, set driver mouse capture off and store it locally as well
	Driver->setCapture(MouseCapture = false);
#endif
}

//*********************************************************************************
bool IsSystemCursorCaptured()
{
	if (!Driver) return false;
#ifdef NL_OS_WINDOWS
	return GetCapture() == Driver->getDisplay();
#else
	/*
		TODO there should be a way to ask the driver if capturing is on or off
	*/
	return MouseCapture;
#endif
}

//*********************************************************************************
void HandleSystemCursorCapture(const CEvent &event)
{
	if (event == EventMouseDownId)
	{
		CEventMouseDown &em = (CEventMouseDown &) event;
		DownMouseButtons |= em.Button & (leftButton | middleButton | rightButton);
		if (IsSystemCursorInClientArea())
		{
			CaptureSystemCursor();
		}
	}

	if (event == EventMouseUpId)
	{
		// if all buttons up
		CEventMouseDown &em = (CEventMouseDown &) event;
		DownMouseButtons &= ~(em.Button & (leftButton | middleButton | rightButton));
		if (DownMouseButtons == 0)
		{
			ReleaseSystemCursor();
		}
	}

	// if focus was lost then says that all buttons are up from this app viewpoint
	if (event == EventSetFocusId)
	{
		DownMouseButtons = 0;
	}
}


//*********************************************************************************
bool IsSystemCursorInClientArea()
{
	if (!Driver) return false;
#ifdef NL_OS_WINDOWS
	HWND drvWnd = Driver->getDisplay();
	if (!drvWnd) return false;
	UDriver::CMode videoMode;
	Driver->getCurrentScreenMode(videoMode);
	if (!videoMode.Windowed || !IsMouseCursorHardware())
	{
		// just test visibility
		return IsWindowVisible(drvWnd) != FALSE;
	}
	else
	{
		POINT cursPos;
		// the mouse should be in the client area of the window
		if (!GetCursorPos(&cursPos))
		{
			return false;
		}
		HWND wnd = WindowFromPoint(cursPos);
		if (wnd != drvWnd)
		{
			return false; // not the same window
		}
		// want that the mouse be in the client area
		RECT clientRect;
		if (!GetClientRect(drvWnd, &clientRect))
		{
			return false;
		}
		POINT tl, br;
		tl.x = clientRect.left;
		tl.y = clientRect.top;
		br.x = clientRect.right;
		br.y = clientRect.bottom;
		if (!ClientToScreen(drvWnd, &tl))
		{
			return false;
		}
		if (!ClientToScreen(drvWnd, &br))
		{
			return false;
		}
		if (cursPos.x < tl.x ||
			cursPos.x >= br.x ||
			cursPos.y < tl.y ||
			cursPos.y >= br.y)
		{
			return false;
		}
	}
#else
	// TODO for Linux and Mac OS
#endif
	return true;
}



sint CNiceInputAuto::_Count = 0;


CNiceInputAuto::CNiceInputAuto()
{
	if (_Count == 0)
	{

		Driver->enableLowLevelMouse(false, false); // but ignore direct input (win 32 msg only)


		CustomMouse.setCursor("curs_default.tga", CRGBA::White, 0, 0x15, 0x18);
		Driver->showCursor(true); // keep cursor visible in windowed mode
		MouseDevice = NULL;
		Driver->enableLowLevelKeyboard (false);
	}
	++ _Count;
}

CNiceInputAuto::~CNiceInputAuto()
{
	-- _Count;
	nlassert(_Count >= 0);
	if (_Count == 0)
	{
		InitMouseWithCursor (ClientCfg.HardwareCursor);
		Driver->enableLowLevelKeyboard (!ClientCfg.DisableDirectInputKeyboard); // the return value has already been tested at startup
	}
}
