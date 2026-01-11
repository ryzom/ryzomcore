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
#include "nel/misc/path.h"
// 3D Interface.
#include "nel/3d/u_driver.h"
// Client.
#include "osd.h"
#include "text.h"
#include "capture.h"
#include "button.h"
#include "radio_button.h"
#include "radio_controller.h"
#include "bitmap.h"
#include "interfaces_manager.h"
#include "interf_list.h"
#include "multi_list.h"
#include "interf_script.h"
#include "chat_control.h"
#include "chat_input.h"
#include "choice_list.h"
#include "candidate_list.h"
#include "horizontal_list.h"
#include "control_list.h"
#include "spell_list.h"
#include "progress_bar.h"
#include "casting_bar.h"
#include "brick_control.h"
// Std
#include <string>


///////////
// Using //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;


/////////////
// Externs //
/////////////
extern UDriver *Driver;
extern UTextContext *TextContext;


static std::list< uint16 > elts;


///////////
// MACRO //
///////////
// Macro pour toute la partie commune de lecture des scripts pour les controls.
#define CONTROL_SCRIPT_MACRO(UserScript)	\
											\
CControl::THotSpot hs = CControl::HS_MM;	\
CControl::THotSpot origin = CControl::HS_MM;\
uint idParent	= 0;						\
float x			= 0.f;						\
float y			= 0.f;						\
float xPixel	= 0.f;						\
float yPixel	= 0.f;						\
float w			= 0.f;						\
float h			= 0.f;						\
float wPixel	= 0.f;						\
float hPixel	= 0.f;						\
											\
elts.clear();								\
char	delimiter[] = "[] \t";				\
char *ptr = strtok(NULL, delimiter);		\
while(ptr != NULL)							\
{											\
	if(strcmp(ptr, "HotSpot:") == 0)		\
		hs = getHotSpot();					\
	else if(strcmp(ptr, "Origin:") == 0)	\
		origin = getHotSpot();				\
	else if(strcmp(ptr, "Parent:") == 0)	\
		idParent = getInt();				\
	else if(strcmp(ptr, "X:") == 0)			\
		x = getFloat();						\
	else if(strcmp(ptr, "Y:") == 0)			\
		y = getFloat();						\
	else if(strcmp(ptr, "X_Pixel:") == 0)	\
		xPixel = getFloat();				\
	else if(strcmp(ptr, "Y_Pixel:") == 0)	\
		yPixel = getFloat();				\
	else if(strcmp(ptr, "W:") == 0)			\
		w = getFloat();						\
	else if(strcmp(ptr, "H:") == 0)			\
		h = getFloat();						\
	else if(strcmp(ptr, "W_Pixel:") == 0)	\
		wPixel = getFloat();				\
	else if(strcmp(ptr, "H_Pixel:") == 0)	\
		hPixel = getFloat();				\
											\
	UserScript								\
											\
	ptr = strtok(NULL, delimiter);			\
}




//-----------------------------------------------
// COSD :
// Constructor.
//-----------------------------------------------
COSD::COSD(bool popUp)
{
	// Common init for all constructors.
	init(0, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.0f, 0.0f, popUp);
}// COSD //

//-----------------------------------------------
// COSD :
// Constructor.
//-----------------------------------------------
COSD::COSD(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, float minWidth, float minHeight, bool popUp)
{
	// Common init for all constructors.
	init(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, minWidth, minHeight, popUp);
}// COSD //


//-----------------------------------------------
// init :
// Initialize the OSD (1 function called for all constructors -> easier).
//-----------------------------------------------
void COSD::init(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, float minWidth, float minHeight, bool popUp)
{
	_Id = id;
	_PopUp = popUp;

	// pop up are never resizable
	if (_PopUp == true)
		_OSD_Mode = TMode::locked;

	// Default is OSD visible.
	_Show = true;

	_W_Min		= minWidth;	// minimal width of the OSD
	_H_Min		= minHeight; // minimal height of the OSD

	_X			= x;		// Position X of the OSD (between 0-1).
	_Y			= y;		// Position Y of the OSD (between 0-1).
	_X_Pixel	= x_pixel;	// Position X of the OSD (in Pixel).
	_Y_Pixel	= y_pixel;	// Position Y of the OSD (in Pixel).

	_W			= w;		// Width  of the OSD (between 0-1).
	_H			= h;		// Height of the OSD (between 0-1).
	_W_Pixel	= w_pixel;	// Width  of the OSD (in Pixel).
	_H_Pixel	= h_pixel;	// Height of the OSD (in Pixel).

	// How to display the OSD.
	_OSD_Mode	= none;
	// Name of the OSD;
	_OSD_Name	= ucstring("");

	// NO Background
	_BG_Mode	= BG_none;

	// NO Title Bar
	_TB_Mode	= TB_none;
	// Height of the move zone.
	_TB_H = 20;
	// Pen for the OSD Name.
	_TB_Pen	= CPen(15, CRGBA(255,0,150,255), true);

	// HighLight size in pixel.
	_HL_Size = 1;

	// Resize
	_RS_Mode = no_resize;
	// Color of the Move zone.
	_RS_Color = CRGBA(200,200,200,128);
	// Width of the resize zone.
	_RS_Size = 4;

	// NO update Function.
	_OSD_Update_Func = 0;

	// Calculate others variables.
//	calculateDisplay(); // done into the 'resize' method, called by the 'open' method

}// init //

//-----------------------------------------------
// ~COSD :
// Destructor.
//-----------------------------------------------
COSD::~COSD()
{
	// Get all the controls and destroy them.
	for(TMapControls::iterator it = _Controls.begin(); it != _Controls.end(); it++)
	{
		// Delete the control if still allocated.
		if((*it).second != NULL)
		{
			delete ((*it).second);
			(*it).second = 0;
		}
	}

	// Clear all the control.
	_Controls.clear();
	_Children.clear();
}// ~COSD //


//-----------------------------------------------
// display :
// Display the OSD.
//-----------------------------------------------
void COSD::display()
{
	// Is the OSD is not visible -> return;
	if(!_Show)
		return;

	// Draw the background
	drawBG();

	// Backup scissor and change scissor to clip the list correctly.
	CScissor oldScissor = Driver->getScissor();
	CScissor scissor;
	scissor.init(_X_Display, _Y_Display, _W_Display, _H_Display);
	Driver->setScissor(scissor);

	// Draw controls in the OSD.
	for(TListControl::reverse_iterator it = _Children.rbegin(); it != _Children.rend(); it++)
		(*it)->display();

	// Restore Scissor.
	Driver->setScissor(oldScissor);

	// How to display the OSD.
	switch(_OSD_Mode)
	{
	// OSD HighLighted
	case selected:
		// Draw Borders.
		drawBorders(_HL_W, _HL_H, _X_Display, _Y_Display, _X_Display+_W_Display, _Y_Display+_H_Display, _HL_Color);
		break;

	// OSD with a Border to resize.
	case resizable:
		// Draw big Borders.
		drawBorders(_RS_W, _RS_H, _X_Display, _Y_Display, _X_Display+_W_Display, _Y_Display+_H_Display, _RS_Color);
		break;

	// OSD with a Border and an Area to move the OSD.
	case movable:
		// Draw the Title Bar.
		drawTB();

		// Draw Big Borders.
		drawBorders(_RS_W, _RS_H, _X_Display, _Y_Display, _X_Display+_W_Display, _Y_Display+_H_Display, _RS_Color);
		// Draw the OSD Name -> if the text is not empty.
		if(!_OSD_Name.empty())
			drawText(_X_Display+_W_Display/2, _Y_Display+_H_Display-_RS_H-_TB_H_Display/2, _OSD_Name, _TB_Pen);
		break;
	}
}// display //


//-----------------------------------------------
// drawBG :
// Function to draw the background according to the mode.
//-----------------------------------------------
void COSD::drawBG()
{
	// Display the Background.
	switch(_BG_Mode)
	{
	// Backgroung is only made by 1 color (RGBA).
	case BG_plain:
		Driver->drawQuad(_X_Display, _Y_Display, _X_Display+_W_Display, _Y_Display+_H_Display, _BG_Color);
		break;

	// Background is stretch at the OSD size.
	case BG_stretch:
		Driver->drawBitmap(_X_Display, _Y_Display, _W_Display, _H_Display, *CInterfMngr::getTexture(_BG), true, _BG_Color);
		break;

	// No Background.
	default:
		break;
	}
}// drawBG //

//-----------------------------------------------
// drawTB :
// Function to draw the Title Bar according to the mode.
//-----------------------------------------------
void COSD::drawTB()
{
	// Display Title Bar.
	switch(_TB_Mode)
	{
	// Backgroung is only made by 1 color (RGBA).
	case TB_plain:
		Driver->drawQuad(_X_Display+_RS_W, _Y_Display+_H_Display-_RS_H-_TB_H_Display, _X_Display+_W_Display-_RS_W, _Y_Display+_H_Display-_RS_H, _TB_Color);
		break;

	// Background is stretch at the OSD size.
	case TB_stretch:
		Driver->drawBitmap(_X_Display+_RS_W, _Y_Display+_H_Display-_RS_H-_TB_H_Display, _W_Display-2*_RS_W, _TB_H_Display, *CInterfMngr::getTexture(_TB), true, _TB_Color);
		break;

	// No Background.
	default:
		break;
	}
}// drawTB //


//-----------------------------------------------
// drawBorders :
// Draw the resize borders.
//-----------------------------------------------
void COSD::drawBorders(float bSizeW, float bSizeH, float x0, float y0, float x1, float y1, const CRGBA &color)
{
	Driver->drawQuad(x0,		y0,			x0+bSizeW,	y1,			color);
	Driver->drawQuad(x1-bSizeW,	y0,			x1,			y1,			color);
	Driver->drawQuad(x0,		y0,			x1,			y0+bSizeH,	color);
	Driver->drawQuad(x0,		y1-bSizeH,	x1,			y1,			color);
}// drawBorders //

//-----------------------------------------------
// drawText :
// Draw a text with all information needed.
//-----------------------------------------------
void COSD::drawText(float x, float y, const ucstring &text, const CPen &pen)
{
	TextContext->setHotSpot(UTextContext::MiddleMiddle);
	TextContext->setColor(pen.color());
	TextContext->setFontSize(pen.fontSize());
	TextContext->setShaded(pen.shadow());
	TextContext->printAt(x, y, text);
}// drawText //


//-----------------------------------------------
// resize :
// The OSD size has changed -> resize controls.
//-----------------------------------------------
void COSD::resize(uint32 width, uint32 height)
{
	// Update variables because of the resize.
	calculateDisplay();

	// resize the other controls.
	for(TMapControls::iterator it = _Controls.begin(); it != _Controls.end(); it++)
	{
		// If the control as a reference different than the OSD -> the Reference will resize this control.
		if((*it).second->parent() == 0)
		{
			float x, y;
			(*it).second->resize(width, height);
			calculatePos(x, y, (*it).second->origin());
			(*it).second->ref(x, y, _W_Display, _H_Display);
		}
	}
}// resize //

//-----------------------------------------------
// update :
// Update the OSD (for timer, etc.).
//-----------------------------------------------
bool COSD::update(float x, float y, bool fullUse)
{
	bool used = false;


	// Function called every frame.
	CInterfMngr::runFuncCtrl(_OSD_Update_Func, 0);

	//
	if(fullUse)
	{
		if(_RS_Mode != no_resize)
		{
			// Get the window size.
			uint32 w, h;
			CInterfMngr::getWindowSize(w, h);

			// Resize correctly the OSD.
			switch(_RS_Mode)
			{
			// Resize Bottom OSD Border.
			case resize_B:
				resizeB(x, y);
				// Resize -> resize cursor.
				_Cursor = Cur_Resize_B;
				break;
			// Resize Top OSD Border.
			case resize_T:
				resizeT(x, y);
				// Resize -> resize cursor.
				_Cursor = Cur_Resize_T;
				break;
			// Resize Left OSD Border.
			case resize_L:
				resizeL(x, y);
				// Resize -> resize cursor.
				_Cursor = Cur_Resize_L;
				break;
			// Resize Right OSD Border.
			case resize_R:
				resizeR(x, y);
				// Resize -> resize cursor.
				_Cursor = Cur_Resize_R;
				break;
			// Resize Bottom Left OSD Border.
			case resize_BL:
				resizeB(x, y);
				resizeL(x, y);
				// Resize -> resize cursor.
				_Cursor = Cur_Resize_BL;
				break;
			// Resize Top Left OSD Border.
			case resize_TL:
				resizeT(x, y);
				resizeL(x, y);
				// Resize -> resize cursor.
				_Cursor = Cur_Resize_TL;
				break;
			// Resize Bottom Right OSD Border.
			case resize_BR:
				resizeB(x, y);
				resizeR(x, y);
				// Resize -> resize cursor.
				_Cursor = Cur_Resize_BR;
				break;
			// Resize Top Right OSD Border.
			case resize_TR:
				resizeT(x, y);
				resizeR(x, y);
				// Resize -> resize cursor.
				_Cursor = Cur_Resize_TR;
				break;
			// Move the OSD.
			case resize_move:
				move(x, y);
				// Move -> move cursor.
				_Cursor = Cur_Move;
				break;

			default:
				_Cursor = Cur_None;
				break;
			}

			// Give the new OSD size to controls.
			for(TMapControls::iterator it = _Controls.begin(); it != _Controls.end(); it++)
			{
				if((*it).second->parent() == 0)
				{
					float x, y;
					calculatePos(x, y, (*it).second->origin());
					(*it).second->ref(x, y, _W_Display, _H_Display);
				}
			}

			//
			used = true;
		}
		else
		{
			switch(_OSD_Mode)
			{
			// In Locked mode, the OSD mode can't change.
			case locked:
				// OSD locked -> no cursor needed by the OSD.
				_Cursor = Cur_None;
				break;

			// Else the OSD mode changes according to the mouse position.
			default:
				// Check the cursor is inside the OSD.
				if(x>=_X_Display && x<_X_Display+_W_Display && y>=_Y_Display && y<_Y_Display+_H_Display)
				{
					// Change the cursor look according to its position.
					// Get the window size.
					uint32 w, h;
					CInterfMngr::getWindowSize(w, h);

					float cornerSizeX = _RS_W+_TB_H/(float)w;
					float cornerSizeY = _RS_H+_TB_H/(float)h;

					enum TXState
					{
						X_0 = 0x00,
						X_1 = 0x01,
						X_2 = 0x02,
						X_3 = 0x03,
						X_4 = 0x04
					};

					enum TYState
					{
						Y_0 = 0x00,
						Y_1 = 0x10,
						Y_2 = 0x20,
						Y_3 = 0x30,
						Y_4 = 0x40
					};

					TXState x_State;
					TYState y_State;

					// X POSITION in the OSD.
					// X is in the right part of the OSD (move, resize right, or resize right corners)
					if(x >= _X_Display+_W_Display-cornerSizeX)
					{
						// Resize right or right corners.
						if(x >= _X_Display+_W_Display-_RS_W)
							x_State = X_4;
						// Move or resize right corners.
						else
							x_State = X_3;
					}
					// X is in the left part of the OSD (move, resize left, or resize left corners)
					else if(x < _X_Display+cornerSizeX)
					{
						// Resize left or left corners.
						if(x < _X_Display+_RS_W)
							x_State = X_0;
						// Move or resize left corners.
						else
							x_State = X_1;
					}
					// X is in the middle part of the OSD (move, resize top or bottom).
					else
						x_State = X_2;



					// Y POSITION in the OSD.
					// Y is in the Top part of the OSD (move, resize top, or resize top corners)
					if(y >= _Y_Display+_H_Display-cornerSizeY)
					{
						// Resize top or top corners.
						if(y >= _Y_Display+_H_Display-_RS_H)
							y_State = Y_4;
						// Move or resize top corners.
						else
							y_State = Y_3;
					}
					// Y is in the bottom part of the OSD (move, resize bottom, or resize bottom corners)
					else if(y < _Y_Display+cornerSizeY)
					{
						// Resize bottom or bottm corners.
						if(y < _Y_Display+_RS_H)
							y_State = Y_0;
						// Move or resize bottom corners.
						else
							y_State = Y_1;
					}
					// Y is in the middle part of the OSD (move, resize left or right).
					else
						y_State = Y_2;


					// Determine the Resize Mode.
					switch(x_State | y_State)
					{
					// Left Bottom corner
					case X_1|Y_0:
					case X_0|Y_0:
					case X_0|Y_1:
						_Cursor = Cur_Resize_BL;
						break;
					// Left side
					case X_0|Y_2:
						_Cursor = Cur_Resize_L;
						break;
					// Left Top corner
					case X_0|Y_3:
					case X_0|Y_4:
					case X_1|Y_4:
						_Cursor = Cur_Resize_TL;
						break;
					// Top Side.
					case X_2|Y_4:
						_Cursor = Cur_Resize_T;
						break;
					// Right Top Corner.
					case X_3|Y_4:
					case X_4|Y_4:
					case X_4|Y_3:
						_Cursor = Cur_Resize_TR;
						break;
					// Right Side.
					case X_4|Y_2:
						_Cursor = Cur_Resize_R;
						break;
					// Right Bottom Corner.
					case X_4|Y_1:
					case X_4|Y_0:
					case X_3|Y_0:
						_Cursor = Cur_Resize_BR;
						break;
					// Bottom Side.
					case X_2|Y_0:
						_Cursor = Cur_Resize_B;
						break;
					// Move Area
					case X_1|Y_3:
					case X_2|Y_3:
					case X_3|Y_3:
						_Cursor = Cur_Move;
						break;
					// Inside Area
					default:
						_Cursor = Cur_None;
						break;
					}


					// Change the display of the OSD according to the cursor position.
					testMode(x, y, _X_Display+_RS_W*2, _Y_Display+_RS_H*2, _X_Display+_W_Display-_RS_W*2, _Y_Display+_H_Display-_TB_H_Display-_RS_H*2);
					// ...
					used = true;
				}
				else
				{
					_OSD_Mode = none;
					// Cursor outside the OSD -> no cursor needed by the OSD.
					_Cursor = Cur_None;
				}
				break;
			}
		}
	}
	else
	{
		// Manage the OSD mode when the OSD do not have the Focus.
		switch(_OSD_Mode)
		{
		// In locked mode the mode do not change.
		case locked:
			break;

		// Else the OSD mode changes to "none".
		default:
			_OSD_Mode = none;
			break;
		}

		// OSD do not have any interaction -> no cursor.
		_Cursor = Cur_None;
	}


	// call the mouseMove() method on every child control
	TListControl::iterator itControl;
	const TListControl::iterator itControlEnd = _Children.end();

	for (itControl = _Children.begin() ; itControl != itControlEnd ; ++itControl)
	{
		nlassert(*itControl);
		// only send the mouse move event to controls with show = true
		if ( (*itControl)->show() )
			(*itControl)->mouseMove( x, y );
	}


	// Return if the OSD as react.
	return used;
}// update //


//-----------------------------------------------
// cursor :
// Return the cursor used by the OSD at the moment.
// \return ECursor : 'Cur_None' if no cursor needed for the OSD.
// \warning This method should be called after the update one to be up to date.
//-----------------------------------------------
COSD::ECursor COSD::cursor()
{
	// Is the OSD is not visible -> no cursor needed;
	if(!_Show)
		return Cur_None;

	// Return the cursor used for the OSD at the moment.
	return _Cursor;
}// cursor //


//-----------------------------------------------
// click :
// Manage the mouse click.
//-----------------------------------------------
void COSD::click(float x, float y, bool &taken)
{
	// Is the OSD is not visible -> return;
	if(!_Show)
		return;

	// if pop_up and cick was outside the window bounds: destroy that window
	if ( _PopUp )
	{
		if ( (x < _X_Display) || (x >= _X_Display + _W_Display) || (y < _Y_Display) || (y >= _Y_Display + _H_Display)  )
		{
			CInterfMngr::deleteOSD( this->_Id);
			return;
		}
	}

	_OffsetX = x-_X_Display;
	_OffsetY = y-_Y_Display;
	_OffsetW = x-(_X_Display+_W_Display);
	_OffsetH = y-(_Y_Display+_H_Display);

	// If some resize operations are in progress -> Stop resize.
	if(_RS_Mode != no_resize)
	{
		_RS_Mode = no_resize;
		taken = true;
	}
	else if (!taken)
	{

		// Look if the OSD will be resized.
		switch(_OSD_Mode)
		{
		// In some _OSD_Mode the aspect OSD can change -> test if it changes.
		case resizable:
		case movable:
			// Select the right resize mode -> mode != no_resize -> click = true.
			if(resizeMode(x, y) != TResize::no_resize)
				taken = true;
			break;
		}
	}

	// Dispatch the click in the controls.
	const TListControl::iterator itE = _Children.end();
	for(TListControl::iterator it = _Children.begin(); it != itE ; ++it)
	{
		// only send the click event to controls with show = true
		if ( (*it)->show() )
			(*it)->click(x, y, taken);
	}
}// click //






//-----------------------------------------------
// clickRight :
// Manage the mouse right click.
//-----------------------------------------------
void COSD::clickRight(float x, float y, bool &taken)
{
	// Is the OSD is not visible -> return;
	if(!_Show)
		return;

	// if pop_up and cick was outside the window bounds: destroy that window
	if ( _PopUp && ( (x < _X_Display) || (x >= _X_Display + _W_Display) || (y < _Y_Display) || (y >= _Y_Display + _H_Display) ) )
	{
		CInterfMngr::deleteOSD( this->_Id);
		return;
	}

	_OffsetX = x-_X_Display;
	_OffsetY = y-_Y_Display;
	_OffsetW = x-(_X_Display+_W_Display);
	_OffsetH = y-(_Y_Display+_H_Display);

	// If some resize operations are in progress -> Stop resize.
	if(_RS_Mode != no_resize)
	{
		_RS_Mode = no_resize;
		_OSD_Mode = locked;
		taken = true;
	}

	// Dispatch the click in the controls.
	const TListControl::iterator itE = _Children.end();
	for(TListControl::iterator it = _Children.begin(); it != itE ; ++it)
	{
		// only send the click event to controls with show = true
		if ( (*it)->show() )
			(*it)->clickRight(x, y, taken);
	}

	// Lock/Unlock the OSD
	if(!taken)
	{
		// Check that the cursor is in the OSD.
		if((x >= _X_Display) && (x < _X_Display + _W_Display) && (y >= _Y_Display) && (y < _Y_Display + _H_Display) )
		{
			if(_OSD_Mode != locked)
				_OSD_Mode = locked;
			else
				_OSD_Mode = none;

			taken = true;
		}
	}
}// clickRight //





//-----------------------------------------------
// resizeL :
// Resize the Left border of the OSD.
//-----------------------------------------------
void COSD::resizeL(float x, float y)
{
	uint32 width, height;
	CInterfMngr::getWindowSize(width, height);

	const float wMin = _W_Min / width;

	float xTmp = _X_Display + _W_Display;

	_X_Display = x - _OffsetX;

	if( _X_Display > xTmp - wMin )
		_X_Display = xTmp - wMin;

	_X = _X_Display;
	_W_Display = xTmp - _X_Display;
	_W_Pixel = _W_Display*width;
}// resizeL //

//-----------------------------------------------
// resizeR :
// Resize the Right border of the OSD.
//-----------------------------------------------
void COSD::resizeR(float x, float y)
{
	uint32 width, height;
	CInterfMngr::getWindowSize(width, height);

	const float wMin = _W_Min / width;

	_W_Display = x - _OffsetW - _X_Display;
	if(_W_Display < wMin )
		_W_Display = wMin;

	_W_Pixel = _W_Display*width;
}// resizeR //

//-----------------------------------------------
// resizeB :
// Resize the Bottom border of the OSD.
//-----------------------------------------------
void COSD::resizeB(float x, float y)
{
	uint32 width, height;
	CInterfMngr::getWindowSize(width, height);

	const float hMin = _H_Min / height;

	float yTmp = _Y_Display + _H_Display;
	_Y_Display = y-_OffsetY;
	if(_Y_Display > yTmp - hMin )
		_Y_Display = yTmp - hMin;

	_Y = _Y_Display;
	_H_Display = yTmp - _Y_Display;
	_H_Pixel = _H_Display*height;
}// resizeB //

//-----------------------------------------------
// resizeT :
// Resize the Top border of the OSD.
//-----------------------------------------------
void COSD::resizeT(float x, float y)
{
	uint32 width, height;
	CInterfMngr::getWindowSize(width, height);

	const float hMin = _H_Min / height;

	_H_Display = y - _OffsetH - _Y_Display;
	if(_H_Display < hMin )
		_H_Display = hMin;

	_H_Pixel = _H_Display*height;
}// resizeT //

//-----------------------------------------------
// move :
// Move the OSD.
//-----------------------------------------------
void COSD::move(float x, float y)
{
	_X_Display = x-_OffsetX;
	_Y_Display = y-_OffsetY;
	_X = _X_Display;
	_Y = _Y_Display;
}// move //





//-----------------------------------------------
// testMode :
// Test the mode of the OSD.
//-----------------------------------------------
void COSD::testMode(float x, float y, float rectX0, float rectY0, float rectX1, float rectY1)
{
	if(y>rectY1)
		_OSD_Mode = movable;
	else
	{
		if(x < rectX0 || x > rectX1 || y < rectY0)
			_OSD_Mode = resizable;
		else
			_OSD_Mode = selected;
	}
}// testMode //



/////////
// OSD //
//-----------------------------------------------
// osdSetPosition :
// Change the OSD Position.
//-----------------------------------------------
void COSD::osdSetPosition(float x, float y)
{
	_X = _X_Display = x;
	_Y = _Y_Display = y;

	// resize the other controls.
	for(TMapControls::iterator it = _Controls.begin(); it != _Controls.end(); it++)
	{
		// If the control as a reference different than the OSD -> the Reference will resize this control.
		if((*it).second->parent() == 0)
		{
			float x0, y0;
			calculatePos(x0, y0, (*it).second->origin());
			(*it).second->ref(x0, y0, _W_Display, _H_Display);
		}
	}
}// osdSetPosition //

//-----------------------------------------------
// osdGetPosition :
// Return the OSD Position.
//-----------------------------------------------
void COSD::osdGetPosition(float &x, float &y) const
{
	x = _X_Display;
	y = _Y_Display;
}// osdGetPosition //

//-----------------------------------------------
// osdGetSize :
// Change the OSD Size (between 0-1).
//-----------------------------------------------
void COSD::osdSetSize(float w, float h)
{
	uint32 width, height;
	CInterfMngr::getWindowSize(width, height);

	_W_Display = w;
	_H_Display = h;
	_W_Pixel = _W_Display*width;
	_H_Pixel = _H_Display*height;

	// resize the other controls.
	for(TMapControls::iterator it = _Controls.begin(); it != _Controls.end(); it++)
	{
		// If the control as a reference different than the OSD -> the Reference will resize this control.
		if((*it).second->parent() == 0)
		{
			float x0, y0;
			calculatePos(x0, y0, (*it).second->origin());
			(*it).second->ref(x0, y0, _W_Display, _H_Display);
		}
	}
}// osdGetSize //

//-----------------------------------------------
// osdGetSize :
// Return the OSD Size (between 0-1).
//-----------------------------------------------
void COSD::osdGetSize(float &w, float &h) const
{
	w = _W_Display;
	h = _H_Display;
}// osdGetSize //

//-----------------------------------------------
// osdMode :
// Change the OSD Mode (locked, resize, selected, etc.)
//-----------------------------------------------
void COSD::osdMode(TMode osdMode)
{
	_OSD_Mode = osdMode;
}// osdMode //

//-----------------------------------------------
// osdMode :
// Return the current OSD Mode.
//-----------------------------------------------
COSD::TMode COSD::osdMode() const
{
	return _OSD_Mode;
}// osdMode //

//-----------------------------------------------
// osdName :
// Change the OSD Name.
//-----------------------------------------------
void COSD::osdName(const ucstring &osdName)
{
	_OSD_Name = osdName;
}// osdName //

//-----------------------------------------------
// osdName :
// Return the OSD Name.
//-----------------------------------------------
ucstring COSD::osdName() const
{
	return _OSD_Name;
}// osdName //

//-----------------------------------------------
// updateFunc :
// Set the update Function.
//-----------------------------------------------
void COSD::osdUpdateFunc(uint updateFunc)
{
	_OSD_Update_Func = osdUpdateFunc;
}// updateFunc //

//-----------------------------------------------
// updateFunc :
// Get the update Function.
//-----------------------------------------------
uint COSD::osdUpdateFunc() const
{
	return _OSD_Update_Func;
}// updateFunc //


////////////////
// BACKGROUND //
//-----------------------------------------------
// bgMode :
// Set the Background display mode.
void COSD::bgMode(TBG mode)
//-----------------------------------------------
{
	_BG_Mode = mode;
}// bgMode //

//-----------------------------------------------
// bgMode :
// Get the Background display mode.
//-----------------------------------------------
COSD::TBG COSD::bgMode() const
{
	return _BG_Mode;
}// bgMode //

//-----------------------------------------------
// bg :
// Set the texture Id for the Background.
//-----------------------------------------------
void COSD::bg(uint b)
{
	_BG = b;
}// bg //

//-----------------------------------------------
// bg :
// Get background Id.
//-----------------------------------------------
uint COSD::bg() const
{
	return _BG;
}// bg //

//-----------------------------------------------
// bgColor :
// Set the Background RGBA.
//-----------------------------------------------
void COSD::bgColor(const CRGBA &bRGBA)
{
	_BG_Color = bRGBA;
}// bgColor //

//-----------------------------------------------
// bgColor :
// Get the background RGBA.
//-----------------------------------------------
CRGBA COSD::bgColor() const
{
	return _BG_Color;
}// bgColor //


///////////////
// TITLE BAR //
//-----------------------------------------------
// tbMode :
// Set the Title Bar display mode.
//-----------------------------------------------
void COSD::tbMode(TTB mode)
{
	_TB_Mode = mode;
}// tbMode //

//-----------------------------------------------
// tbMode :
// Get the Title Bar display mode.
//-----------------------------------------------
COSD::TTB COSD::tbMode() const
{
	return _TB_Mode;
}// tbMode //

//-----------------------------------------------
// tb :
// Set the texture Id for the Title Bar.
//-----------------------------------------------
void COSD::tb(uint t)
{
	_TB = t;
}// tb //

//-----------------------------------------------
// tb :
// Get Title Bar Id.
//-----------------------------------------------
uint COSD::tb() const
{
	return _TB;
}// tb //

//-----------------------------------------------
// tbColor :
// Set the Title Bar RGBA.
//-----------------------------------------------
void COSD::tbColor(const CRGBA &tRGBA)
{
	_TB_Color = tRGBA;
}// tbColor //

//-----------------------------------------------
// tbColor :
// Get the Title Bar RGBA.
//-----------------------------------------------
CRGBA COSD::tbColor() const
{
	return _TB_Color;
}// tbColor //

//-----------------------------------------------
// tbHeight :
// Set Title Bar Height (in Pixel)
//-----------------------------------------------
void COSD::tbHeight(float height)
{
	uint32 w, h;
	CInterfMngr::getWindowSize(w, h);

	_TB_H = height;
	// Calculate the Title Bar height for Display.
	_TB_H_Display = _TB_H/h;

}// tbHeight //

//-----------------------------------------------
// tbHeight :
// Get Title Bar Height (in Pixel)
//-----------------------------------------------
float COSD::tbHeight() const
{
	return _TB_H;
}// tbHeight //

//-----------------------------------------------
// pen :
// Set the Pen for the Title Bar.
//-----------------------------------------------
void COSD::tbPen(const CPen &pen)
{
	_TB_Pen = pen;
}// pen //

//-----------------------------------------------
// pen :
// Get the Pen of the Title Bar.
//-----------------------------------------------
CPen COSD::tbPen() const
{
	return _TB_Pen;
}// pen //


///////////////
// HIGHLIGHT //
//-----------------------------------------------
// hlColor :
// Set the HighLight Color.
//-----------------------------------------------
void COSD::hlColor(const CRGBA &hlColor)
{
	_HL_Color = hlColor;
}// hlColor //

//-----------------------------------------------
// hlColor :
// Get the HighLight Color.
//-----------------------------------------------
CRGBA COSD::hlColor() const
{
	return _HL_Color;
}// hlColor //

//-----------------------------------------------
// hlSize :
// Set the HighLight Size (in Pixel).
//-----------------------------------------------
void COSD::hlSize(float hlSize)
{
	uint32 w, h;
	CInterfMngr::getWindowSize(w, h);

	_HL_Size = hlSize;
	// Calculate borders for the HighLight.
	_HL_W			= _HL_Size/w;	// Left and Right.
	_HL_H			= _HL_Size/h;	// Bottom and Top.
}// hlSize //

//-----------------------------------------------
// hlSize :
// Get the HighLight Size (in Pixel).
//-----------------------------------------------
float COSD::hlSize() const
{
	return _HL_Size;
}// hlSize //


////////////
// RESIZE //
//-----------------------------------------------
// rsMode :
// Set the Resize Mode.
//-----------------------------------------------
void COSD::rsMode(TResize rsMode)
{
	_RS_Mode = rsMode;
}// rsMode //

//-----------------------------------------------
// rsMode :
// Get the Resize Mode.
//-----------------------------------------------
COSD::TResize COSD::rsMode() const
{
	return _RS_Mode;
}// rsMode //

//-----------------------------------------------
// rsColor :
// Set Resize borders Color
//-----------------------------------------------
void COSD::rsColor(CRGBA rsColor)
{
	_RS_Color = rsColor;
}// rsColor //

//-----------------------------------------------
// rsColor :
// Get Resize borders Color
//-----------------------------------------------
CRGBA COSD::rsColor() const
{
	return _RS_Color;
}// rsColor //

//-----------------------------------------------
// rsSize :
// Set Resize size (in pixel).
//-----------------------------------------------
void COSD::rsSize(float rsSize)
{
	uint32 w, h;
	CInterfMngr::getWindowSize(w, h);

	_RS_Size = rsSize;
	// Calculate borders for the Resize.
	_RS_W = _RS_Size/w;	// Left and Right.
	_RS_H = _RS_Size/h;	// Bottom and Top.
}// rsSize //

//-----------------------------------------------
// rsSize :
// Get Resize size (in pixel).
//-----------------------------------------------
float COSD::rsSize() const
{
	return _RS_Size;
}// rsSize //





//-----------------------------------------------
// getCtrl :
// Return the pointer of the control "id".
//-----------------------------------------------
CControl * COSD::getCtrl(uint id)
{
	TMapControls::iterator it = _Controls.find(id);

	if(it != _Controls.end())
		return (*it).second;
	else
		return 0;
}// getCtrl //


//-----------------------------------------------
// addChild :
// Add a control.
//-----------------------------------------------
void COSD::addChild(uint idCtrl, CControl *ctrl)
{
	// ...
	_Controls.insert(TMapControls::value_type(idCtrl, ctrl));
	// The list of control in order to display.
	_Children.push_front(ctrl);
}// addChild //


//-----------------------------------------------
// delChild :
// Delete a control by the Id.
//-----------------------------------------------
void COSD::delChild(uint idCtrl)
{
	TMapControls::iterator it = _Controls.find(idCtrl);
	if(it != _Controls.end())
	{
		CControl *ctrl = (*it).second;

		// Erase control.
		_Controls.erase(it);
		// Check if the control is allocated.
		if(ctrl)
		{
			// Erase Control.
			for(TListControl::iterator itChild = _Children.begin(); itChild != _Children.end(); ++itChild)
			{
				if((*itChild) == ctrl)
				{
					_Children.erase(itChild);
					break;
				}
			}
			// Delete Control.
//			delete ctrl;
//			ctrl = 0;
		}
	}
}// delChild //





//-----------------------------------------------
// removeFromChildren :
// Add a control.
//-----------------------------------------------
void COSD::removeFromChildren(uint idCtrl)
{
	TMapControls::iterator it = _Controls.find(idCtrl);
	if(it != _Controls.end())
	{
		CControl *ctrl = (*it).second;

		// Check if the control is allocated.
		if(ctrl)
		{
			// Erase Control.
			for(TListControl::iterator itChild = _Children.begin(); itChild != _Children.end(); ++itChild)
			{
				if((*itChild) == ctrl)
				{
					_Children.erase(itChild);
					break;
				}
			}
		}
	}
}// removeFromChildren //


//-----------------------------------------------
// calculatePos :
// Calculate a position according to the origin.
//-----------------------------------------------
void COSD::calculatePos(float &x, float &y, CControl::THotSpot origin)
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
}// calculatePos //


//-----------------------------------------------
// calculateDisplay :
// Calculate the Display X, Y, Width, Height.
//-----------------------------------------------
void COSD::calculateDisplay()
{
	uint32 w, h;
	CInterfMngr::getWindowSize(w, h);


	// Calculate the Title Bar height for Display.
	_TB_H_Display	= _TB_H/h;
	// Calculate borders for the Resize.
	_RS_W			= _RS_Size/w;	// Left and Right.
	_RS_H			= _RS_Size/h;	// Bottom and Top.
	// Calculate borders for the HighLight.
	_HL_W			= _HL_Size/w;	// Left and Right.
	_HL_H			= _HL_Size/h;	// Bottom and Top.

	// Calculate the display Width and Height.
	if(w!=0)
		_W_Display	= _W_Pixel/w;	//_W*_W_Ref + _W_Pixel/w;
	else
		_W_Display	= 0.f;			//_W*_W_Ref;

	if(h!=0)
		_H_Display	= _H_Pixel/h;	//_H*_H_Ref + _H_Pixel/h;
	else
		_H_Display	= 0.f;			//_H*_H_Ref;

//	// Calculate the HotSpot.
//	calculateHS();

	_X_Display = _X;				//_X_Ref + _X*_W_Ref + _X_Pixel/w + _X_HotSpot;
	_Y_Display = _Y;				//_Y_Ref + _Y*_H_Ref + _Y_Pixel/h + _Y_HotSpot;
}// calculateDisplay //




///////////////////////////
// OPERATIONS ON THE OSD //
///////////////////////////
//-----------------------------------------------
// testInRect :
// Function to test if a coordinate is in the rect.
//-----------------------------------------------
bool COSD::testInRect(float x, float y, float rectX0, float rectY0, float rectX1, float rectY1)
{
	if(x>=rectX0 && x<rectX1 && y>=rectY0 && y<rectY1)
		return true;
	return false;
}// testInRect //


//-----------------------------------------------
// resizeMode :
// Determine the Resize Mode.
//-----------------------------------------------
bool COSD::resizeMode(float x, float y)
{
	// Get the window size.
	uint32 w, h;
	CInterfMngr::getWindowSize(w, h);

	float cornerSizeX = _RS_W+_TB_H/(float)w;
	float cornerSizeY = _RS_H+_TB_H/(float)h;

	enum TXState
	{
		X_0 = 0x00,
		X_1 = 0x01,
		X_2 = 0x02,
		X_3 = 0x03,
		X_4 = 0x04
	};

	enum TYState
	{
		Y_0 = 0x00,
		Y_1 = 0x10,
		Y_2 = 0x20,
		Y_3 = 0x30,
		Y_4 = 0x40
	};

	TXState x_State;
	TYState y_State;

	// X POSITION in the OSD.
	// X is in the right part of the OSD (move, resize right, or resize right corners)
	if(x >= _X_Display+_W_Display-cornerSizeX)
	{
		// Resize right or right corners.
		if(x >= _X_Display+_W_Display-_RS_W)
			x_State = X_4;
		// Move or resize right corners.
		else
			x_State = X_3;
	}
	// X is in the left part of the OSD (move, resize left, or resize left corners)
	else if(x < _X_Display+cornerSizeX)
	{
		// Resize left or left corners.
		if(x < _X_Display+_RS_W)
			x_State = X_0;
		// Move or resize left corners.
		else
			x_State = X_1;
	}
	// X is in the middle part of the OSD (move, resize top or bottom).
	else
		x_State = X_2;



	// Y POSITION in the OSD.
	// Y is in the Top part of the OSD (move, resize top, or resize top corners)
	if(y >= _Y_Display+_H_Display-cornerSizeY)
	{
		// Resize top or top corners.
		if(y >= _Y_Display+_H_Display-_RS_H)
			y_State = Y_4;
		// Move or resize top corners.
		else
			y_State = Y_3;
	}
	// Y is in the bottom part of the OSD (move, resize bottom, or resize bottom corners)
	else if(y < _Y_Display+cornerSizeY)
	{
		// Resize bottom or bottm corners.
		if(y < _Y_Display+_RS_H)
			y_State = Y_0;
		// Move or resize bottom corners.
		else
			y_State = Y_1;
	}
	// Y is in the middle part of the OSD (move, resize left or right).
	else
		y_State = Y_2;


	bool resize = true;
	// Determine the Resize Mode.
	switch(x_State | y_State)
	{
	// Left Bottom corner
	case X_1|Y_0:
	case X_0|Y_0:
	case X_0|Y_1:
		_RS_Mode = resize_BL;
		break;
	// Left side
	case X_0|Y_2:
		_RS_Mode = resize_L;
		break;
	// Left Top corner
	case X_0|Y_3:
	case X_0|Y_4:
	case X_1|Y_4:
		_RS_Mode = resize_TL;
		break;
	// Top Side.
	case X_2|Y_4:
		_RS_Mode = resize_T;
		break;
	// Right Top Corner.
	case X_3|Y_4:
	case X_4|Y_4:
	case X_4|Y_3:
		_RS_Mode = resize_TR;
		break;
	// Right Side.
	case X_4|Y_2:
		_RS_Mode = resize_R;
		break;
	// Right Bottom Corner.
	case X_4|Y_1:
	case X_4|Y_0:
	case X_3|Y_0:
		_RS_Mode = resize_BR;
		break;
	// Bottom Side.
	case X_2|Y_0:
		_RS_Mode = resize_B;
		break;
	// Move Area
	case X_1|Y_3:
	case X_2|Y_3:
	case X_3|Y_3:
		_RS_Mode = resize_move;
		break;
	// Inside Area
	default:
		_RS_Mode = no_resize;
		resize = false;
		break;
	}

	return resize;
}// resizeMode //



////////////
// SCRIPT //
////////////
//-----------------------------------------------
// getText :
//
//-----------------------------------------------
void COSD::getText(uint idCtrl)
{
	uint idTxt	= 0;
	uint idPen	= 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Text.
		else if(strcmp(ptr, "Text:") == 0)
			idTxt = getInt();
		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
	)

	// Create the list.
	CText *text = new CText(idCtrl, x, y, xPixel, yPixel, CInterfMngr::getText(idTxt), CInterfMngr::getPen(idPen));
	if(text)
	{
		text->origin(origin);
		text->hotSpot(hs);
		text->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(text);
				text->parent(parent);
			}
		}

		addChild(idCtrl, text);
	}
}// getText //

//-----------------------------------------------
// getCapture :
//
//-----------------------------------------------
void COSD::getCapture(uint idCtrl)
{
	uint idtexture = 2;
	CRGBA rgba(255,255,255,128);
	uint numFunc	= 0;
	uint idPen		= 0;
	uint idPrompt	= 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Function.
		else if(strcmp(ptr, "Function:") == 0)
			numFunc = getInt();
		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
		// Get the Prompt.
		else if(strcmp(ptr, "Prompt:") == 0)
			idPrompt = getInt();
	)

	// Create the list.
	CCapture *capture = new CCapture(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, numFunc, CInterfMngr::getPen(idPen));
	if(capture)
	{
		capture->origin(origin);
		capture->hotSpot(hs);
		capture->ref(_X_Display, _Y_Display, _W_Display, _H_Display);
		capture->setPrompt( CInterfMngr::getText(idPrompt) );

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(capture);
				capture->parent(parent);
			}
		}

		addChild(idCtrl, capture);
	}
}// getCapture //


//-----------------------------------------------
// getButton :
//
//-----------------------------------------------
void COSD::getButton(uint idCtrl)
{
	uint numFunc	= 0;
	uint numFuncR	= 0;
	uint numFuncD	= 0;

	uint idTxt		= 0;
	uint idPen		= 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Function for left click
		else if(strcmp(ptr, "Function:") == 0)
			numFunc = getInt();

		// Get the Function for Right click
		else if(strcmp(ptr, "FunctionRight:") == 0)
			numFuncR = getInt();

		// Get the Function for double click
		else if(strcmp(ptr, "FunctionDouble:") == 0)
			numFuncD = getInt();

		// Get the Text.
		else if(strcmp(ptr, "Text:") == 0)
			idTxt = getInt();

		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
	)

	// Create the button.
	CButton *button = new CButton(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, numFunc, numFuncR, numFuncD, CInterfMngr::getButton(idCtrl));
	if(button)
	{
		button->origin(origin);
		button->hotSpot(hs);
		button->ref(_X_Display, _Y_Display, _W_Display, _H_Display);
		button->pen(CInterfMngr::getPen(idPen));
		if(idTxt != 0)
			button->text(CInterfMngr::getText(idTxt));

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(button);
				button->parent(parent);
			}
		}
		addChild(idCtrl, button);
	}
}// getButton //


//-----------------------------------------------
// getRadioButton :
//
//-----------------------------------------------
void COSD::getRadioButton(uint idCtrl)
{
	uint numFunc	= 0;
	uint numFuncR	= 0;
	uint numFuncD	= 0;

	uint idTxt		= 0;
	uint idPen		= 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Function for left click
		else if(strcmp(ptr, "Function:") == 0)
			numFunc = getInt();

		// Get the Function for Right click
		else if(strcmp(ptr, "FunctionRight:") == 0)
			numFuncR = getInt();

		// Get the Function for double click
		else if(strcmp(ptr, "FunctionDouble:") == 0)
			numFuncD = getInt();

		// Get the Text.
		else if(strcmp(ptr, "Text:") == 0)
			idTxt = getInt();

		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
	)

	// Create the button.
	CRadioButton *button = new CRadioButton(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, numFunc, numFuncR, numFuncD, CInterfMngr::getButton(idCtrl));
	if(button)
	{
		button->origin(origin);
		button->hotSpot(hs);
		button->ref(_X_Display, _Y_Display, _W_Display, _H_Display);
		button->pen(CInterfMngr::getPen(idPen));
		if(idTxt != 0)
			button->text(CInterfMngr::getText(idTxt));

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(button);
				button->parent(parent);
			}
		}
		addChild(idCtrl, button);
	}
}// getRadioButton //


//-----------------------------------------------
// getRadioController :
//
//-----------------------------------------------
void COSD::getRadioController(uint idCtrl)
{
	char	delimiter[] = "[] \t";

	// Create the radio controller.
	CRadioController *radioController = new CRadioController(idCtrl);
	if(radioController)
	{
		uint buttonId;
		TMapControls::iterator it;

		char *ptr = strtok(NULL, delimiter);
		while(ptr != NULL)
		{
			// Get all buttons.
			if(strcmp(ptr, "Buttons:") == 0)
			{
				ptr = strtok(NULL, delimiter);
				while((ptr != NULL) && (strcmp(ptr, "End")!=0))
				{
					buttonId = atoi(ptr);
					it = _Controls.find(buttonId);
					if(it != _Controls.end())
					{
						if(radioController->add(dynamic_cast<CRadioButton*>((*it).second)) == false)
						{
							nlerror("Interface Error : control %d can't be add in the radio button %d", buttonId, idCtrl);
						}
					}

					// Next Id.
					ptr = strtok(NULL, delimiter);
				}
			}

			// Next Token
			ptr = strtok(NULL, delimiter);
		}

		// Test if there is at least 1 button)
		if( radioController->size() > 0)
		{
			// Insert the radio controller in the control list.
			addChild(idCtrl, radioController);
		}
		// No Button -> Destroy Radio Button.
		else
		{
			delete radioController;
			radioController = 0;
		}
	}
}// getRadioController //

//-----------------------------------------------
// getBitmap :
//
//-----------------------------------------------
void COSD::getBitmap(uint idCtrl)
{
	uint idtexture = 0;
	bool tiled = false;
	CRGBA rgba;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Texture.
		else if(strcmp(ptr, "Texture:") == 0)
			idtexture = getInt();
		// Get the RGBA.
		else if(strcmp(ptr, "RGBA:") == 0)
			rgba = getRGBA();
		// tiled or stretched
		else if(strcmp(ptr, "Tiled:") == 0)
		{
			if (getInt() ==1)
				tiled = true;
		}
	)

	// Create the list.
	CBitm *bitmap = new CBitm(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, idtexture, rgba);
	if(bitmap)
	{
		bitmap->tiled( tiled );
		bitmap->origin(origin);
		bitmap->hotSpot(hs);
		bitmap->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(bitmap);
				bitmap->parent(parent);
			}
		}

		addChild(idCtrl, bitmap);
	}
}// getBitmap //

//-----------------------------------------------
// getList :
//
//-----------------------------------------------
void COSD::getList(uint idCtrl)
{
	uint idPen = 0;

	uint upTexture = 0;
	uint downTexture = 0;
	uint scrollBarTexture = 0;
	CRGBA scrollBarRgba(255,255,255,255);

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
		// Get the Texture for the 'up' arrow
		else if(strcmp(ptr, "UpTexture:") == 0)
			upTexture = getInt();
		// Get the Texture for the 'down' arrow
		else if(strcmp(ptr, "DownTexture:") == 0)
			downTexture = getInt();
		// Get the Texture for the scroll bar body
		else if(strcmp(ptr, "ScrollBarTexture:") == 0)
			scrollBarTexture = getInt();
		// Get the RGBA for the scroll bar
		else if(strcmp(ptr, "ScrollBarRGBA:") == 0)
			scrollBarRgba = getRGBA();
	)

	// Create the list.
	CList *list = new CList(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, CInterfMngr::getPen(idPen));
	if(list)
	{
		list->origin(origin);
		list->hotSpot(hs);
		list->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		CScrollBar *scroll = list->getVScroll();
		if(scroll)
		{
			scroll->setDownArrowTextureOn( downTexture );
			scroll->setUpArrowTextureOn( upTexture );
			scroll->textureOn( scrollBarTexture );
			scroll->setDownArrowTextureOff( downTexture );
			scroll->setUpArrowTextureOff( upTexture );
			scroll->textureDisable( scrollBarTexture );
			scroll->colorOn( scrollBarRgba );
			scroll->enable(true);
		}

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(list);
				list->parent(parent);
			}
		}

		addChild(idCtrl, list);
	}
}// getList //


//-----------------------------------------------
// getMultiList :
//
//-----------------------------------------------
void COSD::getMultiList(uint idCtrl)
{
	uint idtexture = 0;
	uint upTexture = 0;
	uint downTexture = 0;
	uint scrollBarTexture = 0;
	uint nbCol = 2;
	uint16 spacing = 0;
	uint16 lineHeight = 0;

	std::vector<float> colSize;

	CRGBA rgba(255,255,255,255);
	CRGBA scrollBarRgba(255,255,255,255);

	uint idPen = 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the number of columns
		else if(strcmp(ptr, "NbColumns:") == 0)
			nbCol = getInt();

		// Get the columns size.
		else if(strcmp(ptr, "ColSize:") == 0)
			colSize = getVectorOfFloat(nbCol);

		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();

		// Get the Spacing.
		else if(strcmp(ptr, "Spacing:") == 0)
			spacing = getInt();

		// Get the specified Line Height.
		else if(strcmp(ptr, "LineHeight:") == 0)
			lineHeight = getInt();

		// Get the Texture for the 'up' arrow
		else if(strcmp(ptr, "UpTexture:") == 0)
			upTexture = getInt();
		// Get the Texture for the 'down' arrow
		else if(strcmp(ptr, "DownTexture:") == 0)
			downTexture = getInt();
		// Get the Texture for the scroll bar body
		else if(strcmp(ptr, "ScrollBarTexture:") == 0)
			scrollBarTexture = getInt();
		// Get the RGBA for the scroll bar
		else if(strcmp(ptr, "ScrollBarRGBA:") == 0)
			scrollBarRgba = getRGBA();
	)

	// Create the list.
	CMultiList *list = new CMultiList(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, CInterfMngr::getPen(idPen), nbCol);
	if(list)
	{
		list->origin(origin);
		list->hotSpot(hs);
		list->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if ( nbCol == colSize.size() )
			list->setColSize( colSize );

		list->setSpacing( spacing );
		list->setLineHeight( lineHeight );

		CScrollBar *scroll = list->getVScroll();
		if (scroll)
		{
			scroll->setDownArrowTextureOn( downTexture );
			scroll->setUpArrowTextureOn( upTexture );
			scroll->textureOn( scrollBarTexture );
			scroll->setDownArrowTextureOff( downTexture );
			scroll->setUpArrowTextureOff( upTexture );
			scroll->textureDisable( scrollBarTexture );
			scroll->colorOn( scrollBarRgba );
			scroll->enable(true);
		}

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(list);
				list->parent(parent);
			}
		}

		addChild(idCtrl, list);
	}
}// getMultiList //



//-----------------------------------------------
// getChatBox :
//
//-----------------------------------------------
void COSD::getChatBox(uint idCtrl)
{
	uint idtexture = 0;
	uint upTexture = 0;
	uint downTexture = 0;
	uint scrollBarTexture = 0;
	CRGBA rgba(255,255,255,255);
	CRGBA scrollBarRgba(255,255,255,255);
	uint idPen = 0;
	uint leftFunc = 0;
	uint rightFunc = 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();

		// Get the function to execute on left click on player name
		else if(strcmp(ptr, "LeftClickFunction:") == 0)
			leftFunc = getInt();

		// Get the function to execute on left click on player name
		else if(strcmp(ptr, "RightClickFunction:") == 0)
			rightFunc = getInt();
		// Get the Texture for the 'up' arrow
		else if(strcmp(ptr, "UpTexture:") == 0)
			upTexture = getInt();
		// Get the Texture for the 'down' arrow
		else if(strcmp(ptr, "DownTexture:") == 0)
			downTexture = getInt();
		// Get the Texture for the scroll bar body
		else if(strcmp(ptr, "ScrollBarTexture:") == 0)
			scrollBarTexture = getInt();
		// Get the RGBA for the scroll bar
		else if(strcmp(ptr, "ScrollBarRGBA:") == 0)
			scrollBarRgba = getRGBA();
	)

	// Create the list.
	CChatControl *chat = new CChatControl(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, leftFunc, rightFunc, CInterfMngr::getPen(idPen));
	if(chat)
	{
		chat->origin(origin);
		chat->hotSpot(hs);
		chat->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		CScrollBar *scroll = chat->getVScroll();
		if (scroll)
		{
			scroll->setDownArrowTextureOn( downTexture );
			scroll->setUpArrowTextureOn( upTexture );
			scroll->textureOn( scrollBarTexture );
			scroll->setDownArrowTextureOff( downTexture );
			scroll->setUpArrowTextureOff( upTexture );
			scroll->textureDisable( scrollBarTexture );
			scroll->colorOn( scrollBarRgba );
			scroll->enable(true);
		}


		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(chat);
				chat->parent(parent);
			}
		}

		addChild(idCtrl, chat);
	}
}// getChatBox //


//-----------------------------------------------
// getCandidateList :
//
//-----------------------------------------------
void COSD::getCandidateList(uint idCtrl)
{
	uint idtexture = 0;
	uint upTexture = 0;
	uint downTexture = 0;
	uint scrollBarTexture = 0;
	uint spacing = 0;
	uint lineHeight = 0;
	uint leftFunc = 0;
	uint rightFunc = 0;
	std::vector<float> colSize;

	CRGBA rgba(255,255,255,255);
	CRGBA selRgba(0,0,0,255);
	CRGBA scrollBarRgba(255,255,255,255);

	uint idPen = 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the columns size.
		else if(strcmp(ptr, "ColSize:") == 0)
			colSize = getVectorOfFloat(2);

		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();

		// Get the Spacing.
		else if(strcmp(ptr, "Spacing:") == 0)
			spacing = getInt();

		// Get the function to execute on left click on candidate name
		else if(strcmp(ptr, "LeftClickFunction:") == 0)
			leftFunc = getInt();

		// Get the function to execute on left click on candidate name
		else if(strcmp(ptr, "RightClickFunction:") == 0)
			rightFunc = getInt();

		// Get the specified Line Height.
		else if(strcmp(ptr, "LineHeight:") == 0)
			lineHeight = getInt();

		// Get the Texture for the 'up' arrow
		else if(strcmp(ptr, "UpTexture:") == 0)
			upTexture = getInt();
		// Get the Texture for the 'down' arrow
		else if(strcmp(ptr, "DownTexture:") == 0)
			downTexture = getInt();
		// Get the Texture for the scroll bar body
		else if(strcmp(ptr, "ScrollBarTexture:") == 0)
			scrollBarTexture = getInt();
		// Get the RGBA for the scroll bar
		else if(strcmp(ptr, "ScrollBarRGBA:") == 0)
			scrollBarRgba = getRGBA();

		// Get the RGBA for the selection box
		else if(strcmp(ptr, "SelectionRGBA:") == 0)
			selRgba = getRGBA();
	)

	// Create the list.
	CCandidateList *list = new CCandidateList(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, CInterfMngr::getPen(idPen));
	if(list)
	{
		list->origin(origin);
		list->hotSpot(hs);
		list->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if( 2 == colSize.size() )
			list->setColSize( colSize );

		list->setSpacing( spacing );
		list->setLineHeight( lineHeight );
		list->setSelectedColor( selRgba );
		list->setLeftClickFunction( leftFunc );
		list->setRightClickFunction( rightFunc );


		CScrollBar *scroll = list->getVScroll();
		if (scroll)
		{
			scroll->setDownArrowTextureOn( downTexture );
			scroll->setUpArrowTextureOn( upTexture );
			scroll->textureOn( scrollBarTexture );
			scroll->setDownArrowTextureOff( downTexture );
			scroll->setUpArrowTextureOff( upTexture );
			scroll->textureDisable( scrollBarTexture );
			scroll->colorOn( scrollBarRgba );
			scroll->enable(true);
		}

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(list);
				list->parent(parent);
			}
		}

		addChild(idCtrl, list);
	}
}// getCandidateList //

//-----------------------------------------------
// getChatInput :
//
//-----------------------------------------------
void COSD::getChatInput(uint idCtrl)
{
	uint idtexture = 0;
	CRGBA rgba;
	uint numFunc = 0;
	uint idPen = 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Function.
		else if(strcmp(ptr, "Function:") == 0)
			numFunc = getInt();
		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
	)

	// Create the chat input
	CChatInput *chat = new CChatInput(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, numFunc, CInterfMngr::getPen(idPen));
	if(chat)
	{
		chat->origin(origin);
		chat->hotSpot(hs);
		chat->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(chat);
				chat->parent(parent);
			}
		}

		addChild(idCtrl, chat);
	}
}// getChatInput //


//-----------------------------------------------
// getChatInput :
//
//-----------------------------------------------
void COSD::getChoiceList(uint idCtrl)
{
	uint idPen = 0;

	uint upTexture = 0;
	uint downTexture = 0;
	uint scrollBarTexture = 0;
	CRGBA scrollBarRgba(255,255,255,255);


	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
		// Get the Texture for the 'up' arrow
		else if(strcmp(ptr, "UpTexture:") == 0)
			upTexture = getInt();
		// Get the Texture for the 'down' arrow
		else if(strcmp(ptr, "DownTexture:") == 0)
			downTexture = getInt();
		// Get the Texture for the scroll bar body
		else if(strcmp(ptr, "ScrollBarTexture:") == 0)
			scrollBarTexture = getInt();
		// Get the RGBA for the scroll bar
		else if(strcmp(ptr, "ScrollBarRGBA:") == 0)
			scrollBarRgba = getRGBA();
	)

	// Create the chat input
	CChoiceList *choiceList = new CChoiceList(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, CInterfMngr::getPen(idPen));
	if(choiceList)
	{
		choiceList->origin(origin);
		choiceList->hotSpot(hs);
		choiceList->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		CScrollBar *scroll = choiceList->getVScroll();
		if (scroll)
		{
			scroll->setDownArrowTextureOn( downTexture );
			scroll->setUpArrowTextureOn( upTexture );
			scroll->textureOn( scrollBarTexture );
			scroll->setDownArrowTextureOff( downTexture );
			scroll->setUpArrowTextureOff( upTexture );
			scroll->textureDisable( scrollBarTexture );
			scroll->colorOn( scrollBarRgba );
			scroll->enable(true);
		}

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(choiceList);
				choiceList->parent(parent);
			}
		}

		addChild(idCtrl, choiceList);
	}
}// getChatInput //


//-----------------------------------------------
// getHorizontalList :
//
//-----------------------------------------------
void COSD::getHorizontalList(uint idCtrl)
{
	uint16	spacing = 0;
	uint	scrollLeftId = 0;
	uint	scrollRightId = 0;
	uint	idtexture = 0;
	CRGBA	rgba(255,255,255,255);
	bool	tiled = false;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Spacing.
		else if(strcmp(ptr, "Spacing:") == 0)
			spacing = getInt();

		else if(strcmp(ptr, "ScrollLeft:") == 0)
			scrollLeftId = getInt();

		else if(strcmp(ptr, "ScrollRight:") == 0)
			scrollRightId = getInt();

		else if(strcmp(ptr, "Controls:") == 0)
		{
			ptr = strtok(NULL, delimiter);
			while((ptr != NULL) && (strcmp(ptr, "End")!=0))
			{
				elts.push_back( atoi(ptr) );
				ptr = strtok(NULL, delimiter);
			}
		}

		// Get the Texture.
		else if(strcmp(ptr, "Texture:") == 0)
			idtexture = getInt();
		// Get the RGBA.
		else if(strcmp(ptr, "RGBA:") == 0)
			rgba = getRGBA();
		else if(strcmp(ptr, "Tiled:") == 0)
		{
			if ( getInt() == 1 )
				tiled = true;
		}
	)

	// Create the list.
	CHorizontalList *list = new CHorizontalList(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, spacing, idtexture, rgba);
	if(list)
	{
		list->tiled( tiled );

		std::list< uint16 >::iterator	itElts, itEltsEnd = elts.end();
		TMapControls::iterator it;

		for (itElts = elts.begin() ; itElts != itEltsEnd ; ++itElts )
		{
			it = _Controls.find( *itElts );
			if(it != _Controls.end())
			{
				CControl *ctrl = (*it).second;
				// Erase control.
				_Controls.erase(it);
				// Check if the control is allocated.
				if(ctrl)
				{
					// add control to the list
					list->add( ctrl );

					// Erase Control.
					for(TListControl::iterator itChild = _Children.begin(); itChild != _Children.end(); ++itChild)
					{
						if((*itChild) == ctrl)
						{
							_Children.erase(itChild);
							break;
						}
					}
				}
			}
		}

		// get left scroll bitmap
		if (scrollLeftId != 0)
		{
			it = _Controls.find( scrollLeftId );
			if(it != _Controls.end())
			{
				CControl *ctrl = (*it).second;
				// Erase control.
				//_Controls.erase(it);
				if(ctrl)
				{
					// add control to the list
					list->setLeftScrollBitmap( safe_cast<CBitm*> (ctrl) );

					// Erase Control.
					for(TListControl::iterator itChild = _Children.begin(); itChild != _Children.end(); ++itChild)
					{
						if((*itChild) == ctrl)
						{
							_Children.erase(itChild);
							break;
						}
					}
				}
			}
		}
		// get right scroll bitmap
		if (scrollRightId != 0)
		{
			it = _Controls.find( scrollRightId );
			if(it != _Controls.end())
			{
				CControl *ctrl = (*it).second;
				// Erase control.
				//_Controls.erase(it);
				if(ctrl)
				{
					// add control to the list
					list->setRightScrollBitmap( safe_cast<CBitm*> (ctrl) );

					// Erase Control.
					for(TListControl::iterator itChild = _Children.begin(); itChild != _Children.end(); ++itChild)
					{
						if((*itChild) == ctrl)
						{
							_Children.erase(itChild);
							break;
						}
					}
				}
			}
		}

		list->origin(origin);
		list->hotSpot(hs);
		list->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(list);
				list->parent(parent);
			}
		}

		addChild(idCtrl, list);
	}
}// getHorizontalList //


//-----------------------------------------------
// getControlList :
//
//-----------------------------------------------
void COSD::getControlList(uint idCtrl)
{
	uint16 spacing = 0;
	uint upTexture = 0;
	uint downTexture = 0;
	uint scrollBarTexture = 0;
	CRGBA scrollBarRgba(255,255,255,255);

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Spacing.
		else if(strcmp(ptr, "Spacing:") == 0)
			spacing = getInt();

		else if(strcmp(ptr, "Controls:") == 0)
		{
			ptr = strtok(NULL, delimiter);
			while((ptr != NULL) && (strcmp(ptr, "End")!=0))
			{
				elts.push_back( atoi(ptr) );
				ptr = strtok(NULL, delimiter);
			}
		}
		// Get the Texture for the 'up' arrow
		else if(strcmp(ptr, "UpTexture:") == 0)
			upTexture = getInt();
		// Get the Texture for the 'down' arrow
		else if(strcmp(ptr, "DownTexture:") == 0)
			downTexture = getInt();
		// Get the Texture for the scroll bar body
		else if(strcmp(ptr, "ScrollBarTexture:") == 0)
			scrollBarTexture = getInt();
		// Get the RGBA for the scroll bar
		else if(strcmp(ptr, "ScrollBarRGBA:") == 0)
			scrollBarRgba = getRGBA();

	)

	// Create the list.
	CControlList *list = new CControlList(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, spacing);
	if(list)
	{
		CScrollBar *scroll = list->getVScroll();
		if (scroll)
		{
			scroll->setDownArrowTextureOn( downTexture );
			scroll->setUpArrowTextureOn( upTexture );
			scroll->textureOn( scrollBarTexture );
			scroll->setDownArrowTextureOff( downTexture );
			scroll->setUpArrowTextureOff( upTexture );
			scroll->textureDisable( scrollBarTexture );
			scroll->colorOn( scrollBarRgba );
			scroll->enable(true);
		}

		std::list< uint16 >::iterator	itElts, itEltsEnd = elts.end();
		TMapControls::iterator it;

		for (itElts = elts.begin() ; itElts != itEltsEnd ; ++itElts )
		{
			it = _Controls.find( *itElts );
			if(it != _Controls.end())
			{
				CControl *ctrl = (*it).second;
				// Erase control.
				//_Controls.erase(it);
				// Check if the control is allocated.
				if(ctrl)
				{
					// add control to the list
					list->add( ctrl );

					// Erase Control.
					for(TListControl::iterator itChild = _Children.begin(); itChild != _Children.end(); ++itChild)
					{
						if((*itChild) == ctrl)
						{
							_Children.erase(itChild);
							break;
						}
					}
				}
			}
		}
		list->origin(origin);
		list->hotSpot(hs);
		list->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(list);
				list->parent(parent);
			}
		}

		addChild(idCtrl, list);
	}
}// getControlList //


//-----------------------------------------------
// getSpellList :
//
//-----------------------------------------------
void COSD::getSpellList(uint idCtrl)
{
	uint16	spacing = 0;
	uint	idSpellPen = 0 ;
	uint	idCommentPen = 0 ;
	float	buttonW = 0;
	float	buttonH = 0;
	uint	leftFunc = 0;
	uint	rightFunc = 0;
	float	line_H = 0;
	float	line_H_Pixel = 0;
	uint upTexture = 0;
	uint downTexture = 0;
	uint scrollBarTexture = 0;
	CRGBA scrollBarRgba(255,255,255,255);

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Spacing.
		else if(strcmp(ptr, "Spacing:") == 0)
			spacing = getInt();
		// Get the Pen.
		else if(strcmp(ptr, "SpellPen:") == 0)
			idSpellPen = getInt();
		// Get the Pen.
		else if(strcmp(ptr, "CommentPen:") == 0)
			idCommentPen = getInt();
		// Get the button width
		else if(strcmp(ptr, "ButtonW:") == 0)
			buttonW = getFloat();
		// Get the button width
		else if(strcmp(ptr, "ButtonH:") == 0)
			buttonH = getFloat();
		// Get the function to execute on left click on candidate name
		else if(strcmp(ptr, "LeftClickFunction:") == 0)
			leftFunc = getInt();
		// Get the function to execute on left click on candidate name
		else if(strcmp(ptr, "RightClickFunction:") == 0)
			rightFunc = getInt();
		// Get the height of a line in the control (relative to the size of the list)
		else if(strcmp(ptr, "Line_H:") == 0)
			line_H = getFloat();
		// Get the height of a line in the control (in pixels)
		else if(strcmp(ptr, "Line_H_Pixel:") == 0)
			line_H_Pixel = getFloat();
		// Get the Texture for the 'up' arrow
		else if(strcmp(ptr, "UpTexture:") == 0)
			upTexture = getInt();
		// Get the Texture for the 'down' arrow
		else if(strcmp(ptr, "DownTexture:") == 0)
			downTexture = getInt();
		// Get the Texture for the scroll bar body
		else if(strcmp(ptr, "ScrollBarTexture:") == 0)
			scrollBarTexture = getInt();
		// Get the RGBA for the scroll bar
		else if(strcmp(ptr, "ScrollBarRGBA:") == 0)
			scrollBarRgba = getRGBA();
	)

	// Create the list.
	CSpellList *list = new CSpellList(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, spacing , leftFunc, rightFunc);

	if(list)
	{
		CScrollBar *scroll = list->getVScroll();
		if (scroll)
		{
			scroll->setDownArrowTextureOn( downTexture );
			scroll->setUpArrowTextureOn( upTexture );
			scroll->textureOn( scrollBarTexture );
			scroll->setDownArrowTextureOff( downTexture );
			scroll->setUpArrowTextureOff( upTexture );
			scroll->textureDisable( scrollBarTexture );
			scroll->colorOn( scrollBarRgba );
			scroll->enable(true);
		}

		list->setLineHeight( line_H, line_H_Pixel);
		list->setButtonParam( buttonW, buttonH, CInterfMngr::getButton(idCtrl) );

		list->setCommentPen( CInterfMngr::getPen(idCommentPen) );
		list->setSpellPen( CInterfMngr::getPen(idSpellPen) );

		list->origin(origin);
		list->hotSpot(hs);
		list->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(list);
				list->parent(parent);
			}
		}

		addChild(idCtrl, list);
	}
}// getSpellList //



//-----------------------------------------------
// getProgressBar :
//-----------------------------------------------
void COSD::getProgressBar(uint idCtrl)
{
	uint bkgTexture = 0;
	uint barTexture = 0;
	uint idPen = 0;
	uint idText = 0;


	uint32 range = 100;
	uint32 step = 0;

	CRGBA bkgRgba(255,255,255,255);
	CRGBA barRgba(255,255,255,255);

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Texture for the progress bar
		else if(strcmp(ptr, "BarTexture:") == 0)
			barTexture = getInt();
		// Get the Texture for the background
		else if(strcmp(ptr, "BackgroundTexture:") == 0)
			bkgTexture = getInt();
		// Get the RGBA for the progress bar
		else if(strcmp(ptr, "BarRGBA:") == 0)
			barRgba = getRGBA();
		// Get the RGBA for the background
		else if(strcmp(ptr, "BackgroundRGBA:") == 0)
			bkgRgba = getRGBA();
		// get the range
		else if(strcmp(ptr, "Range:") == 0)
			range = getInt();
		// get the step inc
		else if(strcmp(ptr, "Step:") == 0)
			step = getInt();
		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
		// Get the Text
		else if (strcmp(ptr, "Text:") == 0)
			idText = getInt();

	)

	// Create the control
	CProgressBar *ctrl = new CProgressBar(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, range);

	if(ctrl)
	{
		ctrl->origin(origin);
		ctrl->hotSpot(hs);
		ctrl->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		ctrl->setStep( step );
		ctrl->setBackgroundColor( bkgRgba );
		ctrl->setBackgroundTexture( bkgTexture );
		ctrl->setProgressBarColor( barRgba );
		ctrl->setProgressBarTexture( barTexture );

		CPen pen = CInterfMngr::getPen(idPen);
		ctrl->shadow( pen.shadow() );
		ctrl->fontSize( pen.fontSize() );
		ctrl->color( pen.color() );
		if (idText !=0)
			ctrl->setText( CInterfMngr::getText(idText).toString() );

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(ctrl);
				ctrl->parent(parent);
			}
		}

		addChild(idCtrl, ctrl);
	}
}// getProgressBar //




//-----------------------------------------------
// getCastingBar :
//-----------------------------------------------
void COSD::getCastingBar(uint idCtrl)
{
	uint bkgTexture = 0;
	uint barTexture = 0;
	uint idPen = 0;
	uint idText = 0;


	uint32 range = 100;
	uint32 step = 0;

	CRGBA bkgRgba(255,255,255,255);
	CRGBA barRgba(255,255,255,255);

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Texture for the progress bar
		else if(strcmp(ptr, "BarTexture:") == 0)
			barTexture = getInt();
		// Get the Texture for the background
		else if(strcmp(ptr, "BackgroundTexture:") == 0)
			bkgTexture = getInt();
		// Get the RGBA for the progress bar
		else if(strcmp(ptr, "BarRGBA:") == 0)
			barRgba = getRGBA();
		// Get the RGBA for the background
		else if(strcmp(ptr, "BackgroundRGBA:") == 0)
			bkgRgba = getRGBA();
		// get the range
		else if(strcmp(ptr, "Range:") == 0)
			range = getInt();
		// get the step inc
		else if(strcmp(ptr, "Step:") == 0)
			step = getInt();
		// Get the Pen.
		else if(strcmp(ptr, "Pen:") == 0)
			idPen = getInt();
		// Get the Text
		else if (strcmp(ptr, "Text:") == 0)
			idText = getInt();

	)

	// Create the control
	CCastingBar *ctrl = new CCastingBar(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, range);

	if(ctrl)
	{
		ctrl->origin(origin);
		ctrl->hotSpot(hs);
		ctrl->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		ctrl->setStep( step );
		ctrl->setBackgroundColor( bkgRgba );
		ctrl->setBackgroundTexture( bkgTexture );
		ctrl->setProgressBarColor( barRgba );
		ctrl->setProgressBarTexture( barTexture );

		CPen pen = CInterfMngr::getPen(idPen);
		ctrl->shadow( pen.shadow() );
		ctrl->fontSize( pen.fontSize() );
		ctrl->color( pen.color() );
		if (idText !=0)
			ctrl->setText( CInterfMngr::getText(idText).toString() );

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(ctrl);
				ctrl->parent(parent);
			}
		}

		addChild(idCtrl, ctrl);
	}
}// getCastingBar //



//-----------------------------------------------
// getBrickControl :
//
//-----------------------------------------------
void COSD::getBrickControl(uint idCtrl)
{
	uint idtexture = 0;
	bool tiled = false;
	CRGBA rgba;
	uint idText = 0;
	uint leftFunc = 0;
	uint rightFunc = 0;

	// Lecture de la partie commune du script de control.
	CONTROL_SCRIPT_MACRO
	(
		// Get the Texture.
		else if(strcmp(ptr, "Texture:") == 0)
			idtexture = getInt();
		// Get the RGBA.
		else if(strcmp(ptr, "RGBA:") == 0)
			rgba = getRGBA();

/*		// Get the function to execute on left click
		else if(strcmp(ptr, "LeftClickFunction:") == 0)
			leftFunc = getInt();
		// Get the function to execute on left click
		else if(strcmp(ptr, "RightClickFunction:") == 0)
			rightFunc = getInt();

		// tiled or stretched
		else if(strcmp(ptr, "Tiled:") == 0)
		{
			if (getInt() ==1)
				tiled = true;
		}
*/
	)

	// Create the control.
	CBrickControl *brick = new CBrickControl(idCtrl, x, y, xPixel, yPixel, w, h, wPixel, hPixel, leftFunc, rightFunc);
	if(brick)
	{
		brick->origin(origin);
		brick->hotSpot(hs);
		brick->ref(_X_Display, _Y_Display, _W_Display, _H_Display);

		if(idParent != 0)
		{
			CControl *parent = CInterfMngr::getCtrl(idParent);
			if(parent)
			{
				parent->addChild(brick);
				brick->parent(parent);
			}
		}



		addChild(idCtrl, brick);
	}
}// getBrickControl //





//-----------------------------------------------
// open :
//
//-----------------------------------------------
void COSD::open(ifstream &file)
{
	char tmpBuff[_MAX_LINE_SIZE];
	char delimiter[] = "[] \t";
	uint line = 0;

	// While it's not the end of the file.
	while(!file.eof())
	{
		file.getline(tmpBuff, _MAX_LINE_SIZE);
		line++;

		char *key = strtok(tmpBuff, delimiter);

		// if the first char is a / then this is a comment line, skip it and go to next line
		if ((key != NULL) && (*key) != '/' )
		{
			// Make the id.
			uint idCtrl = atoi(key);
			switch(CInterfMngr::getType(idCtrl))
			{
			// The control is a Text.
			case CInterfMngr::CtrlText:
				getText(idCtrl);
				break;

			// The control is a Capture.
			case CInterfMngr::CtrlCapture:
				getCapture(idCtrl);
				break;

			// The control is a Button.
			case CInterfMngr::CtrlButton:
				getButton(idCtrl);
				break;

			// The control is a Radio Button.
			case CInterfMngr::CtrlRadioButton:
				getRadioButton(idCtrl);
				break;

			// The control is a Radio Controller
			case CInterfMngr::CtrlRadioController:
				getRadioController(idCtrl);
				break;

			// The control is a Bitmap.
			case CInterfMngr::CtrlBitmap:
				getBitmap(idCtrl);
				break;

			// The control is a List.
			case CInterfMngr::CtrlList:
				getList(idCtrl);
				break;

			// The control is a MultiList.
			case CInterfMngr::CtrlMultiList:
				getMultiList(idCtrl);
				break;

			// The control is a ChatBox.
			case CInterfMngr::CtrlChat:
				getChatBox(idCtrl);
				break;

			// The control is a ChatInput.
			case CInterfMngr::CtrlChatInput:
				getChatInput(idCtrl);
				break;

			// The control is a ChoiceList
			case CInterfMngr::CtrlChoiceList:
				getChoiceList(idCtrl);
				break;

			// The control is a CandidateList
			case CInterfMngr::CtrlCandidateList:
				getCandidateList(idCtrl);
				break;

			// The control is an HorizontalList
			case CInterfMngr::CtrlHorizontalList:
				getHorizontalList(idCtrl);
				break;

			// The control is an ControlList
			case CInterfMngr::CtrlControlList:
				getControlList(idCtrl);
				break;

			// The control is an SpellList
			case CInterfMngr::CtrlSpellList:
				getSpellList(idCtrl);
				break;

			// The control is an ProgressBar
			case CInterfMngr::CtrlProgressBar:
				getProgressBar(idCtrl);
				break;

			// The control is an CastingBar
			case CInterfMngr::CtrlCastingBar:
				getCastingBar(idCtrl);
				break;

			// The control is a brick control
			case CInterfMngr::CtrlBrick:
				getBrickControl(idCtrl);
				break;

			// The control is a Unknown.
			default:
				nlerror("Line %d : ID %d is undeclared in \"ctrls.txt\" OR the type is unknown !!", line, idCtrl);
				break;
			}
		}
	}

	uint32 width, height;
	CInterfMngr::getWindowSize(width, height);
	resize(width, height);
}// open //



