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


//////////////
// Includes //
//////////////
// Misc.

// Net.

// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"

// Client.
#include "button.h"
#include "interfaces_manager.h"


///////////
// Using //
///////////
using namespace std;
using namespace NL3D;


/////////////
// Externs //
/////////////
extern UDriver		*Driver;
extern UTextContext *TextContext;


/////////////
// Globals //
/////////////


///////////////
// Functions //
///////////////
//-----------------------------------------------
// CButton :
// Constructor.
//-----------------------------------------------
CButton::CButton(uint id)
: CControl(id)
{
	init(0, 0, 0);
}// CButton //

//-----------------------------------------------
// CButton :
// Constructor.
//-----------------------------------------------
CButton::CButton(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR, uint numFuncD, const CButtonBase &buttonBase)
: CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CButtonBase(buttonBase)
{
	init(numFuncOn, numFuncR, numFuncD);
}// CButton //

//-----------------------------------------------
// CButton :
// Constructor.
//-----------------------------------------------
CButton::CButton(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR, uint numFuncD, CRGBA on, CRGBA off, CRGBA disable)
: CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CButtonBase(on, off, disable)
{
	init(numFuncOn, numFuncR, numFuncD);
}// CButton //


//-----------------------------------------------
// init :
// Initialize the button (1 function called for all constructors -> easier).
//-----------------------------------------------
void CButton::init(uint numFuncOn, uint numFuncR, uint numFuncD)
{
	_NumFuncOn			= numFuncOn;
	_NumFuncRightClick	= numFuncR;
	_NumFuncDbleClick	= numFuncD;
	_State				= released;
	_Text.clear();
}// init //

//-----------------------------------------------
// pen :
// Set the Pen to use for the Text of the button.
//-----------------------------------------------
void CButton::pen(const CPen &pen)
{
	_Pen = pen;
}// pen //

//-----------------------------------------------
// unSelect :
// unselect the button.
//-----------------------------------------------
void CButton::unSelect()
{
	CButtonBase::unSelect();
	_State = released;
}// push //


//-----------------------------------------------
// display :
// Display the Button.
//-----------------------------------------------
void CButton::display()
{
	// If the control is hide -> return
	if(!_Show)
		return;

	TBG mode;
	uint texture;
	CRGBA color;

	if(_Enable)
	{
		if(_On)
		{
			mode = _BGModeOn;
			texture = _TextureOn;
			color = _ColorOn;
		}
		else
		{
			mode = _BGModeOff;
			texture = _TextureOff;
			color = _ColorOff;
		}
	}
	else
	{
		mode = _BGModeDisable;
		texture = _TextureDisable;
		color = _ColorDisable;
	}

	switch(mode)
	{
	case BG_plain:
		Driver->drawQuad(_X_Display, _Y_Display, _X_Display+_W_Display, _Y_Display+_H_Display, color);
		break;

	case BG_stretch:
		{
		UTextureFile *utexture = CInterfMngr::getTexture(texture);
		if(utexture)
		{
			Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *utexture, true, color);
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
		break;
	}

	// Display the Text of the Button.
	if(!_Text.empty())
	{
		TextContext->setShaded(_Pen.shadow());
		TextContext->setFontSize(_Pen.fontSize());
		TextContext->setColor(_Pen.color());
		TextContext->setHotSpot(UTextContext::MiddleMiddle);
		TextContext->printAt(_X_Display+_W_Display/2, _Y_Display+_H_Display/2, _Text);
	}
}// display //

//-----------------------------------------------
// click :
// Manage the click of the mouse for the Button.
//-----------------------------------------------
void CButton::click(float x, float y, bool &taken)
{
	// If the button is enabled and taken is false
	if(_Enable && (!taken) )
	{
		// test click ccordinates
		if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
		{
			/// \todo Malkav: TO DO : SIMULATE DOUBLE CLICK !!!!!!! SHOULD MAKE A REAL DOUBLE CLICK EVENT
			if ( (_State == left_clicked) && (_NumFuncDbleClick != 0) )
			{
				CButtonBase::select();
				_State = double_clicked;
				CInterfMngr::runFuncCtrl(_NumFuncDbleClick, id());
				taken = true;
			}
			else if (_NumFuncOn != 0)
			{
				CButtonBase::select();
				_State = left_clicked;
				CInterfMngr::runFuncCtrl(_NumFuncOn, id());
				taken = true;
			}
		}
	}
}// click //


//-----------------------------------------------
// click :
// Manage the click of the mouse for the Button.
//-----------------------------------------------
void CButton::clickRight(float x, float y, bool &taken)
{
		// If the button is enabled and taken is false
	if(_Enable && (!taken) && (_NumFuncRightClick != 0) )
	{
		// test click ccordinates
		if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
		{
			CButtonBase::select();
			_State = right_clicked;
			CInterfMngr::runFuncCtrl(_NumFuncRightClick, id());
			taken = true;
		}
	}
}// click //
