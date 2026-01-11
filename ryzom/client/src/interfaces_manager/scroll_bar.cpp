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

#include "nel/3d/u_driver.h"
#include "interfaces_manager.h"
#include "scroll_bar.h"
#include "scrollable_control.h"


#include "interf_list.h"


/////////////
// Externs //
/////////////
extern NL3D::UDriver		*Driver;


/*
 * default Constructor
 */
CScrollBar::CScrollBar(uint id)
: CControl(id)
{
	init();
}

/*
 * Constructor
 */
CScrollBar::CScrollBar(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, bool vertical, CScrollableControl *ctrl)
: CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel)
{
	init();

	_Vertical = vertical;
	_Parent = ctrl;

	uint32 wndW,wndH;
	CInterfMngr::getWindowSize( wndW, wndH);

	if (wndH == 0)
	{
		_ButtonDisplaySize = 0;
		return;
	}

	if (_Vertical)
		_ButtonDisplaySize = float(w_pixel) / wndH +  w * _H_Ref; // h_button = w_control
	else
		_ButtonDisplaySize = float(h_pixel) / wndH +  h * _W_Ref; // w_button = h_control
}


void CScrollBar::init()
{
	_LeftTextureOn = 2;
	_LeftTextureOff = 2;

	_RightTextureOn = 2;
	_RightTextureOff = 2;

	_UpTextureOn = 2;
	_UpTextureOff = 2;

	_DownTextureOn = 2;
	_DownTextureOff = 2;

	_TextureOn = 0;
	_TextureDisable = 0;
	_ColorOn = CRGBA(255,255,255,255);
	_ColorDisable = CRGBA(100,100,100,255);
	_Enable = true;
	_Show = false;
}



/*
 * display
 */
void CScrollBar::display()
{
	// If the control is hide -> return
	if(!_Show)
		return;

	if(_Enable)
	{
		// body
		Driver->drawBitmap( _X_Display, _Y_Display, _W_Display, _H_Display, *CInterfMngr::getTexture(_TextureOn), true, _ColorOn);
		// buttons
		if (_Vertical)
		{
			// down
			Driver->drawBitmap( _X_Display, _Y_Display, _W_Display, _ButtonDisplaySize, *CInterfMngr::getTexture( _DownTextureOn ), true, _ColorOn);
			// up
			Driver->drawBitmap( _X_Display, _Y_Display + _H_Display - _ButtonDisplaySize, _W_Display, _ButtonDisplaySize , *CInterfMngr::getTexture( _UpTextureOn ), true, _ColorOn);
		}
		// horizontal
		else
		{
			// left
			Driver->drawBitmap( _X_Display, _Y_Display, _ButtonDisplaySize, _H_Display , *CInterfMngr::getTexture( _LeftTextureOn ), true, _ColorOn);
			// right
			Driver->drawBitmap( _X_Display + _W_Display - _ButtonDisplaySize, _Y_Display, _ButtonDisplaySize, _H_Display, *CInterfMngr::getTexture( _RightTextureOn ), true, _ColorOn);
		}
	}
	//disable
	else
	{
		// body
		Driver->drawBitmap( _X_Display, _Y_Display, _W_Display, _H_Display, *CInterfMngr::getTexture(_TextureDisable), true, _ColorDisable);
		// buttOffs
		if (_Vertical)
		{
			// down
			Driver->drawBitmap( _X_Display, _Y_Display, _W_Display, _ButtonDisplaySize, *CInterfMngr::getTexture( _DownTextureOff ), true, _ColorDisable);
			// up
			Driver->drawBitmap( _X_Display, _Y_Display + _H_Display - _ButtonDisplaySize, _W_Display, _ButtonDisplaySize , *CInterfMngr::getTexture( _UpTextureOff ), true, _ColorDisable);
		}
		// horizOfftal
		else
		{
			// left
			Driver->drawBitmap( _X_Display, _Y_Display, _ButtonDisplaySize, _H_Display , *CInterfMngr::getTexture( _LeftTextureOff ), true, _ColorDisable);
			// right
			Driver->drawBitmap( _X_Display + _W_Display - _ButtonDisplaySize, _Y_Display, _ButtonDisplaySize, _H_Display, *CInterfMngr::getTexture( _RightTextureOff ), true, _ColorDisable);
		}
	}

// draw the scroll bar body (arrows + bar)


	// draw the slider
		// TO DO
}


/*
 * click
 */
void CScrollBar::click(float x, float y, bool &taken)
{

	if( _Enable && (!taken))
	{
		// click into the control
		if ( (x >= _X_Display) && ( x <= (_X_Display + _W_Display) ) && (y >= _Y_Display) && (y <= (_Y_Display+_H_Display) )  )
		{
			if ( _Vertical == true)
			{
				// click into the upper arrow
				if ( y >= (_Y_Display + _H_Display - _ButtonDisplaySize) )
				{
					dynamic_cast<CScrollableControl*> (_Parent)->scrollV( 1 );
					taken = true;
				}
				// click into the down arrow
				if ( y <= (_Y_Display + _ButtonDisplaySize) )
				{
					dynamic_cast<CScrollableControl*> (_Parent)->scrollV( -1 );
					taken = true;
				}

				// SLIDER :  TO DO
			}
			// horizontal
			else
			{
				// click into the right arrow
				if ( x >= (_X_Display + _W_Display - _ButtonDisplaySize) )
				{
					dynamic_cast<CScrollableControl*> (_Parent)->scrollH( 1 );
					taken = true;
				}
				// click into the left arrow
				if ( x <= (_X_Display + _ButtonDisplaySize) )
				{
					dynamic_cast<CScrollableControl*> (_Parent)->scrollH( -1 );
					taken = true;
				}

				// SLIDER :  TO DO
			}
		}
	}
}




/*
 * textureOn
 */
void CScrollBar::textureOn(uint32 texture)
{
	_TextureOn = texture;
}// textureOn //


/*
 * textureDisable
 */
void CScrollBar::textureDisable(uint32 texture)
{
	_TextureDisable = texture;
}// textureDisable //


/*
 * colorOn
 */
void CScrollBar::colorOn(const NLMISC::CRGBA &color)
{
	_ColorOn = color;
}// colorOn //

/*
 * colorDisable
 */
void CScrollBar::colorDisable(const NLMISC::CRGBA &color)
{
	_ColorDisable = color;
}// colorDisable //


/*
 * enable
 */
bool CScrollBar::enable()
{
	return _Enable;
}// enable //

/*
 * enable
 */
void CScrollBar::enable(bool e)
{
	_Enable = e;
}// enable //



/*
 * size
 */
float CScrollBar::size() const
{
	if (_Vertical)
		return _W_Display;
	else
		return _H_Display;
}// size //

