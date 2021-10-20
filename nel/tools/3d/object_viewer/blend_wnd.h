// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_BLEND_WND_H
#define NL_BLEND_WND_H

#include "nel/misc/types_nl.h"


/**
 * Window class for the dlg slot blend window
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CBlendWnd
{
public:

	/// Constructor
	CBlendWnd();

	// *** From CWnd

	void OnPaint (const RECT& client, CDC* pDc, float StartBlend, float EndBlend, float StartBlendTime, float EndBlendTime, 
		float Smoothness, float StartTime, float EndTime, bool enabled);

private:
	// Build a rect
	void MakeRect (const RECT& src, RECT& dst, float x, float y, float width, float height);

	// Build a point
	void MakePoint (const RECT& src, POINT& dst, float x, float y);
};


#endif // NL_BLEND_WND_H

/* End of blend_wnd.h */
