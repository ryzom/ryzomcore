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



#ifndef CL_FX_STICK_MODE_H
#define CL_FX_STICK_MODE_H


#include "nel/misc/string_mapper.h"

namespace NLGEORGES
{
	class UFormElm;
}

/**
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CFXStickMode
{

public:

	enum TStickMode
	{
		Spawn = 0,
		UserBone,
		Follow,
		FollowNoRotation,
		OrientedTowardTargeter,
		UserBoneOrientedTowardTargeter,
		StaticMatrix,
		StaticObjectCastRay = StaticMatrix, // cast ray for object such as towers (static once created)
		UserBoneRay,
		SpawnPermanent // spawn and delegate to fx manager when the fx is stopped
	};

public:

	TStickMode			Mode;
	NLMISC::TStringId	UserBoneName; // bone to which the fx must be sticked when StickBone == UserBone

public:

	// Constructor
	CFXStickMode() : Mode(Follow), UserBoneName(0) {}

	/// Build the stick mode from an external script.
	bool build(const NLGEORGES::UFormElm &item, const std::string &prefix = "");

	/// Save and Load
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

#endif
