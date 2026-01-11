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
#include "candidate_list.h"
#include "interfaces_manager.h"


///////////
// Using //
///////////
using namespace NL3D;
using namespace NLMISC;


////////////
// Extern //
////////////
extern UDriver *Driver;
extern UTextContext *TextContext;




//-----------------------------------------------
// CCandidateList :
// Constructor.
//-----------------------------------------------
CCandidateList::CCandidateList(uint id)
:CMultiList(id, 2)
{
	_SelectedCandidate = NULL;
} // CCandidateList //


//-----------------------------------------------
// CCandidateList :
// Constructor.
//-----------------------------------------------
CCandidateList::CCandidateList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, const CPen &pen)
:CMultiList(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, pen, 2)
{
	_SelectedCandidate = NULL;
} // CCandidateList //


//-----------------------------------------------
// CCandidateList :
// Constructor.
//-----------------------------------------------
CCandidateList::CCandidateList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint32 fontSize, CRGBA color, bool shadow)
:CMultiList(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, fontSize, color, shadow , 2)
{
	_SelectedCandidate = NULL;
} // CCandidateList //



//-----------------------------------------------
// ~CCandidateList :
// Destructor.
//-----------------------------------------------
CCandidateList::~CCandidateList()
{
} // ~CCandidateList //





//-----------------------------------------------
// display :
// Display the control.
//-----------------------------------------------
void CCandidateList::display()
{
	// clear _NamesYPos
	_NamesYPos.clear();

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
	// compute the spacing and the line height
	static uint32 windowW, windowH;
	CInterfMngr::getWindowSize(windowW, windowH);
	const float spacing = float(_Spacing) / windowH;
	const float lineHeight = float(_LineHeight) / windowH;

	TextContext->setShaded(_Shadow);
	TextContext->setFontSize(_FontSize);
	TextContext->setHotSpot(UTextContext::BottomLeft);

	uint32 index;
	UTextContext::CStringInfo info;

	// if the ending item is the last item (so the first item in reverse order), hide/disable scrollbar
//	if (_EndingIterator == _ItemsList.rbegin() )
//		vScroll( false );

	static std::list<ucstring>::const_iterator	itText;

	TItemList::reverse_iterator	ritList;

	const TItemList::reverse_iterator  ritEndList = _ItemsList.rend();

	float maxH = 0;
	float y = _Y_Display + 5.0f/windowH; // start 5 pixels above list bottom
	float x = _X_Display;
	const float yLimit = yDisplay + hDisplay;
	const float xLimit = _X_Display + wDisplay;

	for( ritList = _EndingIterator ; ritList != ritEndList ; ++ritList)
	{
		maxH = 0;
		x = _X_Display;

		itText = (*ritList).begin();


		// display candidate name
		index = TextContext->textPush( *itText );
		info = TextContext->getStringInfo(index);
		info.convertTo01Size(Driver);
		if ( lineHeight == 0)
		{
			if (maxH < info.StringHeight)
				maxH = info.StringHeight;
		}
		else
			maxH = lineHeight;

		// if it's the selected candidate, draw a bitmap
		if ( _SelectedCandidate != NULL && ((*itText) == *_SelectedCandidate) )
		{
			Driver->drawQuad(_X_Display, y - 5.0f/windowH, _X_Display+wDisplay, y+info.StringHeight+3.0f/windowH, _SelectedColor );
		}

		TextContext->printAt(x, y, index);
		TextContext->erase(index);
		x += _ColSize[0] * wDisplay;

		// insert candidate names and Sid coordinates in the _NamesYPos map (only if it's not a system message)
		_NamesYPos.push_back( std::make_pair( std::make_pair(y, y+info.StringHeight),  &(*itText)   ) );

		++ itText;

		// print text
		index = TextContext->textPush( *itText );
		if ( lineHeight == 0)
		{
			info = TextContext->getStringInfo(index);
			info.convertTo01Size(Driver);
			if (maxH < info.StringHeight)
				maxH = info.StringHeight;
		}
		else
			maxH = lineHeight;

		TextContext->printAt(x, y, index);
		TextContext->erase(index);
		x += _ColSize[1] * wDisplay;


		// if display boundaries are reached, enable scroll bar and stop displaying
		y += maxH;
		y += spacing;

		if(y >= yLimit)
		{
			vScroll( true );
			break;
		}
	}

	// Restore Scissor.
	Driver->setScissor(oldScissor);

	// draw scrollBar
	_VScroll->display();
}// display //



//-----------------------------------------------
// click :
// Manage left mouse click
//-----------------------------------------------
void CCandidateList::click(float x, float y, bool &taken)
{
	if (_VScroll != NULL)
		_VScroll->click(x,y,taken);

	// if click still not caught by a control
	if ( !taken)
	{
		// test x coordinate
		if ( (x >= _X_Display) && ( x <= (_X_Display + _W_Display * _ColSize[0])  ) )
		{
			// find the name with the y coordinate
			bool found = searchCandidateAtPos( y , _SelectedCandidate ) ;

			// if a player was found
			if ( found )
			{
				CInterfMngr::runFuncCtrl(_LeftClickFunction, id(), (CInterfMngr::TParam) _SelectedCandidate);
				taken = true;
				return;
			}
		}
	}

//	_SelectedCandidate  = NULL;
}// click //



//-----------------------------------------------
// clickRight :
// Manage right mouse click
//-----------------------------------------------
void CCandidateList::clickRight(float x, float y, bool &taken)
{

	if ( !taken )
	{
		// test x coordinate
		if ( (x >= _X_Display) && ( x <= (_X_Display + _W_Display * _ColSize[0])  ) )
		{
			// find the name with the y coordinate
			 bool found = searchCandidateAtPos( y , _SelectedCandidate ) ;

			// if a player was found
			if ( found )
			{
				CInterfMngr::runFuncCtrl(_RightClickFunction, id(), (CInterfMngr::TParam) _SelectedCandidate);
				taken = true;
				return;
			}
		}
	}

	_SelectedCandidate = NULL;
}// clickRight //



//-----------------------------------------------
// searchCandidateAtPos :
//-----------------------------------------------
bool CCandidateList::searchCandidateAtPos(float y, ucstring *&name) const
{
	// allready test if y is within display limits
	if ( (y < _Y_Display)  || (y>_Y_Display + _H_Display) )
		return NULL;


	const TPairPFloatStr::const_iterator	itB   = _NamesYPos.begin();
	const TPairPFloatStr::const_iterator	itEnd = _NamesYPos.end();
	TPairPFloatStr::const_iterator it = itB;

	const uint32 size = _NamesYPos.size();

	if (size == 0) return NULL;

	// we know the first name is at _Y_Display and the last one at _Y_Display+_H_Display (approx.)
	// so we make an interpolation of name position in the list according to the y coordinate and the limits coordinates
	for ( uint32 start = static_cast<uint32> ( (y - _Y_Display)/(_H_Display) * size) ; start > 0 ; --start)
	{
		++it;
		if (it == itEnd)
		{
			--it;
			break;
		}
	}

	float yMin = (*it).first.first, yMax = (*it).first.second;

	while ( (y < yMin) )
	{
		if (it == itB)
		{
			return NULL;
		}
		--it;
		yMin = (*it).first.first;
	}
	while ( (y > yMax) )
	{
		++it;
		if (it == itEnd)
		{
			return NULL;
		}
		yMax = (*it).first.second;
	}

	if ( y <= yMax && y >= yMin)
	{
		name = const_cast<ucstring*> ( (*it).second );
		return true;
	}

	return false;

}// searchCandidateAtPos //