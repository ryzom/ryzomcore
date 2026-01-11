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

#ifndef NL_EVENT_MOUSE_LISTENER_H
#define NL_EVENT_MOUSE_LISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/matrix.h"
#include "nel/3d/viewport.h"
#include "nel/3d/frustum.h"
#include "nel/3d/u_3d_mouse_listener.h"

#ifdef _X
#	undef _X
#endif

namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CMatrix;


/**
 * CEvent3dMouseListener is a listener that handle a 3d matrix with mouse events.
 * This can be the view matrix, or the matrix of any object.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CEvent3dMouseListener : public NLMISC::IEventListener, public U3dMouseListener
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
	CEvent3dMouseListener();
	virtual ~CEvent3dMouseListener() {}

	/// \name Setup

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
	void setFrustrum (const CFrustum& frustrum)
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
	  * Set the mouse mode.
	  * \param mouseMode is the mode you want to use.
	  * \see TMouseMode
	  */
	void setMouseMode(TMouseMode mouseMode)
	{
		_MouseMode=mouseMode;
	}

	/// enable / disable model matrix edition mode. (the default deals with the with matrix)
	void enableModelMatrixEdition(bool enable = true)
	{
		_EnableModelMatrixEdition = enable ;
	}

	enum TAxis { xAxis = 0, yAxis = 1, zAxis = 2 } ;

	/// set the current axe of rotation for the model matrix
	void setModelMatrixRotationAxis(TAxis axis) { _CurrentModelRotationAxis = axis ;}

	/// get the current axe of rotation for the model matrix
	TAxis getModelMatrixRotationAxis(void) const  { return _CurrentModelRotationAxis ;}



	/** enable / disable an axe for translation (model matrix)
	  * \param axis the axis to enable / diable
	  * \param enabled true if the trnaslation is permitted on that axis
	  */
	void enableModelTranslationAxis(TAxis axis, bool enabled) ;

	/** check whether translation on the given axis is permitted
	  * \param axis the axis to check
	  * \return true if translation is permitted
	  */
	bool isModelTranslationEnabled(TAxis axis) ;

	/** Each move of the modelMatrix is first transformed by this matrix before applying. NB: rot are still done in local
	 *	\transModelMove Trans part is removed
	 */
	void setModelMatrixTransformMove(const NLMISC::CMatrix& transModelMove);

	/// see setModelMatrixTransformMove
	void getModelMatrixTransformMove(NLMISC::CMatrix& transModelMove) const;


	/** if true, XY translation is made in XY World plane (instead of XY view plane).
	 *	false by default.
	 */
	void enableTranslateXYInWorld(bool enabled);

	/** \see translateXYInWorld()
	 */
	bool isTranslateXYInWorldEnabled() const {return _TranslateXYInWorld;}

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

private:
	/// Internal use
	virtual void operator ()(const NLMISC::CEvent& event);



	TAxis                _CurrentModelRotationAxis ;
	bool                _XModelTranslateEnabled ;
	bool                _YModelTranslateEnabled ;
	bool                _ZModelTranslateEnabled ;

	CMatrix				_Matrix;
	CMatrix				_ModelMatrix ;
	bool				_EnableModelMatrixEdition  ;
	CFrustum			_Frustrum;
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
	bool				_TranslateXYInWorld;
	NLMISC::CEventListenerAsync	_AsyncListener;

	CMatrix				_ModelMatrixTransformMove;

	/** remove composant of translations that are not permitted
	  * \see enableModelTranslationAxis()
	  */
	void truncateVect(CVector &v);

	virtual NLMISC::IEventListener &getEventListenerInterface() { return *this; }


}; // NL3D

}

#endif // NL_EVENT_MOUSE_LISTENER_H

/* End of event_mouse_listener.h */
