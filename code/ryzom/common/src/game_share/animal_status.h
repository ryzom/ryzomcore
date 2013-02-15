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



#ifndef NL_ANIMAL_STATUS_H
#define NL_ANIMAL_STATUS_H

#include "nel/misc/types_nl.h"


namespace ANIMAL_STATUS
{
	typedef uint8	EAnimalStatus;

	enum	EAnimalStatusFlag
	{
		/* There is no PresentFlag: the animal is not present if AliveFlag and InLandscapeFlag are not set (cause an animal cannot be dead in a stable!)
			"Present" is tested if not all 0!
		*/
		AliveFlag=					0x0001,		// if set, the animal is alive
		InLandscapeFlag=			0x0002,		// if set, the animal is in Landscape. not set: in Stable
		InventoryAvailableFlag=		0x0004,		// if set, the animal inventory is available
		CanEnterLeaveStableFlag=	0x0008,		// if set, an order "Leave" or "Enter Stable" can be issued
	};

	// true if the animal is present and spawned
	inline bool		isSpawned(EAnimalStatus e)
	{
		return e!=0;
	}

	// true if the animal is present, spawned, and alive
	inline bool		isAlive(EAnimalStatus e)
	{
		return isSpawned(e) && (e&AliveFlag)!=0;
	}

	// true if the animal is present, spawned, but dead
	inline bool		isDead(EAnimalStatus e)
	{
		return isSpawned(e) && (e&AliveFlag)==0;
	}

	// true if the animal is present, spawned, and his inventory is available
	inline bool		isInventoryAvailable(EAnimalStatus e)
	{
		return isSpawned(e) && (e&InventoryAvailableFlag)!=0;
	}

	// true if the animal is present, spawned, and in landscape (dead or not, and whatever the inventory state)
	inline bool		isInLandscape(EAnimalStatus e)
	{
		return isSpawned(e) && (e&InLandscapeFlag)!=0;
	}

	// true if the animal is present, spawned, and in a stable
	inline bool		isInStable(EAnimalStatus e)
	{
		return isSpawned(e) && (e&InLandscapeFlag)==0;
	}

	// true if the animal is present, and can enter / leave stable
	inline bool		canEnterLeaveStable(EAnimalStatus e)
	{
		return isSpawned(e) && (e&CanEnterLeaveStableFlag)!=0;
	}


}; // ANIMAL_STATUS


#endif // NL_ANIMAL_STATUS_H

/* End of animal_status.h */
