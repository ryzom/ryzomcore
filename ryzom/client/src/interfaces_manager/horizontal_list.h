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



#ifndef CL_HORIZONTAL_LIST_H
#define CL_HORIZONTAL_LIST_H

// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"

// Client
#include "control.h"
#include "bitmap.h"


/**
 * class for horizontal list of controls (text, bitmaps, buttons...)
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CHorizontalList : public CControl, public CBitmapBase
{
public:

	/// Constructor
	CHorizontalList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint16 spacing = 3, uint texture = 0, NLMISC::CRGBA rgba = CRGBA(255,255,255,255) );

	/// Destructor
	~CHorizontalList();

	/**
	 * set the bitmap to use to scroll left
	 * \param CBitm *bitmap adress of the bitmap to use
	 */
	void setLeftScrollBitmap( CBitm *bitmap);

	/**
	 * set the bitmap to use to scroll right
	 * \param CBitm *bitmap adress of the bitmap to use
	 */
	void setRightScrollBitmap( CBitm *bitmap);

	/**
	 * scroll horizontaly by 'scroll' units in either direction (right if scroll >0 and left is scroll<0 )
	 * \param sint32 scroll : the number of unit to scroll right or left
	 */
	void scroll(sint32 scroll);


	/// Display the control.
	virtual void display();

	/// Manage the click for the control.
	virtual void click(float x, float y, bool &taken);

	/// Manage the right click for the control.
	virtual void clickRight(float x, float y, bool &taken);

	/**
	 * add a control to the list
	 * \param CControl* pCtrl pointer on the control to add
	 */
	virtual void add(CControl *pCtrl);

	/**
	 * set the inter item space
	 * \param uint16 new spacing in pixel
	 */
	void setSpacing(uint16 spacing) { _Spacing = spacing; }

	/// Set some references for the display.
	virtual void ref(float x, float y, float w, float h);

	/**
	 * clear the control - erase all controls
	 * \param bool delElts if true all the controls in the lis will be deleted, otherwise, they will just be removed from the list
	 */
	void clear( bool delElts = true );

	/// center or not the elements in the list
	void center( bool center) { _Centered = center; }


	bool center() const { return _Centered; }

	/// Change the Hot Spot.
	virtual void hotSpot(THotSpot hs);

protected:
	/// the objects (controls) in the list
	TListControl	_Items; // see control.h for the definition of TListControl

	/// the number of displayed elements
	mutable uint16	_NbDisplayedElts;

	/// the number of elements (== _Items.size() )
	uint16			_NbElts;

	/// iterator on the fisrt element displayed in the list
	TListControl::iterator	_StartingIterator;

	/// the bitmap for scrolling to the left
	CBitm *			_ScrollLeft;

	/// the bitmap for scrolling to the right
	CBitm *			_ScrollRight;

	/// spacing (in pixels)
	uint16			_Spacing;

	/// boolean : indicate if controls are centered in the list or not (vertically) default = false
	bool			_Centered;


	/// internal use only, set if the list is cleared during a skim through the controls in the list. clear() will be called on the next display
	bool			_ClearRequest;

	/// internal use only, indicate if the elements of the list should be deleted ( ie : delete calld on controls pointers)
	bool			_DeleteRequest;

	/// internal use only, lock the list during a skim through the controls (if a click on a control of the list clear the list for exemple)
	bool			_Locked;

	/// internal use only, buffer the add() request when the control is locked, add all the elts in the buffer on the next display
	TListControl	_AddBuffer;

private:
	/// Initialize the object (1 function called for all constructors -> easier).
	inline void init();
};


#endif // CL_HORIZONTAL_LIST_H

/* End of horizontal_list.h */
