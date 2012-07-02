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



#ifndef RY_ANIMAL_TYPE_H
#define RY_ANIMAL_TYPE_H

#include "nel/misc/types_nl.h"


namespace ANIMAL_TYPE
{
	enum	EAnimalType
	{
		All = 0,
		Mount,
		Packer,
		Demon,

		AnimalTypeSize
	};

	// Special values for HUNGER database leaf
	const uint DbHungryValue = 0;
	const uint MaxDbSatiety = 31;

	// Special value for DESPAWN_TIMER database leaf
	const uint MaxDbTimeBeforeDespawn = 71;

}; // ANIMAL_TYPE


#endif // RY_ANIMAL_TYPE_H

/* End of animal_type.h */
