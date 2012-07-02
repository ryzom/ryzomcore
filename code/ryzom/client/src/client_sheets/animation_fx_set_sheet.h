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



#ifndef CL_ANIMATION_FX_SET_SHEET_H
#define CL_ANIMATION_FX_SET_SHEET_H


#include "animation_fx_set_sheet.h"
#include "animation_fx_sheet.h"
#include "entity_sheet.h"

/** A set of anim fx
  * This class allow to inherit an array of fx in georges (otherwise a simple vector would have been sufficient)
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CAnimationFXSetSheet : public CEntitySheet
{
public:
	enum { MaxNumFX = 4 };
	std::vector<CAnimationFXSheet> FX;
	bool						   CanReplaceStickMode[MaxNumFX];
	bool						   CanReplaceStickBone[MaxNumFX];
public:
	// ctor
	CAnimationFXSetSheet();
	/// Build the fx from an external script.
	void buildWithPrefix(const NLGEORGES::UFormElm &item, const std::string &prefix = "");
	// from CEntitySheet
	void build(const NLGEORGES::UFormElm &item);
	/// Serialize sheet into binary data file.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};




#endif
