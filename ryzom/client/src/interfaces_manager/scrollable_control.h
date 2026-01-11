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



#ifndef NL_SCROLLABLE_CONTROL_H
#define NL_SCROLLABLE_CONTROL_H

#include "nel/misc/types_nl.h"
#include "control.h"
#include "scroll_bar.h"

/**
 * class managing scrollable controls. These controls can use scroll bars thanks to virtual methods
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CScrollableControl : public CControl
{
public:

	/// default Constructor
	CScrollableControl(uint id);

	/// constructor
	CScrollableControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel)
		: CControl(id, x, y, x_pixel, y_pixel, w, h, w_pixel, h_pixel)
	{
		_HScroll = new CScrollBar(0, 0, 0, 0, 0, w , 0, w_pixel , 16, false, this);
		_VScroll = new CScrollBar(0, 0, 0, 0, 0, 0 , h, 16 , h_pixel, true, this);

		// set hot spot and origin of the horizontal scroll bar
		_HScroll->hotSpot( CControl::THotSpot::HS_TR);
		_HScroll->origin( CControl::THotSpot::HS_BL);

		// set hot spot and origin of the vertical scroll bar
		_VScroll->hotSpot( CControl::THotSpot::HS_TL);
		_VScroll->origin( CControl::THotSpot::HS_BR);

		// add the scroll bar to the scrollable control children list
		_Children.push_back( _VScroll );
		_Children.push_back( _HScroll );

		_AutoHide = false;
	}

	/// destructor
	virtual ~CScrollableControl();

	/// scroll horizontaly by 'scroll' units in either direction (right if scroll >0 and left is scroll<0 for example) (implementation dependent)
	virtual void scrollH(sint32 scroll) = 0;

	/// scroll verticaly by 'scroll' units in either direction
	virtual void scrollV(sint32 scroll) = 0;

	/// show(enable)/hide(disable)  /create the horizontal scroll bar
	void hScroll( bool on );

	/// show(enable)/hide(disable) /create the vertical scroll bar
	void vScroll( bool on );

	/// enable/disable auto hiding of the scroll bars
	void autoHide( bool on );

	/// manage left mouse button click
	virtual void click(float x, float y, bool &taken);

	/// manage right mouse button click
	virtual void clickRight(float x, float y, bool &taken);

	/// get a pointer on the horizontal scroll bar
	CScrollBar *getHScroll() { return _HScroll; }
	/// get a pointer on the vertical scroll bar
	CScrollBar *getVScroll() { return _VScroll; }

// attributes
protected:
	/// auto-hide the scroll bars when they are unnecessary or just disable them
	bool		_AutoHide;

	/// the horizontal scroll bar
	CScrollBar	*_HScroll;

	/// the vertical scroll bar
	CScrollBar	*_VScroll;
};


#endif // NL_SCROLLABLE_CONTROL_H

/* End of scrollable_control.h */
