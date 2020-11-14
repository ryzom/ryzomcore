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



#ifndef CL_INTERF_LIST_H
#define CL_INTERF_LIST_H


/////////////
// Include //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"
// Client
#include "scrollable_control.h"
#include "pen.h"
// Std
#include <vector>


///////////
// Using //
///////////
using std::list;


/**
 * Manages "List" control in Interface.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CList : public CScrollableControl, public CPen
{
private:
	/// Initialize the control (1 function called for all constructors -> easier).
	inline void init();

protected:
	typedef list<ucstring> TItemList;
	TItemList	_ItemsList;

	/// indicate if the list is in auto-scroll mode or not (true : the text will allways scroll to show the last line)
	bool		_AutoScroll;

	/// indicate if the list autowrap it's elements when they are longer than display size
	bool		_AutoWrap;

	/// the max number of lines memorized in the list, 0 => no limit
	uint32		_HistorySize;

	/**
	 * the ending position in the list for display, by default it's the last item in the list (the newest one)
	 * if auto scroll mode is 'on' (_AutoScroll == true), this iterator is always equal to the last item in the list
	 */
	TItemList::reverse_iterator	_EndingIterator;

	/// background texture
	uint	_BackgroundTexture;

	/// background RGBA
	CRGBA	_BackgroundColor;

public:
	/// Constructor
	CList(uint id);
	CList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, const CPen &pen);
	CList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint32 fontSize, CRGBA color, bool shadow);

	/// add text to the list
	virtual void add(const ucstring &str);

	/// Display the Bitmap.
	virtual void display();

	/**
	 * set the auto scroll mode of the list, default mode is 'on' (true)
	 * \param the new mode
	 */
	void setAutoScroll( bool scroll) { _AutoScroll = scroll; }

	/**
	* \return the auto scroll mode of the list control
	*/
	bool getAutoScroll() const { return _AutoScroll; }

	/**
	 * set the max number of memorized lines fo the list,  0 = no limit, default (at creation) is 150
	 * \param the new limit for the number of string in the list
	 */
	void setHistorySize( uint32 size ) { _HistorySize = size; }

	/**
	 * return the current max number of memorized phrases in the control
	 * \return the history max size
	 */
	uint32 getHistorySize() const { return _HistorySize; }

	/// scroll horizontaly by 'scroll' units in either direction (right if scroll >0 and left is scroll<0 for example) (implementation dependent)
	virtual void scrollH(sint32 scroll);

	/// scroll verticaly by 'scroll' units in either direction
	virtual void scrollV(sint32 scroll);

	/**
	 * clear the list, erasing all stored items.
	 */
	void clear();

	/**
	 * set the background texture
	 * \param uint textureId
	 */
	void setBackgroundTexture(uint textureId) { _BackgroundTexture = textureId; }

	/**
	 * set the background color
	 * \param CRGBA &color
	 */
	void setBackgroundColor(const CRGBA &color) { _BackgroundColor = color; }
};


#endif // CL_INTERF_LIST_H

/* End of interf_list.h */
