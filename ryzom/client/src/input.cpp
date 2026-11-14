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
bool							InitMouseFirstTime = true;
bool							SetMousePosFirstTime = true;

// mask for mouse buttons that are known to be down
uint							DownMouseButtons = 0;

// track whether the mouse is currently captured
static bool						MouseCaptured = false;

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
	bool cursorKindChanged = hardware != MouseHardware;
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
				bool cursorMoved = false;
				if (SetMousePosFirstTime)
				{
					SetMousePosFirstTime = false;
				}
				else if ((cursorKindChanged || !hardware) && width != 0 && height != 0)
				{
					// the cursor of the other kind appears where the pointer of the interface is, and the
					// hidden system cursor of a software cursor stays under the pointer that is drawn. A
					// hardware cursor that was one already stays where the player has it: after a loading
					// screen the pointer is the stale one (it saw no mouse move)
					Driver->setMousePos(x / width, y / height);
					cursorMoved = true;
				}
				if (!cursorMoved)
				{
					// the next mouse move brings the pointer to the cursor, and so does a click before it
					CInputHandlerManager::getInstance()->recoverMousePos();
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
// Use this method to put the mouse in cursor mode whatever mode it is in
void ResetMouseCursor()
{
	// a hardware cursor stays where the player has it, unless a free look had taken it. The hidden system
	// cursor of a software cursor goes under the pointer that is drawn
	bool moveCursor = MouseFreeLook || !MouseHardware;
	SetMouseFreeLook();
	SetMouseCursor(moveCursor);
	if (!moveCursor)
	{
		// the next mouse move brings the pointer to the cursor, and so does a click before it
		CInputHandlerManager::getInstance()->recoverMousePos();
	}
}

// *********************************************************************************
void ResetMouseCaptureState()
{
	DownMouseButtons = 0;
	if (MouseCaptured)
	{
		MouseCaptured = false;
		Driver->setCapture(false);
	}
}

// *********************************************************************************
void HandleSystemCursorCapture(const CEvent &event)
{
	// capture on first move event after button is held down or free look is activated
	if (event == EventMouseMoveId && !MouseCaptured && (MouseFreeLook || DownMouseButtons != 0))
	{
		MouseCaptured = true;
		Driver->setCapture(true);
	}

	// release when button is released and not in free look
	if (MouseCaptured && !MouseFreeLook && DownMouseButtons == 0)
	{
		MouseCaptured = false;
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
