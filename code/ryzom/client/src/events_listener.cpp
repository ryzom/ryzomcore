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



#include "stdpch.h"


#include "events_listener.h"
#include "nel/misc/events.h"
#include "nel/misc/event_server.h"
#include "release.h"
#include "actions.h"
#include "client_cfg.h"
#include "time_client.h"
#include "input.h"
#include "interface_v3/interface_manager.h"
#include "global.h"


using namespace NLMISC;

extern CActionsManager Actions;	// Actions Manager.
extern bool MouseFreeLook;

//---------------------------------------------------
// CEventsListener :
// Constructor
//---------------------------------------------------
CEventsListener::CEventsListener()
{
	_MouseMoveX = false;
	_MouseMoveY = false;
	_MouseAngleX = false;
	_MouseAngleY = false;
	_Wheel = CEventsListener::no_move;

	_MouseX = _OldMouseX = 0.f;
	_MouseY = _OldMouseY = 0.f;
	_MouseButtonPushed = noButton;
	_MouseButtonReleased = noButton;
	_MouseButtonDown = noButton;
	_LastFreeLookUpdateDate = 0;
	//
	_MouseDeltaAX = 0.f;
	_MouseDeltaAY = 0.f;
	_WantedMouseAX = 0.f;
	_WantedMouseAY = 0.f;
	_CurrSmoothedMouseAX = 0.f;
	_CurrSmoothedMouseAY = 0.f;
	_OldSmoothedMouseAX = 0.f;
	_OldSmoothedMouseAY = 0.f;

	_MouseSmoothingOn = false;

	_HandledUIButtonMask = NLMISC::noButton;
}// CEventsListener //

//---------------------------------------------------
// ~CEventsListener :
// Destructor
//---------------------------------------------------
CEventsListener::~CEventsListener()
{
}// ~CEventsListener //

//---------------------------------------------------
// addToServer :
// Add the listener to the server.
//---------------------------------------------------
void CEventsListener::addToServer(CEventServer& server)
{
	server.addListener(EventMouseMoveId,	this);
	server.addListener(EventMouseDownId,	this);
	server.addListener(EventMouseUpId,		this);
	server.addListener(EventMouseWheelId,	this);
	server.addListener(EventKeyDownId,		this);
	server.addListener(EventKeyUpId,		this);
	server.addListener(EventDestroyWindowId,this);
	server.addListener(EventCloseWindowId,	this);
	server.addListener(EventSetFocusId,		this);
	server.addListener(EventDisplayChangeId,	this);
}// addToServer //

//---------------------------------------------------
// removeFromServer :
// Remove the listener to the server.
//---------------------------------------------------
void CEventsListener::removeFromServer (CEventServer& server)
{
	server.removeListener(EventMouseMoveId,	this);
	server.removeListener(EventMouseDownId,	this);
	server.removeListener(EventMouseUpId,	this);
	server.removeListener(EventMouseWheelId,this);
	server.removeListener(EventKeyDownId,	this);
	server.removeListener(EventKeyUpId,		this);
	server.removeListener(EventDestroyWindowId,this);
	server.removeListener(EventCloseWindowId,	this);
	server.removeListener(EventSetFocusId,		this);
}// removeFromServer //

static bool s_MouseFreeLookReady = false;
static sint s_MouseFreeLookLastX;
static sint s_MouseFreeLookLastY;
static sint s_MouseFreeLookFrameX = 0;
static sint s_MouseFreeLookFrameY = 0;
static bool s_MouseFreeLookWaitCenter;

//---------------------------------------------------
// operator() :
// CallBack to manage events from inputs.
//---------------------------------------------------
void CEventsListener::operator()(const CEvent& event)
{
	HandleSystemCursorCapture(event);
	// Window closed.
	if(event == EventDestroyWindowId)
	{
		// Interface saving
		CInterfaceManager::getInstance()->uninitInGame0();

		/* YOYO:
			quitting safely sometimes crash in CContinentMngr::select()
			quitting unsafely crash always in ~CEntityManager()
			Id did not succeed to reproduce the crash in CContinentMngr::select().
			Hence I let it to quit "safely", calling release methods
		*/
		releaseMainLoop(true);
		release();

		// quit properly crash info
		extern void quitCrashReport ();
		quitCrashReport ();

		// kill process
		exit(EXIT_SUCCESS);
	}
	// Window close button pressed.
	if(event == EventCloseWindowId)
	{
		CAHManager::getInstance()->runActionHandler("enter_modal", NULL, "group=ui:interface:quit_dialog");
	}
	// Event from the Mouse (MOVE)
	else if(event == EventMouseMoveId)
	{
		CEventMouseMove* mouseEvent=(CEventMouseMove*)&event;
		if (!MouseFreeLook)
		{
			updateCursorPos(mouseEvent->X, mouseEvent->Y);
			s_MouseFreeLookReady = false;
		}
		else
		{
			// Get in pixel space, centered
			uint32 drW, drH;
			Driver->getWindowSize(drW, drH);
			float fX = mouseEvent->X; // from 0 to 1.0
			float fY = (ClientCfg.FreeLookInverted ? -mouseEvent->Y : mouseEvent->Y);
			sint scX = (sint32)(fX * (float)drW) - ((sint32)drW >> 1); // in pixels, centered
			sint scY = (sint32)(fY * (float)drH) - ((sint32)drH >> 1);
			if (!s_MouseFreeLookReady)
			{
				float pfX = _MouseX;
				float pfY = (ClientCfg.FreeLookInverted ? -_MouseY : _MouseY);
				sint pscX = (sint32)(pfX * (float)drW) - ((sint32)drW >> 1); // in pixels, centered
				sint pscY = (sint32)(pfY * (float)drH) - ((sint32)drH >> 1);
				s_MouseFreeLookReady = true;
				s_MouseFreeLookLastX = pscX;
				s_MouseFreeLookLastY = pscY;
				s_MouseFreeLookWaitCenter = false;
			}

			// NOTE: No 0, 0 center mouse message in Windows (lower mouse message rate), but safe to assume any movement messages are requeued relative to our new position
			// In case free look bugs on other platform, we may need to push in our own message on setMousePos for Windows
			if (s_MouseFreeLookWaitCenter) // scX == 0 && scY == 0)
			{
				// Centered, set last to 0
				s_MouseFreeLookLastX = 0;
				s_MouseFreeLookLastY = 0;
				s_MouseFreeLookWaitCenter = false;
			}

			// Get delta since last center
			sint scXd = scX - s_MouseFreeLookLastX;
			sint scYd = scY - s_MouseFreeLookLastY;
			s_MouseFreeLookLastX = scX;
			s_MouseFreeLookLastY = scY;

			s_MouseFreeLookFrameX += scXd;
			s_MouseFreeLookFrameY += scYd;
			// updateFreeLookPos is called in updateMouseSmoothing per frame

			// Center cursor
			bool outsideBounds = ((abs(scX) > (drW >> 3)) || (abs(scY) > (drH >> 3)));
			if (outsideBounds)
			{
				s_MouseFreeLookWaitCenter = true;
				Driver->setMousePos(0.5f, 0.5f);
			}
		}
	}
	// Event from the Mouse (DOWN BUTTONS)
	else if(event == EventMouseDownId)
	{
		CEventMouseDown* mousebutton=(CEventMouseDown*)&event;
		TMouseButton mouseButton = (TMouseButton)((mousebutton->Button) & (leftButton | middleButton| rightButton));
		_MouseButtonPushed = (TMouseButton)(_MouseButtonPushed|mouseButton);
		_MouseButtonDown = (TMouseButton)(_MouseButtonDown|mouseButton);

	}
	// Event from the Mouse (UP BUTTONS)
	else if(event == EventMouseUpId)
	{
		CEventMouseUp* mousebutton=(CEventMouseUp*)&event;
		TMouseButton mouseButton = (TMouseButton)((mousebutton->Button) & (leftButton | middleButton| rightButton));
		_MouseButtonReleased = (TMouseButton)(_MouseButtonReleased|mouseButton);
		_MouseButtonDown = (TMouseButton)(_MouseButtonDown&~mouseButton);
	}
	// Event from the Mouse (WHEEL)
	else if(event == EventMouseWheelId)
	{
		CEventMouseWheel* mouseWheel = (CEventMouseWheel*)&event;
		if(mouseWheel->Direction)
			_Wheel = foreward;
		else
			_Wheel = backward;
	}

	// if some events were handled by the ui, avoid that they be handled twice
	_MouseButtonReleased = (NLMISC::TMouseButton) (_MouseButtonReleased & ~_HandledUIButtonMask);
	_MouseButtonPushed = (NLMISC::TMouseButton) (_MouseButtonPushed & ~_HandledUIButtonMask);

	_HandledUIButtonMask = NLMISC::noButton;

	if (event==EventSetFocusId)
	{
		CEventSetFocus *pEvent=(CEventSetFocus *)&event;
		if (!pEvent->Get)
		{
			// Disactive all keys
			_MouseButtonPushed = noButton;
			_MouseButtonReleased = noButton;
			_MouseButtonDown = noButton;
		}
	}



}// operator() //




// ***********************************************************************
void CEventsListener::smoothMouseCoordinates(float &x, float &y, float smoothingPeriod)
{
	if (smoothingPeriod > 0.001f)
	{
		_MouseSmoother.setSamplingPeriod(smoothingPeriod);
		CVector2f smoothedPos = _MouseSmoother.samplePos(CVector2f(x, y), TimeInSec);
		x = smoothedPos.x;
		y = smoothedPos.y;
	}
}


// *************************************************************
void CEventsListener::updateMouseSmoothing()
{
	if (_LastFreeLookUpdateDate != TimeInSec)
	{
		updateFreeLookPos((float)s_MouseFreeLookFrameX, (float)s_MouseFreeLookFrameY);
		s_MouseFreeLookFrameX = 0;
		s_MouseFreeLookFrameY = 0;
	}
}

// ***************************************************************
void CEventsListener::enableMouseSmoothing(bool on)
{
	if (on == _MouseSmoothingOn) return;
	_MouseSmoothingOn = on;
	if (_MouseSmoothingOn)
	{
		_WantedMouseAX = 0.f;
		_WantedMouseAY = 0.f;
		_CurrSmoothedMouseAX = 0.f;
		_CurrSmoothedMouseAY = 0.f;
		_MouseDeltaAX = 0.f;
		_MouseDeltaAY = 0.f;
		_MouseSmoother.reset();
	}
}


// ***************************************************************
void CEventsListener::updateFreeLookPos(float x, float y)
{
	if (ClientCfg.FreeLookSmoothingPeriod == 0 || !_MouseSmoothingOn)
	{
		_MouseDeltaAX = x * ClientCfg.FreeLookSpeed;
		_MouseDeltaAY = y * ClientCfg.FreeLookSpeed;
		_FreeLookSmoothed = false;
	}
	else
	{
		if (!_FreeLookSmoothed)
		{
			_WantedMouseAX = 0.f;
			_WantedMouseAY = 0.f;
			_CurrSmoothedMouseAX = 0.f;
			_CurrSmoothedMouseAY = 0.f;
			_FreeLookSmoothed = true;
		}
		//
		_WantedMouseAX += x * ClientCfg.FreeLookSpeed;
		_WantedMouseAY += y * ClientCfg.FreeLookSpeed;
		if (_MouseSmoother.isReseted())
		{
			// first frame case
			_CurrSmoothedMouseAX = _WantedMouseAX;
			_CurrSmoothedMouseAY = _WantedMouseAY;
		}
		_OldSmoothedMouseAX = _CurrSmoothedMouseAX;
		_OldSmoothedMouseAY = _CurrSmoothedMouseAY;
		_CurrSmoothedMouseAX = _WantedMouseAX;
		_CurrSmoothedMouseAY = _WantedMouseAY;

		smoothMouseCoordinates(_CurrSmoothedMouseAX, _CurrSmoothedMouseAY, ClientCfg.FreeLookSmoothingPeriod);

		_MouseDeltaAX = _CurrSmoothedMouseAX - _OldSmoothedMouseAX;
		_MouseDeltaAY = _CurrSmoothedMouseAY - _OldSmoothedMouseAY;

	}

	_LastFreeLookUpdateDate = TimeInSec;

	const float epsilon = 10E-6f;

	// If mouse pos X changed.
	if (fabs(_MouseDeltaAX) > epsilon)
	{
		_MouseAngleX = true;
	}
	else
	{
		_MouseAngleX = false;
	}

	// If mouse pos Y changed.
	if (fabs(_MouseDeltaAY) > epsilon)
	{
		_MouseAngleY = true;
	}
	else
	{
		_MouseAngleY = false;
	}
}

// ***************************************************************
void CEventsListener::updateCursorPos(float x, float y)
{
	// Backup mouse
	_OldMouseX = _MouseX;
	_OldMouseY = _MouseY;

	// New mouse pos
	_MouseX = x;
	_MouseY = y;

	// If Mouse pos X changed.
	if(_MouseX != _OldMouseX)
		_MouseMoveX = true;
	else
		_MouseMoveX = false;

	// If Mouse pos Y changed.
	if(_MouseY != _OldMouseY)
		_MouseMoveY = true;
	else
		_MouseMoveY = false;
}

//---------------------------------------------------
// getWheelState :
// As soon as we call the function, _Wheel will change to noWheel.
//---------------------------------------------------
CEventsListener::TWheelState CEventsListener::getWheelState()
{
	TWheelState wheelState = _Wheel;
	_Wheel = no_move;
	return wheelState;
}// getWheelState //


//---------------------------------------------------
// isMouseMoveX :
// As soon as we call the function, _MouseMoveX will change to noWheel.
//---------------------------------------------------
bool CEventsListener::isMouseMoveX()
{
	bool mouseMove = _MouseMoveX;
	_MouseMoveX = false;
	return mouseMove;
}// getMouseMove //


//---------------------------------------------------
// isMouseMoveY :
// As soon as we call the function, _MouseMoveY will change to noWheel.
//---------------------------------------------------
bool CEventsListener::isMouseMoveY()
{
	bool mouseMove = _MouseMoveY;
	_MouseMoveY = false;
	return mouseMove;
}// getMouseMove //

//---------------------------------------------------
// isMouseAngleX :
// As soon as we call the function, _MouseAngleX will change to noWheel.
//---------------------------------------------------
bool CEventsListener::isMouseAngleX()
{
	bool mouseAngle = _MouseAngleX;
	_MouseAngleX = false;
	return mouseAngle;
}// getMouseAngle //


//---------------------------------------------------
// isMouseAngleY :
// As soon as we call the function, _MouseAngleY will change to noWheel.
//---------------------------------------------------
bool CEventsListener::isMouseAngleY()
{
	bool mouseAngle = _MouseAngleY;
	_MouseAngleY = false;
	return mouseAngle;
}// getMouseAngle //

//---------------------------------------------------
// return a float the mousePosX.
//---------------------------------------------------
float CEventsListener::getMousePosX()
{
	return _MouseX;
}// getMouseMoveX //

//---------------------------------------------------
// return a float the mousePosY.
//---------------------------------------------------
float CEventsListener::getMousePosY()
{
	return _MouseY;
}// getMouseMoveY //

//---------------------------------------------------
// return a float the mousePosX.
//---------------------------------------------------
float CEventsListener::getMouseAngleX()
{
	return _MouseDeltaAX;
}// getMouseAngleX //

//---------------------------------------------------
// return a float the mousePosY.
//---------------------------------------------------
float CEventsListener::getMouseAngleY()
{
	return _MouseDeltaAY;
}// getMouseAngleY //

// ***************************************************************************

bool CEventsListener::isMouseButtonPushed (NLMISC::TMouseButton button) const
{
	return (_MouseButtonPushed & button) == button;
}

// ***************************************************************************

bool CEventsListener::isMouseButtonReleased (NLMISC::TMouseButton button) const
{
	return (_MouseButtonReleased & button) == button;
}

// ***************************************************************************
void CEventsListener::setUIHandledButtonMask(NLMISC::TMouseButton buttons)
{
	_HandledUIButtonMask = buttons;
}

// ***************************************************************************
void CEventsListener::addUIHandledButtonMask(NLMISC::TMouseButton mask)
{
	_HandledUIButtonMask = (NLMISC::TMouseButton) (_HandledUIButtonMask | mask);
}

// ***************************************************************************

bool CEventsListener::isMouseButtonDown (NLMISC::TMouseButton button) const
{
	return (_MouseButtonDown & button) == button;
}

// ***************************************************************************

bool CEventsListener::isMouseButtonUp (NLMISC::TMouseButton button) const
{
	return ((_MouseButtonDown & button) == 0);
}

// ***************************************************************************

void CEventsListener::update ()
{
	_MouseButtonPushed = noButton;
	_MouseButtonReleased = noButton;
}

// ***************************************************************************
