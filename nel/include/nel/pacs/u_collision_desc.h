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

#ifndef NL_U_COLLISION_DESC_H
#define NL_U_COLLISION_DESC_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vectord.h"

#include "u_move_primitive.h"

#include <vector>

namespace NLMISC
{
	class IStream;
}

namespace NLPACS
{

/**
 * Description of the contact of a collision
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UCollisionDesc
{
public:
	NLMISC::CVectorD		ContactPosition;
	NLMISC::CVectorD		ContactNormal0;
	NLMISC::CVectorD		ContactNormal1;
	double					ContactTime;

	// Serial method
	void serial (NLMISC::IStream& stream);
};

/**
 * Description of the contact of a collision
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UTriggerInfo
{
public:
	enum
	{
		In = 0,
		Inside,
		Out
	};

	UMovePrimitive::TUserData	Object0;
	UMovePrimitive::TUserData	Object1;
	UCollisionDesc				CollisionDesc;
	uint8						CollisionType;

	// Serial method
	void serial (NLMISC::IStream& stream);
};

} // NLPACS


#endif // NL_U_COLLISION_DESC_H

/* End of collision_desc.h */
