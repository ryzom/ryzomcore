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

/////////////
// Include //
/////////////
// 3D
#include "nel/3d/u_driver.h"

// Misc
#include "nel/misc/debug.h"

// Client
#include "horizontal_list.h"
#include "interfaces_manager.h"
#include "graphic.h"

/////////////
// USING   //
/////////////
using namespace NL3D;


/////////////
// EXTERN  //
/////////////
extern UDriver *Driver;
extern UTextContext *TextContext;


//-----------------------------------------------
// Constructor :
//-----------------------------------------------
CHorizontalList::CHorizontalList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel,
								 uint16 spacing, uint texture, CRGBA rgba)
 : CControl( id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel ), CBitmapBase( texture, rgba)
{
	_Spacing = spacing;
	init();
} // contructor //

//-----------------------------------------------
// Destructor :
//-----------------------------------------------
CHorizontalList::~CHorizontalList()
{
	clear();

	// delete scroll arrows
	if (_ScrollLeft != NULL)
	{
		delete _ScrollLeft;
	}
	if (_ScrollRight != NULL)
	{
		delete _ScrollRight;
	}
} // Destructor //

//-----------------------------------------------
// clear :
//-----------------------------------------------
void CHorizontalList::clear( bool delElts )
{
	if (_Locked)
	{
		_ClearRequest = true;
		_DeleteRequest = delElts;
		return;
	}

	if (delElts == true)
	{
		TListControl::iterator it, itE = _Items.end();

		for (it = _Items.begin() ; it != itE ; ++it)
		{
			if ( *it != NULL  && (*it)->id() == 0)
			{
				delete (*it);
				(*it) = NULL;
			}
		}
	}

	_Items.clear();
	_StartingIterator = _Items.begin();
	_NbElts = 0;

	_ClearRequest = false;
	_DeleteRequest = false;
} // clear //



//-----------------------------------------------
// init :
// Initialize the object (1 function called for all constructors -> easier).
//-----------------------------------------------
void CHorizontalList::init()
{
	_StartingIterator = _Items.begin();
	_NbElts = 0;
	_ScrollLeft = NULL;
	_ScrollRight = NULL;
	_Centered = false;

	_Locked = false;
	_ClearRequest = false;
	_DeleteRequest = false;
}// init //

//-----------------------------------------------
// hotSpot :
// Change the Hot Spot.
//-----------------------------------------------
void CHorizontalList::hotSpot(THotSpot hs)
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

	switch( hs )
	{
	case THotSpot::HS_TR:
	case THotSpot::HS_BR:
	case THotSpot::HS_MR:
	case THotSpot::HS_MM:
		if (_ScrollLeft != NULL )
			_ScrollLeft->hotSpot(THotSpot::HS_MR);

		if (_ScrollRight != NULL )
			_ScrollRight->hotSpot(THotSpot::HS_ML);
		break;

	case THotSpot::HS_TL:
	case THotSpot::HS_BL:
	case THotSpot::HS_ML:
		if (_ScrollLeft != NULL )
			_ScrollLeft->hotSpot(THotSpot::HS_ML);

		if (_ScrollRight != NULL )
			_ScrollRight->hotSpot(THotSpot::HS_MR);
		break;
	}
}// hotSpot //

//-----------------------------------------------
// setLeftScrollBitmap :
//-----------------------------------------------
void CHorizontalList::setLeftScrollBitmap( CBitm *bitmap)
{
	nlassert( bitmap );
	_ScrollLeft = new CBitm( *bitmap);

	_ScrollLeft->hotSpot(THotSpot::HS_MR);
	_ScrollLeft->origin(THotSpot::HS_BL);
	_ScrollLeft->ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
} // setLeftScrollBitmap //


//-----------------------------------------------
// setRightScrollBitmap :
//-----------------------------------------------
void CHorizontalList::setRightScrollBitmap( CBitm *bitmap)
{
	nlassert( bitmap );
	_ScrollRight = new CBitm( *bitmap);

	_ScrollRight->hotSpot(THotSpot::HS_ML);
	_ScrollRight->origin(THotSpot::HS_BR);
	_ScrollRight->ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
} // setRightScrollBitmap //


//-----------------------------------------------
// add :
// add a control to the list
//-----------------------------------------------
void CHorizontalList::add( CControl *pCtrl)
{
	if ( !pCtrl)
	{
		nlwarning("<CHorizontalList::add> : param (CControl*) is NULL, abort");
		return;
	}

	// if locked, bufferize the request
	if (_Locked)
	{
		_AddBuffer.push_back( pCtrl );
		return;
	}

	_Items.push_back( pCtrl );

	++_NbElts;

	// if this is the first item, init the starting point
	if ( _NbElts == 1)
		_StartingIterator = _Items.begin();

	pCtrl->hotSpot(THotSpot::HS_TR);
	pCtrl->origin(THotSpot::HS_BL);
	if (pCtrl->id() == 0)
		pCtrl->ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
} // add //


//-----------------------------------------------
// scroll :
//-----------------------------------------------
void CHorizontalList::scroll(sint32 scroll)
{
	if ( _NbElts == 0)
		return;

	// scrolling on the left
	if (scroll < 0)
	{
		const TListControl::iterator itB = _Items.begin();

		while ( ( _StartingIterator != itB) && (scroll != 0) )
		{
			--_StartingIterator;
			++scroll;
		}
	}
	// scrolling on the right
	else
	{
		const TListControl::iterator itE = --_Items.end();

		while ( ( _StartingIterator != itE) && (scroll != 0) )
		{
			++_StartingIterator;
			--scroll;
		}
	}
}// scrollH /


//-----------------------------------------------
// click :
// manage left mouse button click
//-----------------------------------------------
void CHorizontalList::click(float x, float y, bool &taken)
{
	// test click coordinates
	if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
	{
		// test arrows
		static float x1=0, y1=0, h1=0, w1=0;
		if (_ScrollLeft != NULL)
		{
			_ScrollLeft->getDisplayValues(x1, y1, h1, w1);
			if ( x >= x1 && x <= (x1+w1) && y >= y1 && y <= (y1+h1) )
			{
				scroll( -1 );
				taken = true;
				return;
			}
		}

		if (_ScrollRight != NULL)
		{
			_ScrollRight->getDisplayValues(x1, y1, h1, w1);
			if ( x >= x1 && x <= (x1+w1) && y >= y1 && y <= (y1+h1) )
			{
				scroll( +1 );
				taken = true;
				return;
			}
		}

		_Locked = true;

		// send the click to all elts
		TListControl::iterator it;

		for (it = _Items.begin() ; it != _Items.end() ; ++it)
		{
			nlassert(*it);
			(*it)->click(x,y,taken);
		}

		_Locked = false;
	}
}// click //


//-----------------------------------------------
// clickRight :
// manage right mouse button click
//-----------------------------------------------
void CHorizontalList::clickRight(float x, float y, bool &taken)
{
	// test click coordinates
	if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
	{
		_Locked = true;

		// send the click to all elts
		TListControl::iterator it, itE = _Items.end();

		for (it = _Items.begin() ; it != itE ; ++it)
		{
			nlassert(*it);
			(*it)->clickRight(x,y,taken);
		}

		_Locked = false;
	}
}// clickRight //

//-----------------------------------------------
// display :
// display the control
//-----------------------------------------------
void CHorizontalList::display()
{
	//----------------------
	// If the list need to be cleared, do it now
	if (_ClearRequest)
	{
		clear(_DeleteRequest);
		_ClearRequest = false;
	}

	// add all the elements in the add buffer
	if (!_Locked)
	{
		for( TListControl::const_iterator itbuff = _AddBuffer.begin() ; itbuff != _AddBuffer.end() ; ++itbuff)
		{
			add( (*itbuff) );
		}

		_AddBuffer.clear();
	}
	//----------------------

	// If the control is hide -> return
	if(!_Show)
		return;

	// calculate the real display size ( clip if scroll arrows are visible )
	float wDisplay, yDisplay, hDisplay, xDisplay;

	float x1=0, y1=0, h1=0, w1=0;
	if (_ScrollLeft != NULL)
		_ScrollLeft->getDisplayValues(x1, y1, h1, w1);

	float x2=0, y2=0, h2=0, w2=0;
	if (_ScrollRight != NULL)
		_ScrollRight->getDisplayValues(x2, y2, h2, w2);

	wDisplay = _W_Display - w1 - w2;

	hDisplay = _H_Display;
	yDisplay = _Y_Display;
	xDisplay = _X_Display + w1;

	/// \todo GUIGUI : initialize the scissor with oldScissor and remove tmp variables.
	// Backup scissor and create the new scissor to clip the list correctly.
	CScissor oldScissor = Driver->getScissor();
	CScissor scissor;

	float scisX, scisY, scisWidth, scisHeight;
	scisX		= oldScissor.X;
	scisY		= oldScissor.Y;
	scisWidth	= oldScissor.Width;
	scisHeight	= oldScissor.Height;

	float xtmp = xDisplay + wDisplay;
	float ytmp = yDisplay + hDisplay;
	float xscistmp = scisX + scisWidth;
	float yscistmp = scisY + scisHeight;

	if( xDisplay > scisX )
		scisX = xDisplay;
	if( yDisplay > scisY)
		scisY = yDisplay;
	if( xtmp < xscistmp)
		scisWidth = xtmp-scisX;
	else
		scisWidth = xscistmp-scisX;
	if( ytmp < yscistmp)
		scisHeight = ytmp-scisY;
	else
		scisHeight = yscistmp-scisY;

	scissor.init(scisX, scisY, scisWidth, scisHeight);
	Driver->setScissor(scissor);

// draw background if a texture is specified
	if ( _TextureId != 0)
	{
		if ( _Tiled && !_TexturePath.empty() )
		{
			uint32 w, h;
			CInterfMngr::getWindowSize(w, h);
			drawBitmapTiled( Driver, xDisplay, yDisplay, wDisplay, hDisplay, w, h, *_Texture, _TextureWidth, _TextureHeight, true, _RGBA);
		}
		else
		{
			Driver->drawBitmap(xDisplay, yDisplay, wDisplay, hDisplay, *_Texture, true, _RGBA);
		}
	}

// draw the visible controls
	float w = 0, h=0, wPixel=0, hPixel=0;

	if (_ScrollLeft != NULL)
		_ScrollLeft->getSize( w, h, wPixel, hPixel);

	float x = _X + w;
	float xPixel = _X_Pixel + wPixel;


	_NbDisplayedElts = 0 ;

	TListControl::const_iterator it, itE = _Items.end();

	//	hide all the controls
	for (it = _Items.begin() ; it != itE ; ++it)
	{
		(*it)->show( false );
		(*it)->setPosition( -1,-1,-1,-1);
	}

	for (it = _StartingIterator ; it != itE ; ++it)
	{
		(*it)->show( true );
		nlassert( *it );

		// set control position
		/// \todo Malkav: set control hotspot according to current list hotspot (but not == ), centered or not
		(*it)->hotSpot( _HotSpot );
		(*it)->setPosition( x, _Y, xPixel , _Y_Pixel);

		(*it)->display();

		(*it)->getSize(w, h, wPixel, hPixel);
		x += w;
		xPixel += wPixel;
		xPixel += _Spacing;

		++ _NbDisplayedElts;

		// if this control was clipped, stop displaying ???
		if ( x >= (scisX + scisWidth) )
			break;
	}

// draw the scoll arrows if any
	if ( _ScrollLeft != NULL || _ScrollRight != NULL)
	{
		// calculate the scissor
		wDisplay = _W_Display;
		hDisplay = _H_Display;
		yDisplay = _Y_Display;
		xDisplay = _X_Display;

		/// \todo GUIGUI : initialize the scissor with oldScissor and remove tmp variables.
		// Backup scissor and create the new scissor to clip the list correctly.
		scisX		= oldScissor.X;
		scisY		= oldScissor.Y;
		scisWidth	= oldScissor.Width;
		scisHeight	= oldScissor.Height;

		xtmp = xDisplay + wDisplay;
		ytmp = yDisplay + hDisplay;
		xscistmp = scisX + scisWidth;
		yscistmp = scisY + scisHeight;

		if( xDisplay > scisX )
			scisX = xDisplay;
		if( yDisplay > scisY)
			scisY = yDisplay;
		if( xtmp < xscistmp)
			scisWidth = xtmp-scisX;
		else
			scisWidth = xscistmp-scisX;
		if( ytmp < yscistmp)
			scisHeight = ytmp-scisY;
		else
			scisHeight = yscistmp-scisY;

		scissor.init(scisX, scisY, scisWidth, scisHeight);
		Driver->setScissor(scissor);

		// display the scroll arrows
		switch ( _HotSpot )
		{
		case THotSpot::HS_TR:
			if (_ScrollLeft != NULL )
				_ScrollLeft->setPosition( _X, _Y + _H/2, _X_Pixel, _Y_Pixel + _H_Pixel/2);

			if (_ScrollRight != NULL )
				_ScrollRight->setPosition( _X + _W, _Y + _H/2, _X_Pixel + _W_Pixel, _Y_Pixel + _H_Pixel/2);
			break;

		case THotSpot::HS_BR:
			if (_ScrollLeft != NULL )
				_ScrollLeft->setPosition( _X, _Y - _H/2, _X_Pixel, _Y_Pixel - _H_Pixel/2);

			if (_ScrollRight != NULL )
				_ScrollRight->setPosition( _X + _W, _Y - _H/2, _X_Pixel + _W_Pixel, _Y_Pixel - _H_Pixel/2);
			break;

		case THotSpot::HS_TL:
			if (_ScrollLeft != NULL )
				_ScrollLeft->setPosition( _X - _W, _Y + _H/2, _X_Pixel - _W_Pixel, _Y_Pixel + _H_Pixel/2);

			if (_ScrollRight != NULL )
				_ScrollRight->setPosition( _X, _Y + _H/2, _X_Pixel , _Y_Pixel + _H_Pixel/2);
			break;

		case THotSpot::HS_BL:
			if (_ScrollLeft != NULL )
				_ScrollLeft->setPosition( _X  - _W, _Y - _H/2, _X_Pixel - _W_Pixel, _Y_Pixel - _H_Pixel/2);

			if (_ScrollRight != NULL )
				_ScrollRight->setPosition( _X, _Y - _H/2, _X_Pixel, _Y_Pixel - _H_Pixel/2);
			break;

		case THotSpot::HS_MR:
			if (_ScrollLeft != NULL )
				_ScrollLeft->setPosition( _X, _Y, _X_Pixel, _Y_Pixel);

			if (_ScrollRight != NULL )
				_ScrollRight->setPosition( _X + _W, _Y, _X_Pixel + _W_Pixel, _Y_Pixel);
			break;

		case THotSpot::HS_ML:
			if (_ScrollLeft != NULL )
				_ScrollLeft->setPosition( _X - _W, _Y, _X_Pixel - _W_Pixel, _Y_Pixel);

			if (_ScrollRight != NULL )
				_ScrollRight->setPosition( _X, _Y, _X_Pixel, _Y_Pixel);
			break;

		case THotSpot::HS_MM:
			if (_ScrollLeft != NULL )
				_ScrollLeft->setPosition( _X - _W/2, _Y, _X_Pixel - _W_Pixel/2, _Y_Pixel);

			if (_ScrollRight != NULL )
				_ScrollRight->setPosition( _X + _W /2, _Y, _X_Pixel + _W_Pixel/2, _Y_Pixel);
			break;

		}
		if (_ScrollLeft != NULL )
			_ScrollLeft->display();
		if (_ScrollRight != NULL )
			_ScrollRight->display();
	}

	// restore scissor
	Driver->setScissor(oldScissor);
}// display //



//-----------------------------------------------
// ref :
// Set some references for the display.
//-----------------------------------------------
void CHorizontalList::ref(float x, float y, float w, float h)
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
			float xo, yo;
			calculateOrigin(xo, yo, (*it)->origin());
			(*it)->ref(xo, yo, _W_Ref, _H_Ref);
		}
	}

	// update controls in the list
	_Locked = true;

	const TListControl::iterator itEnd = _Items.end();

	for (it = _Items.begin() ; it != itEnd ; ++it)
	{
		nlassert( *it != NULL);
		if ( (*it)->id() == 0)
			(*it)->ref( x, y, w, h);
	}

	_Locked = false;

	// update scroll bitmaps if any
	if (_ScrollLeft != NULL)
	{
		_ScrollLeft->ref(x,y,w,h);
	}
	if (_ScrollRight != NULL)
	{
		_ScrollRight->ref(x,y,w,h);
	}

}// ref //