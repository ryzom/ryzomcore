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

#include "std3d.h"

#include "nel/3d/event_mouse_listener.h"
#include "nel/misc/event_server.h"
#include "nel/3d/camera.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/quat.h"

using namespace NLMISC;

namespace NL3D
{


CEvent3dMouseListener::CEvent3dMouseListener() :  _CurrentModelRotationAxis(zAxis)
										          ,_XModelTranslateEnabled(true)
												  ,_YModelTranslateEnabled(true)
												  ,_ZModelTranslateEnabled(true)
{
	_Matrix.identity();
	_ModelMatrix.identity() ;
	_EnableModelMatrixEdition = false ;
	_HotSpot.set (0,0,0);
	_Viewport.initFullScreen();
	_Frustrum.init (2.f, 2.f, -1.f, 1.f);
	_MouseMode=nelStyle;
	setSpeed (10.f);
	_LastTime=CTime::getLocalTime ();
	_TranslateXYInWorld= false;
}



void CEvent3dMouseListener::enableModelTranslationAxis(TAxis axis, bool enabled)
{
	switch (axis)
	{
		case xAxis: _XModelTranslateEnabled = enabled ; break ;
		case yAxis: _YModelTranslateEnabled = enabled ; break ;
		case zAxis: _ZModelTranslateEnabled = enabled ; break ;
	}
}

bool CEvent3dMouseListener::isModelTranslationEnabled(TAxis axis)
{
	switch (axis)
	{
		case xAxis: return _XModelTranslateEnabled ; break ;
		case yAxis: return _YModelTranslateEnabled ; break ;
		case zAxis: return _ZModelTranslateEnabled ; break ;
		default: return false ; break ;
	}
}


void CEvent3dMouseListener::truncateVect(CVector &v)
{
	if (!_XModelTranslateEnabled) v.x = 0.f ;
	if (!_YModelTranslateEnabled) v.y = 0.f ;
	if (!_ZModelTranslateEnabled) v.z = 0.f ;
}


void CEvent3dMouseListener::operator ()(const CEvent& event)
{
	CEventMouse* mouseEvent=(CEventMouse*)&event;
	if (event==EventMouseMoveId)
	{
		bool bRotate=false;
		bool bTranslateXY=false;
		bool bTranslateZ=false;
		bool bZoom=false;


		// Rotate Axis
		CVector axis;

		if (_MouseMode==nelStyle)
		{
			bRotate=(mouseEvent->Button==(ctrlButton|rightButton));
			bTranslateXY=(mouseEvent->Button==(ctrlButton|leftButton));
			bTranslateZ=(mouseEvent->Button==(ctrlButton|shiftButton|leftButton));
			bZoom=(mouseEvent->Button==(altButton|leftButton));
			axis=_HotSpot;
		}
		else if (_MouseMode==edit3d)
		{
			bRotate=(mouseEvent->Button==(altButton|middleButton)) || (mouseEvent->Button==(altButton|leftButton));
			bTranslateXY=(mouseEvent->Button==(ctrlButton|leftButton)) || (mouseEvent->Button==middleButton);
			bTranslateZ=(mouseEvent->Button==(ctrlButton|shiftButton|leftButton)) || (mouseEvent->Button==(ctrlButton|middleButton));
			bZoom=(mouseEvent->Button==(shiftButton|leftButton)) || (mouseEvent->Button==(ctrlButton|altButton|middleButton));
			axis=_HotSpot;
		}
		else // if (_MouseMode==firstPerson)
		{
			bRotate=(mouseEvent->Button&leftButton)!=0;
			bTranslateXY=false;
			bTranslateZ=false;
			bZoom=false;
			axis=_Matrix.getPos();
		}

		if (bRotate)
		{
			if (!_EnableModelMatrixEdition)
			{
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
				turnX.rotateX ((float) Pi*2.f*(mouseEvent->Y-_Y));

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

			}
			else
			{
				CVector pos = _ModelMatrix.getPos() ;
				NLMISC::CQuat r ;
				switch (_CurrentModelRotationAxis)
				{
					case xAxis : r = CQuat(CAngleAxis(_ModelMatrix.getI(), (float) Pi*2.f*(_X-mouseEvent->X))) ; break ;
					case yAxis : r = CQuat(CAngleAxis(_ModelMatrix.getJ(), (float) Pi*2.f*(_X-mouseEvent->X))) ; break ;
					case zAxis : r = CQuat(CAngleAxis(_ModelMatrix.getK(), (float) Pi*2.f*(_X-mouseEvent->X))) ; break ;
				} ;


				CMatrix rm ;
				rm.rotate(r) ;

				_ModelMatrix = rm * _ModelMatrix ;
				_ModelMatrix.setPos(pos) ;

				_ModelMatrix.normalize (CMatrix::XYZ);
			}
		}


		if (bTranslateXY||bTranslateZ||bZoom)
		{
			// Move in plane

			CPlane plane;

			// For precision problem, do all the compute local to the hotspot/model.
			CVector		decal;

			// Plane of the hotspot
			if (! _EnableModelMatrixEdition)
			{
				decal= axis;
			}
			else
			{
				decal= _ModelMatrix.getPos();
			}
			// Choose plane to move on
			if (bTranslateXY && _TranslateXYInWorld)
			{
				plane.make (CVector::K, CVector::Null);
			}
			else
			{
				plane.make (_Matrix.getJ(), CVector::Null);
			}


			// Get ray from mouse point
			CMatrix		localViewMatrix= _Matrix;
			localViewMatrix.setPos(_Matrix.getPos() - decal);
			CVector localPoint1, localPoint2;
			CVector pos, dir;
			_Viewport.getRayWithPoint (_X, _Y, pos, dir, localViewMatrix, _Frustrum);
			localPoint1=plane.intersect (pos, pos+dir);
			_Viewport.getRayWithPoint (mouseEvent->X, mouseEvent->Y, pos, dir, localViewMatrix, _Frustrum);
			localPoint2=plane.intersect (pos, pos+dir);

			// Move the camera
			if (bTranslateXY)
			{
				if (! _EnableModelMatrixEdition)
				{
					_Matrix.setPos(_Matrix.getPos()+localPoint1-localPoint2);
				}
				else
				{
					CVector dir =  - localPoint1 + localPoint2 ;
					// transform the translation as needed.
					dir= _ModelMatrixTransformMove * dir;
					truncateVect(dir) ;
					_ModelMatrix.setPos(_ModelMatrix.getPos()+dir);
				}
			}
			else if (bTranslateZ)
			{
				CVector vect=localPoint1-localPoint2;
				if (! _EnableModelMatrixEdition)
				{
					_Matrix.setPos(_Matrix.getPos()+_Matrix.getK()*(vect.x+vect.y+vect.z));
				}
				else
				{
					CVector dir = _Matrix.getK()*(vect.x+vect.y+vect.z) ;
					// transform the translation as needed.
					dir= _ModelMatrixTransformMove * dir;
					truncateVect(dir) ;
					_ModelMatrix.setPos(_ModelMatrix.getPos()+dir);
				}
			}
			else if (bZoom)
			{
				CVector vect=localPoint1-localPoint2;
				CVector direc=axis-_Matrix.getPos();
				direc.normalize();
				if (! _EnableModelMatrixEdition)
				{
					_Matrix.setPos(_Matrix.getPos()+direc*(vect.x+vect.y+vect.z));
				}
				else
				{
					// transform the translation as needed.
					direc= _ModelMatrixTransformMove * direc;
					direc.normalize();
					_ModelMatrix.setPos(_ModelMatrix.getPos()+direc*(vect.x+vect.y+vect.z));
				}
			}
		}


		// Update mouse position
		_X=mouseEvent->X;
		_Y=mouseEvent->Y;
	}
	else if (event==EventMouseDownId)
	{
		// Update mouse position
		_X=mouseEvent->X;
		_Y=mouseEvent->Y;
	}
	else if (event==EventMouseUpId)
	{
		// Update mouse position
		_X=mouseEvent->X;
		_Y=mouseEvent->Y;
	}
	else if (event==EventMouseWheelId)
	{
		// Zoom..
		CEventMouseWheel* mouseEvent=(CEventMouseWheel*)&event;

		CVector direc=_HotSpot-_Matrix.getPos();
		if (! _EnableModelMatrixEdition)
		{
			_Matrix.setPos(_Matrix.getPos()+direc*(mouseEvent->Direction?0.1f:-0.1f));
		}
		else
		{
			CVector dir = direc*(mouseEvent->Direction?0.1f:-0.1f) ;
			// transform the translation as needed.
			dir= _ModelMatrixTransformMove * dir;
			truncateVect(dir) ;
			_ModelMatrix.setPos(_ModelMatrix.getPos() + dir);
		}
	}
}

void CEvent3dMouseListener::addToServer (CEventServer& server)
{
	server.addListener (EventMouseMoveId, this);
	server.addListener (EventMouseDownId, this);
	server.addListener (EventMouseUpId, this);
	server.addListener (EventMouseWheelId, this);
	_AsyncListener.addToServer (server);
}

void CEvent3dMouseListener::removeFromServer (CEventServer& server)
{
	server.removeListener (EventMouseMoveId, this);
	server.removeListener (EventMouseDownId, this);
	server.removeListener (EventMouseUpId, this);
	server.removeListener (EventMouseWheelId, this);
	_AsyncListener.removeFromServer (server);
}

const NLMISC::CMatrix& CEvent3dMouseListener::getViewMatrix ()
{
	// Mode first person ?
	if (_MouseMode==firstPerson)
	{
		// CVector
		CVector dir (0,0,0);
		bool find=false;

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
	}

	// Last time
	_LastTime=CTime::getLocalTime ();


	// Return the matrix
	return _Matrix;
}


void CEvent3dMouseListener::enableTranslateXYInWorld(bool enabled)
{
	_TranslateXYInWorld= enabled;
}

void CEvent3dMouseListener::setModelMatrixTransformMove(const NLMISC::CMatrix& transModelMove)
{
	_ModelMatrixTransformMove= transModelMove;
}

void CEvent3dMouseListener::getModelMatrixTransformMove(NLMISC::CMatrix& transModelMove) const
{
	transModelMove= _ModelMatrixTransformMove;
}


}; // NL3D
