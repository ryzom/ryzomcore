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



#ifndef CL_OSD_BASE_H
#define CL_OSD_BASE_H

#include "nel/misc/types_nl.h"
#include "osd.h"


/**
 * Class to manage OSD Base.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class COSDBase
{
public:

	/// Constructor
	COSDBase();

	/** \name Background
	 * Variables to manage the background of the OSD.
	 */
	//@{
	/// Display mode for the background.
	COSD::TBG	_BG_Mode;
	/// Id of the texture for the background.
	uint		_BG;
	/// Color of the Background.
	CRGBA		_BG_Color;
	//@}

	/** \name Title Bar
	 * Variables to manage the Title Bar of the OSD.
	 */
	//@{
	/// Display mode for the Title Bar.
	COSD::TTB	_TB_Mode;
	/// Id of the texture for the Title BAr.
	uint		_TB;
	/// Color of the Title Bar.
	CRGBA		_TB_Color;
	/// Pen of the Title Bar.
	CPen		_TB_Pen;
	//@}

	/** \name HighLight
	 * Variables to manage the HighLight of the OSD.
	 */
	//@{
	/// Color of the HighLight.
	CRGBA		_HL_Color;
	/// HighLight Size (in Pixel).
	float		_HL_Size;
	//@}

	/** \name Resize
	 * Variables to manage the Resize of the OSD.
	 */
	//@{
	/// Resize borders Color
	CRGBA		_RS_Color;
	/// Resize size (in pixel).
	float		_RS_Size;
	//@}
};


#endif // CL_OSD_BASE_H

/* End of osd_base.h */
