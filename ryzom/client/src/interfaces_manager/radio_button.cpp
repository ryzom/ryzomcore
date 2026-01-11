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

// Misc
#include "nel/misc/debug.h"
// Client
#include "radio_button.h"
#include "radio_controller.h"
#include "interfaces_manager.h"



///////////////
// Functions //
///////////////

//-----------------------------------------------
// CRadioButton :
// Default Constructor.
//-----------------------------------------------
CRadioButton::CRadioButton(uint id)
: CButton(id)
{
	_Controller = NULL;
}// CRadioButton //

//-----------------------------------------------
// CRadioButton :
// Constructor.
//-----------------------------------------------
CRadioButton::CRadioButton(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR, uint numFuncD, const CButtonBase &buttonBase)
: CButton(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, numFuncOn, numFuncR, numFuncD, buttonBase)
{
	_Controller = NULL;
}// CRadioButton //

//-----------------------------------------------
// CRadioButton :
// Constructor.
//-----------------------------------------------
CRadioButton::CRadioButton(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFuncOn, uint numFuncR, uint numFuncD, CRGBA on, CRGBA off, CRGBA disable)
: CButton(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, numFuncOn, numFuncR, numFuncD, on, off, disable)
{
	_Controller = NULL;
}// CRadioButton //



//-----------------------------------------------
// radioClick :
//-----------------------------------------------
void CRadioButton::radioClick(float x, float y, bool &taken)
{
		// If the button is enabled and taken is false
	if(_Enable && (!taken) )
	{
		// test click coordinates
		if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
		{
			// TO DO : SIMULATE DOUBLE CLICK !!!!!!! SHOULD MAKE A REAL DOUBLE CLICK EVENT
			if ( (_State == left_clicked) && (_NumFuncDbleClick != 0) )
			{
				select();
				_State = double_clicked;
				CInterfMngr::runFuncCtrl(_NumFuncDbleClick, id());
				taken = true;
			}
			else if (_NumFuncOn != 0)
			{
				select();
				_State = left_clicked;
				CInterfMngr::runFuncCtrl(_NumFuncOn, id());
				taken = true;
			}
		}
	}
}// radioClick //


//-----------------------------------------------
// click :
//-----------------------------------------------
void CRadioButton::click(float x, float y, bool &taken)
{
	// if this button is controlled by a radio controller, do nothing,
	// if it's not controlled, call radioClick
	if ( _Controller == NULL)
	{
		radioClick(x,y,taken);
	}
}// click //

//-----------------------------------------------
// setController :
//-----------------------------------------------
void CRadioButton::setController(CRadioController *controller)
{
	nlassert( controller );
	_Controller = controller;
}// setController //


//-----------------------------------------------
// select :
//-----------------------------------------------
void CRadioButton::select()
{
	if( _Controller )
	{
		_Controller->unselectAll();
	}

	CButton::select();
}// select //