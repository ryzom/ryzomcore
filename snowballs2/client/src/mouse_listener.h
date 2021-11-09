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

#ifndef MOUSE_LISTENER_H
#define MOUSE_LISTENER_H

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/matrix.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/viewport.h>
#include <nel/3d/frustum.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_instance.h>

#include "camera.h"

//
// External definitions
//

using NLMISC::CVector;
using NLMISC::CMatrix;

namespace SBCLIENT {

//
// External classes
//

/**
 * C3dMouseListener is a listener that handle a 3d matrix with mouse events.
 * This can be the view matrix, or the matrix of any object.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class C3dMouseListener : public NLMISC::IEventListener, public NL3D::U3dMouseListener
{
public:
	/**
	  * Mouse mode.
	  * There is two move modes: 3d editor style and NeL style. Default mode is NeL style.
	  * 
	  * (nelStyle) NeL style is:
	  * MouseRotateHotSpot:			CTRL + RIGHTMOUSE
	  * MouseTranslateXYHotSpot:	CTRL + LEFTMOUSE
	  * MouseTranslateZHotSpot:		CTRL + SHIFT + LEFTMOUSE
	  * MouseZoomHotSpot:			ALT + LEFTMOUSE
	  * 
	  * (edit3dStyle) 3d editor style is:
	  * MouseRotateHotSpot:			ALT + MIDDLEMOUSE
	  * MouseTranslateXYHotSpot:	MIDDLEMOUSE
	  * MouseTranslateZHotSpot:		CTRL + MIDDLEMOUSE
	  * 
	  * (firstPerson) First person shooter style is:
	  * MouseRotateView:			MOUSE MOVE
	  * KeyUp:						MOVE FORWARD
	  * KeyDown:					MOVE BACKWARD
	  * Left:						STRAF LEFT
	  * Right:						STRAF RIGHT
	  * PageUp:						MOVE UP
	  * PageDown:					MOVE DOWN
	  */
	//enum TMouseMode { nelStyle, edit3d, firstPerson };

	/** 
	  * Constructor. 
	  * You should call setMatrix, setFrustrum, setViewport, setHotStop and setMouseMode to initialize
	  * the whole object. By default, the viewmatrix is identity, the frustrum is (-1,1,-1,1,1,-1), the hot spot is (0,0,0) 
	  * and the viewport is fullscreen. The mouse mode is set to the NelStyle.
	  */
	C3dMouseListener();

	~C3dMouseListener();

	/// \name Setup

	/// Setup the mouse mode. No op actually.
	void setMouseMode(TMouseMode mouseMode) {}

	/// Setup the camera to be used
	void setCamera (NL3D::UCamera camera) { _Camera = camera; }

	/** 
	  * Set both the current view matrix to use.
	  * \param matrix is the matrix to set.
	  * \see getViewMatrix()
	  */
	void setMatrix (const NLMISC::CMatrix& matrix)
	{
		_Matrix=matrix;	
	}

	/** Set the model matrix only
	  *  
	  * param matrix is the matrix to set.
	  * getModelMatrix()
	  */
	void setModelMatrix(const NLMISC::CMatrix& matrix)
	{
		_ModelMatrix = matrix ;
	}

	/** 
	  * Set the current frustrum to use.
	  * \param frustrum is the frustrum.
	  */
	void setFrustrum (const NL3D::CFrustum& frustrum)
	{
		_Frustrum=frustrum;
	}

	/** 
	  * Set the viewport in use in the window. By default, the viewport is fullwindow.
	  * \param viewport is the viewport to use. All events outside the viewport are ignored.
	  */
	void setViewport (const NL3D::CViewport& viewport)
	{
		_Viewport=viewport;
	}

	/** 
	  * Set the current hot spot.
	  * \param hotSpot is the target to use when the mouse move. It can be for exemple the center.
	  * of the selected object. The hotspot is not modified by mouse events.
	  * \see getViewMatrix()
	  */
	void setHotSpot (const CVector& hotSpot)
	{
		_HotSpot=hotSpot;
	}


	/** 
	  * Set the speed for first person mode. Default 10.f;
	  * \param speed is in unit per second.
	  * \see TMouseMode
	  */
	void setSpeed (float speed)
	{
		_Speed=speed;
	}

	/// \name Get

	/**
	  * Get the current view matrix.	  
	  * \return The current view matrix.
	  * \see setMatrix()
	  */
	const NLMISC::CMatrix& getViewMatrix () ;


	/**
	  * Get the current model matrix.
	  * \return The current view matrix.
	  * \see setModelMatrix()
	  */
	const NLMISC::CMatrix& getModelMatrix()
	{
		return _ModelMatrix ;	
	}

	NLMISC::IEventListener &getEventListenerInterface() { return *this; }

	/** 
	  * Get the current hot spot.
	  * \return the target used when the mouse move. It can be for exemple the center.
	  * of the selected object. The hotspot is not modified by mouse events.
	  * \see getViewMatrix()
	  */
	CVector getHotSpot () const
	{
		return _HotSpot;
	}

	/** 
	  * Register the listener to the server.
	  */
	void addToServer (NLMISC::CEventServer& server);

	/** 
	  * Unregister the listener to the server.
	  */
	void removeFromServer (NLMISC::CEventServer& server);


	/// update the position of the listener
	void setPosition(const NLMISC::CVector &position);
	/// get the position of the listener
	NLMISC::CVector	getPosition() const;
	/// set the orientation angle of the listener
	void setOrientation(float angle);
	/// get orientiation angle
	float getOrientation();
	/// get the view direction
	CVector	getViewDirection();
	/// get the aiming state
	bool getAimingState() const { return _AimingState; }
	/// get the target position (only if getAimingState() == true)
	CVector	getAimedTarget() const { return _AimedTarget; }
	/// get the damage value
	float getDamage() const { return _AimingDamage; }

	// get the various character states
	bool isAiming() const { return _AimingState; }
	bool isWalking() const { return _IsWalking; }

	/// Updates the mouselistener
	void update();

	/// Updates the camera
	void updateCamera();

	//
	bool getInvertMouseMode() const { return _InvertedMouse; }
	void setInvertMouseMode(bool invertMouse) { _InvertedMouse = invertMouse; }

	/// get the ViewLagBehind of the camera
	float getViewLagBehind() const { return _ViewLagBehind; }
	/// set the ViewLagBehind of the camera
	void setViewLagBehind(float viewLagBehind) { _ViewLagBehind = viewLagBehind; }
	/// get the ViewHeight of the camera
	float getViewHeight() const { return _ViewHeight; }
	/// set the ViewHeight of the camera
	void setViewHeight(float viewHeight) { _ViewHeight = viewHeight; }
	/// get the ViewTargetHeight of the camera
	float getViewTargetHeight() const { return _ViewTargetHeight; }
	/// set the ViewTargetHeight of the camera
	void setViewTargetHeight(float viewTargetHeight) { _ViewTargetHeight = viewTargetHeight; }


private:
	/// Internal use
	virtual void operator ()(const NLMISC::CEvent& event);



	CMatrix				_Matrix;
	CMatrix				_ModelMatrix ;
	bool				_EnableModelMatrixEdition  ;
	NL3D::CFrustum			_Frustrum;
	CVector				_HotSpot;
	NL3D::CViewport		_Viewport;
	bool				_LeftPushed;
	bool				_MiddlePushed;
	bool				_RightPushed;
	float				_X;
	float				_Y;
	float				_Speed;
	uint64				_LastTime;
	TMouseMode			_MouseMode;
	NLMISC::CEventListenerAsync	_AsyncListener;

	// viewing camera attributes
	NL3D::UCamera		_Camera;
	float				_ViewLagBehind;
	float				_ViewHeight;
	float				_ViewTargetHeight;

	// aiming system attributes
	bool				_AimingState;
	float				_AimingDamage;
	float				_AimingSpeed;
	float				_AimingMax;
	NLMISC::TTime		_AimingStartTime;
	NLMISC::TTime		_AimingLastUpdateTime;
	NLMISC::TTime		_AimingRefreshRate;
	NLMISC::CVector		_AimedTarget;
	NLMISC::CVector		_AimingPosition;
	NL3D::UInstance		_AimingInstance;
	bool				_InvertedMouse;

	// character state
	bool				_IsWalking;
};

//
// External functions
//

void initMouseListenerConfig();
void releaseMouseListenerConfig();

} /* namespace SBCLIENT */

#endif // MOUSE_LISTENER_H

/* End of mouse_listener.h */
