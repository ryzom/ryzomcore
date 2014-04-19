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




#ifndef CL_USER_CONTROLS_H
#define CL_USER_CONTROLS_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/vector.h"
// Std.
#include <string>

namespace NLMISC {
	class CCDBNodeLeaf;
}

///////////
// CLASS //
///////////
/**
 * Class to manage the motion.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CUserControls
{
	friend class CAHForward;
	friend class CAHBackward;
	friend class CAHToggleAutoWalk;
public:
	/// moving modes
	enum TMoveMode
	{
		InterfaceMode = 0,	// Interface Mode (with mouse displayed)
		AIMode,				// Mode for the AI.
		DeathMode,			// Mode for the Death.
		MountMode,			// Mode for the Mount.
		ThirdMode,			// Third Person View Mode

		nbMode				// Not really a mode, just here to know how many modes are there.
	};

	enum TDirectionMove
	{
		none		= 0x00,
		forward		= 0x01,
		backward	= 0x02,
		left		= 0x04,
		right		= 0x08,
		autowalk	= 0x10
	};

protected:
	NLMISC::TTime	_LeftClickStart;
	NLMISC::TTime	_LeftClickEnd;
	NLMISC::TTime	_RightClickStart;
	NLMISC::TTime	_RightClickEnd;
	NLMISC::TTime	_TimeBeforeMouseSlide;
	NLMISC::TTime	_TimeLongClick;

private:
	/// Initialize all the components.
	void init();

	/// Just manage the free look.
	void freeLook(bool fullMode);
	/// Just manage the camera look
	void cameraLook(bool fullMode);
	/// common to freeLook and cameraLook
	void getMouseAngleMove(float &dx, float &dy);

	/// Manage interactions in interactive mode (start).
	void interfaceModeStart();
	/// Manage interactions in interactive mode (stop).
	void interfaceModeStop();
	/// Manage interactions in interactive mode.
	void interfaceMode();

	/// Manage the AI Mode (start).
	void aiModeStart();
	/// Manage the AI Mode (stop).
	void aiModeStop();
	/// Manage the AI Mode.
	void aiMode();

	/// Manage the Death Mode (start).
	void deathModeStart();
	/// Manage the Death Mode (stop).
	void deathModeStop();
	/// Manage the Death Mode.
	void deathMode();

	/// Manage the Mount Mode (start).
	void mountModeStart();
	/// Manage the Mount Mode (stop).
	void mountModeStop();
	/// Manage the Mount Mode.
	void mountMode();

	/// Manage the Third Person View Mode (start).
	void thirdModeStart();
	/// Manage the Third Person View Mode (stop).
	void thirdModeStop();
	/// Manage the Third Person View Mode.
	void thirdMode();

	/// Manage some common actions.
	void commonMove();
	/// Move the caracter according to the inputs
	void move();
	/// Calculate the destination point when clicking on the ground in "Free Head" Mode.
	void findDestination(float x, float y);
	/// ...
	void turnBack();
	/// Update the cursor position.
	void updateCursorPosition();
	/// Manage some common view setup.
	void commonSetView();

	// Execute action depending on the cursor position (left/right click).
	void execActionCursorPos(bool rightClick, bool dblClick);

	// test merge the camera Yaw and user Yaw. Only in some cases
	void testApplyCameraYawToUser();

	///  when user moves, some actions must be cancelled
	void cancelActionsWhenMoving();

public:
	/// Constructor
	CUserControls();

	/// Return the string associated to the motion Mode.
	std::string modeStr() const;
	/// Return the motion Mode.
	TMoveMode mode() const {return _Mode;}
	/// Change the current motion Mode.
	void mode(const TMoveMode mode);

	/// Update the motion.
	void update();

	/// Lock or unlock the motion.
	void locked(bool l) {_Locked = l;}
	bool locked() {return _Locked;}

	/// Begin Free Look. Additionaly, cancel any follow/moteTo of the user
	void startFreeLook();

	/// Stop Free Look (can be called multiple times if needed). Additionaly, the mouse/pointer is restored
	void stopFreeLook();

	/// Is the camera inside the character.
	bool isInternalView() {return _InternalView;}

	/// Update keyboard rotation
	void keyboardRotationLR (bool left, bool right);
	void keyboardRotationUD (bool up, bool down);
	void keyboardRotationCameraLR (bool left, bool right);

	void startCombat() {_CameraAuto = true;}

	/// Enable/Disable Autowalk
	void autowalkState(bool enable);
	/// Return the autowalk state ('true'=enable).
	bool autowalkState() const {return ((_DirectionMove & autowalk)!=0);}

	/// Update velocity
	static void updateVelocity (float deltaTime, float acceleration, float brake, float speedMax, float &speed);


	// get the camera Delta Yaw
	float	getCameraDeltaYaw() const {return _UserCameraDeltaYaw;}

	// just reset the camera Delta Yaw (instantaneously)
	void	resetCameraDeltaYaw();

	// reset the camera Delta Yaw (smooth over time, canceled by user)
	void	resetSmoothCameraDeltaYaw();

	// append to the camera Delta Yaw (NB: any smooth reset is then canceled)
	void	appendCameraDeltaYaw(float dYaw);

	// reset the camera Delta Yaw and change the user front so the final Yaw remains the same
	void	applyCameraDeltaYawToUser();

	// true if there is currently a smooth reset of the cameraDeltaYaw (in force mode)
	bool	isResetSmoothCDYForced() const {return _ResetSmoothCameraDeltaYaw==ResetCDYForced;}

	// capture the mouse, prevent the free look from being trigger on a long click
	void    captureMouse() { _MouseCaptured = true; }
	void    releaseMouse() { _MouseCaptured = false; }
	bool    isMouseCaptured() { return _MouseCaptured; }

	/// user has to release forward key before he can go forward again
	void	needReleaseForward();

	/// return true if user do forward or backward move action with key or mouth
	bool	isMoving() { return _DirectionMove != none; }

private:

	bool   _MouseCaptured; // no free look allowed when mouse is captures

	// Is the camera controled by the user or not.
	bool	_CameraAuto;

	bool	_LastFrameForward;
	bool	_LastFrameBackward;
	bool	_LastFrameAutowalk;
	bool	_LastFrameStrafeLeft;
	bool	_LastFrameStrafeRight;
	bool	_LastFrameTurnLeft;
	bool	_LastFrameTurnRight;
	bool	_LastFrameLeftButtonDown;
	// NB: modified only when not in freelook mode
	float	_LastFrameMousePosX;
	float	_LastFrameMousePosY;
	bool	_CurrentFrameFreeLookCalled;
	/// Used to lock motion.
	bool	_Locked;

	/// Are we in displacement or not ?
	uint32 _DirectionMove;

	/// moving mode
	TMoveMode _Mode;
	/// last moving mode
	TMoveMode _LastMode;

	/// speed in translation
	float _TransSpeed;

	/// fly velocity
	float _FlyVerticalVelocity;
	float _FlyFrontVelocity;
	float _FlyLateralVelocity;
	float _RotateUserLRVelocity;
	float _RotateUserUDVelocity;
	float _RotateCameraLRVelocity;

	///
	bool _UpdateView;

	///
	NLMISC::CVector _Start;
	///
	NLMISC::CVector _Destination;

	///
	float _Acc;
	float _T0;
	float _T;
	float _V0;

	///
	float _ZOscil;

	///
	float _Dist;
	float _PrevDist;

	///
	bool _TurnBack;

	/// "true" if the character has a destination to go.
	bool _ClickMove;

	///
	float _T0View;
	float _TView;
	float _ViewSpeed;

	bool _FreeLook;

	bool _InternalView;

	// User Camera Yaw
	float		_UserCameraDeltaYaw;
	// Forced mode is related to moveTo/follow feature
	enum	TResetCDY {ResetCDYOff=0, ResetCDYOn, ResetCDYForced};
	TResetCDY	_ResetSmoothCameraDeltaYaw;

	/// when true user has to release forward key before he can go forward again
	bool _NeedReleaseForward;

	/// when true the next forward action will cancel any moveto
	bool _NextForwardCancelMoveTo;

	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _UiVarMkMoveDB;
};

/// User Controls (mouse, keyboard, interfaces, ...)
extern CUserControls UserControls;


#endif // CL_USER_CONTROLS_H

/* End of user_controls.h */
