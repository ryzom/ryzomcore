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



#ifndef CL_BODY_TO_BONE_H
#define CL_BODY_TO_BONE_H

#include "game_share/body.h"

#include <nel/misc/string_mapper.h>

namespace NLGEORGES
{
	class UFormElm;
}

/** Sheet that do a match between a part of the body and a bone
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CBodyToBoneSheet
{
public:
	// bone for each body part
	NLMISC::TSStringId Head;
	NLMISC::TSStringId Chest;
	NLMISC::TSStringId LeftArm;
	NLMISC::TSStringId RightArm;
	NLMISC::TSStringId LeftHand;
	NLMISC::TSStringId RightHand;
	NLMISC::TSStringId LeftLeg;
	NLMISC::TSStringId RightLeg;
	NLMISC::TSStringId LeftFoot;
	NLMISC::TSStringId RightFoot;
public:
	// ctor
	CBodyToBoneSheet();
	// build that an external script
	void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	/** From a body part and a side, retrieves bone name (or NULL if none).
	  * The side is meaningless for part such as 'chest'
	  */
	const char *getBoneName(BODY::TBodyPart part, BODY::TSide side) const;
};



#endif
