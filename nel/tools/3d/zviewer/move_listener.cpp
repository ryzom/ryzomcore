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

#include "move_listener.h"
#include <nel/misc/event_server.h>
#include <nel/3d/driver.h>
#include <nel/3d/nelu.h>


using namespace NLMISC;

namespace NL3D {





/****************************************************************\
						 CMoveListener()
\****************************************************************/
CMoveListener::CMoveListener()
{
	_ViewMatrix.identity();
	_Viewport.initFullScreen();
	
	_Mode = WALK;

	_MouseFree = false;

	_CursorInit = false;

	_Width = 0;
	_Height = 0;

	_Top=1.f;
	_Bottom=1.f;
	_Left=1.f;
	_Right=1.f;
	_Depth=1.f;

	_RotZ = 0;
	_RotX = 0;

	_TransSpeed = 1.66f;
	_RotSpeed = 1;

	_CurrentTime = 0;
	_LastTime = 0;

	_EyesHeight = 1.80f;
}


/****************************************************************\
						 changeViewMatrix()
\****************************************************************/
void CMoveListener::changeViewMatrix()
{
	const float time(deltaTimeInSeconds());

	// Forward
	if(CNELU::AsyncListener.isKeyDown(KeyUP))
	{
		if(_Mode==WALK)
		{
			_Pos.x -= time*_TransSpeed * (float)sin(_RotZ);
			_Pos.y += time*_TransSpeed * (float)cos(_RotZ);
		}
		else
		{
			_Pos.x -= time*_TransSpeed * (float)sin(_RotZ)*(float)cos(_RotX);
			_Pos.y += time*_TransSpeed * (float)cos(_RotZ)*(float)cos(_RotX);
			_Pos.z += time*_TransSpeed * (float)sin(_RotX);
		}
	}
	// Backward
	if(CNELU::AsyncListener.isKeyDown(KeyDOWN))
	{
		if(_Mode==WALK)
		{
			_Pos.x += time*_TransSpeed * (float)sin(_RotZ);
			_Pos.y -= time*_TransSpeed * (float)cos(_RotZ);
		}
		else
		{
			_Pos.x += time*_TransSpeed * (float)sin(_RotZ)*(float)cos(_RotX);
			_Pos.y -= time*_TransSpeed * (float)cos(_RotZ)*(float)cos(_RotX);
			_Pos.z -= time*_TransSpeed * (float)sin(_RotX);
		}
	}

	// Strafe left
	if(CNELU::AsyncListener.isKeyDown(KeyLEFT))
	{
		_Pos.x = _Pos.x - time*_TransSpeed*(float)cos(_RotZ);
		_Pos.y = _Pos.y - time*_TransSpeed*(float)sin(_RotZ);
	}
	// Strafe right
	if(CNELU::AsyncListener.isKeyDown(KeyRIGHT))
	{
		_Pos.x = _Pos.x + time*_TransSpeed*(float)cos(_RotZ);
		_Pos.y = _Pos.y + time*_TransSpeed*(float)sin(_RotZ);
	}
	// Up
	if(CNELU::AsyncListener.isKeyDown(KeySHIFT))
	{
		_Pos.z += time*_TransSpeed;
	}
	// Down
	if(CNELU::AsyncListener.isKeyDown(KeyCONTROL))
	{
		_Pos.z -= time*_TransSpeed;
	}
	
	if(CNELU::AsyncListener.isKeyPushed(Key1))
	{
		_TransSpeed = 5/3.6f; // 5km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key2))
	{
		_TransSpeed = 10/3.6f; // 10km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key3))
	{
		_TransSpeed = 20/3.6f; // 20km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key4))
	{
		_TransSpeed = 40/3.6f; //40km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key5))
	{
		_TransSpeed = 80/3.6f; //80km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key6))
	{
		_TransSpeed = 160/3.6f; //160km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key7))
	{
		_TransSpeed = 320/3.6f; //320km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key8))
	{
		_TransSpeed = 640/3.6f; //640km/h
	}
	
	// Transform view matrix
	_ViewMatrix.identity();
	_ViewMatrix.translate(CVector(0,0,_EyesHeight));
	_ViewMatrix.translate(_Pos);
	_ViewMatrix.rotateZ(_RotZ);
	_ViewMatrix.rotateX(_RotX);

	CNELU::Camera->setMatrix(_ViewMatrix);

}


/****************************************************************\
						 operator()
\****************************************************************/
void CMoveListener::operator()(const CEvent& event)
{
	// Rotation
	if (event==EventMouseMoveId)
	{
		CEventMouse* mouseEvent=(CEventMouse*)&event;

		if(!_MouseFree)
		{
			if(_CursorInit)
			{
				// Mouse positions are fractions of the window, measure the motion in whole pixels
				// from the pixel the cursor is put back on, so that both axes turn the same per pixel
				// and the half pixel of an odd window size does not drift the view
				uint32 width, height;
				_Scene->getDriver()->getWindowSize(width, height);
				if (width > 0 && height > 0)
				{
					sint centerX = (sint)((float)width * 0.5f);
					sint centerY = (sint)((float)height * 0.5f);
					sint mouseX = (sint)floor(mouseEvent->X * (float)width + 0.5f);
					sint mouseY = (sint)floor((1.f - mouseEvent->Y) * (float)height + 0.5f);

					// one window width of motion turns by _RotSpeed radians
					float radPerPixel = _RotSpeed / (float)width;
					_RotZ += radPerPixel * (float)(centerX - mouseX);
					_RotX += radPerPixel * (float)(centerY - mouseY);

					// stop short of looking straight up or down, past it the view flips over
					const float maxRotX = (float)Pi * 0.5f - 0.001f;
					clamp(_RotX, -maxRotX, maxRotX);
				}
			}
			else
			{
				_CursorInit = true;
			}
			_Scene->getDriver()->setMousePos(0.5,0.5);
		}
	}
}


/****************************************************************\
						 addToServer()
\****************************************************************/
void CMoveListener::addToServer(CEventServer& server)
{
	server.addListener (EventMouseMoveId, this);
}


/****************************************************************\
						 removeFromServer()
\****************************************************************/
void CMoveListener::removeFromServer (CEventServer& server)
{
	server.removeListener (EventMouseMoveId, this);
}

}; // NL3D
