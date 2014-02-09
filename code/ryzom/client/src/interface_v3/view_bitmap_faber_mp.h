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



#ifndef NL_VIEW_BITMAP_MP_FABER_H
#define NL_VIEW_BITMAP_MP_FABER_H

#include "nel/misc/types_nl.h"
#include "nel/gui/view_bitmap.h"


///\todo nico : do the real display when item icons are available
#include "nel/gui/view_text.h"

/**
 * class used to display mps for faber interface
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CViewBitmapFaberMp : public CViewBitmap
{
public:
	CViewBitmapFaberMp(const TCtorParam &param) : CViewBitmap(param)
	{
		_AccIconBackId = _AccIconMainId = _AccIconOverId = _TextureNoItemId = -2;
	}
	/**
	 * parse an xml node and initialize the base view members. Must call CViewBase::parse
	 * \param cur : pointer to the xml node to be parsed
	 * \param parentGroup : the parent group of the view
	 * \partam id : a refence to the string that will receive the view ID
	 * \return true if success
	 */
	bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);
	virtual uint32 getMemory() { return (uint32)(sizeof(*this) + _Id.size() + _AccIconBackString.size() +
								_AccIconMainString.size() + _AccIconOverString.size() + _TextureNoItemName.size()); }

	/**
	 * draw the view
	 */
	void draw();

private:
	CInterfaceProperty _SheetId;
	std::string _AccIconBackString;
	sint32		_AccIconBackId;
	std::string _AccIconMainString;
	sint32		_AccIconMainId;
	std::string _AccIconOverString;
	sint32		_AccIconOverId;


	std::string _TextureNoItemName;
	sint32		_TextureNoItemId;
	CInterfaceProperty _ColorNoItem;

	CInterfaceProperty _NeededQuantity;
	CInterfaceProperty _Quantity;
	CInterfaceProperty _Quality;
/*	CViewText* _SheetText;
	CViewText* _QuantityText;
	CViewText* _QualityText;*/
};


#endif // NL_VIEW_BITMAP_MP_FABER_H

/* End of view_bitmap_faber_mp.h */
