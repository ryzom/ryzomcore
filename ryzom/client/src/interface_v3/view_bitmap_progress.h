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



#ifndef RZ_DBVIEW_BAR_H
#define RZ_DBVIEW_BAR_H

#include "nel/misc/types_nl.h"
#include "view_bitmap.h"

/**
 * class implementing a bitmap used as the front texture of a progress bar
 * the bitmap is drawn from _X to _W * _Range/_RangeMax
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class CDBViewBar : CViewBitmap
{
public:

	/// Constructor
	CDBViewBar(const TCtorParam &param)
		: CViewBitmap(param),
		_Slot(TCtorParam()),
		_Bar(TCtorParam())
	{}

	bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);
	virtual uint32 getMemory() { return sizeof(*this)+_Id.size(); }

	virtual void draw ();

protected:

	CViewBitmap	_Slot;
	CViewBitmap _Bar;

	//range of the progression in arbitrary units. should be integer
	CInterfaceProperty _Range;
	//max range of the progression in arbitrary units. should be integer
	CInterfaceProperty _RangeMax;

	CInterfaceProperty _Vertical;

};


#endif // RZ_DBVIEW_BAR_H

/* End of dbview_bar.h */
