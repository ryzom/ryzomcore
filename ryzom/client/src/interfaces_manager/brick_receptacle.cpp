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
// Client
#include "brick_receptacle.h"
// 3D Interface.
#include "nel/3d/u_text_context.h"


///////////
// Using //
///////////
using namespace NL3D;

/////////////
// Externs //
/////////////
extern UTextContext *TextContext;
extern UTextContext *TextContext;
extern TMapIdToFamily FamiliesMap;

//---------------------------------------------------
// Constructor :
//---------------------------------------------------
CBrickReceptacle::CBrickReceptacle(uint id)
{
	init();
} // Constructor //


//---------------------------------------------------
// Constructor :
//---------------------------------------------------
CBrickReceptacle::CBrickReceptacle(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel,
				 uint texture, const CRGBA &rgba, uint16 family, uint numFunc )
	:CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CBitmapBase( texture, rgba)
{
	init( family, numFunc );

	TMapIdToFamily::iterator it = FamiliesMap.find( family );
	if (it != FamiliesMap.end() )
		this->texture( (*it).second->TextureId );

}

//---------------------------------------------------
// Destructor :
//---------------------------------------------------
CBrickReceptacle::~CBrickReceptacle()
{
	clear();
} // Destructor //



//---------------------------------------------------
// init :
//---------------------------------------------------
void CBrickReceptacle::init(uint16 family, uint numFunc)
{
	_Family = family;
	_Brick = NULL;
	_Locked = false;
	_RGBA = CRGBA(128,128,128,255);
	_NumFuncLeftClick = numFunc;
} // init //


//---------------------------------------------------
// init :
//---------------------------------------------------
void CBrickReceptacle::clear()
{
	if (_Brick != NULL)
	{
		delete _Brick;
		_Brick = NULL;
	}
} // clear //


//---------------------------------------------------
// brick :
//---------------------------------------------------
bool CBrickReceptacle::brick( CBrickControl *brick)
{
	if (_Locked )
		return false;

//	nlassert( brick );
	if ( !brick )
	{
		nlwarning("<CBrickReceptacle::brick> : param in NULL, abort");
		return false;
	}

	if (brick->getAssociatedBrick()->Family == _Family)
	{
		clear();

		_Brick = new CBrickControl( *brick );

		_Brick->ref( _X_Ref, _Y_Ref, _W_Ref, _H_Ref );
		_Brick->hotSpot(THotSpot::HS_MM);
		_Brick->origin(THotSpot::HS_MM);

		return true;
	}
	else
		return false;

} // brick //



//---------------------------------------------------
// family :
//---------------------------------------------------
bool CBrickReceptacle::family( uint16 family)
{
	if (_Brick != NULL)
		return false;
	else
	{
		_Family = family;
		return true;
	}

	TMapIdToFamily::iterator it = FamiliesMap.find( family );
	if (it != FamiliesMap.end() )
		texture( (*it).second->TextureId );
} // family //


//-----------------------------------------------
// ref :
// Set some references for the display.
//-----------------------------------------------
void CBrickReceptacle::ref(float x, float y, float w, float h)
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

	// update brick
	if ( _Brick != NULL )
	{
		_Brick->ref( x, y, w, h);
	}

}


//---------------------------------------------------
// display :
//---------------------------------------------------
void CBrickReceptacle::display()
{
	if (!_Show)
		return;

	// draw the brick control if any
	if (_Brick != NULL)
	{
		_Brick->setPosition(_X + _W/2, _Y + _H/2, _X_Pixel + _W_Pixel/2, _Y_Pixel + _H_Pixel/2 );
		_Brick->display();
	}
	else
	{
			// draw the slot background
		if( _Texture )
		{
			Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *_Texture, true, _RGBA);
		}
		else
		{
			// Draw a quad
			Driver->drawQuad(_X_Display, _Y_Display, _X_Display+_W_Display, _Y_Display+_H_Display, CRGBA(255,0,255));
			// Draw the debug msg.
			TextContext->setShaded(true);
			TextContext->setFontSize(10);
			TextContext->setColor(CRGBA(255,255,255));
			TextContext->setHotSpot(UTextContext::BottomLeft);
			TextContext->printfAt(_X_Display, _Y_Display, "%d Miss", texture);
		}
	}
} // display //



//-----------------------------------------------
// click :
// Manage the click of the mouse for the control.
//-----------------------------------------------
void CBrickReceptacle::click(float x, float y, bool &taken)
{
	// If the button is enabled and taken is false
	if( taken == true || _Show == false || _Locked == true )
		return;

	// test click ccordinates
	if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
	{
		if ( _NumFuncLeftClick != 0)
		{
			CInterfMngr::runFuncCtrl( _NumFuncLeftClick, id(), this);
		}
		else
		{
			clear();
		}
		taken = true;
	}
}// click //