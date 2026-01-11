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
// Net.
// 3D Interface.
#include "nel/3d/u_driver.h"

// Client.
#include "bitmap.h"
#include "interfaces_manager.h"
#include "graphic.h"

///////////
// Using //
///////////
using namespace NL3D;
using namespace std;


/////////////
// Externs //
/////////////
extern UDriver	*Driver;


/////////////
// Globals //
/////////////


///////////////
// Functions //
///////////////
//-----------------------------------------------
// CBitm :
// Constructor.
//-----------------------------------------------
CBitm::CBitm(uint id)
: CControl(id)
{
	init();
}// CBitm //

//-----------------------------------------------
// CBitm :
// Constructor.
//-----------------------------------------------
CBitm::CBitm(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, const CBitmapBase &bitmapBase)
: CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CBitmapBase(bitmapBase)
{
	init();
}// CBitm //

//-----------------------------------------------
// CBitm :
// Constructor.
//-----------------------------------------------
CBitm::CBitm(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint texture, const CRGBA &rgba)
: CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CBitmapBase(texture, rgba)
{
	init();
}// CBitm //


//-----------------------------------------------
// init :
// Initialize the button (1 function called for all constructors -> easier).
//-----------------------------------------------
void CBitm::init()
{
}// init //



//-----------------------------------------------
// display :
// Display the Button.
//-----------------------------------------------
void CBitm::display()
{
	// If the control is hide -> return
	if(!_Show)
		return;

	// Draw the Bitmap.
	if ( _Tiled && !_TexturePath.empty() )
	{
		uint32 w, h;
		CInterfMngr::getWindowSize(w, h);
		drawBitmapTiled( Driver,_X_Display, _Y_Display, _W_Display, _H_Display, w, h, *_Texture, _TextureWidth, _TextureHeight, true, _RGBA);
	}
	else if (_Texture != NULL)
	{
		Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *_Texture, true, _RGBA);
	}
	else
	{
		Driver->drawQuad(_X_Display, _Y_Display, _W_Display, _H_Display, _RGBA);
	}

//	Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *texture, true, _RGBA);
}// display //

