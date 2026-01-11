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
// Client
#include "text.h"
#include "interfaces_manager.h"


////////////////
// Namespaces //
////////////////
using namespace NL3D;


/////////////
// Externs //
/////////////
extern UTextContext *TextContext;

uint32 inin;

//-----------------------------------------------
// CText :
// Constructor.
//-----------------------------------------------
CText::CText(uint id)
: CControl(id)
{
	init(ucstring(""));
}// CText //

//-----------------------------------------------
// CText :
// Constructor.
//-----------------------------------------------
CText::CText(uint id, float x, float y, float x_pixel, float y_pixel, const ucstring &text, const CPen &pen)
: CControl(id, x, y, x_pixel, y_pixel, 0.f, 0.f, 0.f, 0.f), CPen(pen)
{
	init(text);
}// CText //

//-----------------------------------------------
// CText :
// Constructor.
//-----------------------------------------------
CText::CText(uint id, float x, float y, float x_pixel, float y_pixel, const ucstring &text, uint32 fontSize, CRGBA color, bool shadow)
: CControl(id, x, y, x_pixel, y_pixel, 0.f, 0.f, 0.f, 0.f), CPen(fontSize, color, shadow)
{
	init(text);
}// CText //

//-----------------------------------------------
// init :
// Initialize the button (1 function called for all constructors -> easier).
//-----------------------------------------------
void CText::init(const ucstring &text)
{
	calculateDisplay();
	_Text	= text;

	this->text( text );
}// init //


//-----------------------------------------------
// display :
// Display the text.
//-----------------------------------------------
void CText::display()
{
	// If the control is hide -> return
	if(!_Show)
		return;

/*	TextContext->setShaded(_Shadow);
*/	TextContext->setHotSpot(_TextHotSpot);
/*	TextContext->setColor(_Color);
	TextContext->setFontSize(_FontSize);
*/	//TextContext->printAt(_X_Display, _Y_Display, _Text);
	TextContext->printAt(_X_Display, _Y_Display, _Index);

}// display //



//-----------------------------------------------
// Get the Text.
//-----------------------------------------------
ucstring CText::text()
{
	return _Text;
}
//-----------------------------------------------
// Set the Text.
//-----------------------------------------------
void CText::text(ucstring txt)
{
	bool shadow;
	uint32 fontSize;

	// get the current text context param
	shadow = TextContext->getShaded();
	fontSize = TextContext->getFontSize();

	_Text = txt;

	TextContext->setShaded(_Shadow);
	TextContext->setColor(_Color);
	TextContext->setFontSize(_FontSize);

	TextContext->erase( _Index );

	_Index = TextContext->textPush( _Text );
	_Info = TextContext->getStringInfo( _Index);

	// restore old values
	TextContext->setShaded(shadow);
	TextContext->setFontSize(fontSize);
}


//-----------------------------------------------
// calculateDisplay :
// Calculate the Display X, Y, Width, Height.
//-----------------------------------------------
void CText::calculateDisplay()
{
	uint32 w, h;
	CInterfMngr::getWindowSize(w, h);

	// Calculate the HotSpot.
	calculateHS();

	_X_Display = _X_Ref + _X*_W_Ref + _X_Pixel/w;
	_Y_Display = _Y_Ref + _Y*_H_Ref + _Y_Pixel/h;

	// Calculate the display Width and Height.
	_W_Display = _Info.StringWidth / w;
	_H_Display = _Info.StringHeight / h;

	_W_Pixel = _Info.StringWidth;
	_H_Pixel = _Info.StringHeight;
}// calculateDisplay //

//-----------------------------------------------
// calculateHS :
// Calculate the display position of the control in relation to the position of the control (Hot Spot).
//-----------------------------------------------
void CText::calculateHS()
{
	switch(_HotSpot)
	{
	case HS_TL:
		_TextHotSpot = UTextContext::BottomRight;
		break;
	case HS_TM:
		_TextHotSpot = UTextContext::MiddleBottom;
		break;
	case HS_TR:
		_TextHotSpot = UTextContext::BottomLeft;
		break;

	case HS_ML:
		_TextHotSpot = UTextContext::MiddleRight;
		break;
	case HS_MM:
		_TextHotSpot = UTextContext::MiddleMiddle;
		break;
	case HS_MR:
		_TextHotSpot = UTextContext::MiddleLeft;
		break;

	case HS_BL:
		_TextHotSpot = UTextContext::TopRight;
		break;
	case HS_BM:
		_TextHotSpot = UTextContext::MiddleTop;
		break;
	case HS_BR:
		_TextHotSpot = UTextContext::TopLeft;
		break;
	}
}// calculateHS //


//-----------------------------------------------
// fontSize :
// Set the font size.
//-----------------------------------------------
void CText::fontSize(uint32 fs)
{
	_FontSize = fs;
	text( _Text );
}// fontSize //

//-----------------------------------------------
// color :
// Set the pen color.
//-----------------------------------------------
void CText::color(CRGBA color)
{
	_Color = color;
	text( _Text );
}// color //

//-----------------------------------------------
//  shadow:
// Set the shadow state.
//-----------------------------------------------
void CText::shadow(bool s)
{
	_Shadow = s;
	text( _Text );
}// shadow //

