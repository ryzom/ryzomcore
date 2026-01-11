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
#include "multi_list.h"
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
// CMultiList :
// Constructor.
//-----------------------------------------------
CMultiList::CMultiList(uint id, uint16 nbCol)
: CScrollableControl(id)
{
	_NbCol = nbCol;
	init();
}// CMultiList //

//-----------------------------------------------
// CMultiList :
// Constructor.
//-----------------------------------------------
CMultiList::CMultiList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, const CPen &pen, uint16 nbCol)
: CScrollableControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CPen(pen)
{
	_NbCol = nbCol;
	init();
}// CMultiList //


//-----------------------------------------------
// CMultiList :
// Constructor.
//-----------------------------------------------
CMultiList::CMultiList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint32 fontSize, CRGBA color, bool shadow, uint16 nbCol)
: CScrollableControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel), CPen(fontSize, color, shadow)
{
	_NbCol = nbCol;
	init();
}// CMultiList //


//-----------------------------------------------
// init :
//-----------------------------------------------
void CMultiList::init()
{
	_HistorySize = 150; // memorize the last 150 lines
	_AutoScroll = true;
	_EndingIterator = _ItemsList.rbegin();
	_Spacing = 1;
	_LineHeight = 0;

	// draw the vertical scroll bar
	vScroll( true );

	const float size = static_cast<float> (1.0/_NbCol);
	_ColSize.assign( _NbCol, size);
}// init //


//-----------------------------------------------
// add :
// ...
//-----------------------------------------------
void CMultiList::add(const std::list<ucstring> &strList)
{
	nlassert(strList.size() == _NbCol);

	// if the max number of line isn't reached
	if (  ( _HistorySize > _ItemsList.size() ) || ( _HistorySize == 0) )
	{
		// add the new string
		_ItemsList.push_back(strList);

		// if this is the first string, init the ending point
		if ( _ItemsList.size() == 1)
			_EndingIterator = _ItemsList.rbegin();
	}
	// max number reached, delete the oldest line and insert the new one
	else
	{
		// remove the oldest string
		_ItemsList.pop_front();

		// add the new one
		_ItemsList.push_back(strList);

		if ( (_AutoScroll) || (_ItemsList.size() == 1) )
			_EndingIterator = _ItemsList.rbegin();
	}
}// add //


//-----------------------------------------------
// display :
// Display the Button.
//-----------------------------------------------
void CMultiList::display()
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


	// Display the text of the list.
	// compute the spacing
	static uint32 windowW, windowH;
	CInterfMngr::getWindowSize(windowW, windowH);
	const float spacing = float(_Spacing) / windowH;
	const float lineHeight = float(_LineHeight) / windowH;

	TextContext->setShaded(_Shadow);
	TextContext->setFontSize(_FontSize);
	TextContext->setColor(_Color);
	TextContext->setHotSpot(UTextContext::BottomLeft);

	uint32 index;
	UTextContext::CStringInfo info;

	// if the ending item is the last item (so the first item in reverse order), hide/disable scrollbar
	if (_EndingIterator == _ItemsList.rbegin() )
		vScroll( false );

	std::list<ucstring>::const_iterator	itText, itTextEnd;

	TItemList::reverse_iterator	ritList;
	const TItemList::reverse_iterator  ritEndList = _ItemsList.rend();

	uint i = 0;
	float maxH = 0;
	float y = yDisplay + 5.0f/windowH; // start 5 pixels above the bottom
	float x = _X_Display;
	const float yLimit = yDisplay + hDisplay;
	const float xLimit = _X_Display + wDisplay;

	for( ritList = _EndingIterator ; ritList != ritEndList ; ++ritList)
	{
		itTextEnd = (*ritList).end();

		i = 0;
		maxH = 0;
		x = _X_Display;

		for (itText = (*ritList).begin() ; itText != itTextEnd ; ++itText)
		{
			index = TextContext->textPush( *itText );
			if ( lineHeight == 0)
			{
				info = TextContext->getStringInfo(index);
				info.convertTo01Size(Driver);
				if (maxH < info.StringHeight)
					maxH = info.StringHeight;
			}

			TextContext->printAt(x, y, index);
			TextContext->erase(index);
			x += _ColSize[i] * wDisplay;
			++i;
		}
		// the line height
		if ( lineHeight == 0)
			y += maxH;
		else
			y += lineHeight;

		// the inter line space
		y += spacing;

		// if some lines are hidden, show/enable scroll bar
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
// scrollH :
//-----------------------------------------------
void CMultiList::scrollH(sint32 scroll)
{
}// scrollH //




//-----------------------------------------------
// scrollV :
//-----------------------------------------------
void CMultiList::scrollV(sint32 scroll)
{
	// scrolling downward
	if (scroll < 0)
	{
		const TItemList::reverse_iterator itB = _ItemsList.rbegin();

		while ( ( _EndingIterator != itB) && (scroll != 0) )
		{
			--_EndingIterator;
			++scroll;
		}
	}
	// scrolling upward
	else
	{
		const TItemList::reverse_iterator itE = --_ItemsList.rend();

		while ( ( _EndingIterator != itE) && (scroll != 0) )
		{
			++_EndingIterator;
			--scroll;
		}
	}
}// scrollV //



//-----------------------------------------------
// setColSize :
//-----------------------------------------------
void CMultiList::setColSize( const std::vector<float> &size)
{
	nlassert( size.size() == _NbCol);

	_ColSize = size;
}// setColSize //


//-----------------------------------------------
// clear :
//-----------------------------------------------
void CMultiList::clear()
{
	_ItemsList.clear();
	_EndingIterator = _ItemsList.rbegin();
}// clear //



