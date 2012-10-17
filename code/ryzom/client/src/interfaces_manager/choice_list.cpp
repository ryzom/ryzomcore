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
// Interface 3D
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"
// Client
#include "choice_list.h"
#include "interfaces_manager.h"


///////////
// Using //
///////////
using namespace NL3D;


////////////
// Extern //
////////////
extern UDriver *Driver;
extern UTextContext *TextContext;


//-----------------------------------------------
// CChoiceList :
// Constructor.
//-----------------------------------------------
CChoiceList::CChoiceList(uint id)
: CList(id)
{
}// CChoiceList //

//-----------------------------------------------
// CChoiceList :
// Constructor.
//-----------------------------------------------
CChoiceList::CChoiceList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, const CPen &pen)
: CList(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, pen)
{
}// CChoiceList //

//-----------------------------------------------
// CChoiceList :
// Constructor.
//-----------------------------------------------
CChoiceList::CChoiceList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint32 fontSize, CRGBA color, bool shadow)
: CList(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, fontSize, color, shadow)
{
}// CChoiceList //


//-----------------------------------------------
// display :
// Display the Button.
//-----------------------------------------------
void CChoiceList::display()
{
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


	// Display the Text of the Button.
	TextContext->setShaded(_Shadow);
	TextContext->setFontSize(_FontSize);
	TextContext->setColor(_Color);
	TextContext->setHotSpot(UTextContext::BottomLeft);

	const float yLimit = yDisplay + hDisplay;
	float y;

	uint32 index;
	UTextContext::CStringInfo info;

	// if the ending item is the last item (so the first item in reverse order), hide/disable scrollbar
	if (_EndingIterator == _ItemsList.rbegin() )
		vScroll( false );

	y = yDisplay;

	TItemList::reverse_iterator	ritList;
	const TItemList::reverse_iterator  ritEndList = _ItemsList.rend();

	for( ritList = _EndingIterator ; ritList != ritEndList ; ++ritList)
	{
		// if some lines are hidden, show/enable scroll bar
		index = TextContext->textPush( *ritList );
		info = TextContext->getStringInfo(index);
		info.convertTo01Size(Driver);

		// If the current itrator is the selected one -> draw bitmap under the text.
		if(ritList == _ItSelected)
			Driver->drawQuad(_X_Display, y, _X_Display+wDisplay, y+info.StringHeight, CRGBA(0,0,255,255));

		// Display the text.
		TextContext->printAt(_X_Display, y, index);
		TextContext->erase(index);

		// Change line.
		y += info.StringHeight;

		if(y >= yLimit)
		{
			vScroll( true );
			break;
		}
	}

	// draw scrollBar
	_VScroll->display();

	// Restore Scissor.
	Driver->setScissor(oldScissor);
}// display //


//-----------------------------------------------
// click :
// Manage the click of the mouse for the Button.
//-----------------------------------------------
void CChoiceList::click(float x, float y, bool &taken)
{
	// If the cursor is not in the list -> return.
	if((x < _X_Display) || (x >= _X_Display + _W_Display) || (y < _Y_Display) || (y >= _Y_Display+_H_Display))
		return;

	// Check the click in the scrollbar if there is one.
	if(_VScroll != NULL)
		_VScroll->click(x,y,taken);

	// If the click is already caught by another control -> return.
	if(taken)
		return;

	// Now the control take the focus.
	taken = true;

	if(_ItSelected == _ItemsList.rend())
		return;

	sint _NumSelected = (sint)(_ItemsList.size()) - 1;
	TItemList::reverse_iterator	ritList;
	const TItemList::reverse_iterator  ritEndList = _ItemsList.rend();
	for(ritList = _EndingIterator; ritList != ritEndList; ++ritList)
	{
		if(_ItSelected==ritList)
		{
			CInterfMngr::runFuncCtrl(100, id(), (CInterfMngr::TParam)_NumSelected);
			break;
		}
		_NumSelected--;
	}
}// click //

//-----------------------------------------------
// mouseMove :
// called when the mouse has moved
// \param the x coordinate of the mouse
// \param the y coordinate of the mouse
//-----------------------------------------------
void CChoiceList::mouseMove( float x, float y)
{
	_ItSelected = _ItemsList.rend();

	// If the cursor is not in the list -> return.
	if((x < _X_Display) || (x >= _X_Display + _W_Display) || (y < _Y_Display) || (y >= _Y_Display+_H_Display))
		return;

	// Check the click in the scrollbar if there is one.
//	if(_VScroll != NULL)
//		_VScroll->click(x,y,taken);

	// If the click is already caught by another control -> return.
//	if(taken)
//		return;

	// Calculate the text area size.
	float wDisplay, yDisplay, hDisplay;
	if(_VScroll->show())
		wDisplay = _W_Display - _VScroll->size();
	else
		 wDisplay = _W_Display;

	if(_HScroll->show())
	{
		 hDisplay = _H_Display - _HScroll->size();
		 yDisplay = _Y_Display + _HScroll->size();
	}
	else
	{
		 hDisplay = _H_Display;
		 yDisplay = _Y_Display;
	}

	// Cursor in the scrollbar
	if(x>_X_Display+wDisplay)
		return;

	// Display the Text of the Button.
	TextContext->setShaded(_Shadow);
	TextContext->setFontSize(_FontSize);
	TextContext->setColor(_Color);
	TextContext->setHotSpot(UTextContext::BottomLeft);

	const float yLimit = yDisplay + hDisplay;
	float yTmp = yDisplay;

	uint32 index;
	UTextContext::CStringInfo info;

	TItemList::reverse_iterator	ritList;
	const TItemList::reverse_iterator  ritEndList = _ItemsList.rend();
	for( ritList = _EndingIterator ; ritList != ritEndList ; ++ritList)
	{
		// if some lines are hidden, show/enable scroll bar
		index = TextContext->textPush( *ritList );
		info = TextContext->getStringInfo(index);
		info.convertTo01Size(Driver);
		TextContext->erase(index);

		if(y>=yTmp && y<yTmp+info.StringHeight)
		{
			_ItSelected = ritList;
			break;
		}

		// Next Line.
		yTmp += info.StringHeight;

		if(yTmp >= yLimit)
			break;
	}
}// mouseMove //
