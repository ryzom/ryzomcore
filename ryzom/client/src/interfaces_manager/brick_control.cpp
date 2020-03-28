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

// Client
#include "brick_control.h"
#include "interfaces_manager.h"
#include "time_client.h"
// 3D Interface.
#include "nel/3d/u_text_context.h"



////////////////
// Namespaces //
////////////////
using namespace NL3D;


/////////////
// Externs //
/////////////
extern UTextContext *TextContext;
extern sint64	T1;			// Time for the current frame.


//-----------------------------------------------
// constructor :
//-----------------------------------------------
CBrickControl::CBrickControl(uint id)
{
	init();
} // constructor //


/*
//-----------------------------------------------
// copy constructor :
//-----------------------------------------------
CBrickControl::CBrickControl( const CBrickControl &b)
{
	_AssociatedBrick = b._AssociatedBrick;
	_RGBA = b._RGBA;
	_NumFuncOn = b._NumFuncOn;

	/// TO DO !!!
} // copy constructor //
*/


//-----------------------------------------------
// constructor :
//-----------------------------------------------
CBrickControl::CBrickControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR )
					:CControl( id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel)
{
	init(numFuncOn, numFuncR);
} // constructor //

//-----------------------------------------------
// constructor :
//-----------------------------------------------
void CBrickControl::init(uint numFuncOn, uint numFuncR)
{
	_AssociatedBrick = NULL;
	_RGBA = CRGBA(255,255,255,255);
	_NumFuncOn = numFuncOn;
	_NumFuncR = numFuncR;
} // constructor //



//-----------------------------------------------
// display :
//-----------------------------------------------
void CBrickControl::display()
{	// If the control is hide -> return
	if(!_Show)
		return;

	if (_AssociatedBrick == NULL)
		return;

	if (_AssociatedBrick->getTexture() == NULL)
	{
		Driver->drawQuad(_X_Display, _Y_Display, _W_Display, _H_Display, _RGBA);
	}
	else
	{
		// Draw the Bitmap.

		//if brick is latent, grey the bitmap
		if ( _AssociatedBrick->LatencyEndDate > T1)
		{
			Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *CInterfMngr::getTexture(_AssociatedBrick->getTexture()), true, CRGBA(128,128,128,255));
		}
		else
		{
			Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *CInterfMngr::getTexture(_AssociatedBrick->getTexture()), true, _RGBA);
		}
	}

	// write the Text of the brick on the lower right corner of the bitmap
	TextContext->setShaded( false );
	if ( _AssociatedBrick->LatencyEndDate > T1)
	{
		TextContext->setColor( CRGBA(128,128,128,255) );
	}
	else
	{
		TextContext->setColor( CRGBA(255,255,255,255) );
	}

	TextContext->setFontSize( 12 );
	TextContext->setHotSpot(UTextContext::BottomRight);
	TextContext->printAt( _X_Display + _W_Display, _Y_Display, ucstring(_AssociatedBrick->Text) );
//	TextContext->printAt( _X_Display + _W_Display/2, _Y_Display + _H_Display/2, ucstring(_AssociatedBrick->Text) );
} // display //



//-----------------------------------------------
// click :
// manage left mouse button click
//-----------------------------------------------
void CBrickControl::click(float x, float y, bool &taken)
{
	if(!taken)
	{
		// test click ccordinates
		if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
		{
			if (_NumFuncOn != 0)
			{
				CInterfMngr::runFuncCtrl(_NumFuncOn, id(), this);
				taken = true;
			}
		}
	}
}// click //


//-----------------------------------------------
// clickRight :
// manage right mouse button click
//-----------------------------------------------
void CBrickControl::clickRight(float x, float y, bool &taken)
{
	if(!taken)
	{
		// test click ccordinates
		if( x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display) )
		{
			if (_NumFuncR != 0)
			{
				CInterfMngr::runFuncCtrl(_NumFuncR, id(), this);
				taken = true;
			}
		}
	}
}// clickRight //