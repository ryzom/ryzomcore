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

#ifndef NL_MOVE_LISTENER_H
#define NL_MOVE_LISTENER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/matrix.h>
#include <nel/3d/viewport.h>
#include <nel/3d/scene.h>
#include <nel/3d/camera.h>
#include <nel/3d/driver.h>



namespace NL3D 
{


/**
 * CMoveListener is a listener that handle a 3d matrix with mouse events.
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CMoveListener : public NLMISC::IEventListener
{

public :
	
	/** 
	 * TMoveMode. 
	 * moving modes
	 */
	enum TMoveMode
	{
		WALK,
		FREE
	};

private:
	
	/// Internal use
	virtual void operator()(const NLMISC::CEvent& event);

	/// moving mode
	TMoveMode _Mode;
	
	/// view matrix
	NLMISC::CMatrix _ViewMatrix;

	/// viewport
	NL3D::CViewport	_Viewport;
	
	/// true if first setMousePos done
	bool _CursorInit;

	CScene * _Scene;
	
	/// screen width
	uint _Width;

	/// screen height
	uint _Height;

	///frustum parameters
	float _Top;
	float _Bottom;
	float _Left;
	float _Right;
	float _Depth;
	
	/// current position
	CVector _Pos;

	/// eyes height
	float _EyesHeight;

	///current rotation z angle
	float _RotZ;

	///current rotation x angle
	float _RotX;
	
	/// last local time measured
	NLMISC::TTime _LastTime;

	/// current local time
	NLMISC::TTime _CurrentTime;

	/// step for translation
	float _TransSpeed;

	/// step for rotation
	float _RotSpeed;

	/// false if mouse is use to move
	bool _MouseFree;

public:
	
	
	/** 
	 * Constructor. 
	 * You should call init then.
	 */
	CMoveListener();


	/** 
	 * Constructor. 
	 * You should call init then.
	 * \param scene the scene
	 * \param w screen width in pixels
	 * \param h screen height in pixels
	 * \param camera the camera
	 */
	void init(CScene * scene, uint w, uint h, const class CCamera& camera)
	{
		_Scene = scene;
		_Width = w;
		_Height = h;
		
		_Viewport = scene->getViewport();

		float dummy;
		_ViewMatrix = camera.getMatrix ();
		camera.getFrustum (_Left, _Right, _Bottom, _Top, _Depth, dummy);
	}

	/**
	 * Set the moving mode
	 * \param m the moving mode
	 */
	void setMode(TMoveMode m)
	{
		_Mode = m;
	}

	void swapMode()
	{
		if(_Mode==WALK)
		{
			_Mode = FREE;
		}
		else
		{
			_Mode = WALK;
		}
	}

	TMoveMode getMode() const
	{
		return _Mode;
	}

	void changeViewMatrix();

	/** 
     * Register the listener to the server.
	 */
	void addToServer (NLMISC::CEventServer& server);

	/** 
	  * Unregister the listener to the server.
	  */
	void removeFromServer (NLMISC::CEventServer& server);

	void setEyesHeight(float eh) { _EyesHeight = eh; }
	float getEyesHeight() const { return _EyesHeight; }

	void setPos(CVector pos) { _Pos = pos; }
	CVector getPos() const { return _Pos; }

	float getRotX() const { return _RotX; }
	float getRotZ() const { return _RotZ; }

	float getSpeed() const { return _TransSpeed; }

	void setLocalTime(NLMISC::TTime time)
	{
		_LastTime = _CurrentTime;
		_CurrentTime = time;
	}

	void changeControlMode()
	{
		_MouseFree = ! _MouseFree;
		if(_MouseFree)
		{
			_Scene->getDriver()->showCursor(true);
		}
		else
		{
			_Scene->getDriver()->showCursor(false);
		}	
	}
		

}; // NL3D

}

#endif // NL_MOVE_LISTENER_H

/* End of move_listener.h */
