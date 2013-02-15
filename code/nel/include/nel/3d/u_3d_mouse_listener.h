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

#ifndef NL_U_3D_MOUSE_LISTENER_H
#define NL_U_3D_MOUSE_LISTENER_H

#include "nel/misc/types_nl.h"

namespace NLMISC
{
	class IEventListener;
}


namespace NL3D
{


/**
 * 3d mouse listener
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class U3dMouseListener
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
	enum TMouseMode { nelStyle, edit3d, firstPerson };

	virtual ~U3dMouseListener() {}

	/// \name Setup

	/**
	  * Set the current view matrix to use.
	  * \param matrix is the matrix to set.
	  * \see getViewMatrix()
	  */
	virtual void setMatrix (const NLMISC::CMatrix& matrix) =0;

	/**
	  * Set the current frustrum to use.
	  * \param frustrum is the frustrum.
	  */
	virtual void setFrustrum (const CFrustum& frustrum) =0;

	/**
	  * Set the viewport in use in the window. By default, the viewport is full window.
	  * \param viewport is the viewport to use. All events outside the viewport are ignored.
	  */
	virtual void setViewport (const NL3D::CViewport& viewport) =0;

	/**
	  * Set the current hot spot.
	  * \param hotSpot is the target to use when the mouse move. It can be for example the center.
	  * of the selected object. The hotspot is not modified by mouse events.
	  * \see getViewMatrix()
	  */
	virtual void setHotSpot (const CVector& hotSpot) =0;

	/**
	  * Set the mouse mode.
	  * \param mouseMode is the mode you want to use.
	  * \see TMouseMode
	  */
	virtual void setMouseMode(TMouseMode mouseMode) =0;

	/**
	  * Set the speed for first person mode. Default 10.f;
	  * \param speed is in unit per second.
	  * \see TMouseMode
	  */
	virtual void setSpeed (float speed) =0;

	/// \name Get

	/**
	  * Get the current view matrix. This matrix is updated with mouse events.
	  * \return The current view matrix.
	  * \see setMatrix()
	  */
	virtual const NLMISC::CMatrix& getViewMatrix () =0;

	/**
	  * Get the current hot spot.
	  * \return the target used when the mouse move. It can be for example the center.
	  * of the selected object. The hotspot is not modified by mouse events.
	  * \see getViewMatrix()
	  */
	virtual CVector getHotSpot () const =0;

	// Obtain a NLMISC::IEventListener interface on that listener
	virtual NLMISC::IEventListener &getEventListenerInterface() = 0;

};

} // NL3D

#endif // NL_U_3D_MOUSE_LISTENER_H

/* End of u_3d_mouse_listener.h */
