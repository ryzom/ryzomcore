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

#include "scrollable_control.h"


/*
 * default Constructor
 */
CScrollableControl::CScrollableControl(uint id)
: CControl(id)
{
	_HScroll = NULL;
	_VScroll = NULL;
	_AutoHide = false;
	_Show = !_AutoHide;
}

/*
 * Destructor
 */
CScrollableControl::~CScrollableControl()
{
	if ( _HScroll != NULL )
		delete _HScroll;

	if ( _VScroll != NULL )
		delete _VScroll;
}


/**
 * hScroll
 */
void CScrollableControl::hScroll( bool on)
{
	// if we want to show a HScroll bar and that it doesn't exist, build a new one
	if ( (on == true) && ( _HScroll == NULL) )
	{
		_HScroll = new CScrollBar(0, 0, 0, 0, 0, _W , 0, _W_Pixel , 16, false, this);
		_HScroll->hotSpot( CControl::THotSpot::HS_TR );
		_HScroll->origin( CControl::THotSpot::HS_BL );

		_Children.push_back( _HScroll );
	}
	// hide the scroll bar
	else if ( (on == false) && ( _HScroll != NULL) )
	{
		if (_AutoHide == true)
			_HScroll->show( false );
		else
			_HScroll->enable( false );
	}
	// show the scroll bar
	else if ( (on == true)&& ( _HScroll != NULL) )
	{
		_HScroll->show( true );
		_HScroll->enable( true );
	}
}

/**
 * vScroll
 */
void CScrollableControl::vScroll( bool on )
{
	if ( (on == true) && ( _VScroll == NULL) )
	{
		_VScroll = new CScrollBar(0, 0, 0, 0, 0, 0 , _H, 16 , _H_Pixel, false, this);

		_HScroll->hotSpot( CControl::THotSpot::HS_TL );
		_HScroll->origin( CControl::THotSpot::HS_BR );

		_Children.push_back( _VScroll );
	}
	// hide the scroll bar
	else if ( (on == false) && ( _VScroll != NULL) )
	{
		if (_AutoHide == true)
			_VScroll->show( false );
		else
			_VScroll->enable( false );
	}
	// show the scroll bar
	else if ( (on == true)&& ( _VScroll != NULL) )
	{
			_VScroll->show( true );
			_VScroll->enable( true );
	}
}


/**
 * autoHide
 */
void CScrollableControl::autoHide(bool on)
{
	_AutoHide = on;
}


//-----------------------------------------------
// click :
//-----------------------------------------------
void CScrollableControl::click(float x, float y, bool &taken)
{
	if (_VScroll != NULL)
		_VScroll->click(x,y,taken);
	if (_HScroll != NULL)
		_HScroll->click(x,y,taken);
}


//-----------------------------------------------
// clickRight :
//-----------------------------------------------
void CScrollableControl::clickRight(float x, float y, bool &taken)
{
	if (_VScroll != NULL)
		_VScroll->clickRight(x,y,taken);
	if (_HScroll != NULL)
		_HScroll->clickRight(x,y,taken);
}