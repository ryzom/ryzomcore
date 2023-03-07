// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "global.h"
#include "actions.h"
#include "input.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/input_handler_manager.h"
#include "client_cfg.h"
#include "time_client.h"
// 3D
#include "nel/3d/u_driver.h"
// Misc
#include "nel/misc/mouse_smoother.h"
#include "nel/misc/system_utils.h"
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
bool							MouseHardware = false;
bool							MouseFreeLook = false;
float							MouseCursorSpeed = 1.f;
uint							MouseCursorAcceleration = 0;
bool							InitMouseFirstTime = true;
bool							SetMousePosFirstTime = true;

// mask for mouse buttons that are known to be down
uint							DownMouseButtons = 0;

//////////////
// FUNCTION //
//////////////

// *********************************************************************************
uint GetMouseButtonsState()
{
	return DownMouseButtons;
}

// *********************************************************************************
// Initialize the mouse
bool	InitMouseWithCursor (bool hardware)
{
	Driver->showCursor(false);

	// Get the new mouse state
	MouseHardware = hardware;
	CViewPointer::setHWMouse( hardware );

	if (InitMouseFirstTime)
	{
		InitMouseFirstTime = false;
	}
	else
	{
		if (!MouseFreeLook)
		{
			// Get the current mouse position
			CInterfaceManager *pIm = CInterfaceManager::getInstance();
			CViewPointer *vp = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
			Driver->showCursor(hardware);
			if (vp)
			{
				float x = (float) vp->getX();
				float y = (float) vp->getY();
				// First, hide the hardware mouse
				uint width = Driver->getWindowWidth();
				uint height = Driver->getWindowHeight();
				if (SetMousePosFirstTime)
				{
					SetMousePosFirstTime = false;
				}
				else if (width != 0 && height != 0)
				{
					nlwarning("Mouse pos %f, %f", x, y);
					Driver->setMousePos(x / width, y / height);
				}
			}
		}
	}

	return true;
}

// *********************************************************************************
// Is mouse cursor hardware ?
bool	IsMouseCursorHardware ()
{
	return MouseHardware;
}

// *********************************************************************************
// Use this method to toggle the mouse (freelook <- cursor)
void	SetMouseFreeLook ()
{
	if (!MouseFreeLook)
	{
		MouseFreeLook = true;
		if (MouseHardware)
		{
			Driver->showCursor(false);
		}
		else
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			if (im)
			{
				CViewPointer *pointer = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
				if (pointer)
					pointer->show (false);
			}
		}
	}
}

// *********************************************************************************
bool IsMouseFreeLook()
{
	return MouseFreeLook;
}

// *********************************************************************************
// Use this method to toggle the mouse (freelook -> cursor)
void	SetMouseCursor (bool updatePos)
{
	if (MouseFreeLook)
	{
		// Get the last cursor
		float x = 0.5f, y = 0.5f;

		// Screen size
		uint width, height;
		CViewRenderer::getInstance()->getScreenSize(width, height);

		// Update the interface pointer
		CInterfaceManager *instance = CInterfaceManager::getInstance();
		if (instance)
		{
			// Get the cursor instance
			CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
			if (cursor)
			{
				sint32 ix, iy;
				cursor->getPointerPos (ix, iy);
				if (ix != CViewPointer::InvalidCoord && iy != CViewPointer::InvalidCoord)
				{
					x = (float)ix / (float)width;
					y = (float)iy / (float)height;
				}
			}
		}

		MouseFreeLook = false;

		// Integer coordinates
		sint ix = (sint)(x*(float)width+0.5f);
		sint iy = (sint)(y*(float)height+0.5f);

		if (updatePos)
		{
			Driver->setMousePos(x, y);
		}

		// Update the interface pointer
		if (instance)
		{
			// Get the cursor instance
			CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
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
				CViewPointer *pointer = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
				if (pointer)
					pointer->show (true);
			}
		}
	}
}

// *********************************************************************************
// Use this method to set the cursor speed
void	SetMouseSpeed (float speed)
{
	MouseCursorSpeed = speed;
}

// *********************************************************************************
// Use this method to set the cursor acceleration
void	SetMouseAcceleration (uint accel)
{
	MouseCursorAcceleration = accel;
}

// *********************************************************************************
void HandleSystemCursorCapture(const CEvent &event)
{
	static bool mouseCaptured = false;

	// capture on first move event after button is held down or free look is activated
	if (event == EventMouseMoveId && !mouseCaptured && (MouseFreeLook || DownMouseButtons != 0))
	{
		mouseCaptured = true;
		Driver->setCapture(true);
	}

	// release when button is released and not in free look
	if (mouseCaptured && !MouseFreeLook && DownMouseButtons == 0)
	{
		mouseCaptured = false;
		Driver->setCapture(false);
	}

	if (event == EventMouseDownId)
	{
		CEventMouseDown &em = (CEventMouseDown &) event;
		DownMouseButtons |= em.Button & (leftButton | middleButton | rightButton);

		CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
		if (cursor)
		{
				cursor->setPointerDown(em.Button == leftButton);
				cursor->setPointerMiddleDown(em.Button == middleButton);
				cursor->setPointerRightDown(em.Button == rightButton);
		}
	}

	if (event == EventMouseUpId)
	{
		// if all buttons up
		CEventMouseDown &em = (CEventMouseDown &) event;
		DownMouseButtons &= ~(em.Button & (leftButton | middleButton | rightButton));
		if (DownMouseButtons == 0)
		{
			CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
			if (cursor)
			{
				cursor->setPointerDown(false);
				cursor->setPointerMiddleDown(false);
				cursor->setPointerRightDown(false);
			}
		}
	}

	// if focus was lost then says that all buttons are up from this app viewpoint
	if (event == EventSetFocusId)
	{
		DownMouseButtons = 0;
	}
}

sint CNiceInputAuto::_Count = 0;


CNiceInputAuto::CNiceInputAuto()
{
	if (_Count == 0)
	{
		Driver->setCursor("curs_default.tga", CRGBA::White, 0, 0x15, 0x18);
		Driver->showCursor(true); // keep cursor visible in windowed mode
	}
	++_Count;
}

CNiceInputAuto::~CNiceInputAuto()
{
	--_Count;
	nlassert(_Count >= 0);
	if (_Count == 0)
	{
		InitMouseWithCursor(ClientCfg.HardwareCursor && !StereoDisplayAttached);
	}
}
