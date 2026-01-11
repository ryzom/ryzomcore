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



#ifndef NL_CHOICE_LIST_H
#define NL_CHOICE_LIST_H

#include "nel/misc/types_nl.h"
#include "interf_list.h"


/**
 * <Class description>
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CChoiceList : public CList
{
	TItemList::reverse_iterator	_ItSelected;

	sint _NumSelected;

public:
	/// Constructor
	CChoiceList(uint id);
	CChoiceList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, const CPen &pen);
	CChoiceList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint32 fontSize, CRGBA color, bool shadow);

	/// Display the Bitmap.
	virtual void display();

	virtual void click(float x, float y, bool &taken);

	/**
	 * called when the mouse has moved
	 * \param the x coordinate of the mouse
	 * \param the y coordinate of the mouse
	 */
	virtual void mouseMove( float x, float y);
};


#endif // NL_CHOICE_LIST_H

/* End of choice_list.h */
