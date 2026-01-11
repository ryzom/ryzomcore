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



#ifndef CL_ANIMATION_FX_ID_ARRAY_H
#define CL_ANIMATION_FX_ID_ARRAY_H

#include "animation_fx.h"

class CIDToStringArraySheet;

namespace NL3D
{
	class UAnimationSet;
}

/**
  * Sorts animation fx by an arbitrary ID
  * Must be initialized from a .id_to_string_array sheet, which gives the id / sheet_name pairs
  * The sheet name must be a .animation_fx one
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CAnimationFXIDArray
{
public:
	// ctor
	CAnimationFXIDArray();
	/** init from a .id_to_string_array sheet
	  * animation set is required to build fxs tracks.
	  * \param mustDeleteAnimSet true if ownerShip of animset must be given to that object (e.g it is deleted by that object at release())
	  */
	void init(const CIDToStringArraySheet &sheet, NL3D::UAnimationSet *animSet, bool mustDeleteAnimSet = false);
	/** init from a .id_to_string_array sheet name
	  * animation set is required to build fxs tracks.
  	  * \param mustDeleteAnimSet true if ownerShip of animset must be given to that object (e.g it is deleted by that object at release())
	  */
	void init(const std::string &sheetName, NL3D::UAnimationSet *animSet, bool mustDeleteAnimSet = false);
	// release data from that object
	void release();
	// retrieve a fx from its id, or NULL if not known
	const CAnimationFX *getFX(uint32 id) const;
private:
	struct CIDToFX
	{
		uint32 ID;
		CAnimationFX FX;
		bool operator < (const CIDToFX &rhs) const { return this->ID < rhs.ID; }
	};
	std::vector<CIDToFX> _IDToFXArray;
	NL3D::UAnimationSet  *_AnimSet;
};



#endif
