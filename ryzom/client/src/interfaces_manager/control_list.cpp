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
#include "control_list.h"
#include "interfaces_manager.h"


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
// Constructor
//-----------------------------------------------
CControlList::CControlList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint16 spacing)
:CScrollableControl( id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel)
{
	init(spacing);
} // Constructor //


//-----------------------------------------------
// Destructor
//-----------------------------------------------
CControlList::~CControlList()
{
	clear();
} // Destructor //


//-----------------------------------------------
// add :
//-----------------------------------------------
void CControlList::init(uint16 spacing)
{
	_EndingIterator = _Items.rbegin();
	_NbElts = 0;

	_Spacing = spacing;
	_Locked = false;
	_ClearRequest = false;
	_DeleteRequest = false;

	vScroll(true);
}// init //




//-----------------------------------------------
// add :
//-----------------------------------------------
void CControlList::add( CControl *pCtrl, bool pushFront)
{
	if ( !pCtrl)
	{
		nlwarning("<CControlList::add> : param (CControl*) is NULL, abort");
		return;
	}

	// if locked, bufferize the request
	if (_Locked)
	{
		if (pushFront)
			_AddBuffer.push_front( pCtrl );
		else
			_AddBuffer.push_back( pCtrl );

		return;
	}

	if ( pushFront )
		_Items.push_front( pCtrl );
	else
		_Items.push_back( pCtrl );

	++_NbElts;

	// if this is the first item or inserted in front, init the starting point
	if ( _NbElts == 1 || pushFront )
		_EndingIterator = _Items.rbegin();

	if (pCtrl->id() == 0)
		pCtrl->ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref);

	pCtrl->hotSpot(THotSpot::HS_TR);
	pCtrl->origin(THotSpot::HS_BL);
}// add //


//-----------------------------------------------
// clear :
// clear the list, delete the elts if specified
//-----------------------------------------------
void CControlList::clear(bool deleteElts)
{
	if (_Locked)
	{
		_ClearRequest = true;
		_DeleteRequest = deleteElts;
		return;
	}

	if (deleteElts == true)
	{
		TListControl::iterator it, itE = _Items.end();

		for (it = _Items.begin() ; it != itE ; ++it)
		{
			if ( *it != NULL && (*it)->id() == 0 )
			{
				delete (*it);
				(*it) = NULL;
			}
		}
	}

	_Items.clear();
	_EndingIterator = _Items.rbegin();
	_NbElts = 0;
	_NbDisplayedElts = 0;

	_ClearRequest = false;
	_DeleteRequest = false;
} // clear  //

//-----------------------------------------------
// click :
// manage left mouse button click
//-----------------------------------------------
void CControlList::click(float x, float y, bool &taken)
{
	_Locked = true;

	if (taken)
		return;

	float w = 0;

	if (_VScroll != NULL)
	{
		_VScroll->click(x,y,taken);
		w = _VScroll->size();
	}

	// test click coordinates
	if(x>=_X_Display && x<=(_X_Display + _W_Display - w) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
	{
		// send the click to all elts
		TListControl::iterator it;

		for (it = _Items.begin() ; it != _Items.end() ; ++it)
		{
			nlassert(*it);
			(*it)->click(x,y,taken);
		}
	}

	_Locked = false;
}// click //


//-----------------------------------------------
// clickRight :
// manage right mouse button click
//-----------------------------------------------
void CControlList::clickRight(float x, float y, bool &taken)
{
	_Locked = true;

	if (taken)
		return;

	float w = 0;

	if (_VScroll != NULL)
	{
		_VScroll->clickRight(x,y,taken);
		w = _VScroll->size();
	}


	// test click coordinates
	if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
	{
		// send the click to all elts
		TListControl::iterator it, itE = _Items.end();

		for (it = _Items.begin() ; it != itE ; ++it)
		{
			nlassert(*it);
			(*it)->clickRight(x,y,taken);
		}

	}

	_Locked = false;
}// clickRight //

//-----------------------------------------------
// scrollH :
//-----------------------------------------------
void CControlList::scrollH(sint32 scroll)
{
}// scrollH //




//-----------------------------------------------
// scrollV :
//-----------------------------------------------
void CControlList::scrollV(sint32 scroll)
{
	if ( this->_NbElts == 0)
		return;

	// scrolling downward
	if (scroll < 0)
	{
		const TListControl::reverse_iterator itB = _Items.rbegin();

		while ( ( _EndingIterator != itB) && (scroll != 0) )
		{
			--_EndingIterator;
			++scroll;
		}
	}
	// scrolling upward
	else
	{
		const TListControl::reverse_iterator itE = --_Items.rend();

		while ( ( _EndingIterator != itE) && (scroll != 0) )
		{
			++_EndingIterator;
			--scroll;
		}
	}
}// scrollV //



//-----------------------------------------------
// display :
//-----------------------------------------------
void CControlList::display()
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

	/// \todo GUIGUI : if the scissor is corrected -> draw the bitmap after the scissor.
	// calculate the real display size ( clip if scroll bar is visible )
	float wDisplay, yDisplay, hDisplay;

	if (_VScroll->show() )
		wDisplay = _W_Display - _VScroll->size();
	else
		wDisplay = _W_Display;

	if (_HScroll->show() )
	{
		 hDisplay = _H_Display - _HScroll->size();
		 yDisplay = _Y_Display + _HScroll->size();
	}
	else
	{
		 hDisplay = _H_Display;
		 yDisplay = _Y_Display;
	}


	/// \todo GUIGUI : initialize the scissor with oldScissor and remove tmp variables.
	// Backup scissor and create the new scissor to clip the list correctly.
	CScissor oldScissor = Driver->getScissor();
	CScissor scissor;

	float scisX, scisY, scisWidth, scisHeight;
	scisX		= oldScissor.X;
	scisY		= oldScissor.Y;
	scisWidth	= oldScissor.Width;
	scisHeight	= oldScissor.Height;

	float xtmp = _X_Display+_W_Display;
	float ytmp = _Y_Display+_H_Display;
	float xscistmp = scisX+scisWidth;
	float yscistmp = scisY+scisHeight;
	if(_X_Display>scisX)
		scisX = _X_Display;
	if(_Y_Display>scisY)
		scisY = _Y_Display;
	if(xtmp<xscistmp)
		scisWidth = xtmp-scisX;
	else
		scisWidth = xscistmp-scisX;
	if(ytmp<yscistmp)
		scisHeight = ytmp-scisY;
	else
		scisHeight = yscistmp-scisY;
	scissor.init(scisX, scisY, scisWidth, scisHeight);
	Driver->setScissor(scissor);

// draw background
// ONLY FOR DEBUG
/*	UTextureFile *utexture = CInterfMngr::getTexture(1);
	if(utexture)
	{
		Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *utexture, true);
	}

	for (TListControl::iterator itItem = _Items.begin() ; itItem != _Items.end() ; ++itItem)
	{
		(*itItem)->show( false );
		(*itItem)->setPosition( -1,-1,-1,-1);
	}
*/
// draw the visible controls
	float y = _Y;
	float yPixel = _Y_Pixel;

	float w = 0, h = 0, wPixel = 0, hPixel = 0;

	_NbDisplayedElts = 0 ;

	TListControl::reverse_iterator it, itE = _Items.rend();

	for (it = _EndingIterator ; it != itE ; ++it)
	{
		//nlassert( *it );
		if ( *it == NULL)
		{
			nlwarning( "CControlList : found a NULL control in the list");
			continue;
		}

		(*it)->show( true );

		(*it)->setPosition( _X, y, _X_Pixel , yPixel);
		(*it)->display();

		(*it)->getSize(w, h, wPixel, hPixel);
		y += h;
		yPixel += hPixel;
		yPixel += _Spacing;

		++ _NbDisplayedElts;

		// if this control was clipped, stop displaying
		if ( y >= (scisY + scisHeight) )
			break;
	}

	// draw scrollBar
	_VScroll->display();

	// restore scissor
	Driver->setScissor(oldScissor);
}// display //




//-----------------------------------------------
// ref :
// Set some references for the display.
//-----------------------------------------------
void CControlList::ref(float x, float y, float w, float h)
{
	_Locked = true;

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

	// update controls in the list
	const TListControl::iterator itEnd = _Items.end();

	for (it = _Items.begin() ; it != itEnd ; ++it)
	{
		nlassert( *it != NULL);
		if ( (*it)->id() == 0)
			(*it)->ref( x, y, w, h);
	}

	_Locked = false;

}// ref //