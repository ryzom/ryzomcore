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

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <nel/misc/event_server.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_visual_collision_entity.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_instance.h>

#include <nel/misc/time_nl.h>
#include <nel/misc/quat.h>
#include <nel/misc/plane.h>
#include <nel/misc/config_file.h>

#include "snowballs_client.h"
#include "mouse_listener.h"
#include "entities.h"
#include "camera.h"
#include "landscape.h"
#include "network.h"

//
// Namespaces
//

using namespace NLMISC;
using namespace NL3D;

namespace SBCLIENT {

//
// Variables
//

static float MouseZoomStep;

static float GroundCamLimit = 0.5f;

//
// Functions
//

C3dMouseListener::C3dMouseListener()
{
	// -- -- a mouse listener that can aim at the ground with a shape
	//       and does a bunch of other random things that should be
	//       done by something else, really

	// -- -- half of this thing has to be split up into a
	//       CKeyboardControlledMovePrimitive or something

	_Matrix.identity();
	_ModelMatrix.identity() ;
	_EnableModelMatrixEdition = false ;
	_HotSpot.set (0,0,0);
	_Viewport.initFullScreen();
	_Frustrum.init (2.f, 2.f, -1.f, 1.f);
	_MouseMode=nelStyle;
	setSpeed (10.f);
	_LastTime=CTime::getLocalTime ();

	_ViewLagBehind = 20.0f;
	_ViewHeight = 2.0f;
	_ViewTargetHeight = 3.0f;

	_AimingState = false;
	_AimingDamage = 0.0f;
	_AimingSpeed = 1.0f;
	_AimingMax = 5.0f;
	_AimingStartTime = 0;
	_AimingRefreshRate = 100;
	_AimingInstance = Scene->createInstance("aim.shape");
	_AimingInstance.setTransformMode(UTransformable::RotQuat);

	_X = 0.5f;
	_Y = 0.5f;
	_InvertedMouse = false;
}

C3dMouseListener::~C3dMouseListener()
{
	// -- -- doesn't need to know about the snowballs specific global, 
	//       scene shout be passed at construction to create aiming instance
	// -- -- random note: aiminginstance here and aimingentity in landscape.cpp ...
	//       one of them obviously is in the wrong place
	Scene->deleteInstance(_AimingInstance);
}



void C3dMouseListener::operator ()(const CEvent& event)
{
	CEventMouse* mouseEvent=(CEventMouse*)&event;
	if (event==EventMouseMoveId) // && mouseEvent->Button&leftButton!=0)
	{
		// Rotate Axis
		CVector axis;

		axis=_Matrix.getPos();

		// First in the hotSpot
		CMatrix comeFromHotSpot=_Matrix;
		comeFromHotSpot.setPos (axis);

		// Then turn along the Z axis with X mouse
		CMatrix turnZ;
		turnZ.identity();
		turnZ.rotateZ ((float) Pi*2.f*(_X-mouseEvent->X));

		// Then turn along the X axis with Y mouse
		CMatrix turnX;
		turnX.identity();
		if (_InvertedMouse)
			_ViewHeight += 3.0f*(mouseEvent->Y-_Y);
		else
			_ViewHeight -= 3.0f*(mouseEvent->Y-_Y);

		// Then come back from hotspot
		CMatrix goToHotSpot=comeFromHotSpot;
		goToHotSpot.invert();

		// Make the matrix
		CMatrix negPivot, Pivot;
		negPivot.identity();
		negPivot.setPos (-axis);
		Pivot.identity();
		Pivot.setPos (axis);

		// Make this transformation \\//
		//_Matrix=Pivot*turnZ*negPivot*comeFromHotSpot*turnX*goToHotSpot*_Matrix;
		Pivot*=turnZ;
		Pivot*=negPivot;
		Pivot*=comeFromHotSpot;
		Pivot*=turnX;
		Pivot*=goToHotSpot;

		
		Pivot*=_Matrix;
		_Matrix=Pivot;						
		// Normalize, too much transformation could give an ugly matrix..
		_Matrix.normalize (CMatrix::XYZ);			

		// Update mouse position
		Driver->setMousePos(0.5f, 0.5f);
		_X = 0.5f;
		_Y = 0.5f;
	}
	else if (event==EventMouseDownId)
	{
		// aim
		_AimingState = true;
		_AimingDamage = 0.0f;
		_AimingStartTime = CTime::getLocalTime();
		_AimingLastUpdateTime = 0;
	}
	else if (event==EventMouseUpId)
	{
		// throw snowball
//		nlinfo("damage=%f", _AimingDamage);
		_AimingState = false;
		_AimingStartTime = 0;
		CVector	direction = (_AimedTarget-_AimingPosition).normed();

		if (isOnline ())
		{
			sendSnowBall (Self->Id, _AimingPosition, _AimedTarget, SnowballSpeed, _AimingDamage);
		}
		else
		{
			shotSnowball (NextEID++, Self->Id, _AimingPosition, _AimedTarget, SnowballSpeed, _AimingDamage);
		}
	}
	else if (event==EventMouseWheelId)
	{
		CEventMouseWheel* mouseEvent=(CEventMouseWheel*)&event;
		_ViewLagBehind += (mouseEvent->Direction? -MouseZoomStep : +MouseZoomStep);
		if (_ViewLagBehind < 2.0f)
			_ViewLagBehind = 2.0f;
	}
}

void C3dMouseListener::addToServer (CEventServer& server)
{
	server.addListener (EventMouseMoveId, this);
	server.addListener (EventMouseDownId, this);
	server.addListener (EventMouseUpId, this);
	server.addListener (EventMouseWheelId, this);
	_AsyncListener.addToServer (server);
}

void C3dMouseListener::removeFromServer (CEventServer& server)
{
	server.removeListener (EventMouseMoveId, this);
	server.removeListener (EventMouseDownId, this);
	server.removeListener (EventMouseUpId, this);
	server.removeListener (EventMouseWheelId, this);
	_AsyncListener.removeFromServer (server);
}

const NLMISC::CMatrix& C3dMouseListener::getViewMatrix ()
{
	// Return the matrix
	return _Matrix;
}

void C3dMouseListener::update()
{
	// CVector
	CVector dir (0,0,0);
	bool find=false;

	// -- -- what does the mouse listener have to do with the keyboard?!

	// Key pushed ?
	if (_AsyncListener.isKeyDown (KeyUP))
	{
		dir+=CVector (0, 1, 0);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyDOWN))
	{
		dir+=CVector (0, -1, 0);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyRIGHT))
	{
		dir+=CVector (1, 0, 0);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyLEFT))
	{
		dir+=CVector (-1, 0, 0);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyNEXT))
	{
		dir+=CVector (0, 0, -1);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyPRIOR))
	{
		dir+=CVector (0, 0, 1);
		find=true;
	}

	// Character state setup
	if (Self != NULL)
	{
		// modify the orientation depending on the straff
		// The straff is determined by the keys that are down simultaneously
		if (_AsyncListener.isKeyDown (KeyUP))
		{
			if (_AsyncListener.isKeyDown (KeyLEFT))
			{
				Self->AuxiliaryAngle = (float)Pi/4.0f;
			}
			else if (_AsyncListener.isKeyDown (KeyRIGHT))
			{
				Self->AuxiliaryAngle = -(float)Pi/4.0f;
			}
			else
			{
				Self->AuxiliaryAngle = 0;
			}
			_IsWalking = true;
		}
		else if (_AsyncListener.isKeyDown (KeyDOWN))
		{
			if (_AsyncListener.isKeyDown (KeyLEFT))
			{
				Self->AuxiliaryAngle = (float)Pi-(float)Pi/4.0f;
			}
			else if (_AsyncListener.isKeyDown (KeyRIGHT))
			{
				Self->AuxiliaryAngle = -(float)Pi+(float)Pi/4.0f;
			}
			else
			{
				Self->AuxiliaryAngle = (float)Pi;
			}
			_IsWalking = true;
		}
		else if (_AsyncListener.isKeyDown (KeyLEFT))
		{
			Self->AuxiliaryAngle = (float)Pi/2.0f;
			_IsWalking = true;
		}
		else if (_AsyncListener.isKeyDown (KeyRIGHT))
		{
			Self->AuxiliaryAngle = -(float)Pi/2.0f;
			_IsWalking = true;
		}
		else
		{
			_IsWalking = false;
		}

		if (isAiming())
			Self->AuxiliaryAngle = 0.0f;

		Self->IsAiming = isAiming();
		Self->IsWalking = isWalking();
	}


	// if is aiming
	if (_AimingState)
	{
		find = false;
		TTime	newTime = CTime::getLocalTime();
		float	delta = (float)(newTime-_AimingStartTime)/1000.0f;
		_AimingDamage = _AimingSpeed*delta;
		_AimingDamage = std::min(_AimingDamage, _AimingMax);

		// if we have to update the aiming position
		if (newTime - _AimingLastUpdateTime > _AimingRefreshRate)
		{
			// update the last target
			_AimingLastUpdateTime = newTime;
			// set up the aiming position
			_AimingPosition = MouseListener->getPosition()+CVector(0.0f, 0.0f, 2.0f);
			// compute the target
			_AimedTarget = getTarget(_AimingPosition,
									 MouseListener->getViewDirection(),
									 100);
		}

		if (Self != NULL && !_AimingInstance.empty())
		{
			_AimingInstance.lookAt(MouseListener->getAimedTarget(), Camera.getMatrix().getPos());
			float	scale = MouseListener->getDamage();
			_AimingInstance.setScale(scale, scale, scale);
			_AimingInstance.show();
		}
	}
	else
	{
		if (Self != NULL && !_AimingInstance.empty())
			_AimingInstance.hide();
	}


	// key found ?
	if (find)
	{
		// Time elapsed
		uint32 milli=(uint32)(CTime::getLocalTime ()-_LastTime);

		// Speed
		float dPos=_Speed*(float)milli/1000.f;

		// Good direction
		dir.normalize ();
		dir*=dPos;

		// Orientation
		dir=_Matrix.mulVector (dir);

		// New position
		_Matrix.setPos (_Matrix.getPos ()+dir);
	}

	// Last time
	_LastTime=CTime::getLocalTime ();

}


void	C3dMouseListener::setPosition(const CVector &position)
{
	_Matrix.setPos(position);
}

NLMISC::CVector	C3dMouseListener::getPosition() const
{
	return _Matrix.getPos();
}

void	C3dMouseListener::setOrientation(float angle)
{
	/// todo
}

float	C3dMouseListener::getOrientation()
{
	CVector	j = getViewMatrix().getJ();
	j.z = 0.0f;
	j.normalize();
	return (float)atan2(j.y, j.x);
}

CVector	C3dMouseListener::getViewDirection()
{
//	float	angle = getOrientation();
//	return CVector((float)cos(angle), (float)sin(angle), (_ViewTar-_ViewHeight)/_ViewLagBehind).normed();
	return Camera.getMatrix().getJ();
}

void	C3dMouseListener::updateCamera()
{
	CVector	tpos = getPosition()+CVector(0.0f, 0.0f, _ViewTargetHeight);
	CVector	cpos = getPosition()+CVector(-(float)cos(getOrientation())*_ViewLagBehind, -(float)sin(getOrientation())*_ViewLagBehind, _ViewHeight);
	CVector snapped = cpos,
			normal;
	if (CamCollisionEntity->snapToGround(snapped, normal) && (cpos.z-(snapped.z+GroundCamLimit))*normal.z < 0.0f)
	{
		cpos = snapped+CVector(0.0f, 0.0f, GroundCamLimit);
		_ViewHeight = cpos.z - getPosition().z;
	}
	if (StereoHMD)
	{
		// look at straight forward
		tpos.z = cpos.z;
	}
	_Camera.lookAt(cpos, tpos);
}

void	cbUpdateMouseListenerConfig(CConfigFile::CVar &var)
{
	if (var.Name == "MouseInvert") MouseListener->setInvertMouseMode(var.asInt() != 0);
	else if (var.Name == "MouseZoomStep") MouseZoomStep = var.asFloat ();
	else if (var.Name == "ViewLagBehind") MouseListener->setViewLagBehind(var.asFloat ());
	else if (var.Name == "ViewHeight") MouseListener->setViewHeight(var.asFloat ());
	else if (var.Name == "ViewTargetHeight") MouseListener->setViewTargetHeight(var.asFloat ());
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void	initMouseListenerConfig()
{
	ConfigFile->setCallback ("MouseInvert", cbUpdateMouseListenerConfig);
	ConfigFile->setCallback ("MouseZoomStep", cbUpdateMouseListenerConfig);
	ConfigFile->setCallback ("ViewLagBehind", cbUpdateMouseListenerConfig);
	ConfigFile->setCallback ("ViewHeight", cbUpdateMouseListenerConfig);
	ConfigFile->setCallback ("ViewTargetHeight", cbUpdateMouseListenerConfig);

	cbUpdateMouseListenerConfig(ConfigFile->getVar ("MouseInvert"));
	cbUpdateMouseListenerConfig(ConfigFile->getVar ("MouseZoomStep"));
	cbUpdateMouseListenerConfig(ConfigFile->getVar ("ViewLagBehind"));
	cbUpdateMouseListenerConfig(ConfigFile->getVar ("ViewHeight"));
	cbUpdateMouseListenerConfig(ConfigFile->getVar ("ViewTargetHeight"));
}

void releaseMouseListenerConfig()
{
	ConfigFile->setCallback("MouseInvert", NULL);
	ConfigFile->setCallback("MouseZoomStep", NULL);
	ConfigFile->setCallback("ViewLagBehind", NULL);
	ConfigFile->setCallback("ViewHeight", NULL);
	ConfigFile->setCallback("ViewTargetHeight", NULL);
}

} /* namespace SBCLIENT */

/* end of file */
