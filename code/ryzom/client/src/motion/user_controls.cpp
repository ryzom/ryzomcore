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


//////////////
// Includes //
//////////////
// GAME SHARE
#include "game_share/bot_chat_types.h"
// Client.
#include "user_controls.h"
#include "../actions_client.h"
#include "../user_entity.h"
#include "../cursor_functions.h"
#include "../time_client.h"
#include "../interface_v3/interface_manager.h"
#include "../entities.h"
#include "../view.h"
#include "../input.h"
//
#include "../r2/editor.h"


///////////
// Using //
///////////
using namespace std;
using namespace NLMISC;
using namespace NL3D;


/////////////
// Externs //
/////////////
extern UDriver				*Driver;
extern UScene				*Scene;
extern CEventsListener		EventsListener;				// Inputs Manager

/////////////
// Globals //
/////////////
// User Controls (mouse, keyboard, interfaces, ...)
CUserControls UserControls;

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_User_Controls_Update )


// ----------------------------------------------------------------------------
static bool isSwimming()
{
	if (UserEntity != NULL)
		return (UserEntity->mode() == MBEHAV::SWIM || UserEntity->mode() == MBEHAV::MOUNT_SWIM);
	else
		return false;
}

static bool isSit()
{
	if (UserEntity)
		return UserEntity->isSit();
	else
		return false;
}

static bool isRiding()
{
	if (UserEntity)
		return UserEntity->isRiding();
	else
		return false;
}


///////////////
// Functions //
///////////////
//---------------------------------------------------
// CUserControls :
// Constructor.
//---------------------------------------------------
CUserControls::CUserControls()
{
	init();
}// CUserControls //

//---------------------------------------------------
// init :
// Initialize all the components.
//---------------------------------------------------
void CUserControls::init()
{
	// Default Mode is Interface Mode.
	_LastMode = _Mode	= InterfaceMode;

	_LeftClickStart		= 0;
	_LeftClickEnd		= 0;
	_RightClickStart	= 0;
	_RightClickEnd		= 0;
	_TimeBeforeMouseSlide	= 50;
	_TimeLongClick			= 200;

	_TransSpeed			= 1.f;

	_UpdateView			= false;
	_ZOscil				= 0;
	_Acc				= 0;
	_T0					= 0;
	_T					= 0;
	_V0					= _TransSpeed;

	_T0View				= 0;
	_TView				= 0;

	_TurnBack			= false;
	_ClickMove			= false;

	_Start				= CVector(0,0,0);
	_Destination		= _Start;

	_Dist				= -1;

	_DirectionMove		= none;

	_Locked				= false;	// Move is not locked.

	_FreeLook			= false;
	_InternalView		= true;

	_LastFrameBackward	= false;
	_LastFrameForward	= false;
	_LastFrameAutowalk	= false;
	_LastFrameStrafeLeft	= false;
	_LastFrameStrafeRight	= false;
	_LastFrameTurnLeft		= false;
	_LastFrameTurnRight		= false;
	_LastFrameLeftButtonDown= false;
	_LastFrameMousePosX	= 0.f;
	_LastFrameMousePosY	= 0.f;


	_CameraAuto			= false;

	_UserCameraDeltaYaw	= 0;
	_ResetSmoothCameraDeltaYaw= ResetCDYOff;

	_CurrentFrameFreeLookCalled= false;

	_RotateUserLRVelocity= 0.f;
	_RotateUserUDVelocity= 0.f;
	_RotateCameraLRVelocity= 0.f;

	_MouseCaptured = false;

	_NeedReleaseForward = false;
	_NextForwardCancelMoveTo = false;

}// init //


//-----------------------------------------------
// needReleaseForward :
//-----------------------------------------------
void CUserControls::needReleaseForward()
{
	if( Actions.valide("forward") )
	{
		_NeedReleaseForward = true;
	}
}


//-----------------------------------------------
// autowalkState :
//-----------------------------------------------
void CUserControls::autowalkState(bool enable)
{
	if(enable)
	{
		_DirectionMove |= autowalk;
		// If not backward, default is forward.
		if(!(_DirectionMove & backward))
			_DirectionMove |= forward;
	}
	else
		_DirectionMove &= ~(autowalk|forward|backward);
}// autowalkState //


//-----------------------------------------------
// update :
// Main function of the Class -> Call it each frame.
//-----------------------------------------------
void CUserControls::update()
{
	H_AUTO_USE ( RZ_Client_User_Controls_Update );

	View.update();

	// Reset the moving direction.
	_DirectionMove &= ~(left | right);

	// if not locked, move
	if(!_Locked)
	{
		// What is the current _Mode ?
		switch(_Mode)
		{
		// Interface Mode.
		case InterfaceMode:
			interfaceMode();
			break;

		// AI Mode
		case AIMode:
			aiMode();
			break;

		// Death Mode
		case DeathMode:
			deathMode();
			break;

		// Mount Mode
		case MountMode:
			mountMode();
			break;

		// Third Person View Mode
		case ThirdMode:
			thirdMode();
			break;

		default:
			break;
		}

		_LastFrameForward  = Actions.valide("forward") && !_NeedReleaseForward;
		if( UserEntity->mode() != MBEHAV::MOUNT_SWIM )
		{
			_LastFrameBackward = Actions.valide("backward");
		}
		_LastFrameAutowalk = Actions.valide("toggle_auto_walk");
		if( isSit() || UserEntity->isAFK() )
		{
			if( _LastFrameForward || _LastFrameBackward )
			{
				UserEntity->sit(false);
				UserEntity->setAFK(false);
			}
		}
		if (isSwimming() || isRiding())
		{
			_LastFrameStrafeLeft	= false;
			_LastFrameStrafeRight	= false;
		}
		else
		{
			_LastFrameStrafeLeft	= Actions.valide("strafe_left");
			_LastFrameStrafeRight	= Actions.valide("strafe_right");
		}
		_LastFrameTurnLeft		= Actions.valide("turn_left");
		_LastFrameTurnRight		= Actions.valide("turn_right");
		_LastFrameLeftButtonDown= EventsListener.isMouseButtonDown(leftButton);
		_LastFrameMousePosX	= EventsListener.getMousePosX();
		_LastFrameMousePosY	= EventsListener.getMousePosY();
	}

	// update the refinePos (for landscape refining...)
	// In fly mode, take the view pos.
	if(_Mode == AIMode)
		View.refinePos(View.viewPos());
	// Else take the entity pos (for less update if 3rd personn)
	else
		View.refinePos(UserEntity->pos());

	// update camera collision once per frame
	View.updateCameraCollision();

	NLMISC::CCDBNodeLeaf *node = _UiVarMkMoveDB ? &*_UiVarMkMoveDB
		: &*(_UiVarMkMoveDB = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MK_MOVE"));
	node->setValue32(autowalkState());
}// update //


//-----------------------------------------------
// Acceleration helper
//-----------------------------------------------
void CUserControls::updateVelocity (float deltaTime, float acceleration, float brake, float speedMax, float &speed)
{
	speed += acceleration * deltaTime;
	// No acclereration, brake
	if (acceleration == 0)
	{
		brake *= deltaTime;
		if (speed < 0)
		{
			if (speed > -brake)
				speed = 0;
			else
				speed += brake;
		}
		else
		{
			if (speed < brake)
				speed = 0;
			else
				speed -= brake;
		}
	}
	// Clamp the speed
	clamp (speed, -speedMax, speedMax);
}// updateVelocity //

//-----------------------------------------------
// keyboardRotation :
// Manage keyboard rotation
//-----------------------------------------------
void CUserControls::keyboardRotationLR (bool left, bool right)
{
	if (left || right)
	{
		float accel = 0;
		if (left)
		{
			accel += ClientCfg.RotAccel;
			_RotateUserLRVelocity = std::max (_RotateUserLRVelocity, ClientCfg.RotKeySpeedMin);
		}

		if (right)
		{
			accel -= ClientCfg.RotAccel;
			_RotateUserLRVelocity = std::min (_RotateUserLRVelocity, -ClientCfg.RotKeySpeedMin);
		}
		updateVelocity (DT, accel, ClientCfg.RotAccel, ClientCfg.RotKeySpeedMax, _RotateUserLRVelocity);

		// Rotate the body.
		UserEntity->rotate(DT*_RotateUserLRVelocity);
	}
	else
		_RotateUserLRVelocity = 0;
}// keyboardRotation //

//-----------------------------------------------
// keyboardRotationUD :
//-----------------------------------------------
void CUserControls::keyboardRotationUD (bool up, bool down)
{
	if (up || down)
	{
		float accel = 0;
		if (up)
		{
			accel += ClientCfg.RotAccel;
			_RotateUserUDVelocity = std::max (_RotateUserUDVelocity, ClientCfg.RotKeySpeedMin);
		}

		if (down)
		{
			accel -= ClientCfg.RotAccel;
			_RotateUserUDVelocity = std::min (_RotateUserUDVelocity, -ClientCfg.RotKeySpeedMin);
		}
		updateVelocity (DT, accel, ClientCfg.RotAccel, ClientCfg.RotKeySpeedMax, _RotateUserUDVelocity);

		UserEntity->rotHeadVertically(DT*_RotateUserUDVelocity);
		UserEntity->stopForceHeadPitchInFollow();
	}
	else
		_RotateUserUDVelocity = 0;
}// keyboardRotationUD  //

//-----------------------------------------------
// keyboardRotationCameraLR :
//-----------------------------------------------
void CUserControls::keyboardRotationCameraLR (bool left, bool right)
{
	if (left || right)
	{
		float accel = 0;
		if (left)
		{
			accel += ClientCfg.RotAccel;
			_RotateCameraLRVelocity = std::max (_RotateCameraLRVelocity, ClientCfg.RotKeySpeedMin);
		}

		if (right)
		{
			accel -= ClientCfg.RotAccel;
			_RotateCameraLRVelocity = std::min (_RotateCameraLRVelocity, -ClientCfg.RotKeySpeedMin);
		}
		updateVelocity (DT, accel, ClientCfg.RotAccel, ClientCfg.RotKeySpeedMax, _RotateCameraLRVelocity);

		// Rotate the camera
		appendCameraDeltaYaw(DT*_RotateCameraLRVelocity);
	}
	else
		_RotateCameraLRVelocity = 0;
}


//-----------------------------------------------
// getMouseAngleMove
//-----------------------------------------------
void CUserControls::getMouseAngleMove(float &dx, float &dy)
{
	dx = 0.0f;
	dy = 0.0f;

	// The mouse may still "StandardMove" ie through a CEventMouseMove
	// This can happens cause DirectInputDisabled, or because of the 
	// "Rotation Anti-Lag system" which start to rotate before the mouse is hid 
	// and message mode passed to RawMode
	// 
	// On X11 and Cocoa, there is no MouseDevice, do it without.

	extern IMouseDevice				*MouseDevice;

	// if the mouse position changed
	if( EventsListener.getMousePosX() != _LastFrameMousePosX ||
		EventsListener.getMousePosY() != _LastFrameMousePosY )
	{
		// get the mouse movement delta
		float	dmpx= EventsListener.getMousePosX() - _LastFrameMousePosX;
		float	dmpy= EventsListener.getMousePosY() - _LastFrameMousePosY;

		// simulate mickeys mode if there is a mouse device
		if (MouseDevice)
			MouseDevice->convertStdMouseMoveInMickeys(dmpx, dmpy);
		else
		{
			dmpx *= (float)Driver->getWindowWidth();
			dmpy *= (float)Driver->getWindowHeight();
		}

		// handle inverted mouse, if enabled
		if(ClientCfg.FreeLookInverted) dmpy = -dmpy;
		
		// update free look
		EventsListener.updateFreeLookPos(dmpx, dmpy);
	}

	// If the mouse move on the axis X, with a CGDMouseMove
	if(EventsListener.isMouseAngleX())
		dx = -EventsListener.getMouseAngleX ();

	// If the mouse move on the axis Y, with a CGDMouseMove
	if(EventsListener.isMouseAngleY())
		dy = EventsListener.getMouseAngleY ();
}


//-----------------------------------------------
// freeLook :
// Manage a free look.
//-----------------------------------------------
void CUserControls::freeLook(bool fullMode)
{
	static bool leftButtonDownInFullMode = false;

	EventsListener.enableMouseSmoothing(true);
	// **** Mouse rotation => apply to user
	float	dx,dy;
	getMouseAngleMove(dx,dy);

	// apply
	if(dx != 0.f)
	{
		if( UserEntity->canChangeFront() )
		{
			UserEntity->rotate(dx);
		}
		else
		{
			appendCameraDeltaYaw(dx);
		}
	}
	if(dy != 0.f)
	{
		UserEntity->rotHeadVertically(dy);
		UserEntity->stopForceHeadPitchInFollow();
	}


	// **** Only in fullMode (longClick for instance)
	if(fullMode)
	{
		_CurrentFrameFreeLookCalled= true;

		CInterfaceManager *IM = CInterfaceManager::getInstance ();

		// FREE LOOK : Hide the cursor
		// disable interface mouse handling.
		CWidgetManager::getInstance()->enableMouseHandling(false);
		// Get the cursor instance
		CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
		if(cursor)
		{
			// Hide the Cursor.
			SetMouseFreeLook();
		}

		// **** special actions keys while in free look
		if (isSwimming() || isRiding())
			keyboardRotationLR (false, false);
		else
			keyboardRotationLR (Actions.valide("strafe_left"), Actions.valide("strafe_right"));
		// Strafe Left
		if(Actions.valide("turn_left"))
			_DirectionMove |= left;
		// Strafe Right
		if(Actions.valide("turn_right"))
			_DirectionMove |= right;

		// **** Left mouse click while in freelook: forward
		// NB: in commonMove() the case "rightButtonDown+leftButtonDown" is managed.
		// but still important to test it here, in case of FreeLook mode (no right button down needed)
		if(EventsListener.isMouseButtonDown (leftButton))
		{
			// Forward.
			_DirectionMove |= forward;
			_DirectionMove &= ~backward;
			// Quit autowalk if first frame in this mode
			if( !leftButtonDownInFullMode )
				_DirectionMove &= ~autowalk;

			// Moving Break any Follow Mode
			UserEntity->disableFollow();
			UserEntity->moveTo(CLFECOMMON::INVALID_SLOT, 0.0, CUserEntity::None);

			leftButtonDownInFullMode = true;
		}
		else
		{
			leftButtonDownInFullMode = false;
		}

		if( UserEntity->canChangeFront() )
		{
			// **** reset delta camera
			applyCameraDeltaYawToUser();
		}
	}

}// freeLook //


//-----------------------------------------------
// cameraLook :
// Manage the camera look.
//-----------------------------------------------
void CUserControls::cameraLook(bool fullMode)
{
	EventsListener.enableMouseSmoothing(true);
	// **** Mouse rotation => apply X to camera, Y to user
	float	dx,dy;
	getMouseAngleMove(dx,dy);

	// apply
	if(dx != 0.f)
		appendCameraDeltaYaw(dx);
	// If the mouse move on the axis Y, cahnge the actual user pitch (not the delta camera)
	if(dy != 0.f)
	{
		UserEntity->rotHeadVertically(dy);
		UserEntity->stopForceHeadPitchInFollow();
	}


	// **** Only in fullMode (longClick for instance)
	if(fullMode)
	{
		CInterfaceManager *IM = CInterfaceManager::getInstance ();

		// CAMERA LOOK : Hide the cursor
		// disable interface mouse handling.
		CWidgetManager::getInstance()->enableMouseHandling(false);
		// Get the cursor instance
		CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
		if(cursor)
		{
			// Hide the Cursor.
			SetMouseFreeLook();
		}
	}


}// cameraLook //


//-----------------------------------------------
// commonMove :
// Manage some common actions.
// \todo GUIGUI : manage control before entities update, but view only after move
//-----------------------------------------------
void CUserControls::commonMove()
{
	CInterfaceManager *IM = CInterfaceManager::getInstance ();

	/////////////////
	// RESET USER CAMERA YAW
	testApplyCameraYawToUser();

	/////////////////
	// MOUSE WHEEL //
	CEventsListener::TWheelState wheelState = EventsListener.getWheelState();	// Done all the time, to reset the state
	View.changeCameraDist((wheelState == CEventsListener::foreward), (wheelState == CEventsListener::backward));
	View.changeCameraDist(Actions.valide("camera_foreward"), Actions.valide("camera_backward"));
	// Camera Up/Down.
	View.changeCameraHeight(Actions.valide("camera_up"), Actions.valide("camera_down"));
	//////////////////
	// LOOK UP/DOWN //
	keyboardRotationUD(Actions.valide("look_up"), Actions.valide("look_down"));
	/////////////
	// FORWARD //
	// ON
	if(Actions.valide("forward") && !_NeedReleaseForward)
	{
		// Forward
		_DirectionMove |= forward;
		_DirectionMove &= ~backward;

		// Start Action
		if(!_LastFrameForward)
		{
			// Start autowalk mode.
			if(Actions.valide("toggle_auto_walk"))
				_DirectionMove |= autowalk;
			// Leave autowalk mode.
			else
				_DirectionMove &= ~autowalk;
		}

		if( _NextForwardCancelMoveTo )
		{
			_NextForwardCancelMoveTo = false;
			UserEntity->moveTo(CLFECOMMON::INVALID_SLOT, 0.0, CUserEntity::None);
			applyCameraDeltaYawToUser();
		}
	}
	// OFF
	else
	{
		if(_LastFrameForward)
			if(!(_DirectionMove & autowalk))
				_DirectionMove &= ~forward;

		// since forward is released, forward is now enable again
		if( !Actions.valide("forward") && _NeedReleaseForward)
		{
			_NeedReleaseForward = false;
			_NextForwardCancelMoveTo = true;
		}
	}
	//////////////
	// BACKWARD //
	// ON
	if(Actions.valide("backward"))
	{
		if( UserEntity->mode()!=MBEHAV::MOUNT_SWIM )
		{
			// Backward
			_DirectionMove |= backward;
			_DirectionMove &= ~forward;
		}
		// Start Action
		if(!_LastFrameBackward)
		{
			// Start autowalk mode.
			if(Actions.valide("toggle_auto_walk"))
				_DirectionMove |= autowalk;
			// Leave autowalk mode.
			else
				_DirectionMove &= ~autowalk;
		}
	}
	// OFF
	else
	{
		if(_LastFrameBackward)
			if(!(_DirectionMove & autowalk))
				_DirectionMove &= ~backward;
	}

	// turn left or right will break the move-to, so forward is enable again here
	if(Actions.valide("turn_left") || Actions.valide("turn_right"))
	{
		if( _NeedReleaseForward )
		{
			_NeedReleaseForward = false;
			_NextForwardCancelMoveTo = false;
			// if free cam mode reset the view because move if target turned around camera may be
			if( !ClientCfg.AutomaticCamera )
			{
				resetSmoothCameraDeltaYaw();
			}
		}
	}

	//////////////
	// AUTOWALK //
	// Must be after forward and backward to know the autowalk direction
	// ON
	if(Actions.valide("toggle_auto_walk"))
	{
		// Action start
		if(!_LastFrameAutowalk)
		{
			// Leave autowalk if forward and backward not pushed and only if autowalk just pushed.
			if((_DirectionMove & autowalk) && !Actions.valide("forward") && !Actions.valide("backward"))
				_DirectionMove &= ~(autowalk|forward|backward);
			else
			{
				// Start Autowalk
				_DirectionMove |= autowalk;
				// If not backward, default is forward.
				if(!(_DirectionMove & backward))
					_DirectionMove |= forward;
			}
		}
	}


	//////////////
	// MOUSE MANAGEMENT
	//////////////

	// Left Click and Dbl Click
	bool dblClickLeft = false;
	if(EventsListener.isMouseButtonPushed(leftButton))
	{
		_LeftClickStart = T1;
	}
	if( EventsListener.isMouseButtonReleased (leftButton) )
	{
		if(T1 <= _LeftClickEnd + CWidgetManager::getInstance()->getUserDblClickDelay())
		{
			dblClickLeft = true;
		}
		_LeftClickEnd = T1;
	}

	// Right Click and Dbl Click
	bool dblClickRight = false;
	if(EventsListener.isMouseButtonPushed(rightButton))
	{
		_RightClickStart = T1;
	}
	if( EventsListener.isMouseButtonReleased (rightButton) )
	{
		if(T1 <= _RightClickEnd + CWidgetManager::getInstance()->getUserDblClickDelay())
		{
			dblClickRight = true;
		}
		_RightClickEnd = T1;
	}

	_CurrentFrameFreeLookCalled= false;
	if(_FreeLook)
	{
		if(EventsListener.isMouseButtonReleased (rightButton))
		{
			// stop FreeLook (and restore mouse/cursor)
			stopFreeLook();
		}
		else
		{
			// if we are in follow mode, do a cameraLook instead
			if(UserEntity->follow() || UserEntity->moveTo())
				cameraLook(true);
			else
				freeLook(true);
		}
	}
	else
	{
		if (!_MouseCaptured)
		{
			// if rightbutton release while left MouseButton UP
			if ( EventsListener.isMouseButtonReleased (rightButton) && EventsListener.isMouseButtonUp (leftButton))
			{
				// Cursor mode.
				SetMouseCursor ();

				// Short Right Click -> Check Action
				if((T1-_RightClickStart) <= _TimeLongClick)
				{
					if(ClientCfg.SelectWithRClick || R2::isEditionCurrent())
					{
						// nb : the ring editor also need that kind of events
						execActionCursorPos(true,dblClickRight);
					}

					// Launch Context Menu
					if (R2::getEditor().getMode() != R2::CEditor::EditionMode)
					{
						IM->launchContextMenuInGame("ui:interface:game_context_menu");
					}
				}


				// Give back the mouse handling to the interface.
				CWidgetManager::getInstance()->enableMouseHandling(true);
				EventsListener.enableMouseSmoothing(false);
			}

			// if leftbutton release while right MouseButton UP
			if ( EventsListener.isMouseButtonReleased (leftButton) && EventsListener.isMouseButtonUp (rightButton))
			{
				// Cursor mode.
				SetMouseCursor ();

				// Short Left Click -> Check Action
				if((T1-_LeftClickStart) <= _TimeLongClick)
				{
					execActionCursorPos(false, dblClickLeft);
				}

				// Give back the mouse handling to the interface.
				CWidgetManager::getInstance()->enableMouseHandling(true);
				EventsListener.enableMouseSmoothing(false);
			}


			// if Right button down
			if (EventsListener.isMouseButtonDown (rightButton))
			{
				// NB: start the freeLook sooner than a longclick
				if((T1-_RightClickStart) > _TimeBeforeMouseSlide)
				{
					// if we are in follow mode, do a cameraLook on RightSlide, else do a freeLook
					if(UserEntity->follow() || UserEntity->moveTo() || UserEntity->isSit() || UserEntity->isAFK())
						// FullMode (hide mouse...) only if longClick
						cameraLook(T1-_RightClickStart > _TimeLongClick);
					else
					{
						// FullMode (hide mouse...) only if longClick
						freeLook(T1-_RightClickStart > _TimeLongClick);

						if( _NeedReleaseForward )
						{
							_NeedReleaseForward = false;
							_NextForwardCancelMoveTo = false;
							// if free cam mode reset the view because move if target turned around camera may be
							if( !ClientCfg.AutomaticCamera )
							{
								resetSmoothCameraDeltaYaw();
							}
						}
					}
				}
			}
			// if Right Mouse button is up
			else
			{
				// mouse camera look: when the user hold down the left button for a time, while the right button is up
				if (EventsListener.isMouseButtonDown (leftButton))
				{
					// NB: start the freeLook sooner than a longclick
					if((T1 -_LeftClickStart ) > _TimeBeforeMouseSlide)
						// FullMode (hide mouse...) only if longClick
						cameraLook(T1-_LeftClickStart > _TimeLongClick);
				}

				if ( ! (isSwimming() || isRiding()))
				{
					// Strafe Left
					if(Actions.valide("strafe_left"))
						_DirectionMove |= left;
					// Strafe Right
					if(Actions.valide("strafe_right"))
						_DirectionMove |= right;
				}

				if( !UserEntity->isSit() && !UserEntity->isAFK() )
				{
					// Test rotation keys
					if( UserEntity->canChangeFront() )
					{
						keyboardRotationLR (Actions.valide("turn_left"), Actions.valide("turn_right"));
					}
					else
					{
						keyboardRotationCameraLR (Actions.valide("turn_left"), Actions.valide("turn_right"));
					}
				}
			}
		}
	}

	if (!_MouseCaptured)
	{
		// If both button are pressed in any case (whatever the _TimeBeforeMouseSlide time), treat as "forward"
		static bool bothButtonPressed = false;
		if(EventsListener.isMouseButtonDown (leftButton) && EventsListener.isMouseButtonDown (rightButton))
		{
			// Forward.
			_DirectionMove |= forward;
			_DirectionMove &= ~backward;
			// quit autowalk if entering both button pressed mode
			if( !bothButtonPressed )
			{
				_DirectionMove &= ~autowalk;
			}

			// Moving Break any Follow Mode
			UserEntity->disableFollow();
			UserEntity->moveTo(CLFECOMMON::INVALID_SLOT, 0.0, CUserEntity::None);

			// if sit, stand up
			UserEntity->sit(false);
			// and no more in afk mode
			UserEntity->setAFK(false);

			bothButtonPressed = true;
		}
		else
		{
			bothButtonPressed = false;
		}
	}


	// Manage here the Release of Forward in any of third case (keyUP, leftButton in freeLook, and both buttons down)
	if( (_DirectionMove & forward) && (_DirectionMove & autowalk)==0)
	{
		// freeLook() called and leftButton down => walk
		bool	walkInFreeLook= _CurrentFrameFreeLookCalled && EventsListener.isMouseButtonDown(leftButton);
		// both button down => walk too
		bool	walkWithBothButton= (EventsListener.isMouseButtonDown (leftButton) && EventsListener.isMouseButtonDown (rightButton));
		// if no forward action valid, quit forward
		if(! (Actions.valide("forward") || walkInFreeLook || walkWithBothButton) )
			_DirectionMove &= ~forward;
	}


	////////////////////
	// CAMERA DELTA YAW ADDS
	////////////////////
	if (!_MouseCaptured)
	{
		// reset too if middle button clicked
		if(EventsListener.isMouseButtonReleased(middleButton))
		{
			resetSmoothCameraDeltaYaw();
		}

		// Handle rotations with keys
		bool cameraLeft,cameraRight;
		if( UserEntity->isSit() || UserEntity->isAFK() )
		{
			cameraLeft = Actions.valide("camera_turn_left") || Actions.valide("turn_left");
			cameraRight = Actions.valide("camera_turn_right") || Actions.valide("turn_right");
		}
		else
		{
			cameraLeft = Actions.valide("camera_turn_left");
			cameraRight = Actions.valide("camera_turn_right");
		}
		keyboardRotationCameraLR (cameraLeft, cameraRight);


		// reset over time
		if(_ResetSmoothCameraDeltaYaw!=ResetCDYOff)
		{
			// normalize in -Pi/Pi
			_UserCameraDeltaYaw = float(fmod(_UserCameraDeltaYaw, 2.0f*(float)Pi));
			if(_UserCameraDeltaYaw>Pi)
				_UserCameraDeltaYaw-= 2*float(Pi);
			else if(_UserCameraDeltaYaw<-Pi)
				_UserCameraDeltaYaw+= 2*float(Pi);

			// reset over time
			if(_UserCameraDeltaYaw<0)
			{
				_UserCameraDeltaYaw+= DT*ClientCfg.CameraResetSpeed;
				if(_UserCameraDeltaYaw>=0)
				{
					_UserCameraDeltaYaw= 0;
					_ResetSmoothCameraDeltaYaw= ResetCDYOff;
				}
			}
			else
			{
				_UserCameraDeltaYaw-= DT*ClientCfg.CameraResetSpeed;
				if(_UserCameraDeltaYaw<=0)
				{
					_UserCameraDeltaYaw= 0;
					_ResetSmoothCameraDeltaYaw= ResetCDYOff;
				}
			}
		}
	}

	////////////////////
	// APPLY MOTION.
	////////////////////
	move();
}// commonMove //

//-----------------------------------------------
// move :
// Move the caracter according to the inputs.
//-----------------------------------------------
void CUserControls::move()
{
	// If the user is sit(), remove any move
	if( UserEntity->isSit() || UserEntity->isAFK() )
		_DirectionMove= none;

	if( _DirectionMove != none )
	{
		cancelActionsWhenMoving();
	}

	// Forward/backward motion.
	float direction = 0;
	if(_DirectionMove&forward)
		direction += 1;
	if( UserEntity->mode() != MBEHAV::MOUNT_SWIM )
	{
		if(_DirectionMove&backward)
			direction -= 1;
	}
	// Changes the front velocity for the user.
	UserEntity->frontVelocity(direction);

	// Strafe motion.
	float directionStrafe = 0;
	if( ! (isSwimming() || isRiding()) )
	{
		if(_DirectionMove&left)
			directionStrafe += 1;
		if(_DirectionMove&right)
			directionStrafe -= 1;
	}
	else if ( isRiding() )
	{
		// Check if the mount is able to run, and force walking mode if not, because the player can always switch the mode
		UserEntity->checkMountAbleToRun();
	}

	// Changes the lateral velocity for the user.
	UserEntity->lateralVelocity(directionStrafe);

}// move //


//-----------------------------------------------
// mode :
// Change the current Mode.
//-----------------------------------------------
void CUserControls::mode(const TMoveMode mode)
{
	// Unlock the motion;
	_Locked = false;

	// If the mode didn't change -> return.
	if(_Mode == mode)
		return;

	// Write that the last Mode end.
	switch(_Mode)
	{
	case InterfaceMode:
		interfaceModeStop();
		break;

	case AIMode:
		aiModeStop();
		break;

	// Death Mode
	case DeathMode:
		deathModeStop();
		break;

	// Mount Mode
	case MountMode:
		mountModeStop();
		break;

	// Third Person View Mode
	case ThirdMode:
		thirdModeStop();
		break;

	default:
		break;
	}

	// Backup the last Mode.
	_LastMode = _Mode;
	// Set the new mode.
	_Mode = mode;

	// Write that the new Mode Start.
	switch(_Mode)
	{
	case InterfaceMode:
		interfaceModeStart();
		break;

	// AI Mode
	case AIMode:
		aiModeStart();
		break;

	// Death Mode
	case DeathMode:
		deathModeStart();
		break;

	// Mount Mode
	case MountMode:
		mountModeStart();
		break;

	// Third Person View Mode
	case ThirdMode:
		thirdModeStart();
		break;

	default:
		break;
	}
}// mode //

//-----------------------------------------------
// modeStr :
// Return the string associated to the motion Mode.
//-----------------------------------------------
string CUserControls::modeStr() const
{
	switch(mode())
	{
	// Interface Mode
	case CUserControls::InterfaceMode:
		return "InterfaceMode";

	// Camera Mode
	case CUserControls::AIMode:
		return "AIMode";

	// Death Mode
	case CUserControls::DeathMode:
		return "DeathMode";

	// Mount Mode
	case CUserControls::MountMode:
		return "MountMode";

	// Third Person View Mode
	case CUserControls::ThirdMode:
		return "Third Person View Mode";

	// Unknown Mode
	default:
		return "Unknown Control Mode";
	}
}// modeStr //

//-----------------------------------------------

//-----------------------------------------------
// execActionCursorPos :
// Execute action depending on the cursor position (left/right click).
//-----------------------------------------------
void CUserControls::execActionCursorPos(bool rightClick, bool dblClick)
{
	// Check there is no interface under the cursor.
	if( CWidgetManager::getInstance()->isMouseOverWindow())
		return;
	// Update the cursor.
	ContextCur.check();
	// Execute Action.
	ContextCur.execute(rightClick,dblClick);
}// execActionCursorPos //


//-----------------------------------------------
// resetCameraDeltaYaw()
//-----------------------------------------------
void	CUserControls::resetCameraDeltaYaw()
{
	_UserCameraDeltaYaw= 0.f;
	_ResetSmoothCameraDeltaYaw= ResetCDYOff;
}


//-----------------------------------------------
// appendCameraDeltaYaw()
//-----------------------------------------------
void	CUserControls::appendCameraDeltaYaw(float dYaw)
{
	_UserCameraDeltaYaw+= dYaw;

	// Stop the smooth reset if any
	_ResetSmoothCameraDeltaYaw= ResetCDYOff;
}


//-----------------------------------------------
// resetSmoothCameraDeltaYaw()
//-----------------------------------------------
void	CUserControls::resetSmoothCameraDeltaYaw()
{
	// Force the reset if the user currently in follow/moveTo
	if(UserEntity->follow() || UserEntity->moveTo())
		_ResetSmoothCameraDeltaYaw= ResetCDYForced;
	else
		_ResetSmoothCameraDeltaYaw= ResetCDYOn;
}


//-----------------------------------------------
// applyCameraDeltaYawToUser();
//-----------------------------------------------
void	CUserControls::applyCameraDeltaYawToUser()
{
	// apply delta camera to the UserEntity
	UserEntity->rotate(_UserCameraDeltaYaw);
	// and reset
	_UserCameraDeltaYaw= 0.f;
	_ResetSmoothCameraDeltaYaw= ResetCDYOff;
}


//-----------------------------------------------
// testApplyCameraYawToUser()
//-----------------------------------------------
void	CUserControls::testApplyCameraYawToUser()
{
	// only if not in "camera look" mode
	if(!EventsListener.isMouseButtonDown(leftButton))
	{
		// if any of those actions start
		if( ( !_LastFrameForward && Actions.valide("forward") && !_NeedReleaseForward) ||
			( !_LastFrameBackward && Actions.valide("backward") ) ||
			( !_LastFrameAutowalk && Actions.valide("toggle_auto_walk") ) )
			/*( !_LastFrameStrafeLeft && Actions.valide("strafe_left") ) ||
			( !_LastFrameStrafeRight && Actions.valide("strafe_right") ) ||
			( !_LastFrameTurnLeft && Actions.valide("turn_left") ) ||
			( !_LastFrameTurnRight && Actions.valide("turn_right") ) )*/
		{
			applyCameraDeltaYawToUser();
		}
	}

	// if both buttons are pressed, then reset.
	// This is for the case where right button is pressed while left button was down for cameraLook()
	if( EventsListener.isMouseButtonDown(leftButton) && EventsListener.isMouseButtonDown(rightButton))
	{
		applyCameraDeltaYawToUser();
	}

}


//-----------------------------------------------
//commonSetView()
//-----------------------------------------------
void	CUserControls::commonSetView()
{
	// Set the view direction.
	CMatrix camMat;
	camMat.identity();
	camMat.setRot(CVector(0,-1,0), CVector(1,0,0), CVector(0,0,1), true);
	camMat.rotateZ(UserEntity->frontYaw() + _UserCameraDeltaYaw);
	camMat.rotateX(float(UserEntity->getHeadPitch()));
	View.view(camMat.getJ());

	// Update the mount direction.
	CEntityCL *mount = EntitiesMngr.entity(UserEntity->parent());
	if(mount)
	{
		mount->front(UserEntity->front());
	}
}


//-----------------------------------------------
// startFreeLook()
//-----------------------------------------------
void CUserControls::startFreeLook()
{
	_FreeLook= true;
}

//-----------------------------------------------
// stopFreeLook()
//-----------------------------------------------
void CUserControls::stopFreeLook()
{
	EventsListener.enableMouseSmoothing(false);
	_FreeLook= false;

	// Cursor mode
	SetMouseCursor ();
	// Give back the mouse handling to the interface.
	CWidgetManager::getInstance()->enableMouseHandling(true);
}


//-----------------------------------------------
// cancelActionsWhenMoving()
//-----------------------------------------------
void CUserControls::cancelActionsWhenMoving()
{
	if( UserEntity->behaviour() == MBEHAV::EXTRACTING ||
		(UserEntity->behaviour() >= MBEHAV::MAGIC_CASTING_BEHAVIOUR_BEGIN && UserEntity->behaviour() <= MBEHAV::MAGIC_CASTING_BEHAVIOUR_END) )
	{
		UserEntity->cancelAllPhrases();
	}
	if( UserEntity->behaviour() == MBEHAV::RANGE_ATTACK )
	{
		//UserEntity->disengage();
	}
}
