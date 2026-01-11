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



#ifndef CL_BITMAP_H
#define CL_BITMAP_H


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/types_nl.h"
// Client.
#include "control.h"
#include "bitmap_base.h"
#include "osd.h"


/**
 * Manages "Bitmap" control in Interface.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CBitm : public CControl, public CBitmapBase
{
private:
	/// Initialize the button (1 function called for all constructors -> easier).
	inline void init();

public:
	/// Constructor
	CBitm(uint id = 0);
	CBitm(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, const CBitmapBase &bitmapBase);
	CBitm(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint texture, const CRGBA &rgba);

	/// Display the Bitmap.
	virtual void display();
};


#endif // CL_BITMAP_H

/* End of bitmap.h */
