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
#include "chat_control.h"
#include "interfaces_manager.h"
//
#include <stdlib.h> // for rand()
#include <time.h>


///////////
// Using //
///////////
using namespace NL3D;
using namespace NLMISC;
using namespace std;


////////////
// Extern //
////////////
extern UDriver *Driver;
extern UTextContext *TextContext;


/*
 * Constructor
 */
CChatControl::CChatControl(uint id)
: CMultiList(id, 2)
{
	init(0,0);
}

//-----------------------------------------------
// CChatControl :
// Constructor.
//-----------------------------------------------
CChatControl::CChatControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint leftFunc, uint rightFunc, const CPen &pen)
: CMultiList(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, pen, 2)
{
	init(leftFunc, rightFunc);
}// CMultiList //


//-----------------------------------------------
// CMultiList :
// Constructor.
//-----------------------------------------------
CChatControl::CChatControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint leftFunc, uint rightFunc, uint32 fontSize, CRGBA color, bool shadow)
: CMultiList(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel, fontSize, color, shadow, 2)
{
	init(leftFunc, rightFunc);
}// CMultiList //


//-----------------------------------------------
// init :
//-----------------------------------------------
void CChatControl::init(uint left, uint right)
{
	_LeftClickFunction = left;
	_RightClickFunction = right;

	_HistorySize = 200; // memorize the last 200 lines
	_AutoScroll = true;

	const float size = static_cast<float> (1.0/_NbCol);
	_ColSize[0] = size/2;
	_ColSize[1] = static_cast<float> (size * 1.5);

	_LastStringHeight = 0;
	_MouseX = 0;
	_MouseY = 0;

	_MutedColor =			NLMISC::CRGBA(100,100,100);
	_MutedHighlightColor =	NLMISC::CRGBA(100,200,200);
	_HighlightColor =		NLMISC::CRGBA(255,255,255);
	_SysTextColor =			NLMISC::CRGBA(255,255,255);

	_UsedColors.insert(_MutedColor);
	_UsedColors.insert(_MutedHighlightColor);
	_UsedColors.insert(_HighlightColor);
	_UsedColors.insert(_SysTextColor);

	_SystemDisplayText = ucstring("");
	_CommandSid = CEntityId();	// \todo GUIGUI : REMOVE SID.

	_SelectedPlayer		= NULL;
	_SelectedPlayerSid  = NULL;

	/* Seed the random-number generator with current time so that
    * the numbers will be different every time we run.
    */
   srand( (unsigned)time( NULL ) );

   // map the command dummy NLMISC::CEntityId with the display color
	_PlayersColor.insert( std::make_pair( _CommandSid, &_SysTextColor ) );
}// init //



//-----------------------------------------------
// clear :
//-----------------------------------------------
void CChatControl::clear()
{
	_ItemsList.clear();
	_PlayersSid.clear();
	_EndingIterator = _ItemsList.rbegin();
	_EndingSidIterator = _PlayersSid.rbegin();
}// clear //



//-----------------------------------------------
// add :(private)
// DEPRECATED, do not use this function
//-----------------------------------------------
void CChatControl::add(const std::list<ucstring> &strList)
{
}// add //


//-----------------------------------------------
// add :
// add text from player
//-----------------------------------------------
void CChatControl::add( const NLMISC::CEntityId &sid, const ucstring &name, const ucstring &text)
{
	/// \toto Malkav: ensure the name is different from system messages displayed text (_SystemDisplayText)


	// if the player is muted, ignore this phrase
	if ( _MutedPlayers.find( sid ) != _MutedPlayers.end() )
		return;

	// make a list with the name and text
	static std::list<ucstring> ls;
	ls.clear();
	ls.push_back( name );
	ls.push_back( text );

	// add the NLMISC::CEntityId to the list of NLMISC::CEntityId
	_PlayersSid.push_back( sid );

	// if the max number of line isn't reached
	if (  ( _HistorySize > _ItemsList.size() ) || ( _HistorySize == 0) )
	{
		// add the new string
		_ItemsList.push_back( ls );

		// if this is the first string, init the ending point
		if ( _ItemsList.size() == 1)
		{
			_EndingIterator = _ItemsList.rbegin();
			_EndingSidIterator = _PlayersSid.rbegin();
		}
	}
	// max number reached, delete the oldest line and insert the new one
	else
	{
		// remove the oldest string
		_ItemsList.pop_front();
		// remove the oldest NLMISC::CEntityId
		_PlayersSid.pop_front();

		// add the new one
		_ItemsList.push_back( ls );

		if ( (_AutoScroll) || (_ItemsList.size() == 1) )
		{
			_EndingIterator = _ItemsList.rbegin();
			_EndingSidIterator = _PlayersSid.rbegin();
		}
	}

	// if this a an unknown player:
	if ( _PlayersColor.find( sid ) == _PlayersColor.end() )
	{
		// get a new display color for that player
		static CRGBA color;
		color = getNewColor();

		// add the color to the set of existing color
		std::pair<TSetColors::iterator, bool> ret = _UsedColors.insert( color );
		//nlassert(ret.second == true);
		if (ret.second != true)
		{
			nlwarning("<CChatControl::add> failed to insert color in _UsedColors !");
		}

		// map the NLMISC::CEntityId with the display color
		_PlayersColor.insert( std::make_pair( sid, &(*(ret.first)) ) );
	}
}


//-----------------------------------------------
// add :
// add system text
//-----------------------------------------------
void CChatControl::add(const ucstring &text)
{

// make a list with the name and text
	static std::list<ucstring> ls;
	ls.clear();
	ls.push_back( _SystemDisplayText );
	ls.push_back( text );

	// add the dummy NLMISC::CEntityId to the list of NLMISC::CEntityId
	_PlayersSid.push_back( _CommandSid );

	// if the max number of line isn't reached
	if (  ( _HistorySize > _ItemsList.size() ) || ( _HistorySize == 0) )
	{
		// add the new string
		_ItemsList.push_back( ls );

		// if this is the first string, init the ending point
		if ( _ItemsList.size() == 1)
		{
			_EndingIterator = _ItemsList.rbegin();
			_EndingSidIterator = _PlayersSid.rbegin();
		}
	}
	// max number reached, delete the oldest line and insert the new one
	else
	{
		// remove the oldest string
		_ItemsList.pop_front();
		// remove the oldest NLMISC::CEntityId
		_PlayersSid.pop_front();

		// add the new one
		_ItemsList.push_back( ls );

		if ( (_AutoScroll) || (_ItemsList.size() == 1) )
		{
			_EndingIterator = _ItemsList.rbegin();
			_EndingSidIterator = _PlayersSid.rbegin();
		}
	}
}// add //

//-----------------------------------------------
// display :
// Display the control.
//-----------------------------------------------
void CChatControl::display()
{
	// \todo GUIGUI : remove this damn thing after the UBI demo.
	const CRGBA playerColors[10] =
	{
		CRGBA(250,250, 10),	// 0 : Jaune
		CRGBA( 50,200, 50),	// 1 : Vert
		CRGBA(  0,  0,255),	// 2 : Bleu
		CRGBA(255,100,  0),	// 3 : Orange
		CRGBA( 64,  0,128),	// 4 : Violet
		CRGBA(128,128,  0),	// 5 : Verdatre
		CRGBA(128,  0, 64),	// 6 : Pourpre
		CRGBA(255,128,192),	// 7 : Rose
		CRGBA(  0,255,255),	// 8 : Bleu ciel
		CRGBA(100,100,100)	// 9 : Gris
	};

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

	// Draw each Background
/*	float w = 0 , x_display = _X_Display;

	for (uint j = 0 ;  j < _NbCol ; ++j)
	{
		w = _ColSize[j] * wDisplay;
		Driver->drawBitmap(x_display , yDisplay, w, hDisplay, *CInterfMngr::getTexture(_BackgroundTexture), true, _BackgroundColor);
		x_display += w;
	}
*/

	/// \todo GUIGUI : initialize the scissor with oldScissor and remove tmp variables.
	// Backup scissor and create the new scissor to clip the list correctly.
	CScissor oldScissor = Driver->getScissor();
	CScissor scissor;

	float scisX, scisY, scisWidth, scisHeight;
	scisX		= oldScissor.X;
	scisY		= oldScissor.Y;
	scisWidth	= oldScissor.Width;
	scisHeight	= oldScissor.Height;

	float xtmp = _X_Display + _ColSize[0] * wDisplay;
	float ytmp = _Y_Display + _H_Display;
	float xscistmp = scisX + scisWidth;
	float yscistmp = scisY + scisHeight;
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
	TextContext->setShaded(_Shadow);
	TextContext->setFontSize(_FontSize);
	TextContext->setHotSpot(UTextContext::BottomLeft);

	uint32 index;
	UTextContext::CStringInfo info;

	// if the ending item is the last item (so the first item in reverse order), hide/disable scrollbar
//	if (_EndingIterator == _ItemsList.rbegin() )
//		vScroll( false );

	static std::list<ucstring>::const_iterator	itText;
	static NLMISC::CEntityId * sid = NULL;

	TItemList::reverse_iterator	ritList;
	TSidList::reverse_iterator	ritSidList = _EndingSidIterator;

	const TItemList::reverse_iterator  ritEndList = _ItemsList.rend();
	const TMapSidToColor::iterator itColorEnd = _PlayersColor.end();

	float maxH = 0;
	float y = _Y_Display + _LastStringHeight/3.0f; // start a bit upper to have room to print a line
	float x = _X_Display;
	const float yLimit = yDisplay + hDisplay;
	const float xLimit = _X_Display + wDisplay;

	for( ritList = _EndingIterator ; ritList != ritEndList ; ++ritList)
	{
		maxH = 0;
		x = _X_Display;

		itText = (*ritList).begin();
		sid = const_cast<NLMISC::CEntityId*> (&(*ritSidList));

		// Test mouse position.
		if ( (_MouseY >= y) && (_MouseY <= y + _LastStringHeight) && (_MouseX >= x) && (_MouseX <= xLimit))
		{
			// if player is muted
			if ( (!_MutedPlayers.empty() ) && ( _MutedPlayers.find( *sid ) != _MutedPlayers.end() ) )
			{
				TextContext->setColor( _MutedHighlightColor );
			}
			else
				TextContext->setColor( _HighlightColor );
		}
		else
		{
			// if player is muted
			if ( (!_MutedPlayers.empty() ) && ( _MutedPlayers.find( *sid ) != _MutedPlayers.end() ) )
			{
				TextContext->setColor( _MutedColor );
			}
			else
			{
				// \todo GUIGUI : remove those thing after UBI demo.
				// Get display color for that player.
				TMapSidToColor::iterator itColor = _PlayersColor.find( *sid );
				if(itColor != itColorEnd)
					TextContext->setColor(playerColors[((*sid).Id / 10)%10]);
				else
					nlwarning("CChatControl::display : Player does not exist.");
//				TextContext->setColor( *(*itColor).second );
			}
		}

		// display player name
		// if this is a command line, then, use SysColor;
		if ( (*sid) == _CommandSid )
			TextContext->setColor( _SysTextColor );

		index = TextContext->textPush( *itText );
		info = TextContext->getStringInfo(index);
		info.convertTo01Size(Driver);
		if (maxH < info.StringHeight)
			maxH = info.StringHeight;

		TextContext->printAt(x, y, index);
		TextContext->erase(index);
		x += _ColSize[0] * wDisplay;

		// insert player names and NLMISC::CEntityId coordinates in the _NamesYPos map (only if it's not a system message)
		if ( (*sid) != _CommandSid)
			_NamesYPos.push_back( std::make_pair( std::make_pair(y, y+info.StringHeight), std::make_pair( &(*itText), sid)   ) );

		++ itText;

		// set the new scissor
		scisX		= oldScissor.X;
		scisWidth	= oldScissor.Width;

		xtmp = _X_Display + _W_Display;
		xscistmp = scisX + scisWidth;

		if(xtmp<xscistmp)
			scisWidth = xtmp-scisX;
		else
			scisWidth = xscistmp-scisX;

		scissor.init(scisX, scisY, scisWidth, scisHeight);
		Driver->setScissor(scissor);

		// print text
		index = TextContext->textPush( *itText );
		info = TextContext->getStringInfo(index);
		info.convertTo01Size(Driver);
		if (maxH < info.StringHeight)
			maxH = info.StringHeight;

		TextContext->printAt(x, y, index);
		TextContext->erase(index);
		x += _ColSize[1] * wDisplay;

		_LastStringHeight = info.StringHeight;

		// if display boundaries are reached, enable scroll bar and stop displaying
		y += maxH;
		if(y >= yLimit)
		{
			vScroll( true );
			break;
		}

		++ ritSidList;
	}

	// Restore Scissor.
	Driver->setScissor(oldScissor);

	// draw scrollBar
	_VScroll->display();

}// display //



//-----------------------------------------------
// click :
// Manage the click of the mouse for the Button.
//-----------------------------------------------
void CChatControl::click(float x, float y, bool &taken)
{
	if (_VScroll != NULL)
		_VScroll->click(x,y,taken);

	// if click still not caught by a control
	if ( !taken)
	{
		// if user has clicked on a player ID, mute/unmute that player
		// test x coordinate
		if ( (x >= _X_Display) && ( x <= (_X_Display + _W_Display * _ColSize[0])  ) )
		{
			// find the name with the y coordinate
			bool found = searchPlayerAtPos( y ,_SelectedPlayerSid, _SelectedPlayer ) ;

			// if a player was found
			if ( found )
			{
				CInterfMngr::runFuncCtrl(_LeftClickFunction, id());
				taken = true;
			}
		}
	}

	_SelectedPlayer  = NULL;
	_SelectedPlayerSid  = NULL;
}// click //



//-----------------------------------------------
// clickRight :
// Manage the click of the mouse for the Button.
//-----------------------------------------------
void CChatControl::clickRight(float x, float y, bool &taken)
{

	if ( !taken )
	{
	// if user has right clicked on a player, open the good pop-up menu
		// if user has clicked on a player ID, mute/unmute that player
		// test x coordinate
		if ( (x >= _X_Display) && ( x <= (_X_Display + _W_Display * _ColSize[0])  ) )
		{
			// find the name with the y coordinate
			 bool found = searchPlayerAtPos( y ,_SelectedPlayerSid, _SelectedPlayer ) ;

			// if a player was found
			if ( found )
			{
				CInterfMngr::runFuncCtrl(_RightClickFunction, id());
				taken = true;
			}
		}
	}

	_SelectedPlayer  = NULL;
	_SelectedPlayerSid  = NULL;
}// clickRight //



//-----------------------------------------------
// mouseMove :
//-----------------------------------------------
void CChatControl::mouseMove( float x, float y)
{
	_MouseX = x;
	_MouseY = y;
}// mouseMove //



//-----------------------------------------------
// searchNameAtPos :
//-----------------------------------------------
bool CChatControl::searchPlayerAtPos(float y, NLMISC::CEntityId *&sid, ucstring *&name) const
{
	// allready test if y is within display limits
	if ( (y < _Y_Display)  || (y>_Y_Display + _H_Display) )
		return NULL;


	const TPairPFloatPStrSid::const_iterator	itB   = _NamesYPos.begin();
	const TPairPFloatPStrSid::const_iterator	itEnd = _NamesYPos.end();
	TPairPFloatPStrSid::const_iterator it = itB;

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
		name = const_cast<ucstring*> ( (*it).second.first );
		sid  = const_cast<NLMISC::CEntityId*>  ( (*it).second.second );
		return true;
	}

	return false;

}// searchNameAtPos //


//-----------------------------------------------
// getNewColor :
//-----------------------------------------------
CRGBA CChatControl::getNewColor() const
{
	static CRGBA color(255,0,0);

	const TSetColors::const_iterator itColorsEnd = _UsedColors.end();

	uint16 max = 0;

	while( (_UsedColors.find( color ) != itColorsEnd )&& (max <180) )
	{
		// generate a new color
		// randomly add a number beetween 10 and 200 (in absolute) to each composant (rgb)

		sint8 val = static_cast<sint8> ( (rand() - RAND_MAX/2) % 190 );
		if (val>=0) val+=10;
		else val -= 10;

		color.R += val;
		max = color.R;

		val = static_cast<sint8> ( (rand() - RAND_MAX/2) % 190 );
		if (val>=0) val+=10;
		else val -= 10;

		color.G += val;
		if( color.G > max)
			max = color.G;

		val = static_cast<sint8> ( (rand() - RAND_MAX/2) % 190 );
		if (val>=0) val+=10;
		else val -= 10;

		color.B += val;
		if( color.B > max)
			max = color.B;
	}

	return color;
}// getNewColor //



//-----------------------------------------------
// scrollV :
//-----------------------------------------------
void CChatControl::scrollV(sint32 scroll)
{
	// scrolling downward
	if (scroll < 0)
	{
		const TItemList::reverse_iterator itB = _ItemsList.rbegin();

		while ( ( _EndingIterator != itB) && (scroll != 0) )
		{
			--_EndingIterator;
			--_EndingSidIterator;
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
			++_EndingSidIterator;
			--scroll;
		}
	}
}// scrollV //