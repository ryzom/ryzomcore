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

#ifndef NL_VIEWPORT_H
#define NL_VIEWPORT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"
#include "nel/3d/frustum.h"

#ifdef _X
#	undef _X
#endif

namespace NL3D
{

using NLMISC::CVector;
using NLMISC::CMatrix;



/**
 * CViewport is the description of the viewport used to render with a driver
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class CViewport
{
public:

	/// Default constructor. Setup a fullscreen viewport
	CViewport ();

	/**
	  * Constructor
	  *
	  * \param x coordinate of the left edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param y coordinate of the bottom edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param width of the view port. Must be between 0.f and 1.f-x.
	  * \param height of the view port. Must be between 0.f and 1.f-y.
	  */
	void init (float x, float y, float width, float height);

	/// Setup a fullscreen viewport.
	void initFullScreen ();

	/// Setup a 16/9 viewport.
	void init16_9 ();

	/** Get a 3d ray with a 2d point
	  *
	  * \param x is the x coordinate in the window coordinate system of the 2d point.
	  * \param y is the y coordinate in the window coordinate system of the 2d point.
	  * \param pos gets the position of a 3d point on the ray. It is also the position of the camera
	  * \param dir gets the direction of the ray. The direction is the same than the camera one. It is NOT normalized.
	  * \param camMatrix is the matrix of the camera in use in this viewport.
	  * \param camFrust is the frustum of the camera in use in this viewport.
	  */
	void getRayWithPoint (float x, float y, CVector& pos, CVector& dir, const CMatrix& camMatrix, const CFrustum& camFrust) const;

	/** Get the viewport values
	  *
	  * \param x get the x coordinate of the left edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param y get the y coordinate of the bottom edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param width get the width of the view port. Must be between 0.f and 1.f-x.
	  * \param height get the height of the view port. Must be between 0.f and 1.f-y.
	  */
	void getValues (float& x, float& y, float& width, float& height) const
	{
		x=_X;
		y=_Y;
		width=_Width;
		height=_Height;
	}

	float getX() const { return _X; }
	float getY() const { return _Y; }
	float getWidth() const { return _Width; }
	float getHeight() const { return _Height; }


private:
	float	_X;
	float	_Y;
	float	_Width;
	float	_Height;
};


} // NL3D


#endif // NL_VIEWPORT_H

/* End of viewport.h */
