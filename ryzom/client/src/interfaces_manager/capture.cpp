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
// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"
// Client
#include "capture.h"
#include "interfaces_manager.h"


////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;


/////////////
// Externs //
/////////////
extern UDriver		*Driver;
extern UTextContext *TextContext;


//-----------------------------------------------
// CCapture :
// Constructor.
//-----------------------------------------------
CCapture::CCapture(uint id)
: CControl(id)
{
	init(0);
}// CCapture //

//-----------------------------------------------
// CCapture :
// Constructor.
//-----------------------------------------------
CCapture::CCapture(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFunc, const CPen &pen)
: CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CPen(pen)
{
	init(numFunc);
}// CCapture //

//-----------------------------------------------
// CCapture :
// Constructor.
//-----------------------------------------------
CCapture::CCapture(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint numFunc, uint32 fontSize, CRGBA color, bool shadow)
: CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CPen(fontSize, color, shadow)
{
	init(numFunc);
}// CCapture //

//-----------------------------------------------
// ~CCapture :
// Destructor.
//-----------------------------------------------
CCapture::~CCapture()
{
	// Remove the listener.
	Driver->EventServer.removeListener(EventCharId, this);
}// ~CCapture //

//-----------------------------------------------
// init :
// Initialize the button (1 function called for all constructors -> easier).
//-----------------------------------------------
void CCapture::init(uint numFunc)
{
	//
	_NumFunc	= numFunc;
	_MaxChar	= 256;
	_Insert		= false;
	_Prompt		= "";

	// Add an event Listener for the char event.
	Driver->EventServer.addListener(EventCharId, this);
}// init //


//---------------------------------------------------
// operator() :
// Function that receives events.
//---------------------------------------------------
void CCapture::operator()(const CEvent& event)
{
	// Not in insert mode -> return (or disconnect listener).
	if(!_Insert)
		return;

	// What is the char pushed ?
	CEventChar ec = (CEventChar &) event;
	switch(ec.Char)
	{
	// SPACE
//	case KeySPACE:
//		return;
//		break;

	// RETURN
	case KeyRETURN:
		_Insert = false;
		CInterfMngr::runFuncCtrl(_NumFunc, id());
		break;

	// TAB
	case KeyTAB:
		break;

	// ESCAPE
	case KeyESCAPE:
		break;

	// BACKSPACE
	case KeyBACK:
		// delete last character
		if(_Str.size () > 0)
		{
			ucstring::iterator it = _Str.end();
			_Str.erase(--it);
		}
		break;

	// OTHER
	default:
		// If the char is not alphanumeric -> return.
//		if(!isalnum(ec.Char))
//			return;

		if(_Str.size() < _MaxChar)
			_Str += ec.Char;
		break;
	}
}// operator() //



//-----------------------------------------------
// display :
// Display the text.
//-----------------------------------------------
void CCapture::display()
{
	// If the control is hide -> return
	if(!_Show)
		return;

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


	// write text
	ucstring str(_Prompt);
	str += _Str;

	if(_Insert)
	{
		str += "_";
	}

	TextContext->setShaded(_Shadow);
	TextContext->setHotSpot(UTextContext::MiddleLeft);
	TextContext->setColor(_Color);
	TextContext->setFontSize(_FontSize);

	const uint32 index = TextContext->textPush( str );
	const float strWidth = TextContext->getStringInfo(index).StringWidth / Driver->getWindowWidth();


// display string starting by the first character if !insert or display to ensure the input prompt is visible in insert mode
	// calculate the x coordinate where to start displaying
	float xDisplay = _X_Display;
	if (_Insert)
	{
		if (strWidth > _W_Display)
			xDisplay -= (strWidth - _W_Display);
	}

	TextContext->printAt( xDisplay , _Y_Display+_H_Display/2, index);
	TextContext->erase(index);

	// Restore Scissor.
	Driver->setScissor(oldScissor);
}// display //

//-----------------------------------------------
// click :
// Manage the click for the control.
//-----------------------------------------------
void CCapture::click(float x, float y, bool &taken)
{
	_Insert = false;

	if (!taken)
	{
		// If the click is in the capture Rect.
		if(x>=_X_Display && x<=(_X_Display+_W_Display) && y>=_Y_Display && y<=(_Y_Display+_H_Display))
		{
			_Insert = true;
			taken = true;
		}
	}
}// click //
