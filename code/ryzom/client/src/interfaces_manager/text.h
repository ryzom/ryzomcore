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



#ifndef CL_TEXT_H
#define CL_TEXT_H


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/ucstring.h"
#include "nel/misc/rgba.h"
// 3D Interface.
#include "nel/3d/u_text_context.h"
// Client.
#include "control.h"
#include "pen.h"


///////////
// Using //
///////////
using NLMISC::CRGBA;
using NL3D::UTextContext;


/**
 * Class to manage a Text.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CText : public CControl, public CPen
{
private:
	/// Initialize the button (1 function called for all constructors -> easier).
	inline void init(const ucstring &text);

protected:
	/// Text to display.
	ucstring				_Text;
	/// The text HotSpot (in the text library).
	UTextContext::THotSpot	_TextHotSpot;

	/// index of the computed String associated to this text control
	uint32					_Index;

	/// info on the computed String associated to this text control
	UTextContext::CStringInfo _Info;

	/// Calculate the Display X, Y, Width, Height.
	virtual void calculateDisplay();
	/// Calculate the display position of the control in relation to the position of the control (Hot Spot).
	virtual void calculateHS();

public:
	/// Constructor
	CText(uint id = 0);
	CText(uint id, float x, float y, float x_pixel, float y_pixel, const ucstring &text, const CPen &pen);
	CText(uint id, float x, float y, float x_pixel, float y_pixel, const ucstring &text, uint32 fontSize, CRGBA color, bool shadow);

	/// Display the Bitmap.
	virtual void display();

	/// Get the Text.
	ucstring text();
	/// Set the Text.
	void text(ucstring txt);

	// replace the methods of CPen class (the CComputedString needs to be recomputed when we change color or size or shadow)
	/// Set the font size.
	void fontSize(uint32 fs);

	/// Set the pen color.
	void color(CRGBA color);

	/// Set the shadow state.
	void shadow(bool s);
};


#endif // CL_TEXT_H

/* End of text.h */
