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



#ifndef NL_CONTROL_LIST_H
#define NL_CONTROL_LIST_H

// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
// Client
#include "scrollable_control.h"

/**
 * Vertical list of controls with scroll bars
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CControlList : public CScrollableControl
{
public:
	/// Constructor
	CControlList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint16 spacing);

	/// Destructor
	~CControlList();


	/**
	 * add control to the list
	 * \param CControl* pointer on the control to add
	 * \param bool pushFront if true the new control will be added in front of the list instead of the end, and so will be displayed at the top of the list
	 */
	virtual void add( CControl *pCtrl, bool pushFront = false);

	/// Display the list.
	virtual void display();

	/// Manage the click for the control.
	virtual void click(float x, float y, bool &taken);

	/// Manage the right click for the control.
	virtual void clickRight(float x, float y, bool &taken);

	/// scroll horizontaly by 'scroll' units in either direction (right if scroll >0 and left is scroll<0 for example) (implementation dependent)
	virtual void scrollH(sint32 scroll);

	/// scroll verticaly by 'scroll' units in either direction
	virtual void scrollV(sint32 scroll);

	/// Set some references for the display.
	virtual void ref(float x, float y, float w, float h);

	/// remove all items from list, delete them if specified
	virtual void clear(bool deleteElts = true);

protected:
	/// the objects (controls) in the list
	TListControl	_Items; // see control.h for the definition of TListControl

	/// the number of displayed elements
	mutable uint16	_NbDisplayedElts;

	/// the number of elements (== _Items.size() )
	uint16			_NbElts;

	/// iterator on the last element displayed in the list
	TListControl::reverse_iterator	_EndingIterator;

	/// spacing (in pixels)
	uint16			_Spacing;

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
	inline void init( uint16 spacing);

};


#endif // NL_CONTROL_LIST_H

/* End of control_list.h */
