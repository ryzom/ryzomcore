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




//////////////
// Includes //
//////////////
#include "stdpch.h"
// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_camera.h"
// Client.
#include "input.h"
#include "user_controls.h"
#include "actions_client.h"
#include "user_entity.h"
#include "view.h"
#include "time_client.h"
#include "interface_manager.h"
#include "misc.h"


///////////
// Using //
///////////
using namespace std;
using namespace NLMISC;
using namespace NL3D;


/////////////
// Externs //
/////////////
extern UDriver					*Driver;
extern UScene					*Scene;


///////////////
// Functions //
///////////////
//---------------------------------------------------
// accelerationFromLengthCoeff :
// ...
//---------------------------------------------------
static float accelerationFromLengthCoeff(float v0, float d0, float coeff)
{
	// Whithout acceleration, time to cover d0 at velocity v0 is : to = d0/v0
	// We want to cover the same distance in to*coeff :
	// What's the acceleration ?
	// rq : x = v0*t + 0.5*a*t*t
	float a = 2*(1-coeff)*v0*v0/(coeff*coeff*d0);

	return a;
}// accelerationFromLengthCoeff //


//---------------------------------------------------
// turnBack
// ...
//---------------------------------------------------
void CUserControls::turnBack()
{
	_T0 = _T;
	_T = (float)T1*0.001f;
	float dt = _T-_T0;

	// turn head
	if(_TurnBack)
	{
		if(fabs(_Teta)<Pi/2)
		{
			_Teta += 4*dt*_TetaCoef*ClientCfg.RotKeySpeedMax;

			CMatrix mV;
			mV.rotateZ( 4*dt*_TetaCoef*ClientCfg.RotKeySpeedMax);
			UserEntity->head((mV * UserEntity->head()).normed());

			CMatrix mB;
			mB.rotateZ( 8*dt*_TetaCoef*ClientCfg.RotKeySpeedMax);
			UserEntity->front((mB * UserEntity->front()).normed());
		}
		else
		{
			_TurnBack = false;
			_PrevTeta = _Teta;
			_Teta = 0;
		}
	}
}// turnBack //


//---------------------------------------------------
// updateCursorPosition :
// Update the cursor position.
//---------------------------------------------------
void CUserControls::updateCursorPosition()
{
	float angle;

	// Updating _CursorX
	//==================
	float cosinus = UserEntity->head()*UserEntity->front();
	if(cosinus>1)
	{
		cosinus = 1;
	}
	if(cosinus<-1)
	{
		cosinus = -1;
	}
	angle = (float) acos(cosinus);
	// the cross product gives the angle's sign
	CVector temp = UserEntity->head()^UserEntity->front();
	if(temp.z<0)
	{
		angle = -angle;
	}
	if(_UpdateView)
	{
		angle = -angle;
	}

	_CursorX = (float)( angle/Pi + 0.5f );


	// Updating _CursorY
	//==================
	angle = 0;	// pitch();	// TODO.
	if(angle>=0)
	{
		// up look range : Pi/2
		//							|_
		//
		_CursorY = (float)( angle/Pi + 0.5f );
	}
	else
	{
		// down look range : Pi/4   _
		//							\
		//
		_CursorY = (float)( 2*angle/Pi + 0.5f );
	}
}// updateCursorPosition //


//---------------------------------------------------
// updateViewHeading :
// ...
//---------------------------------------------------
void CUserControls::updateViewHeading()
{
	if(_UpdateView)
	{
		// At first step _Teta==_PrevTeta
		// _Teta always >=0 and decreasing as getting closer of bodyheading
		if(_Teta-_PrevTeta <= 0)
		{
			if(_Teta<(float)Pi/12 && _T0View==0)
			{
				_T0View = (float)T1*0.001f;
			}
			if(_T0View!=0)
			{
				_TView = (float)T1*0.001f;
				float acc = accelerationFromLengthCoeff(1,_Teta,2); //length is twice
				float tmpSpeed = _ViewSpeed;
				_ViewSpeed += acc*(_TView-_T0View);
				if(_ViewSpeed<=0)
				{
					_ViewSpeed = tmpSpeed;
				}
			}


			_PrevTeta = _Teta;
			CMatrix m;
			m.identity();
			m.rotateZ( DT*_TetaCoef*_ViewSpeed );
			UserEntity->head((m * UserEntity->head()).normed());
			_Teta = (float) fabs(acos(UserEntity->front()*UserEntity->head()));
			if(_Teta-_PrevTeta > 0)
			{
				_UpdateView = false;
				UserEntity->head(UserEntity->front());
				_PrevTeta = _Teta;
				_Teta = 0;
				_T0View = 0;
			}
		}
	}
}// updateViewHeading //


//---------------------------------------------------
// findDestination :
// Calculate the destination point.
//---------------------------------------------------
void CUserControls::findDestination(float x, float y)
{
	uint32 width, height;
	Driver->getWindowSize(width, height);

	uint32 mx = (uint32)(x*width);
	uint32 my = (uint32)(y*height);

	// Get zbuffer corresponding to the cursor position.
	CRect rect(mx,my,1,1);
	vector<float> zbuffer;
	Driver->getZBufferPart(zbuffer, rect);

	// If the part of the zbuffer is empty -> return.
	if(zbuffer.size()==0)
		return;

	// Get the destination position.
	UCamera camera = Scene->getCam();
	camera.setTransformMode(UTransform::DirectMatrix);
	CMatrix cam  = camera.getMatrix();
	CFrustum fst = camera.getFrustum();
	_Destination = CVector(x,y,zbuffer[0]);
	_Destination = fst.unProject(_Destination);
	_Destination = cam * _Destination;
	// Get the star position.
	_Start = UserEntity->pos();

	// path
	CVector path = _Destination-_Start;

	// If the distance is too far -> do not move the character.
	if(path.norm() > 10)
		return;

	_Acc		= 0;
	_T0			= 0;
	_TransSpeed	= _V0;
	_TurnBack	= false;
	_Dist		= -1;

	// Update body heading.
	path.z = 0;
	UserEntity->front(path.normed());

	// Teta : abs value of angle between future view heading and current body heading
	_UpdateView	= true;
	_Teta		= (float) fabs(acos(UserEntity->front()*UserEntity->head()));
	_PrevTeta	= _Teta;
	if(x>0.5f)
		_TetaCoef = -1;
	else if(x<0.5f)
		_TetaCoef = 1;
	else
		_TetaCoef = 0;

	// speed for updating view
	_ViewSpeed = 1;

	// go
	UserEntity->frontVelocity( _TransSpeed );
	_ClickMove = true;
}// findDestination //


//-----------------------------------------------
// freeHeadModeStart :
// Manage interactions in free head mode (start).
//-----------------------------------------------
void CUserControls::freeHeadModeStart()
{
	// Get the interface instace.
	CInterfaceManager *instance = CInterfaceManager::getInstance();
	if(instance == 0)
		return;

	// Get the cursor instance
	CViewPointer *cursor = instance->getPointer();
	if(cursor == 0)
		return;

	// Mouse free look
	SetMouseFreeLook ();

	updateCursorPosition();
}// freeHeadModeStart //

//-----------------------------------------------
// freeHeadModeStop :
// Manage interactions in free head mode (stop).
//-----------------------------------------------
void CUserControls::freeHeadModeStop()
{
}// freeHeadModeStop //

//-----------------------------------------------
// freeHeadMode :
// Manage interactions in free head mode.
//-----------------------------------------------
void CUserControls::freeHeadMode()
{
	// Left Mouse Button.
	if(Actions.isMouseButtonReleased (leftButton))
		UserEntity->head(UserEntity->front());

	// Left Mouse Button.
	if(Actions.isMouseButtonReleased (rightButton))
		mode(_LastMode);

	if (EventsListener.isMouseAngleX())
	{
		// If the mouse move on the axis X.
		UserEntity->rotHeadHorizontally(-Actions.getMouseAngleX ());
	}

	if (EventsListener.isMouseAngleY())
	{
		// Mouse Angle Y
		UserEntity->rotHeadVertically(Actions.getMouseAngleY ());
	}

	// Right / Left rotation
	float accel = 0;
	bool left = Actions.valide("strafe_left");
	bool right = Actions.valide("strafe_right");
	if (left || right)
	{
		if (left)
		{
			accel += ClientCfg.RotAccel;
			_RotateVelocity = std::max (_RotateVelocity, ClientCfg.RotKeySpeedMin);
		}

		if (right)
		{
			accel -= ClientCfg.RotAccel;
			_RotateVelocity = std::min (_RotateVelocity, -ClientCfg.RotKeySpeedMin);
		}
		updateVelocity (DT, accel, ClientCfg.RotAccel, ClientCfg.RotKeySpeedMax, _RotateVelocity);
		UserEntity->rotHeadHorizontally(DT*_RotateVelocity);
	}

	updateCursorPosition();

	// Up / Down rotation
	accel = 0;
	bool up = Actions.valide("look_up");
	bool down = Actions.valide("look_down");
	if (up || down)
	{
		if (up)
		{
			accel += ClientCfg.RotAccel;
			_RotateVerticalVelocity = std::max (_RotateVerticalVelocity, ClientCfg.RotKeySpeedMin);
		}

		if (down)
		{
			accel -= ClientCfg.RotAccel;
			_RotateVerticalVelocity = std::min (_RotateVerticalVelocity, -ClientCfg.RotKeySpeedMin);
		}
		updateVelocity (DT, accel, ClientCfg.RotAccel, ClientCfg.RotKeySpeedMax, _RotateVerticalVelocity);
		UserEntity->rotHeadVertically(DT*ClientCfg._RotateVerticalVelocity);
	}

	// Auto-walk
	// (rq : is disabled by forward/backward keyboard action )
	if(Actions.begin("toggle_auto_walk"))
		_DirectionMove ^= autowalk;

	// Action "Foraward" is valide -> Autowalk false, forward true.
	if(Actions.valide("forward"))
	{
		_DirectionMove |= forward;
		_DirectionMove &= ~autowalk;
	}

	// Action "backward" is valide -> Autowalk false, backward true.
	if(Actions.valide("backward"))
	{
		_DirectionMove |= backward;
		_DirectionMove &= ~autowalk;
	}

	if( !isSit() )
	{
		// Action "strafe_left" is valide -> strafeLeft true.
		if(Actions.valide("turn_left"))
			_DirectionMove |= left;

		// Action "strafe_right" is valide -> strafeRight true.
		if(Actions.valide("turn_right"))
			_DirectionMove |= right;
	}

	// if there's no move due to a click
	if(_ClickMove==false)
	{
		move();

		// Turn-back
		//==========
		const float margin = 0.006f;
		if(UserEntity->frontVelocity()==0)
		{
			if(!_TurnBack)
			{
				if(_CursorX < margin)
				{
					_TetaCoef	= 1;
					_TurnBack	= true;
					_Teta		= 0;
					_T0			= (float)T1*0.001f;
					_T			= (float)T1*0.001f;
				}

				if(_CursorX > 1-margin)
				{
					_TetaCoef	= -1;
					_TurnBack	= true;
					_Teta		= 0;
					_T0			= (float)T1*0.001f;
					_T			= (float)T1*0.001f;
				}
			}
		}

		if(_TurnBack)
			turnBack();
	}
	else if(UserEntity->frontVelocity() != 0 )
	{
		CVector tmp = _Destination-UserEntity->pos();
		_PrevDist = _Dist;
		_Dist = (float)sqrt(tmp.x*tmp.x + tmp.y*tmp.y);

		if(_Dist>=_PrevDist && _PrevDist!=-1)
		{
			_Dist = -1;
			UserEntity->frontVelocity(0);
			_T0 = 0;
			_T = 0;
			_Acc = 0;
			_TransSpeed = _V0;
			_ClickMove = false;
		}
		else
		{
			float remainingDist = (_Destination-UserEntity->pos()).norm();
			float totalDist = (_Destination-_Start).norm();
			if(remainingDist<totalDist*20.f/100 && _Acc==0)
			{
				_T0 = (float)T1*0.001f;
				_T = _T0;
				_V0 = _TransSpeed;
			}

			if(_T0!=0)
			{
				if(_Acc == 0)
				{
					_Acc = accelerationFromLengthCoeff(_V0,remainingDist,2); //length is twice
				}
				else	// _Acc!=0
				{
					_T = (float)T1*0.001f;
					_TransSpeed = _V0 + _Acc*(_T-_T0);
					if(_TransSpeed>=0)
					{
						UserEntity->frontVelocity(_TransSpeed);
					}
				}
			}

			float time = (float)T1*0.001f;
			// we suppose : 2 step in one second (one step <-> one period(2Pi))
			// origin's phase : -Pi/2
			// +1 to have only positive values
			// amplitude is 5 centimeters at velocity 3.33 (sin+1 is between 0 and 2(->2m))
			_ZOscil = (float) (sin(2*Pi*2*(time-_StartTime) -Pi/2) + 1 )*(0.05f*_TransSpeed/(3.333333f*2));
			_ZOscil *= (UserEntity->front())*UserEntity->head();
		}
	}

	updateViewHeading();

	// Set the view.
	View.viewPos(UserEntity->pos() + CVector(0,0, UserEntity->eyesHeight()) + CVector(0,0,_ZOscil));
	View.view(UserEntity->head());
}// freeHeadMode //
