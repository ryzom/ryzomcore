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



#include "stdpch.h"

#include "control.h"
#include "interfaces_manager.h"


//-----------------------------------------------
// CControl :
// Constructor.
//-----------------------------------------------
CControl::CControl(uint id)
{
	init(id, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
}// CControl //


//-----------------------------------------------
// CControl :
// Constructor.
//-----------------------------------------------
CControl::CControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel)
{
	init(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel);
}// CControl //


//-----------------------------------------------
// init :
// Constructor.
//-----------------------------------------------
void CControl::init(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel)
{
	// Not child.
	_Children.clear();

	// Id of the control.
	_Id			= id;

	_X			= x;		// Position X of the Control (between 0-1).
	_Y			= y;		// Position Y of the Control (between 0-1).
	_X_Pixel	= x_pixel;	// Position X of the Control (in Pixel).
	_Y_Pixel	= y_pixel;	// Position Y of the Control (in Pixel).

	_W			= w;		// Width  of the control (between 0-1).
	_H			= h;		// Height of the control (between 0-1).
	_W_Pixel	= w_pixel;	// Width  of the control (in Pixel).
	_H_Pixel	= h_pixel;	// Height of the control (in Pixel).

	// The control position is relative to this Reference.
	_X_Ref		= 0.f;
	_Y_Ref		= 0.f;
	// Size of the Parent.
	_W_Ref		= 1.f;
	_H_Ref		= 1.f;

	// Delta to add to the position because of the Hot Spot.
	_X_HotSpot	= 0.f;
	_Y_HotSpot	= 0.f;

	// Hot Spot
	_HotSpot	= HS_MR;
	// Do the control have to be displayed. true -> yes.
	_Show		= true;

	_Origin		= HS_MM;
	_Parent		= 0;

	// Calculate others variables.
	calculateDisplay();
}// CControl //


//-----------------------------------------------
// resize :
// The window size has changed -> resize the control.
//-----------------------------------------------
void CControl::resize(uint32, uint32)
{
	calculateDisplay();

	// Update chidren.
	for(TListControl::iterator it = _Children.begin(); it != _Children.end(); ++it)
	{
		if((*it)->parent() == this)
		{
			float x, y;
			calculateOrigin(x, y, (*it)->origin());
			(*it)->ref(x, y, _W_Ref, _H_Ref);
		}
	}
}// resize //

//-----------------------------------------------
// ref :
// Set some references for the display.
//-----------------------------------------------
void CControl::ref(float x, float y, float w, float h)
{
	_X_Ref	= x;
	_Y_Ref	= y;
	_W_Ref	= w;
	_H_Ref	= h;

	calculateDisplay();

	// Update chidren.
	for(TListControl::iterator it = _Children.begin(); it != _Children.end(); ++it)
	{
		if((*it)->parent() == this)
		{
			float x, y;
			calculateOrigin(x, y, (*it)->origin());
			(*it)->ref(x, y, _W_Ref, _H_Ref);
		}
	}
}// ref //

//-----------------------------------------------
// addChild :
// Add a child to the control.
//-----------------------------------------------
void CControl::addChild(CControl *ctrl)
{
	_Children.push_back(ctrl);
}// addChild //


//-----------------------------------------------
// hotSpot :
// Change the Hot Spot.
//-----------------------------------------------
void CControl::hotSpot(THotSpot hs)
{
	_HotSpot = hs;
	calculateDisplay();

	// Update chidren.
	for(TListControl::iterator it = _Children.begin(); it != _Children.end(); ++it)
	{
		if((*it)->parent() == this)
		{
			float x, y;
			calculateOrigin(x, y, (*it)->origin());
			(*it)->ref(x, y, _W_Ref, _H_Ref);
		}
	}
}// hotSpot //

//-----------------------------------------------
// hotSpot :
// Return the Hot Spot.
//-----------------------------------------------
CControl::THotSpot CControl::hotSpot()
{
	return _HotSpot;
}// hotSpot //


//-----------------------------------------------
// show :
// Hide or show the control. false -> hide, true -> show.
//-----------------------------------------------
void CControl::show(bool show)
{
	_Show = show;
}// show //

//-----------------------------------------------
// show :
// Return the show of the control.
//-----------------------------------------------
bool CControl::show()
{
	return _Show;
}// show //

//-----------------------------------------------
// calculateDisplay :
// Calculate the Display X, Y, Width, Height.
//-----------------------------------------------
void CControl::calculateDisplay()
{
	uint32 w, h;
	CInterfMngr::getWindowSize(w, h);

	// Calculate the display Width and Height.
	if(w!=0)
		_W_Display	= _W*_W_Ref + _W_Pixel/w;
	else
		_W_Display	= _W*_W_Ref;

	if(h!=0)
		_H_Display	= _H*_H_Ref + _H_Pixel/h;
	else
		_H_Display	= _H*_H_Ref;

	// Calculate the HotSpot.
	calculateHS();

	_X_Display = _X_Ref + _X*_W_Ref + _X_Pixel/w + _X_HotSpot;
	_Y_Display = _Y_Ref + _Y*_H_Ref + _Y_Pixel/h + _Y_HotSpot;
}// calculateDisplay //

//-----------------------------------------------
// calculateHS :
// Calculate the display position of the control in relation to the position of the control (Hot Spot).
//-----------------------------------------------
void CControl::calculateHS()
{
	switch(_HotSpot)
	{
	case HS_TL:
		_X_HotSpot	= -_W_Display;
		_Y_HotSpot	= 0.f;
		break;
	case HS_TM:
		_X_HotSpot	= -_W_Display/2.f;
		_Y_HotSpot	= 0.f;
		break;
	case HS_TR:
		_X_HotSpot	= 0.f;
		_Y_HotSpot	= 0.f;
		break;

	case HS_ML:
		_X_HotSpot	= -_W_Display;
		_Y_HotSpot	= -_H_Display/2.f;
		break;
	case HS_MM:
		_X_HotSpot	= -_W_Display/2.f;
		_Y_HotSpot	= -_H_Display/2.f;
		break;
	case HS_MR:
		_X_HotSpot	= 0.f;
		_Y_HotSpot	= -_H_Display/2.f;
		break;

	case HS_BL:
		_X_HotSpot	= -_W_Display;
		_Y_HotSpot	= -_H_Display;
		break;
	case HS_BM:
		_X_HotSpot	= -_W_Display/2.f;
		_Y_HotSpot	= -_H_Display;
		break;
	case HS_BR:
		_X_HotSpot	= 0.f;
		_Y_HotSpot	= -_H_Display;
		break;
	}
}// calculateHS() //

//-----------------------------------------------
// calculateOrigin :
// Function to calculate where to position a child.
//-----------------------------------------------
void CControl::calculateOrigin(float &x, float &y, THotSpot origin)
{
	switch(origin)
	{
	case CControl::HS_TL:
		x = _X_Display;
		y = _Y_Display+_H_Display;
		break;
	case CControl::HS_TM:
		x = _X_Display+_W_Display/2.f;
		y = _Y_Display+_H_Display;
		break;
	case CControl::HS_TR:
		x = _X_Display+_W_Display;
		y = _Y_Display+_H_Display;
		break;

	case CControl::HS_ML:
		x = _X_Display;
		y = _Y_Display+_H_Display/2.f;
		break;
	case CControl::HS_MM:
		x = _X_Display+_W_Display/2.f;
		y = _Y_Display+_H_Display/2.f;
		break;
	case CControl::HS_MR:
		x = _X_Display+_W_Display;
		y = _Y_Display+_H_Display/2.f;
		break;

	case CControl::HS_BL:
		x = _X_Display;
		y = _Y_Display;
		break;
	case CControl::HS_BM:
		x = _X_Display+_W_Display/2.f;
		y = _Y_Display;
		break;
	case CControl::HS_BR:
		x = _X_Display+_W_Display;
		y = _Y_Display;
		break;
	}
}// calculateOrigin //



//-----------------------------------------------
// mouseMove :
// called when the mouse has moved, give the new coordinates
//-----------------------------------------------
void CControl::mouseMove( float x, float y)
{
	// send the message to all it's children

	TListControl::iterator	itChild;
	const TListControl::iterator	itChildEnd = _Children.end();

	for (itChild = _Children.begin() ; itChild != itChildEnd ; ++itChild)
	{
		(*itChild)->mouseMove( x, y );
	}
}// mouseMove //


//-----------------------------------------------
// click :
// manage left mouse button click
//-----------------------------------------------
void CControl::click(float x, float y, bool &taken)
{
}// click //


//-----------------------------------------------
// clickRight :
// manage right mouse button click
//-----------------------------------------------
void CControl::clickRight(float x, float y, bool &taken)
{
}// clickRight //


//-----------------------------------------------
// getDisplayValues :
// get display values of this control
//-----------------------------------------------
void CControl::getDisplayValues(float &x, float &y, float &h, float &w) const
{
	x = _X_Display;
	y = _Y_Display;
	h = _H_Display;
	w = _W_Display;
}// getDisplayValues //



//-----------------------------------------------
// getSize :
// get the size of the control
//-----------------------------------------------
void CControl::getSize( float &w, float &h, float &wPixel, float &hPixel) const
{
	w = _W;
	h = _H;
	wPixel = _W_Pixel;
	hPixel = _H_Pixel;
}// getSize //


//-----------------------------------------------
// getPosition :
// get the position of the control
//-----------------------------------------------
void CControl::getPosition( float &x, float &y, float &xPixel, float &yPixel ) const
{
	x = _X;
	y = _Y;
	xPixel = _X_Pixel;
	yPixel = _Y_Pixel;
}// getPosition //


//-----------------------------------------------
// setSize :
// set the size of the control
//-----------------------------------------------
void CControl::setSize( float w, float h, float wPixel, float hPixel )
{
	_W = w;
	_H = h;
	_W_Pixel = wPixel;
	_H_Pixel = hPixel;

	calculateDisplay();

	// update children
	for(TListControl::iterator it = _Children.begin(); it != _Children.end(); ++it)
	{
		if((*it)->parent() == this)
		{
			float nx, ny;
			calculateOrigin(nx, ny, (*it)->origin());
			(*it)->ref(nx, ny, _W_Ref, _H_Ref);
		}
	}
}// setSize //


//-----------------------------------------------
// setPosition :
// set the position of the control
//-----------------------------------------------
void CControl::setPosition( float x, float y, float xPixel, float yPixel)
{
	_X = x;
	_Y = y;
	_X_Pixel = xPixel;
	_Y_Pixel = yPixel;

	calculateDisplay();

	// update children
	for(TListControl::iterator it = _Children.begin(); it != _Children.end(); ++it)
	{
		if((*it)->parent() == this)
		{
			float nx, ny;
			calculateOrigin(nx, ny, (*it)->origin());
			(*it)->ref(nx, ny, _W_Ref, _H_Ref);
		}
	}

}// setPosition //
