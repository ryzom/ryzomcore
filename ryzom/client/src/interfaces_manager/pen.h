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



#ifndef CL_PEN_H
#define CL_PEN_H

/////////////
// Include //
/////////////
//Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"


///////////
// Using //
///////////
using NLMISC::CRGBA;

/**
 * Class to manage a Pen.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CPen
{
protected:
	uint32		_FontSize;
	CRGBA		_Color;
	bool		_Shadow;

public:
	/// Constructor
	CPen();
	CPen(uint32 fontSize, CRGBA color, bool shadow);

	/// Get the font size.
	uint32 fontSize() const;
	/// Set the font size.
	void fontSize(uint32 fs);

	/// Get the pen color.
	CRGBA color() const;
	/// Set the pen color.
	void color(CRGBA color);

	/// Get the shadow state.
	bool shadow() const;
	/// Set the shadow state.
	void shadow(bool s);
};


#endif // CL_PEN_H

/* End of pen.h */
